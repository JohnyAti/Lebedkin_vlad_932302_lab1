#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int r)
{
    wasSigHup = 1;
}

int main() 
{
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) 
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr = 
    {
        .sin_family = AF_INET,
        .sin_port = htons(12345),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
    {
        perror("bind");
        close(listenSocket);
        exit(1);
    }

    if (listen(listenSocket, SOMAXCONN) == -1) 
    {
        perror("listen");
        close(listenSocket);
        exit(1);
    }

    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    printf("Server started on port 12345\n");

    int activeConnection = -1; 

    while (1)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(listenSocket, &fds);
        int maxFd = listenSocket;

        if (activeConnection != -1) {
            FD_SET(activeConnection, &fds);
            if (activeConnection > maxFd) maxFd = activeConnection;
        }

        int ready = pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask);
        if (ready == -1) {
            if (errno == EINTR) 
            {
                if (wasSigHup)
                 {
                    printf("Received SIGHUP signal\n");
                    wasSigHup = 0;
                }
                continue;
            }
            perror("pselect");
            break;
        }

        if (FD_ISSET(listenSocket, &fds)) 
        {
            int newConn = accept(listenSocket, NULL, NULL);
            if (newConn == -1)
             {
                perror("accept");

            } 
            else 
            {
                printf("New connection: fd=%d\n", newConn);
                if (activeConnection == -1) 
                {
                    activeConnection = newConn;
                    printf("Connection %d accepted\n", newConn);
                } else 
                {
                    close(newConn);
                    printf("Connection %d closed (already have active connection)\n", newConn);
                }
            }
        }

     
        if (activeConnection != -1 && FD_ISSET(activeConnection, &fds))
         {
            char buf[1024];
            ssize_t bytesRead = recv(activeConnection, buf, sizeof(buf), 0);
            if (bytesRead > 0)
             {
                printf("Received %zd bytes from connection %d\n", bytesRead, activeConnection);
            } 
            else
            {
                if (bytesRead == 0) 
                {
                    printf("Connection %d closed by peer\n", activeConnection);
                } 
                else
                {
                    perror("recv");
                }
                close(activeConnection);
                activeConnection = -1;
            }
        }
    }

    if (activeConnection != -1) close(activeConnection);
    close(listenSocket);
    return 0;
}
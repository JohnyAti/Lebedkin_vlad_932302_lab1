#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <sys/select.h>

using namespace std;

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int) 
{
    wasSigHup = 1;
}

int main() 
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) 
    {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6000);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == -1) 
    {
        perror("bind");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) 
    {
        perror("listen");
        close(serverSocket);
        return 1;
    }

    struct sigaction sa{};
    sigaction(SIGHUP, nullptr, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, nullptr);

    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    vector<int> clients;
    cout << "Server started on port 6000" << endl;

    while (true) 
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(serverSocket, &fds);
        
        int maxFd = serverSocket;
        for (int client : clients)
        {
            FD_SET(client, &fds);
            if (client > maxFd) maxFd = client;
        }

        int res = pselect(maxFd + 1, &fds, nullptr, nullptr, nullptr, &origMask);
        
        if (res == -1) 
        {
            if (errno == EINTR) 
            {
                if (wasSigHup) 
                {
                    cout << "Received SIGHUP signal" << endl;
                    wasSigHup = 0;
                }
                continue;
            }
            perror("pselect");
            break;
        }

        if (FD_ISSET(serverSocket, &fds)) 
        {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == -1) 
            {
                perror("accept");
                continue;
            }

            if (clients.empty()) 
            {
                clients.push_back(clientSocket);
                cout << "New connection accepted" << endl;
            } else 
            {
                cout << "Connection closed (only one allowed)" << endl;
                close(clientSocket);
            }
        }

        for (auto it = clients.begin(); it != clients.end(); ) 
        {
            if (FD_ISSET(*it, &fds)) 
            {
                char buffer[1024];
                ssize_t  bytesRead = recv(*it, buffer, sizeof(buffer), 0);
                
                if (bytesRead > 0) 
                {
                    cout << "Received " << bytesRead << " bytes from client" << endl;
                    it++;
                } 
                else 
                {
                    close(*it);
                    it = clients.erase(it);
                    cout << "Client disconnected" << endl;
                }
            } 
            else 
            {
                it++;
            }
        }
    }

    for (int client : clients) close(client);
    close(serverSocket);
    return 0;
}
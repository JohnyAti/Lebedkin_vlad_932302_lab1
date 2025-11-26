
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <clocale>

using namespace std;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;

void* provider(void* arg) 
{
    int counter = 0;
    while (counter < 5)
     { 
        sleep(1); 
        
        pthread_mutex_lock(&lock);
        if (ready == 1) 
        {
            pthread_mutex_unlock(&lock);
            continue;
        }
        ready = 1;
        printf("Событие отправлено \n");
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&lock);
        
        counter++;
    }
    return NULL;
}

void* consumer(void* arg)
 {
    int counter = 0;
    while (counter < 5) 
    {
        pthread_mutex_lock(&lock);
        while (ready == 0) 
        {
            pthread_cond_wait(&cond1, &lock);
        }
        ready = 0;
        printf("Событие принято\n");
        pthread_mutex_unlock(&lock);
        
        counter++;
    }
    return NULL;
}

int main() 
{
    setlocale(LC_ALL,"Russian");
    pthread_t provider_thread, consumer_thread;
    
    pthread_create(&provider_thread, NULL, provider, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);
    
    pthread_join(provider_thread, NULL);
    pthread_join(consumer_thread, NULL);
    
    return 0;
}
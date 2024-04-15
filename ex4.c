#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h> 

pthread_mutex_t mutex;

char* producer();
void* consumer();


int main(int argc, char* argv[]) {
    pthread_t p1, p2, c1, c2;

    pthread_create(&p1, NULL, &consumer, NULL);
    pthread_create(&p2, NULL, &consumer, NULL);

    pthread_join(p2, NULL);
    pthread_join(p1, NULL);

    return 0;
} 

char* producer() {
    
}

void* consumer() {
    printf("testing this thread\n");
}
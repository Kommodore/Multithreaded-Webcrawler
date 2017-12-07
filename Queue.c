#include <memory.h>
#include "Queue.h"

void queueInit(Queue* queue){
    if(pthread_mutex_init(&queue->locked, NULL) != 0){
        printf("Could not create mutex.");
        exit(1);
    }

    if(pthread_cond_init(&queue->notFull, NULL) != 0){
        printf("Could not create notFull condition");
        exit(1);
    }

    if(pthread_cond_init(&queue->notEmpty, NULL) != 0){
        printf("Could not create notEmpty condition");
        exit(1);
    }

    queue->full = 0;
    queue->elements = 0;
    queue->finished = 0;
    queue->empty = 1;
}

void queueDelete(Queue* queue){
    queue->empty = 1;
    queue->elements = 0;
    queue->finished = 0;
    pthread_mutex_destroy(&queue->locked);
    pthread_cond_destroy(&queue->notEmpty);
    pthread_cond_destroy(&queue->notFull);
}

void queuePush(Queue* queue, char* hostname, char* documentPath, int id){
    if(queue->elements != QUEUESIZE){
        strcpy(queue->hosts[queue->elements].hostname, hostname);
        strcpy(queue->hosts[queue->elements].documentPath, documentPath);
        queue->hosts[queue->elements].id = id;

        queue->elements++;
        queue->empty = 0;
        pthread_cond_signal(&queue->notEmpty);
    }

    if(queue->elements == QUEUESIZE){
        queue->full = 1;
    }
}

Host queuePop(Queue* queue){
    if(queue->empty == 0){
        Host temp = queue->hosts[0];
        for(int i = 0; i < QUEUESIZE-1; i++){
            queue->hosts[i]= queue->hosts[i+1];
        }

        queue->elements--;
        queue->full = 0;
        if(queue->elements == 0){
            queue->empty = 1;
        }

        pthread_cond_signal(&queue->notFull);
        return temp;
    }

    return (Host){.documentPath = "", .hostname = "", .id= -1};
}

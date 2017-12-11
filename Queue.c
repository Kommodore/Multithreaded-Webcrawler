#include <memory.h>
#include "Queue.h"

void queueDelete(Queue* queue){
    queue->empty = 1;
    queue->elements = 0;
    queue->finished = 0;
    queue->offset = 0;
    pthread_mutex_destroy(&queue->locked);
    pthread_cond_destroy(&queue->notEmpty);
    pthread_cond_destroy(&queue->notFull);
}

void queuePush(Queue* queue, char* hostname, char* documentPath, int id){
    if(queue->elements != QUEUESIZE){
        strcpy(queue->hosts[(queue->elements+queue->offset)%QUEUESIZE].hostname, hostname);
        strcpy(queue->hosts[(queue->elements+queue->offset)%QUEUESIZE].documentPath, documentPath);
        queue->hosts[(queue->elements+queue->offset)%QUEUESIZE].id = id;

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
        Host temp = queue->hosts[queue->offset];
        queue->offset++;
        if(queue->offset == QUEUESIZE){
            queue->offset = 0;
        }

        queue->elements--;
        queue->full = 0;
        if(queue->elements < 0){
            queue->empty = 1;
            queue->elements = 0;
        }

        pthread_cond_signal(&queue->notFull);
        return temp;
    }

    return (Host){.documentPath = "", .hostname = "", .id= -1};
}

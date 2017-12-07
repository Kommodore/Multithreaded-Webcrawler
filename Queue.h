#ifndef BS_PRAKTIKUM2_QUEUE_H
#define BS_PRAKTIKUM2_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define QUEUESIZE 8

typedef struct{
    char hostname[256];
    char documentPath[256];
    int id;
} Host;

typedef struct{
    int empty;
    int full;
    int finished;
    pthread_mutex_t locked;
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
    int elements;
    Host hosts[QUEUESIZE];
} Queue;

void queueInit(Queue* queue);

void queueDelete(Queue* queue);

void queuePush(Queue* queue, char* hostname, char* documentPath, int id);

Host queuePop(Queue* queue);

#endif

#ifndef BS_PRAKTIKUM2_WEBBOT_H
#define BS_PRAKTIKUM2_WEBBOT_H

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>

#include "msocket.h"
#include "Queue.h"

#define THREADCOUNT 3

typedef struct {
    Queue* queue;
    char* fileName;
} toPass;

typedef struct {
    Queue* queue;
    int threadId;
} toPass2;

void getFileName(char* filename);

Host* fetchHosts(Queue* queue, FILE* file, Host* save, int* line);

void* readerThread(void* file);

void* workerThread(void* arg);

void saveSiteContent(int hostNumber, int threadId, const char* address, const char* page);

#endif //BS_PRAKTIKUM2_WEBBOT_H

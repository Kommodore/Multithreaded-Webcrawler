#ifndef BS_PRAKTIKUM2_WEBBOT_H
#define BS_PRAKTIKUM2_WEBBOT_H

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>

#include "msocket.h"
#include "Queue.h"

#define THREADCOUNT 3

void getFileName(char* filename);

void fetchHosts(FILE* file, int* line);

void* readerThread(void* file);

void* workerThread(void* arg);

void saveSiteContent(int hostNumber, pthread_t threadId, const char* address, const char* page);

#endif //BS_PRAKTIKUM2_WEBBOT_H

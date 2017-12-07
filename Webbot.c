#include "Webbot.h"
#include "Queue.h"

Queue queue;

int main(){
	char filename[256];
    pthread_t workerThreads[THREADCOUNT];
    pthread_t readerThreadId = NULL;
    pthread_t testThread = NULL;

    getFileName(filename);

    int errorCode = pthread_create(&readerThreadId, NULL, readerThread, (void*)(char*)filename);
    if(errorCode != 0){
        printf("Reader Thread konnte nicht erstellt werden");
    }


    errorCode = pthread_create(&testThread, NULL, workerThread, (void*)1);
    if(errorCode != 0){
        printf("Test Thread konnte nicht erstellt werden");
    }

    /*for(unsigned int i = 0; i < THREADCOUNT; i++){
        errorCode = pthread_create(&(workerThreads[i]), NULL, workerThread, (void*)(long)i);
        if(errorCode != 0){
            printf("Thread %i konnte nicht erstellt werden",i);
            exit(1);
        }
    }

    for(int i = 0; i < THREADCOUNT; i++){
        pthread_join(workerThreads[i], NULL);
    }*/
     pthread_join(readerThreadId, NULL);

    queueDelete(&queue);
    printf("Programm beendet");
    exit(0);
}

void* readerThread(void* file){
    char* filename = (char*)file;
    FILE* pagelist;
    int line = 0;

    if((pagelist = fopen(filename, "r")) == NULL){
        printf("Datei %s konnte nicht geöffnet werden. Programm wird beendet", filename);
        exit(2);
    }

    queueInit(&queue);
    while(!queue.finished){
        pthread_mutex_lock(&queue.locked);
        while(queue.full){
            printf("Warteschlange ist voll. Thread Reader wartet...\n");
            pthread_cond_wait(&queue.notFull, &queue.locked);
        }
        fetchHosts(pagelist, &line);
        pthread_mutex_unlock(&queue.locked);
    }
    fclose(pagelist);

    return NULL;
}

void* workerThread(void* id){

    const int threadId = (int)id;
    while(!queue.finished){
        pthread_mutex_lock(&queue.locked);
        while(queue.empty){
            printf("Warteschlange leer. Thread %d wartet...\n", threadId);
            pthread_cond_wait(&queue.notEmpty, &queue.locked);
        }
        Host currHost = queuePop(&queue);
        pthread_mutex_unlock(&queue.locked);
        if(strcmp(currHost.hostname, "") != 0){
            saveSiteContent(currHost.id, threadId, currHost.hostname, currHost.documentPath);
        }
    }

    return NULL;
}

void getFileName(char* filename){
    /*printf("Welche Datei soll geöffnet werden?\n");
    fgets(filename, 256, stdin);
    strtok(filename, "\n");*/
    strcpy(filename, "testSites.txt");
}

void fetchHosts(FILE* file, int* line){
    char hostname[256];
    char documentPath[256];
    while (fscanf(file, "%s %s\n", hostname, documentPath) != EOF){
        queuePush(&queue, hostname, documentPath, *line);
        (*line)++;
        return;
    }
    queue.finished = 1;
}

void saveSiteContent(int hostNumber, int threadId, const char* address, const char* page){
    char temp[128];
	char fileName[256] = "file_";
    sprintf(temp, "%d", hostNumber);
	strcat(fileName, temp);
	strcat(fileName, "_");
    sprintf(temp, "%d", threadId);
	strcat(fileName, temp);
	strcat(fileName, ".html");

    int error = askServer(address, page, fileName);

    if(error != 0){
        printf("Die Seite konnte nicht gefetcht werden!");
    }
}

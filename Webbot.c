#include "Webbot.h"

Queue queue;

int main(){
	char filename[256];
    time_t startTime, endTime;
    pthread_t workerThreads[THREADCOUNT];
    pthread_t readerThreadId = NULL;

    getFileName(filename);

    time(&startTime);

    // Create threads
    int errorCode = pthread_create(&readerThreadId, NULL, readerThread, (void*)(char*)filename);
    if(errorCode != 0){
        printf("Reader Thread konnte nicht erstellt werden");
        exit(1);
    }

    for(unsigned int i = 0; i < THREADCOUNT; i++){
        errorCode = pthread_create(&(workerThreads[i]), NULL, workerThread, (void*)(long)i);
        if(errorCode != 0){
            printf("Worker Thread %i konnte nicht erstellt werden",i);
            exit(1);
        }
    }

    for(int i = 0; i < THREADCOUNT; i++){
        pthread_join(workerThreads[i], NULL);
    }
    pthread_join(readerThreadId, NULL);
    time(&endTime);

    queueDelete(&queue);
    printf("Programm beendet in %f Sekunden", difftime(endTime, startTime));
    exit(0);
}

/**
 * Parse a file for urls and save it's content in a queue.
 *
 * @param file The file to be parsed
 * @return
 */
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

/**
 * Read URL's from the queue and save their content in files.
 *
 * @param id The id of the Thread
 * @return
 */
void* workerThread(void* id){

    const int threadId = (*(int*)id);
    while(!queue.finished || !queue.empty){
        pthread_mutex_lock(&queue.locked);
        while(queue.empty){
            if(queue.finished){
                return NULL;
            }
            printf("Warteschlange ist leer. Thread %d wartet...\n", threadId);
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

/**
 * Ask the user for the name of the input file.
 *
 * @param filename
 */
void getFileName(char* filename){
    printf("Welche Datei soll geöffnet werden?\n");
    fgets(filename, 256, stdin);
    strtok(filename, "\n");
}

/**
 * Read the hosts line by line from the file.
 *
 * @param file File to be parsed
 * @param line The current line
 */
void fetchHosts(FILE* file, int* line){
    char hostname[256];
    char documentPath[256];
    if(fscanf(file, "%s %s\n", hostname, documentPath) != EOF){
        queuePush(&queue, hostname, documentPath, *line);
        (*line)++;
    } else {
        queue.finished = 1;
    }
}

/**
 * Request the content from an URL and save it into a file.
 *
 * @param hostNumber The id of the host (order in which the addresses where read from the input file)
 * @param threadId The id of the corresponding thread
 * @param address The address part of the url
 * @param page The directory part of the url
 */
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
    printf("Thread %d fetched %s\n",threadId, address);
    if(error != 0){
        printf("Die Seite konnte nicht gefetcht werden!");
    }
}

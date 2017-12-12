#include "Webbot.h"
#include "Queue.h"

int main(){
    Queue queue = {
            .elements = 0,
            .empty = 1,
            .full = 0,
            .finished = 0,
            .locked = PTHREAD_MUTEX_INITIALIZER,
            .notEmpty = PTHREAD_COND_INITIALIZER,
            .notFull = PTHREAD_COND_INITIALIZER,
    };

	char filename[256];
    time_t startTime, endTime;

    // Reader Thread Zeugs
    pthread_t readerThreadId;
    toPass readerData = {.fileName = filename, .queue = &queue};

    // Worker Thread Zeugs
    pthread_t workerThreads[THREADCOUNT];
    toPass2 workerData[THREADCOUNT];


    getFileName(filename);
    time(&startTime);

    // Create threads
    int errorCode = pthread_create(&readerThreadId, NULL, readerThread, &readerData);
    if(errorCode != 0){
        printf("Reader Thread konnte nicht erstellt werden");
        exit(1);
    }

    for(int i = 0; i < THREADCOUNT; i++){
        workerData[i].queue = &queue;
        workerData[i].threadId = i;
        errorCode = pthread_create(&(workerThreads[i]), NULL, workerThread, &workerData[i]);
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
void* readerThread(void* data){
    toPass* temp = (toPass*)data;
    Queue* queue = temp->queue;
    char* filename = temp->fileName;
    FILE* pagelist;
    Host* tempHost = malloc(sizeof(Host));
    int line = 0;

    if((pagelist = fopen(filename, "r")) == NULL){
        printf("Datei %s konnte nicht geöffnet werden. Programm wird beendet", filename);
        exit(2);
    }

    while(1){
        tempHost = fetchHosts(queue, pagelist, tempHost, &line); // Eine Zeile aus der Datei auslesen

        if(!queue->finished){  // Falls Zeile vorhanden in Warteschlange packen, sollte die nicht voll sein
            pthread_mutex_lock(&queue->locked);
            while(queue->full){
                printf("Warteschlange ist voll. Thread Reader wartet...\n");
                pthread_cond_signal(&queue->notEmpty);
                pthread_cond_wait(&queue->notFull, &queue->locked);
            }
            queuePush(queue, tempHost);
            pthread_mutex_unlock(&queue->locked);
        } else {
            fclose(pagelist);
            free(tempHost);
            return NULL;
        }
    }
}

/**
 * Read URL's from the queue and save their content in files.
 *
 * @param id The id of the Thread
 * @return
 */
void* workerThread(void* arg){
    toPass2* data= arg;
    Queue* queue = data->queue;
    int threadId = data->threadId;
    while(1){
        pthread_mutex_lock(&queue->locked);
        if(queue->finished && queue->empty){ // Wenn Warteschlange beendet (keine neuen Einträge) UND Warteschalnge leer Prozess verlassen
            pthread_mutex_unlock(&queue->locked);
            return NULL;
        }

        while(queue->empty){
            printf("Warteschlange ist leer. Thread %u wartet...\n", threadId);
            pthread_cond_signal(&queue->notFull);
            pthread_cond_wait(&queue->notEmpty, &queue->locked);
        }

        Host currHost = queuePop(queue);
        pthread_mutex_unlock(&queue->locked);
        if(strcmp(currHost.hostname, "") != 0){ // Sollte Eintrag vorhanden sein Seite fetchen und speichern
            saveSiteContent(currHost.id, threadId, currHost.hostname, currHost.documentPath);
        }
    }
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
Host* fetchHosts(Queue* queue, FILE* file, Host* save, int* line){
    char hostname[256];
    char documentPath[256];
    if(fscanf(file, "%s %s\n", hostname, documentPath) != EOF){
        strcpy(save->hostname, hostname);
        strcpy(save->documentPath, documentPath);
        save->id = *line;
        (*line)++;
        return save;
    } else { // Dummy Platzhalter wenn keine neue Zeile (Struct kann in C nicht NULL sein)
        queue->finished = 1;
        strcpy(save->hostname, "");
        strcpy(save->documentPath, "");
        save->id = -1;
        return save;
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
    sprintf(temp, "%u", threadId);
	strcat(fileName, temp);
	strcat(fileName, ".html");

    int error = askServer(address, page, fileName);
    printf("Thread %u fetched %s\n",threadId, address);
    if(error != 0){
        printf("Die Seite konnte nicht gefetcht werden!");
    }
}

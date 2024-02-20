#include "fileManager.h"

//tocar aquests
my_semaphore sem = new my_semaphore();


void  initialiseFdProvider(FileManager * fm, int argc, char **argv) {
    // Complete the initialisation
    /* Your rest of the initailisation comes here*/
    sem.lock = 1;
    sem.cond= 1; //num de threads q poden pasar
    sem.i= 0; //
    
    pthread_mutex_lock(&fm->lock);

    fm->nFilesTotal = argc -1;
    fm->nFilesRemaining = fm->nFilesTotal;
    // Initialise enough memory to  store the arrays
    fm->fdData = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fdCRC= malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileFinished = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileAvailable = malloc(sizeof(int) * fm->nFilesTotal);

    int i;
    for (i = 1; i < fm->nFilesTotal +1; ++i) {
        char path[100];
        strcpy(path, argv[i]);
        strcat(path, ".crc");
        fm->fdData[i] = open(argv[i], O_RDONLY);
        fm->fdCRC[i] = open(path, O_RDONLY);

        fm->fileFinished[i] = 0;
        fm->fileAvailable[i] = 1;
    }
}

void  destroyFdProvider(FileManager * fm) {
    int i;
    for (i = 0; i < fm->nFilesTotal; i++) {
        close(fm->fdData[i]);
        close(fm->fdCRC[i]);
    }
    free(fm->fdData);
    free(fm->fdCRC);
    free(fm->fileFinished);
}

int getAndReserveFile(FileManager *fm, dataEntry * d) {
    // This function needs to be implemented by the students
    pthread_mutex_lock(&fm->lock);
    while(sem.i==0){
        pthread_cond_wait(&sem.cond,&fm->lock);
    }
    
    
    int i;
    for (i = 0; i < fm->nFilesTotal; ++i) {
        if (fm->fileAvailable[i] && !fm->fileFinished[i]) {
            d->fdcrc = fm->fdCRC[i];
            d->fddata = fm->fdData[i];
            d->index = i;


            sem.i--;
            pthread_mutex_lock(&fm->lock);

            // You should mark that the file is not available 
        
            return 0;
        }
    }  
    pthread_mutex_unlock(&fm->lock);
    return 1;
}

void unreserveFile(FileManager *fm,dataEntry * d) {
    pthread_mutex_lock(&fm->lock);
    
    fm->fileAvailable[d->index] = 1;

    pthread_mutex_unlock(&fm->lock); 
}

void markFileAsFinished(FileManager * fm, dataEntry * d) {
    pthread_mutex_lock(&fm->lock);
    
    fm->fileFinished[d->index] = 1;
    fm->nFilesRemaining--; //mark that a file has finished
    if (fm->nFilesRemaining == 0) {
        printf("All files have been processed\n");
        //TO COMPLETE: unblock all waiting threads, if needed
        pthread_cond_broadcast(&fm->condition);
    }
    pthread_mutex_unlock(&fm->lock);
}

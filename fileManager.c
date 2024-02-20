#include "fileManager.h"
#include "myutils.h"

//tocar aquests
my_semaphore sem;


void  initialiseFdProvider(FileManager * fm, int argc, char **argv) {
    my_sem_init(&sem, 1);
    // Complete the initialisation
    /* Your rest of the initailisation comes here*/
    //sem.lock = 1;
    //sem.cond= 1; //num de threads q poden pasar
    //sem.i= 0; //
    
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
    int i;
    for (i = 0; i < fm->nFilesTotal; ++i) {
        my_sem_wait(&sem);
        if (fm->fileAvailable[i] && !fm->fileFinished[i]) {
            
            d->fdcrc = fm->fdCRC[i];
            d->fddata = fm->fdData[i];
            d->index = i;

            fm->fileAvailable[i]=0;

            sem.i--;
            my_sem_signal(&sem);

            // You should mark that the file is not available 

        
            return 0;
        }
         my_sem_signal(&sem);

    }  
    return 1;
}

void unreserveFile(FileManager *fm,dataEntry * d) {
    my_sem_wait(&sem);
    
    fm->fileAvailable[d->index] = 1;

    my_sem_signal(&sem); 
}

void markFileAsFinished(FileManager * fm, dataEntry * d) {
    
    my_sem_wait(&sem);
    
    fm->fileFinished[d->index] = 1;
    fm->nFilesRemaining--; //mark that a file has finished
    if (fm->nFilesRemaining == 0) {
        printf("All files have been processed\n");
        //TO COMPLETE: unblock all waiting threads, if needed.
        pthread_cond_broadcast(&sem.cond);
        
    }
    my_sem_signal(&sem); 
}

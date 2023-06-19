/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.c                       
    PURPOSE: implementation of reader module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "reader.h"
#include "../enums/enums.h"

// PATH FOR FILE THAT READER WILL RECEIVE DATA FROM
#define PATH "/home/denis/CUT/f.txt"

// PROTOTYPE FUNCTIONS FOR INSIDE WORLD
int Reader_read(ProcessorStats* const, const long);
static void* Reader_threadf(void* const);

// STRUCTURE FOR HOLDING READER OBJECT
struct reader {
    Buffer* buffer;
    pthread_t thread;
    bool thread_started;
    char padding[7];
    long proc;
};

// STRUCTURE FOR HOLDING PARAMS PASSED TO READER THREAD FUNCTION
typedef struct ThreadParams {
    Reader* reader;
    volatile sig_atomic_t* status;
} ThreadParams;

/*
    METHOD: Reader_init
    PURPOSE: creation of Reader 'object'
    RETURN: Reader 'object' or NULL in 
        case creation was not possible 
*/
Reader* Reader_init(
    Buffer* const buffer,
    const long proc
) {
    printf("[READER]: INIT STARTED\n");

    if (proc <= 0 || buffer == NULL) { return NULL; }
    
    Reader* reader = (Reader*) malloc(sizeof(Reader));
    
    if (reader == NULL) { return NULL; }

    *reader = (Reader) { 
        .buffer = buffer,
        .thread_started = false,
        .proc = proc
    };

    printf("[READER]: INIT FINISHED\n");

    return reader;
}

/*
    METHOD: Reader_start
    PURPOSE: creation and start of a given 
        object's thread
    RETURN: finish code meaning if function 
        worked propely
*/
int Reader_start(
    Reader* const reader,
    volatile sig_atomic_t* status
) {
    printf("[READER]: START STARTED\n");

    if (
        reader == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    ThreadParams* params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .reader = reader,
        .status = status
    };

    if (pthread_create(&(reader -> thread), NULL, Reader_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    reader -> thread_started = true;

    printf("[READER]: START FINISHED\n");

    return SUCCESS;
}

int Reader_join(
    Reader* const reader
) {
    printf("[READER]: JOIN STARTED\n");

    if (reader == NULL) { return ERR_PARAMS; }
    if (reader -> thread_started == false) { return ERR_PARAMS; }
    if (pthread_join(reader -> thread, NULL) != 0) {
        return ERR_JOIN;
    }

    printf("[READER]: JOIN FINISHED\n");

    return SUCCESS;
}

/*
    METHOD: Reader_thread_function
    PURPOSE: acomplishing reader's thread work
    RETURN: NULL
*/
static void* Reader_threadf(
    void* const args
) {
    printf("[READER]: THREAD FUNCTION STARTED\n");

    ThreadParams* params = (ThreadParams*) args;
    ProcessorStats stats;
    struct timespec sleepTime;

    while (*(params -> status) == RUNNING) {
        if (Reader_read(&stats, params -> reader -> proc) != SUCCESS) {
            break;
        }
        
        if (Buffer_push(params -> reader -> buffer, &stats) != SUCCESS) {
            free(stats.cores);
            break;
        }

        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        nanosleep(&sleepTime, NULL);
    }

    printf("[READER]: THREAD FUNCTION FINISHED\n");

    free(params);

    pthread_exit(NULL);
}

/*
    METHOD: Reader_read
    PURPOSE: reads all required data from a saved file field
    RETURN: Stats 'object' including necessary data or null
*/
int Reader_read(
    ProcessorStats* const processorStats,
    const long proc
) {
    printf("[READER]: READ STARTED\n");

    if (processorStats == NULL || proc <= 0) { return ERR_PARAMS; }
    
    FILE* file = fopen(PATH, "r");

    if (file == NULL) {
        return ERR_FILE_OPEN; 
    }

    if (fscanf(
            file, 
            "cpu %d %d %d %d %d %d %d %d %*d %*d\n",
            &(processorStats -> average.user), 
            &(processorStats -> average.nice), 
            &(processorStats -> average.system),
            &(processorStats -> average.idle), 
            &(processorStats -> average.iowait), 
            &(processorStats -> average.irq), 
            &(processorStats -> average.sortirq),
            &(processorStats -> average.steal)
        ) != 8
    ) {
        return ERR_FILE_READ;
    }

    processorStats -> cores = (CoreStats*) malloc(sizeof(CoreStats) * (unsigned long) proc);

    if (processorStats -> cores == NULL) {
        fclose(file);
        return ERR_ALLOC; 
    }

    int coreCount = 0;

    while (coreCount < proc) {
        if (fscanf(
                file, 
                "cpu%*d %d %d %d %d %d %d %d %d %*d %*d\n",
                &(processorStats -> cores[coreCount].user), 
                &(processorStats -> cores[coreCount].nice), 
                &(processorStats -> cores[coreCount].system),
                &(processorStats -> cores[coreCount].idle), 
                &(processorStats -> cores[coreCount].iowait), 
                &(processorStats -> cores[coreCount].irq), 
                &(processorStats -> cores[coreCount].sortirq),
                &(processorStats -> cores[coreCount].steal)
            ) != 8
        ) {
            fclose(file);
            free(processorStats -> cores);
            return ERR_FILE_READ;
        }

        coreCount++;
    }

    fclose(file);

    processorStats -> count = coreCount;

    printf("[READER]: READ FINISHED\n");

    return SUCCESS; 
}

/*
    METHOD: Reader_destroy
    PURPOSE: frees reserved memory for a given Reader 'object' and its nested 'objects'
    RETURN: Reader 'object' or NULL in 
        case creation was not possible 
*/
void Reader_destroy(
    Reader* const reader
) {
    printf("[READER]: DESTROY STARTED\n");

    if (reader == NULL) { return; }

    reader -> proc = 0;
    reader -> thread_started = false;

    free(reader);

    printf("[READER]: DESTROY FINISHED\n");
}

/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.c                       
    PURPOSE: implementation of reader module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "reader.h"
#include "../enums/enums.h"

#define PATH "/proc/stat"

static void* Reader_thread_function(void* args);

// READER STRUCTURE: holds reader object
struct reader {
    Buffer* buffer;
    long proc;
};

// THREADPARAMS STRUCTURE: holds params used by thread function
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
    Buffer* buffer,
    long proc
) {
    printf("[READER]: INIT STARTED\n");
    if (proc <= 0 || buffer == NULL) { return NULL; }
    
    Reader* reader = malloc(sizeof(Reader));
    
    if (reader == NULL) { 
        return NULL; 
    }

    *reader = (Reader) { 
        .buffer = buffer,
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
    Reader* reader,
    volatile sig_atomic_t* status
) {
    printf("[READER]: START STARTED\n");
    if (
        reader == NULL ||
        status == CREATED
    ) { return ERR_PARAMS; }

    ThreadParams* params = malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }
    
    pthread_t thread;

    *params = (ThreadParams) {
        .reader = reader,
        .status = status
    };

    pthread_create(&thread, NULL, Reader_thread_function, (void*)params);
    pthread_join(thread, NULL);

    free(params);
    printf("[READER]: START FINISHED\n");
    return SUCCESS;
}

/*
    METHOD: Reader_thread_function
    PURPOSE: acomplishing reader's thread work
    RETURN: NULL
*/
static void* Reader_thread_function(
    void* args
) {
    printf("[READER]: THREAD FUNCTION STARTED\n");
    ThreadParams* params = (ThreadParams*)args;

    struct timespec sleepTime;
    
    while (*(params -> status) == RUNNING) {
        ProcessorStats stats;
        if (Reader_read(&stats, params -> reader -> proc) != SUCCESS) {
            free(stats.cores);
            pthread_exit(NULL);
        }
        
        Buffer_push(params -> reader -> buffer, &stats);

        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        nanosleep(&sleepTime, NULL);
    }
    printf("[READER]: THREAD FUNTION FINISHED\n");
    pthread_exit(NULL);
}

/*
    METHOD: Reader_read
    PURPOSE: reads all required data from a saved file field
    RETURN: Stats 'object' including necessary data or null
*/
int Reader_read(
    ProcessorStats* processorStats,
    long proc
) {
    printf("[READER]: READ STARTED\n");
    FILE* file = fopen(PATH, "r");

    if (file == NULL) {
        return ERR_FILE_OPEN; 
    }

    char line[256];

    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return ERR_FILE_READ; 
    }
    printf("[LINE]: %s\n", line);
    sscanf(
        line, 
        "%s %d %d %d %d %d %d %d %d",
        processorStats->average.name,
        &(processorStats->average.user),
        &(processorStats->average.nice),
        &(processorStats->average.system),
        &(processorStats->average.idle),
        &(processorStats->average.iowait),
        &(processorStats->average.irq),
        &(processorStats->average.sortirq),
        &(processorStats->average.steal)
    );

    processorStats -> cores = (CoreStats*) malloc(sizeof(CoreStats) * (unsigned long)proc);

    if (processorStats -> cores == NULL) {
        fclose(file);
        return ERR_ALLOC; 
    }

    int coreCount = 0;

    while (coreCount < proc && fgets(line, sizeof(line), file) != NULL) {
        CoreStats* coreStats = &(processorStats -> cores[coreCount]);
        printf("[LINE]: %s\n", line);
        sscanf(
            line, 
            "%s %d %d %d %d %d %d %d %d",
            coreStats->name,
            &(coreStats->user),
            &(coreStats->nice),
            &(coreStats->system),
            &(coreStats->idle),
            &(coreStats->iowait),
            &(coreStats->irq),
            &(coreStats->sortirq),
            &(coreStats->steal)
        );

        coreCount++;
    }

    fclose(file);

    processorStats -> count = coreCount;
    printf("[READER]: READ FINISHED\n");
    return SUCCESS; 
}

/*
    METHOD: Reader_free
    PURPOSE: frees reserved memory for a given Reader 'object' and its nested 'objects'
    RETURN: Reader 'object' or NULL in 
        case creation was not possible 
*/
void Reader_free(
    Reader* const reader
) {
    printf("[READER]: FREE STARTED\n");
    if (reader == NULL) { return; }

    reader -> proc = 0;
    free(reader);
    printf("[READER]: FREE FINISHED\n");
}

/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.c                       
    PURPOSE: implementation of reader module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// INCLUDES OF INSIDE LIBRARIES
#include "reader.h"
#include "../enums/enums.h"
#include "../watchdog/watchdog.h"
#include "../notifier/notifier.h"
#include "../logger/logger.h"
#include "../stats/stats.h"

// MACRO DEFINITION
#define PATH "/proc/stat"

// PROTOTYPE FUNCTIONS FOR INSIDE WORLD
int Reader_read(ProcessorStats* const, const long);
static void* Reader_threadf(void* const);

// STRUCTURE FOR HOLDING READER OBJECT
struct reader {
    Watchdog* watchdog;
    Notifier* notifier;
    Buffer* buffer;
    pthread_t thread;
    long proc;
};

// STRUCTURE FOR HOLDING PARAMS PASSED TO READER THREAD FUNCTION
typedef struct ThreadParams {
    Reader* reader;
    volatile sig_atomic_t* status;
} ThreadParams;

/*
    METHOD: Reader_init
    ARGUMENTS:
        buffer - buffer object to work on
        proc - number of computer's cores
    PURPOSE: creation of Reader object
    RETURN: Reader object or NULL in case creation was not possible 
*/
Reader* Reader_init(
    Buffer* const buffer,
    const long proc
) {
    Watchdog* watchdog;
    Notifier* notifier;
    Reader* reader;

    // Logger_log("READER", "INIT STARTED");

    if (proc <= 0 || buffer == NULL) { return NULL; }
    
    reader = (Reader*) malloc(sizeof(Reader));
    
    if (reader == NULL) { return NULL; }

    notifier = Notifier_init();

    if (notifier == NULL) { return NULL; }

    watchdog = Watchdog_init(notifier, "READER");

    if (watchdog == NULL) { return NULL; }
    

    *reader = (Reader) { 
        .watchdog = watchdog,
        .notifier = notifier,
        .buffer = buffer,
        .proc = proc
    };

    // Logger_log("READER", "INIT FINISHED");

    return reader;
}

/*
    METHOD: Reader_start
    ARUGMENTS:
        reader - reader object to work on
        status - a pointer to tracker's status field
    PURPOSE: creation and start of a given object's thread
    RETURN: finish code meaning if function worked propely
*/
int Reader_start(
    Reader* const reader,
    volatile sig_atomic_t* status
) {
    ThreadParams* params;

    // Logger_log("READER", "START STARTED");

    if (
        reader == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .reader = reader,
        .status = status
    };

    if (pthread_create(&(reader -> thread), NULL, Reader_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    // Logger_log("READER", "START FINISHED");

    return SUCCESS;
}

/*
    METHOD: Reader_join
    ARUGMENTS:
        reader - reader object to work on
    PURPOSE: join of a given object's thread to its parent thread
    RETURN: enums integer value
*/
int Reader_join(
    Reader* const reader
) {
    // Logger_log("READER", "JOIN STARTED");

    if (reader == NULL) { return ERR_PARAMS; }
    if (pthread_join(reader -> thread, NULL) != 0) { return ERR_JOIN; }

    // Logger_log("READER", "JOIN FINISHED");

    return SUCCESS;
}

/*
    METHOD: Reader_threadf
    ARUGMENTS:
        args - a pointer to function's parameters
    PURPOSE: acomplishing reader's thread work
    RETURN: enums integer value
*/
static void* Reader_threadf(
    void* const args
) {
    ThreadParams* params;
    ProcessorStats stats;
    struct timespec timebreak;

    // Logger_log("READER", "THREAD FUNCTION STARTED");

    params = (ThreadParams*) args;

    // Logger_log("READER", "STARTING WATCHDOG THREAD");

    if (Watchdog_start(params -> reader -> watchdog, params -> status) != SUCCESS) {
        // Logger_log("READER", "COULD NOT START WATCHDOG THREAD");
        free(params);
        pthread_exit(NULL);
    }

    while (*(params -> status) == RUNNING) {
        if (Reader_read(&stats, params -> reader -> proc) != SUCCESS) {
            // Logger_log("READER", "READ FAILED");
            break;
        }
        
        if (Buffer_push(params -> reader -> buffer, &stats) != SUCCESS) {
            // Logger_log("READER", "PUSH FAILED");
            free(stats.cores);
            break;
        }

        if (Notifier_notify(params -> reader -> notifier) != SUCCESS) {
            // Logger_log("READER", "NOTIFY FAILED");
            free(stats.cores);
            break;
        }

        timebreak.tv_sec = 1;
        timebreak.tv_nsec = 0;

        nanosleep(&timebreak, NULL);
    }

    // Logger_log("READER", "JOINING WATCHDOG THREAD");

    Watchdog_join(params -> reader -> watchdog);

    free(params);

    // Logger_log("READER", "THREAD FUNCTION FINISHED");

    pthread_exit(NULL);
}

/*
    METHOD: Reader_read
    PURPOSE: reads all required data from a saved file field
    RETURN: Stats 'object' including necessary data or null
*/
/*
    METHOD: Reader_read
    ARUGMENTS:
        processorStats - object that data will be saved to
        proc - computer's core count
    PURPOSE: reads all required data from a saved file field
    RETURN: enums integer value
*/
int Reader_read(
    ProcessorStats* const processorStats,
    const long proc
) {
    FILE* file;
    int coreCount;

    // Logger_log("READER", "READ STARTED");

    if (processorStats == NULL || proc <= 0) { return ERR_PARAMS; }
    
    file = fopen(PATH, "r");

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

    coreCount = 0;

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

    // Logger_log("READER", "READ FINISHED");

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

    Watchdog_destroy(reader -> watchdog);
    Notifier_destroy(reader -> notifier);
    reader -> proc = 0;

    free(reader);

    printf("[READER]: DESTROY FINISHED\n");
}

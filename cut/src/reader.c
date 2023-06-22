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
#include <stdint.h>
#include <unistd.h>

// INCLUDES OF INSIDE LIBRARIES
#include "../inc/reader.h"
#include "../inc/enums.h"
#include "../inc/watchdog.h"
#include "../inc/notifier.h"
#include "../inc/logger.h"
#include "../inc/stats.h"

// MACRO DEFINITION
#define PATH "/proc/stat"

// PROTOTYPE FUNCTIONS FOR INSIDE WORLD
int Reader_read(ProcessorStats* const, const uint8_t);
static void* Reader_threadf(void* const);

// STRUCTURE FOR HOLDING READER OBJECT
struct reader {
    Watchdog* watchdog;
    Notifier* notifier;
    Buffer* buffer;
    pthread_t thread;
    uint8_t proc;
    char padding[7];
};

// STRUCTURE FOR HOLDING PARAMS PASSED TO READER THREAD FUNCTION
typedef struct ThreadParams {
    Reader* reader;
    volatile sig_atomic_t* status;
    atomic_flag* status_watch;
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
    const uint8_t proc
) {
    Watchdog* watchdog;
    Notifier* notifier;
    Reader* reader;

    Logger_log("READER", "INIT STARTED");

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

    Logger_log("READER", "INIT FINISHED");

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
    volatile sig_atomic_t* status,
    atomic_flag* status_watch
) {
    ThreadParams* params;

    Logger_log("READER", "START STARTED");

    if (
        reader == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .reader = reader,
        .status = status,
        .status_watch = status_watch
    };

    if (pthread_create(&(reader -> thread), NULL, Reader_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    Logger_log("READER", "START FINISHED");

    return OK;
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
    Logger_log("READER", "JOIN STARTED");

    if (reader == NULL) { return ERR_PARAMS; }
    if (pthread_join(reader -> thread, NULL) != 0) { return ERR_JOIN; }

    Logger_log("READER", "JOIN FINISHED");

    return OK;
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

    Logger_log("READER", "THREAD FUNCTION STARTED");

    params = (ThreadParams*) args;

    Logger_log("READER", "STARTING WATCHDOG THREAD");

    if (Watchdog_start(params -> reader -> watchdog, params -> status, params -> status_watch) != OK) {
        Logger_log("READER", "COULD NOT START WATCHDOG THREAD");
        free(params);
        pthread_exit(NULL);
    }

    while (*(params -> status) == RUNNING) {
        if (Reader_read(&stats, params -> reader -> proc) != OK) {
            Logger_log("READER", "READ FAILED");
            break;
        }
        
        if (Buffer_push(params -> reader -> buffer, &stats) != OK) {
            Logger_log("READER", "PUSH FAILED");
            free(stats.cores);
            break;
        }

        if (Notifier_notify(params -> reader -> notifier) != OK) {
            Logger_log("READER", "NOTIFY FAILED");
            free(stats.cores);
            break;
        }

        timebreak.tv_sec = 1;
        timebreak.tv_nsec = 0;

        nanosleep(&timebreak, NULL);
    }

    Logger_log("READER", "JOINING WATCHDOG THREAD");

    Watchdog_join(params -> reader -> watchdog);

    free(params);

    Logger_log("READER", "THREAD FUNCTION FINISHED");

    pthread_exit(NULL);
}

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
    const uint8_t proc
) {
    FILE* file;
    uint8_t coreCount;

    Logger_log("READER", "READ STARTED");

    if (processorStats == NULL || proc <= 0) { return ERR_PARAMS; }
    
    file = fopen(PATH, "r");

    if (file == NULL) {
        return ERR_FILE_OPEN; 
    }

    Logger_log("READER", "READLINE STARTED");

    if (fscanf(
            file, 
            "cpu %d %d %d %d %d %d %d %d %*d %*d\n",
            &(processorStats -> cores_average.user), 
            &(processorStats -> cores_average.nice), 
            &(processorStats -> cores_average.system),
            &(processorStats -> cores_average.idle), 
            &(processorStats -> cores_average.iowait), 
            &(processorStats -> cores_average.irq), 
            &(processorStats -> cores_average.sortirq),
            &(processorStats -> cores_average.steal)
        ) != 8
    ) {
        Logger_log("READER", "READLINE FAILED");
        return ERR_FILE_READ;
    }

    Logger_log("READER", "READLINE FINISHED");

    processorStats -> cores = (CoreStats*) malloc(sizeof(CoreStats) * proc);

    if (processorStats -> cores == NULL) {
        fclose(file);
        return ERR_ALLOC; 
    }

    coreCount = 0;

    while (coreCount < proc) {
        Logger_log("READER", "READLINE STARTED");

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
            Logger_log("READER", "READLINE FAILED");
            fclose(file);
            free(processorStats -> cores);
            return ERR_FILE_READ;
        }

        coreCount++;

        Logger_log("READER", "READLINE FINISHED");
    }

    fclose(file);

    processorStats -> count = coreCount;

    Logger_log("READER", "READ FINISHED");

    return OK; 
}

/*
    METHOD: Reader_destroy
    ARGUMENTS:
        reader - a Reader object to be freed
    PURPOSE: frees reserved memory for a given Reader 'object' and its nested 'objects'
    RETURN: Reader 'object' or NULL in 
        case creation was not possible 
*/
void Reader_destroy(
    Reader* const reader
) {
    Logger_log("READER", "DESTROY STARTED");

    if (reader == NULL) { return; }

    Watchdog_destroy(reader -> watchdog);
    Notifier_destroy(reader -> notifier);
    reader -> proc = 0;

    free(reader);

    Logger_log("READER", "DESTROY FINISHED");
}

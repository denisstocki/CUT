/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.c                       
    PURPOSE: implementation of analyzer module
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include "analyzer.h"
#include "../enums/enums.h"

static void* Analyzer_thread_function(void* args);

struct analyzer {
    long* cores_total_prev;
    long* cores_idle_prev;
    long cpu_total_prev;
    long cpu_idle_prev;
    bool prev_analyzed;
    Buffer* buffer;
};

// THREADPARAMS STRUCTURE: holds params used by thread function
typedef struct ThreadParams {
    Analyzer* analyzer;
    volatile sig_atomic_t* status;
} ThreadParams;

/*
    METHOD: Analyzer_init
    PURPOSE: creation of Analyzer 'object'
    RETURN: Analyzer 'object' or NULL in 
        case creation was not possible 
*/
Analyzer* Analyzer_init(
    Buffer* buffer
) {
    printf("[ANALYZER]: INIT STARTED\n");
    if (buffer == NULL) { return NULL; }
    
    Analyzer* analyzer = malloc(sizeof(Analyzer));

    if (analyzer == NULL) { return NULL; }
    
    *analyzer = (Analyzer) {
        .cores_total_prev = NULL,
        .cores_idle_prev = NULL,
        .cpu_total_prev = 0,
        .cpu_idle_prev = 0,
        .prev_analyzed = false,
        .buffer = buffer
    };
    printf("[ANALYZER]: INIT FINISHED\n");
    return analyzer;
}

/*
    METHOD: Analyzer_Start
    PURPOSE: creation and start of a given 
        object's thread
    RETURN: finish code meaning if function 
        worked propely
*/
int Analyzer_start(
    Analyzer* analyzer,
    volatile sig_atomic_t* status
) {
    printf("[ANALYZER]: START STARTED\n");
    if (
        analyzer == NULL ||
        status == CREATED
    ) { return ERR_PARAMS; }

    ThreadParams* params = malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }
    
    pthread_t thread;

    *params = (ThreadParams) {
        .analyzer = analyzer,
        .status = status
    };

    pthread_create(&thread, NULL, Analyzer_thread_function, (void*)params);
    pthread_join(thread, NULL);

    free(params);
    printf("[ANALYZER]: START FINISHED\n");
    return SUCCESS;
}

/*
    METHOD: Analyzer_thread_function
    PURPOSE: acomplishing reader's thread work
    RETURN: NULL
*/
static void* Analyzer_thread_function(
    void* args
) {
    printf("[ANALYZER]: THREAD FUNCTION STARTED\n");
    ThreadParams* params = (ThreadParams*)args;

    ProcessorStats stats;
    struct timespec sleepTime;
    
    while (*(params -> status) == RUNNING) {
        if (Buffer_pop(params -> analyzer -> buffer, &stats) != SUCCESS) {
            break;
        }
        
        if (Analyzer_analyze(params -> analyzer, &stats) != SUCCESS) {
            break;
        }
        
        free(stats.cores);
        
        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        nanosleep(&sleepTime, NULL);
    }
    printf("[ANALYZER]: THREAD FUNTION FINISHED\n");
    pthread_exit(NULL);
}

/*
    METHOD: Analyzer_analyze
    PURPOSE: counts percentage for each core in a given ProcessorStats
    RETURN: interger meaning if the operation successed or failed with an error
*/
int Analyzer_analyze(
    Analyzer* analyzer,
    ProcessorStats* processorStats
) {
    if (processorStats == NULL || analyzer == NULL) { return ERR_PARAMS; }
    
    CoreStats coreStats;

    coreStats = processorStats -> average;

}

/*
    METHOD: Analyzer_free
    PURPOSE: frees reserved memory for a given Analyzer 'object' and its nested 'objects'
    RETURN: Analyzer 'object' or NULL in 
        case creation was not possible 
*/
void Analyzer_free(
    Analyzer* analyzer
) {
    printf("[ANALYZER]: FREE STARTED\n");
    if (analyzer == NULL) { return; }
    
    analyzer -> cpu_idle_prev = 0;
    analyzer -> cpu_total_prev = 0;
    analyzer -> prev_analyzed = false;

    free(analyzer -> cores_idle_prev);
    free(analyzer -> cores_total_prev);
    free(analyzer);
    printf("[ANALYZER]: FREE FINISHED\n");
}

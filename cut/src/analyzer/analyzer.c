/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.c                       
    PURPOSE: implementation of analyzer module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "analyzer.h"
#include "../enums/enums.h"

// PROTOTYPE FUNCTIONS FOR INSIDE WORLD
static void* Analyzer_threadf(void* args);
static int Analyzer_analyze(Analyzer* analyzer, ProcessorStats*);

// STRUCTURE FOR HOLDING ANALYZER OBJECT
struct analyzer {
    Buffer* buffer;
    pthread_t thread;
    bool thread_started;
    bool prev_analyzed;
    char padding[6];
    long* cores_total_prev;
    long* cores_idle_prev;
    long cpu_total_prev;
    long cpu_idle_prev;
};

// STRUCTURE FOR HOLDING PARAMS PASSED TO READER THREAD FUNCTION
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
    Buffer* buffer,
    long proc
) {
    printf("[ANALYZER]: INIT STARTED\n");

    if (buffer == NULL || proc <= 0) { return NULL; }
    
    Analyzer* analyzer = (Analyzer*) malloc(sizeof(Analyzer));

    if (analyzer == NULL) { return NULL; }
    
    *analyzer = (Analyzer) {
        .buffer = buffer,
        .thread_started = false,
        .prev_analyzed = false,
        .cores_total_prev = NULL,
        .cores_idle_prev = NULL,
        .cpu_total_prev = 0,
        .cpu_idle_prev = 0
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
    Analyzer* const analyzer,
    volatile sig_atomic_t* status
) {
    printf("[ANALYZER]: START STARTED\n");

    if (
        analyzer == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    ThreadParams* params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .analyzer = analyzer,
        .status = status
    };

    if (pthread_create(&(analyzer -> thread), NULL, Analyzer_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    analyzer -> thread_started = true;

    printf("[ANALYZER]: START FINISHED\n");

    return SUCCESS;
}

int Analyzer_join(
    Analyzer* const analyzer
) {
    printf("[ANALYZER]: JOIN STARTED\n");

    if (analyzer == NULL) { return ERR_PARAMS; }
    if (analyzer -> thread_started == false) { return ERR_PARAMS; }
    if (pthread_join(analyzer -> thread, NULL) != 0) {
        return ERR_JOIN;
    }

    printf("[ANALYZER]: JOIN FINISHED\n");

    return SUCCESS;
}

/*
    METHOD: Analyzer_threadf
    PURPOSE: acomplishing reader's thread work
    RETURN: NULL
*/
static void* Analyzer_threadf(
    void* args
) {
    printf("[ANALYZER]: THREAD FUNCTION STARTED\n");

    ThreadParams* params = (ThreadParams*)args;
    ProcessorStats stats;
    struct timespec sleepTime;
    printf("STATUSIK: %d\n", *(params -> status));
    while (*(params -> status) == RUNNING) {
        printf("CO SIE DZIEJE CHUI\n");
        if (Buffer_pop(params -> analyzer -> buffer, &stats) != SUCCESS) {
            printf("NO ZJEBAloSIE\n");
            break;
        }
        
        if (Analyzer_analyze(params -> analyzer, &stats) != SUCCESS) {
            break;
        }
            printf("SIEMA2\n");
        
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
static int Analyzer_analyze(
    Analyzer* analyzer,
    ProcessorStats* processorStats
) {
    if (processorStats == NULL || analyzer == NULL) { return ERR_PARAMS; }
    
    printf("[SUCCESS]\n");

    return SUCCESS;
}

/*
    METHOD: Analyzer_free
    PURPOSE: frees reserved memory for a given Analyzer 'object' and its nested 'objects'
    RETURN: Analyzer 'object' or NULL in 
        case creation was not possible 
*/
void Analyzer_destroy(
    Analyzer* analyzer
) {
    printf("[ANALYZER]: DESTROY STARTED\n");

    if (analyzer == NULL) { return; }
    
    analyzer -> cpu_idle_prev = 0;
    analyzer -> cpu_total_prev = 0;
    analyzer -> prev_analyzed = false;

    free(analyzer);

    printf("[ANALYZER]: DESTROY FINISHED\n");
}

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
#include "../watchdog/watchdog.h"
#include "../notifier/notifier.h"
#include "../logger/logger.h"
#include "../stats/stats.h"

// PROTOTYPE FUNCTIONS FOR INSIDE WORLD
static void* Analyzer_threadf(void* args);
static int Analyzer_analyze(Analyzer* analyzer, ProcessorStats*, ConvertedStats*);
static float Analyzer_toPercent(CoreStats*, long*, long*);

// STRUCTURE FOR HOLDING ANALYZER OBJECT
struct analyzer {
    Watchdog* watchdog;
    Notifier* notifier;
    Buffer* bufferRA;
    Buffer* bufferAP;
    pthread_t thread;
    bool thread_started;
    bool prev_analyzed;
    char padding[6];
    long* cores_total_prev;
    long* cores_idle_prev;
    long cpu_total_prev;
    long cpu_idle_prev;
    long proc;
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
    Buffer* bufferRA,
    Buffer* bufferAP,
    long proc
) {
    Watchdog* watchdog;
    Notifier* notifier;
    Analyzer* analyzer;
    Logger_log("ANALYZER", "INIT STARTED");

    if (
        bufferRA == NULL || 
        bufferAP == NULL ||
        proc <= 0
    ) { 
        return NULL; 
    }
    
    analyzer = (Analyzer*) malloc(sizeof(Analyzer));

    if (analyzer == NULL) { return NULL; }

    notifier = Notifier_init();

    if (notifier == NULL) { return NULL; }

    watchdog = Watchdog_init(notifier, "ANALYZER");

    if (watchdog == NULL) { return NULL; }
    
    *analyzer = (Analyzer) {
        .watchdog = watchdog,
        .notifier = notifier,
        .bufferRA = bufferRA,
        .bufferAP = bufferAP,
        .thread_started = false,
        .prev_analyzed = false,
        .cores_total_prev = NULL,
        .cores_idle_prev = NULL,
        .cpu_total_prev = 0,
        .cpu_idle_prev = 0,
        .proc = proc
    };

    Logger_log("ANALYZER", "INIT FINISHED");

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
    ThreadParams* params;

    Logger_log("ANALYZER", "START STARTED");

    if (
        analyzer == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .analyzer = analyzer,
        .status = status
    };

    if (pthread_create(&(analyzer -> thread), NULL, Analyzer_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    analyzer -> thread_started = true;

    Logger_log("ANALYZER", "START FINISHED");

    return SUCCESS;
}

int Analyzer_join(
    Analyzer* const analyzer
) {
    Logger_log("ANALYZER", "JOIN STARTED");

    if (analyzer == NULL) { return ERR_PARAMS; }
    if (analyzer -> thread_started == false) { return ERR_PARAMS; }
    if (pthread_join(analyzer -> thread, NULL) != 0) {
        return ERR_JOIN;
    }

    Logger_log("ANALYZER", "JOIN FINISHED");

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
    ThreadParams* params;
    ProcessorStats* stats;
    ConvertedStats converted;
    struct timespec sleepTime;

    Logger_log("ANALYZER", "THREAD FUNCTION STARTED");

    params = (ThreadParams*)args;
    stats = malloc(sizeof(ProcessorStats) + sizeof(CoreStats) * (unsigned long) params -> analyzer -> proc);

    if (stats == NULL) {
        pthread_exit(NULL);
    } 

    Watchdog_start(params -> analyzer -> watchdog, params -> status);

    while (*(params -> status) == RUNNING) {
        if (Buffer_pop(params -> analyzer -> bufferRA, stats) != SUCCESS) {
            free(stats);
            break;
        }
        
        if (Analyzer_analyze(params -> analyzer, stats, &converted) == SUCCESS) {
            if (Buffer_push(params -> analyzer -> bufferAP, &converted) != SUCCESS) {
                free(stats -> cores);
                free(converted.percentages);
                break;
            }
        }

        Notifier_notify(params -> analyzer -> notifier);

        free(stats -> cores);
        
        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        nanosleep(&sleepTime, NULL);
    }

    Watchdog_join(params -> analyzer -> watchdog);

    Logger_log("ANALYZER", "THREAD FUNCTION FINISHED");

    free(stats);
    free(params);

    pthread_exit(NULL);
}

/*
    METHOD: Analyzer_analyze
    PURPOSE: counts percentage for each core in a given ProcessorStats
    RETURN: interger meaning if the operation successed or failed with an error
*/
static int Analyzer_analyze(
    Analyzer* analyzer,
    ProcessorStats* processorStats,
    ConvertedStats* convertedStats
) {
    long idle;
    long non_idle;
    Logger_log("ANALYZER", "ANALYZE STARTED");

    if (
        processorStats == NULL || 
        analyzer == NULL || 
        convertedStats == NULL
    ) { 
        return ERR_PARAMS; 
    }

    if (!analyzer -> prev_analyzed) {
        analyzer -> cores_total_prev = malloc(sizeof(long) * (unsigned long) analyzer -> proc);

        if (analyzer -> cores_total_prev == NULL) { return ERR_ALLOC; }
        
        analyzer -> cores_idle_prev = malloc(sizeof(long) * (unsigned long) analyzer -> proc);

        if (analyzer -> cores_idle_prev == NULL) {
            free(analyzer -> cores_total_prev);
            return ERR_ALLOC;
        }

        idle = processorStats -> average.idle 
            + processorStats -> average.iowait;

        non_idle = processorStats -> average.user 
            + processorStats -> average.nice 
            + processorStats -> average.system 
            + processorStats -> average.irq 
            + processorStats -> average.sortirq
            + processorStats -> average.steal;

        analyzer -> cpu_total_prev = idle + non_idle;
        analyzer -> cpu_idle_prev = idle;

        for(long i = 0; i < processorStats -> count; i++) {
            idle = processorStats -> cores[i].idle 
                + processorStats -> cores[i].iowait;

            non_idle = processorStats -> cores[i].user 
                + processorStats -> cores[i].nice 
                + processorStats -> cores[i].system 
                + processorStats -> cores[i].irq 
                + processorStats -> cores[i].sortirq 
                + processorStats -> cores[i].steal;
    
            analyzer -> cores_total_prev[i] = idle + non_idle;
            analyzer -> cores_idle_prev[i] = idle;
        }

        analyzer -> prev_analyzed = true;

        convertedStats -> percentages = NULL;
        convertedStats -> count = processorStats -> count;

        return ANALYZED;
    }
    
    convertedStats -> percentages = malloc(sizeof(float) * (unsigned long) processorStats -> count);
    
    if(convertedStats -> percentages == NULL) { return ERR_ALLOC; }

    convertedStats -> count = processorStats -> count;
    convertedStats -> average_percentage = Analyzer_toPercent(
        &processorStats -> average, 
        &analyzer -> cpu_total_prev, 
        &analyzer -> cpu_idle_prev
    );

    for(uint8_t i = 0; i < processorStats -> count; i++) {
        convertedStats -> percentages[i] = Analyzer_toPercent(
            &(processorStats -> cores[i]), 
            &(analyzer -> cores_total_prev[i]), 
            &(analyzer -> cores_idle_prev[i])
        );
    }

    Logger_log("ANALYZER", "ANALYZE FINISHED");

    return SUCCESS;
}

static float Analyzer_toPercent(
    CoreStats* stats, 
    long* total_prev, 
    long* idle_prev
) {
    long idle;
    long non_idle;
    long total;
    long idled;
    float percentage;
    Logger_log("ANALYZER", "TOPERCENT STARTED");
    idle = stats -> idle 
        + stats -> iowait;

    non_idle = stats -> user 
        + stats -> nice 
        + stats -> system 
        + stats -> irq 
        + stats -> sortirq 
        + stats -> steal;

    total = idle 
        + non_idle 
        - *total_prev;

    idled = idle 
        - *idle_prev;

    percentage = 0.0f;

    if(total != 0) { percentage = (float)(total - idled) / (float) total * 100.0f; }

    *total_prev = idle 
        + non_idle;

    *idle_prev = idle;

    Logger_log("ANALYZER", "TOPERCENT FINISHED");

    return percentage;
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
    Logger_log("ANALYZER", "DESTROY STARTED");

    if (analyzer == NULL) { return; }

    Watchdog_destroy(analyzer -> watchdog);
    Notifier_destroy(analyzer -> notifier);
    
    free(analyzer -> cores_total_prev);
    free(analyzer -> cores_idle_prev);

    free(analyzer);

    Logger_log("ANALYZER", "DESTROY FINISHED");
}

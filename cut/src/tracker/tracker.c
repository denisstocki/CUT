/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.c                       
    PURPOSE: implementation of tracker module
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <stdatomic.h>  
#include "tracker.h"
#include "../analyzer/analyzer.h"
#include "../printer/printer.h"
#include "../reader/reader.h"
#include "../logger/logger.h"
#include "../buffer/buffer.h"
#include "../enums/enums.h"

// STRUCTURE FOR HOLDING TRACKER OBJECT
struct tracker {
    Reader* reader;
    Buffer* bufferRA;
    Analyzer* analyzer;
    volatile sig_atomic_t status;
    char padding[4];
};

/*
    METHOD: Tracker_init
    PURPOSE: creation of Tracker 'object'
    RETURN: Tracker 'object' or NULL in 
        case creation was not possible 
*/
Tracker* Tracker_init(
    void
) {
    printf("[TRACKER]: INIT STARTED\n");

    Tracker* tracker = (Tracker*) malloc(sizeof(Tracker));

    if (tracker == NULL) { return NULL; }

    long proc = sysconf(_SC_NPROCESSORS_ONLN);
    
    if (proc <= 0) {
        free(tracker);
        return NULL;
    }

    Buffer* bufferRA = Buffer_init();

    if (bufferRA == NULL) { 
        free(tracker);
        return NULL; 
    }
    
    Reader* reader = Reader_init(bufferRA, proc);

    if (reader == NULL) {
        free(tracker);
        Buffer_free(bufferRA);
        return NULL;
    }

    Analyzer* analyzer = Analyzer_init(bufferRA, proc);

    if (analyzer == NULL) {
        free(tracker);
        Buffer_free(bufferRA);
        Reader_destroy(reader);
        return NULL;
    }

    *tracker = (Tracker) {
        .reader = reader,
        .bufferRA = bufferRA,
        .analyzer = analyzer,
        .status = ATOMIC_VAR_INIT(CREATED)
    };

    printf("[TRACKER]: INIT FINISHED\n");

    return tracker;
}

/*
    METHOD: Tracker_start
    PURPOSE: start of whole Tracker 'object''s thread work
    RETURN: nothing
*/
int Tracker_start(
    Tracker* tracker
) {
    printf("[TRACKER]: START STARTED\n");

    if (tracker == NULL) { return ERR_PARAMS; }
    if (tracker -> status != CREATED) { return ERR_PARAMS; }

    tracker -> status = RUNNING;

    if (Reader_start(tracker -> reader, &(tracker -> status)) != SUCCESS) {
        Tracker_destroy(tracker);
        return ERR_RUN;
    }
    // printf("[STATUS]: %d\n", tracker -> status);
    // if (Analyzer_start(tracker -> analyzer, &(tracker -> status)) != SUCCESS) {
    //     Tracker_destroy(tracker);
    //     return ERR_RUN;
    // }
    printf("[STATUS]: %d\n", tracker -> status);
    if (Reader_join(tracker -> reader)) {
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }
    // printf("[STATUS]: %d\n", tracker -> status);
    // if (Analyzer_join(tracker -> analyzer)) {
    //     Tracker_destroy(tracker);
    //     return ERR_JOIN;
    // }
    printf("[STATUS]: %d\n", tracker -> status);
    Tracker_destroy(tracker);

    printf("[TRACKER]: START FINISHED\n");

    return SUCCESS;
}

/*
    METHOD: Tracker_terminate
    PURPOSE: sets status on a given tracker to a TERMINATED status if possible
    RETURN: nothing
*/
int Tracker_terminate(
    Tracker* const tracker
) {
    printf("[TRACKER]: TERMINATE STARTED\n");

    if (tracker == NULL) { return ERR_PARAMS; }
    if (tracker -> status == TERMINATED) { return ERR_PARAMS; }

    Reader_destroy(tracker -> reader);

    tracker -> status = TERMINATED;

    printf("[TRACKER]: TERMINATE FINISHED\n");

    return SUCCESS;
}

/*
    METHOD: Tracker_destroy
    PURPOSE: frees reserved memory for a given Tracker 'object' and its nested 'objects'
    RETURN: nothing
*/
void Tracker_destroy(
    Tracker* tracker
) {
    printf("[TRACKER]: FREE STARTED\n");

    if (tracker == NULL) { return; }
    
    Buffer_free(tracker -> bufferRA);
    Reader_destroy(tracker -> reader);
    Analyzer_destroy(tracker -> analyzer);

    tracker -> status = 0;

    free(tracker);

    printf("[TRACKER]: FREE FINISHED\n");
}

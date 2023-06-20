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
    Buffer* bufferAP;
    Printer* printer;
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
    Tracker* tracker = (Tracker*) malloc(sizeof(Tracker));

    if(Logger_init() == ERR_CREATE) { return NULL; }

    if (tracker == NULL) { return NULL; }

    long proc = sysconf(_SC_NPROCESSORS_ONLN);
    
    if (proc <= 0) {
        free(tracker);
        Logger_destroy();
        return NULL;
    }

    Buffer* bufferRA = Buffer_init(sizeof(ProcessorStats) + sizeof(CoreStats) * (unsigned long) proc, 32);

    if (bufferRA == NULL) { 
        free(tracker);
        Logger_destroy();
        return NULL; 
    }
    
    Reader* reader = Reader_init(bufferRA, proc);

    if (reader == NULL) {
        free(tracker);
        Buffer_free(bufferRA);
        Logger_destroy();
        return NULL;
    }

    Buffer* bufferAP = Buffer_init(sizeof(ConvertedStats) + sizeof(float) * (unsigned long) proc, 32);

    if (bufferAP == NULL) { 
        free(bufferRA);
        free(reader);
        free(tracker);
        Logger_destroy();
        return NULL; 
    }

    Analyzer* analyzer = Analyzer_init(bufferRA, bufferAP, proc);

    if (analyzer == NULL) {
        free(tracker);
        Buffer_free(bufferRA);
        Buffer_free(bufferAP);
        Reader_destroy(reader);
        Logger_destroy();
        return NULL;
    }

    Printer* printer = Printer_init(bufferAP, proc);

    if (printer == NULL) {
        free(tracker);
        Buffer_free(bufferRA);
        Buffer_free(bufferAP);
        Reader_destroy(reader);
        Analyzer_destroy(analyzer);
        Logger_destroy();
        return NULL;
    }
    
    *tracker = (Tracker) {
        .reader = reader,
        .bufferRA = bufferRA,
        .analyzer = analyzer,
        .bufferAP = bufferAP,
        .printer = printer,
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
    if (tracker == NULL) { return ERR_PARAMS; }
    if (tracker -> status != CREATED) { return ERR_PARAMS; }

    tracker -> status = RUNNING;

    if (Logger_start(&(tracker -> status)) != SUCCESS) {
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    if (Reader_start(tracker -> reader, &(tracker -> status)) != SUCCESS) {
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    if (Analyzer_start(tracker -> analyzer, &(tracker -> status)) != SUCCESS) {
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    if (Printer_start(tracker -> printer, &(tracker -> status)) != SUCCESS) {
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    if (Reader_join(tracker -> reader)) {
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

    if (Analyzer_join(tracker -> analyzer)) {
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

    if (Printer_join(tracker -> printer)) {
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

    if (Logger_join()) {
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

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

    ProcessorStats trasher1;
    ConvertedStats trasher2;
    
    while(!Buffer_isEmpty(tracker -> bufferRA)) {
        Buffer_pop(tracker -> bufferRA, &trasher1);
        free(trasher1.cores);
    }

    while(!Buffer_isEmpty(tracker -> bufferAP)) {
        Buffer_pop(tracker -> bufferAP, &trasher2);
        free(trasher2.percentages);
    }
    
    Reader_destroy(tracker -> reader);
    Analyzer_destroy(tracker -> analyzer);
    Printer_destroy(tracker -> printer);
    Buffer_free(tracker -> bufferRA);
    Buffer_free(tracker -> bufferAP);

    tracker -> status = 0;

    free(tracker);

    printf("[TRACKER]: FREE FINISHED\n");

    Logger_destroy();
}

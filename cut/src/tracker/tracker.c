/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.c                       
    PURPOSE: implementation of tracker module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdatomic.h>  
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

// INCLUDES OF INSIDE LIBRARIES
#include "../analyzer/analyzer.h"
#include "../printer/printer.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"
#include "../reader/reader.h"
#include "../enums/enums.h"
#include "../stats/stats.h"
#include "tracker.h"

// STRUCTURE FOR HOLDING TRACKER OBJECT
struct tracker {
    Buffer* bufferRA;
    Buffer* bufferAP;
    Reader* reader;
    Analyzer* analyzer;
    Printer* printer;
    volatile sig_atomic_t status;
    char padding[4];
};

/*
    METHOD: Tracker_init
    ARGUMENTS: none
    PURPOSE: creation of Tracker object
    RETURN: Tracker object or NULL in 
        case creation was not possible 
*/
Tracker* Tracker_init(
    void
) {
    Tracker* tracker;
    Buffer* bufferRA;
    Buffer* bufferAP;
    Reader* reader;
    Analyzer* analyzer;
    Printer* printer;
    long proc;

    Logger_log("TRACKER", "INIT STARTED");

    tracker = (Tracker*) malloc(sizeof(Tracker));

    if (tracker == NULL) { return NULL; }

    proc = sysconf(_SC_NPROCESSORS_ONLN);
    if (proc <= 0) { goto err_proc_load; }

    bufferRA = Buffer_init(sizeof(ProcessorStats) + sizeof(CoreStats) * (unsigned long) proc, 32);
    if (bufferRA == NULL) { goto err_proc_load; }
    
    reader = Reader_init(bufferRA, proc);
    if (reader == NULL) { goto err_reader_init; }

    bufferAP = Buffer_init(sizeof(ConvertedStats) + sizeof(float) * (unsigned long) proc, 32);
    if (bufferAP == NULL) { goto err_bufferAP_init; }

    analyzer = Analyzer_init(bufferRA, bufferAP, proc);
    if (analyzer == NULL) { goto err_analyzer_init; }

    printer = Printer_init(bufferAP, proc);
    if (printer == NULL) { goto err_printer_init; }
    
    *tracker = (Tracker) {
        .reader = reader,
        .bufferRA = bufferRA,
        .analyzer = analyzer,
        .bufferAP = bufferAP,
        .printer = printer,
        .status = ATOMIC_VAR_INIT(CREATED)
    };

    Logger_log("TRACKER", "INIT FINISHED");

    return tracker;

    err_printer_init:
        Analyzer_destroy(analyzer);
    err_analyzer_init:
        Buffer_destroy(bufferAP);
    err_bufferAP_init:
        Reader_destroy(reader);
    err_reader_init:
        Buffer_destroy(bufferRA);
    err_proc_load:
        free(tracker);

    printf("[TRACKER]: INIT MEMORY ALLOC ERROR\n");

    return NULL;
}

/*
    METHOD: Tracker_start
    ARGUMENTS:
        tracker - reference to an object which Tracker_start function is going to work on
    PURPOSE: start of whole Tracker object's thread work
    RETURN: enums integer value
*/
int Tracker_start(
    Tracker* const tracker
) {
    if (tracker == NULL) { return ERR_PARAMS; }
    if (tracker -> status != CREATED) { return ERR_PARAMS; }

    Logger_log("TRACKER", "START STARTING");

    tracker -> status = RUNNING;

    Logger_log("TRACKER", "STARTING READER");

    if (Reader_start(tracker -> reader, &(tracker -> status)) != SUCCESS) {
        Logger_log("TRACKER", "ERROR WHEN STARTING READER");
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    Logger_log("TRACKER", "STARTING ANALYZER");

    if (Analyzer_start(tracker -> analyzer, &(tracker -> status)) != SUCCESS) {
        Logger_log("TRACKER", "ERROR WHEN STARTING ANALYZER");
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    Logger_log("TRACKER", "STARTING PRINTER");

    if (Printer_start(tracker -> printer, &(tracker -> status)) != SUCCESS) {
        Logger_log("TRACKER", "ERROR WHEN STARTING PRINTER");
        Tracker_destroy(tracker);
        return ERR_RUN;
    }

    Logger_log("TRACKER", "JOINING READER");

    if (Reader_join(tracker -> reader) != SUCCESS) {
        Logger_log("TRACKER", "ERROR WHEN JOINING READER");
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

    Logger_log("TRACKER", "JOINING ANALYZER");

    if (Analyzer_join(tracker -> analyzer) != SUCCESS) {
        Logger_log("TRACKER", "ERROR WHEN JOINING ANALYZER");
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

    Logger_log("TRACKER", "JOINING PRINTER");

    if (Printer_join(tracker -> printer) != SUCCESS) {
        Logger_log("TRACKER", "ERROR WHEN JOINING PRINTER");
        Tracker_destroy(tracker);
        return ERR_JOIN;
    }

    Logger_log("TRACKER", "START FINISHED");

    return SUCCESS;
}

/*
    METHOD: Tracker_terminate
    ARGUMENTS: 
        tracker - reference to an object which Tracker_terminate function is going to work on
    PURPOSE: sets status on a given tracker to TERMINATED if possible
    RETURN: nothing
*/
int Tracker_terminate(
    Tracker* const tracker
) {
    Logger_log("TRACKER", "TERMINATE STARTED");

    if (tracker == NULL) { return ERR_PARAMS; }
    if (tracker -> status == TERMINATED) { return ERR_PARAMS; }

    tracker -> status = TERMINATED;

    Logger_log("TRACKER", "TERMINATE FINISHED");

    return SUCCESS;
}

/*
    METHOD: Tracker_destroy
    ARGUMENTS: 
        tracker - reference to an object which Tracker_destroy function is going to work on
    PURPOSE: frees reserved memory for a given Tracker object and its nested objects
    RETURN: nothing
*/
void Tracker_destroy(
    Tracker* const tracker
) {
    ProcessorStats popRA;
    ConvertedStats popAP;

    Logger_log("TRACKER", "DESTROY STARTED");

    if (tracker == NULL) { return; }
    
    Logger_log("TRACKER", "POPPING BUFFER R-A ELEMENTS");

    while(!Buffer_isEmpty(tracker -> bufferRA)) {
        Buffer_pop(tracker -> bufferRA, &popRA);
        free(popRA.cores);
    }

    Logger_log("TRACKER", "POPPING BUFFER A-P ELEMENTS");

    while(!Buffer_isEmpty(tracker -> bufferAP)) {
        Buffer_pop(tracker -> bufferAP, &popAP);
        free(popAP.percentages);
    }
    
    Reader_destroy(tracker -> reader);
    Analyzer_destroy(tracker -> analyzer);
    Printer_destroy(tracker -> printer);
    Buffer_destroy(tracker -> bufferRA);
    Buffer_destroy(tracker -> bufferAP);

    free(tracker);

    Logger_log("TRACKER", "DESTROY FINISHED");
}

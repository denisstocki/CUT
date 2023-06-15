/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.c                       
    PURPOSE: implementation of tracker module
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>  
#include "tracker.h"
#include "../analyzer/analyzer.h"
#include "../printer/printer.h"
#include "../reader/reader.h"
#include "../logger/logger.h"

#define BASENAME "TRACKER"

long get_proc(void);

struct tracker {
    Printer* printer;
    Reader* reader;
    Analyzer* analyzer;
    Logger* logger;
    long proc;
    volatile sig_atomic_t status;
    char padding[4];
};

/*
    METHOD: Tracker_init
    PURPOSE: creation of Tracker 'object'
    RETURN: Tracker 'object' or NULL in 
        case creation was not possible 
*/
long get_proc(
    void
) {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/*
    METHOD: Tracker_get_proc
    PURPOSE: getter for a given Tracker 'object''s proc field
    RETURN: long value in a given Tracker 'object''s proc field
*/
long Tracker_get_proc(
    Tracker* tracker
) {
    return tracker -> proc;
}


/*
    METHOD: Tracker_init
    PURPOSE: creation of Tracker 'object'
    RETURN: Tracker 'object' or NULL in 
        case creation was not possible 
*/
Tracker* Tracker_init(
    char* path
) {
    Tracker* tracker = (Tracker*) malloc(sizeof(Tracker));
    tracker -> logger = Logger_init(path);
    tracker -> analyzer = Analyzer_init(tracker -> logger);
    tracker -> reader = Reader_init(get_proc());
    tracker -> printer = Printer_init(tracker -> logger);
    tracker -> proc = get_proc();
    tracker -> status = ATOMIC_VAR_INIT(CREATED);

    if (tracker -> proc == -1) {
        Analyzer_free(tracker -> analyzer);
        tracker -> proc = 0;
        return NULL;
    }

    return tracker;
}

/*
    METHOD: Tracker_start
    PURPOSE: start of whole Tracker 'object''s thread work
    RETURN: nothing
*/
void Tracker_start(
    Tracker* tracker
) {
    tracker -> status = RUNNING;
    while (tracker -> status == RUNNING) {
        
    }
    
}

/*
    METHOD: Tracker_free
    PURPOSE: frees reserved memory for a given Tracker 'object' and its nested 'objects'
    RETURN: nothing
*/
void Tracker_free(
    Tracker* tracker
) {
    Analyzer_free(tracker -> analyzer);
    Reader_free(tracker -> reader);
    Printer_free(tracker -> printer);
    Logger_free(tracker -> logger);
    tracker -> proc = 0;
    free(tracker);
}

/*
    METHOD: Tracker_set_status
    PURPOSE: sets status on a given tracker to a given status if possible
    RETURN: 1 in case the change was made properly, else -1 
*/
int Tracker_set_status(
    Tracker* tracker,
    int status
) {
    printf("[MAIN]: SIGNAL HANDLER - TRACKER STATUS CHANGED\n");
    Logger_log(tracker -> logger, BASENAME, "TRYING TO CHANGE STATUS");
    switch (status) {
        case RUNNING:
            if (tracker -> status == CREATED){
                tracker -> status = RUNNING;
                return 1;
            } else return -1;

        case TERMINATED:
            if (tracker -> status == RUNNING){
                tracker -> status = TERMINATED;
                return 1;
            } else return -1;
    
        default:
            return -1;
    }
}

/*
    METHOD: Tracker_get_status
    PURPOSE: returns status of a given tracker
    RETURN: tracker's status field, else -1
*/
int Tracker_get_status(
    Tracker* tracker
) {
    if (tracker != NULL) { return tracker -> status; }
    else return -1;
}

/*
    METHOD: Tracker_get_logger
    PURPOSE: returns logger 'object' of a given tracker
    RETURN: logger's object, else null
*/
Logger* Tracker_get_logger(
    Tracker* tracker
) {
    if (tracker -> logger != NULL) {
        return tracker -> logger;
    } else {
        return NULL;
    }
}

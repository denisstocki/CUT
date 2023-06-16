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

struct tracker {
    Reader* reader;
    Buffer* buffer_reader;
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

    Buffer* buffer_reader = Buffer_init();

    if (buffer_reader == NULL) { 
        free(tracker);
        return NULL; 
    }
    
    Reader* reader = Reader_init(buffer_reader, proc);

    if (reader == NULL) {
        free(tracker);
        Buffer_free(buffer_reader);
        return NULL;
    }

    Analyzer* analyzer = Analyzer_init(buffer_reader);

    if (analyzer == NULL) {
        free(tracker);
        Buffer_free(buffer_reader);
        Reader_free(reader);
        return NULL;
    }

    *tracker = (Tracker) {
        .reader = reader,
        .buffer_reader = buffer_reader,
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
void Tracker_start(
    Tracker* tracker
) {
    printf("[TRACKER]: START STARTED\n");
    if (tracker == NULL) { return; }

    tracker -> status = RUNNING;

    Reader_start(tracker -> reader, &(tracker -> status));
    Analyzer_start(tracker -> analyzer, &(tracker -> status));

    Tracker_free(tracker);
    printf("[TRACKER]: START FINISHED\n");
}

/*
    METHOD: Tracker_free
    PURPOSE: frees reserved memory for a given Tracker 'object' and its nested 'objects'
    RETURN: nothing
*/
void Tracker_free(
    Tracker* tracker
) {
    printf("[TRACKER]: FREE STARTED\n");
    if (tracker == NULL) { return; }
    
    Buffer_free(tracker -> buffer_reader);
    Reader_free(tracker -> reader);
    Analyzer_free(tracker -> analyzer);
    tracker -> status = 0;

    free(tracker);
    printf("[TRACKER]: FREE FINISHED\n");
}

/*
    METHOD: Tracker_terminate
    PURPOSE: sets status on a given tracker to a TERMINATED status if possible
    RETURN: nothing
*/
void Tracker_terminate(
    Tracker* tracker
) {
    printf("[TRACKER]: TERMINATE STARTED\n");
    if (tracker == NULL) { return; }
    if (tracker -> status == TERMINATED) { return; }

    tracker -> status = TERMINATED;
    printf("[TRACKER]: TERMINATE FINISHED\n");
}

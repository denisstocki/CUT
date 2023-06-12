/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.c                       
    PURPOSE: implementation of tracker module
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tracker.h"
#include "../analyzer/analyzer.h"

long get_proc(void);

struct tracker {
    Analyzer* analyzer;
    long proc;
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
    void
) {
    Tracker* tracker = (Tracker*) malloc(sizeof(Tracker*));
    tracker -> analyzer = Analyzer_init();
    tracker -> proc = get_proc();

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
    (void) tracker;
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
    tracker -> proc = 0;
    free(tracker);
}

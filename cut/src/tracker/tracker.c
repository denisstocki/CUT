/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.c                       
    PURPOSE: implementation of tracker module
*/

#include <stdio.h>
#include <stdlib.h>
#include "tracker.h"
#include "../analyzer/analyzer.h"

struct tracker {
    Analyzer* analyzer;
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
    Tracker* tracker = (Tracker*) malloc(sizeof(Tracker*));
    tracker -> analyzer = Analyzer_init();

    return tracker;
}

/*
    METHOD: Tracker_start
    PURPOSE: start of whole Tracker 'object''s thread work
    RETURN: nothing
*/
void Tracker_start(
    void
) {

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
    free(tracker);
}

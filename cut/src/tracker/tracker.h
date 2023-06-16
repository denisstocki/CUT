/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.h                       
    PURPOSE: interface for tracker module 
*/

#ifndef TRACKER_H
#define TRACKER_H

#include "../logger/logger.h"

// Usage of pseudo objective 
// encapsulation on struct tracker.
typedef struct tracker Tracker;

enum {
    CREATED,
    RUNNING, 
    TERMINATED
};

static const char* const TrackerStatusNames[] = {
    "CREATED",
    "RUNNING",
    "TERMINATED"
};

Tracker* Tracker_init(void);
void Tracker_start(Tracker* tracker);
void Tracker_free(Tracker* tracker);
long Tracker_get_proc(Tracker* tracker);
void Tracker_terminate(Tracker* tracker);

#endif 

/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.h                       
    PURPOSE: interface for tracker module 
*/

#ifndef TRACKER_H
#define TRACKER_H

// Usage of pseudo objective 
// encapsulation on struct tracker.
typedef struct tracker Tracker;

Tracker* Tracker_init(void);
void Tracker_start(Tracker* tracker);
void Tracker_free(Tracker* tracker);
long Tracker_get_proc(Tracker* tracker);

#endif 

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
void Tracker_start(void);
void Tracker_free(Tracker* tracker);

#endif 

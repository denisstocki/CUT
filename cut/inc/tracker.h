/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.h                       
    PURPOSE: interface for tracker module 
*/

#ifndef TRACKER_H
#define TRACKER_H

// ENCAPSULATION ON TRACKER OBJECT
typedef struct tracker Tracker;

// DECLARATIONS OF OUTSIDE PROTOTYPES
Tracker* Tracker_init(void);
int Tracker_start(Tracker* const);
int Tracker_terminate(Tracker* const);
void Tracker_destroy(Tracker* const);

#endif 

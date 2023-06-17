/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.h                       
    PURPOSE: interface for tracker module 
*/

#ifndef TRACKER_H
#define TRACKER_H

// INSIDE LIBRARIES
#include "../logger/logger.h"

// ENCAPSULATION ON TRACKER OBJECT
typedef struct tracker Tracker;

// PROTOTYPE FUNCTIONS FOR OUTSIDE WORLD
Tracker* Tracker_init(void);
int Tracker_start(Tracker* const tracker);
int Tracker_terminate(Tracker* const tracker);
void Tracker_destroy(Tracker* const tracker);

#endif 

/*
    AUTHOR: DENIS STOCKI                  
    FILE: stats.h                       
    PURPOSE: structure holder for stats module 
*/

#ifndef STATS_H
#define STATS_H

#include <stdint.h>

typedef struct CoreStats {
    char* name;
    int user;
    int nice;
    int system;
    int idle;
    int iowait;
    int irq;
    int sortirq;
    int steal;
} CoreStats;

typedef struct ProcessorStats {
    CoreStats average;
    CoreStats* cores;
    int count;
} ProcessorStats;

#endif 

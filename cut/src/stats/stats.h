/*
    AUTHOR: DENIS STOCKI                  
    FILE: stats.h                       
    PURPOSE: structure holder for stats module 
*/

#ifndef STATS_H
#define STATS_H

#include <stdint.h>

typedef struct CoreStats {
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
    char padding[4];
} ProcessorStats;

typedef struct ConvertedStats {
    float* percentages;
    float average_percentage;
    int count;
} ConvertedStats;

#endif 

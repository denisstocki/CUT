/*
    AUTHOR: DENIS STOCKI                  
    FILE: stats.h                       
    PURPOSE: structure holder for stats module 
*/

#ifndef STATS_H
#define STATS_H

// STRUCTURE FOR HOLDING CORESTATS
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

// STRUCTURE FOR HOLDING PROCESSORSTATS
typedef struct ProcessorStats {
    CoreStats* cores;
    CoreStats average;
    int count;
    char padding[4];
} ProcessorStats;

// STRUCTURE FOR HOLDING CONVERTEDSTATS
typedef struct ConvertedStats {
    float* percentages;
    float average_percentage;
    int count;
} ConvertedStats;

#endif 

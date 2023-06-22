/*
    AUTHOR: DENIS STOCKI                  
    FILE: stats.h                       
    PURPOSE: structure holder for stats module 
*/

#ifndef STATS_H
#define STATS_H

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdint.h>

// STRUCTURE FOR HOLDING CORESTATS
typedef struct CoreStats {
    uint32_t user;
    uint32_t nice;
    uint32_t system;
    uint32_t idle;
    uint32_t iowait;
    uint32_t irq;
    uint32_t sortirq;
    uint32_t steal;
} CoreStats;

// STRUCTURE FOR HOLDING PROCESSORSTATS
typedef struct ProcessorStats {
    CoreStats* cores;
    CoreStats cores_average;
    uint8_t count;
    char padding[7];
} ProcessorStats;

// STRUCTURE FOR HOLDING CONVERTEDSTATS
typedef struct ConvertedStats {
    float* percentages;
    float percentages_average;
    uint8_t count;
    char padding[3];
} ConvertedStats;

#endif 

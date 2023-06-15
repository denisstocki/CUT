/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.h                       
    PURPOSE: interface for analyzer module 
*/

#ifndef ANALYZER_H
#define ANALYZER_H

#include "../logger/logger.h"

// Usage of pseudo objective 
// encapsulation on struct analyzer.
typedef struct analyzer Analyzer;

Analyzer* Analyzer_init(Logger* logger);
void Analyzer_free(Analyzer* analyzer);

#endif 

/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.h                       
    PURPOSE: interface for analyzer module 
*/

#ifndef ANALYZER_H
#define ANALYZER_H

#include "../buffer/buffer.h"

// Usage of pseudo objective 
// encapsulation on struct analyzer.
typedef struct analyzer Analyzer;

Analyzer* Analyzer_init(Buffer* buffer);
int Analyzer_start(Analyzer*, volatile sig_atomic_t*);
void Analyzer_free(Analyzer* analyzer);

#endif 

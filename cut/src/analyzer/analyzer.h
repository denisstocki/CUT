/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.h                       
    PURPOSE: interface for analyzer module 
*/

#ifndef ANALYZER_H
#define ANALYZER_H

// OUTSIDE LIBRARIES
#include <signal.h>

// INSIDE LIBRARIES
#include "../buffer/buffer.h"

// ENCAPSULATION ON READER OBJECT
typedef struct analyzer Analyzer;

// PROTOTYPE FUNCTIONS FOR OUTSIDE WORLD
Analyzer* Analyzer_init(Buffer* const, Buffer* const, const long);
int Analyzer_start(Analyzer* const, volatile sig_atomic_t*);
int Analyzer_join(Analyzer* const);
void Analyzer_destroy(Analyzer* const);

#endif 

/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.h                       
    PURPOSE: interface for analyzer module 
*/

#ifndef ANALYZER_H
#define ANALYZER_H

// INCLUDES OF OUTSIDE LIBRARIES
#include <signal.h>
#include <stdint.h>
#include <stdatomic.h>

// INCLUDES OF INSIDE LIBRARIES
#include "buffer.h"

// ENCAPSULATION ON READER OBJECT
typedef struct analyzer Analyzer;

// PROTOTYPE FUNCTIONS FOR OUTSIDE WORLD
Analyzer* Analyzer_init(Buffer* const, Buffer* const, uint8_t const);
int Analyzer_start(Analyzer* const, volatile sig_atomic_t*, atomic_flag*);
int Analyzer_join(Analyzer* const);
void Analyzer_destroy(Analyzer* const);

#endif 

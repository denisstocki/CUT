/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.h                       
    PURPOSE: interface for reader module 
*/

#ifndef READER_H
#define READER_H

// OUTSIDE LIBRARIES
#include <signal.h>
#include <stdatomic.h>

// INSIDE LIBRARIES
#include "buffer.h"

// ENCAPSULATION ON READER OBJECT
typedef struct reader Reader;

// PROTOTYPE FUNCTIONS FOR OUTSIDE WORLD
Reader* Reader_init(Buffer* const, const uint8_t); 
int Reader_start(Reader* const, volatile sig_atomic_t*, atomic_flag*); 
int Reader_join(Reader* const);
void Reader_destroy(Reader*);

#endif 

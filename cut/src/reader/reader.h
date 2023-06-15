/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.h                       
    PURPOSE: interface for reader module 
*/

#ifndef READER_H
#define READER_H

#include "../stats/stats.h"

// Usage of pseudo objective 
// encapsulation on struct reader.
typedef struct reader Reader;

Reader* Reader_init(const long);
ProcessorStats* Reader_read(Reader* const);
void Reader_free(Reader* const);

#endif 

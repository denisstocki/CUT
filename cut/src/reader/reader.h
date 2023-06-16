/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.h                       
    PURPOSE: interface for reader module 
*/

#ifndef READER_H
#define READER_H

#include "../stats/stats.h"
#include "../buffer/buffer.h"

// Usage of pseudo objective 
// encapsulation on struct reader.
typedef struct reader Reader;

Reader* Reader_init(Buffer*, long); //ready
int Reader_start(Reader*, volatile sig_atomic_t*); //ready
int Reader_read(ProcessorStats*, long); //ready
void Reader_free(Reader* const);

#endif 

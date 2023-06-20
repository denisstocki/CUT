/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.h                       
    PURPOSE: interface for logger module 
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>    
#include <stdint.h>     
#include <stddef.h>     
#include "../stats/stats.h"

typedef struct buffer Buffer;

Buffer* Buffer_init(size_t size, size_t capacity);
bool Buffer_isEmpty(Buffer*);
bool Buffer_isFull(Buffer*);
int Buffer_push(Buffer*, void*);
int Buffer_pop(Buffer*, void*);
void Buffer_free(Buffer*);

#endif 

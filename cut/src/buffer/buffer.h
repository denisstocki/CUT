/*
    AUTHOR: DENIS STOCKI                  
    FILE: buffer.h                       
    PURPOSE: interface for buffer module 
*/

#ifndef BUFFER_H
#define BUFFER_H

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdbool.h>    
#include <stdint.h>     
#include <stddef.h>     

// ENCAPSULATION ON BUFFER OBJECT
typedef struct buffer Buffer;

// DECLARATIONS OF OUTSIDE PROTOTYPES
Buffer* Buffer_init(size_t const, size_t const);
bool Buffer_isEmpty(Buffer* const);
bool Buffer_isFull(Buffer* const);
int Buffer_push(Buffer* const, void* const);
int Buffer_pop(Buffer* const, void*);
void Buffer_destroy(Buffer*);

#endif 

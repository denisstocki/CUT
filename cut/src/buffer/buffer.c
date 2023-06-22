/*
    AUTHOR: DENIS STOCKI                  
    FILE: buffer.c                       
    PURPOSE: implementation of buffer module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <pthread.h>    
#include <stdlib.h>     
#include <string.h>     
#include <time.h>       
#include <sys/time.h>   

// INCLUDES OF INSIDE LIBRARIES
#include "../buffer/buffer.h"
#include "../enums/enums.h"
#include "../logger/logger.h"

// STRUCTURE FOR HOLDING BUFFER OBJECT
struct buffer {
    pthread_cond_t can_produce;
    pthread_cond_t can_consume;
    pthread_mutex_t mutex;
    size_t tail;
    size_t head;
    size_t count;
    size_t capacity;
    size_t size;
    uint8_t elements[];   
};

/*
    METHOD: Buffer_init
    ARGUMENTS:
        size - size of a single element in an elements array
        capacity - max count of elements in elements array
    PURPOSE: creation of Buffer object
    RETURN: Buffer object or NULL in 
        case creation was not possible 
*/
Buffer* Buffer_init(
    size_t const size,
    size_t const capacity
) {
    Buffer* buffer;

    if (size <= 0 || capacity <= 0) { return NULL; }
    
    buffer = (Buffer*) malloc(sizeof(Buffer) + (size * capacity));

    if (buffer == NULL) { return NULL; }

    *buffer = (Buffer) {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .can_consume = PTHREAD_COND_INITIALIZER,
        .can_produce = PTHREAD_COND_INITIALIZER,
        .tail = 0,
        .head = 0,
        .count = 0,
        .capacity = capacity,
        .size = size
    };

    return buffer;
}

/*
    METHOD: Buffer_isEmpty
    ARGUMENTS:
        buffer - an object where emptiness will be checked
    PURPOSE: check if a given Buffer object is empty
    RETURN: true in case it is empty, else false
*/
bool Buffer_isEmpty(
    Buffer* const buffer
) {
    if (buffer == NULL) { return false; }

    if (buffer -> count == 0) { return true; }
    else { return false; }
}

/*
    METHOD: Buffer_isFull
    ARGUMENTS:
        buffer - an object where fullness will be checked
    PURPOSE: check if a given Buffer object is full
    RETURN: true in case it is full, else false
*/
bool Buffer_isFull(
    Buffer* const buffer
) {
    if (buffer == NULL) { return false; }

    if (buffer -> count == buffer -> capacity) { return true; }
    else { return false; }
}

/*
    METHOD: Buffer_push
    ARGUMENTS:
        buffer - an object where an element will be pushed
        element - an object to be pushed into a given buffer
    PURPOSE: push of an element into buffer
    RETURN: enums integer value
*/
int Buffer_push(
    Buffer* const buffer, 
    void* const element
) {
    if (buffer == NULL || element == NULL) { return ERR_PARAMS; }

    pthread_mutex_lock(&buffer->mutex);

    if (Buffer_isFull(buffer)) {
        pthread_cond_wait(&(buffer -> can_produce), &(buffer -> mutex));
    }

    memcpy(
        &(buffer -> elements[buffer -> head * buffer -> size]),
        element, 
        buffer -> size
    );

    buffer -> count++;
    buffer -> head = (buffer -> head + 1) % buffer -> capacity;

    pthread_cond_signal(&(buffer -> can_consume));
    pthread_mutex_unlock(&(buffer -> mutex));

    return OK;
}

/*
    METHOD: Buffer_pop
    ARGUMENTS:
        buffer - an object where an element will be popped
        element - an object to be popped into a given buffer
    PURPOSE: pop of an element from a given buffer
    RETURN: enums integer value
*/
int Buffer_pop(
    Buffer* const buffer, 
    void* element
) {
    if (buffer == NULL || element == NULL) { return ERR_PARAMS; }

    pthread_mutex_lock(&(buffer -> mutex));

    if (Buffer_isEmpty(buffer)) {
        pthread_cond_wait(&(buffer -> can_consume), &(buffer -> mutex));
    }

    memcpy(
        element,
        &(buffer -> elements[buffer -> tail * buffer -> size]), 
        buffer -> size
    );

    buffer -> count--;
    buffer -> tail = (buffer -> tail + 1) % buffer -> capacity;

    pthread_cond_signal(&(buffer -> can_produce));
    pthread_mutex_unlock(&(buffer -> mutex));

    return OK;
}

/*
    METHOD: Buffer_destroy
    ARGUMENTS:
        buffer - an object where memory will be freed
    PURPOSE: free of a given object's memory
    RETURN: nothing
*/
void Buffer_destroy(
    Buffer* buffer
) {
    if (buffer == NULL) { return; }

    pthread_mutex_destroy(&(buffer -> mutex));
    pthread_cond_destroy(&(buffer -> can_produce));
    pthread_cond_destroy(&(buffer -> can_consume));

    free(buffer);
}

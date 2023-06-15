/*
    AUTHOR: DENIS STOCKI                  
    FILE: buffer.c                       
    PURPOSE: implementation of buffer module
*/

#include <pthread.h>    
#include <stdlib.h>     
#include <string.h>     
#include <time.h>       
#include <sys/time.h>   
#include "../buffer/buffer.h"

#define CAPACITY 32

struct buffer {
    pthread_cond_t can_produce;
    pthread_cond_t can_consume;
    pthread_mutex_t mutex;
    size_t tail;
    size_t head;
    size_t count;
    int elements[CAPACITY];   
};

/*
    METHOD: Buffer_init
    PURPOSE: creation of Buffer 'object'
    RETURN: Buffer 'object' or NULL in 
        case creation was not possible 
*/
Buffer* Buffer_init(

) {
    Buffer* buffer = malloc(sizeof(Buffer) + sizeof(ProcessorStats) * CAPACITY);

    if (buffer == NULL) { return NULL; }

    *buffer = (Buffer) {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .can_consume = PTHREAD_COND_INITIALIZER,
        .can_produce = PTHREAD_COND_INITIALIZER,
        .tail = 0,
        .head = 0,
        .count = 0,
        .elements = {0}
    };

    return buffer;
}

/*
    METHOD: Buffer_isEmpty
    PURPOSE: check if a given Buffer 'object' is empty
    RETURN: true in case it is empty, else false
*/
bool Buffer_isEmpty(
    Buffer* buffer
) {
    if (buffer == NULL) { return false; }

    if (buffer -> count == 0) { return true; }
    else { return false; }
}

bool Buffer_isFull(
    Buffer* buffer
) {
    if (buffer == NULL) { return false; }

    if (buffer -> count == CAPACITY) { return true; }
    else { return false; }
}

int Buffer_push(
    Buffer* buffer, 
    ProcessorStats* element
) {
    if (buffer == NULL || element == NULL) { return -1; }

    pthread_mutex_lock(&buffer->mutex);

    if (Buffer_isFull(buffer)) {
        pthread_cond_wait(&(buffer -> can_produce), &(buffer -> mutex));
    }

    memcpy(
        &(buffer -> elements[buffer -> head * sizeof(ProcessorStats)]), 
        element, 
        sizeof(ProcessorStats)
    );

    buffer -> count++;
    buffer -> head = (buffer -> head + 1) % CAPACITY;

    pthread_cond_signal(&(buffer -> can_consume));
    pthread_mutex_unlock(&(buffer -> mutex));

    return 1;
}

int Buffer_pop(
    Buffer* buffer, 
    ProcessorStats* element
) {
    if (buffer == NULL || element == NULL) { return -1; }

    pthread_mutex_lock(&(buffer -> mutex));

    if (Buffer_isEmpty(buffer)) {
        pthread_cond_wait(&(buffer -> can_consume), &(buffer -> mutex));
    }

    memcpy(
        &(buffer -> elements[buffer -> tail * sizeof(ProcessorStats)]), 
        element, 
        sizeof(ProcessorStats)
    );

    buffer -> count--;
    buffer -> tail = (buffer -> tail + 1) % CAPACITY;

    pthread_cond_signal(&(buffer -> can_produce));
    pthread_mutex_unlock(&(buffer -> mutex));

    return 1;
}

void Buffer_free(
    Buffer* buffer
) {
    if (buffer == NULL) { return; }

    pthread_cond_destroy(&(buffer -> can_produce));
    pthread_cond_destroy(&(buffer -> can_consume));
    pthread_mutex_destroy(&(buffer -> mutex));

    free(buffer);
}

/*
    AUTHOR: DENIS STOCKI                  
    FILE: watchdog.c                       
    PURPOSE: implementation of watchdog module
*/

#include <pthread.h>    
#include <stdlib.h>     
#include <string.h>     
#include <stdbool.h>     
#include <time.h>       
#include <sys/time.h>   
#include "../enums/enums.h"
#include "watchdog.h"

struct watchdog {
    pthread_cond_t can_notify;
    pthread_cond_t can_check;
    pthread_mutex_t mutex;
    volatile sig_atomic_t* status;
    char* name;
    bool notified;
};

/*
    METHOD: Watchdog_init
    PURPOSE: creation of Watchdog 'object'
    RETURN: Watchdog 'object' or NULL in 
        case creation was not possible 
*/
Watchdog* Watchdog_init(
    volatile sig_atomic_t* status,
    char* name
) {
    Watchdog* watchdog;

    if (name == NULL || status == TERMINATED) { return NULL; }
    
    watchdog = (Watchdog*) malloc(sizeof(Watchdog));

    if (watchdog == NULL) { return NULL; }

    *watchdog = (Watchdog) {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .can_notify = PTHREAD_COND_INITIALIZER,
        .can_check = PTHREAD_COND_INITIALIZER,
        .notified = true,
        .status = status
    };

    return watchdog;
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

    if (buffer -> count == buffer -> capacity) { return true; }
    else { return false; }
}

int Buffer_push(
    Buffer* buffer, 
    void* element
) {
    if (buffer == NULL || element == NULL) { return ERR_PARAMS; }

    pthread_mutex_lock(&buffer->mutex);

    if (Buffer_isFull(buffer)) {
        pthread_cond_wait(&(buffer -> can_produce), &(buffer -> mutex));
    }

    uint8_t* ptr = &(buffer -> elements[buffer -> head * buffer -> size]);

    memcpy(
        ptr, 
        element, 
        buffer -> size
    );

    buffer -> count++;
    buffer -> head = (buffer -> head + 1) % buffer -> capacity;

    pthread_cond_signal(&(buffer -> can_consume));
    pthread_mutex_unlock(&(buffer -> mutex));
    return SUCCESS;
}

int Buffer_pop(
    Buffer* buffer, 
    void* element
) {
    if (buffer == NULL || element == NULL) { return ERR_PARAMS; }

    pthread_mutex_lock(&(buffer -> mutex));

    if (Buffer_isEmpty(buffer)) {
        pthread_cond_wait(&(buffer -> can_consume), &(buffer -> mutex));
    }

    uint8_t* ptr = &(buffer -> elements[buffer -> tail * buffer -> size]);

    memcpy(
        element,
        ptr,  
        buffer -> size
    );

    buffer -> count--;
    buffer -> tail = (buffer -> tail + 1) % buffer -> capacity;

    pthread_cond_signal(&(buffer -> can_produce));
    pthread_mutex_unlock(&(buffer -> mutex));
    
    return SUCCESS;
}

void Buffer_free(
    Buffer* buffer
) {
    if (buffer == NULL) { return; }

    pthread_mutex_destroy(&(buffer -> mutex));
    pthread_cond_destroy(&(buffer -> can_produce));
    pthread_cond_destroy(&(buffer -> can_consume));

    free(buffer);
}

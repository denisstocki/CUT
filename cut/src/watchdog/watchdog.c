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
#include "../notifier/notifier.h"
#include "watchdog.h"

struct watchdog {
    pthread_t thread;
    Notifier* notifier;
    char* name;
};

typedef struct ThreadParams {
    Watchdog* watchdog;
    volatile sig_atomic_t* status;
} ThreadParams;

static void* Watchdog_watch(void* const);

/*
    METHOD: Watchdog_init
    PURPOSE: creation of Watchdog 'object'
    RETURN: Watchdog 'object' or NULL in 
        case creation was not possible 
*/
Watchdog* Watchdog_init(
    Notifier* notifier,
    char* name
) {
    Watchdog* watchdog;

    if (name == NULL || notifier == NULL) { return NULL; }
    
    watchdog = (Watchdog*) malloc(sizeof(Watchdog));

    if (watchdog == NULL) { return NULL; }
    
    *watchdog = (Watchdog) {
        .notifier = notifier,
        .name = name
    };

    return watchdog;
}

int Watchdog_start(
    Watchdog* watchdog,
    volatile sig_atomic_t* status
) {
    if (watchdog == NULL || *status != RUNNING) { return ERR_PARAMS; }
    
    ThreadParams* params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .watchdog = watchdog,
        .status = status
    };

    if (pthread_create(&(watchdog -> thread), NULL, Watchdog_watch, (void*) params) != 0) {
        return ERR_CREATE;
    }

    return SUCCESS;
}

static void* Watchdog_watch(
    void* const args
) {
    ThreadParams* params = (ThreadParams*) args;
    bool notified = false;

    while (*(params -> status) == RUNNING) {
        sleep(2);

        if (Notifier_check(params -> watchdog -> notifier, &notified) != SUCCESS) {
            free(params);
            pthread_exit(NULL);
        }

        if (!notified) {
            *params -> status = TERMINATED;
            break;
        }
    }

    Logger_log("READER", "THREAD FUNCTION FINISHED");

    free(params);
    pthread_exit(NULL);
}

int Watchdog_join(
    Watchdog* watchdog
) {
    if (watchdog == NULL) { return ERR_PARAMS; }

    if (pthread_join(watchdog -> thread, NULL) != 0) { return ERR_JOIN; }
    
    return SUCCESS;
}

void Watchdog_destroy(
    Watchdog* watchdog
) {
    if (watchdog == NULL) { return; }

    free(watchdog);
}

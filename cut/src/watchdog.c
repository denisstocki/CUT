/*
    AUTHOR: DENIS STOCKI                  
    FILE: watchdog.c                       
    PURPOSE: implementation of watchdog module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <pthread.h>   
#include <stdio.h> 
#include <stdlib.h>     
#include <string.h>     
#include <stdbool.h>     
#include <unistd.h>
#include <time.h>       
#include <sys/time.h>   

// INCLUDES OF INSIDE LIBRARIES
#include "../inc/enums.h"
#include "../inc/notifier.h"
#include "../inc/logger.h"
#include "../inc/watchdog.h"

// STRUCTURE HOLDING WATCHDOG OBJECT
struct watchdog {
    pthread_t thread;
    Notifier* notifier;
    char* name;
};

// STRUCTURE HOLDING THREAD FUNCTION PARAMETERS
typedef struct ThreadParams {
    Watchdog* watchdog;
    volatile sig_atomic_t* status;
    atomic_flag* status_watch;
} ThreadParams;

// DECLARATIONS OF PROTOTYPE FUNCTIONS
static void* Watchdog_watch(void* const);

/*
    METHOD: Watchdog_init
    ARGUMENTS:
        notifier - a Notifier object to be passed to Watchdog object
        name - name of parent thread creating this object
    PURPOSE: creation of Watchdog object
    RETURN: Watchdog object or NULL in 
        case creation was not possible 
*/
Watchdog* Watchdog_init(
    Notifier* const notifier,
    char* const name
) {
    Watchdog* watchdog;
    size_t mixed_name_length;
    char* mixed_name;
    
    mixed_name_length = strlen("WATCHDOG-") + strlen(name) + 1;
    mixed_name = (char*) malloc(mixed_name_length);

    if (mixed_name == NULL) { return NULL; }

    snprintf(mixed_name, mixed_name_length, "WATCHDOG-%s", name);

    Logger_log(mixed_name, "INIT STARTED");

    if (name == NULL || notifier == NULL) { return NULL; }
    
    watchdog = (Watchdog*) malloc(sizeof(Watchdog));

    if (watchdog == NULL) { return NULL; }
    
    *watchdog = (Watchdog) {
        .notifier = notifier,
        .name = mixed_name
    };

    Logger_log(mixed_name, "INIT FINISHED");

    return watchdog;
}

/*
    METHOD: Watchdog_start
    ARGUMENTS:
        watchdog - a Watchdog object to work on
        status - tracker's status variable
    PURPOSE: start of watchdog thread
    RETURN: enum integer value
*/
int Watchdog_start(
    Watchdog* watchdog,
    volatile sig_atomic_t* status,
    atomic_flag* status_watch
) {
    ThreadParams* params;

    Logger_log(watchdog -> name, "START STARTED");

    if (watchdog == NULL || *status != RUNNING) { return ERR_PARAMS; }
    
    params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .watchdog = watchdog,
        .status = status,
        .status_watch = status_watch
    };

    if (pthread_create(&(watchdog -> thread), NULL, Watchdog_watch, (void*) params) != 0) {
        return ERR_CREATE;
    }

    Logger_log(watchdog -> name, "START FINISHED");

    return OK;
}

/*
    METHOD: Watchdog_watch
    ARGUMENTS:
        args - default function's parameters
    PURPOSE: function acomplishing watchdog thread's work
    RETURN: nothing
*/
static void* Watchdog_watch(
    void* const args
) {
    ThreadParams* params;
    bool notified;

    params = (ThreadParams*) args;
    notified = false;

    Logger_log(params -> watchdog -> name, "WATCH STARTED");

    sleep(2);

    while (*(params -> status) == RUNNING) {
        Logger_log(params -> watchdog -> name, "CHECKING NOTIFIER");

        if (Notifier_check(params -> watchdog -> notifier, &notified) != OK) {
            free(params);
            pthread_exit(NULL);
        }

        if (!notified && *(params -> status) == RUNNING) {
            Logger_log(params -> watchdog -> name, "NOT NOTIFIED");

            if (!atomic_flag_test_and_set(params -> status_watch)) {
                *params -> status = TERMINATED;
                break;
            }
        } else {
            Logger_log(params -> watchdog -> name, "NOTIFIED");
            sleep(2);
        }
    }

    Logger_log(params -> watchdog -> name, "WATCH FINISHED");

    free(params);
    pthread_exit(NULL);
}

/*
    METHOD: Watchdog_join
    ARGUMENTS:
        watchdog - a Watchdog object which thread will be joined
    PURPOSE: join of a given Watchdog object's thread
    RETURN: enum integer value
*/
int Watchdog_join(
    Watchdog* const watchdog
) {
    Logger_log(watchdog -> name, "JOIN STARTED");

    if (watchdog == NULL) { return ERR_PARAMS; }

    if (pthread_join(watchdog -> thread, NULL) != 0) { return ERR_JOIN; }

    Logger_log(watchdog -> name, "JOIN FINISHED");
    
    return OK;
}

/*
    METHOD: Watchdog_destroy
    ARGUMENTS:
        watchdog - a Watchdog object to be freed
    PURPOSE: free of Watchdog object and its inside objects
    RETURN: nothing
*/
void Watchdog_destroy(
    Watchdog* watchdog
) {
    Logger_log(watchdog -> name, "DESTROY STARTED");

    if (watchdog == NULL) { return; }

    Logger_log(watchdog -> name, "DESTROY FINISHED");

    free(watchdog -> name);
    free(watchdog);
}

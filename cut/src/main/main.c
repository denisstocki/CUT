/*
    AUTHOR: DENIS STOCKI                  
    FILE: main.c                       
    PURPOSE: handling of tracker object's functionality
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// INCLUDES OF INSIDE LIBRARIES
#include "../tracker/tracker.h"
#include "../logger/logger.h"
#include "../enums/enums.h"

// PROTOTYPE FUNCTIONS DECLARATIONS
void handle_sigterm(int const);

// GLOBAL VARIABLES DECLARATIONS
static Tracker* tracker;

/*
    METHOD: handle_sigterm
    ARGUMENTS: 
        signum - id of a received signal
    PURPOSE: invocation of behaviour reserved for sigterm signal
    RETURN: nothing
*/
void handle_sigterm(
    int const signum
) {
    if (
        signum == SIGINT || 
        signum == SIGTERM
    ) {
        printf("\n");
        Tracker_terminate(tracker);
    }
}

/*
    METHOD: main
    ARGUMENTS: none
    PURPOSE: invocation of behaviour expected from Tracker object
    RETURN: an integer number describing correction 
        of this function's execution
*/
int main(
    void
) {
    signal(SIGINT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);

    if (Logger_init() != SUCCESS) {
        printf("[MAIN]: ERROR WHEN CREATING LOGGER\n");
        return -1;
    }

    if (Logger_start() != SUCCESS) {
        printf("[MAIN]: ERROR WHEN STARTING LOGGER\n");
        Logger_destroy();
        return -1;
    }

    Logger_log("MAIN", "PROGRAMME STARTED");

    tracker = Tracker_init();

    if (tracker == NULL) { 
        Logger_log("MAIN", "ERROR WHEN CREATING TRACKER");
        Logger_terminate();

        if (Logger_join() != SUCCESS) {
            printf("[MAIN]: ERROR WHEN JOINING LOGGER\n");
        }

        Logger_destroy();

        return -1; 
    }

    if (Tracker_start(tracker) != SUCCESS) { 
        Logger_log("MAIN", "ERROR WHEN STARTING TRACKER");
        Tracker_destroy(tracker);
        Logger_terminate();

        if (Logger_join() != SUCCESS) {
            printf("[MAIN]: ERROR WHEN JOINING LOGGER\n");
        }

        Logger_destroy();
        return -1; 
    }

    Tracker_destroy(tracker);
    printf("[LOGGER]: LOGGING REMAINING LOGS...\n");

    Logger_log("MAIN", "PROGRAMME FINISHED");

    Logger_terminate();

    if (Logger_join() != SUCCESS) {
        printf("[MAIN]: ERROR WHEN JOINING LOGGER\n");
        Logger_destroy();
        return -1;
    }

    Logger_destroy();
    
    return 0;
}

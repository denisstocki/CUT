/*
    AUTHOR: DENIS STOCKI                  
    FILE: main.c                       
    PURPOSE: handling of tracker and logger objects' functionality
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
void handle_signal(int const);

// GLOBAL VARIABLES DECLARATIONS
static Tracker* tracker;

/*
    METHOD: handle_signal
    ARGUMENTS: 
        signum - id of a received signal
    PURPOSE: invocation of behaviour reserved for sigterm and sigint signal
    RETURN: nothing
*/
void handle_signal(
    int const signum
) {
    Logger_log("MAIN", "HANDLE SIGNAL STARTED");

    if (
        signum == SIGINT || 
        signum == SIGTERM
    ) {
        printf("\n");
        Tracker_terminate(tracker);
    }

    Logger_log("MAIN", "HANDLE SIGNAL STARTED");
}

/*
    METHOD: main
    ARGUMENTS: none
    PURPOSE: invocation of behaviour expected from Tracker and Logger objects
    RETURN: an integer number describing correction 
        of this function's execution
*/
int main(
    void
) {
    printf("STARTING PROGRAMME...\n");

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (Logger_init() != OK) {
        printf("[MAIN]: ERROR WHEN CREATING LOGGER\n");
        return -1;
    }

    if (Logger_start() != OK) {
        printf("[MAIN]: ERROR WHEN STARTING LOGGER\n");
        Logger_destroy();
        return -1;
    }

    Logger_log("MAIN", "PROGRAMME STARTED");

    tracker = Tracker_init();

    if (tracker == NULL) { 
        Logger_log("MAIN", "ERROR WHEN CREATING TRACKER");
        Logger_terminate();

        if (Logger_join() != OK) {
            printf("[MAIN]: ERROR WHEN JOINING LOGGER\n");
        }

        Logger_destroy();

        return -1; 
    }

    if (Tracker_start(tracker) != OK) { 
        Logger_log("MAIN", "ERROR WHEN STARTING TRACKER");

        Tracker_destroy(tracker);

        Logger_terminate();

        if (Logger_join() != OK) {
            printf("[MAIN]: ERROR WHEN JOINING LOGGER\n");
        }

        Logger_destroy();

        return -1; 
    }

    Tracker_destroy(tracker);

    printf("LOGGING REMAINING LOGS...\n");

    Logger_log("MAIN", "PROGRAMME FINISHED");
    Logger_terminate();

    if (Logger_join() != OK) {
        printf("[MAIN]: ERROR WHEN JOINING LOGGER\n");
        Logger_destroy();
        return -1;
    }

    Logger_destroy();

    printf("PROGRAMME FINISHED !\n");
    
    return 0;
}

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
    if (signum == SIGINT 
        || signum == SIGTERM
    ) {
        printf("\n");
        Tracker_terminate(tracker);
    }
}

/*
    METHOD: main
    ARGUMENTS: none
    PURPOSE: invocation of behaviour expected from Tracker 'object' (runs whole programme)
    RETURN: an integer number describing correction of this function's execution
*/
int main(
    void
) {
    tracker = Tracker_init();

    if (tracker == NULL) { return -1; }
    
    signal(SIGINT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);

    if (Tracker_start(tracker) != SUCCESS) { return -1; }
    
    return 0;
}

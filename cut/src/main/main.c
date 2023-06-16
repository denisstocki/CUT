/*
    AUTHOR: DENIS STOCKI                  
    FILE: main.c                       
    PURPOSE: handling of tracker 'object''s functionality
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../tracker/tracker.h"
#include "../logger/logger.h"

void handle_sigterm(int signum);

static Tracker* tracker;

/*
    METHOD: handle_sigterm
    PURPOSE: invocation of behaviour reserved for sigterm signal
    RETURN: nothing
*/
void handle_sigterm(
    int signum
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
    PURPOSE: invocation of behaviour expected from Tracker 'object' (runs whole programme)
    RETURN: an integer number describing correction of this function's execution
*/
int main(
    void
) {
    tracker = Tracker_init();

    if (tracker == NULL) {
        return -1;
    }
    
    signal(SIGINT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);

    Tracker_start(tracker);
    
    return 0;
}

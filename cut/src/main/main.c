/*
    AUTHOR: DENIS STOCKI                  
    FILE: main.c                       
    PURPOSE: handling of tracker 'object''s functionality
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../tracker/tracker.h"

void handle_sigterm(int signum) __attribute__((noreturn));

static Tracker* tracker;

/*
    METHOD: handle_sigterm
    PURPOSE: invocation of behaviour reserved for sigterm signal
    RETURN: nothing
*/
void handle_sigterm(
    int signum
) {
    (void) signum;
    printf("[MAIN]: SIGTERM HANDLER - INVOKING FREE ON TRACKER\n");

    Tracker_free(tracker); 
    
    printf("[MAIN]: SIGTERM HANDLER - FREE INVOCATION ON TRACKER FINISHED\n");
    
    exit(0);
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

    signal(SIGINT, handle_sigterm);

    printf("CORES: %ld\n", Tracker_get_proc(tracker));

    Tracker_start(tracker);
    Tracker_free(tracker);
    
    return 0;
}

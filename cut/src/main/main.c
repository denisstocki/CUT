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
static Logger* logger;

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
        printf("[MAIN]: SIGNAL HANDLER - CHANGING TRACKER STATUS TO TERMINATED\n");

        if(Tracker_set_status(tracker, TERMINATED) == 1) {
            printf("[MAIN]: SIGNAL HANDLER - TRACKER STATUS CHANGED\n");
            printf("[MAIN]: SIGNAL HANDLER - TRACKER STATUS: %s\n", TrackerStatusNames[Tracker_get_status(tracker)]);
            printf("[MAIN]: SIGNAL HANDLER - INVOKING FREE ON TRACKER\n");
            Tracker_free(tracker);
            printf("[MAIN]: SIGNAL HANDLER - FREE INVOCATION ON TRACKER FINISHED\n");
            exit(0);   
        } else {
            printf("[MAIN]: SIGNAL HANDLER - TRACKER STATUS NOT CHANGED\n");
            printf("[MAIN]: SIGNAL HANDLER - CAN NOT INVOKE FREE ON TRACKER\n");
        }
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
    tracker = Tracker_init("/Users/denisstocki/CUT/cut/logs/log.txt");

    if (tracker == NULL) {
        fprintf(stderr, "[MAIN]: TRACKER INIT - TRACKER OBJECT FAILED TO CREATE\n");
        fprintf(stderr, "[MAIN]: TRACKER INIT - EXITING PROGRAMME\n");
        return -1;
    }
    
    logger = Tracker_get_logger(tracker);
    
    signal(SIGINT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);

    Tracker_start(tracker);
    
    return 0;
}

/*
    AUTHOR: DENIS STOCKI                  
    FILE: watchdog.h                       
    PURPOSE: interface for watchdog module 
*/

#ifndef WATCHDOG_H
#define WATCHDOG_H

// INCLUDES OF OUTSIDE LIBRARIES
#include <signal.h>
#include <stdatomic.h>

// INCLUDES OF INSIDE LIBRARIES
#include "notifier.h"

// ENCAPSULATION ON WATCHDOG OBJECT
typedef struct watchdog Watchdog;

// DECLARATIONS OF PROTOTYPE FUNCTIONS
Watchdog* Watchdog_init(Notifier* const, char* const);
int Watchdog_start(Watchdog*, volatile sig_atomic_t*, atomic_flag*);
int Watchdog_join(Watchdog* const);
void Watchdog_destroy(Watchdog*);

#endif 

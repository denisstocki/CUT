/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.h                       
    PURPOSE: interface for logger module 
*/

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <signal.h>
#include "../notifier/notifier.h"

typedef struct watchdog Watchdog;

Watchdog* Watchdog_init(Notifier*, char*);
int Watchdog_start(Watchdog*, volatile sig_atomic_t*);
int Watchdog_join(Watchdog*);
void Watchdog_destroy(Watchdog*);

#endif 

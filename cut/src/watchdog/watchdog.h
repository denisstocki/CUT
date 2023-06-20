/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.h                       
    PURPOSE: interface for logger module 
*/

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <signal.h>

typedef struct watchdog Watchdog;

Watchdog* Watchdog_init(volatile sig_atomic_t*, char*);

#endif 

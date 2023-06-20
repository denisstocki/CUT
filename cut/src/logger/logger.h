/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.h                       
    PURPOSE: interface for logger module 
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <signal.h>

int Logger_init(void);
int Logger_join(void);
int Logger_start(volatile sig_atomic_t*);
int Logger_log(char* name, char* info);
void Logger_destroy(void);

#endif 

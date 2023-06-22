/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.h                       
    PURPOSE: interface for logger module 
*/

#ifndef LOGGER_H
#define LOGGER_H

// OUTSIDE WORLD INCLUDES
#include <signal.h>

// INSIDE WORLD INCLUDES
#include "buffer.h"

// DECLARATIONS OF OUTSIDE PROTOTYPES
int Logger_init(void);
int Logger_join(void);
int Logger_start(void);
int Logger_log(char* const, char* const);
void Logger_terminate(void);
void Logger_destroy(void);

#endif 

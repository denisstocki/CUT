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
#include "../buffer/buffer.h"

// DECLARATIONS OF OUTSIDE PROTOTYPES
int Logger_init(Buffer* const);
int Logger_join(void);
int Logger_start(volatile sig_atomic_t*);
int Logger_log(char* const, char* const);
void Logger_destroy(void);

#endif 

/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.h                       
    PURPOSE: interface for logger module 
*/

#ifndef LOGGER_H
#define LOGGER_H

// Usage of pseudo objective 
// encapsulation on struct logger.
typedef struct logger Logger;

Logger* Logger_init(char* path);
void Logger_log(Logger* logger, char* name, char* info);
void Logger_free(Logger* logger);

#endif 

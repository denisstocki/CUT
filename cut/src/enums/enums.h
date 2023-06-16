/*
    AUTHOR: DENIS STOCKI                  
    FILE: tracker.h                       
    PURPOSE: interface for tracker module 
*/

#ifndef ENUMS_H
#define ENUMS_H

enum {
    CREATED,
    RUNNING, 
    TERMINATED
};

enum {
    ERR_ALLOC,
    ERR_PARAMS,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    SUCCESS
};

#endif 

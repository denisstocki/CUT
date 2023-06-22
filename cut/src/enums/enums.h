/*
    AUTHOR: DENIS STOCKI                  
    FILE: enums.h                       
    PURPOSE: interface for enums module 
*/

#ifndef ENUMS_H
#define ENUMS_H

// ENUM FOR STATE VARIABLES
enum states {
    CREATED,
    RUNNING, 
    TERMINATED
};

// ENUM FOR FUNCTIONS RETURN VALUES
enum errors {
    ERR_ALLOC,
    ERR_PARAMS,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    ERR_CREATE,
    ERR_JOIN,
    ERR_READ,
    ERR_RUN,
    ERR_PUSH,
    ERR_INIT,
    OK,
    INITIALIZED,
    ANALYZED
};

#endif 

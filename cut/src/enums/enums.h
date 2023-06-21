/*
    AUTHOR: DENIS STOCKI                  
    FILE: enums.h                       
    PURPOSE: interface for enums module 
*/

#ifndef ENUMS_H
#define ENUMS_H

enum states {
    CREATED,
    RUNNING, 
    TERMINATED
};

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
    SUCCESS,
    ERR_INIT,
    INITIALIZED,
    ANALYZED
};

#endif 

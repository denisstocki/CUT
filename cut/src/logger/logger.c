/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.c                       
    PURPOSE: implementation of logger module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "logger.h"

#define MAX_ENTRIES 32

struct logger {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char* entries[MAX_ENTRIES];
    int count;
    int head;
    int tail;
    char padding[4];
    FILE* file;
};

/*
    METHOD: Logger_init
    PURPOSE: creation of Logger 'object'
    RETURN: Logger 'object' or NULL in 
        case creation was not possible 
*/
Logger* Logger_init(
    char* path
) {
    Logger* logger = (Logger*) malloc(sizeof(Logger));
    logger -> count = 0;
    logger -> tail = 0;
    logger -> head = 0;
    logger -> file = fopen(path, "w");

    if (logger -> file == NULL) {
       printf("[LOGGER]: COULD NOT CREATE FILE UNDER A GIVEN PATH\n");
       return NULL;
    } 

    pthread_mutex_init(&(logger -> mutex), NULL);
    pthread_cond_init(&(logger -> cond), NULL);

    return logger;
}

/*
    METHOD: Logger_log
    PURPOSE: sending a single log into the file under saved path
    RETURN: nothing
*/
void Logger_log(
    Logger* logger, 
    char* name, 
    char* info
) {
    fprintf(logger -> file, "[%s]: %s\n", name, info);
    fflush(logger -> file);
}

/*
    METHOD: Logger_free
    PURPOSE: frees reserved memory for a given Logger 'object' and its nested 'objects'
    RETURN: Logger 'object' or NULL in 
        case creation was not possible 
*/
void Logger_free(
    Logger* logger
) {
    logger -> file = NULL;
    free(logger);
}

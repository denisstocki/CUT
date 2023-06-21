/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.c                       
    PURPOSE: implementation of logger module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

// INCLUDES OF INSIDE LIBRARIES
#include "../buffer/buffer.h"
#include "../enums/enums.h"
#include "logger.h"

// DEFINITIONS OF MACRO
#define PATH "log.txt"

// STRUCTURE FOR HOLDING LOGGER SINGLETON OBJECT
typedef struct logger {
    pthread_t thread;
    Buffer* buffer;
    FILE* file;
} Logger;

// STRUCTURE FOR HOLDING THREAD FUNCTION'S PARAMETERS
typedef struct {
    volatile sig_atomic_t* status;
} ThreadParams;

// STATIC GLOBAL VARIABLES
static bool initialized = false;
static bool joined = false;
static bool started = false;
static Logger* logger;

// PROTOTYPE FUNCTIONS DECLARATIONS
static void* Logger_threadf(void* const);

/*
    METHOD: Logger_init
    ARGUMENTS: 
        buffer - buffer object to work on in the function
    PURPOSE: creation of Logger object
    RETURN: int from enums, explaining result
*/
int Logger_init(
    Buffer* const buffer
) {
    FILE* file;

    if (initialized || buffer == NULL) { return ERR_INIT; }

    file = fopen(PATH, "w");
    if (file == NULL) { return ERR_FILE_OPEN; }

    logger = (Logger*) malloc(sizeof(Logger));

    if (logger == NULL) { 
        fclose(file);
        return ERR_ALLOC; 
    }
    
    *logger = (Logger) {
        .buffer = buffer,
        .file = file
    };

    initialized = true;
    
    return SUCCESS;
}

/*
    METHOD: Logger_join
    ARGUMENTS: none
    PURPOSE: joining Logger's thread to parent's thread
    RETURN: int from enums, explaining result
*/
int Logger_join(
    void
) {
    if (joined) { return ERR_JOIN; }

    if (pthread_join(logger -> thread, NULL) != 0) {
        fclose(logger -> file);
        free(logger);
        return ERR_JOIN;
    }

    joined = true;

    return SUCCESS;
}

/*
    METHOD: Logger_start
    ARGUMENTS: 
        status - an address to a tracker's status variable
    PURPOSE: start of Logger's thread and thread function
    RETURN: int from enums, explaining result
*/
int Logger_start(
    volatile sig_atomic_t* status
) {
    ThreadParams* params;

    if (started) { return ERR_RUN; }
    
    params = (ThreadParams*)malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .status = status
    };

    if (pthread_create(&(logger -> thread), NULL, Logger_threadf, (void*) params) != 0) {
        fclose(logger -> file);
        free(logger);
        return ERR_CREATE; 
    }
    
    started = true;

    return SUCCESS;
}

/*
    METHOD: Logger_threadf
    ARGUMENTS: 
        args - arguments passed by Logger_start function
    PURPOSE: completing Logger thread duties
    RETURN: pointer
*/
static void* Logger_threadf(
    void* const args
) {
    ThreadParams* params;
    struct timespec sleepTime;
    char* message;

    params = (ThreadParams*) args;
    message = malloc(sizeof(char) * 256);

    if (message == NULL) {pthread_exit(NULL); }

    while (*(params -> status) == RUNNING) {
        if (Buffer_pop(logger -> buffer, message) != SUCCESS) {
            break;
        }

        fprintf(logger -> file, "%s\n", message);
        fflush(logger -> file);

        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        nanosleep(&sleepTime, NULL);
    }
    
    free(message);
    free(params);

    pthread_exit(NULL);
}

/*
    METHOD: Logger_log
    ARGUMENTS: 
        name - name of a module which invoked this function
        info - content of a message sent to logger
    PURPOSE: appearance of a single log in log.txt
    RETURN: int from enums, explaining result
*/
int Logger_log(
    char* const name, 
    char* const info
) {
    char message[256];

    if (name == NULL || info == NULL) { return ERR_PARAMS; }

    snprintf(message, sizeof(message), "[%s]: %s", name, info);

    if (Buffer_push(logger -> buffer, message) != SUCCESS) {
        return ERR_PUSH;
    }

    return SUCCESS;
}

/*
    METHOD: Logger_destroy
    ARGUMENTS: none
    PURPOSE: free of memory taken by Logger object
    RETURN: nothing
*/
void Logger_destroy(
    void
) {
    fclose(logger -> file);
    free(logger);
}

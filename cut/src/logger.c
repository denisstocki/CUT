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
#include "../inc/buffer.h"
#include "../inc/enums.h"
#include "../inc/logger.h"

// DEFINITIONS OF MACRO
#define PATH "logs/log.txt"

// STRUCTURE FOR HOLDING LOGGER SINGLETON OBJECT
typedef struct logger {
    pthread_t thread;
    Buffer* buffer;
    FILE* file;
    volatile sig_atomic_t status;
    char padding[4];
} Logger;

// STATIC GLOBAL VARIABLES
static bool initialized = false;
static bool joined = false;
static bool started = false;
static Logger* logger;

// PROTOTYPE FUNCTIONS DECLARATIONS
static void* Logger_threadf(void* const);

/*
    METHOD: Logger_init
    ARGUMENTS: none
    PURPOSE: creation of Logger object
    RETURN: int from enums, explaining result
*/
int Logger_init(
    void
) {
    FILE* file;
    Buffer* buffer;

    if (initialized) { return ERR_INIT; }

    file = fopen(PATH, "w");
    if (file == NULL) { return ERR_FILE_OPEN; }

    buffer = Buffer_init(sizeof(char) * 256, 100);
    if (buffer == NULL) {
        fclose(file);
        return ERR_ALLOC;
    }

    logger = (Logger*) malloc(sizeof(Logger));

    if (logger == NULL) { 
        fclose(file);
        return ERR_ALLOC; 
    }
    
    *logger = (Logger) {
        .buffer = buffer,
        .file = file,
        .status = CREATED
    };

    initialized = true;
    
    return OK;
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

    return OK;
}

/*
    METHOD: Logger_start
    ARGUMENTS: none
    PURPOSE: start of Logger's thread and thread function
    RETURN: int from enums, explaining result
*/
int Logger_start(
    void
) {
    if (started) { return ERR_RUN; }
    
    logger -> status = RUNNING;

    if (pthread_create(&(logger -> thread), NULL, Logger_threadf, NULL) != 0) {
        fclose(logger -> file);
        free(logger);
        return ERR_CREATE; 
    }
    
    started = true;

    return OK;
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
    char* message;

    (void) args;

    message = malloc(sizeof(char) * 256);

    if (message == NULL) {pthread_exit(NULL); }

    while (logger -> status == RUNNING) {
        if (Buffer_pop(logger -> buffer, message) != OK) {
            break;
        }

        fprintf(logger -> file, "%s\n", message);
        fflush(logger -> file);
    }

    while (!Buffer_isEmpty(logger -> buffer)) {
        if (Buffer_pop(logger -> buffer, message) != OK) {
            break;
        }

        fprintf(logger -> file, "%s\n", message);
        fflush(logger -> file);
    }
    
    free(message);

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

    if (Buffer_push(logger -> buffer, message) != OK) {
        return ERR_PUSH;
    }

    return OK;
}

/*
    METHOD: Logger_terminate
    ARGUMENTS: none
    PURPOSE: sets status variable to TERMINATED
    RETURN: nothing
*/
void Logger_terminate(
    void
) {
    logger -> status = TERMINATED;
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

    Buffer_destroy(logger -> buffer);

    free(logger);
}

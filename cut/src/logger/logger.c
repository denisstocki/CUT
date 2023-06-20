/*
    AUTHOR: DENIS STOCKI                  
    FILE: logger.c                       
    PURPOSE: implementation of logger module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "../buffer/buffer.h"
#include "../enums/enums.h"
#include "logger.h"

#define PATH "log.txt"

typedef struct {
    pthread_t thread;
    Buffer* buffer;
    FILE* file;
} Logger;

typedef struct {
    volatile sig_atomic_t* status;
} ThreadParams;

static bool started = false;
static bool initialized = false;
static bool joined = false;

static void* Logger_threadf(void* arg);

static Logger* Logger_instance(

) {
    static Logger logger;

    if (!initialized) {
        logger.buffer = Buffer_init(sizeof(char) * 256, 100);

        if (logger.buffer == NULL) { return NULL; }

        logger.file = fopen(PATH, "w");

        if (logger.file == NULL) { return NULL; }
        
        initialized = true;

        Logger_log("LOGGER", "LOGGINING STARTED");
    }

    return &logger;
}

int Logger_init() {
    if (initialized) { return ERR_INIT; }
    
    Logger* logger = Logger_instance();

    if (logger == NULL) { return ERR_ALLOC; }
    
    return SUCCESS;
}

int Logger_join() {
    if (joined) { return ERR_JOIN; }

    Logger* logger = Logger_instance();

    if (pthread_join(logger -> thread, NULL) != 0) {
        free(logger -> buffer);
        return ERR_JOIN;
    }

    joined = true;

    return SUCCESS;
}

int Logger_start(
    volatile sig_atomic_t* status
) {
    if (started) { return ERR_RUN; }
    
    Logger* logger = Logger_instance();
    ThreadParams* params = (ThreadParams*)malloc(sizeof(ThreadParams));


    *params = (ThreadParams) {
        .status = status
    };

    if (pthread_create(&(logger -> thread), NULL, Logger_threadf, (void*) params) != 0) {
        Buffer_free(logger -> buffer);
        return ERR_CREATE; 
    }
    
    started = true;

    return SUCCESS;
}

int Logger_log(char* name, char* info) {
    Logger* logger = Logger_instance();
    char message[256];

    snprintf(message, sizeof(message), "[%s]: %s", name, info);

    if (Buffer_push(logger -> buffer, message) != SUCCESS) {
        return ERR_PUSH;
    }

    return SUCCESS;
}

void Logger_destroy() {
    Logger* logger = Logger_instance();

    fclose(logger -> file);
}

static void* Logger_threadf(void* arg) {
    ThreadParams* params = (ThreadParams*) arg;
    struct timespec sleepTime;
    Logger* logger = Logger_instance();
    char* text = malloc(sizeof(char) * 256);

    while (*(params -> status) == RUNNING) {
        if (Buffer_pop(logger -> buffer, text) != SUCCESS) {
            break;
        }

        fprintf(logger -> file, "%s\n", text);
        fflush(logger -> file);

        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;
        nanosleep(&sleepTime, NULL);
    }
    
    free(text);
    free(params);

    pthread_exit(NULL);
}


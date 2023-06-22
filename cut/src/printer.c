/*
    AUTHOR: DENIS STOCKI                  
    FILE: printer.c                       
    PURPOSE: implementation of printer module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// INCLUDES OF INSIDE LIBRARIES
#include "../inc/printer.h"
#include "../inc/enums.h"
#include "../inc/watchdog.h"
#include "../inc/logger.h"
#include "../inc/notifier.h"
#include "../inc/stats.h"

// STRUCTURE FOR HOLDING PRINTER OBJECT
struct printer {
    Watchdog* watchdog;
    Notifier* notifier;
    Buffer* bufferAP;
    pthread_t thread;
    uint8_t proc;
    bool thread_started;
    char padding[6];
};

// STRUCTURE FOR HOLDING THREADPARAMS
typedef struct ThreadParams {
    Printer* printer;
    volatile sig_atomic_t* status;
    atomic_flag* status_watch;
} ThreadParams;

// DECLARATIONS OF PROTOTYPE FUNCTIONS
static void* Printer_threadf(void* const);
static void Printer_print(ConvertedStats* const);
static void Printer_toScreen(float const);

/*
    METHOD: Printer_init
    ARGUMENTS:
        bufferAP - an object of Analyzer-Printer buffer
        proc - a value of computer's core count
    PURPOSE: creation of Printer object
    RETURN: Printer object or NULL in 
        case creation was not possible 
*/
Printer* Printer_init(
    Buffer* bufferAP,
    uint8_t proc
) {
    Watchdog* watchdog;
    Notifier* notifier;
    Printer* printer;

    Logger_log("PRINTER", "INIT STARTED");

    if (bufferAP == NULL || proc <= 0) { return NULL; }
    
    printer = (Printer*) malloc(sizeof(Printer));

    if (printer == NULL) { return NULL; }

    notifier = Notifier_init();

    if (notifier == NULL) { return NULL; }

    watchdog = Watchdog_init(notifier, "PRINTER");

    if (watchdog == NULL) { return NULL; }
    
    *printer = (Printer) {
        .watchdog = watchdog,
        .notifier = notifier,
        .bufferAP = bufferAP,
        .proc = proc,
        .thread_started = false
    };

    Logger_log("PRINTER", "INIT FINISHED");

    return printer;
}

/*
    METHOD: Printer_start
    ARGUMENTS:
        printer - a Printer object to work on
        status - tracker's object status variable
    PURPOSE: start of a given printer object's thread
    RETURN: enum integer value
*/
int Printer_start(
    Printer* const printer,
    volatile sig_atomic_t* status,
    atomic_flag* status_watch
) {
    ThreadParams* params;

    Logger_log("PRINTER", "START STARTED");

    if (
        printer == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .printer = printer,
        .status = status,
        .status_watch = status_watch
    };

    if (pthread_create(&(printer -> thread), NULL, Printer_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    printer -> thread_started = true;

    Logger_log("PRINTER", "START FINISHED");

    return OK;
}

/*
    METHOD: Printer_join
    ARGUMENTS:
        printer - a Printer object to work on
    PURPOSE: join of a given printer object's thread
    RETURN: enum integer value
*/
int Printer_join(
    Printer* const printer
) {
    Logger_log("PRINTER", "JOIN STARTED");

    if (printer == NULL) { return ERR_PARAMS; }
    if (printer -> thread_started == false) { return ERR_PARAMS; }
    if (pthread_join(printer -> thread, NULL) != 0) {
        return ERR_JOIN;
    }

    Logger_log("PRINTER", "JOIN FINISHED");

    return OK;
}

/*
    METHOD: Printer_threadf
    ARGUMENTS:
        args - a pointer to function's parameters
    PURPOSE: acomplishing printer thread's work
    RETURN: nothing
*/
static void* Printer_threadf(
    void* const args
) {
    ThreadParams* params;
    ConvertedStats* converted;

    Logger_log("PRINTER", "THREAD FUNCTION STARTED");

    params = (ThreadParams*)args;
    converted = malloc(sizeof(ConvertedStats) + sizeof(float) * params -> printer -> proc);

    if (converted == NULL) {
        pthread_exit(NULL);
    } 

    Watchdog_start(params -> printer -> watchdog, params -> status, params -> status_watch);

    while (*(params -> status) == RUNNING) {
        if (Buffer_pop(params -> printer -> bufferAP, converted) != OK) {
            free(converted);
            break;
        }

        Notifier_notify(params -> printer -> notifier);

        Printer_print(converted);
        
        free(converted -> percentages);
        
        sleep(1);
    }

    Watchdog_join(params -> printer -> watchdog);

    Logger_log("PRINTER", "THREAD FUNCTION FINISHED");

    free(params);
    free(converted);

    pthread_exit(NULL);
}

/*
    METHOD: Printer_print
    ARGUMENTS:
        convertedStats - an object of convertedStats
    PURPOSE: print of a given object to the screen
    RETURN: nothing
*/
static void Printer_print(
    ConvertedStats* convertedStats
) {
    Logger_log("PRINTER", "PRINT STARTED");

    if (convertedStats == NULL) { return; }
    
    printf("\033[H\033[J");

    printf("================ TRACKER ================\n\n");

    printf("cpu:   ");

    Printer_toScreen(convertedStats -> percentages_average);

    printf("\n");

    for(uint8_t i = 0; i < convertedStats -> count; i++) {
        printf("cpu%d:  ", i);
        Printer_toScreen(convertedStats -> percentages[i]);
        printf("\n");
    }

    printf("\n");
    printf("================ TRACKER ================\n");

    Logger_log("PRINTER", "PRINT FINISHED");
}

/*
    METHOD: Printer_toScreen
    ARGUMENTS:
        percentage - a percentage value to be visualised on the screen
    PURPOSE: visualisation of a given percentage value on the screen
    RETURN: nothing
*/
static void Printer_toScreen(
    float const percentage
) {
    int progress;

    progress = (int)(percentage / 4.0f);
    
    Logger_log("PRINTER", "TOSCREEN STARTED");
    
    printf("[");

    for(int i = 0; i < progress; i++)
        printf("%s", "#");

    for(int i = progress; i < 25; i++)
        printf(" ");

    printf("] %0.2f%%", (double)percentage);

    Logger_log("PRINTER", "TOSCREEN FINISHED");
}

/*
    METHOD: Printer_free
    ARGUMENTS:
        printer - a printer object to be freed
    PURPOSE: frees reserved memory for a given Printer object and its nested objects
    RETURN: Printer object or NULL in 
        case creation was not possible 
*/
void Printer_destroy(
    Printer* printer
) {
    Logger_log("PRINTER", "DESTROY STARTED");

    if (printer == NULL) { return; }

    Watchdog_destroy(printer -> watchdog);
    Notifier_destroy(printer -> notifier);
    
    free(printer);

    Logger_log("PRINTER", "DESTROY FINISHED");
}

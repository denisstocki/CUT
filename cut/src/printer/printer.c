/*
    AUTHOR: DENIS STOCKI                  
    FILE: printer.c                       
    PURPOSE: implementation of printer module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "printer.h"
#include "../enums/enums.h"
#include "../watchdog/watchdog.h"
#include "../notifier/notifier.h"

struct printer {
    Watchdog* watchdog;
    Notifier* notifier;
    Buffer* bufferAP;
    pthread_t thread;
    long proc;
    bool thread_started;
    char padding[7];
};

// STRUCTURE FOR HOLDING PARAMS PASSED TO READER THREAD FUNCTION
typedef struct ThreadParams {
    Printer* printer;
    volatile sig_atomic_t* status;
} ThreadParams;

static void* Printer_threadf(void* args);
static void Printer_print(ConvertedStats*);
static void Printer_toScreen(float);

/*
    METHOD: Printer_init
    PURPOSE: creation of Printer 'object'
    RETURN: Printer 'object' or NULL in 
        case creation was not possible 
*/
Printer* Printer_init(
    Buffer* bufferAP,
    long proc
) {
    Watchdog* watchdog;
    Notifier* notifier;
    printf("[PRINTER]: INIT STARTED\n");

    if (bufferAP == NULL || proc <= 0) { return NULL; }
    
    Printer* printer = (Printer*) malloc(sizeof(Printer));

    if (printer == NULL) { return NULL; }

    notifier = Notifier_init();

    if (notifier == NULL) { return NULL; }

    watchdog = Watchdog_init(notifier, "READER");

    if (watchdog == NULL) { return NULL; }
    
    *printer = (Printer) {
        .watchdog = watchdog,
        .notifier = notifier,
        .bufferAP = bufferAP,
        .proc = proc,
        .thread_started = false
    };

    printf("[PRINTER]: INIT FINISHED\n");

    return printer;
}

int Printer_start(
    Printer* const printer,
    volatile sig_atomic_t* status
) {
    printf("[PRINTER]: START STARTED\n");

    if (
        printer == NULL ||
        *status != RUNNING
    ) { return ERR_PARAMS; }

    ThreadParams* params = (ThreadParams*) malloc(sizeof(ThreadParams));

    if (params == NULL) { return ERR_ALLOC; }

    *params = (ThreadParams) {
        .printer = printer,
        .status = status
    };

    if (pthread_create(&(printer -> thread), NULL, Printer_threadf, (void*) params) != 0) {
        return ERR_CREATE;
    }

    printer -> thread_started = true;

    printf("[PRINTER]: START FINISHED\n");

    return SUCCESS;
}

int Printer_join(
    Printer* const printer
) {
    printf("[PRINTER]: JOIN STARTED\n");

    if (printer == NULL) { return ERR_PARAMS; }
    if (printer -> thread_started == false) { return ERR_PARAMS; }
    if (pthread_join(printer -> thread, NULL) != 0) {
        return ERR_JOIN;
    }

    printf("[PRINTER]: JOIN FINISHED\n");

    return SUCCESS;
}

static void* Printer_threadf(
    void* args
) {
    printf("[PRINTER]: THREAD FUNCTION STARTED\n");

    ThreadParams* params = (ThreadParams*)args;
    ConvertedStats* converted = malloc(sizeof(ConvertedStats) + sizeof(float) * (unsigned long) params -> printer -> proc);
    struct timespec sleepTime;

    if (converted == NULL) {
        pthread_exit(NULL);
    } 

    Watchdog_start(params -> printer -> watchdog, params -> status);

    while (*(params -> status) == RUNNING) {
        if (Buffer_pop(params -> printer -> bufferAP, converted) != SUCCESS) {
            free(converted);
            break;
        }

        Notifier_notify(params -> printer -> notifier);

        Printer_print(converted);
        
        free(converted -> percentages);
        
        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;

        nanosleep(&sleepTime, NULL);
    }

    Watchdog_join(params -> printer -> watchdog);

    printf("[ANALYZER]: THREAD FUNCTION FINISHED\n");

    free(params);
    free(converted);

    pthread_exit(NULL);
}

static void Printer_print(
    ConvertedStats* convertedStats
) {
    if (convertedStats == NULL) { return; }
    
    printf("\033[H\033[J");

    printf("### CPU ULTRA TRACKING ###\n");

    printf("total: ");

    Printer_toScreen(convertedStats -> average_percentage);

    printf("\n");

    for(int i = 0; i < convertedStats -> count; i++) {
        printf("cpu %d: ", i);
        Printer_toScreen(convertedStats -> percentages[i]);
        printf("\n");
    }

    printf("\n");
}

static void Printer_toScreen(
    float percentage
) {
    int progress = (int)(percentage / 10.0f);
    
    printf("[");

    for(int i = 0; i < progress; i++)
        printf("%s", "*");

    for(int i = progress; i < 10; i++)
        printf(" ");

    printf("] %0.2f%%", (double)percentage);
}

/*
    METHOD: Printer_free
    PURPOSE: frees reserved memory for a given Printer 'object' and its nested 'objects'
    RETURN: Printer 'object' or NULL in 
        case creation was not possible 
*/
void Printer_destroy(
    Printer* printer
) {
    printf("[PRINTER]: DESTROY STARTED\n");

    if (printer == NULL) { return; }

    Watchdog_destroy(printer -> watchdog);
    Notifier_destroy(printer -> notifier);
    
    free(printer);

    printf("[PRINTER]: DESTROY FINISHED\n");
}

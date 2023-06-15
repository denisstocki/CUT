/*
    AUTHOR: DENIS STOCKI                  
    FILE: reader.c                       
    PURPOSE: implementation of reader module
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "reader.h"

#define BASENAME "READER"
#define PATH "/proc/stat"

struct reader {
    FILE* file;
    pthread_t thread;
    volatile sig_atomic_t* trackerStatus;
    long countCores;
};

/*
    METHOD: Reader_init
    PURPOSE: creation of Reader 'object'
    RETURN: Reader 'object' or NULL in 
        case creation was not possible 
*/
Reader* Reader_init(
    long countCores
) {
    if (countCores <= 0) { return NULL; }
    
    Reader* reader;
    FILE* file = fopen(PATH, "r");

    if (file == NULL) { return NULL; }

    reader = malloc(sizeof(Reader));
    
    if (reader == NULL) { 
        fclose(file);
        return NULL; 
    }

    *reader = (Reader) { 
        .file = file,
        .countCores = countCores
    };

    return reader;
}

/*
    METHOD: Reader_read
    PURPOSE: reads all required data from a saved file field
    RETURN: Stats 'object' including necessary data or null
*/
ProcessorStats* Reader_read(
    Reader* const reader
) {
    if (reader == NULL) { return NULL; }
    
    ProcessorStats* processorStats;
    CoreStats* coreStats;
    char* line;
    int counter = 0;

    rewind(reader -> file);

    if (fgets(line, sizeof(line), reader -> file) == NULL) { return NULL; }

    processorStats = (ProcessorStats*) malloc(sizeof(ProcessorStats));
    coreStats = (CoreStats*) malloc(sizeof(CoreStats));
    processorStats -> cores = malloc(sizeof(CoreStats) * reader -> countCores);
    processorStats -> count = 0;

    sscanf(
        line, 
        "%s %d %d %d %d %d %d %d %d",
        &(coreStats -> name),        
        &(coreStats -> user),        
        &(coreStats -> nice),        
        &(coreStats -> system),        
        &(coreStats -> idle),        
        &(coreStats -> iowait),        
        &(coreStats -> irq),        
        &(coreStats -> sortirq),        
        &(coreStats -> steal)        
    );

    processorStats -> average = *coreStats;

    while (processorStats -> count < reader -> countCores) {
        if (fgets(line, sizeof(line), reader -> file) != NULL) {
            coreStats = (CoreStats*) malloc(sizeof(CoreStats));
            sscanf(
                line, 
                "%s %d %d %d %d %d %d %d %d",
                &(coreStats -> name),        
                &(coreStats -> user),        
                &(coreStats -> nice),        
                &(coreStats -> system),        
                &(coreStats -> idle),        
                &(coreStats -> iowait),        
                &(coreStats -> irq),        
                &(coreStats -> sortirq),        
                &(coreStats -> steal)        
            );
            processorStats -> cores[processorStats -> count] = *coreStats;
        } else {
            return NULL;
        }
    }
    
    return processorStats;
}

/*
    METHOD: Reader_free
    PURPOSE: frees reserved memory for a given Reader 'object' and its nested 'objects'
    RETURN: Reader 'object' or NULL in 
        case creation was not possible 
*/
void Reader_free(
    Reader* const reader
) {
    if (reader == NULL) { return; }

    fclose(reader -> file);
    free(reader);
}

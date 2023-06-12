/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.c                       
    PURPOSE: implementation of analyzer module
*/

#include <stdio.h>
#include <stdlib.h>
#include "analyzer.h"

struct analyzer {
    int value;
};

/*
    METHOD: Analyzer_init
    PURPOSE: creation of Analyzer 'object'
    RETURN: Analyzer 'object' or NULL in 
        case creation was not possible 
*/
Analyzer* Analyzer_init(
    void
) {
    Analyzer* analyzer = (Analyzer*) malloc(sizeof(Analyzer*));
    analyzer -> value = 0;

    return analyzer;
}

/*
    METHOD: Analyzer_free
    PURPOSE: frees reserved memory for a given Analyzer 'object' and its nested 'objects'
    RETURN: Analyzer 'object' or NULL in 
        case creation was not possible 
*/
void Analyzer_free(
    Analyzer* analyzer
) {
    analyzer -> value = 0;
    free(analyzer);
}

/*
    AUTHOR: DENIS STOCKI                  
    FILE: printer.c                       
    PURPOSE: implementation of printer module
*/

#include <stdio.h>
#include <stdlib.h>
#include "printer.h"

#define BASENAME "PRINTER"

struct printer {
    Logger* logger;
    int value;
    char padding[4];
};

/*
    METHOD: Printer_init
    PURPOSE: creation of Printer 'object'
    RETURN: Printer 'object' or NULL in 
        case creation was not possible 
*/
Printer* Printer_init(
    Logger* logger
) {
    Printer* printer = (Printer*) malloc(sizeof(Printer));
    printer -> logger = logger;
    printer -> value = 0;

    return printer;
}

/*
    METHOD: Printer_free
    PURPOSE: frees reserved memory for a given Printer 'object' and its nested 'objects'
    RETURN: Printer 'object' or NULL in 
        case creation was not possible 
*/
void Printer_free(
    Printer* printer
) {
    printer -> value = 0;
    free(printer);
}

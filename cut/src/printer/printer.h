/*
    AUTHOR: DENIS STOCKI                  
    FILE: printer.h                       
    PURPOSE: interface for printer module 
*/

#ifndef PRINTER_H
#define PRINTER_H

#include "../logger/logger.h"

// Usage of pseudo objective 
// encapsulation on struct printer.
typedef struct printer Printer;

Printer* Printer_init(Logger* logger);
void Printer_free(Printer* printer);

#endif 

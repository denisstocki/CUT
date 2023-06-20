/*
    AUTHOR: DENIS STOCKI                  
    FILE: printer.h                       
    PURPOSE: interface for printer module 
*/

#ifndef PRINTER_H
#define PRINTER_H

// OUTSIDE LIBRARIES
#include <signal.h>

// INSIDE LIBRARIES
#include "../buffer/buffer.h"

// Usage of pseudo objective 
// encapsulation on struct printer.
typedef struct printer Printer;

Printer* Printer_init(Buffer* const, long const);
int Printer_start(Printer* const, volatile sig_atomic_t*);
int Printer_join(Printer* const);
void Printer_destroy(Printer* const);

#endif 

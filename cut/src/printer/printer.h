/*
    AUTHOR: DENIS STOCKI                  
    FILE: printer.h                       
    PURPOSE: interface for printer module 
*/

#ifndef PRINTER_H
#define PRINTER_H

// INCLUDES OF OUTSIDE LIBRARIES
#include <signal.h>
#include <stdatomic.h>

// INCLUDES OF INSIDE LIBRARIES
#include "../buffer/buffer.h"

// ENCAPSULATION ON PRINTER OBJECT
typedef struct printer Printer;

// DECLARATIONS OF PROTOTYPE FUNCTIONS
Printer* Printer_init(Buffer* const, uint8_t const);
int Printer_start(Printer* const, volatile sig_atomic_t*, atomic_flag*);
int Printer_join(Printer* const);
void Printer_destroy(Printer*);

#endif 

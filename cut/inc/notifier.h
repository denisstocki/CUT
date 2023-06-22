/*
    AUTHOR: DENIS STOCKI                  
    FILE: notifier.h                       
    PURPOSE: interface for notifier module 
*/

#ifndef NOTIFIER_H
#define NOTIFIER_H

// ENCAPSULATION ON NOTIFIER OBJECT
typedef struct notifier Notifier;

// DECLARATIONS OF OUTSIDE PROTOTYPES
Notifier* Notifier_init(void);
int Notifier_notify(Notifier* const);
int Notifier_check(Notifier* const, bool*);
void Notifier_destroy(Notifier* const);

#endif 

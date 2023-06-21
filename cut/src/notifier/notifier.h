/*
    AUTHOR: DENIS STOCKI                  
    FILE: notifier.h                       
    PURPOSE: interface for notifier module 
*/

#ifndef NOTIFIER_H
#define NOTIFIER_H

typedef struct notifier Notifier;

Notifier* Notifier_init(void);
int Notifier_notify(Notifier*);
int Notifier_check(Notifier*, bool*);
void Notifier_destroy(Notifier*);

#endif 

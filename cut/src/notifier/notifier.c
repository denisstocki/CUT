/*
    AUTHOR: DENIS STOCKI                  
    FILE: notifier.c                       
    PURPOSE: implementation of notifier module
*/

#include <pthread.h>    
#include <stdlib.h>     
#include <string.h>     
#include <stdbool.h>     
#include <time.h>       
#include <sys/time.h>   
#include "../enums/enums.h"
#include "notifier.h"

struct notifier {
    pthread_cond_t can_notify;
    pthread_cond_t can_check;
    pthread_mutex_t mutex;
    bool notified;
    char padding[7];
};

Notifier* Notifier_init(

) {
    Notifier* notifier;

    notifier = (Notifier*) malloc(sizeof(Notifier));

    if (notifier == NULL) { return NULL; }
    
    *notifier = (Notifier) {
        .can_check = PTHREAD_COND_INITIALIZER,
        .can_notify = PTHREAD_COND_INITIALIZER,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .notified = false
    };

    return notifier;
}

int Notifier_notify(
    Notifier* notifier
) {
    if (notifier == NULL) { return ERR_PARAMS; }
    
    pthread_mutex_lock(&(notifier -> mutex));

    notifier -> notified = true;

    pthread_mutex_unlock(&(notifier -> mutex));

    return SUCCESS;
}

int Notifier_check(
    Notifier* notifier,
    bool* notified
) {
    if (notifier == NULL || notified == NULL) { return ERR_PARAMS; }
    
    pthread_mutex_lock(&(notifier -> mutex));

    *notified = notifier -> notified;

    notifier -> notified = false;
    

    pthread_mutex_unlock(&(notifier -> mutex));

    return SUCCESS;
}

void Notifier_destroy(
    Notifier* notifier
) {
    if (notifier == NULL) { return ; }
    
    pthread_cond_destroy(&(notifier -> can_check));
    pthread_cond_destroy(&(notifier -> can_notify));
    pthread_mutex_destroy(&(notifier -> mutex));

    free(notifier);
}

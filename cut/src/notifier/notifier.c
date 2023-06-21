/*
    AUTHOR: DENIS STOCKI                  
    FILE: notifier.c                       
    PURPOSE: implementation of notifier module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <pthread.h>    
#include <stdlib.h>     
#include <string.h>     
#include <stdbool.h>     
#include <time.h>       
#include <sys/time.h>   

// INCLUDES OF INSIDE LIBRARIES
#include "../enums/enums.h"
#include "notifier.h"

// STRUCTURE FOR HOLDING NOTIFIER OBJECT
struct notifier {
    pthread_cond_t can_notify;
    pthread_cond_t can_check;
    pthread_mutex_t mutex;
    bool notified;
    char padding[7];
};

/*
    METHOD: Notifier_init
    ARGUMENTS: none
    PURPOSE: creation of Notifier object
    RETURN: Notifier object or NULL in 
        case creation was not possible 
*/
Notifier* Notifier_init(
    void
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

/*
    METHOD: Notifier_notify
    ARGUMENTS: 
        notifier - an object which notified field will be changed on
    PURPOSE: thread safe change of notfied field to true
    RETURN: enums integer value
*/
int Notifier_notify(
    Notifier* const notifier
) {
    if (notifier == NULL) { return ERR_PARAMS; }
    
    pthread_mutex_lock(&(notifier -> mutex));

    notifier -> notified = true;

    pthread_mutex_unlock(&(notifier -> mutex));

    return SUCCESS;
}

/*
    METHOD: Notifier_check
    ARGUMENTS: 
        notifier - an object which notified field will be checked on
        notfied - a pointer to a variable which the result will be saved into
    PURPOSE: save of notified value to a given pointer
    RETURN: enums integer value
*/
int Notifier_check(
    Notifier* const notifier,
    bool* notified
) {
    if (notifier == NULL || notified == NULL) { return ERR_PARAMS; }
    
    pthread_mutex_lock(&(notifier -> mutex));

    *notified = notifier -> notified;

    notifier -> notified = false;
    

    pthread_mutex_unlock(&(notifier -> mutex));

    return SUCCESS;
}

/*
    METHOD: Notifier_destroy
    ARGUMENTS: 
        notifier - an object which memory will be freed
    PURPOSE: free of a given object
    RETURN: nothing
*/
void Notifier_destroy(
    Notifier* const notifier
) {
    if (notifier == NULL) { return ; }
    
    pthread_cond_destroy(&(notifier -> can_check));
    pthread_cond_destroy(&(notifier -> can_notify));
    pthread_mutex_destroy(&(notifier -> mutex));

    free(notifier);
}

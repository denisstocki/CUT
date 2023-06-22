/*
    AUTHOR: DENIS STOCKI                  
    FILE: notifier_test.c                       
    PURPOSE: testing notifier module 
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdio.h>      
#include <assert.h>     
#include <stdbool.h> 

// INCLUDES OF INSIDE LIBRARIES
#include "notifier_test.h"
#include "../inc/notifier.h"

/*
    METHOD: test_notifier
    ARGUMENTS: none
    PURPOSE: testing notifier notify and check functions
    RETURN: nothing
*/
void test_notifier(
    void
) {
    printf("Starting notifier test...\n");

    Notifier* notifier = Notifier_init();

    bool result = false;

    Notifier_notify(notifier);
    Notifier_check(notifier, &result);

    assert(result == true);
    printf("Notification test success...\n");

    Notifier_check(notifier, &result);

    assert(result == false);
    printf("Check variable set test success...\n");

    printf("Notifier test finished !\n");

    Notifier_destroy(notifier);
}

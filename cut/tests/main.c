/*
    AUTHOR: DENIS STOCKI                  
    FILE: main.c                       
    PURPOSE: testing notifier module
*/

// INCLUDES OF OUTSIDE LIBRARIES
#include <stdio.h>     

// INCLUDES OF INSIDE LIBRARIES
#include "notifier_test.h"

/*
    METHOD: main
    ARGUMENTS: none
    PURPOSE: invocation of behaviour expected from Notifier tests
    RETURN: an integer number describing correction 
        of this function's execution
*/
int main(
    void
) {

    test_notifier();

    return 0;
}
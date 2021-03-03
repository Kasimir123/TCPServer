#include <stdio.h> 
#include <stdlib.h> 

/**
 * Passes error message to perror and then exits program with 
 *      exit failure.
 * @param str Error message to send on perror before exit
 */ 
void error(char* str)
{
    perror(str);
    exit(EXIT_FAILURE); 
}
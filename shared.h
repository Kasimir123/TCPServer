#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>

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

/**
 * Gets the time in string format
 * @return returns time in string format
 */ 
char* getTime()
{
    time_t t = time(NULL);
    char* timeStr = asctime((struct tm *)localtime(&t));
    timeStr[strlen(timeStr)-1] = 0;
    return timeStr;
}

/**
 * Gets the file name from path
 * @param filePath file path
 * @return returns the file name from file path
 */
char* getFileName(const char* filePath)
{
    char* fileName = strchr(filePath, '/');
    fileName++;
    return fileName;
}

/**
 * Concatenates two strings together
 * @param s1 string one
 * @param s2 string two
 * @return concatenated string
 */
char* concat(const char *s1, const char *s2)
{
    // gets first string length
    const size_t len1 = strlen(s1);

    // gets second string length
    const size_t len2 = strlen(s2);

    // mallocs space for two strings plus null terminal
    char *result = malloc(len1 + len2 + 1); 
    
    // copy the first string
    memcpy(result, s1, len1);

    // copy second string and null terminator
    memcpy(result + len1, s2, len2 + 1); 

    // return result
    return result;
}
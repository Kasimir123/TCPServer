#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "./constants.h"
#include "./shared.h"

#include <arpa/inet.h>
#include <sys/stat.h>

// Functional Values
#define ERRORVAL -1
#define SOCKETBACKLOG 3

// Output constants
// Set to 1 to output to terminal, 0 to output to specified file
#define OUTPUTTOCOMMANDLINE 0
#define OUTPUTPATH "bin/"
#define LOGFILE "log.txt"

/**
 * Fills passed char pointer with ip address for address structure that was passed
 * @param ip char pointer to be populated
 * @param address address structure
 */
void getSender(char *ip, struct sockaddr_in address)
{
    struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&address;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    // Turns address into string format
    inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
}

/**
 * Logs a text message
 * @param len length of buffer to write
 * @param buffer buffer to print out
 * @param address address of the sender
 */
void logText(unsigned int len, char *buffer, struct sockaddr_in address)
{
    // Defines the ip char buffer
    char sendIp[INET_ADDRSTRLEN];

    // Gets the sender IP Address
    getSender(sendIp, address);

// Prints out message
#if OUTPUTTOCOMMANDLINE == 1
    fprintf(stdout, ": TEXT : %s : %s : Length: %d : Message: %s\n\n", sendIp, getTime(), len, buffer);
#else
    // Gets log file string
    char *logFile = concat(OUTPUTPATH, LOGFILE);

    // Opens file to write
    FILE *fd = fopen(logFile, "a");

    // Logs text
    fprintf(fd, ": TEXT : %s : %s : Length: %d : Message: %s\n\n", sendIp, getTime(), len, buffer);
    
    // Closes file
    fclose(fd);

    // Frees log file string
    free(logFile);

#endif
}

/**
 * Logs the file
 * @param nameLength length of file name
 * @param fileName file name string
 * @param dataLen length of file data
 * @param data file data
 * @param address socket address used for parsing incoming ip
 */
void logFile(unsigned int nameLength, char *fileName, unsigned long dataLen, char *data, struct sockaddr_in address)
{
    // Defines the ip char buffer
    char sendIp[INET_ADDRSTRLEN];

    // Gets the sender IP Address
    getSender(sendIp, address);

    // Gets the time the file was sent over (not exact but close enough)
    char *timeStr = getTime();

// Prints out message
#if OUTPUTTOCOMMANDLINE == 1
    fprintf(stdout, ": FILE : %s : %s : Length: %lu : File Name: %s\n\n", sendIp, timeStr, dataLen, fileName);
#else

    // Creates log file string
    char *logFile = concat(OUTPUTPATH, LOGFILE);

    // Opens log file
    FILE *fd = fopen(logFile, "a");

    // Writes file information
    fprintf(fd, ": FILE : %s : %s : Length: %lu : File Name: %s\n\n", sendIp, timeStr, dataLen, fileName);
    
    // Close log file
    fclose(fd);

    // Frees log file string
    free(logFile);

#endif

    // Gets temporary path to ip folder
    char *temp = concat(OUTPUTPATH, sendIp);

    // Initializes stat structure
    struct stat st;

    // If the folder does not exist then make it
    if (stat(temp, &st) == -1)
        mkdir(temp, 664);

    // Gets directory with / at the end
    char *ipDir = concat(temp, "/");

    // Frees the first string
    free(temp);

    // Gets real name of the file, current time plus filename
    char *realName = concat(timeStr, fileName);

    // Gets the fulle file path
    char *filePath = concat(ipDir, realName);

    // Opens the file
    FILE *fp = fopen(filePath, "wb");

    // Writes the file data
    fwrite(data, sizeof(char), sizeof(char) * dataLen, fp);

    // Closes the file
    fclose(fp);

    // Frees all remaining strings
    free(filePath);
    free(ipDir);
    free(realName);
}

/**
 * Reads the specified number of bytes into the provided buffer
 *      from the given socket.
 * @param socket socket to read from
 * @param len length of bytes to read
 * @param buffer buffer to read into
 */
void readBytes(int socket, unsigned int len, void *buffer)
{
    // declares and initializes values
    int result, numRead = 0;

    // read loop
    while (numRead < len)
    {
        // reads as many bytes as possible as stores bytes read
        //      into result
        result = read(socket, buffer + numRead, len - numRead);

        // if result is less than 1 then error out
        if (result < 1)
            error(SOCKETREADFAILED);

        // adds the number just read to the total number read
        numRead += result;
    }
}

/**
 * Handles each connection
 * @param sock socket for this connection
 */
void handleConnection(int sock, struct sockaddr_in address)
{

    // declare command char
    char command;

    // read command character
    readBytes(sock, sizeof(command), (void *)(&command));

    // if text flag
    if (command == 't')
    {
        // Need to update this data structure so that client and server always match
        unsigned int dataLen;

        // Reads the first segment from the TCP header, data length
        readBytes(sock, sizeof(dataLen), (void *)(&dataLen));

        // mallocs enough space for the buffer
        char *buffer = malloc(dataLen);

        // reads the data we were sent
        readBytes(sock, dataLen, (void *)buffer);

        // Logs the message
        logText(dataLen, buffer, address);

        // frees the buffer
        free(buffer);
    }
    // else if file flag
    else if (command == 'f')
    {
        // initializes unsigned int for name length
        unsigned int nameLen;

        // reads length of file name
        readBytes(sock, sizeof(nameLen), (void *)(&nameLen));

        // mallocs space for the filename
        char *fileName = malloc(nameLen);

        // Reads the name of the file
        readBytes(sock, nameLen, (void *)fileName);

        // initializes unsigned long for file data
        unsigned long dataLen;

        // Reads the first segment from the TCP header, data length
        readBytes(sock, sizeof(dataLen), (void *)(&dataLen));

        // mallocs enough space for the buffer
        char *data = malloc(dataLen);

        // reads the data we were sent
        readBytes(sock, dataLen, (void *)data);

        // Logs the message
        logFile(nameLen, fileName, dataLen, data, address);

        // frees the file name
        free(fileName);

        // frees the file data
        free(data);
    }
}

/**
 * Main function
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 * @return returns the exit code
 */
int main(int argc, char const *argv[])
{
    // Initialize all values
    int server, pid, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // attempts to create socket
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        error(SOCKETCREATIONFAILED);

// sets socket options, If an apple device then do not use REUSEPORT
#ifdef __APPLE__
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        error(SOCKETOPTIONSFAILED);
#else
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        error(SOCKETOPTIONSFAILED);
#endif

    // configures address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // binds socket to address
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0)
        error(SOCKETBINDFAILED);

    // sets socket up to listen with specified back log
    if (listen(server, SOCKETBACKLOG) < 0)
        error(SOCKETLISTENFAILED);

    // main server loop
    while (true)
    {
        // Try accepting a new connection, will block until a connection comes in, if accepting failed then exit
        if ((newSocket = accept(server, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1)
            exit(EXIT_FAILURE);

        // If fork was unsuccessful then error
        if ((pid = fork()) < 0)
            error(FORKFAILED);

        // If child process
        if (pid == 0)
        {
            // closes the server
            close(server);

            // handles the connection
            handleConnection(newSocket, address);
        }
        // else if parent process
        else
        {
            // closes the new socket since we don't need it
            close(newSocket);
        }
    }

    // exits, currently exit is not an option
    printf("Exit\n");

    // Return
    return 0;
}
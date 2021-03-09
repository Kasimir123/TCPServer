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

// Functional Values
#define ERRORVAL -1
#define SOCKETBACKLOG 3

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
    fprintf(stdout, ": TEXT : %s : Length: %d : Message: %s\n", sendIp, len, buffer);
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
    // Need to update this data structure so that client and server always match
    unsigned int dataLen = 0;

    // declare command char
    char command;

    // read command character
    readBytes(sock, sizeof(command), (void *)(&command));

    // if text flag
    if (!strcmp(&command, "t"))
    {
        // Reads the first segment from the TCP header, data length
        readBytes(sock, sizeof(dataLen), (void *)(&dataLen));

        // mallocs enough space for the buffer
        char *buffer = malloc(dataLen);

        // reads the data we were sent
        readBytes(sock, dataLen, (void *)buffer);

        logText(dataLen, buffer, address);

        // frees the buffer
        free(buffer);
    }
    // else if file flag
    else if (!strcmp(&command, "f"))
    {

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
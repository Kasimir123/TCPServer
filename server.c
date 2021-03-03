#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <stdbool.h>
#include <stdint.h>
#include "./constants.h"
#include "./shared.h"


// Functional Values
#define ERRORVAL -1
#define SOCKETBACKLOG 3


/**
 * Reads the specified number of bytes into the provided buffer
 *      from the given socket.
 * @param socket socket to read from
 * @param len length of bytes to read
 * @param buffer buffer to read into
 */ 
void readBytes(int socket, unsigned int len, void* buffer)
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
        if (result < 1) error(SOCKETREADFAILED);

        // adds the number just read to the total number read
        numRead += result;
    }   
}

/**
 * Handles each connection
 * @param sock socket for this connection
 */ 
void handleConnection(int sock)
{
    // Need to update this so that client and server always match
    unsigned int dataLen = 0;

    // Reads the first segment from the TCP header, data length
    readBytes(sock, sizeof(dataLen), (void*)(&dataLen));

    printf("Length: %d\n", dataLen);

    // mallocs enough space for the buffer
    char* buffer = malloc(dataLen);

    // reads the data we were sent
    readBytes(sock, dataLen, (void*)buffer);

    printf("Result: %s\n", buffer);

    // frees the buffer
    free(buffer);
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
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) == 0) error(SOCKETCREATIONFAILED);
       
    // sets socket options, If an apple device then do not use REUSEPORT
    #ifdef __APPLE__
        if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) error(SOCKETOPTIONSFAILED);
    #else
        if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) error(SOCKETOPTIONSFAILED);
    #endif

    // configures address
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // binds socket to address
    if (bind(server, (struct sockaddr *)&address, sizeof(address))<0) error(SOCKETBINDFAILED);

    // sets socket up to listen with specified back log
    if (listen(server, SOCKETBACKLOG) < 0) error(SOCKETLISTENFAILED);

    // main server loop
    while (true)
    {
        // Try accepting a new connection, will block until a connection comes in, if accepting failed then exit
        if ((newSocket = accept(server, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1) exit(EXIT_FAILURE); 

        // If fork was unsuccessful then error
        if ((pid = fork()) < 0) error(FORKFAILED);
        
        // If child process
        if (pid == 0)
        {
            // closes the server
            close(server);
            
            // handles the connection
            handleConnection(newSocket);
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
    return 0; 
} 
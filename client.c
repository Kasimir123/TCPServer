#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdbool.h>
#include "./constants.h"
#include "./shared.h"

/**
 * Prints client usage and then exists
 */ 
void usage()
{
    printf("Client Commands\n");
    printf("    -t <text>           Send text to the server\n");
    printf("    -f <file-path>      Send file to the server\n");
    exit(EXIT_FAILURE);
}

/**
 * Main function
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 * @return returns program exit code
 */ 
int main(int argc, char const *argv[]) 
{ 
    // Initializes values
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    
    // Attempts to create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) error(SOCKETCREATIONFAILED);
   
    // Sets up server address
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr)<=0) error(INVALIDADDRESS);
   
    // attempt to connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) error(SOCKETCONNECTIONFAILED);

    // If not enough arguments then print usage
    if (argc < 3) usage();
    // Else if the first argument is the text flag go here
    else if (!strcmp(argv[1], TEXTFLAG))
    {
        // initializes data length
        unsigned int dataLen = 0;

        // Gets length of all inputs
        for (int i = 2; i < argc; i++) dataLen += strlen(argv[i]);

        // If no data then print usage and exit
        if (dataLen == 0) usage();

        // Add how many spaces are needed
        dataLen += argc - 3;

        // malloc space for the message
        char* message = malloc(dataLen);

        // strcat the message and spaces from arguments to the message
        for (int i = 2; i < argc; i++) 
        {
            strcat(message, argv[i]);
            if (i < argc - 1) strcat(message, " ");
        }

        // Sends which command is being processed
        send(sock, "t", 1, 0);

        // Writes the data length to the socket
        write(sock, &dataLen, sizeof(dataLen));

        // Sends the message
        send(sock, message, dataLen, 0);

        // Frees message data
        free(message);

    }
    // Else if the first argument is the file flag go here
    else if (!strcmp(argv[1], FILEFLAG))
    {
        // If too many/little arguments then print usage and exit
        if (argc != 3) usage();

        // initializes file descriptor
        FILE *fd;

        // Opens file to read
        fd = fopen(argv[2], "rb");

        // If file could not be opened then error
        if (fd == NULL) error(FILEOPENFAILED);

        // Seek the end of the file
        fseek(fd, 0, SEEK_END);

        // Put end of file into unsigned long
        unsigned long fileSize = ftell(fd);
        
        // rewinds the file
        rewind(fd);

        // mallocs space for all of the file data
        char *data = (char*)malloc(fileSize);

        // sends file flag
        send(sock, "f", 1, 0);

        // Gets length of the file name
        unsigned int nameLength = strlen(getFileName(argv[2]));

        // send length of file name
        write(sock, &nameLength, sizeof(nameLength));

        // send file name
        send(sock, getFileName(argv[2]), nameLength, 0);

        // send size of file
        write(sock, &fileSize, sizeof(fileSize));

        // read in file data
        fread(data, 1, fileSize, fd);

        // send file data
        send(sock, data, fileSize, 0);

        // close file
        fclose(fd);

    }
    // Else print usage
    else usage();

    // Returns 0
    return 0; 
} 
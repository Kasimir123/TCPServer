#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdbool.h>
#include "./constants.h"
#include "./shared.h"

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

    unsigned int len = strlen(argv[1]);
    write(sock, &len, sizeof(len));
    send(sock, argv[1], strlen(argv[1]), 0);


    return 0; 
} 
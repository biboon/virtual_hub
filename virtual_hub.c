/** Main file of virtual hub **/

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "libnet.h"


/* Main function */
int main(int argc, char *argv[]) {
    /* Analyzing otpions */
    if (argc != 2){
        fprintf(stderr, "Syntax: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    /* Server initialization */
    char* port = argv[1]; // Port used to connect
    
    #ifdef DEBUG
        fprintf(stdout, "Port: %s\n", port);
    #endif
    
    /* Socket description */
    int sock = serverInitialization(port, MAX_CONNEXIONS);
    if (sock < 0) { perror("virtual_hub.serverInitialization"); exit(EXIT_FAILURE); }
    serverLoop(sock);
    return 0;
}

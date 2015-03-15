/** Main file of virtual client **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "libnet.h"


/* Main function */
int main(int argc, char *argv[]){
    /* Analyzing options */
    if (argc != 4) {
        fprintf(stderr, "Syntax: %s <server> <port> <interface name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *serveur = argv[1];
    char *port = argv[2];
    char *iface_name = argv[3];
    #ifdef DEBUG
        fprintf(stdout, "HUB at %s port %s iface %s\n", serveur, port, iface_name);
    #endif

    /* Connection to server */
    int sock = serverConnection(serveur, port);
    if (sock < 0){ fprintf(stderr, "Failed to connect to server\n"); exit(EXIT_FAILURE); }

    /* Opening network interface */
    int iface = virtualInterfaceCreation(iface_name);

    /* Communication with server */
    clientLoop(sock, iface);

    /* Closing connection */
    shutdown(sock, SHUT_RDWR);
    return 0;
}

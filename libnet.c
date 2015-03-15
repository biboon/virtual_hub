/* This file contains network functions. */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "libnet.h"

/** Socket managing functions **/
/* Socket to client */
void socketToClient(int s, char **hote, char **service) {
    struct sockaddr_storage adresse;
    socklen_t taille = sizeof adresse;
    int status;

    /* Gets address of remote socket */
    status = getpeername(s, (struct sockaddr *)&adresse, &taille);
    if (status < 0){ perror("socketToName.getpeername"); exit(EXIT_FAILURE); }

    /* Gets machine name */
    *hote = malloc(BUFSIZE);
    if (*hote == NULL) { perror("socketToClient.malloc"); exit(EXIT_FAILURE); }
    *service = malloc(BUFSIZE);
    if (*service == NULL) { perror("socketToClient.malloc"); exit(EXIT_FAILURE); }
    getnameinfo((struct sockaddr *)&adresse, sizeof adresse, *hote, BUFSIZE, *service, BUFSIZE, 0);
}


/* Server connection */
int serverConnection(char *hote, char *service) {
    struct addrinfo precisions, *resultat;
    int status;
    int s;

    /* Creation de l'adresse de socket */
    memset(&precisions, 0, sizeof precisions);
    precisions.ai_family = AF_UNSPEC;
    precisions.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(hote, service, &precisions, &resultat);
    if (status < 0) { perror("serverConnection.getaddrinfo"); exit(EXIT_FAILURE); }

    /* Creation d'une socket */
    s = socket(resultat->ai_family, resultat->ai_socktype, resultat->ai_protocol);
    if (s < 0) { perror("serverConnection.socket"); exit(EXIT_FAILURE); }

    /* Connection de la socket a l'hote */
    status = connect(s, resultat->ai_addr, resultat->ai_addrlen);
    if (status < 0) exit(EXIT_FAILURE);

    /* Liberation de la structure d'informations */
    freeaddrinfo(resultat);

    return s;
}


/* Server initialization */
int serverInitialization(char *service, int connexions) {
    struct addrinfo precisions, *resultat;
    int status;
    int s;

    /* Building address structure */
    memset(&precisions, 0, sizeof precisions);
    precisions.ai_family = AF_UNSPEC;
    precisions.ai_socktype = SOCK_STREAM;
    precisions.ai_flags = AI_PASSIVE;
    status = getaddrinfo(NULL, service, &precisions, &resultat);
    if (status < 0){ perror("serverInitialization.getaddrinfo"); exit(EXIT_FAILURE); }

    /* Creating socket */
    s = socket(resultat->ai_family, resultat->ai_socktype, resultat->ai_protocol);
    if (s < 0){ perror("serverInitialization.socket"); exit(EXIT_FAILURE); }

    /* Useful options */
    int vrai = 1;
    status = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &vrai, sizeof(vrai));
    if (status < 0) {
        perror("serverInitialization.setsockopt (REUSEADDR)");
        exit(EXIT_FAILURE);
    }
    status = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &vrai, sizeof(vrai));
    if (status < 0){
        perror("serverInitialization.setsockopt (NODELAY)");
        exit(EXIT_FAILURE);
    }

    /* Socket address specification */
    status = bind(s, resultat->ai_addr, resultat->ai_addrlen);
    if (status < 0) exit(EXIT_FAILURE);

    /* Freeing informations structure */
    freeaddrinfo(resultat);

    /* Size of waiting queue */
    status = listen(s, connexions);
    if (status < 0) exit(EXIT_FAILURE);

    return s;
}


/** Main functions **/
/* Server's client managing function */
int serverLoop(int ecoute) {
    // Initialize the poll structure and variables
    struct pollfd poll_tab[MAX_CONNEXIONS + 1];
    memset(poll_tab, 0, sizeof(poll_tab));
    int n_clients = 0, timeout = -1; // milliseconds
    int new_fd, status, i, j;
    
    // Set up the initial listening socket
    poll_tab[0].fd = ecoute;
    poll_tab[0].events = POLLIN;
    
    while (1) {
        status = poll(poll_tab, n_clients + 1, timeout);
        if (status < 0) { perror("serverLoop.poll"); exit(EXIT_FAILURE); }
        else if (status == 0) { perror("serverLoop.poll.timeout"); exit(EXIT_FAILURE); }
        
        // New client connecting
        if ((poll_tab[0].revents & POLLIN) != 0) {
            new_fd = accept(ecoute, NULL, NULL);
            if (n_clients < MAX_CONNEXIONS) {
                n_clients++;
                poll_tab[n_clients].fd = new_fd;
                poll_tab[n_clients].events = POLLIN;
                #ifdef DEBUG
                    fprintf(stderr, "New client on socket %d. New number: %d\n", new_fd, n_clients);
                #endif
            }
        }
        
        // Client sending packet
        for (i = 1; i <= n_clients; i++) {
            if ((poll_tab[i].revents & POLLIN) != 0) { // If event detected
                uint16_t packet_length;
                status = read_fixed(poll_tab[i].fd, (unsigned char *) &packet_length, sizeof(packet_length));
                if (status <= 0) { // Client error, closing
                    close(poll_tab[i].fd);
                    for (j = i; j < n_clients; j++) poll_tab[j] = poll_tab[j + 1];
                    n_clients--;
                    #ifdef DEBUG
                        fprintf(stderr, "Client %d disconnected. Number remaining: %d\n", i, n_clients);
                    #endif
                    i--;
                } else {
                    unsigned char packet[BUFSIZE];
                    int hlength = ntohs(packet_length);
                    #ifdef DEBUG
                        fprintf(stderr, "Packet from client %d, length: %d\n", i, hlength);
                    #endif
                    read_fixed(poll_tab[i].fd, packet, hlength);
                    for (j = 1; j <= n_clients; j++)
                        if (j != i) { // Resending to other clients
                            #ifdef DEBUG
                                fprintf(stderr, "Sending packet to client %d (descriptor: %d)\n", j, poll_tab[j].fd);
                            #endif
                            if (write(poll_tab[j].fd, &packet_length, sizeof(packet_length)) == sizeof(packet_length))
                                write(poll_tab[j].fd, packet, hlength);
                        }
                }
            }
        }
    }
    return 0;
}

/* Client server communication */
int clientLoop(int sock, int iface) {
    // Variables
    int status;
    // Initializing structure
    struct pollfd desc[2];
    memset(desc, -1, sizeof(desc));
    desc[0].fd = sock;
    desc[0].events = POLLIN;
    desc[1].fd = iface;
    desc[1].events = POLLIN;
    
    while (1) {
        int nb = poll(desc, 2, -1);
        if (nb < 0) { perror("clientLoop.poll"); exit(EXIT_FAILURE); }
        
        if ((desc[0].revents & POLLIN) != 0) { // Receiving packet from hub
            unsigned char packet[BUFSIZE];
            uint16_t packet_length;
            status = read_fixed(sock, (unsigned char *) &packet_length, sizeof(packet_length));
            if (status <= 0) { fprintf(stderr, "Server not responding !\n"); exit(EXIT_FAILURE); }
            int hlength = ntohs(packet_length);
            status = read_fixed(sock, packet, hlength);
            #ifdef DEBUG
                fprintf(stderr,"Packet of size %d received from HUB\n", hlength);
            #endif
            if (status != hlength) fprintf(stderr, "Wrong size packet received !\n");
            else if (write(iface, packet, hlength) != hlength) {
                fprintf(stderr, "Failed to write on interface !\n");
                exit(EXIT_FAILURE);
            }
        }
        
        if ((desc[1].revents & POLLIN) != 0) { // receiving packet from interface
            unsigned char packet[BUFSIZE];
            int hlength = read(iface, packet, BUFSIZE);
            if (hlength <= 0) { fprintf(stderr, "Interface not responding !\n"); exit(EXIT_FAILURE); }
            #ifdef DEBUG
                fprintf(stderr,"Packet of size %d received from interface\n", hlength);
            #endif
            uint16_t packet_length = htons(hlength);
            if (write(sock, &packet_length, sizeof(packet_length)) != sizeof(packet_length)) {
                fprintf(stderr,"Failed to write on server !\n");
                exit(EXIT_FAILURE);
            }
            if (write(sock, packet, hlength) != hlength) {
                fprintf(stderr,"Failed to write on server !\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}

/* Reads bytes array of a precise size */
int read_fixed(int descripteur, unsigned char *array, int size) {
    int bytes = 0;
    while (bytes < size) {
        int offset = read(descripteur, array + bytes, size - bytes);
        if (offset > 0) bytes += offset; else return -1;
    }
    return bytes;
}

/** Virtual interfaces managing functions **/
/* Opens a new virtual ethernet interface */
int virtualInterfaceCreation(char *nom)
{
    struct ifreq interface;
    int fd, erreur;

    /* opens principal device */
    if((fd = open(TAP_PRINCIPAL, O_RDWR)) < 0) return fd;

    /* Preparation of structure */
    memset(&interface, 0, sizeof(interface));
    interface.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (nom != NULL) strncpy(interface.ifr_name, nom, IFNAMSIZ);

    /* Creating interface */
    if((erreur = ioctl(fd, TUNSETIFF, (void *)&interface)) < 0){ close(fd); return erreur; }

    /* Get interface name */
    if (nom != NULL) strcpy(nom, interface.ifr_name);

    return fd;
}

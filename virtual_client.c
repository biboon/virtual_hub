/**** Fichier principal pour le commutateur virtuel ****/
/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "libnet.h"

/** Quelques constantes **/

/** Variables globales */

/** Fonctions **/

/* Fonction principale */
int main(int argc, char *argv[]){
    // Analyse des arguments
    if (argc != 4) {
        fprintf(stderr, "Syntaxe: %s <serveur> <port> <nom interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *serveur = argv[1];
    char *port = argv[2];
    char *iface_name = argv[3];
    #ifdef DEBUG
        fprintf(stdout, "HUB sur %s port %s iface %s\n", serveur, port, iface_name);
    #endif

    // Connexion au serveur
    int sock = connexionServeur(serveur, port);
    if (sock < 0){ fprintf(stderr, "Erreur de connexion au serveur\n"); exit(EXIT_FAILURE); }

    // Ouverture de l'interface reseau
    int iface = creationInterfaceVirtuelle(iface_name);

    // Communication avec le serveur
    clientLoop(sock, iface);

    // On termine la connexion
    shutdown(sock, SHUT_RDWR);
    return 0;
}

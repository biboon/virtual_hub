/**** Fichier principal pour le repeteur virtuel ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "libnet.h"

/** Quelques constantes **/

/** Variables globales **/

/** Fonctions **/

/* Fonction principale */
int main(int argc, char *argv[]){
    // Analyse des arguments
    if (argc != 2){
        fprintf(stderr, "Syntax: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Initialisation du serveur
    char* port = argv[1]; //Port de connexion
    
    #ifdef DEBUG
        fprintf(stdout, "Port: %s\n", port);
    #endif

    int s; //Descripteur de la SOCKET
    s = initialisationServeur(port, MAX_CONNEXIONS);
    if (s < 0) { perror("virtual_hub.initialisationServeur"); exit(EXIT_FAILURE); }
    boucleServeur(s, gestionClient);
    return 0;
}

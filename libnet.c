/** fichier libnet.c **/

/************************************************************/
/** Ce fichier contient des fonctions reseau.                            **/
/************************************************************/

/**** Fichiers d'inclusion ****/

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "libnet.h"

/**** Constantes ****/
#define MAX_TAMPON 1024

/**** Variables globales *****/

/**** Fonctions de gestion des sockets ****/
//Socket Vers client
void socketVersClient(int s, char **hote, char **service) {
    struct sockaddr_storage adresse;
    socklen_t taille = sizeof adresse;
    int statut;

    /* Recupere l'adresse de la socket distante */
    statut = getpeername(s, (struct sockaddr *)&adresse, &taille);
    if (statut < 0){ perror("socketVersNom.getpeername"); exit(EXIT_FAILURE); }

    /* Recupere le nom de la machine */
    *hote = malloc(MAX_TAMPON);
    if (*hote == NULL) { perror("socketVersClient.malloc"); exit(EXIT_FAILURE); }
    *service = malloc(MAX_TAMPON);
    if (*service == NULL) { perror("socketVersClient.malloc"); exit(EXIT_FAILURE); }
    getnameinfo((struct sockaddr *)&adresse, sizeof adresse, *hote,MAX_TAMPON, *service, MAX_TAMPON, 0);
}


//Connexion seveur
int connexionServeur(char *hote, char *service) {
    struct addrinfo precisions, *resultat;
    int statut;
    int s;

    /* Creation de l'adresse de socket */
    memset(&precisions, 0, sizeof precisions);
    precisions.ai_family = AF_UNSPEC;
    precisions.ai_socktype = SOCK_STREAM;
    statut = getaddrinfo(hote, service, &precisions, &resultat);
    if (statut < 0){ perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }

    /* Creation d'une socket */
    s = socket(resultat->ai_family, resultat->ai_socktype, resultat->ai_protocol);
    if (s < 0){ perror("connexionServeur.socket"); exit(EXIT_FAILURE); }

    /* Connection de la socket a l'hote */
    statut = connect(s, resultat->ai_addr, resultat->ai_addrlen);
    if (statut < 0) exit(EXIT_FAILURE);

    /* Liberation de la structure d'informations */
    freeaddrinfo(resultat);

    return s;
}


//Initialisation serveur
int initialisationServeur(char *service, int connexions) {
    struct addrinfo precisions, *resultat;
    int statut;
    int s;

    /* Construction de la structure adresse */
    memset(&precisions, 0, sizeof precisions);
    precisions.ai_family = AF_UNSPEC;
    precisions.ai_socktype = SOCK_STREAM;
    precisions.ai_flags = AI_PASSIVE;
    statut = getaddrinfo(NULL, service, &precisions, &resultat);
    if (statut < 0){ perror("initialisationServeur.getaddrinfo"); exit(EXIT_FAILURE); }

    /* Creation d'une socket */
    s = socket(resultat->ai_family, resultat->ai_socktype, resultat->ai_protocol);
    if (s < 0){ perror("initialisationServeur.socket"); exit(EXIT_FAILURE); }

    /* Options utiles */
    int vrai = 1;
    statut = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &vrai, sizeof(vrai));
    if (statut < 0) {
        perror("initialisationServeur.setsockopt (REUSEADDR)");
        exit(EXIT_FAILURE);
    }
    statut = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &vrai, sizeof(vrai));
    if (statut < 0){
        perror("initialisationServeur.setsockopt (NODELAY)");
        exit(EXIT_FAILURE);
    }

    /* Specification de l'adresse de la socket */
    statut = bind(s, resultat->ai_addr, resultat->ai_addrlen);
    if (statut < 0) exit(EXIT_FAILURE);

    /* Liberation de la structure d'informations */
    freeaddrinfo(resultat);

    /* Taille de la queue d'attente */
    statut = listen(s, connexions);
    if (statut < 0) exit(EXIT_FAILURE);

    return s;
}

//Gestion des clients
int boucleServeur(int ecoute, int (*traitement)(int))
{
    int dialogue;
    while (1) {
        /* Attente d'une connexion */
        if((dialogue = accept(ecoute, NULL, NULL)) < 0) exit(EXIT_FAILURE);
        /* Passage de la socket de dialogue a la fonction de traitement */
        if (traitement(dialogue) < 0) { close(ecoute); return 0; }
    }
}

int gestionClient(int s)
{
    /* Obtient une structure de fichier */
    FILE *dialogue = fdopen(s, "a+");
    if (dialogue == NULL) { perror("gestionClient.fdopen"); exit(EXIT_FAILURE); }

    /* Echo */
    char ligne[MAX_LIGNE];
    while(fgets(ligne, MAX_LIGNE, dialogue) != NULL)
        fprintf(dialogue, "> %s", ligne);

    /* Termine la connexion */
    fclose(dialogue);
    return 0;
}

/**** Fonctions de gestion des interfaces virtuelles ****/

/** Ouverture d'une interface Ethernet virtuelle **/

int creationInterfaceVirtuelle(char *nom)
{
    struct ifreq interface;
    int fd, erreur;

    /* Ouverture du peripherique principal */
    if((fd = open(TAP_PRINCIPAL, O_RDWR)) < 0) return fd;

    /* Preparation de la structure */
    memset(&interface, 0, sizeof(interface));
    interface.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (nom != NULL) strncpy(interface.ifr_name, nom, IFNAMSIZ);

    /* Creation de l'interface */
    if((erreur = ioctl(fd, TUNSETIFF, (void *)&interface)) < 0){ close(fd); return erreur; }

    /* Recuperation du nom de l'interface */
    if (nom != NULL) strcpy(nom, interface.ifr_name);

    return fd;
}

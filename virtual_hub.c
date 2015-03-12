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
int main(int argc,char *argv[]){
  // Initialisation du serveur
  char* port=argv[1]; //Port de connexion

  // Analyse des arguments
  #ifdef DEBUG
    fprintf(stdout,"Port : %s\n",port);
  #endif
  if(argc!=2){
    fprintf(stderr,"Syntaxe : switch <port>\n");
    exit(-1);
  }

  int s;     //Descripteur de la SOCKET
  s = initialisationServeur(port, MAX_CONNEXIONS);
  boucleServeur(s,gestionClient);
  return 0;
}

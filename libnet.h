/* This file contains descriptions of network functions */

/** Constants **/
#define MAX_CONNEXIONS 32
#define BUFSIZE 2048
#define TAP_PRINCIPAL "/dev/net/tun"

/** Functions **/
void socketToClient(int s, char **hote, char **service);
int serverConnection(char *hote, char *service);
int serverInitialization(char *service, int connexions);
int read_fixed(int descripteur, unsigned char *array, int size);
int virtualInterfaceCreation(char *nom);
int serverLoop(int ecoute);
int clientLoop(int sock, int iface);

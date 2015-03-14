/* Ce fichier decrit les structures et les constantes utilisees
 * par les fonctions reseau.
 */

/** Constantes **/
#define MAX_CONNEXIONS 32
#define BUFSIZE 2048

#define TAP_PRINCIPAL	"/dev/net/tun"

/** Fonctions **/

void socketVersClient(int s, char **hote, char **service);
int connexionServeur(char *hote, char *service);
int initialisationServeur(char *service, int connexions);
int read_fixed(int descripteur, unsigned char *array, int size);
int creationInterfaceVirtuelle(char *nom);
int serverLoop(int ecoute);
int clientLoop(int sock, int iface);

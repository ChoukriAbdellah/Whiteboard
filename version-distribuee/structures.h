#ifndef _STRUCTURES_
#define _STRUCTURES_

#include <sys/sem.h>

#define PORT 8070
#define SERVER_IP "127.0.0.1"
#define TAILLE_MAX 2014

// Nombre de zones max
#define NB_ZONES_MAX 10

// Le fichier utilisé pour les sémaphores
#define FICHIER_SEMAPHORES "./semaphores"

struct sembuf opp;
struct sembuf opv;

// Le fichier utilisé pour le segment de mémoire partagée
#define FICHIER_PARTAGE "./segment-memoire"

// L'entier commun pour calculer la clé utilisée pour le segment de mémoire partagée
#define CLE_PARTAGE 35

// L'entier commun pour calculer la clé utilisée pour le tableau de sémaphores
#define CLE_SEMAPHORES 35


typedef struct{ 
    int sockfd;
    int numZone;
} maj_struct_client;


/* Structures à partager dans le segment */

typedef struct{ 
    int numeroZone;
    char titre[TAILLE_MAX];
    char createurs[TAILLE_MAX];
    char texte[TAILLE_MAX];
} zone;

typedef struct {
    zone zones[NB_ZONES_MAX];
} principale;
typedef struct{ 
    int sockfd;
    principale* memoire;
    int index ;
} maj_struct_serveur;

typedef struct{
    principale* p;
    int index ;
} data;

/* Structure utiles aux sémaphores */

// Union
union semun {
    int val; /* cmd = SETVAL */
    struct semid_ds *buf; /* cmd = IPC_STAT ou IPC_SET */
    unsigned short  *array;  /* cmd = GETALL ou SETALL */
    struct seminfo *__buf ;/* cmd = IPC_INFO (sous Linux) */
};

// Operations
struct sembuf op[] = {
    { 0, -1, SEM_UNDO }, // P
    { 0, 1, SEM_UNDO }, // V
    {0, 0, SEM_UNDO} // Z
};  

 
/* Ou sinon : */

/* Opération P */

/*struct sembuf opp;
opp.sem_num = 0;
opp.sem_op = -1;
opp.sem_flg = SEM_UNDO; */

/* Opération V  */

/*struct sembuf opv;
opv.sem_num = 0;
opv.sem_op = +1;
opv.sem_flg = SEM_UNDO;  */

/* Opération Z  */

/*struct sembuf opz;
opv.sem_num = 0;
opv.sem_op = 0;
opv.sem_flg = SEM_UNDO;  */

#endif

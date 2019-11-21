#ifndef _STRUCTURES_
#define _STRUCTURES_

#include <sys/sem.h>

// Le fichier utilisé pour les sémaphores
#define FICHIER_SEMAPHORES "./semaphores"

// Le fichier utilisé en tant que segment partagé
#define FICHIER_PARTAGE "./segment-memoire"

// L'entier commun pour calculer la clé
#define ENTIER_CLE 35

// L'entier commun pour calculer la clé
#define CLE_SEMAPHORES 35

// Nombre de zones max
#define NB_ZONES_MAX 10

/* Union */

union semun {
    int val; /* cmd = SETVAL */
    struct semid_ds *buf; /* cmd = IPC_STAT ou IPC_SET */
    unsigned short  *array;  /* cmd = GETALL ou SETALL */
    struct seminfo *__buf ;/* cmd = IPC_INFO (sous Linux) */
};

typedef struct{ 
    int numeroZone;
    char* titre;
    char* createur;
    char* texte;
} zone;

typedef struct {
    zone zones[10];
} principale;

/* Structure des opérations */

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

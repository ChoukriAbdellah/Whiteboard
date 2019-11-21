#include <sys/types.h> // Pour key_t
#include <sys/ipc.h> // Pour segments mémoires
#include <sys/sem.h> // Pour sémaphores 
#include <fcntl.h>// Pour open(), O_CREAT O_WRONLY
#include <stdio.h> //Pour printf()
#include <sys/shm.h> // Pour shmget(), shmat(), shmdt(), shmctl()
#include <errno.h> // Pour errno
#include <unistd.h> // Pour close(fd)
#include <stdlib.h> // Pour exit(), NULL

#include "structures.h"

int main(int argc, char **argv){

    if(argc != 2) {
        printf("Utilisation : ./serveur <nbZones>\n");
        exit(0);
    }

    int nbZones = atoi(argv[1]);

    /* Création du fichier s'il n'existe pas */
    int fd = open(FICHIER_PARTAGE, O_CREAT|O_WRONLY, 0644);
    close(fd);
  
    // On calcule notre clé
    key_t cle;
    if ((cle = ftok(FICHIER_PARTAGE, ENTIER_CLE)) == (key_t) -1){
        perror("Erreur ftok ");
        exit(EXIT_FAILURE);
    }

    int idS=0;

    principale p;

    for(int i=0; i<10; i++){
        zone z;
        z.numeroZone = i;
        p.zones[i] = z;
    }

    if ((idS=shmget(cle, sizeof(principale), IPC_CREAT | 0666)) < 0){
        if(errno == EEXIST)
             fprintf(stderr, "Le segment de memoire partagee (cle=%d) existe deja\n", cle);
        else
            perror("Erreur shmget ");
        exit(EXIT_FAILURE);
    } 

    printf("Serveur: Segment de mémoire partagée attaché.\n");

    int * shmaddr;
    if ((shmaddr = (int *) shmat(idS, NULL, 0)) == (void *) -1){
        perror("Erreur shmat ");
        exit(EXIT_FAILURE);
    }
    
    //*shmaddr = nbZones;

    //printf("Serveur: Segment prêt avec %d zones.\n" , *shmaddr);

    //Detachement du segment de memoire partagee */
    if((shmdt(shmaddr)) < 0) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }

    printf("Serveur: Segment détaché.\n");

    

    return 0;
}
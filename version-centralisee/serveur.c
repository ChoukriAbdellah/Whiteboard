#include <sys/types.h> // Pour key_t
#include <sys/ipc.h> // Pour segments mémoires
#include <sys/sem.h> // Pour sémaphores 
#include <fcntl.h>// Pour open(), O_CREAT O_WRONLY
#include <stdio.h> //Pour printf()
#include <sys/shm.h> // Pour shmget(), shmat(), shmdt(), shmctl()
#include <errno.h> // Pour errno
#include <unistd.h> // Pour close(fd)
#include <stdlib.h> // Pour exit(), NULL
#include <sys/socket.h> 
#include<stdio.h>
#include<arpa/inet.h> //inet_addr
#include <string.h> 
#include <netdb.h> 
#include <netinet/in.h> 

#include "structures.h"


void afficheZone( principale *p){
    for(int i=0; i<NB_ZONES_MAX; i++){
printf("---------- Zone %d ---------\n", p->zones[i].numeroZone);
printf("\t Données : %s \n  %s \n\n",p->zones[i].titre, p->zones[i].texte);
    
printf("\tRéalisé par :%s\n", p->zones[i].createurs);
    }
}
void initSemaphores(char* fichier_semaphores, int  cle_semaphore) {
  
  //Création du fichier s'il n'existe pas
  int fd = open(fichier_semaphores, O_CREAT|O_WRONLY, 0644);
  close(fd);
  
  key_t cle = ftok(fichier_semaphores, cle_semaphore);

    // Création de NB_ZONE_MAX sémaphore associée a la clé
    int idSem = semget(cle, NB_ZONES_MAX, IPC_CREAT|0666);

    // Initialisation des sémaphore a 1
    union semun egCtrl;
    egCtrl.val=1;

    for(int i=0; i<NB_ZONES_MAX; i++){
        if(semctl(idSem, i, SETVAL, egCtrl) == -1) {
            perror("Probleme semctl fonction initStructure");
        }
    }  

}


void initZones(char* fichier_zones, int entier_cle) {
    //Création du fichier s'il n'existe pas
    int fd = open(fichier_zones, O_CREAT|O_WRONLY, 0644);
    close(fd);
    
    key_t cle = ftok(fichier_zones, entier_cle);
     if (cle  == (key_t) -1){
        perror("Erreur ftok ");
        exit(EXIT_FAILURE);
    }

    int idS=0;

    principale p;

    for(int i=0; i<NB_ZONES_MAX; i++){
        zone z;
        z.numeroZone = i;
        strcpy(z.createurs , "créateur");
        strcpy(z.texte,"Ceci est un texte");
        strcpy(z.titre,"un titre");
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

    principale * shmaddr;
    if ((shmaddr = (principale *) shmat(idS, NULL, 0)) == (void *) -1){
        perror("Erreur shmat ");
        exit(EXIT_FAILURE);
    }
    
    *shmaddr = p;
  

   /* for(int i=0; i<NB_ZONES_MAX; i++){
    printf("Segment partagé zone %d : %d\n", i, shmaddr->zones[i].numeroZone);
    printf("createur : \t%s\n", shmaddr->zones[i].createur);
    }
    //printf("Serveur: Segment prêt avec %d zones.\n" , *shmaddr);

    //Detachement du segment de memoire partagee */
    afficheZone(shmaddr);
    if((shmdt(shmaddr)) < 0) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }

    printf("Serveur: Segment détaché.\n");


}
/*
void  initSockaddr(struct sockaddr_in* server, struct sockaddr_in* client, int numPort, char* adr){
    int socket_desc ;
    //Création du socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
         printf("La création du socket a échoué\n");
    }
    puts("Socket créé\n");
     
    //Préparation de la structure sockaddr_in 
    server->sin_family = AF_INET; // Définition d'un adresse sur 4 octets
    server->sin_addr.s_addr =inet_addr(adr);
    server->sin_port = htons(numPort);
}

*/
int main(int argc, char **argv){

    if(argc != 1) {
        printf("Utilisation : ./serveur <Port>\t<Adresse>\t \n");
        printf("<Port>: un numèro de port accèssible\n");
        printf("<Adresse>: une adresse de type IPV4\n");

        exit(1);
    }

    initSemaphores(FICHIER_SEMAPHORES, CLE_SEMAPHORES);
    initZones(FICHIER_PARTAGE, CLE_PARTAGE);

    

   
    

    return 0;
}
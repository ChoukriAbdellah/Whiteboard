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
printf("Affichage des données du segment de mémoire\n");
    sleep(1);
    for(int i=0; i<NB_ZONES_MAX; i++){
printf("---------- Zone %d ---------\n", p->zones[i].numeroZone);
printf("\t Données : %s \n  %s \n\n",p->zones[i].titre, p->zones[i].texte);
    
printf("\tRéalisé par :%s\n", p->zones[i].createur);
sleep(1);
    }
}
void editZone(principale* p, int numZone){
        char newData[1024];
        char  input_string[100];
        printf("Entrer vos modification >");
        //scanf("%s",newData);
        //scanf("%[^\n]",newData);
        fgets(newData, sizeof(newData), stdin);
        printf("%s", newData);
        strcat(p->zones[numZone].texte, " ");
        strcat(p->zones[numZone].texte, newData);
        printf("Modification terminine\n");
}

void editData(principale*p){
      int ac, sup,ok, numZone;
      char action[20];
      char  newData[100];  
        
      printf("Quelle action souhaitez vous réaliser ? \n");
      printf("Modifier\t Supprimer\t\n");
       
      
      scanf("%s",action);
      printf("Sur quelle Zone ?\n");
      scanf("%d", &numZone);
      ac= strcmp(action,"modifier");
       sup= strcmp(action,"supprimer");
       ok=0;
     
     while (ok == 0 )
     {
         if(ac==0 && (numZone >=0 && numZone < 10 ) ){
         editZone(p,numZone);
          ok++;
      }
      else
      {
          if (sup==0 && (numZone >=0 && numZone < 10 ))
          {
              // sup zone pas encore fait je simule avec editZone
          editZone(p,numZone);
          ok++;
          }
          else
          {
            printf("Action non autorisee \n");
            printf("Quelle action souhaitez vous réaliser ? \n");
            printf("Modifier\t Supprimer\t\n");
            scanf("%s",action);
            printf("Sur quelle Zone ?\n");
            scanf("%d", &numZone);
          }

      }
      
     } 
    
    
}
int main(int argc, char **argv){
    char* fichier_zones;
    int entier_cle;
    entier_cle=CLE_PARTAGE;
    int fd = open(FICHIER_PARTAGE, O_CREAT|O_WRONLY, 0644);
    close(fd);
    
    key_t cle = ftok(FICHIER_PARTAGE, entier_cle);
     if (cle  == (key_t) -1){
        perror("Erreur ftok ");
        exit(EXIT_FAILURE);
    }
    int idS=0;
    principale p;
     printf("shmget \n");
     if ((idS=shmget(cle, sizeof(principale),0666)) < 0){
        if(errno == EEXIST)
             fprintf(stderr, "Le segment de memoire partagee (cle=%d) existe deja\n", cle);
        else
            perror("Erreur shmget ");
        exit(EXIT_FAILURE);
    } 
    printf("Récupération de l'adresse \n");
    principale * shmaddr;
   
    if ((shmaddr = (principale *) shmat(idS, NULL, 0)) == (void *) -1){
        perror("Erreur shmat ");
        exit(EXIT_FAILURE);
    }
   
      afficheZone(shmaddr);
      
      // Modification des données si souhaité par l'utilisateur
      //editData(shmaddr);
     // sleep(2);
       editZone(shmaddr, 9);
      afficheZone(shmaddr);
       
     
    if((shmdt(shmaddr)) < 0) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }

    printf("Client: Segment détaché.\n");

    printf("Fin \n");

    return 0;
}
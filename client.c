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

void afficheZone(principale *p){
    printf("Affichage des données du segment de mémoire\n");
    //sleep(1);
    for(int i=0; i<NB_ZONES_MAX; i++){
        printf("---------- Zone n°%d ---------\n", p->zones[i].numeroZone);
        printf("\t Titre : %s \n \n",p->zones[i].titre);
        printf("%s \n \n", p->zones[i].texte);
            
        printf("Auteur(s) : %s \n \n", p->zones[i].createurs);
        //sleep(1);
    }
}

void editZone(principale* p, int numZone){

    char newData[TAILLE_MAX];
    printf("Vous vous apprêtez à modifier la zone %d.\n", numZone);
    printf("Entrez vos modifications > ");

    // Cela permet de récupérer un caractère retour à la ligne si il est resté après un scanf de l'utilisateur
    fgetc(stdin);

    // Maintenant fgets ne récupère pas le retour à la ligne mais la prochaine saisie
    if ((fgets(newData, TAILLE_MAX, stdin)) == NULL){
        perror("fgets : ");
        exit(EXIT_FAILURE);
    }

    
    strcat(p->zones[numZone].texte, " ");
    strcat(p->zones[numZone].texte, newData);

    printf("Vos modifications ont bien été enregistrés!\n");
}
 void removeZone(principale* p, int numZone){
      char newData[TAILLE_MAX];
      int n;
    printf("Vous vous apprêtez à supprimer la zone %d.\n", numZone);
    printf("Etes vous sur de vouloir supprimer le contenu de cette Zone ? > ");
    scanf("%s",newData);
     n=strcmp(newData,"oui");

   if(n == 0){
       strcpy(p->zones[numZone].texte, "");
       printf("Contenu supprimé!\n");
   }
   else
   {
    printf("Contenu non supprimé!\n");
   }
   
 
 }
void editData(principale* p){
 
    int numZone, choixMenu, choixCorrect=0;

    while (!choixCorrect){           
            printf("---------- MENU ---------- \n");
            printf("1. Modifier un document\n");
            printf("2. Supprimer un document \n");
            printf("Tapez 1 ou 2 > ");

            scanf("%d", &choixMenu);

            printf("\nChoisissez la zone > ");

            scanf("%d", &numZone);

            //fflush(stdout);

            if(choixMenu == 1 || choixMenu == 2){
                if(numZone >= 0 && numZone < 10){
                    choixCorrect++;
                }
                else
                    printf("\nNuméro de zone incorrect.\n");  
            }
            else
                printf("\nNuméro du menu incorrect. \n");
    }

    if(choixMenu == 1)
        editZone(p, numZone);
    if(choixMenu == 2)
        removeZone(p, numZone);
        
    /*else
        supZone(p, numZone);


          /*  if(ac==0 && (numZone >=0 && numZone < 10) ){
                printf("numero test: %d\n", numZone);
                editZone(p,numZone);
                ok++;
            }

        else{
            if (sup==0 && (numZone >=0 && numZone < 10 )){
                // sup zone pas encore fait je simule avec editZone
                editZone(p,numZone);
                ok++;
            }
            else{
                printf("Action non autorisee \n");
                printf("Quelle action souhaitez vous réaliser ? \n");
                printf("Modifier\t Supprimer\t\n");
                scanf("%s",action);
                printf("Sur quelle Zone ?\n");
                scanf("%d", &numZone);
            }

        }
      
     } */
    
    
}

int main(int argc, char **argv){
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
      
    /* Modification des données si souhaité par l'utilisateur */
    
    editData(shmaddr);
    //editZone(shmaddr, 9);
    afficheZone(shmaddr);
       
     
    if((shmdt(shmaddr)) < 0) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }

    printf("Client: Segment détaché.\n");

    printf("Fin \n");

    return 0;
}
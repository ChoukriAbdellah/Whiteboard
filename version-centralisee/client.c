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
#include <pthread.h>

#include "structures.h"

void afficheZones(principale *p){
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
void afficheZone(principale *p, int zone){
    printf("\nUne mise à ajour vient d'etre réalisée sur cette zone\n");
    

        printf("---------- Zone n°%d ---------\n", p->zones[zone].numeroZone);
        printf("\t Titre : %s \n \n",p->zones[zone].titre);
        printf("%s \n \n", p->zones[zone].texte);
            
        printf("Auteur(s) : %s \n \n", p->zones[zone].createurs);
       
    
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
 void* MAJ(void* arg){
      data *temp =  arg; 

      int fd = open(FICHIER_SEMAPHORES, O_CREAT|O_WRONLY, 0644);
    close(fd);

    // On calcule notre clé
    key_t cleSem;
    if ( (cleSem = ftok(FICHIER_SEMAPHORES, CLE_SEMAPHORES)) == (key_t) -1){
        perror("Erreur ftok ");
        exit(EXIT_FAILURE);
    }

    int idSem=0;

    if ((idSem=semget(cleSem, 1, 0666)) < 0){
        if(errno == EEXIST)
             fprintf(stderr, "La sémaphore (id=%d) existe deja\n", idSem);
        else
            perror("Erreur semget ");
        exit(EXIT_FAILURE);
    } 

    //printf("Processus n°%d : La sémaphore a bien été créée et attachée.\n", getpid());
	opp.sem_op = -1;
	opp.sem_flg = 0;
	
	opv.sem_op = 1;
	opv.sem_flg = 0;
    

        opp.sem_num = temp->index;
		opv.sem_num = temp->index;
        int attente;
        while ( 1)
        {
          if((attente= semctl(idSem, temp->index, GETVAL)) == -1){ // On récupères le nombre de processus restants
            perror("problème init");//suite
        }
         if (attente == 0)
         {
             while (semctl(idSem, temp->index, GETVAL) == 0)
             {
                 // On attend que le client ait fini sa modification
             }
             
             afficheZone(temp->p,temp->index);
             
         }
        }
        
        
        
     
 }
 
void editData(principale* p){
 
    int numZone, choixMenu, choixCorrect=0;

    int fd = open(FICHIER_SEMAPHORES, O_CREAT|O_WRONLY, 0644);
    close(fd);

    // On calcule notre clé
    key_t cleSem;
    if ( (cleSem = ftok(FICHIER_SEMAPHORES, CLE_SEMAPHORES)) == (key_t) -1){
        perror("Erreur ftok ");
        exit(EXIT_FAILURE);
    }

    int idSem=0;

    if ((idSem=semget(cleSem, 1, 0666)) < 0){
        if(errno == EEXIST)
             fprintf(stderr, "La sémaphore (id=%d) existe deja\n", idSem);
        else
            perror("Erreur semget ");
        exit(EXIT_FAILURE);
    } 

    printf("Processus n°%d : La sémaphore a bien été créée et attachée.\n", getpid());
	opp.sem_op = -1;
	opp.sem_flg = 0;
	
	opv.sem_op = 1;
	opv.sem_flg = 0;

    

    while (!choixCorrect){           
            printf("---------- MENU ---------- \n");
            printf("1. Modifier un document\n");
            printf("2. Supprimer un document \n");
            printf("Tapez 1 ou 2 > ");

            scanf("%d", &choixMenu);

            printf("\nChoisissez la zone > ");

            scanf("%d", &numZone);

            opp.sem_num = numZone;
		    opv.sem_num = numZone;
            int attente;
            if((attente= semctl(idSem, numZone, GETVAL)) == -1){ // On récupères le nombre de processus restants
                perror("problème init");//suite
            }

            if (attente == 0) {
                printf("Cette zone est en cours de modification par un autre client, veuillez patientez ... \n");
            }
		   
		    semop(idSem,&opp,1);

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

    semop(idSem,&opv,1);
        
}

int main(int argc, char **argv){
    int entier_cle;
    int idS=0;
    principale p;
    pthread_t arrayT[NB_ZONES_MAX];

    entier_cle=CLE_PARTAGE;
    int fd = open(FICHIER_PARTAGE, O_CREAT|O_WRONLY, 0644);
    close(fd);
    
    key_t cle = ftok(FICHIER_PARTAGE, entier_cle);
     if (cle  == (key_t) -1){
        perror("Erreur ftok ");
        exit(EXIT_FAILURE);
    }
    
    
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
     for (int i = 0; i < NB_ZONES_MAX; ++i) {
          data* d = malloc(sizeof( data*));
          d->p= shmaddr;
          d->index=i;
         
                                    if(pthread_create(&arrayT[i], NULL, MAJ, d) != 0){
                                        perror("error : pthread ");
                                    }
    }
    afficheZones(shmaddr);
      
    /* Modification des données si souhaité par l'utilisateur */
    
    editData(shmaddr);
    //editZone(shmaddr, 9);
    afficheZones(shmaddr);
    //Affichage des notifications                                
   for (int i = 0; i < NB_ZONES_MAX; ++i) {
        pthread_join(arrayT[i], NULL);
								}
    
    
      
     
    if((shmdt(shmaddr)) < 0) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }

    printf("Client: Segment détaché.\n");

    printf("Fin \n");

    return 0;
}
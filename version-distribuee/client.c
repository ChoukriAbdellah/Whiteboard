#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<string.h>//memset
#include<stdlib.h>//sizeof
#include<netinet/in.h>//INADDR_ANY
#include <unistd.h>// Pour close(fd)
#include <arpa/inet.h>
#include <sys/ipc.h> // Pour segments mémoires
#include <sys/sem.h> // Pour sémaphores 
#include <fcntl.h>// Pour open(), O_CREAT O_WRONLY
#include <sys/shm.h> // Pour shmget(), shmat(), shmdt(), shmctl()
#include <errno.h> // Pour errno
#include <netdb.h> 
#include <netinet/in.h> 

#include "structures.h"
#include "TCP.h"


void afficheZone(zone z){
    printf("---------- Zone n°%d ---------\n", z.numeroZone);
    printf("\t Titre : %s \n \n",z.titre);
    printf("%s \n \n", z.texte);
            
    printf("Auteur(s) : %s \n \n", z.createurs);
}

void afficheZones(principale p){
    for(int i=0; i<NB_ZONES_MAX; i++){
        afficheZone(p.zones[i]);
    }
}

principale receptionEspace(int Socket){

   printf("\nClient: Structure en cours de reception...\n");
    principale p;    

    for(int i=0; i<NB_ZONES_MAX; i++){
        zone reception;
        recvPourTCP((char *) &reception, Socket);
        p.zones[i] = reception;
    }

    printf("Client: Tout a bien été reçu.\n");
    return p;
}


/*void* MAJ(void* arg){
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
 }*/


void editZone(principale p, int numZone, int Socket){
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

    strcat(p.zones[numZone].texte, " ");
    strcat(p.zones[numZone].texte, newData);

    // Le client envoie au serveur la zone après l'avoir modifiée

    printf("affichage zone modifiée chez client avant de la renvoyer \n");
    zone new;
    new = p.zones[numZone];
    afficheZone(new);

    sendPourTCP(sizeof(new), (char *) &new, Socket);

    printf("Vos modifications ont bien été enregistrés!\n");
}

void removeZone(principale p, int numZone, int Socket){
    char newData[TAILLE_MAX];
    int n;
    printf("Vous vous apprêtez à supprimer la zone %d.\n", numZone);
    printf("Etes vous sur de vouloir supprimer le contenu de cette Zone ? > ");
    scanf("%s",newData);
    n=strcmp(newData,"oui");

    if(n == 0){
       strcpy(p.zones[numZone].texte, "");
        // Le client envoie au serveur la zone après l'avoir modifiée
        zone new;
        new = p.zones[numZone];

        sendPourTCP(sizeof(new), (char *) &new, Socket);

       printf("Contenu supprimé!\n");
    }
    else{
    printf("Contenu non supprimé!\n");
    }

}

void menu(int Socket, principale p){
 
    int numZone, choixMenu, choixCorrect=0;


    while (!choixCorrect){           
            printf("---------- MENU ---------- \n");
            printf("1. Modifier un document\n");
            printf("2. Supprimer un document \n");
            printf("Tapez 1 ou 2 > ");

            scanf("%d", &choixMenu);

            printf("\nChoisissez la zone > ");

            scanf("%d", &numZone);

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

    printf("avant send: choixMenu: %d numZone %d\n", choixMenu, numZone);

    // Le client informe au serveur quel zone il souhaite modifier
    //sendPourTCP(sizeof(numZone), (char *)&numZone, Socket);
    send(Socket,&numZone,sizeof(numZone),0);
    
    // Le client reçoit du serveur si la zone est accessible ou non
    int statutZone;
    //recvPourTCP((char *)&statutZone, Socket);
    recv(Socket,&statutZone,sizeof(statutZone),0);

    printf("statut zone: %d\n", statutZone);

    if (statutZone == 0){
        printf("Du serveur: La zone que vous avez choisie n'est pas disponible pour le moment.\n");
        printf("Vous pouvez en choisir une autre.\n");
        menu(Socket, p);
    }
    else{
        if(choixMenu == 1)
            editZone(p, numZone, Socket);
        if(choixMenu == 2)
            removeZone(p, numZone, Socket);
    }

        
}

unsigned char * deserialize_int(unsigned char *buffer, int *value){}

int main(int argc, char** argv){
    int sockfd;//to create socket

    struct sockaddr_in serverAddress;//client will connect on this

    int n;

    //create socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    //initialize the socket addresses
    memset(&serverAddress,0,sizeof(serverAddress));
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.s_addr=inet_addr(SERVER_IP);
    serverAddress.sin_port=htons(PORT);

    //client  connect to server on port
    connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
    //send to sever and receive from server

    principale p = receptionEspace(sockfd);

    while(1){
        afficheZones(p);
        menu(sockfd, p);
        p = receptionEspace(sockfd);
    }

    return 0;

}
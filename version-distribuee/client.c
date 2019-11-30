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
#include <pthread.h>

#include "structures.h"
#include "TCP.h"
#include "affichage.h"

pthread_mutex_t verrou= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t structureRecu= PTHREAD_COND_INITIALIZER;

principale receptionEspace(int Socket){

   printf("\nClient: Structure en cours de reception...\n");
    principale p;    

    for(int i=0; i<NB_ZONES_MAX; i++){
        zone reception;
        //pthread_mutex_lock(&verrou);
        recvPourTCP((char *) &reception, Socket);
        //pthread_mutex_unlock(&verrou);
        p.zones[i] = reception;
    }

    printf("Client: Tout a bien été reçu.\n");
    return p;
}

void editZone(principale p, int numZone, int Socket){
    char newData[TAILLE_MAX];
    printf("Vous vous apprêtez à modifier la zone %d.\n\n", numZone);
    afficheZone(p.zones[numZone]);

    int veutStopModif=0;
    while(!veutStopModif){
        int choixCorrect = 0;
        int choix;
        while (!choixCorrect){     
                printf("\n1. Modifier le titre\n");
                printf("2. Modifier le texte \n");
                printf("3. Terminer la modification\n \n");
                printf("Tapez 1, 2 ou 3 > ");

                scanf("%d", &choix);

                if(choix == 1 || choix == 2 || choix == 3)
                    choixCorrect++;
                else
                    printf("\nChoix incorrect. \n");
        }

        if(choix == 3)
            veutStopModif++;
        
        else{
            printf("Entrez vos modifications > ");

            // Cela permet de récupérer un caractère retour à la ligne si il est resté après un scanf de l'utilisateur
            fgetc(stdin);

            // Maintenant fgets ne récupère pas le retour à la ligne mais la prochaine saisie
            if ((fgets(newData, TAILLE_MAX, stdin)) == NULL){
                perror("fgets : ");
                exit(EXIT_FAILURE);
            }

            char hostname[1024];
            hostname[1023] = '\0';
            gethostname(hostname, 1023);
            //printf("Hostname: %s\n", hostname);
            //struct hostent* h;
            //h = gethostbyname(hostname);
            //printf("h_name: %s\n", h->h_name);

            if(choix == 1){
                // On remplace le titre directement
                strcpy(p.zones[numZone].titre, newData);
                strcpy(p.zones[numZone].lastModif,hostname);
            }
            else if(choix == 2){
                // Ici on ajoute la modification à la suite du texte déjà présent
                strcat(p.zones[numZone].texte, " ");
                strcat(p.zones[numZone].texte, newData);
                strcpy(p.zones[numZone].lastModif, hostname);
            }
        }

    }

    // Le client envoie au serveur la zone après l'avoir modifiée

    //printf("affichage zone modifiée chez client avant de la renvoyer \n");
    zone new;
    new = p.zones[numZone];
    //afficheZone(new);

    sendPourTCP(sizeof(new), (char *) &new, Socket);

    printf("Vos modifications ont bien été enregistrés!\n");
}

void removeZone(principale p, int numZone, int Socket){
    char newData[TAILLE_MAX];
    int n;
    printf("\nVous vous apprêtez à supprimer le document n°%d.\n\n", numZone);
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

int menu(int Socket, principale p){
 
    int numZone, choixMenu, choixCorrect=0, souhaiteQuitter=0;

    afficheZonesLeger(p);

    while (!choixCorrect){           
            printf("\n ---------- MENU ---------- \n\n");
            printf("1. Modifier un document\n");
            printf("2. Supprimer un document \n");
            printf("3. Quitter \n\n");
            printf("Tapez 1, 2 ou 3 > ");

            scanf("%d", &choixMenu);


            if(choixMenu == 1 || choixMenu == 2){
                printf("\nChoisissez lequel (0 à %d) > ", NB_ZONES_MAX-1);

                scanf("%d", &numZone);
                if(numZone >= 0 && numZone < NB_ZONES_MAX){
                    choixCorrect++;
                }
                else
                    printf("\nNuméro de zone incorrect.\n");  
            }
            else if(choixMenu == 3){
                choixCorrect++;
                souhaiteQuitter++;
            }

            else
                printf("\nNuméro du menu incorrect. \n");
    }

    if(!souhaiteQuitter){
        //printf("avant send: choixMenu: %d numZone %d\n", choixMenu, numZone);

        // Le client informe au serveur quel zone il souhaite modifier
        send(Socket,&numZone,sizeof(numZone),0);
        //printf("Je send le num de la zone\n");

        // Le client reçoit du serveur si la zone est accessible ou non
        int statutZone;
        //pthread_mutex_lock(&verrou);
        recv(Socket,&statutZone,sizeof(statutZone),0);
        //pthread_mutex_unlock(&verrou);
        //printf("debug \n");
        //printf("statut zone: %d\n", statutZone);

        if (statutZone == 10){
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
        return 0;
    }
    else{
        printf("Vous avez choisi d'arrêter la connexion au serveur. \n");
        return 1;
    }

}

void* afficheMaj(void* args){
   
    //maj_struct_client* temp = args;
     /* printf("/////////////////MAJ RECU//////////////////// \n");
     pthread_mutex_lock(&verrou);
	 pthread_cond_wait (&structureRecu, &verrou);
     pthread_mutex_unlock(&verrou);*/
    //while(1){
       
      /*  maj m;
        pthread_mutex_lock(&verrou);
        recvPourTCP((char *)&m, temp->sockfd);
        //recv(temp->sockfd,&maj,sizeof(maj),0);
        pthread_mutex_unlock(&verrou);
        printf("(debug)Entier reçu %d \n",m.msg);
        if (m.msg== 500){
            zone reception;
            recvPourTCP((char*) &reception, temp->sockfd);
            printf("[!] Un client vient de modifier la zone n°%d, voici les nouvelles informations : \n", reception.numeroZone);
            printf("Debug !!!!!!!!!\n");
            afficheZone(reception);

        } 

        
        zone reception;
        recvPourTCP((char *) &reception, temp->sockfd);
        temp->memoire.zones[reception.numeroZone] = reception;
        printf("/////////UNE MAJ VIENT D ETRE REALISEE SUR CETTE ZONE %d/////////\n", reception.numeroZone);*/
       
        //afficheZone(reception);
        
    //}*/
    
    pthread_exit(NULL);
}

int main(int argc, char** argv){
    if(argc != 3) {
        printf("Utilisation : ./client <IP> <Port>\t\n");
        printf("<IP> : L'adresse IP du serveur\n");
        printf("<Port> : Le numéro de port\n");

        exit(1);
    }
    int sockfd;//to create socket

    struct sockaddr_in serverAddress;//client will connect on this

    char* SERVER_IP = argv[1]; 
    int PORT = atoi(argv[2]);

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

    //pthread_cond_broadcast(&structureRecu);
    //pthread_t thread1;

    maj_struct_client *args = malloc(sizeof *args);
    args->sockfd = sockfd;
    //args->memoire=p;
    /*if(pthread_create(&thread1, NULL, afficheMaj, args) == -1) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }*/

    //pthread_mutex_lock(&verrou);
    //principale p = receptionEspace(sockfd);
    //pthread_mutex_unlock(&verrou);

    int souhaiteQuitter = 0;
    while(!souhaiteQuitter){
        principale p = receptionEspace(sockfd);
        souhaiteQuitter = menu(sockfd, p);
        //Affichage des mises à jour s'il y'en a :   
    }
    //pthread_join(thread1, NULL);
    return 0;

}
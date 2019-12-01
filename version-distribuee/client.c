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
#include <fcntl.h>// Pour open(), O_CREAT O_WRONLY, <fcntl.h>
#include <sys/shm.h> // Pour shmget(), shmat(), shmdt(), shmctl()
#include <errno.h> // Pour errno
#include <netdb.h> 
#include <netinet/in.h> 
#include <pthread.h> 
#include <sys/select.h>
 
#include "structures.h"
#include "TCP.h"
#include "affichage.h"

pthread_mutex_t verrou= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitThread= PTHREAD_COND_INITIALIZER;

int activerMAJ=0;

void desactiverThreadMAJ(int Socket){
    activerMAJ = 0;
    //printf("je viens de désactiver le thread\n");

    int opts;

    opts = fcntl(Socket,F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = opts & (~O_NONBLOCK);

    if (fcntl(Socket,F_SETFL,opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
    return;
}

void activerThreadMAJ(int Socket){
    activerMAJ = 1;
    //printf("je viens de réactiver le thread\n");
    /*if(fcntl(Socket, F_SETFL, fcntl(Socket, F_GETFL) | O_NONBLOCK) < 0) {
        printf("erreur lors du passage en mode non bloquant");
    }*/
 
	int opts;
 
	if ((opts = fcntl(Socket, F_GETFL)) < 0) {
		perror("fcntl(F_GETFL)");
	}
 
	opts = (opts | O_NONBLOCK);
 
	if (fcntl(Socket, F_SETFL, opts) < 0) {
		perror("fcntl(F_SETFL)");
	}


    pthread_cond_broadcast(&waitThread); // On reveille le thred

}

principale* receptionEspace(int Socket){

   printf("\nClient: Structure en cours de reception...\n");
    principale *p = malloc(sizeof(principale));

    for(int i=0; i<NB_ZONES_MAX; i++){
        zone reception;
        recvPourTCP((char *) &reception, Socket);
        p->zones[i] = reception;
    }
    printf("Client: Tout a bien été reçu.\n");

    return p;
}

void editZone(principale *p, int numZone, int Socket){
    char newData[TAILLE_MAX];
    printf("Vous vous apprêtez à modifier le document n°%d.\n\n", numZone);
    afficheZone(p->zones[numZone]);

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

            // Pour enlever le "\n" à la fin de la chaine saisie : 
            newData[strlen(newData)-1] = '\0';
        
            if(choix == 1){
                // On remplace le titre directement
                strcpy(p->zones[numZone].titre, newData);
                strcpy(p->zones[numZone].lastModif,hostname);
            }
            else if(choix == 2){
                // Ici on ajoute la modification à la suite du texte déjà présent
                strcat(p->zones[numZone].texte, " ");
                strcat(p->zones[numZone].texte, newData);
                strcpy(p->zones[numZone].lastModif, hostname);
            }
        }

    }

    // Le client envoie au serveur la zone après l'avoir modifiée

    //printf("affichage zone modifiée chez client avant de la renvoyer \n");
    zone new;
    new = p->zones[numZone];
    //afficheZone(new);

    sendPourTCP(sizeof(new), (char *) &new, Socket);

    printf("Vos modifications ont bien été enregistrés!\n");
}

void removeZone(principale *p, int numZone, int Socket){
    char newData[TAILLE_MAX];
    int n;
    printf("\nVous vous apprêtez à supprimer le document n°%d.\n\n", numZone);
    printf("Etes vous sur de vouloir supprimer le contenu de cette Zone ? > ");
    scanf("%s",newData);
    n=strcmp(newData,"oui");

    if(n == 0){
       strcpy(p->zones[numZone].texte, "");
        // Le client envoie au serveur la zone après l'avoir modifiée
        zone new;
        new = p->zones[numZone];

        sendPourTCP(sizeof(new), (char *) &new, Socket);

       printf("Contenu supprimé!\n");
    }
    else{
        printf("Contenu non supprimé!\n");
    }

}

int menu(int Socket, principale *p){
    int souhaiteQuitter=0;
    int numZone, choixMenu, choixCorrect=0;

    //afficheZonesLeger(p);

    while (!choixCorrect){           
        printf("\n ---------- MENU ---------- \n\n");
        printf("1. Modifier un document\n");
        printf("2. Réinitialiser un document \n");
        printf("3. Quitter \n\n");
        printf("Tapez 1, 2 ou 3 > ");

        scanf("%d", &choixMenu);


        if(choixMenu == 1 || choixMenu == 2){
            printf("\nChoisissez lequel (0 à %d) > ", NB_ZONES_MAX-1);

            //printf("avant scanf");
            scanf("%d", &numZone);
            //printf("apres scanf");
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
        desactiverThreadMAJ(Socket);

        // Le client informe au serveur quel zone il souhaite modifier
        send(Socket,&numZone,sizeof(numZone),0);

        // On envoie une autorisation au serv pour lui indiquer qu'on est prêt à recevoir
        // Car on vient de bloquer le thread des mises à jour
           
        //int autorisation = 1;
        //send(Socket,&autorisation,sizeof(autorisation),0);

        // Le client reçoit du serveur si la zone est accessible ou non
        int peutModif;
        recv(Socket, &peutModif, sizeof(peutModif), 0);

        activerThreadMAJ(Socket);



        if (peutModif==0){
            printf("Du serveur: La zone que vous avez choisie n'est pas disponible pour le moment.\n");
            printf("Vous pouvez en choisir une autre.\n");
            menu(Socket, p);
        }
        else{
            if(choixMenu == 1){
                editZone(p, numZone, Socket);
            }
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
    maj_struct_client* temp = args;
    //printf("thread client en place activerMAJ = %d\n", activerMAJ);
    while(1){
        pthread_mutex_lock(&verrou);
        while(activerMAJ != 1){
            //printf("thread mis en attente le temps de recv\n");
            pthread_cond_wait(&waitThread, &verrou);
        }
        pthread_mutex_unlock(&verrou);

        // Là le thread est autorisé à recevoir
        if(activerMAJ == 1){
            //printf("Thread prêt à recv\n");
            //int m;
            //pthread_mutex_lock(&verrou);
            // Rend la socket non bloquante au cas où la MAJ ne reçoit rien
            /*if((recv(temp->sockfd, &m, sizeof(m), 0)) < 0){
                printf("recv error: %s\n", strerror(errno));
            }*/

            zone reception;
            int erreur = recvNonBloquant(sizeof(reception), (char *) &reception, temp->sockfd);//recvNonBloquant();
            switch(erreur){  
                case 0:
                    //fprintf(stderr, "Serveur: Erreur reçue du client (IP: , zone n°%d) : send = 0\n", reception.numeroZone);
                    break;
                case -1:
                    //fprintf(stderr, "Serveur: Erreur reçue du client (IP: , zone n°%d) : send = -1\n", reception.numeroZone);
                    break;
                case 1:
                    //printf("Serveur: Envoi de la zone %d au client terminé.\n",reception.numeroZone);
                    break;
            }  
            if(erreur > 0){
                printf("taille reception %ld\n", sizeof(reception));
                printf("numZone %d\n", reception.numeroZone);
                pthread_mutex_lock(&verrou);
                temp->p->zones[reception.numeroZone] = reception;
                pthread_mutex_unlock(&verrou);
                printf("UNE MAJ VIENT D ETRE REALISEE SUR CETTE ZONE %d\n", reception.numeroZone);
                afficheZone(reception);
            }
            //else{printf("rien sa mere\n"); sleep(1);}
          //sleep(3); 
            //printf("(debug)Entier reçu %d après mise à jour (activerMAJ=%d)\n",m, activerMAJ);
        }
  
    }
        
    
    pthread_exit(NULL);
}

int main(int argc, char** argv){
    if(argc != 3) {
        printf("Utilisation : ./client <IP> <Port>\t\n");
        printf("<IP> : L'adresse IP du serveur\n");
        printf("<Port> : Le numéro de port\n");

        exit(1);
    }
    int sockfd;

    struct sockaddr_in serverAddress;//adresse distante

    char* SERVER_IP = argv[1];
    int PORT = atoi(argv[2]);

    //Création de la socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    /*if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(EXIT_FAILURE);
	}*/
    //initialize the socket addresses
    memset(&serverAddress,0,sizeof(serverAddress));
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.s_addr=inet_addr(SERVER_IP);
    serverAddress.sin_port=htons(PORT);

    //client  connect to server on port
    connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
    //send to sever and receive from server

    pthread_t threadMAJ;
    principale *p = malloc(sizeof(principale));
    p = receptionEspace(sockfd);
    afficheZonesLeger(*p);

    maj_struct_client *args = malloc(sizeof *args);
    args->sockfd = sockfd;
    args->p=p;

    if(pthread_create(&threadMAJ, NULL, afficheMaj, args) == -1) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    activerThreadMAJ(sockfd);

    int souhaiteQuitter = 0;
    while(!souhaiteQuitter){
        souhaiteQuitter = menu(sockfd, p);
    }
    pthread_join(threadMAJ, NULL);
    return 0;

}
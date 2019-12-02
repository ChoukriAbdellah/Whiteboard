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
	int opts;
 
	if ((opts = fcntl(Socket, F_GETFL)) < 0) { 
		perror("fcntl(F_GETFL)");
	}
 
	opts = (opts | O_NONBLOCK);
 
	if (fcntl(Socket, F_SETFL, opts) < 0) {
		perror("fcntl(F_SETFL)");
	}

    // On reveille le thred
    pthread_cond_broadcast(&waitThread); 

}

principale* receptionEspace(int Socket){

   printf("\nClient: Structure en cours de reception...\n");
    principale *p = malloc(sizeof(principale));
    int erreurRecvPourTcp=0;
    for(int i=0; i<NB_ZONES_MAX; i++){
        zone reception;
        erreurRecvPourTcp= recvPourTCP((char *) &reception, Socket);
        if (erreurRecvPourTcp == -1)
        {
            perror("Error to sendPourTCP fonction");
            exit(EXIT_FAILURE);
        }
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
                printf(BLU"\n1. Modifier le titre\n");
                printf("2. Modifier le texte \n");
                printf("3. Terminer la modification\n \n");
                printf(WHT"Tapez 1, 2 ou 3 > ");

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

    zone new;
    new = p->zones[numZone];

    int erreurSend =sendPourTCP(sizeof(new), (char *) &new, Socket);
    if (erreurSend == -1)
    {
        perror("Error to sendPourTCP fonction");
        exit(EXIT_FAILURE);
    }
    

    printf(MAG"Vos modifications ont bien été enregistrés!\n");
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

    while (!choixCorrect){           
        printf(RED"\n ---------- MENU ---------- \n\n");
        printf(BLU"1. Modifier un document\n");
        printf("2. Réinitialiser un document \n");
        printf("3. Quitter \n\n");
        printf("Tapez 1, 2 ou 3 > ");

        //scanf("%d", &choixMenu);
        char saisie[255];
        while(1)
        {
            fgets(saisie, 255, stdin);
            if (sscanf(saisie, "%d", &choixMenu) == 1) break;
            printf(RED"Saissisez un nombre...\n"WHT); 
        }

        if(choixMenu == 1 || choixMenu == 2){
            printf("\nChoisissez lequel (0 à %d) > ", NB_ZONES_MAX-1);

            scanf("%d", &numZone);
            if(numZone >= 0 && numZone < NB_ZONES_MAX){
                choixCorrect++;
            
            }
            else
                printf("\nNuméro de zone incorrect.\n");  
        }

        else {
            if(choixMenu == 3){
                choixCorrect++;
                souhaiteQuitter++;
            }

            else
                printf(RED"\nNuméro du menu incorrect. \n");
        }
    }

    if(!souhaiteQuitter){
        desactiverThreadMAJ(Socket);

        // Le client informe au serveur quel zone il souhaite modifier
        int erreur=send(Socket,&numZone,sizeof(numZone),0);
        if (erreur == -1)
        {
            perror("error of send fonction");
            exit(EXIT_FAILURE);
        }

        // Le client reçoit du serveur si la zone est accessible (1 ) ou non (0)
        int peutModif;
        int read_size=0;
        read_size=recv(Socket, &peutModif, sizeof(peutModif), 0);
         
          if(read_size == -1)
            {
                perror("Echec de réception");
                exit(EXIT_FAILURE);
            }

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
    int err=0;
    while(1){
        err = pthread_mutex_lock(&verrou);
        if ( err != 0 ) {
               perror("Error to pthread_mutex_lock fonction");
               exit(EXIT_FAILURE);
        }
        while(activerMAJ != 1){
          err = pthread_cond_wait(&waitThread, &verrou);
           if ( err != 0 )
           {
               perror("Error to pthread_cond_wait fonction");
               exit(EXIT_FAILURE);
           }
           
        }
        err = pthread_mutex_unlock(&verrou);
        if ( err != 0 ) {
               perror("Error to pthread_mutex_unlock fonction");
               exit(EXIT_FAILURE);
        }

        // Là le thread est autorisé à recevoir
        if(activerMAJ == 1){
          

            zone reception;
            
            int erreur = recvNonBloquant(sizeof(reception), (char *) &reception, temp->sockfd);
            switch(erreur){  
                case 0:
                    // fprintf(stderr, "Serveur: Erreur reçue du client (IP: , zone n°%d) : send = 0\n", reception.numeroZone);
                    // exit(EXIT_FAILURE);
                    break;
                case -1:
                    // fprintf(stderr, "Serveur: Erreur reçue du client (IP: , zone n°%d) : send = -1\n", reception.numeroZone);
                    // exit(EXIT_FAILURE);
                    break;
                case 1:
                    //printf("Serveur: Envoi de la zone %d au client terminé.\n",reception.numeroZone);
                    err = pthread_mutex_lock(&verrou);
                    if ( err != 0 ) {
                        perror("Error to pthread_mutex_lock fonction");
                        exit(EXIT_FAILURE);
                    }
                    temp->p->zones[reception.numeroZone] = reception;
                    err = pthread_mutex_unlock(&verrou);
                    if ( err != 0 ) {
                        perror("Error to pthread_mutex_unlock fonction");
                        exit(EXIT_FAILURE);
                    }
                    printf(YEL"UNE MAJ VIENT D'ETRE REALISEE SUR CETTE ZONE %d\n", reception.numeroZone);
                    afficheZone(reception);
                    break;
            }  
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
    int sockfd, er;

    struct sockaddr_in serverAddress;//adresse distante

    char* SERVER_IP = argv[1];
    int PORT = atoi(argv[2]);

    //Création de la socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if (sockfd == -1)
    {
         printf("La création du socket a échoué\n");
    }

    //initialize the socket addresses
    memset(&serverAddress,0,sizeof(serverAddress));
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.s_addr=inet_addr(SERVER_IP);
    serverAddress.sin_port=htons(PORT);

    //client  connect to server on port
    er = connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
    if(er ==-1) {
        perror("Error to connect function");
        exit(EXIT_FAILURE);
    }
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
    er=pthread_join(threadMAJ, NULL);
    if( er != 0 )
    {
        printf( "pthread_join error.\n" );
        exit( 1 );
    }
    return 0;

}
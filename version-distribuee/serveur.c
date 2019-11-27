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

void afficheZone(zone z){
    printf("---------- Zone n°%d ---------\n", z.numeroZone);
    printf("\t Titre : %s \n \n",z.titre);
    printf("%s \n \n", z.texte);
            
    printf("Auteur(s) : %s \n \n", z.createurs);
}

void afficheZones(principale *p){
    for(int i=0; i<NB_ZONES_MAX; i++){
        afficheZone(p->zones[i]);
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

principale * initZones(char* fichier_zones, int entier_cle) {
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
    afficheZones(shmaddr);

    return shmaddr;
}

unsigned char * serialize_int(unsigned char *buffer, int value){
  /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
  buffer[0] = value >> 24;
  buffer[1] = value >> 16;
  buffer[2] = value >> 8;
  buffer[3] = value;
  return buffer + 4;
}

unsigned char * serialize_char(unsigned char *buffer, char value){
  buffer[0] = value;
  return buffer + 1;
}

unsigned char * serialize_temp(unsigned char *buffer,  zone *value){
  buffer = serialize_int(buffer, value->numeroZone);
  for(int i=0; i<strlen(value->titre); i++)
     buffer = serialize_char(buffer, value->titre[i]);

  for(int i=0; i<strlen(value->texte); i++)
     buffer = serialize_char(buffer, value->texte[i]);

  for(int i=0; i<strlen(value->createurs); i++)
     buffer = serialize_char(buffer, value->createurs[i]);
  return buffer;
}

int sendPourTCP(int s, char* msg, int Socket){
    // Printf en commentaires pour debug

    // On fait un envoi d'abord pour indiquer la taille à envoyer : 

    send(Socket,&s,sizeof(s),0);

    int i=0;
    int totalSend = 0;
    //printf("Total à envoyer : %d\n", s);
    while(totalSend <  s){
        int nbSend = 0;
        nbSend = send(Socket,msg+totalSend,s-totalSend,0);
        //printf("Iteration i=%d, nb send = %d\n", i, nbSend);


        if (nbSend == -1){
            return -1;
        }
        else if (nbSend == 0){
            //printf("Total envoyé: %d\n", totalSend);
            return 0;
        }
        else
            totalSend+=nbSend;
        i++;
    }

    //printf("Fin correcte\n");
    return 1;
}

int main(int argc, char** argv){

    if(argc != 1) {
        printf("Utilisation : ./serveur <Port>\t<Adresse>\t \n");
        printf("<Port>: un numèro de port accèssible\n");
        printf("<Adresse>: une adresse de type IPV4\n");

        exit(1);
    }

    initSemaphores(FICHIER_SEMAPHORES, CLE_SEMAPHORES);
    principale* p = initZones(FICHIER_PARTAGE, CLE_PARTAGE);

    int sockfd;//to create socket
    int newsockfd;//to accept connection

    struct sockaddr_in serverAddress;//server receive on this address
    struct sockaddr_in clientAddress;//server sends to client on this address

    int n;
    char msg[TAILLE_MAX];
    int clientAddressLength;
    int pid;

    //create socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    //initialize the socket addresses
    memset(&serverAddress,0,sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);

    //bind the socket with the server address and port
    bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress));

    //listen for connection from client
    listen(sockfd,5);

    while(1)
    {
        //parent process waiting to accept a new connection
        printf("\nServeur: En attente de connexion d'un nouveau client...\n");
        clientAddressLength=sizeof(clientAddress);
        newsockfd=accept(sockfd,(struct sockaddr*)&clientAddress,&clientAddressLength);
        printf("Serveur: Connexion d'un nouveau client avec succès. (IP: %s)\n",inet_ntoa(clientAddress.sin_addr));

        //child process is created for serving each new clients
        pid=fork();
        if(pid==0){
            // On est dans le processus fils
            while(1){
                printf("Serveur: Envoi en cours...\n");

                for(int i=0; i<NB_ZONES_MAX; i++){
                    // A chaque envoi, on envoie la zone i
                    zone z;
                    z = p->zones[i];
                    int len = 0;
                    int err;

                     err = sendPourTCP(sizeof(z), (char *) &z, newsockfd);
                    //printf("err: %d \n", err);
                    switch(err){
                        case 0:
                            fprintf(stderr, "Serveur: Erreur reçue du client (IP: %s) : send = 0\n",inet_ntoa(clientAddress.sin_addr));
                            break;
                        case -1:
                            fprintf(stderr, "Serveur: Erreur reçue du client (IP: %s) : send = -1\n",inet_ntoa(clientAddress.sin_addr));
                            break;
                        case 1:
                            printf("Serveur: Envoi de la zone %d au client (IP: %s) terminé.\n",i, inet_ntoa(clientAddress.sin_addr));
                            break;
                    }
                }
                printf("Serveur: Le client (IP: %s) a bien reçue toutes les données.\n", inet_ntoa(clientAddress.sin_addr));
                    
                /*unsigned char buffer[32], *ptr;

                ptr = serialize_temp(buffer, z0);*/


                sleep(30);

            }
                exit(0);
        }
        else{
            close(newsockfd);
            // Le parent ferme la socket
            }
    }

        
    if((shmdt(p)) < 0) {
        perror("Erreur lors du detachement ");
        exit(EXIT_FAILURE);
    }

    printf("Serveur: Segment détaché.\n");

    return 0;


}
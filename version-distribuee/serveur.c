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
#include <sys/select.h>

#include "structures.h"
#include "TCP.h"

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

void envoiEspace(principale* p, int Socket, struct in_addr IP){
    printf("Serveur: Envoi en cours...\n");

    for(int i=0; i<NB_ZONES_MAX; i++){
        // A chaque envoi, on envoie la zone i
        zone z;
        z = p->zones[i];
        //int len = 0;
        int err;

        err = sendPourTCP(sizeof(z), (char *) &z, Socket);
        //printf("err: %d \n", err);
        switch(err){
        case 0:
            fprintf(stderr, "Serveur: Erreur reçue du client (IP: %s, zone n°%d) : send = 0\n", inet_ntoa(IP), i);
            break;
        case -1:
            fprintf(stderr, "Serveur: Erreur reçue du client (IP: %s, zone n°%d) : send = -1\n", inet_ntoa(IP), i);
            break;
        case 1:
            //printf("Serveur: Envoi de la zone %d au client (IP: %s) terminé.\n",i, inet_ntoa(IP));
            break;
        }
    }

    printf("Serveur: Le client (IP: %s) a bien reçue toutes les données.\n", inet_ntoa(IP));
}

int initSemaphores(char* fichier_semaphores, int  cle_semaphore) {
  
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

    return idSem;

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

/*void* sendMajForAllClient(maj_struct_serveur* tem){
    printf("je rentre dans senForAll\n");
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int sockfd2;          // descripteurs de socket
             // tableau qui contiendra tous les descripteurs de sockets,
                                  // avec une taille egale a la taille max de l'ensemble d'une structure fd_set
               // nombre de descripteurs dans le tableau precedent
    char buf[1024];               // espace necessaire pour stocker le message recu

    memset(buf,'\0',1024);        // initialisation du buffer qui sera utilisé
  
    t[0]=tem->sockfd; // on ajoute deja la socket d'ecoute au tableau de descripteurs
    taille++;    // et donc on augmente "taille"

    while(1){
        int i;
        for(i=1;i<taille;i++){ // on parcourt tous les autres descripteurs du tableau     
                int k;
                //char buf[100];
                strcpy(buf,"-------UNE MAJ VIENT D'ETRE  REALISEE---------");
                for(k=1;k<taille;k++){ // puis on l'envoie a tous les clients...
                    
                        if(send(t[k],buf,strlen(buf),0) == -1){
                            perror("Erreur lors de l'appel a send -> ");
                            exit(1);
                        }
                    
                }
            
        }
    }
}*/

/*void* sendMajForAllClient(void* arg){
    maj_struct_serveur* tem= arg;
    printf("je rentre dans senForAll\n");
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int sockfd2;          // descripteurs de socket
    int t[FD_SETSIZE];            // tableau qui contiendra tous les descripteurs de sockets,
                                  // avec une taille egale a la taille max de l'ensemble d'une structure fd_set
    int taille=0;                 // nombre de descripteurs dans le tableau precedent
    char buf[1024];               // espace necessaire pour stocker le message recu

    memset(buf,'\0',1024);        // initialisation du buffer qui sera utilisé
  
    t[0]=tem->sockfd; // on ajoute deja la socket d'ecoute au tableau de descripteurs
    taille++;    // et donc on augmente "taille"

    while(1){
        int i;
        for(i=1;i<taille;i++){ // on parcourt tous les autres descripteurs du tableau     
                int k;
                //char buf[100];
                strcpy(buf,"-------UNE MAJ VIENT D'ETRE  REALISEE---------");
                for(k=1;k<taille;k++){ // puis on l'envoie a tous les clients...
                    
                        if(send(t[k],buf,strlen(buf),0) == -1){
                            perror("Erreur lors de l'appel a send -> ");
                            exit(1);
                        }
                    
                }
            
        }
    }
}*/

void* MAJ(void* arg){
    maj_struct_serveur *temp =  arg; 

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
        while (1)
        {
            if((attente= semctl(idSem, temp->index, GETVAL)) == -1){ // On récupères le nombre de processus restants
                perror("problème init");//suite
            }

            if (attente == 0){
                while (semctl(idSem, temp->index, GETVAL) == 0){
                    // On attend que le client ait fini sa modification
                    // Pour plus tard : mettre un wait au lieu de boucler en boucle
                }
            
                // Le client a fini sa modif
                printf("Un client a modif une zone, go en informer tout le monde\n");
                int msg = 500;
                int n=send(temp->sockfd,&msg,sizeof(msg),0);
                printf("j'ai envoyé %d octets\n", n);

                //afficheZone(temp->p,temp->index);
            }
        }

}

int main(int argc, char** argv){
    if(argc != 1) {
        printf("Utilisation : ./serveur <Port>\t<Adresse>\t \n");
        printf("<Port>: un numèro de port accessible\n");
        printf("<Adresse>: une adresse de type IPV4\n");

        exit(1);
    }

    int idSem = initSemaphores(FICHIER_SEMAPHORES, CLE_SEMAPHORES);
    principale* p = initZones(FICHIER_PARTAGE, CLE_PARTAGE);



    int sockfd;//to create socket
    int newsockfd;//to accept connection

    struct sockaddr_in serverAddress;//server receive on this address
    struct sockaddr_in clientAddress;//server sends to client on this address

    //int n, listener, rv;
    socklen_t clientAddressLength;
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
        //Le processus parent attend une nouvelle connexion
        printf("\nServeur: En attente de connexion d'un nouveau client...\n");
        clientAddressLength=sizeof(clientAddress);
        newsockfd=accept(sockfd,(struct sockaddr*)&clientAddress,&clientAddressLength);
        printf("Serveur: Connexion d'un nouveau client avec succès. (IP: %s)\n",inet_ntoa(clientAddress.sin_addr));

        //Un processus fils est crée pour chaque nouveau client
        pid=fork();
        if(pid==0){
            // On est dans le processus fils
            while(1){
                pthread_t arrayT[NB_ZONES_MAX];
                // On met en place le thread d'envoi propre au client associé à ce processus fils

                for(int i=0; i<NB_ZONES_MAX; i++){
                    maj_struct_serveur* var = malloc(sizeof *var);
                    var->sockfd= newsockfd;
                    var->memoire = p;
                    var->index = i;

                    if(pthread_create(&arrayT[i], NULL, MAJ, var) == -1) {
                        perror("pthread_create");
                        return EXIT_FAILURE;
                    }
                }

                envoiEspace(p, newsockfd, clientAddress.sin_addr);
                    
                // Le serveur reçoit une zone où le client souhaite intervenir
                int zoneOccupee;
                zoneOccupee=1;
                while(zoneOccupee == 1){//Envoie tu msg d err ( zone occupée) en boucle tant que le client continu de dmd
                                            // l accée a une zone occupée
                    // Opération P
                    opp.sem_op = -1;
                    opp.sem_flg = 0;
        
                    // Opération V
                    opv.sem_op = 1;
                    opv.sem_flg = 0;

                    int numZone;
                    //recvPourTCP((char *)&numZone, sockfd);
                    recv(newsockfd,&numZone,sizeof(numZone),0);

                    opp.sem_num = numZone;
                    opv.sem_num = numZone;

                    int attente;
                    if((attente= semctl(idSem, numZone, GETVAL)) == -1){ // On récupères le nombre de processus restants
                        perror("problème init");//suite
                    }

                    if (attente == 0) {
                        printf("Cette zone est en cours de modification par un autre client, veuillez patientez ... \n");
                        int msg_attente;
                        msg_attente=10;
                        if(send(newsockfd ,&msg_attente , sizeof(msg_attente) , 0) < 0)
                            {
                                puts("L'envoi a échoué");
                                return 1;
                            }
                    }

                    else{
                        zoneOccupee++;
                        semop(idSem,&opp,1);
                        
                        // Le serveur renvoie au client le statut de la zone en cours
                        //sendPourTCP(sizeof(attente), (char *)&attente, sockfd);
                        send(newsockfd,&attente,sizeof(attente),0);

                        // Le serveur attend en retour la nouvelle zone après que le client ait fini
                        zone new;
                        int err;
                        err = recvPourTCP((char *) &new, newsockfd);
                        printf("err reception new zone : %d \n", err);

                        // Le serveur valide la modification en écrasant la zone dans le segment de mémoire
                        p->zones[numZone] = new;
        

                        // Le serveur redonne l'accès à la zone
                        semop(idSem,&opv,1);

                        printf("test affichage p chez serv après modif\n");
                        afficheZones(p);



                        // Le serveur renvoie le segment entier au client
                        envoiEspace(p, newsockfd, clientAddress.sin_addr);
                        

                    }

                }

                for (int i = 0; i < NB_ZONES_MAX; ++i) {
                    pthread_join(arrayT[i], NULL);
                }

                //sleep(5);

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
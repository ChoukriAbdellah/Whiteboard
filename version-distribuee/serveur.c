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
#include "affichage.h"
#include "TCP.h"

//pthread_cond_t *zoneModifiee = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
//for(int i=0; i<NB_ZONES_MAX; i++) zoneModifiee[i] = PTHREAD_COND_INITIALIZER;
pthread_cond_t zoneModifiee = PTHREAD_COND_INITIALIZER;
pthread_cond_t zoneModifiee2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;


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

int initSemaphores() {
  
    //Création du fichier s'il n'existe pas
    int fd = open(FICHIER_SEMAPHORES, O_CREAT|O_WRONLY, 0644);
    close(fd);
  
    key_t cle = ftok(FICHIER_SEMAPHORES, CLE_SEMAPHORES);

    // Création de NB_ZONE_MAX sémaphore associée a la clé
    int idSem = semget(cle, NB_ZONES_MAX, IPC_CREAT|0666);

    // Initialisation des sémaphore a 1
    union semun egCtrl;
    egCtrl.val=1;

    for(int i=0; i<NB_ZONES_MAX; i++){
        if(semctl(idSem, i, SETVAL, egCtrl) == -1) {
            perror("Probleme semctl fonction initSemaphores()");
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
        strcpy(z.lastModif , "Personne");
        strcpy(z.texte,"Ceci est un texte");
        strcpy(z.titre,"Document vide");
        p.zones[i] = z;
    }

     if ((idS=shmget(cle, sizeof(principale), IPC_CREAT | 0666)) < 0){
        if(errno == EEXIST)
             fprintf(stderr, "Le segment de memoire partagee (cle=%d) existe deja\n", cle);
        else
            perror("Erreur shmget ");
        exit(EXIT_FAILURE);
    } 

    printf("Serveur: Segment de mémoire partagée crée et attaché.\n");

    principale * shmaddr;
    if ((shmaddr = (principale *) shmat(idS, NULL, 0)) == (void *) -1){
        perror("Erreur shmat ");
        exit(EXIT_FAILURE);
    }
    
    *shmaddr = p;

    printf("Serveur: Initialisation de l'espace partagé terminé.\n");
  
    //afficheZonesLeger(*shmaddr);

    return shmaddr;
}

void* MAJ(void* arg){

    maj_struct_serveur *temp =  arg; 
    //printf("thread n°%d crée chez fils\n", temp->index);

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
        perror("Erreur semget ");
        exit(EXIT_FAILURE);
    } 


    while(1){
            /*int attente;
            if((attente = semctl(idSem, temp->index, GETVAL)) == -1){ // On récupères le nombre de processus restants
                perror("problème init");//suite
            }

            pthread_mutex_lock(&verrou);
            while(attente != 0){
                //printf("thread n°%d : en attente qu'un client accede à une zone\n", temp->index);
                pthread_cond_wait(&zoneModifiee, &verrou);
                if((attente = semctl(idSem, temp->index, GETVAL)) == -1){ // On récupères le nombre de processus restants
                    perror("problème init");//suite
                }
            }// la c le premier wait tant que personne a modif le truc (attente != 0)

            pthread_mutex_unlock(&verrou);

            printf("thread n°%d : en cours de modif\n", temp->index);

            pthread_mutex_lock(&verrou);
            while(attente != 1){
                //printf("thread n°%d : en attente qu'un client accede à une zone\n", temp->index);
                pthread_cond_wait(&zoneModifiee2, &verrou);
                if((attente = semctl(idSem, temp->index, GETVAL)) == -1){ // On récupères le nombre de processus restants
                    perror("problème init");//suite
                }
            }// la c le premier wait tant que personne a modif le truc (attente != 0)

            pthread_mutex_unlock(&verrou);*/

            opz.sem_op = 0;
            opz.sem_flg = 0;
            opz.sem_num = temp->index;
            opz.sem_flg = SEM_UNDO;

            printf("thread n°%d : en attente d'une fin de modif\n", temp->index);

            if ((semop(temp->idSemMAJ,&opz,1)) < 0){ // Mise en attente
                perror("Erreur semop ");
                exit(EXIT_FAILURE);
            }

            printf("thread n°%d : fin modif\n", temp->index);

            // Là le thread doit envoyer la mise à jour
            zone z;
            z = temp->p->zones[temp->index];
            printf("test affichage zone avant envoi\n");
            afficheZone(z);
            //printf("envoi de la mise à jour : en cours\n");
            int erreur = sendPourTCP(sizeof(z), (char *)&z, temp->sockfd);
            switch(erreur){
                case 0:
                    fprintf(stderr, "Serveur: Erreur reçue du client (IP: , zone n°%d) : send = 0\n", temp->index);
                    break;
                case -1:
                    fprintf(stderr, "Serveur: Erreur reçue du client (IP: , zone n°%d) : send = -1\n", temp->index);
                    break;
                case 1:
                    printf("Serveur: Envoi de la mise à jour du document n°%d au client terminé.\n",temp->index);
                    break;
            }

            // On rebloque le thread après qu'il ai envoyé sa mise à jour
            semop(temp->idSemMAJ,&opp,1);
    }

        pthread_exit(NULL);
}

int main(int argc, char** argv){
    if(argc != 2) {
        printf("Utilisation : ./serveur <Port>\t\n");
        printf("<Port> : un numero de port accessible\n");

        exit(1);
    }

    int PORT = atoi(argv[1]);

    // Tableau utilisé pour bloquer l'accès aux zones lorsqu'elles sont en cours de modification par un client :
    int idSem = initSemaphores();

    // Tableau utilisé pour faire un RDV entre tous les processus lors de l'envoi de la MAJ
    // Chaque thread se met en attente sur une zone qui lui est associée et quand le client finit une modif sur la zone qu'il surveille...
    /// ... alors il va se réveiller et envoyer à son client la mise à jour concernant la zone qu'il a surveillé
    // Etant donné que chaque processus fils gère un client, donc chaque fils par le biais de ses threads va envoyer à son client la mise à jour, ce qui fait que tout le monde va la recevoir

    int idSemMAJ = initSemaphores();

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

            // Pour chaque fils, on crée autant de threads qu'il y'a de zones

            pthread_t arrayT[NB_ZONES_MAX];
            for(int i=0; i<NB_ZONES_MAX; i++){
                    maj_struct_serveur* var = malloc(sizeof *var);
                    var->sockfd= newsockfd;
                    var->p = p;
                    var->index = i;
                    var->idSemMAJ = idSemMAJ;

                    if(pthread_create(&arrayT[i], NULL, MAJ, var) == -1) {
                        perror("pthread_create");
                        return EXIT_FAILURE;
                    }
            }

            while(1){

                // Il envoie toujours au client le contenu de l'espace au début de sa connexion
                envoiEspace(p, newsockfd, clientAddress.sin_addr);
                    
                while(1){
                    // Opération P
                    opp.sem_op = -1;
                    opp.sem_flg = 0;
                    opp.sem_flg = SEM_UNDO;
        
                    // Opération V
                    opv.sem_op = 1;
                    opv.sem_flg = 0;
                    opv.sem_flg = SEM_UNDO;

                    // Le serveur reçoit une zone où le client souhaite intervenir

                    int numZone;
                    //recvPourTCP((char *)&numZone, sockfd);
                    recv(newsockfd,&numZone,sizeof(numZone),0);

                    opp.sem_num = numZone;      
                    opv.sem_num = numZone;

                    printf("Serveur: Mon client cherche à agir sur la zone n°%d\n", numZone);

                    int attente;
                    //printf("Avant attente\n");
                    if((attente= semctl(idSem, numZone, GETVAL)) == -1){ // On récupères le nombre de processus restants
                        perror("Problème semaphore");//suite
                        exit(EXIT_FAILURE);
                    }

                    if (attente == 0) {
                        int peutModif = 0;
                        send(newsockfd,&(peutModif),sizeof(peutModif),0);

                        /*printf("Cette zone est en cours de modification par un autre client, veuillez patientez ... \n");
                        zone z;
                        char titre[10] ="erreur";
                        char createur[10]= "Admin";
                        char texte[80]= "Du serveur: La zone que vous avez choisie n'est pas disponible pour le moment.";
                        z.numeroZone=-1;
                        strcpy(z.titre, titre);
                        strcpy(z.lastModif, createur);
                        strcpy(z.texte, texte);
                        int err;

                        err = sendPourTCP(sizeof(z), (char *) &z, newsockfd);
                        //printf("err: %d \n", err);
                        switch(err){
                        case 0:
                            fprintf(stderr, "Serveur: Erreur reçue  : send = 0 pour la zone %d\n",  z.numeroZone);
                            break;
                        case -1:
                            fprintf(stderr, "Serveur: Erreur reçue  send = -1 pour la zone %d\n", z.numeroZone);
                            break;
                        case 1:
                            //printf("Serveur: Envoi de la zone %d au client (IP: %s) terminé.\n",i, inet_ntoa(IP));
                            break;
                        }*/
                        
                    
                    }

                    else{
                        semop(idSem,&opp,1);
                        //pthread_cond_broadcast(&zoneModifiee); // Il reveille le thread en cours de modif
                        int peutModif = 1;

                        send(newsockfd,&(peutModif),sizeof(peutModif),0);


                        // Le serveur attend en retour la nouvelle zone après que le client ait fini
                    
                        //int err;
                        /*err = */
                        zone new;
                        recvPourTCP((char *) &new, newsockfd);
                        //pthread_cond_broadcast(&zoneModifiee2); 
                        // Il reveille le thread en fin de modif : 
                        semop(idSemMAJ,&opp,1);

                        //printf("err reception new zone : %d \n", err);

                        // Le serveur valide la modification en écrasant la zone dans le segment de mémoire
                        p->zones[numZone] = new;
        
                        // Le serveur redonne l'accès à la zone
                        semop(idSem,&opv,1);
                        
                        //printf("test affichage p chez serv après modif\n");
                        //afficheZones(p);        

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
            //printf("Serveur : Le client (IP: %s) s'est déconnecté.\n", inet_ntoa(clientAddress.sin_addr));
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
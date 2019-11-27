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

void afficheZones(principale p){
    for(int i=0; i<NB_ZONES_MAX; i++){
        afficheZone(p.zones[i]);
    }
}

/*void editData(principale* p){
 
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
        
}*/

unsigned char * deserialize_int(unsigned char *buffer, int *value){}

int recvPourTCP(char* msg, int Socket){
    // Printf en commentaires pour debug


    int s;
    int i=0;
    // On fait une réception d'abord pour récuperer la taille à recevoir : 
    recv(Socket,&s,sizeof(s),0);
    //printf("Client: On veut recevoir %d octets \n", s);
    int totalRecv = 0;

    while(totalRecv  < s){
        int nbRecv = 0;
        nbRecv = recv(Socket,msg+totalRecv,s-totalRecv,0);
        //printf("Iteration i=%d, nb reçus = %d\n", i, nbRecv);

        if (nbRecv == -1){
            return -1;
        }
        else if (nbRecv == 0){
            //printf("Total recu: %d\n", totalRecv);
            return 0;
        }
        else
            totalRecv+=nbRecv;
        i++;
    }

    //rintf("Fin correcte\n");
    return 1;
}

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


    printf("\nClient: Structure en cours de reception...\n");


    principale p;    

    for(int i=0; i<NB_ZONES_MAX; i++){
        zone reception;
        recvPourTCP((char *) &reception, sockfd);
        p.zones[i] = reception;
    }

    afficheZones(p);

    while(1){

        printf("Client: Tout a bien été reçu.\n");
        //editData(p);
        sleep(29);

    }

    return 0;

}
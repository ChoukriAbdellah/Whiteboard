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

int recvPourTCP(char* msg, int Socket){
    int s;
    int i=0;
    recv(Socket,&s,sizeof(s),0);
    printf("Client: On veut recevoir %d octets \n", s);
    int totalRecv = 0;

    while(totalRecv  < s){
        int nbRecv = 0;
        nbRecv = recv(Socket,msg+totalRecv,s-totalRecv,0);
        printf("Iteration i=%d, nb reçus = %d\n", i, nbRecv);

        if (nbRecv == -1){
            return -1;
        }
        else if (nbRecv == 0){
            return 0;
        }
        else
            totalRecv+=nbRecv;
        i++;
        sleep(1);
    }

    return 1;
}

int main(int argc, char** argv){
    int sockfd;//to create socket

    struct sockaddr_in serverAddress;//client will connect on this

    int n;
    char msg1[TAILLE_MAX];
    char msg2[TAILLE_MAX];

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

    while(1){
        printf("\nStructure reçue : \n");
        /*fgets(msg1,TAILLE_MAX,stdin);
        if(msg1[0]=='#')
        break;

        n=strlen(msg1)+1;
        send(sockfd,msg1,n,0);*/

        // Ajouter fonction qui vérifie si le nb octets reçus = nb octets attendus

        zone reception;

        recvPourTCP((char *) &reception, sockfd);

        afficheZone(reception);

        sleep(29);

    }

    return 0;

}
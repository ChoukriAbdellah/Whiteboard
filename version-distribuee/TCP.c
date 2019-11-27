#include "TCP.h"

#include<sys/socket.h>//socket

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
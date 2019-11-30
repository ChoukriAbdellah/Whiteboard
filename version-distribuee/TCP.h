#ifndef _TCP_
#define _TCP_

#include<sys/socket.h>//socket

int sendPourTCP(int s, char* msg, int Socket);
int recvPourTCP(char* msg, int Socket);

#endif
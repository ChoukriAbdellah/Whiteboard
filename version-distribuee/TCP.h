#ifndef _TCP_
#define _TCP_

#include <sys/socket.h>//socket
#include <unistd.h>
#include <stdio.h>

int sendPourTCP(int s, char* msg, int Socket);
int recvPourTCP(char* msg, int Socket);
int recvNonBloquant(char* msg, int Socket);

#endif
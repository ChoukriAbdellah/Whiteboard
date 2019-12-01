#ifndef _TCP_
#define _TCP_

#include <sys/socket.h>//socket
#include <unistd.h>
#include <stdio.h>
#include "structures.h"

int sendPourTCP(int s, char* msg, int Socket);
int recvPourTCP(char* msg, int Socket);
int recvNonBloquant(int s, char* msg, int Socket);

#endif
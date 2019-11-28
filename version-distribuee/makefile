CC = gcc
CFLAGS = -Wall -g

all: serveur client

serveur: serveur.c 
	$(CC) $(CFLAGS) -c serveur.c TCP.c
	$(CC) $(CFLAGS) serveur.o TCP.o -o serveur -lpthread

client: client.c
	$(CC) $(CFLAGS) -c client.c TCP.c
	$(CC) $(CFLAGS) client.o TCP.o -o client -lpthread



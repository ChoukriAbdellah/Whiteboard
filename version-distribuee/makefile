CC = gcc
CFLAGS = -Wall -g

all: serveur client

serveur: serveur.c 
	$(CC) $(CFLAGS) -c serveur.c TCP.c affichage.c
	$(CC) $(CFLAGS) serveur.o TCP.o affichage.c -o serveur -lpthread

client: client.c
	$(CC) $(CFLAGS) -c client.c TCP.c affichage.c
	$(CC) $(CFLAGS) client.o TCP.o affichage.c -o client -lpthread



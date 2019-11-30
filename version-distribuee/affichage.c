#include<stdio.h>

#include "affichage.h"

void afficheZone(zone z){
    printf("---------- Document n°%d ---------\n", z.numeroZone);
    printf("\t Titre : %s \n \n",z.titre);
    printf("%s \n \n", z.texte);
            
    printf("Dernière modification par : %s \n \n", z.lastModif);
}

void afficheZoneLeger(zone z){
    printf("%d. Titre : [%s], dernière modif par : %s\n", z.numeroZone, z.titre, z.lastModif);
}

void afficheZones(principale p){
    for(int i=0; i<NB_ZONES_MAX; i++){
        afficheZone(p.zones[i]);
    }
}

void afficheZonesLeger(principale p){
    for(int i=0; i<NB_ZONES_MAX; i++){
        afficheZoneLeger(p.zones[i]);
    }
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>


int main() {
    int i , *ptri =&i;
    double d, *ptrd =&d;
    char c1= '1'   , *ptrc1 =&c1;

    printf("ptri =%u ptrc1 =%u\n",ptri,ptrc1); // pour afficher les adresses des variables i et c1 en décimal
    printf("ptri =%x ptrc1 =%x\n",ptri,ptrc1); // pour afficher les adresses des variables i et c1 en hexadécimal

    printf("ptri =%p ptrc1 =%p\n", ptri, ptrc1); // pour afficher les adresses des variables i et c1 en utilisant le format de pointeur
    printf("*ptri =%d et *ptrc1 =%c\n", *ptri, *ptrc1); // pour afficher les valeurs pointées par ptri et ptrc1, c'est à dire les valeurs de i et c1
    printf("ptrd =%f et *ptrd = %f et &ptrd=%p\n",ptrd,*ptrd,&ptrd);

    ++ptri; // incrémentation du pointeur ptri pour pointer vers l'adresse suivante
    ptri++; // incrémentation du pointeur ptri pour pointer vers l'adresse suivante
    ptrc1=(char *)ptri; // affectation de l'adresse pointée par ptri à ptrc1, en la convertissant en pointeur de type char
    printf("ptri =%p ptrc1 =%p\n", *ptri, *ptrc1); // pour afficher 
   ptrc1+=sizeof(int);   // incrémentation du pointeur ptrc1 de la taille d'un int en octets pour pointer vers l'adresse suivante
   printf("ptrc1 =%d\n",ptrc1); // pour afficher la valeur de ptrc1, qui est maintenant égale à la taille d'un int en octets
    
    return 0;
}
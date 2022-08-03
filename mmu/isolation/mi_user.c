#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hardware.h"
#include "hw_config.h"
#include "mi_syscall.h"

/**
 * @brief Opération de calcul effectué par un pocessus
 * 
 * @param ptr L'adresse virtuelle où le processus courant va écrire le résultat
 *            de l'opération
 * @return int retourne le résultat du calcul de la somme
 */
int sum(void *ptr) {
    int i;
    int sum = 0;

    for(i = 0; i < PAGE_SIZE * N/2 ; i++)
        sum += ((char*)ptr)[i];
    return sum;
}

void init(void) {
    void *ptr;
    int r0, r1;
   
    ptr =  virtual_memory;
    printf("virtual_memory = %p\n", ptr);
    
    _int(SYSCALL_SWTCH_0);
    printf("*** Remplissage des pages de mémoire du processus 0 avec des 1 ***\n");
    memset(ptr, 1, PAGE_SIZE * N/2);
    
    _int(SYSCALL_SWTCH_1);
    printf("*** Remplissage des pages de mémoire du processus 1 avec des 3 ***\n");
    memset(ptr, 3, PAGE_SIZE * N/2);
    
    _int(SYSCALL_SWTCH_0);
    r0 = sum(ptr);
    printf("--> Resultat du processus 0 : %d\n", r0);

    _int(SYSCALL_SWTCH_1);
    r1 = sum(ptr);
    printf("--> Resultat processus 1 : %d\n", r1);

    assert(r0*3 == r1);
}
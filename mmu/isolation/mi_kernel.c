/* memory isolation
   Philippe Marquet, Nov 2017
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware.h"
#include "mi_syscall.h"
#include "hw_config.h"
#include "mi_kernel.h"

// numero du processus courant
static int current_process;

static void switch_to_process0(void) {
    current_process = 0;
    printf("\n----------------------------\nprocessus 0\n----------------------------\n\n");
    _out(MMU_CMD, MMU_RESET);
}

static void switch_to_process1(void) {
    current_process = 1;
    printf("\n----------------------------\nprocessus 1\n----------------------------\n\n");
    _out(MMU_CMD, MMU_RESET);
}

/**
 * Vérification de la validité de l'adresse virtuelle.
 * \param[in] vaddr L'adresse à tester.
 * \return 1 si l'adresse est dans l'espace des adresses virtuelles,
 * sinon 0.
 */
static unsigned is_vaddr(unsigned vaddr){
    if(vaddr < VM_BEGIN || vaddr > VM_END) 
        return 0;
    return 1;
}

/**
 * @brief Fonction de mapping(correspondance entre une page virtuelle et une page physique)
 * Retourne le numéro de la page physique associée à la page virtuelle vpage du process{process}
 * (0 pour le premier processus, 1 pour le second).
 * @return La fonction retourne -1 si l’adresse de page virtuelle est en dehors de l’espace alloue au processus.
 */
static int ppage_of_vaddr(int process, unsigned vpage) {
    int ppage;

    /*  verification de la validité de la page virtuel
        On ne peut continuer si on est dehors de l’espace alloue au processus. 
        Le processus a droit à accédder à page de 0 à N/2-1 (car il a N/2 pages)
     */
    if(vpage > ((N/2) -1)) {
        fprintf(stderr, "Erreur page virtuelle incorrecte.\n");
        return -1;
    }

    /*  En fonction du processus en leur associe une page physique.
        Les 2 processus ne partagent pas le même espace d'adressage physique
        car ils pointent sur des pages physiques différentes
    */
    if (process == 0)
        ppage = vpage + 1;  // vpage + 1, car le début de la mémoire physique est occupé par le vecteur d’interruption(IRQ_VECTOR)
    else if (process == 1)
        ppage = vpage + (N/2) + 1;
    return ppage;
}

/**
 * @brief Defines an empty interruption
 */
static void empty_it(void) {};

/**
 * @brief Fonction associée aux interruptions MMU. Elle permettra de mettre en place le
 * mapping pour le processus courant.
 */
static void mmuhandler() {
    printf("current process = %d\n", current_process);
    struct tlb_entry_s tlb;
    unsigned int vpage;
    unsigned int vaddr = ((long int)_in(MMU_FAULT_ADDR_HI)) << 32 | (_in(MMU_FAULT_ADDR_LO) & 0xFFFFFFFF);
    
    /*  verification de la validité de l'adresse virtuel
        On ne peut pas continuer si on est dehors de l’espace alloué au processus. 
     */
    if (!is_vaddr(vaddr)) {
        fprintf(stderr, "Erreur l'adresse virtuel n'est pas valide.\n");
        exit(EXIT_FAILURE);
    }
    
    // Le numero de page virtuelle associée à une adresse est obtenu en ignorant les 12 bits de poids faible de l’adresse.
    // On soustrait l'adresse virtuelle de la 1ère adresse de la mémoire virtuelle afin d'avoir une adresse virtuelle relative
    // à la zone mémoire associée au processus
    vpage = ((vaddr - VM_BEGIN) >> 12) & 0xFFF;
    printf("vpage = %d\n",vpage);
    
    // on récupère le numéro de page physique associé à la page virtuelle vpage du processus courant
    int ppage = ppage_of_vaddr(current_process, vpage);
    printf("ppage = %d\n",ppage);

    // Termine si la page retournée est hors la zone mémoire du processus courant
    if(ppage == -1) {
        fprintf(stderr, "Erreur lors de la recherche d'une page physique pour une adresse vaddr.\n");
        exit(EXIT_FAILURE);
    }

    tlb.tlb_vpage = vpage;
    tlb.tlb_ppage = ppage;
    tlb.tlb_acces = EXEC_ACCESS + WRITE_ACCESS + READ_ACCESS;  // en binaire: 0b111 -> accès en execution + accès en ecriture + accès en lecture
    tlb.tlb_active = 1;
    _out(TLB_ADD_ENTRY, *(int *)(&tlb)); // l'ajout d'une entrée(association mem_virt-mem_phy) dans TLB
}


int main(int argc, char **argv) {
    unsigned int i;

    /* init hardware */
    if (init_hardware(HARDWARE_INI) == 0) {
       fprintf(stderr, "Error in hardware initialization\n");
       exit(EXIT_FAILURE);
    }

    /* dummy interrupt handlers */
    for (i=0; i<IRQ_VECTOR_SIZE; i++)
       IRQVECTOR[i] = empty_it;

    IRQVECTOR[MMU_IRQ] = mmuhandler;
    IRQVECTOR[SYSCALL_SWTCH_0] = switch_to_process0;
    IRQVECTOR[SYSCALL_SWTCH_1] = switch_to_process1;
    
    /* activation du user mode et allows all IT */
    _mask(0x1001);
    
    /* passage en mode utilisateur */
    init();

    exit(EXIT_SUCCESS);
}
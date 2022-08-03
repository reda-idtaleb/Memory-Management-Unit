#include "mmu_manager.h"

extern void user_process(int operation);

p_mapping pm_mapping[PM_PAGES];
v_mapping vm_mapping[VM_PAGES];

/** mémorise la page virtuelle courante afin de l'enregitrer dans le fichier swap */
int current_vpage = -1; 

/** indique la page physique courante */
int current_ppage = 1;

static void empty_it() {}

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

static void mmuhandler_simple () {
    struct tlb_entry_s tlb;
    unsigned int vpage;
    // récupération de l'adresse fautive
    unsigned int vaddr = ((long int)_in(MMU_FAULT_ADDR_HI)) << 32 
                                 | (_in(MMU_FAULT_ADDR_LO) & 0xFFFFFFFF);
    
    int ppage = 1; // une seule page physique autorisée pour ce handler

    // on verifie si l'addresse virtuelle fautive est dans l'espace d'adressage virtuelle
    if (!is_vaddr(vaddr)) {
        fprintf(stderr, "Erreur: l'adresse virtuelle %d n'est pas valide.\n", vaddr);
        exit(EXIT_FAILURE);
    }
    
    // calculer la page virtuelle de l'adresse fautive(12 bits du milieu de vaddr)
    vpage = ((vaddr - VM_BEGIN) >> 12) & 0xFFF; 
    
    if (current_vpage != -1) {
        //On sauvegarde donc cette page dans le fichier de swap
        store_to_swap(current_vpage, ppage);
        tlb.tlb_vpage = current_vpage;
        tlb.tlb_ppage = ppage;
        tlb.tlb_acces = EXEC_ACCESS + WRITE_ACCESS + READ_ACCESS; 
        tlb.tlb_active = 1;
        // on supprime le mapping dans la TLB pour la page fautive
        _out(TLB_DEL_ENTRY, *(int *)(&tlb));
        // On charge la page correspondante a l’adresse fautive
        fetch_from_swap(vpage, ppage);
    }

    // mapping de la page virtuelle à la page physique fautive
    // et mise à jout de la TLB
    tlb.tlb_vpage = vpage;
    tlb.tlb_ppage = ppage;
    tlb.tlb_acces = EXEC_ACCESS + WRITE_ACCESS + READ_ACCESS; 
    tlb.tlb_active = 1;

    _out(TLB_ADD_ENTRY, *(int *)(&tlb)); 
    current_vpage = vpage;
}

static void mmuhandler_extended () {
    struct tlb_entry_s tlb;
    unsigned int vpage;
    // récupération de l'adresse fautive
    unsigned int vaddr = ((long int)_in(MMU_FAULT_ADDR_HI)) << 32 
                                 | (_in(MMU_FAULT_ADDR_LO) & 0xFFFFFFFF);


    // on verifie si l'addresse virtuelle fautive est dans l'espace d'adressage virtuelle
    if (!is_vaddr(vaddr)) {
        fprintf(stderr, "Erreur: l'adresse virtuelle %d n'est pas valide.\n", vaddr);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "hey\n");
    // calculer la page virtuelle de l'adresse fautive(12 bits du milieu de vaddr)
    vpage = ((vaddr - VM_BEGIN) >> 12) & 0xFFF; 
   /** fprintf(stderr, "1. vpage = %d\n", vpage);
    fprintf(stderr, "1. current_ppage = %d\n\n", current_ppage);
    */
    // On regarde si la page est mappée en mémoire physique
    if (vm_mapping[vpage].is_in_memory) {
        //fprintf(stderr, "déjà en mémoire physique\n");
        // ici, ça veut dire qu'il ne manque que l'entrée dans la TLB
        
        // on construit une nouvelle entre de la TLB
        tlb.tlb_vpage = vpage; // la page virtuelle fautive
        tlb.tlb_ppage = vm_mapping[vpage].ppage; // le numéro de page physique qui lui est associée dans la mémoire
        tlb.tlb_acces = EXEC_ACCESS + WRITE_ACCESS + READ_ACCESS; 
        tlb.tlb_active = 1;
        _out(TLB_ADD_ENTRY, *(int*)(&tlb)); // Ecrire dans TLB
    }
    else {
        //fprintf(stderr, "libération d'une ppage\n");
        // si on est là -> la page demandée n'est pas en mémoire physique
        // Donc on doit libérer une page physique dans le swap puis on met à jour la TLB

        // libérer la page physique d'avant SI elle existe
        if(pm_mapping[current_ppage].is_mapped) {
            // Je stocke la page dans le swap (disque) 
            store_to_swap(pm_mapping[current_ppage].vpage, current_ppage); 
            tlb.tlb_ppage = current_ppage;
            tlb.tlb_vpage = vpage;
            tlb.tlb_acces = EXEC_ACCESS + WRITE_ACCESS + READ_ACCESS; 
            tlb.tlb_active = 1;
            // Je supprime l'entrée dans la LTB
            _out(TLB_DEL_ENTRY, *(int *)(&tlb));
            // On indique que la vpage est désormais dans  le swap
            pm_mapping[current_ppage].is_mapped = 0;
            // On dit que la page virtuelle qui se trouvait à l'endroit de la page physique qu'on a supprimé est supprimée ! 
            vm_mapping[pm_mapping[current_ppage].vpage].is_in_memory = 0;
            fetch_from_swap(vpage, current_ppage);
            // fprintf(stderr, "no information to fetch\n");
        }
        tlb.tlb_ppage = current_ppage;
        tlb.tlb_vpage = vpage;
        tlb.tlb_acces = EXEC_ACCESS + WRITE_ACCESS + READ_ACCESS;
        tlb.tlb_active = 1;
        _out(TLB_ADD_ENTRY, *(int*)(&tlb));
        // mise à jour du mapping des pages virtuelles 
        vm_mapping[vpage].ppage = current_ppage; // vpage mappée à la page current_ppage;
        vm_mapping[vpage].is_in_memory = 1; // vpage est désormais mappé en mémoire physique 

        pm_mapping[current_ppage].vpage = vpage;
        pm_mapping[current_ppage].is_mapped = 1; // on dit que curent_ppage a une page virtuelle
    }
    
    current_ppage++;
    if (current_ppage >= PM_PAGES)
        current_ppage = 1;
}

void init_mappings() {
    unsigned int i;
    // initialiser vm_mapping : toures les page
	for(i = 0; i < VM_PAGES; i++) {

		vm_mapping[i].is_in_memory = 0;
    }

	// initialiser pm_mapping à FAUX partout
	for(i = 0; i < PM_PAGES; i++) {
		pm_mapping[i].is_mapped = 0;
    }
}

void usage(char **argv) {
    fprintf(stderr, "Programme a échoué!\n");
    fprintf(stderr, "Usage: %s [arg1] [arg2]\n\n", argv[0]);
    fprintf(stderr, "* arg1 = 0 pour le handler qui utilise une seule page physique.\n");
    fprintf(stderr, "* arg1 = 1 pour le handler qui utilise toute la mémoire physique.\n\n");
    fprintf(stderr, "* arg2 = 0 pour tester l'addition des matrices.\n");
    fprintf(stderr, "* arg2 = 1 pour tester la multiplication des matrices.\n");
}

int main(int argc, char **argv) {
    unsigned int i;
    int operation;
    int arg1, arg2;

    /* init hardware */
    if (init_hardware(HARDWARE_INI) == 0) {
        fprintf(stderr, "Error in hardware initialization\n");
        exit(EXIT_FAILURE);
    }
    /* dummy interrupt handlers */
    for (i=0; i<IRQ_VECTOR_SIZE; i++)
        IRQVECTOR[i] = empty_it;

    if (init_swap() < 0) {
        fprintf(stderr, "Error in swap initialization\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 3) {
        arg1 = atoi(argv[1]);
        arg2 = atoi(argv[2]);
        
        if (arg1 == 0) 
            IRQVECTOR[MMU_IRQ] = mmuhandler_simple;  /* handler pour utiliser une simple page en mémoire */
        else if (arg1 == 1) 
            IRQVECTOR[MMU_IRQ] = mmuhandler_extended;  /* handler pour utiliser toute la mémoire physique */  
        else {
            usage(argv);
            exit(EXIT_FAILURE);
        }

        if (arg2 == MATRIX_ADD || arg2 == MATRIX_MUL)
            operation = arg2;
        else {
            usage(argv);
            exit(EXIT_FAILURE);
        }      
    }
    else {
        usage(argv);
        exit(EXIT_FAILURE);
    }

    init_mappings(); /* pour l'exercice 9: utilisation de toute la mémoire physique */
    
    /* activation du user mode et allows all IT */
    _mask(0x1001);
    
    /* user mode */
    user_process(operation);
    return 0;
}


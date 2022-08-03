#ifndef MI_KERNEL_H
#define MI_KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hardware.h>
#include "hw_config.h"
#include "swap.c"
#include "matrix.h"

#define EXEC_ACCESS  (1 << 2)
#define WRITE_ACCESS (1 << 1)
#define READ_ACCESS  (1 << 0)

#define VM_BEGIN *(unsigned*)&virtual_memory
#define VM_END (VM_BEGIN + VM_SIZE -1)

#define PM_BEGIN *(unsigned*)&physical_memory
#define PM_END (PM_BEGIN + PM_SIZE - 1)

/**
 * @brief Structure qui représente une entrée de la TLB.
 * Format des entrees par ordre decroissant de poids de bits.
 */
struct tlb_entry_s {
    /** R.F.U. -> 8 bits*/
    unsigned tlb_rfu :8;
    /** page virtuelle -> 12 bits*/
    unsigned tlb_vpage :12;
    /** page physique -> 8 bits*/
    unsigned tlb_ppage :8;
    /** accès exécution, écriture, lecture -> 3 bits*/
    unsigned tlb_acces :3;
    /** entrée utilisée ou non -> 1 bits*/
    unsigned tlb_active :1;
};

/**
 * structure pour associer une page physique à une page virtuelle 
 */
struct p_mapping {
    /** indque le numéro de la page virtuelle qui est mise dans la page physique */
	unsigned int vpage :12; 
	/** boolean: 1 indique si la page physique est reliée à une page virtuelle */
    unsigned int is_mapped;  
};

/**
 * structure pour associer une page virtuelle à une page physique 
 */
struct v_mapping {
    /** indique le numéro de la page physique où on a mis la page virtuelle */
	unsigned int ppage :8;
    /** boolean: 1 indique si la page virtuelle est déjà  mappée en mémoire */
	unsigned int is_in_memory;
};

typedef struct p_mapping p_mapping;
typedef struct v_mapping v_mapping;

/**
 * tableau des associations page_physique -> page virtuelle 
 */
extern struct p_mapping pm_mapping[];

/**
 * tableau des associations page_virtuelle -> page physique 
 */
extern struct v_mapping vm_mapping[];

#endif //MI_KERNEL_H
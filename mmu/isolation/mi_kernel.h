#ifndef MI_KERNEL_H
#define MI_KERNEL_H

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

#endif //MI_KERNEL_H
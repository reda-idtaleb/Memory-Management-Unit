/* memory isolation
   Philippe Marquet, Nov 2017
*/

#ifndef _MI_H_
#define _MI_H_

#define SYSCALL_SWTCH_0 16
#define SYSCALL_SWTCH_1 17

/**
 * @brief Passe alternativement l'execution du processus 0 
 * et du processus 1 en mode user
 */
void init(void);

#endif


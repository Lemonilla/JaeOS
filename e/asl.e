#ifndef ASL
#define ASL

/************************** ASL.E ******************************
*
*  The externals declaration file for the Active Semaphore List
*    Module.
*
*  Written by Mikeyg
*/

#include "../h/types.h"

// insert p into semd_t list where p_semAdd = semAdd
extern int insertBlocked (int *semAdd, pcb_t *p);
// remove the top pcb_t from semd_t where p_semAdd == semAdd
extern pcb_t* removeBlocked (int *semAdd);

// remove from middle of list
extern pcb_t* outBlocked (pcb_t *p);

extern pcb_t* headBlocked (int *semAdd);

extern void initASL ();

/***************************************************************/

#endif

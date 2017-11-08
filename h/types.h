/******************************TYPES.H*******************************
 * Written by Neal Troscinski, Timothy Wright, and MikeyG
 *
 * This header files contains definitions of types used 
 * in the project.
 *******************************************************************/

#ifndef TYPES
#define TYPES

#include "/usr/include/uarm/uARMtypes.h"


typedef unsigned int uint;

/* process table entry type */
typedef struct pcb_t {
        /* process queue fields */
    struct pcb_t   *p_next,     /* ptr to next entry     */
                   *p_prev;     /* ptr to previous entry */

    /* process tree fields */
    struct pcb_t   *p_prnt,     /* ptr to parent         */
                   *p_child,    /* ptr to 1st child      */
                   *p_sib;      /* ptr to next sibling   */

    /* process status information */
    state_t        p_s;         /* processor state       */
    int            *p_semAdd;   /* ptr to semaphore on   */
                                /* which proc is blocked */
    uint           p_cpuTime;   /* time on cpu.          */
                                /* uint > int in size.   */
    int            p_startTime;
                                /* 0: PGM_OLD */
                                /* 1: TLB_OLD */
                                /* 2: SYS_OLD */
                                /* 3: PGM_NEW */
                                /* 4: TLB_NEW */
                                /* 5: SYS_NEW */
    int            p_handlers[6];

}  pcb_t, *pcb_PTR;

/* semaphore descriptor type */
typedef struct semd_t {
    struct semd_t   *s_next;    /* next element on the ASL         */
    int             *s_semAdd;  /* pointer to the semaphore        */
    pcb_t           *s_tp;      /* tail pointer to a process queue */
} semd_t;






#endif

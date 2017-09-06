#ifndef TYPES
#define TYPES

/****************************************************************************
 *
 * This header file contains utility types definitions.
 *
 ****************************************************************************/

#include "/usr/include/uarm/uARMtypes.h"

/* process table entry type */
typedef struct pcb_t {
        /* process queue fields */
    struct pcb_t   *p_next,             /* ptr to next entry     */
                   *p_prev;             /* ptr to previous entry */

    /* process tree fields */
    struct pcb_t   *p_prnt,             /* ptr to parent         */
                   *p_child,            /* ptr to 1st child      */
                   *p_sib,              /* ptr to next sibling   */

    /* process status information */
    state_t        p_s;                 /* processor state       */
    int            *p_semAdd;           /* ptr to semaphore on   */
                                        /* which proc is blocked */
}  pcb_t, *pcb_PTR;



#endif

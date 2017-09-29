/*********************************ASL.C******************************
 * Written by Neal Troscinski and Timothy Wright
 *
 * This application manages a singularly linked and sorted list 
 * of Active Semaphores. The list is assorted in ascending order 
 * and has two dummy nodes. The first dummy node is at the head 
 * of the list and has a Semaphore Address of 0. The second dummy 
 * node is at the tail and has a Semaphore Address of MAXINT.
 ********************************************************************/

#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"
#include "../e/asl.e"

HIDDEN semd_t* freeList;
HIDDEN semd_t* asl;


/************************ Private Fucntions ************************/

/**** getFreeASL (private)
 * Gets a SEMD from the freelist.
 *
 * Parameters:
 * - semAdd  The semaphore address
 *           to initialize the SEMD to.
 *
 * End State:
 * - Returns NULL if the freelist is empty,
 *   otherwise returns a new SEMD.
 ****/
HIDDEN semd_t* getFreeASL(int* semAdd)
{
    if (freeList == NULL) return NULL;
    semd_t* ret = freeList;
    freeList = freeList->s_next;
    ret->s_next = NULL;
    ret->s_tp = mkEmptyProcQ();
    ret->s_semAdd = semAdd;
    return ret; 
}


/**** find (private)
 * Finds and returns the SEMD before the
 * SEMD with the specified semAdd, or
 * where the semAdd should be in the
 * sorted list.
 *
 * Parameters:
 * - semAdd  The semaphore address
 *           to find the previous of.
 *
 * End State:
 * - Returns the SEMD before the SEMD with
 *   the specified semAdd if it is in the 
 *   list.
 * - If the semAdd isn't in the list, it 
 *   returns the SEMD before where the 
 *   semAdd should appear.
 ****/
HIDDEN semd_t* find(int* semAdd)
{

    semd_t* a = asl;
    semd_t* b = asl->s_next;
    while (b->s_semAdd != NULL && b->s_semAdd < semAdd)
    {
        if (b->s_semAdd == MAXINT) break;
        a = b;
        b = b->s_next;
    }
    // b's semAdd is eol
    return a;
}


/**** freeASL (private)
 * Adds a SEMD to the freelist.
 *
 * Parameters:
 * - toFree  The SEMD to add to the
 *           freelist.
 *
 * End State:
 * - toFree is cleared and in the
 *   freelist.
 ****/
HIDDEN void freeASL(semd_t* toFree)
{
    toFree->s_next = freeList;
    toFree->s_semAdd = NULL;
    toFree->s_tp = NULL;
    freeList = toFree;
}


/************************* Public Fucntions ************************/

/**** insertBlocked
 * Inserts a PCB with semAdd into the 
 * proper list, adding a new SEMD at
 * the proper place if needed.
 *
 * Parameters:
 * - p       The PCB to add to the list.
 * - semAdd  The semaphore to add p to.
 *
 * End State:
 * - p is now added at a SEMD with a
 *   semaphore of semAdd.
 * - Returns TRUE if no SEMD is available
 *   to add when one is needed.  Returns
 *   FALSE in all other case.
 ****/
int insertBlocked (int *semAdd, pcb_t *p)
{
    semd_t* prev = find(semAdd);
    semd_t* sem = prev->s_next; 

    // case we need to add a SEMD
    if (sem->s_semAdd != semAdd)
    {
        sem = getFreeASL(semAdd);
        if (sem == NULL) return TRUE;
        sem->s_next = prev->s_next;
        prev->s_next = sem;
        sem->s_semAdd = semAdd;
    }

    // add the PCB
    p->p_semAdd = semAdd;
    insertProcQ(&(sem->s_tp),p);
    return FALSE;
}


/**** removeBlocked
 * Removes the top PCB from the SEMD
 * with a semaphore of semAdd and
 * return it.
 *
 * Parameters:
 * - semAdd  The semaphore to remove from.
 *
 * End State:
 * - Returns NULL if the SEMD with semAdd
 *   does not exist. Returns the removed 
 *   PCB otherwise.
 ****/
pcb_t* removeBlocked (int *semAdd)
{
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        pcb_t* ret = removeProcQ(&(prev->s_next->s_tp));

        // case SEMD is empty and needs to be removed
        if (emptyProcQ(prev->s_next->s_tp))
        {
            semd_t* tmp = prev->s_next;
            prev->s_next = prev->s_next->s_next;
            freeASL(tmp);
        }
        
        return ret;
    }
    // case SEMD doesn't exist
    return NULL;
}


/**** outBlocked
 * Removes the a PCB from the SEMD
 * with a semaphore of it's semAdd
 * and return it.
 *
 * Parameters:
 * - pcb     The PCB to remove.
 *
 * End State:
 * - Returns the PCB after removing it.
 * - Returns NULL if the PCB isn't in a
 *   SEMD's list.
 ****/
pcb_t* outBlocked (pcb_t *pcb)
{
    semd_t* prev = find(pcb->p_semAdd);
    if (prev->s_next->s_semAdd == pcb->p_semAdd)
    {
        pcb_t* ret = outProcQ(&(prev->s_next->s_tp),pcb);

        // remove the empty SEMD
        if (emptyProcQ(prev->s_next->s_tp))
        {
            semd_t* tmp = prev->s_next;
            prev->s_next = prev->s_next->s_next;
            freeASL(tmp);
        }
        return ret;
    }
    // case SEMD doesn't exist
    return NULL;
}


/**** headBlocked
 * Retuns the first PCB with a given
 * semaphore address.
 *
 * Parameters:
 * - semAdd  The address of the semaphore
 *           that the SEMD has that should
 *           returns it's first PCB.
 *
 * End State:
 * - Returns the PCB of the SEMD with a 
 *   semaphore of semAdd.
 * - Returns NULL if the SEMD doesn't exist.
 ****/
pcb_t* headBlocked (int *semAdd)
{
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        pcb_t* ret = headProcQ(prev->s_next->s_tp);
        return ret;
    }
    return NULL;
}


/**** aslEmpty
 * Checks to see if the ASL is empty.
 *
 * Parameters:
 * - head    The head of the ASL to check.
 *
 * End State:
 * - Returns TRUE if the ASL is empty.
 *   Returns FALSE otherwise.
 ****/
int aslEmpty (semd_t** head) 
{
    return (*head)->s_next->s_semAdd == MAXINT;
}


/**** initASL
 * Initializes the SEMDs and inserts them
 * into the freelist.
 * Sets up the ASL to have two dummy nodes
 * with semAdd of 0 and MAXINT.
 * Should only be run once.
 *
 * Parameters:
 *
 * End State:
 * - The freelist is populated with MAXPROC
 *   empty SEMDs.
 * - The ASL is populated with two dummy
 *   nodes with semAdd of 0 and MAXINT.
 ****/
void initASL()
{
    static semd_t aslList[MAXPROC+2];
    freeList = &aslList;

    // enque each asl onto the free list
    for(int i = 0; i < MAXPROC-1; i++)
    {
        aslList[i].s_next = &aslList[i+1];
    }
    aslList[MAXPROC-1].s_next = NULL;

    // init asl
    asl = &freeList[MAXPROC];
    asl->s_next = &freeList[MAXPROC+1];
    asl->s_semAdd = 0;
    asl->s_next->s_semAdd = MAXINT;
}


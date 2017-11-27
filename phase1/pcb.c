/*******************************PCB.C********************************
 * Written by Neal Troscinski and Timothy Wright
 *
 * This application manages a circular doubly linked queue of 
 * Process Control Blocks.  Each queue is referenced by its tail 
 * pointer as to allow for the insert and remove operations to be 
 * a constant time operation. It is doubly linked so that we may 
 * insert and remove from the middle of the queue.
 *******************************************************************/

#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"

HIDDEN pcb_t* freeList;


/************************ Private Fucntions ************************/

/**** insertSibling (private)
 * Recursivly walks down a sibling tree
 * until the end to insert a child.
 *
 * Parameters:
 * - sister  A pointer to the child to check
 *           for siblings.
 * - baby    The Process Control Block to
 *           insert as a child.
 *
 * End State:
 * - Adds the PCB baby to the end of the
 *   sibling list that PCB sister is in.
 ****/
HIDDEN void insertSibling (pcb_t* sister, pcb_t* baby)
{
    // if no other siblings we're at the end
    if (sister->p_sib == NULL)
    {
        // make baby a sibling of sister
        sister->p_sib = baby;
        return;
    }

    // recursively walk down the list
    insertSibling(sister->p_sib, baby);
    return;
}


/************************ Public Fucntions *************************/

/**** freePcb
 * Inserts a pcb into the freelist.
 *
 * Parameters:
 * - p       The process control block
 *           to add to the freelist.
 *
 * End State:
 * - p is on the freelist.
 ****/
void freePcb (pcb_t* p)
{
    insertProcQ(&freeList, p);
}


/**** allocPcb
 * Remove a Process Control Block off the
 * freelist and return it.v
 *
 * Parameters:
 *
 * End State:
 * - NULL returned if freelist is empty.
 * - Remove and return an empty Process
 *   Control Block from the freelist.
 ****/
pcb_t* allocPcb ()
{
    // If the freelist is empty return NULL
    if (emptyProcQ(freeList)) return NULL;

    // grab an unused pcb_t
    pcb_t* tmp = removeProcQ(&freeList);

    // NULL out all of the fields so we 
    // can insert into the PCB Queue
    tmp->p_next = NULL;
    tmp->p_prev = NULL;
    tmp->p_prnt = NULL;
    tmp->p_child = NULL;
    tmp->p_sib = NULL;
    tmp->p_semAdd = NULL;
    tmp->p_s.a1 = NULL;
    tmp->p_s.a2 = NULL;
    tmp->p_s.a3 = NULL;
    tmp->p_s.a4 = NULL;
    tmp->p_s.v1 = NULL;
    tmp->p_s.v2 = NULL;
    tmp->p_s.v3 = NULL;
    tmp->p_s.v4 = NULL;
    tmp->p_s.v5 = NULL;
    tmp->p_s.v6 = NULL;
    tmp->p_s.sl = NULL;
    tmp->p_s.fp = NULL;
    tmp->p_s.ip = NULL;
    tmp->p_s.sp = NULL;
    tmp->p_s.lr = NULL;
    tmp->p_s.pc = NULL;
    tmp->p_s.cpsr = NULL;
    tmp->p_s.CP15_Control = NULL;
    tmp->p_s.CP15_EntryHi = NULL;
    tmp->p_s.CP15_Cause = NULL;
    tmp->p_s.TOD_Hi = NULL;
    tmp->p_s.TOD_Low = NULL;

    // Return the node off the Free List
    return tmp;
}


/**** initPcbs
 * Initialize the Process Control Blocks
 * and insert them into the freelist.
 * Should only be run once!
 *
 * Parameters:
 *
 * End State:
 * - A number of Process Control Blocks
 *   are added to the freelist defined by
 *   the constant MAXPROC from const.h
 ****/
void initPcbs ()
{
    // initialize the pcbs
    static pcb_t pcbList[MAXPROC];
    freeList = (pcb_t*) &pcbList;

    // enqueue each pcb onto the free list
    for(int i = 0; i < MAXPROC; i++)
    {
        pcbList[i].p_id = i;
        freePcb((pcb_t*)&pcbList[i]);
    }
}


/**** mkEmptyProcQ
 * Creates an empty queue of Process 
 * Control Blocks.
 *
 * Parameters:
 *
 * End State:
 * - Returns an empty que of Process
 *   Control Blocks.
 ****/
pcb_t* mkEmptyProcQ ()
{
    return NULL;
}


/**** emptyProcQ
 * Checks if the Process Control Block
 * queue passed in is emtpy or not.
 *
 * Parameters:
 * - tp      A pointer to the tail of the
 *           Process Control Block queue
 *           to check if empty.
 *
 * End State:
 * - Returns TRUE if the Process Control 
 *   Block queue is empty.  Otherwise
 *   returns FALSE.
 ****/
int emptyProcQ (pcb_t* tp)
{
    return (tp == mkEmptyProcQ());
}


/**** insertProcQ
 * Inserts a Process Control Block into a
 * Process Control Block queue.
 *
 * Parameters:
 * - p       A pointer to the Pocess Control 
 *           Block to enqueue.
 * - tp      A pointer to the tail of the
 *           Process Control Block queue
 *           to enqueue into.
 *
 * End State:
 * - The Process Control Block is the new
 *   tail of the Process Control Block queue.
 ****/
void insertProcQ (pcb_t** tp, pcb_t* p)
{
    // case 0:  procQ is empty
    if (emptyProcQ(*tp))
    {
        (*tp) = p;
        p->p_next = p;
        p->p_prev = p;
        return;
    }
    // case 1: procQ has 1 or more elements
    p->p_next = (*tp)->p_next;
    p->p_prev = (*tp);
    (*tp)->p_next->p_prev = p;
    (*tp)->p_next = p;
    (*tp) = p;
    return;
}


/**** removeProcQ
 * Removes the next Process Control Block in the
 * Process Control Block queue specified and
 * returns a pointer to the removed block.
 *
 * Parameters:
 * - tp      A pointer to the tail of the
 *           Process Control Block queue
 *           to dequeue from.
 *
 * End State:
 * - The Process Control Block that was the head
 *   of tp is removed from the queue.
 * - The old head of the PCB queue is returned.
 * - NULL is returned if the PCB queue is empty.
 ****/
pcb_t* removeProcQ (pcb_t** tp)
{
    // case 0: PCB queue is empty
    if (emptyProcQ(*tp))
    {
        return NULL;
    }

    pcb_t* ret = (*tp)->p_next;

    // case 1: There is only one PCB in the queue
    if (ret == (*tp))
    {
        (*tp) = NULL;
        return ret;
    }

    // case 2+
    (*tp)->p_next->p_next->p_prev = (*tp);
    (*tp)->p_next = (*tp)->p_next->p_next;
    ret->p_next = NULL;
    ret->p_prev = NULL;
    return ret;
}


/**** outProcQ
 * Removes a specific Process Control Block 
 * from the specified Process Control Block 
 * queue specified and returns a pointer to 
 * the removed block.
 *
 * Parameters:
 * - p       A pointer to the Pocess Control 
 *           Block to remove.
 * - tp      A pointer to the tail of the
 *           Process Control Block queue
 *           to remove it from.
 *
 * End State:
 * - The Process Control Block specified by p
 *   is removed from the queue specified by tp.
 * - The removed Process Control Block is returned.
 * - NULL is returned if the specified Process
 *   Control Block is not in the specified queue
 *   or the specified queue is empty.
 ****/
pcb_t* outProcQ (pcb_t** tp, pcb_t* p)
{
    // case 0
    if ((*tp) == NULL) return NULL;

    // case 1+
    pcb_t* working = (*tp);

    // look at entire (*tp) since we can't assume that p is in (*tp)
    while (working != p)
    {
        working = working->p_next;
        // not in list
        if (working == (*tp)) return NULL;
    }
    if (*tp == p)
    {
        // case 2+
        *tp = p->p_next;

        // case 1
        if (*tp == p) 
            *tp = NULL;
    }
    working->p_next->p_prev = working->p_prev;
    working->p_prev->p_next = working->p_next;
    working->p_next = NULL;
    working->p_prev = NULL;
    return working;
}


/**** headProcQ
 * Returns a pointer to the head of the specified
 * Process Control Block queue.
 *
 * Parameters:
 * - tp      A pointer to the tail of the
 *           Process Control Block queue
 *           to look at.
 *
 * End State:
 * - The Process Control Block at the head of
 *   the queue is returned.
 * - NULL is returned if the specified queue 
 *   is empty.
 ****/
pcb_t* headProcQ (pcb_t* tp)
{
    // if empty return NULL
    if (emptyProcQ(tp)) return NULL;

    // otherwise return the head
    return (tp->p_next);
}


/**** emptyChild
 * Checks to see if a given Process Control
 * Block is childless.
 *
 * Parameters:
 * - p       A pointer to the Process Control
 *           Block to check for children.
 *
 * End State:
 * - TRUE is returned if the process has 
 *   no children.
 * - FALSE is returned if it has them.
 ****/
int emptyChild (pcb_t* p)
{
    if (p==NULL) return 0;
    return !(p->p_child);
}


/**** insertChild
 * Inserts a Process Control Block as a
 * child of a given Process Control Block.
 * Inserts at the end of the children list.
 * Uses the helper function insertSibling to
 * insert at the end of the list.
 *
 * Parameters:
 * - p       The PCB to insert into the family.
 * - prnt    The PCB of which to make p a child of.
 *
 * End State:
 * - The PCB p is now a child of PCB prnt.
 ****/
void insertChild (pcb_t* prnt, pcb_t* p)
{
    p->p_prnt = prnt;

    // if there are no other children
    if (emptyChild(prnt)) {
        prnt->p_child = p;
        return;
    }

    // otherwise insert new child at end of queue
    insertSibling(prnt->p_child, p);
    return;
}


/**** removeChild
 * Removes the first child of a given Process
 * Control Block.
 *
 * Parameters:
 * - p       The PCB to remove a child from.
 *
 * End State:
 * - Returns the disowned child.
 * - NULL is returned if PCB p doesn't have any
 *   children to disown.
 ****/
pcb_t* removeChild (pcb_t* p)
{
    // Case p has no children
    if (p->p_child == NULL) 
        return NULL;

    // remove first child
    pcb_t* child = p->p_child;
    p->p_child = child->p_sib;
    child->p_prnt = NULL;
    return child;
}


/**** outChild
 * Disowns a specified Process Control Block
 * from it's parent if it has one.
 *
 * Parameters:
 * - p       The PCB to disown.
 *
 * End State:
 * - Returns the disowned child.
 * - NULL is returned if PCB p doesn't have
 *   a parent.
 ****/
pcb_t* outChild (pcb_t* p)
{
    // case p was never a childn :(
    if (p->p_prnt == NULL) 
        return NULL;

    // case p is direct child of parent
    if (p->p_prnt->p_child == p)
    {
        p->p_prnt->p_child = p->p_sib;
        p->p_prnt = NULL;
        return p;
    }

    // case p is not direct child
    pcb_t* tmp = p->p_prnt->p_child;
    while (tmp->p_sib != p)
    {
        tmp = tmp->p_sib;
    }
    tmp->p_sib = p->p_sib;
    p->p_prnt = NULL;
    p->p_sib = NULL;
    return p;
}

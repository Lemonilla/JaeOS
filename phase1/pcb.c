#include "../h/types.h"
#include "../e/pcb.e"

// Insert into the free list
extern void freePcb (pcb_t* p)
{
    insertProcQ(freeList, p)
}

// Return NULL if the pcbFree list is empty. Otherwise, remove
// an element from the pcbFree list, provide initial values for ALL
// of the ProcBlk’s fields (i.e. NULL and/or 0) and then return a
// pointer to the removed element. ProcBlk’s get reused, so it is
// important that no previous values persist in a ProcBlk when it
// gets reallocated.
extern pcb_t* allocPcb ()
{
    if (emptyProcQ(freeList)) return NULL;
    pcb_t* tmp = removeProcQ(freeList);
    tmp->p_next = NULL;
    tmp->p_prev = NULL;
    tmp->p_prnt = NULL;
    tmp->p_child = NULL;
    tmp->p_sib = NULL;
    tmp->p_s = NULL; // REPLACE THIS LATER ONCE We KNOW WHAT A STATE_T IS
    TMP->p_semAdd = NULL;
    return tmp;
}

// Initialize the pcbFree list to contain all the elements of
// the static array of MAXPROC ProcBlk’s. This method will be called
// only once during data structure initialization.
extern void initPcbs ()
{
    static pcb_t[MAXPROC] pcbList;
    static pcb_t* freeList = &pcbList;
    // enque each pcb onto the free list
    for(int i = 0; i < MAXPROC; i++){
        freePcb(&pcbList + (i*sizeof(pcb_t)))
    }
}

// creates an empty queue
extern pcb_t* mkEmptyProcQ ()
{
    return NULL;
}

// returns TRUE if the queue is empty
extern bool emptyProcQ (pcb_t* tp)
{
    return (tp==mkEmptyProcQ());
}

// Inserts a pcb node into a pcb queue
extern void insertProcQ (pcb_t** tp, pcb_t* p)
{
    // case 0
    if emptyProcQ(*tp) 
    {
        *tp = p;
        p->p_next = p;
        p->p_prev = p;
        return;
    }
    // case 1+
    p->p_next = *tp->p_next
    *tp->p_next = p;
    p->p_prev = *tp;
    p->p_next->p_prev = p;
    *tp = p;
    return;
}

// removes the top node from the pcb queue
extern pcb_t* removeProcQ (pcb_t** tp)
{
    // case 0
    if emptyProcQ(*tp)
    {
        return NULL
    }
    void* ret = *tp->p_next;
    // case 1
    if (ret == *tp)
    {
        *tp = NULL;
        return ret;
    }
    // case 2+
    *tp->p_next->p_next->p_prev = *tp;
    *tp->p_next = *tp->p_next->p_next;
    ret->p_next = NULL;
    ret->p_prev = NULL;
    return ret;

}

// removes a given node from the pcb queue
// returns p or null p isn't in the list
extern pcb_t* outProcQ (pcb_t** tp, pcb_t* p)
{
    
}

// returns a pointer to the top of the pcb queue
// or null if the list is empty
extern pcb_t* headProcQ (pcb_t** tp)
{
    if (emptyProcQ()) return NULL;
    return *tp->p_next;
}

//
extern int emptyChild (pcb_t* p)
{
    if (p==NULL) return NULL;
    return !(p->p_child);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////LEFT OFF HERE////////////////////////////////////////
extern void insertChild (pcb_t* prnt, pcb_t* p)
{
    p->p_prnt = prnt;
    if (emptyChild(prnt)) {
        prnt->p_child = p;
        return;
    }
    insertSibling(prnt->p_child, p);
    return;
}

// recursivly walk down siblings tree
extern void insertSibling (pcb_t* brother, pcb_t* baby)
{
    if (brother->p_sib == NULL)
    {
        brother->p_sib = baby;
        return;
    }
    insertSibling(brother->p_sib, baby);
    return;
}

extern pcb_t* removeChild (pcb_t* p)
{
    
}

extern pcb_t* outChild (pcb_t* p)
{
    
}

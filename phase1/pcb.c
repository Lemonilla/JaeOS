#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"


HIDDEN pcb_t* freeList;
HIDDEN pcb_t* tp;

void debuga(int a, int b, int c, int d)
{
    int i=42;
    i++;
}

// Insert into the free list
void freePcb (pcb_t* p)
{
    insertProcQ(&freeList, p);
}

// Return NULL if the pcbFree list is empty. Otherwise, remove
// an element from the pcbFree list, provide initial values for ALL
// of the ProcBlk’s fields (i.e. NULL and/or 0) and then return a
// pointer to the removed element. ProcBlk’s get reused, so it is
// important that no previous values persist in a ProcBlk when it
// gets reallocated.
pcb_t* allocPcb ()
{
    if (emptyProcQ(freeList)) return NULL;
    pcb_t* tmp = removeProcQ(&freeList);
    tmp->p_next = NULL;
    tmp->p_prev = NULL;
    tmp->p_prnt = NULL;
    tmp->p_child = NULL;
    tmp->p_sib = NULL;
   // tmp->p_s = NULL; // REPLACE THIS LATER ONCE WE KNOW WHAT IT'S USED FOR
   // tmp->p_semAdd = NULL; // REPLACE THIS LATER ONCE WE KNOW WHAT IT'S USED FOR
    return tmp;
}

// Initialize the pcbFree list to contain all the elements of
// the static array of MAXPROC ProcBlk’s. This method will be called
// only once during data structure initialization.
void initPcbs ()
{
    static pcb_t pcbList[MAXPROC];
    freeList = (pcb_t*) &pcbList;
    // enque each pcb onto the free list
    for(int i = 0; i < MAXPROC; i++)
    {
        pcbList[i].debug = i;
        freePcb((pcb_t*)&pcbList[i]);
    }
}

// creates an empty queue
pcb_t* mkEmptyProcQ ()
{
    return NULL;
}

// returns TRUE if the queue is empty
int emptyProcQ (pcb_t* tp)
{
    return (tp==mkEmptyProcQ());
}

// Inserts a pcb node into a pcb queue
void insertProcQ (pcb_t** tp, pcb_t* p)
{
    // case 0
    if (emptyProcQ(*tp))
    {
        (*tp) = p;
        p->p_next = p;
        p->p_prev = p;
        return;
    }
    // case 1+
    p->p_next = (*tp)->p_next;
    (*tp)->p_next = p;
    p->p_prev = (*tp);
    p->p_next->p_prev = p;
    (*tp) = p;
    return;
}

// Removes the top node from the pcb queue
// and returns it
pcb_t* removeProcQ (pcb_t** tp)
{
    // case 0
    if (emptyProcQ(*tp))
    {
        return NULL;
    }
    pcb_t* ret = (*tp)->p_next;
    // case 1
    if (ret == (*tp))
    {
        (*tp) = NULL;
        return ret;
    }
    // case 2
    if ((*tp)->p_next->p_next == (*tp))
    {
        (*tp)->p_next = (*tp);
        (*tp)->p_prev = (*tp);
        ret->p_next = NULL;
        ret->p_prev = NULL;
        return ret;
    }

    // case 3+
    (*tp)->p_next->p_next->p_prev = (*tp);
    (*tp)->p_next = (*tp)->p_next->p_next;
    ret->p_next = NULL;
    ret->p_prev = NULL;
    return ret;
}

// removes a given node from the pcb queue
// returns p or null p isn't in the list
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
        if (working == (*tp)) return NULL; // not in list
    }
    working->p_next->p_prev = working->p_prev;
    working->p_prev->p_next = working->p_next;
    working->p_next = NULL;
    working->p_prev = NULL;
    return working;
}

// returns a pointer to the top of the pcb queue
// or null if the list is empty
pcb_t* headProcQ (pcb_t* tp)
{
    if (emptyProcQ(tp)) return NULL;
    return (tp->p_next);
}

// returns TRUE if p has children
// returns NULL if p is a null pointer
int emptyChild (pcb_t* p)
{
    if (p==NULL) return 0;
    return !(p->p_child);
}

// Makes a pcb p into a child of pcb prnt
// AS THE LAST CHILD IN THE LINKED LIST
void insertChild (pcb_t* prnt, pcb_t* p)
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
void insertSibling (pcb_t* sister, pcb_t* baby)
{
    if (sister->p_sib == NULL)
    {
        sister->p_sib = baby;
        return;
    }
    insertSibling(sister->p_sib, baby);
    return;
}

// Removes the first child of pcb p if it exists
// and returns that child or null
pcb_t* removeChild (pcb_t* p)
{
    if (p->p_child == NULL) return NULL;
    pcb_t* child = p->p_child;
    p->p_child = child->p_sib;
    child->p_prnt = NULL;
    return child;
}

// Disown a child p from it's parents and siblings
pcb_t* outChild (pcb_t* p)
{
    // case no parents
    if (p->p_prnt == NULL) return NULL;

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

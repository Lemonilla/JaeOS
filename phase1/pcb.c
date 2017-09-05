#include "../h/types.h"
#include "../e/pcb.e"


extern void freePcb (pcb_t* p)
{
    
}

extern pcb_t* allocPcb ()
{
    
}

extern void initPcbs ()
{
    static pcb_t[MAXPROC] freeList;
}


extern pcb_t* mkEmptyProcQ ()
{
    
}

// returns TRUE if the queue is empty
extern bool emptyProcQ (pcb_t* tp)
{
    return (tp==null);
}

// Inserts a pcb node into a pcb queue
extern void insertProcQ (pcb_t** tp, pcb_t* p)
{
    // case 0
    if emptyProcQ(*tp) 
    {
        *tp = p;
        p->next = p;
        p->prev = p;
        return;
    }
    // case 1+
    p->next = *tp->next
    *tp->next = p;
    p->prev = *tp;
    p->next->prev = p;
    *tp = p;
    return;
}

// removes the top node from the pcb queue
extern pcb_t* removeProcQ (pcb_t** tp)
{
    // case 0
    if emptyProcQ(*tp)
    {
        return null
    }
    void* ret = *tp->next;
    // case 1
    if (ret == *tp)
    {
        *tp = null;
        return ret;
    }
    // case 2+
    *tp->next->next->prev = *tp;
    *tp->next = *tp->next->next;
    ret->next = null;
    ret->prev = null;
    return ret;

}

// removes a given node from the pcb queue
extern pcb_t* outProcQ (pcb_t** tp, pcb_t* p)
{
    
}

extern pcb_t* headProcQ (pcb_t* tp)
{
    
}


extern int emptyChild (pcb_t* p)
{
    
}

extern void insertChild (pcb_t* prnt, pcb_t* p)
{
    
}

extern pcb_t* removeChild (pcb_t* p)
{
    
}

extern pcb_t* outChild (pcb_t* p)
{
    
}

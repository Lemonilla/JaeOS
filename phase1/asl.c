#include "../h/types.h"
#include "../h/const.h"
#include "../e/asl.e"

HIDDEN semd_t* freeList;
HIDDEN semd_t* asl;

// insert p into semd_t list where s_semAdd = semAdd
extern int insertBlocked (int *semAdd, pcb_t *p)
{
    
}

// remove the top pcb_t from semd_t where s_semAdd == semAdd
extern pcb_t* removeBlocked (int *semAdd)
{
    
}

// remove from middle of list
extern pcb_t* outBlocked (pcb_t *p)
{
    
}

// view top pcb_t of a given semaphore
extern pcb_t* headBlocked (int *semAdd)
{
    semd_t* p = asl;
    while (p->s_semAdd != semAdd){
        p = p->s_next;
    }
    return p->s_tp;
}

int aslEmtpy (semd_t** head) 
{
    return (*head)->s_next->s_semAdd == MAXINT;
}

extern void initASL ()
{
    static semd_t aslList[MAXPROC+2];
    freeList = &aslList;
    // enque each asl onto the free list
    for(int i = 0; i < MAXPROC; i++)
    {
        insertBlocked(&aslList + (i*sizeof(semd_t)),NULL);
    }
    insertBlocked(0,NULL);
    insertBlocked(MAXINT,NULL);
}
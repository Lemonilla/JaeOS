#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"
#include "../e/asl.e"

HIDDEN semd_t* freeList;
HIDDEN semd_t* asl;


HIDDEN semd_t* getFreeASL(int* semAdd)
{
    if (freeList == NULL) return NULL;
    semd_t* ret = freeList;
    freeList = freeList->s_next;
    ret->s_next = NULL;
    return ret; 
}

semd_t* find(int* semAdd)
{
    semd_t* a = asl;
    semd_t* b = asl->s_next;
    while (b->s_semAdd != NULL && b->s_semAdd < semAdd)
    {
        a = b;
        b = b->s_next;
    }
    // b's semAdd is eol
    return a;
}

// insert p into semd_t list where s_semAdd = semAdd
int insertBlocked (int *semAdd, pcb_t *p)
{
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        insertProcQ(prev->s_next->s_tp,p);
        return TRUE;
    }
    semd_t* newSem = getFreeASL(semAdd);
    if (newSem == NULL) return FALSE;
    newSem->s_next = prev->s_next;
    prev->s_next = newSem;
    insertProcQ(newSem->s_tp,p);
    return TRUE;
}

// remove the top pcb_t from semd_t where s_semAdd == semAdd
pcb_t* removeBlocked (int *semAdd)
{
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        return removeProcQ(prev->s_next->s_tp);
    }
    return NULL;
}

// remove from middle of a list
pcb_t* outBlocked (pcb_t *pcb)
{
    semd_t* prev = find(pcb->p_semAdd);
    if (prev->s_next->s_semAdd == pcb->p_semAdd)
    {
        return outProcQ(prev->s_next->s_tp, pcb);
    }
    return NULL;
}

// view top pcb_t of a given semaphore
pcb_t* headBlocked (int *semAdd)
{
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        return headProcQ(prev->s_next->s_tp);
    }
    return NULL;
}

int aslEmtpy (semd_t** head) 
{
    return (*head)->s_next->s_semAdd == MAXINT;
}

void initASL()
{
    static semd_t aslList[MAXPROC+2];
    freeList = &aslList;
    // enque each asl onto the free list
    for(int i = 0; i < MAXPROC; i++)
    {
        
    }
}
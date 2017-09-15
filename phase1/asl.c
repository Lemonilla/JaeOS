#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"
#include "../e/asl.e"

HIDDEN semd_t* freeList;
HIDDEN semd_t* asl;


void debug (int a, int b, int c, int d)
{
    int i=0;
    i++;
}


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

HIDDEN semd_t* find(int* semAdd)
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

HIDDEN void freeASL(semd_t* toFree)
{
    toFree->s_next = freeList;
    freeList = toFree;

}

// insert p into semd_t list where s_semAdd = semAdd
// returns TRUE if the pcb can't be added
int insertBlocked (int *semAdd, pcb_t *p)
{
    debuga(0x10,(int)semAdd,p->debug,0);
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        insertProcQ(prev->s_next->s_tp,p);
        return FALSE;
    }
    semd_t* newSem = getFreeASL(semAdd);
    if (newSem == NULL) return TRUE;
    newSem->s_next = prev->s_next;
    prev->s_next = newSem;
    insertProcQ(&newSem->s_tp,p);
    return FALSE;
}

// remove the top pcb_t from semd_t where s_semAdd == semAdd
pcb_t* removeBlocked (int *semAdd)
{
    debuga(0x20,(int)semAdd,0,0);
    semd_t* prev = find(semAdd);
    if (prev->s_next->s_semAdd == semAdd)
    {
        debuga(0xFF,prev->s_next->s_tp,0,0);
        pcb_t* ret = removeProcQ(prev->s_next->s_tp);
        debuga(0xFF,prev->s_next->s_tp,0,0);
        if (emptyProcQ(prev->s_next->s_tp))
        {
            prev->s_next = prev->s_next->s_next;
            freeASL(prev->s_next);
        }
        return ret;
    }
    debug(0xBF,0,0,0);
    return NULL;
}

// remove from middle of a list
pcb_t* outBlocked (pcb_t *pcb)
{
    semd_t* prev = find(pcb->p_semAdd);
    debuga(0x20,(int)prev->s_next->s_semAdd,pcb->debug,0);
    if (prev->s_next->s_semAdd == pcb->p_semAdd)
    {
        pcb_t* ret = outProcQ(prev->s_next->s_tp, pcb);
        if (emptyProcQ(prev->s_next->s_tp))
        {
            prev->s_next = prev->s_next->s_next;
            freeASL(prev->s_next);
        }
        return ret;
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
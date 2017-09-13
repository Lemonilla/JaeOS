#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"
#include "../e/asl.e"

HIDDEN semd_t* freeList;
HIDDEN semd_t* asl;


semd_t* insertASL(int* semAdd)
{
    if (freeList == NULL) return NULL;
    semd_t* ret = freeList;
    freeList = freeList->s_next;
    ret->s_next = NULL;
    return ret; 
}

// insert p into semd_t list where s_semAdd = semAdd
int insertBlocked (int *semAdd, pcb_t *p)
{
    semd_t* temp_semAdd = asl;
    semd_t* temp_semD = asl->s_next;
    while (semAdd != temp_semAdd->s_semAdd)
    {
        // we found a missing semd_t
        if (semAdd < temp_semD->s_semAdd)
        {
            semd_t* new_semd = insertASL(semAdd); // add new semd_t
            if (new_semd == NULL) return FALSE;
            new_semd->s_next = temp_semD;
            temp_semAdd->s_next = new_semd;
            insertProcQ(new_semd->s_tp, p);
            return TRUE;
        }
    // advance
    temp_semAdd = temp_semD->s_next;
    temp_semD = temp_semD->s_next;
    }
    // we found the actual thing @ temp_semAdd
    insertProcQ(temp_semAdd->s_tp, p);
    return TRUE;
}

// remove the top pcb_t from semd_t where s_semAdd == semAdd
pcb_t* removeBlocked (int *semAdd)
{
    semd_t* p = asl;
    while (p->s_semAdd != MAXINT && p->s_semAdd != semAdd)
    {
        p = p->s_next;
    }
    // we're at semAdd or MAXINT now
    if (p->s_semAdd == MAXINT) return NULL;
    return removeProcQ(p->s_tp);
}

// remove from middle of list
pcb_t* outBlocked (pcb_t *pcb)
{
    semd_t* p = asl;
    pcb_t* ret;
    while ((ret = outProcQ(p->s_tp,pcb)) == NULL)
    {
        if (p->s_next == NULL) 
        {
            return NULL;
        }
        p = p->s_next;
    }
    return ret;
}

// view top pcb_t of a given semaphore
pcb_t* headBlocked (int *semAdd)
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

void initASL()
{
    static semd_t aslList[MAXPROC+2];
    freeList = &aslList;
    // enque each asl onto the free list
    for(int i = 0; i < MAXPROC; i++)
    {
        
    }
}
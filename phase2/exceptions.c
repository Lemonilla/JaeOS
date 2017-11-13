// exceptions.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/interrupt.e"
#include "../e/scheduler.e"

HIDDEN void passUpOrDie(int offset)
{
    if (currentProc->p_handlers[offset+CUSTOM_HANDLER_NEW_OFFSET] == NULL)
    {
        // if we haven't specified what to do here, GLASS 'EM
        sys2();
    }
    // they have a custom handler
    // pass off the problem to that

    // copy state into old state
    uint toCopyFrom;
    switch(offset)
    {
        case CUSTOM_PGM: toCopyFrom=PGMTOLD; break;
        case CUSTOM_TLB: toCopyFrom=TLBOLD; break;
        case CUSTOM_SYS: toCopyFrom=SYSOLD; break;
    }
    currentProc->p_handlers[offset] = toCopyFrom;

    // load new state
    LDST(currentProc->p_handlers[offset+CUSTOM_HANDLER_NEW_OFFSET]); 
}

HIDDEN void killGeneration(pcb_t* p)
{
    pcb_t* tmp;
    pcb_t* removedProc;
    while(p != NULL)
    {
        // if we have a child, kill that generation
        if (!emptyChild(p))
        {
            killGeneration(removeChild(p));
        }

        // it's just 
        if (p == currentProc)
        {
            currentProc = NULL;
            outChild(p);
        }

        // it's in readyQ
        if (p->p_semAdd == NULL)
        {
            outProcQ(&readyQ,p);
        }

        // blocked on ASL
        if (p->p_semAdd != NULL)
        {
            // we're softblocked
            if (p->p_semAdd >= &devSem[0] && p->p_semAdd <= &devSem[DEVICESPERLINE*NUMOFDEVICELINES])
            {
                softBlockCount--;
            }
            else 
            {
                *(p->p_semAdd)++;
            }
            outBlocked(p);
        }

        processCount--;

        // Free it for use later
        freePcb(p);
    }
}

void pgmHandle()
{
    debug(0x11,0,0,0);
    passUpOrDie(CUSTOM_PGM);
}

void tlbHandle()
{
    debug(0x12,0,0,0);
    passUpOrDie(CUSTOM_TLB);
}

sysHandle()
{    
    // update currentProc state
    copyState(&(currentProc->p_s),(state_t*) SYSOLD);

    debug(0x13,currentProc->p_s.a1,0,0);

    // if in kernal mode
    if (currentProc->p_s.cpsr & SYS_MODE == SYS_MODE)
        switch(currentProc->p_s.a1) // on syscall arg
        {
            case 1: // - BIRTH NEW PROC
                sys1();
            case 2: // - GLASS 'EM
                sys2();  
            case 3: // - SIGNAL
                sys3();
            case 4: // - WAIT
                sys4();
            case 5: // - SET NEW HANDLERS
                sys5();
            case 6: // - CPU TIME
                sys6();
            case 7: // - WAIT A BIT
                sys7();
            case 8: // - WAIT FOR I/O
                sys8();
            default: // - Pass up or die
                // do we have a custom handler for this?
                passUpOrDie(CUSTOM_SYS);
        }
    // if in User Mode
    // save old state in PGTold
    state_t* programTrapOld = (uint) PGMTOLD;
    state_t* sysHandlerOld = (uint) SYSOLD;
    programTrapOld = sysHandlerOld;

    // update cause to RI (reserved instruction)
    programTrapOld->CP15_Cause = ALLOFF | RESERVED_INSTRUCTION_CODE;

    // goto program trap handler
    state_t* programTrapNew = (uint) PGMTNEW;
    LDST(programTrapNew);
}

void sys1() // Babies
{
debug(0x13,0xFF,1,0);
    // make new proc
    pcb_t* p = allocPcb();

    // update currentProc's state
    copyState(&(currentProc->p_s),SYSOLD);

    // default we can't make a child ;_;
    currentProc->p_s.a1 = -1; // return error code

    // if we can, fix return value and make the child
    if (p != NULL)
    {
        currentProc->p_s.a1 = 0; // return OK code
        insertChild(currentProc,p); // make it a child of currentProc
    }

    // resume execution
    LDST(&(currentProc->p_s));
}

void sys2() // GLASS 'EM
{
debug(0x13,0xFF,2,0);
    // kill a proc & all it's children
    pcb_t* p = currentProc;
    if (p->p_child != NULL) 
    {
        killGeneration(p->p_child);
    }
    // p can't be in a queue since it's currentProc
    // so we can just kill it
    freePcb(p);
    
    scheduler();
}

void sys3() // signal
{
    debug(0x13,0xFF,3,0);
    copyState(&(currentProc->p_s),SYSOLD);

    int* semAdd = currentProc->p_s.a2;

    (*semAdd)++;
    if (*semAdd <= 0)
    {
        pcb_t* rem = removeBlocked(semAdd);
        rem->p_semAdd = NULL;
        insertProcQ(&readyQ, rem);
    }
    LDST(&(currentProc->p_s));
}

void sys4() // wait
{
    debug(0x13,0xFF,4,0);

    copyState(&(currentProc->p_s),SYSOLD);
    int* semAdd = currentProc->p_s.a2;

    (*semAdd)--;
    if (*semAdd < 0)
    {
        updateTime();
        insertBlocked(semAdd,currentProc);
        scheduler();
    }

    LDST(&(currentProc->p_s));
}

void sys5() // set custom handler
{
    debug(0x13,0xFF,5,0);
    // update currentProc state
    copyState(&(currentProc->p_s),SYSOLD);

    /* 0: PGM_OLD */
    /* 1: TLB_OLD */
    /* 2: SYS_OLD */
    /* 3: PGM_NEW */
    /* 4: TLB_NEW */
    /* 5: SYS_NEW */
    int handlerId = currentProc->p_s.a2;
    uint old = currentProc->p_s.a3;
    uint new = currentProc->p_s.a4;

    // if we've already done this, GLASS 'EM
    if (currentProc->p_handlers[handlerId+CUSTOM_HANDLER_NEW_OFFSET] == NULL)
    {
        sys2();
    }

    // set old/new in address array
    currentProc->p_handlers[handlerId] = old;
    currentProc->p_handlers[handlerId+CUSTOM_HANDLER_NEW_OFFSET] = new;

    // reload state and continue execution
    LDST(&(currentProc->p_s));
}

int sys6() // get CPU time
{
    debug(0x13,0xFF,6,0);
    // update currentProc state
    copyState(&(currentProc->p_s),SYSOLD);

    // update time
    updateTime();

    // set oldSys's A1 to cpu time
    currentProc->p_s.a1 = currentProc->p_cpuTime;

    // load oldSys
    LDST(&(currentProc->p_s));
}

void sys7() // wait 100 ms
{
    debug(0x13,0xFF,7,0);
    // update currentProc state
    copyState(&(currentProc->p_s),SYSOLD);
    
    // time update handled in sys4
    // call sys 4 on psudo-timer for requesting process
    currentProc->p_s.a2 = devSem[PSUDOTIMER_SEM_INDEX];
    sys4();
}

void sys8() // wait for I/O
{
    debug(0x13,0xFF,8,0);
    // update currentProc state
    copyState(&(currentProc->p_s),SYSOLD);

    // index of device semaphore is Line# * 16 + Device#
    int lineNum = currentProc->p_s.a2;
    int devNum = currentProc->p_s.a3;
    bool readFromTerminal = currentProc->p_s.a4;

    int index = lineNum*16 + devNum; 
    if (lineNum == TERMINAL_LINE && readFromTerminal) index+=8; 

    if (--devSem[index] > 0)
    {
        softBlockCount++;
        insertBlocked(&(devSem[index]),currentProc);
        currentProc = NULL;
        scheduler();
    }
    // we got the interrupt first
    currentProc->p_s.a1 = devStat[index];
    devStat[index] = NULL;
    LDST(&(currentProc->p_s));
}

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
    volatile uint toCopyFrom;

    if (currentProc->p_handlers[offset] == NULL)
    {
        // if we haven't specified what to do here, GLASS 'EM
        glassThem(currentProc);
        currentProc = NULL;
        scheduler();
    }
    // they have a custom handler
    // pass off the problem to that

    // copy state into old state
    switch(offset)
    {
        case CUSTOM_PGM: toCopyFrom = PGMTOLD; break;
        case CUSTOM_TLB: toCopyFrom = TLBOLD; break;
        case CUSTOM_SYS: toCopyFrom = SYSOLD; break;
    }
    copyState(currentProc->p_handlers[offset],toCopyFrom);


    // load new state
    LDST(currentProc->p_handlers[offset+CUSTOM_HANDLER_NEW_OFFSET]); 
}


void glassThem(pcb_t* p)
{
    if (p == NULL) return;


    while(!emptyChild(p))
    {
        glassThem(removeChild(p));
    }
    if (p == currentProc)
    {
        currentProc = NULL;
        outChild(p);
    } 
    else 
    {
        if (p->p_semAdd != NULL) // we're blocked
        {
            outBlocked(p);
            if (p->p_semAdd <= devSem[DEVICESPERLINE * NUMOFDEVICELINES] && p->p_semAdd >= devSem[0])
            {
                softBlockCount = softBlockCount - 1;
            }
            else
            {
                (*(p->p_semAdd)) = (*(p->p_semAdd)) + 1;
            }
        }
        else // on readyQ
        {
            outProcQ(&readyQ,p);
        }
    }
    freePcb(p);
    processCount = processCount - 1;
    return;
}

void pgmHandle()
{
    passUpOrDie(CUSTOM_PGM);
}

void tlbHandle()
{
    passUpOrDie(CUSTOM_TLB);
}

void sysHandle()
{    

    volatile state_t* programTrapOld;

    // update currentProc state
    copyState(&(currentProc->p_s),(state_t*) SYSOLD);

    // if in kernal mode
    if (currentProc->p_s.cpsr & SYS_MODE == SYS_MODE)
    {
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
    }

    if (currentProc->p_s.a1 > 8) passUpOrDie(CUSTOM_SYS);

    // if in User Mode
    // save old state in PGTold
    copyState(PGMTOLD,SYSOLD);

    // update cause to RI (reserved instruction)
    programTrapOld = (uint) PGMTOLD;
    programTrapOld->CP15_Cause = RESERVED_INSTRUCTION_CODE;

    // goto program trap handler
    LDST(PGMTNEW);
}

void sys1() // Babies
{
    // make new proc
    volatile pcb_t* sys1p;
    volatile state_t* sentState;
    sys1p = allocPcb();

    // default we can't make a child ;_;
    currentProc->p_s.a1 = -1; // default return code is error code
    sentState = currentProc->p_s.a2;

    // if we can, fix return value and make the child
    if (sys1p != NULL)
    {
        copyState(&(sys1p->p_s),sentState);
        currentProc->p_s.a1 = 0; // return OK code
        insertChild(currentProc,sys1p); // make it a child of currentProc
        processCount = processCount + 1;
        insertProcQ(&readyQ,sys1p);
    }

    // resume execution
    LDST(&(currentProc->p_s));
}

void sys2() // GLASS 'EM
{
    // kill a proc & all it's children

    glassThem(currentProc);

    resetStopwatch();
    
    scheduler();
}

void sys3() // signal
{
    volatile int* semAdd;
    volatile pcb_t* rem;
    semAdd = currentProc->p_s.a2;

    (*semAdd) = ((*semAdd) + 1);
    if (*semAdd <= 0)
    {
        rem = removeBlocked(semAdd);
        rem->p_semAdd = NULL;
        insertProcQ(&readyQ, rem);
    }
    LDST(&(currentProc->p_s));
}

void sys4() // wait
{
    volatile int* semAdd;
    semAdd = currentProc->p_s.a2;

    (*semAdd) = ((*semAdd) - 1);
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
    /* 0: PGM_OLD */
    /* 1: TLB_OLD */
    /* 2: SYS_OLD */
    /* 3: PGM_NEW */
    /* 4: TLB_NEW */
    /* 5: SYS_NEW */
    volatile int handlerId;
    volatile uint old;
    volatile uint new;
    handlerId = currentProc->p_s.a2;
    old = currentProc->p_s.a3;
    new = currentProc->p_s.a4;

    // if we've already done this, GLASS 'EM
    if (currentProc->p_handlers[handlerId+CUSTOM_HANDLER_NEW_OFFSET] != NULL)
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

    // update time
    updateTime();

    // set oldSys's A1 to cpu time
    currentProc->p_s.a1 = currentProc->p_cpuTime;

    // load oldSys
    LDST(&(currentProc->p_s));
}

void sys7() // wait 100 ms
{
    devSem[PSUDOTIMER_SEM_INDEX]--;
    softBlockCount++;

    updateTime();

    insertBlocked(&(devSem[PSUDOTIMER_SEM_INDEX]),currentProc);

    currentProc = NULL;
    scheduler();
}

void sys8() // wait for I/O
{

    // index of device semaphore is Line# * 16 + Device#
    volatile int lineNum;
    volatile int devNum;
    volatile bool readFromTerminal;
    volatile int index;

    lineNum = currentProc->p_s.a2;
    devNum = currentProc->p_s.a3;
    readFromTerminal = currentProc->p_s.a4;
    index = (lineNum * DEVICESPERLINE + devNum - 0x30) / 2;
    if (lineNum == TERMINAL_LINE && readFromTerminal) 
        index = index + 8; 

    if (--devSem[index] < 0)
    {
        softBlockCount++;
        currentProc->p_semAdd = &devSem[index];
        updateTime();
        insertBlocked(&(devSem[index]),currentProc);
        currentProc = NULL;
        scheduler();
    }
    // we got the interrupt first
    currentProc->p_s.a1 = devStat[index];

    devStat[index] = NULL;
    LDST(&(currentProc->p_s));
}

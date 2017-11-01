// exceptions.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/interrupt.e"
#include "../e/scheduler.e"

HIDDEN void killGeneration(pcb_t* p)
{
    pcb_t* tmp;
    pcb_t* removedProc;
    while(p != NULL)
    {
        // if we have a child, kill that generation
        if (p->p_child != NULL)
        {
            killChildRow(p->p_child);
        }

        // get the next child in line
        tmp = p->p_sib;

        // remove from the lists we need to
        removedProc = outProcQ(readyQ,p);
        if (remove0dProc == NULL)
        {
            // we're on the ASL
            removedProc = outBlocked(p);
            *(p->p_semAdd)--; // fix semaphore
        }

        // Free it for use later
        freePcb(p);

        // move to next one
        p = tmp;
    }
}

void programTrapHandler()
{
    // increment pc
    currentProc->p_s->pc += WORDSIZE;

    if (currentProc->p_handlers[CUSTOM_PGM_NEW] == NULL)
    {
        // if we haven't specified what to do here, GLASS 'EM
        sys2();
    }
    // they have a custom handler
    // pass off the problem to that

    // copy state into old state
    currentProc->p_handlers[CUSTOM_PGM_OLD] = PGMTOLD;

    // load new state
    LDST(currentProc->p_handlers[CUSTOM_PGM_NEW]);  
}

void TLBExceptionHandler()
{
    // increment pc
    currentProc->p_s->pc += WORDSIZE;

    if (currentProc->p_handlers[CUSTOM_TLB_NEW] == NULL)
    {
        // if we haven't specified what to do here, GLASS 'EM
        sys2();
    }
    // they have a custom handler
    // pass off the problem to that

    // copy state into old state
    currentProc->p_handlers[CUSTOM_TLB_OLD] = TLBOLD;

    // load new state
    LDST(currentProc->p_handlers[CUSTOM_TLB_NEW]);  
}

sysCall()
{
    // increment pc
    currentProc->p_s->pc += WORDSIZE;
    
    state_t* currentState = (state_t*) SYSOLD

    // if in kernal mode
    if (currentState->CPSR & SYS_MODE == SYS_MODE)
        switch(currentState->A1) // on syscall arg
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
                if (currentProc->p_handlers[CUSTOM_SYS_NEW] != NULL)
                {
                    // we have custom handler for these calls
                    // so lets go there instead.
                    LDST(currentProc->p_handlers[CUSTOM_SYS_NEW])
                }
                // otherwise, GLASS 'EM
                sys2();
        }
    // if in User Mode
    // save old state in PGTold
    state_t* programTrapOld = (unsigned int) PGMTOLD;
    state_t* sysHandlerOld = (unsigned int) SYSOLD;
    programTrapOld = sysHandlerOld;

    // update cause to RI (reserved instruction)
    programTrapOld->CP15_CAUSE = ALLOFF | RESERVED_INSTRUCTION_CODE

    // goto program trap handler
    state_t* programTrapNew = (unsigned int) PGMTNEW;
    LDST(programTrapNew);
}

void sys1() // Babies
{
    // make new proc
    pcb_t* p = allocPcb();

    // default we can't make a child ;_;
    currentProc->p_s->A1 = -1; // return error code

    // if we can, fix return value and make the child
    if (p != NULL)
    {
        currentProc->p_s->A1 = 0; // return OK code
        insertChild(currentProc,p); // make it a child of currentProc
    }

    // load oldSys
    LDST(currentProc->p_s);
}

void sys2() // GLASS 'EM
{
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
    state_t* s = (state_t*) OLDSYS;

    // signal on sem s
    // p = removeBlocked()
    pcb_t* p = removeBlocked(s->A2);

    insertProcQ(p,readyQ);

    // load oldSys
    LDST(s);
}

void sys4() // wait
{
    // update currentProc cpu time
    currentProc->p_cpuTime += TODCLOCK() - currentProc->p_startTime;

    // insert into ASL
    insertBlocked(currentProc, currentProc->p_s->A2);

    // move onto next thing
    scheduler();
}

void sys5() // set custom handler
{
    /* 0: PGM_OLD */
    /* 1: TLB_OLD */
    /* 2: SYS_OLD */
    /* 3: PGM_NEW */
    /* 4: TLB_NEW */
    /* 5: SYS_NEW */
    int handlerId = currentProc->p_s->A2;
    uint old = currentProc->p_s->A3;
    uint new = currentProc->p_s->A4;

    // if we've already done this, GLASS 'EM
    if (currentProc->p_handlers[handlerId+CUSTOM_HANDLER_NEW_OFFSET] == NULL)
    {
        sys2();
    }

    // set old/new in address array
    currentProc->p_handlers[handlerId] = old;
    currentProc->p_handlers[handlerId+CUSTOM_HANDLER_NEW_OFFSET] = new;

    // reload state and continue execution
    LDST(currentProc->p_s);
}

int sys6() // get CPU time
{
    // update time
    currentProc->p_cpuTime += TODCLOCK() - currentProc->p_startTime;
    currentProc->p_startTime = TODCLOCK();

    // set oldSys's A1 to cpu time
    currentProc->p_s->A1 = currentProc->p_cpuTime;

    // load oldSys
    LDST(currentProc->p_s);
}

void sys7() // wait 100 ms
{
    // time update handled in sys4
    // call sys 4 on psudo-timer for requesting process
    currentProc->p_s->A2 = semDev[PSUDOTIMER_SEM_INDEX]
    sys4();
}

void sys8() // wait for I/O
{
    // timing stuff
    currentProc->p_cpuTime += TODCLOCK() - currentProc->p_startTime;
    currentProc->p_startTime = TODCLOCK();

    // softblocked += 1
    softblocked++;

    // index of device semaphore is Line# * 16 + Device#
    int lineNum = currentProc->p_s->A2;
    int devNum = currentProc->p_s->A3;
    int semIndex = lineNum*16 + devNum;
    int semAdd = &devSem[semIndex];
    // call sys 4 on IO's semaphore
    currentProc->p_s->A2 = semAdd;
    sys4();
}

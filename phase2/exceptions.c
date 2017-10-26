// exceptions.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/interrupt.e"
#include "../e/scheduler.e"

void programTrapHandler()
{
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
    // turn off interrupts <----- HOW DO WE DO THIS?  WHICH CPSR are we changing?
    currentProc->p_s->CPSR = currentProc->p_s->CPSR | INT_DISABLED;

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
    programTrapOld->CP15_CAUSE = ALLOFF | RESERVED_INSTRUCTION_CODE // <----- DEFINE THIS VALUE IN const.h LATER!

    // turn on interrupts
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;

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

    // turn on interrupts
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;

    // load oldSys
    LDST(currentProc->p_s);
}

void sys2() // GLASS 'EM
{
    // kill a proc & all it's children
    

    // turn on interrupts    
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;
    
    scheduler();
}

void sys3() // signal
{
    state_t* s = (state_t*) OLDSYS;

    // signal on sem s
    // p = removeBlocked()
    pcb_t* p = removeBlocked(s->A2);

    insertProcQ(p,readyQ);

    // turn on interrupts
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;
    
    // load oldSys
    LDST(s);
}

void sys4() // wait
{
    // update currentProc cpu time
    currentProc->p_cpuTime += TODCLOCK() - currentProc->p_startTime;

    // insert into ASL
    insertBlocked(currentProc, currentProc->p_s->A2);

    // turn on interrupts
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;

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
        
    // turn on interrupts
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;

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

    // turn on interrupts
    currentProc->p_s->CPSR = currentProc->p_s->CPSR & INT_ENABLED;

    // load oldSys
    LDST(currentProc->p_s);
}

void sys7() // wait 100 ms
{
    // time update handled in sys4
    // call sys 4 on psudo-timer for requesting process
    currentProc->p_s->A2 = PSUDOTIMER_SEM_ADDRESS
    sys4();
}

void sys8() // wait for I/O
{
    // timing stuff
    currentProc->p_cpuTime += TODCLOCK() - currentProc->p_startTime;
    currentProc->p_startTime = TODCLOCK();

    // softblocked += 1
    softblocked++;

    // index of device semaphore is (Line# - 1) * 16 + Device#
    int lineNum = currentProc->p_s->A2;
    int devNum = currentProc->p_s->A3;
    int semIndex = (lineNum - 1)*16 + devNum;
    int semAdd = &devSem[semIndex];
    // call sys 4 on IO's semaphore
    currentProc->p_s->A2 = semAdd;
    sys4();
}

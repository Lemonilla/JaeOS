/***************************** EXCEPTIONS.C **************************
 * Written by Neal Troscinski and Timothy Wright
 *
 * This file houses the functions that handle program trap exceptions,
 * translation lookaside buffer (TLB) trap exceptions, and system
 * calls.  TLB and program traps are handled by using custom handlers 
 * or killing the process if those don't exist.
 *
 * System calls are handled differently depending on the privilage 
 * mode of the issuing program.  User Mode calls to 1-8 are handled
 * as program trap exceptions.  Calls over 8, in either mode, are 
 * considered system call exceptions.  If the program is in System
 * Mode and makes a call between 1 and 8, the following functions
 * are performed:
 *  1 - Create a child process
 *  2 - Kill this process
 *  3 - Signal a semaphore
 *  4 - Wait on a semaphore
 *  5 - Define a custom exception handler
 *  6 - Get the program's cpu time
 *  7 - Wait on the interval timer
 *  8 - Wait on I/O
 *
 ********************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/interrupt.e"
#include "../e/scheduler.e"

/************************ Private Fucntions ************************/

/**** passUpOrDie (private)
 * Runs a program's custom exception 
 * handler if one has been defined
 * by issuing a sys5 call.
 * If no handler has been defined
 * the process and all it's children
 * are killed.
 *
 * Parameters:
 * - offset     A number code that defines
 *              which exception handler to
 *              load.
 *                0 = TLB Trap
 *                1 = Program Trap
 *                2 = System Call
 * End State:
 *  - Loads the state of the custom
 *    exception handler if one exists.
 *  - If no custom handler has been
 *    defined, the process is killed
 *    and the scheduler is called.
 ****/
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


/**** sys1 (private)
 * Creates a child process of the
 * current process with a specified
 * state.
 *
 * Parameters:
 * - a2         A pointer to the state
 *              for the child to start in.
 *
 * End State:
 *  - Returns 0 to the current process
 *    if the process could be created. 
 *    Otherwise returns -1.
 ****/
HIDDEN void sys1() // Babies
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


/**** sys2 (private)
 * Terminates the current process
 * and all of it's children.
 *
 * Parameters:
 *
 * End State:
 *  - Calls the scheduler to load
 *    the next job.
 ****/
HIDDEN void sys2() // GLASS 'EM
{
    // kill a proc & all it's children

    glassThem(currentProc);

    resetStopwatch();
    
    scheduler();
}


/**** sys3 (private)
 * Performs a Verhogen operation
 * on the specified semaphore.
 *
 * Parameters:
 * - a2         The address of the 
 *              semaphore to V.
 *
 * End State:
 *  - Returns to the current process.
 ****/
HIDDEN void sys3() // signal
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


/**** sys4 (private)
 * Performs a Passeren operation
 * on the specified semaphore.
 *
 * Parameters:
 * - a2         The address of the 
 *              semaphore to P.
 *
 * End State:
 *  - If the process is blocked, 
 *    it calls the scheduler to load
 *    the next job. Otherwise it
 *    returns to the current job.
 ****/
HIDDEN void sys4() // wait
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


/**** sys5 (private)
 * Sets the custom handlers for the
 * current process that are used
 * in passUpOrDie.
 *
 * Parameters:
 * - a2         The ID of the handler.
 * - a3         A pointer to the location
 *              to save the OLD state prior
 *              to calling the custom handler.
 *                0 = TLB Trap
 *                1 = Program Trap
 *                2 = System Call
 * - a4         A poitner to the location
 *              of the state to load that
 *              is the handler.
 *
 * End State:
 *  - If the process has already set a
 *    handler, the process is terminated
 *    and all it's children with it.
 *    otherwise the process is reloaded
 *    after setting the handlers.
 ****/
HIDDEN void sys5() // set custom handler
{
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


/**** sys6 (private)
 * Returns the current process's
 * accumulated time on the cpu.
 *
 * Parameters:
 *
 * End State:
 *  - The process' time is stored
 *    in a1 and the process reloaded.
 ****/
HIDDEN int sys6() // get CPU time
{

    // update time
    updateTime();

    // set currentProc's A1 to cpu time
    currentProc->p_s.a1 = currentProc->p_cpuTime;

    // load currentProc
    LDST(&(currentProc->p_s));
}


/**** sys7 (private)
 * Places the current process on
 * the interval timer's semaphore.
 * Process on the interval timer's
 * semaphore are woken up every 100ms.
 *
 * Parameters:
 *
 * End State:
 *  - The process is blocked on the
 *    interval timer's semaphore,
 *    and scheduler is called to load
 *    the next job.
 ****/
HIDDEN void sys7() // wait 100 ms
{
    devSem[PSUDOTIMER_SEM_INDEX]--;
    softBlockCount++;

    updateTime();

    insertBlocked(&(devSem[PSUDOTIMER_SEM_INDEX]),currentProc);

    currentProc = NULL;
    scheduler();
}


/**** sys8 (private)
 * Places the current process on
 * an I/O device's semaphore.
 * If that I/O device has already
 * triggered it's interrupt, the
 * device's status is loaded from 
 * storage and the process is not
 * blocked.
 *
 * Parameters:
 * - a2         The line number of the device.
 * - a3         The device number of the device.
 * - a4         TRUE if the device is a terminal
 *              and it is a read operation.
 *
 * End State:
 *  - If device's interrupt has not
 *    happened, the proccess is blocked
 *    on the device's semaphore and a
 *    new job is loaded.
 *  - if the device interrupt has already
 *    happened, then the status is loaded
 *    into a1 and returns to the process'
 *    execution. 
 ****/
HIDDEN void sys8() // wait for I/O
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


/*********************** Public Fucntions ************************/

/**** glassThem (private)
 * Kills a process and all of it's
 * children processes.
 * Corrects softblockCount, 
 * proccessCount, and semaphores.
 *
 * Parameters:
 * - p          A pointer to the
 *              process to kill.
 * End State:
 *  - Returns to the calling function
 *    after killing the process and
 *    all of it's children.
 ****/
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
            if (p->p_semAdd <= devSem[DEVICESPERLINE * NUMOFDEVICELINES] 
                && p->p_semAdd >= devSem[0])
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


/**** pgmHandle
 * Function that is called when a 
 * program trap happens.  It calls
 * passUpOrDie specifying program trap.
 *
 * Parameters:
 *
 * End State:
 *  - Loads passUpOrDie with the 
 *    Program Trap parameter code.
 ****/
void pgmHandle()
{
    passUpOrDie(CUSTOM_PGM);
}


/**** tlbHandle
 * Function that is called when a 
 * TLB trap happens.  It calls
 * passUpOrDie specifying TLB trap.
 *
 * Parameters:
 *
 * End State:
 *  - Loads passUpOrDie with the 
 *    TLB Trap parameter code.
 ****/
void tlbHandle()
{
    passUpOrDie(CUSTOM_TLB);
}


/**** sysHandle
 * Function that is called when a 
 * SWI opcode is loaded by the current
 * job.  It reads the first argument
 * and calls the correct function.
 *
 * Parameters:
 * - a1         Specifies the function
 *              to call.
 *
 * End State:
 *  - Loads passUpOrDie with the 
 *    System Call parameter code
 *    if the parameter is > 8.
 *  - Loads passUpOrDie with the
 *    Program Trap parameter code
 *    if a call is made to 1 - 8
 *    while in User Mode.
 *  - Otherwise calls the proper
 *    function and either returns
 *    to the current process or
 *    blocks that process and 
 *    calls scheduler. 
 ****/
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

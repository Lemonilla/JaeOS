/****************************** INITIAL.C ****************************
 * Written by Neal Troscinski and Timothy Wright
 *
 * This file contains the initialization for the exception and
 * interrupt handlers, as well as the initialization of all other 
 * phase 2 global variables as well as some helper functions.  It
 * then creates the starting process and starts it running.
 *
 ********************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/scheduler.e"
#include "../e/initial.e"
#include "../e/interrupt.e"
#include "../e/exceptions.e"
#include "p2test.c"

/*********************** Global Definitions *************************/
// flow control variables
volatile int processCount = 0;                      
volatile int softBlockCount = 0;
volatile pcb_t* readyQ;
volatile pcb_t* currentProc = NULL;

// init device semaphors
// index of device semaphore is Line# * DEVICESPERLINE + Device#
// terminals utilize the full 16 semaphores.
// The first 8 are for writing, and the 2nd 8 are for reading.
// The two arrays are in parallel
volatile int devSem[DEVICESPERLINE * NUMOFDEVICELINES];  // semaphores
volatile int devStat[DEVICESPERLINE * NUMOFDEVICELINES]; // status

// Timing variables
volatile int Sys7WakeupTimestamp;
volatile int QuantomPart2;
volatile uint startTime_Hi;
volatile uint startTime_Lo;


/*********************** Private Fucntions **************************/

/**** getTimeRunning (private)
 * Returns the time that a process
 * has spent running.
 *
 * Parameters:
 *
 * End State:
 *  - Returns the time a process
 *    has spent running.
 ****/
HIDDEN int getTimeRunning()
{
    uint new_hi = getTODHI();
    uint new_lo = getTODLO();
    uint old_hi = startTime_Hi;
    uint old_lo = startTime_Lo;

    if (new_hi > old_hi)
    {
        // we need to incress new_lo by maxint - old_lo
        return new_lo + (MAXINT - old_lo);
    }
    return new_lo - old_lo;
}



/*********************** Public Fucntions ***************************/

/**** updateTime
 * Updates the current process'
 * cpu time.
 *
 * Parameters:
 *
 * End State:
 *  - The p_cpuTime field of the 
 *    current process' pcb is updated
 *    to reflect the current runtime.
 *  - The start time is reset to reflect
 *    the updating of the time.
 ****/
void updateTime()
{
    currentProc->p_cpuTime += getTimeRunning();
    resetStopwatch();
}


/**** resetStopwatch
 * Reset the current process'
 * start time.
 *
 * Parameters:
 *
 * End State:
 *  - The start time is reset.
 ****/
void resetStopwatch()
{
    startTime_Hi = getTODHI();
    startTime_Lo = getTODLO();
}


/**** copyState
 * Performs a deep copy on two 
 * state_t structures.
 *
 * Parameters:
 * - copy       The pointer to the state
 *              to copy into.
 * - initial    The pointer to the state
 *              to copy.
 *
 * End State:
 *  - copy is now a deep copy of
 *    initial.
 ****/
void copyState(state_t* copy, state_t* initial)
{
    copy->a1 = initial->a1;
    copy->a2 = initial->a2;
    copy->a3 = initial->a3;
    copy->a4 = initial->a4;
    copy->v1 = initial->v1;
    copy->v2 = initial->v2;
    copy->v3 = initial->v3;
    copy->v4 = initial->v4;
    copy->v5 = initial->v5;
    copy->v6 = initial->v6;
    copy->sl = initial->sl;
    copy->fp = initial->fp;
    copy->ip = initial->ip;
    copy->sp = initial->sp;
    copy->lr = initial->lr;
    copy->pc = initial->pc;
    copy->cpsr = initial->cpsr;
    copy->CP15_Control = initial->CP15_Control;
    copy->CP15_EntryHi = initial->CP15_EntryHi;
    copy->CP15_Cause = initial->CP15_Cause;
    copy->TOD_Hi = initial->TOD_Hi;
    copy->TOD_Low = initial->TOD_Low; 
}

/**** main
 * Sets up the JaeOS Operating System's
 * phase 1 and 2 components and initializes
 * the first process.
 * This function is in charge of initializing:
 * - Phase 2 Global Variables
 * - Process Control Blocks
 * - Active Semaphore List
 * - Exception Vector states
 * - The initial program
 *
 * Parameters:
 *
 * End State:
 *  - Calls the scheduler to load
 *    the first job.
 ****/
void main()
{
    // init semaphores
    int i = 0;
    while (i < DEVICESPERLINE * NUMOFDEVICELINES)
    {
        devSem[i] = 0;
        devStat[i] = NULL;
        i++;   
    }

    // init timing values
    resetStopwatch();
    Sys7WakeupTimestamp = getTODLO() + 100000;
    QuantomPart2 = 0;

    // initialize the Process Control Blocks
    initPcbs();
    readyQ = mkEmptyProcQ();
    
    // initialize the Semaphores
    initASL();

    // initialize the exception handler table
    state_t* intNew = (state_t *) INTNEW;
    intNew->pc = (uint) intHandle;
    intNew->sp = RAM_TOP;
    intNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;
    intNew->CP15_Control = ALLOFF;

    state_t* tlbNew = (state_t *) TLBNEW;
    tlbNew->pc = (uint) tlbHandle;
    tlbNew->sp = RAM_TOP;
    tlbNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;
    tlbNew->CP15_Control = ALLOFF;

    state_t* pgmtNew = (state_t *) PGMTNEW;
    pgmtNew->pc = (uint) pgmHandle;
    pgmtNew->sp = RAM_TOP;
    pgmtNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;
    pgmtNew->CP15_Control = ALLOFF;

    state_t* sysNew = (state_t *) SYSNEW;
    sysNew->pc = (uint) sysHandle;
    sysNew->sp = RAM_TOP;
    sysNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;
    sysNew->CP15_Control = ALLOFF;

    // initialize first process
    pcb_t* firstProc = allocPcb();
    firstProc->p_s.pc = (uint) test;
    firstProc->p_s.sp = RAM_TOP - PAGESIZE;
    firstProc->p_s.cpsr = ALLOFF | SYS_MODE | INT_ENABLED;
    insertProcQ(&readyQ,firstProc);
    processCount++;

    // call scheduler and we're done
    scheduler();
}
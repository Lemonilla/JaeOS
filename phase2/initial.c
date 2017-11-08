// initial.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/scheduler.e"
#include "../e/initial.e"
#include "../e/interrupt.e"
#include "../e/exceptions.e"
#include "p2test.c"


void debug(int a, int b, int c, int d)
{
    int i = 42;
    i++;
}


// initialize kernal variables
int processCount = 0;
int softBlockCount = 0;
pcb_t* readyQ;
pcb_t* currentProc = NULL;
int startTime_Hi, startTime_Lo;

// init device semaphors
// index of device semaphore is Line# * 16 + Device#
int devSem[16*NUMOFDEVICELINES];


int getTimeRunning()
{
    int new_hi = getTODHI();
    int new_lo = getTODLO();
    int old_hi = startTime_Hi;
    int old_lo = startTime_Lo;

    if (new_hi > old_hi)
    {
        // we need to incress new_lo by maxint - old_lo
        return new_lo + (MAXINT - old_lo);
    }
    return new_lo - old_lo;
}


void copyState(state_t* copy, state_t* initial)
{
    debug(0x20,0,0,0);
    copy->a1 = initial->a1;
    copy->a2 = initial->a2;
    copy->a3 = initial->a3;
    copy->a4 = initial->a4;
    copy->v1 = initial->v1;
    copy->v2 = initial->v2;
    copy->v3 = initial->v3;
    copy->v4 = initial->v4;
    debug(0x20,8,0,0);
    // we get TLB from this?
    copy->v5 = initial->v5;
    debug(0x20,9,0,0);
    copy->v6 = initial->v6;
    debug(0x20,0x10,0,0);
   // copy->sl = initial->sl;
    debug(0x20,0x11,0,0);
    copy->fp = initial->fp;
    debug(0x20,0x12,0,0);
    copy->ip = initial->ip;
    debug(0x20,0x13,0,0);
    copy->sp = initial->sp;
    debug(0x20,0x14,0,0);
    copy->lr = initial->lr;
    debug(0x20,0x15,0,0);
    copy->pc = initial->pc;
    debug(0x20,0x16,0,0);
    copy->cpsr = initial->cpsr;
    debug(0x20,0x17,0,0);
    copy->CP15_Control = initial->CP15_Control;
    debug(0x20,0x18,0,0);
    copy->CP15_EntryHi = initial->CP15_EntryHi;
    debug(0x20,0x19,0,0);
    copy->CP15_Cause = initial->CP15_Cause;
    debug(0x20,0x20,0,0);
    copy->TOD_Hi = initial->TOD_Hi;
    debug(0x20,0x21,0,0);
    copy->TOD_Low = initial->TOD_Low; 
    debug(0x20,0xFF,0,0);   
}


void main()
{
    // init semaphores
    for (int i = 0; i < 16*NUMOFDEVICELINES; i++) 
    {
        devSem[i] = 0;
    }

    // init timing values
    startTime_Hi = getTODHI();
    startTime_Lo = getTODLO();

    // initialize the Process Control Blocks
    initPcbs();
    readyQ = mkEmptyProcQ();
    
    // initialize the Semaphores
    initASL();

    // initialize the exception handler table
    state_t* intNew = (state_t *) INTNEW;
    intNew->pc = (uint) interruptHandler;
    intNew->sp = RAM_TOP;
    intNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    state_t* tlbNew = (state_t *) TLBNEW;
    tlbNew->pc = (uint) TLBExceptionHandler;
    tlbNew->sp = RAM_TOP;
    tlbNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    state_t* pgmtNew = (state_t *) PGMTNEW;
    pgmtNew->pc = (uint) programTrapHandler;
    pgmtNew->sp = RAM_TOP;
    pgmtNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    state_t* sysNew = (state_t *) SYSNEW;
    sysNew->pc = (uint) sysCall;
    sysNew->sp = RAM_TOP;
    sysNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    // initialize first process
    pcb_t* firstProc = allocPcb();
    firstProc->p_s.pc = (uint) test;
    firstProc->p_s.sp = RAM_TOP - PAGESIZE;  // already set?
    firstProc->p_s.cpsr = ALLOFF | SYS_MODE | INT_ENABLED;
    insertProcQ(&readyQ,firstProc);
    processCount++;

    // call scheduler and we're done
    scheduler();
}
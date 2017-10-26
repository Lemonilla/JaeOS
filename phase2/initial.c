// initial.c
// Neal Troscinski & Timmy Wright


// initialize kernal variables
int processCount = 0;
int softBlockCount = 0;
pcb_t* readyQ = mkEmptyProcQ();
pcb_t* currentProc = NULL;

// init device semaphors
// index of device semaphore is (Line# - 1) * 16 + Device#
int devSem[16*NUMOFDEVICELINES];
for (int i = 0; i < 16*NUMOFDEVICELINES; i++) devSem[i] = 0;

void main()
{
    // initialize the Process Control Blocks
    initPCB() 
    
    // initialize the Semaphores
    initASL() 

    // initialize the exception handler table
    state_t* intNew = (state_t *) INTNEW;
    intNew->pc = (unsigned int) InterruptHandler;
    intNew->sp = RAM_TOP;
    intNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    state_t* tlbNew = (state_t *) TLBNEW;
    tlbNew->pc = (unsigned int) TLBExceptionHandler;
    tlbNew->sp = RAM_TOP;
    tlbNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    state_t* pgmtNew = (state_t *) PGMTNEW;
    pgmtNew->pc = (unsigned int) programTrapHandler;
    pgmtNew->sp = RAM_TOP;
    pgmtNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    state_t* sysNew = (state_t *) SYSNEW;
    sysNew->pc = (unsigned int) sysCall;
    sysNew->sp = RAM_TOP;
    sysNew->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;

    // initialize first process
    pcb_t* firstProc = allocPcb();
    firstProc->p_s = (unsigned int) test;    // <--- where is this?
    firstProc->p_s->sp = RAM_TOP - PAGESIZE;  // already set?
    firstProc->p_s->->cpsr = ALLOFF | SYS_MODE | INT_DISABLED;
    insertProcQ(firstProc,readyQ);

    // call scheduler and we're done
    scheduler();
}
// interrupt.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

state_t* old;

void debugA(int a, int b, int c, int d)
{
    int i=42;
    i++;
}

void intHandle()
{
    debug(0x17,0,0,0);

    // setup access to interrupting state
    // we don't update currentProc because we might be in WAIT();
    old = (state_t*) INTOLD;

    // figure out highest priority device alert
    uint lineNumber = NULL;
    uint mask = 0x80000000;
    int line = 7;
    while (mask >= 0x01000000)
    {
        if (old->CP15_Cause & mask) lineNumber = line;
        --line;
        mask = mask >> 1;

    }

    // if nothing is interrupting, panic
    if (lineNumber == NULL) PANIC();

    // if end of quantum
    if (lineNumber == CLOCK_LINE)
    {
        // ASSUME IT"S END OF QUANTOM

        // if there's no prog who's quantom to end, skip this shit
        if (currentProc == NULL) scheduler();

        // update state
        copyState(&(currentProc->p_s), INTOLD);

        // increment currentProc cpu time
        updateTime();

        // put currentProc on readyQ
        insertProcQ(&readyQ, currentProc);

        // call the scheduler
        scheduler();

    }
    // get device number of highest priority
    int devNumber = NULL;
    mask = 0x00000080;
    int dev = 7;
    uint* bitmap = 0x6fe0 + (lineNumber-3)*4;
    while (mask != 0)
    {
        if (*bitmap & mask) devNumber = dev;
        --dev;
        mask = mask >> 1;
    }

    // get device semaphore
    //DEVICESPERLINE
    int index = (lineNumber * DEVICESPERLINE + devNumber - 0x30) / 2;

    // if no device is interrupting, panic
    if ((devNumber+1) == NULL) {
        PANIC();
    }

    // get device register (taken from book pg 36)
    devreg_t* deviceReg = (index * DEVICESPERLINE) + (*(uint*) 0x02D8);

    // copy status code for later use
    uint status = deviceReg->term.transm_status;

    // set command to acknowledged
    deviceReg->term.recv_command = CMD_ACK;
    deviceReg->term.transm_command = CMD_ACK;
    //deviceReg->data0 = CMD_ACK;

    // increment and test to see if we need to pop
    devSem[index] = devSem[index] + 1;
    if (devSem[index] <= 0)
    {
        softBlockCount--;
        pcb_t* tmp = removeBlocked(&(devSem[index]));
        tmp->p_semAdd = NULL;
        tmp->p_s.a1 = status;
        insertProcQ(&readyQ, tmp);
    }
    else 
    {
        // if we beat the IOWAIT request
        devStat[index] = status;
    }

    if (currentProc == NULL) scheduler();

    // stop prefetching
    ((state_t*)INTOLD)->pc = ((state_t*) INTOLD)->pc - 4;

    LDST(INTOLD);
}
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

void intHandle()
{

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

    debug(0x17,0xFF,lineNumber,devNumber);

    // if no device is interrupting, panic
    //if (devNumber == NULL) PANIC();

    // get device register (taken from book pg 36)
    devregister_t* deviceReg = (devreg_t*) (*((uint*)0x02D8) + (lineNumber-3)*0x80 + (devNumber * 0x10));
    // devregister_t* deviceReg = DEVICEREGSTART + (lineNumber * LINEOFFSET) + (semIndex * DEVICEOFFSET);

    // devregister_t* deviceReg = *((void*)0x02D8) + ()


    debug(0x17,0xF0,(*deviceReg).command,(*deviceReg).data1);

    // get device semaphore
    int index = lineNumber * NUMOFDEVICELINES + devNumber;
    debug(0x17,0xF1,0,0);

    // copy status code for later use
    uint status = deviceReg->status;
    debug(0x17,0xF2,0,0);

    // set command to acknowledged
    deviceReg->command = CMD_ACK;
    debug(0x17,0xF3,0,0);

    // increment and test to see if we need to pop
    if (++devSem[index] <= 0)
    {
        softBlockCount--;
        pcb_t* tmp = removeBlocked(&(devSem[index]));
        tmp->p_semAdd = NULL;
        tmp->p_s.a2 = status;
        insertProcQ(&readyQ, tmp);
    }
    debug(0x17,0xF4,0,0);

    // if we beat the IOWAIT request
    devStat[index] = status;

    debug(0x17,0xF5,0,0);

    if (currentProc == NULL) scheduler();
    // return;
    LDST(old);
}
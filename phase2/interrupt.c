// interrupt.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"


void intHandle()
{
    debug(0x17,0,0,0);

    // setup access to interrupting state
    // we don't update currentProc because we might be in WAIT();
    state_t* old = (state_t*) INTOLD;

    // figure out highest priority device alert
    uint lineNumber;
    if (old->CP15_Cause & FIRST_BIT_ON)   lineNumber = 7;
    if (old->CP15_Cause & SECOND_BIT_ON)  lineNumber = 6;
    if (old->CP15_Cause & THIRD_BIT_ON)   lineNumber = 5;
    if (old->CP15_Cause & FOURTH_BIT_ON)  lineNumber = 4;
    if (old->CP15_Cause & FIFTH_BIT_ON)   lineNumber = 3;
    if (old->CP15_Cause & SIXTH_BIT_ON)   lineNumber = 2;
    if (old->CP15_Cause & SEVENTH_BIT_ON) lineNumber = 1;
    if (old->CP15_Cause & EIGTH_BIT_ON)   lineNumber = 0;

    // if end of quantum
    if (lineNumber == CLOCK_LINE)
    {
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

    // get the device flag table
    devflagtable_t* devTable = (devflagtable_t*) DEV_FLAG_PTR;

    // get device number of highest priority
    int devNumber = NULL;
    if (devTable->lines[lineNumber] & FIRST_BIT_ON)   devNumber = 7;
    if (devTable->lines[lineNumber] & SECOND_BIT_ON)  devNumber = 6;
    if (devTable->lines[lineNumber] & THIRD_BIT_ON)   devNumber = 5;
    if (devTable->lines[lineNumber] & FOURTH_BIT_ON)  devNumber = 4;
    if (devTable->lines[lineNumber] & FIFTH_BIT_ON)   devNumber = 3;
    if (devTable->lines[lineNumber] & SIXTH_BIT_ON)   devNumber = 2;
    if (devTable->lines[lineNumber] & SEVENTH_BIT_ON) devNumber = 1;
    if (devTable->lines[lineNumber] & EIGTH_BIT_ON)   devNumber = 0;

    // if no device is interrupting, panic
    if (devNumber == NULL) PANIC();

    // get device register (taken from book)
    devregister_t* deviceReg = (devreg_t*) (DEV_REG_TABLE + (lineNumber-3)*0x80 + (devNumber * 0x10));

    // get device semaphore
    int index = lineNumber * 16 + devNumber;

    // copy status code for later use
    uint status = deviceReg->status;

    // set command to acknowledged
    deviceReg->command = CMD_ACK;

    // increment and test to see if we need to pop
    if (++devSem[index] <= 0)
    {
        softBlockCount--;
        pcb_t* tmp = removeBlocked(&(devSem[index]));
        tmp->p_semAdd = NULL;
        tmp->p_s.a2 = status;
        insertProcQ(&readyQ, tmp);
    }

    // if we beat the IOWAIT request
    devStat[index] = status;

    // return;
    LDST(old);
}
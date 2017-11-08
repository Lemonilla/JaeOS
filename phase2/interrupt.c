// interrupt.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

extern void interruptHandler()
{
    if (currentProc == NULL) scheduler();

    // figure out highest priority device alert
    uint lineNumber;

    state_t* old = (state_t*) INTOLD;

    if (old->CP15_Cause & FIRST_BIT_ON)   lineNumber = 7;
    if (old->CP15_Cause & SECOND_BIT_ON)  lineNumber = 6;
    if (old->CP15_Cause & THIRD_BIT_ON)   lineNumber = 5;
    if (old->CP15_Cause & FOURTH_BIT_ON)  lineNumber = 4;
    if (old->CP15_Cause & FIFTH_BIT_ON)   lineNumber = 3;
    if (old->CP15_Cause & SIXTH_BIT_ON)   lineNumber = 2;
    if (old->CP15_Cause & SEVENTH_BIT_ON) lineNumber = 1;
    if (old->CP15_Cause & EIGTH_BIT_ON)   lineNumber = 0;

    // if end of quantum
    if (lineNumber == 2){
            //turn off alarm (optional)
            //Done by setting new value in timer

            //copy oldInt into currentProc->p_state
            copyState(&(currentProc->p_s), INTOLD);

            //increment currentProc cpu time
            int currentTod = TODCLOCK();
            int tempTime = currentTod - currentProc->p_startTime;
            currentProc->p_cpuTime = tempTime;

            //put currentProc on readyQ
            insertProcQ(currentProc, readyQ);

            //call the scheduler
            scheduler();
    }

    // get the device flag table
    devflagtable_t* devTable = (devflagtable_t*) DEV_FLAG_PTR;

    // get lowest line number with interrupts
    // int lineNumber = 0;
    // for ( ; devTable->lines[lineNumber] == 0; lineNumber++) {};

    // get device number
    int devNumber = NULL;
    if (devTable->lines[lineNumber] & FIRST_BIT_ON)   devNumber = 7;
    if (devTable->lines[lineNumber] & SECOND_BIT_ON)  devNumber = 6;
    if (devTable->lines[lineNumber] & THIRD_BIT_ON)   devNumber = 5;
    if (devTable->lines[lineNumber] & FOURTH_BIT_ON)  devNumber = 4;
    if (devTable->lines[lineNumber] & FIFTH_BIT_ON)   devNumber = 3;
    if (devTable->lines[lineNumber] & SIXTH_BIT_ON)   devNumber = 2;
    if (devTable->lines[lineNumber] & SEVENTH_BIT_ON) devNumber = 1;
    if (devTable->lines[lineNumber] & EIGTH_BIT_ON)   devNumber = 0;

    // get device register
    devregister_t* deviceReg = (devreg_t*) (DEV_REG_TABLE + (lineNumber-3)*0x80 + (devNumber * 0x10));

    // get device semaphore
    int devSemIndex = lineNumber * 16 + devNumber;
    int* devSemAddress = &(devSem[devSemIndex]);

    // copy status code for later use
    uint status = deviceReg->status;

    // set command to acknowledged
    deviceReg->command = ACK;

    // if IO, decrement softblocked
    --softBlockCount;
    pcb_t* tempProc = removeBlocked((int*)devSemAddress);

    // put status in pcb gotten
    tempProc->p_s.a2 = status;

    // put pcb on readyQ
    insertProcQ(&readyQ,tempProc);

    LDST(old);


}
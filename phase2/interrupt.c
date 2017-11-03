// interrupt.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

void InterruptHandler()
{

    if (currentProc == NULL) scheduler();

    // figure out highest priority device alert
    uint lineNumber;
    uint devNumber;

    if (OLDINT->CP15_Cause & FIRST_BIT_ON)   lineNumber = 7;
    if (OLDINT->CP15_Cause & SECOND_BIT_ON)  lineNumber = 6;
    if (OLDINT->CP15_Cause & THIRD_BIT_ON)   lineNumber = 5;
    if (OLDINT->CP15_Cause & FOURTH_BIT_ON)  lineNumber = 4;
    if (OLDINT->CP15_Cause & FIFTH_BIT_ON)   lineNumber = 3;
    if (OLDINT->CP15_Cause & SIXTH_BIT_ON)   lineNumber = 2;
    if (OLDINT->CP15_Cause & SEVENTH_BIT_ON) lineNumber = 1;
    if (OLDINT->CP15_Cause & EIGTH_BIT_ON)   lineNumber = 0;

    // if end of quantum
    if (lineNumber == 2){
            //turn off alarm (optional)
            //Done by setting new value in timer

            //copy oldInt into currentProc->p_state
            copyState(&(currentProc->p_s), INTOLD);

            //increment currentProc cpu time
            currentTod = TODCLOCK();
            tempTime = currentTod - currentProc->p_startTime;
            currentProc->p_cpuTime = tempTime;

            //put currentProc on readyQ
            insertProcQ(currentProc, readyQ);

            //call the scheduler
            scheduler();
    }

    // get device number
    unsigned int devTable = *((unsigned int) DEV_FLAG_PTR);
    while (*devTable == 0) 
    {
        devTable += WORDSIZE;
        // if no flags are on, why are we here?
        if (devTable > *((unsigned int) DEV_FLAG_PTR) + 0x10) PANIC();
    }
    if (*devTable & FIRST_BIT_ON)   devNumber = 7;
    if (*devTable & SECOND_BIT_ON)  devNumber = 6;
    if (*devTable & THIRD_BIT_ON)   devNumber = 5;
    if (*devTable & FOURTH_BIT_ON)  devNumber = 4;
    if (*devTable & FIFTH_BIT_ON)   devNumber = 3;
    if (*devTable & SIXTH_BIT_ON)   devNumber = 2;
    if (*devTable & SEVENTH_BIT_ON) devNumber = 1;
    if (*devTable & EIGTH_BIT_ON)   devNumber = 0;

    // get address of device register [status,command,data1,data2]
    void* deviceRegister = DEV_REG_TABLE + devNumber*WORDSIZE

    // calculate index of device semaphore
    int devIndex = lineNumber * 16 + devNumber;
    uint devSemAddress = &(devSem[devIndex])

    // copy status
    uint status = *(deviceRegister);

    // set command to acknowledged
    *(deviceRegister+WORDSIZE) = 0;

    // if IO, decrement softblocked
    --softBlockCount;
    pcb_t* tempProc = removeBlocked(devAddress);

    // put status in pcb gotten
    tempProc->a2 = status;

    // put pcb on readyQ
    insertProcQ(tempProc,readyQ);

    LDST(INTOLD);
}
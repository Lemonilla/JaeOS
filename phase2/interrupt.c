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
    // figure out highest priority device alert
    uint lineNumber;
    // DOUBLE CHECK THESE ARE ACTUALLY THE LINES WE THINK THEY ARE
    if (OLDINT->CP15_Cause & 0x01000000) lineNumber = 7;
    if (OLDINT->CP15_Cause & 0x02000000) lineNumber = 6;
    if (OLDINT->CP15_Cause & 0x04000000) lineNumber = 5;
    if (OLDINT->CP15_Cause & 0x08000000) lineNumber = 4;
    if (OLDINT->CP15_Cause & 0x10000000) lineNumber = 3;
    if (OLDINT->CP15_Cause & 0x20000000) lineNumber = 2;
    if (OLDINT->CP15_Cause & 0x40000000) lineNumber = 1;
    if (OLDINT->CP15_Cause & 0x80000000) lineNumber = 0;

    Decide which line has highest priority
    // if end of quantum
    if (lineNumber == 2){
            //turn off alarm (optional)

            //Done by setting new value in timer
            setTimer(QUANTUM);
            //copy oldInt into currentProc->p_state
            CopyState(currentProc->p_s, INTOLD);
            //increment currentProc cpu time
            currentTod = TODCLOCK();
            tempTime = currentTod - currentProc->p_startTime;
            currentProc->p_cpuTime = tempTime;
            //put currentProc on readyQ
            insertProcQ(currentProc, readyQ);
            //call the scheduler
            scheduler();
    }
    // get address of device register [status,command,data1,data2]
    // calculate index of device semaphore
    int devIndex = lineNumber * 16;
    devIndex += devNumber;
    uint devAddress = &(devSem[devIndex])
    // copy status

    // set command to acknowledged

    //Turn the lines off
    pcb_t* tempProc = removeBlocked(devAddress)
    CopyState(tempProc, );

    // if IO, decrement softblocked
    --softBlockCount;

    // put status in pcb gotten
    ?????????

    // put pcb on readyQ
    insertProcQ(tempProc,readyQ);

    LDST(INTOLD);
}
// interrupt.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

HIDDEN void debug(int a, int b, int c, int d)
{
    int i = 42;
    i++;
}

state_t* old;

void intHandle()
{
    // setup access to interrupting state
    // we don't update currentProc because we might be in WAIT();
    old = (state_t*) INTOLD;

    // do this first so we don't charge for interrupt time
    if (currentProc != NULL) updateTime();
    debug(0x17,0xFF,0,0); // NESSISARY (i don't know why)

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
        // handle wakup and return to process
        if (getTODLO() >= Sys7WakeupTimestamp) 
        {
            pcb_t* tmp = NULL;
            
            while (TRUE) {
                tmp = removeBlocked(&(devSem[PSUDOTIMER_SEM_INDEX]));
                if (tmp == NULL) break;
                insertProcQ(&readyQ,tmp);
                softBlockCount = softBlockCount - 1; // ????
            }

            setTIMER(QuantomPart2);
            QuantomPart2 = 0;
            Sys7WakeupTimestamp = getTODLO() + 100000;

            // stop prefetching
            ((state_t*)INTOLD)->pc = ((state_t*) INTOLD)->pc - 4;

            // don't charge for interrupt
            resetStopwatch();

            if(currentProc == NULL)
            {
               scheduler();
            }
            LDST(INTOLD);
        }

        // OTHERWISE IT"S END OF QUANTOM
        debug(0x17,0,0,0); // NESSISARY? (idk why)
        // if there's no prog who's quantom to end, skip this shit
        if (currentProc == NULL) scheduler();

        // update state
        copyState(&(currentProc->p_s), INTOLD);
        // stop prefetch?
        currentProc->p_s.pc = currentProc->p_s.pc - 4;

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
    // and to determine which terminal opperation we're dealing with
    uint status = deviceReg->term.transm_status;

    // set command to acknowledged
    if (status != DEV_S_READY)
    {
        // then we're dealing with transmition
        deviceReg->term.transm_command = CMD_ACK;
    }
    else 
    {
        // we're dealing with reception
        status = deviceReg->term.recv_status;
        deviceReg->term.recv_command = CMD_ACK;
    }

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

    // don't charge for interrupt
    resetStopwatch();

    LDST(INTOLD);
}

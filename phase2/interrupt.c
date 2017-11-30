/***************************** INTERRUPT.C ***************************
 * Written by Neal Troscinski and Timothy Wright
 *
 * This file contains the scheduler, which determines which job on
 * the ready queue should be the next job to run.  It implements a
 * first come first served algorithm which loads the jobs in the 
 * order in which they are added to the queue.
 * If no job is ready, a series of checks are executed to determine
 * the action.
 * If no jobs are left in the system, the HALT ROM function is called.
 * If there are jobs left, but they are all waiting on I/O,
 * the system enters a wait state by calling the WAIT ROM function.
 * If there are jobs left, but none are waiting on I/O, then deadlock
 * has occured, and the system panics with the PANIC ROM function.
 *
 ********************************************************************/

#include "../h/const.h"
#include "../h/types.h"

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

/************************ Private Fucntions ************************/

/**** debug_int (private)
 * This function does nothing
 * and only exists to force the
 * compiler not to optomize code.
 *
 * Parameters:
 * - a          A meaningless number.
 * - b          A meaningless number.
 * - c          A meaningless number.
 * - d          A meaningless number.
 *
 * End State:
 *  - Returns without doing anything.
 ****/
HIDDEN void debug_int(int a, int b, int c, int d)
{
    int i = 42;
    i++;
}


/*********************** Public Fucntions ************************/

/**** intHandle
 * This function runs whenever
 * an interrupt happens and handles
 * each cause appropriately.  The
 * current process is not billed
 * for the time it takes to handle
 * an intterupt.
 * An interrupt may occure when
 * - The quantom ends and it is
 *   time for the current job to
 *   release the cpu.
 * - The interval timer triggers
 *   and it is time to wake up
 *   processes that issued sys7
 *   calls.
 * - A device has completed it's
 *   directive, and the status is
 *   available to pass on to the
 *   program.
 *
 * Parameters:
 *
 * End State:
 *  - If it is the end of the
 *    quantom, then the current
 *    process is updated and
 *    put back on the ready queue.
 *    Then scheduler is called.
 *  - If it is the interval timer
 *    then all jobs waiting on it
 *    are put back onto the ready
 *    queue and the current process
 *    continues.
 *  - If it is a device that needs
 *    to be acknowledged, the status
 *    will be returned into the first
 *    process on that device's semaphore.
 *    If no device is waiting, then it
 *    is stored off in the status array
 *    for when the next process requests
 *    that device.  If there is a job
 *    currently running, that is reloaded
 *    and updated.  If no job is running
 *    shceduler is called.
 ****/
void intHandle()
{
    // setup access to interrupting state
    // we don't update currentProc because we might be in WAIT();
    volatile state_t* old;
    volatile pcb_t* tmp;
    volatile uint lineNumber;
    volatile uint mask;
    volatile int line;
    volatile int devNumber;
    volatile uint* bitmap;
    volatile int dev;
    volatile int index;
    volatile uint status;
    volatile devreg_t* deviceReg;
    volatile uint* semAddress;

    old = (state_t*) INTOLD;

    // do this first so we don't charge for interrupt time
    if (currentProc != NULL) updateTime();

    // figure out highest priority device alert
    lineNumber = NULL;
    mask = DEV_MAX_LINE_FLAG_BIT;
    line = 7;
    while (mask >= DEV_MIN_LINE_FLAG_BIT)
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
            // pop all the processes waiting back onto the ready queue
            tmp = NULL;
            while (TRUE) {
                tmp = removeBlocked(&(devSem[PSUDOTIMER_SEM_INDEX]));
                if (tmp == NULL) break;
                insertProcQ(&readyQ,tmp);
                softBlockCount = softBlockCount - 1;
            }

            // put the rest of the time on the clock
            setTIMER(QuantomPart2);
            QuantomPart2 = 0;
            Sys7WakeupTimestamp = getTODLO() + INTERVAL_TIMER_LENGTH;

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

        // if there's no prog who's quantom to end, skip this shit
        if (currentProc == NULL) scheduler();

        // update state
        copyState(&(currentProc->p_s), INTOLD);

        // stop prefetch
        currentProc->p_s.pc = currentProc->p_s.pc - 4;

        // increment currentProc cpu time
        updateTime();

        // put currentProc on readyQ
        insertProcQ(&readyQ, currentProc);

        // call the scheduler
        scheduler();

    }
    // get device number of highest priority
    devNumber = NULL;
    mask = DEV_MAX_DEV_FLAG_BIT;
    dev = 7;
    bitmap = DEV_BITMAP_ADDR + (lineNumber-3)*4;
    while (mask != 0)
    {
        if (*bitmap & mask) devNumber = dev;
        --dev;
        mask = mask >> 1;
    }

    // get device semaphore
    index = (lineNumber * DEVICESPERLINE + devNumber - DEV_SEM_INDEX_OFFSET) / 2;

    // if no device is interrupting, panic
    if ((devNumber+1) == NULL) {
        PANIC();
    }

    // get device register
    deviceReg = (index * DEVICESPERLINE) + (*(uint*) DEV_REG_BASEADDR);

    // copy status code for later use 
    // and to determine which terminal opperation we're dealing with
    status = deviceReg->term.transm_status;

    // set command to acknowledged
    if (status != DEV_S_READY)
    {
        // then we're dealing with transmition
        deviceReg->term.transm_command = CMD_ACK;
    }
    else 
    {
        // we're dealing with reception
        index = index + 8;
        status = deviceReg->term.recv_status;
        deviceReg->term.recv_command = CMD_ACK;
    }

    // increment and test to see if we need to pop
    devSem[index] = devSem[index] + 1;
    semAddress = &(devSem[index]);
    if (devSem[index] <= 0)
    {
        softBlockCount--;
        tmp = removeBlocked(&(devSem[index]));
        tmp->p_semAdd = NULL;
        tmp->p_s.a1 = status;

        // Necessary debug call.  Unknown reason.
        debug_int(0xf0,status,&(devSem[index]),tmp->p_id);

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

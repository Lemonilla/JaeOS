/***************************** SCHEDULER.C ***************************
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
#include "../e/interrupt.e"

/*********************** Public Fucntions ************************/

/**** scheduler
 * Picks the next job to run using a
 * basic first come first served
 * algorithm.  If no jobs are ready
 * then it checks to see if there are
 * jobs waiting on I/O, if there are
 * any jobs left to run, or if
 * deadlock has occured.
 * The scheduler is also in charge
 * of setting the timer.
 *
 * Parameters:
 *
 * End State:
 *  - PANIC if deadlock has occured.
 *  - WAIT if all jobs are waiting
 *    on I/O requests.
 *  - HALT if all jobs have finished.
 *  - Otherwise it will load the next
 *    job in the queue.
 ****/
void scheduler()
{
    // get next in line
    volatile pcb_t* next;
    volatile int tod;
    next = removeProcQ(&readyQ);

    // if list is empty
    if (next == NULL)
    {
        // check if processCount = 0
        if (processCount == 0) 
        {
            HALT();
        }

        // if not:
        // check softblock count = 0
        if (softBlockCount != 0) 
        {
            currentProc = NULL;

            // TURN ON INTERRUPTS!
            setSTATUS(ALLOFF | SYS_MODE | INT_ENABLED );
            WAIT();
        } 
        // we've hit deadlock
        PANIC();
    }
    // if list is not empty
    currentProc = next;

    // start timer thing
    resetStopwatch();

    // set timer
    tod = getTODLO();
    if (tod+QUANTOM > Sys7WakeupTimestamp)
    {
        QuantomPart2 = QUANTOM - (Sys7WakeupTimestamp - (tod+QUANTOM));
        setTIMER(tod+QUANTOM - Sys7WakeupTimestamp);
    }
    else 
    {
        setTIMER(QUANTOM);
    }

    // load state in p
    LDST(&(currentProc->p_s));
}
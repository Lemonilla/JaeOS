// scheduler.c
// Neal Troscinski & Timmy Wright

#include "../h/const.h"
#include "../h/types.h"
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/exceptions.e"
#include "../e/interrupt.e"


void scheduler()
{

    // get next in line
    pcb_t* next = removeProcQ(&readyQ);

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
    int tod = getTODLO();
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
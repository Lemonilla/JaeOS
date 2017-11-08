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
    // we're done with our timeslice, get back in line
    insertProcQ(&readyQ, currentProc);

    // get next in line
    pcb_t* next = removeProcQ(&readyQ);

    // if list is empty
    if (next == NULL)
    {
        // check if processCount = 0
        if (processCount == 0) HALT();

        // if not:
        // check softblock count = 0
        if (softBlockCount != 0) 
        {
            currentProc = NULL;
            // TURN ON INTERRUPTS!
            // using getSTATUS and setSTATUS
            WAIT();
        } 
        
        // we've hit deadlock
        PANIC();
    }
    
        
    // if list is not empty
    currentProc = next;

    // start timer thing
    currentProc->p_startTime = TODTIMER();

    // set timer
    setTIMER(QUANTOM);

    // load state in p
    LDST(&(currentProc->p_s));
}
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
    pcb_t* next = removeProcQ();
    // if list is empty
    if (next == NULL)
    {
        // check if processCount = 0
        if (processCount == 0) HALT();

        // if not:
        // check softblock count = 0
        if (softblock != 0) 
        {
            currentProc = NULL;
            // TURN ON INTERRUPTS!
            // HOW???
            WAIT();
        } 
        
        PANIC();

    }
    
        
    // if list is not empty
    // if currentProc is done
    currentProc = next;
    // start timer thing
    currentProc->p_startTime = TODTIMER() // what's that called?

    // set timer
    SETTIMER(QUANTUM);

    // load state in p
    LDST(&(currentProc->p_s));
}
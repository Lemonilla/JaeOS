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
debug(3,0,0,0);
    // we're done with our timeslice, get back in line
   // insertProcQ(&readyQ, currentProc);

    // get next in line
    pcb_t* next = removeProcQ(&readyQ);

    // if list is empty
    if (next == NULL)
    {
        // check if processCount = 0
        if (processCount == 0) 
        {
            debug(3,1,0,0);
            HALT();
        }

        // if not:
        // check softblock count = 0
        if (softBlockCount != 0) 
        {
            debug(3,2,0,0);
            currentProc = NULL;
            // TURN ON INTERRUPTS!
            debug(0x3,2,getSTATUS() & 0xFFFFFF3F,0);

            setSTATUS(getSTATUS() & 0xFFFFFF3F);
            debug(0x3,2,getSTATUS(),0);
            WAIT();
        } 
        debug(3,3,0,0);
        // we've hit deadlock
        PANIC();
    }

    // if list is not empty
    currentProc = next;

    // start timer thing
    startTime_Hi = getTODHI();
    startTime_Lo = getTODHI();

    // set timer
    setTIMER(QUANTOM);

    // load state in p
    LDST(&(currentProc->p_s));
}
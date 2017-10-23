// scheduler.c
// Neal Troscinski & Timmy Wright

void scheduler()
{
    pcb_t* next = removeProcQ();
    // if list is empty
    if (next == NULL)
    {
        // check if processCount = 0
        if (processCount == 0) 
        {
            // is so, HALT()
            HALT();
        }
        // if not:
        // check softblock count = 0
        if (softblock != 0)
        {
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
    LDST(currentProc->p_s);
}
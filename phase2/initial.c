
void main()
{
    // initialize the Process Control Blocks
    initPCB() 
    // initialize the Semaphores
    initASL() 

    // initialize the exception handler table


    // initialize kernal variables
    int processCount = 0;
    int softBlockCount = 0;
    pcb_t* readyQ = mkEmptyProcQ();
    pcb_t* currentProc = NULL;

    // initialize first process
    

    // set timer
    

    // call scheduler and we're done
    scheduler();
}
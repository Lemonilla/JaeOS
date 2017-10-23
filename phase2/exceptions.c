// exceptions.c
// Neal Troscinski & Timmy Wright


void programTrapHandler()
{
    // if pcb_t->trapHander[w/e we need] == NULL 
    //      Glass 'em
    // otherwise
    // copy state into old state
    // copy handlerState from new
    // load new state
}

void TLBExceptionHandler()
{
    // if pcb_t->TLBHander[w/e we need] == NULL 
    //      Glass 'em  
    // otherwise
    // copy state into old state
    // copy handlerState from new
    // load new state
}

sysCall()
{
    // turn off interrupts

    // if in kernal mode
        switch() // on syscall arg
        {
            case 1: // - BIRTH NEW PROC
                sys1();
            case 2: // - GLASS 'EM
                sys2();  
            case 3: // - SIGNAL
                sys3();
            case 4: // - WAIT
                sys4();
            case 5: // - SET NEW HANDLERS
                sys5();
            case 6: // - CPU TIME
                sys6();
            case 7: // - ???
                sys7();
            case 8: // - WAIT FOR I/O
                sys8();
            default: // - Pass up or die
                // if set own SYScallHandler, use that
                // otherwise, GLASS 'EM
                sys2();
        }
    // if in User Mode
        // save old state in PGTold
        // set CP15 (????)
        // update cause to RI (reserved instruction)
        // call programTrapHandler();
}

void sys1() // Babies
{
    // make new proc

    // if can't make, A1 = -1
    // else A1 = 0
    // turn on interrupts
    // load oldSys
}

void sys2() // GLASS 'EM
{
    // kill a proc & all it's children
    // freePCB()

    // turn on interrupts
    // load oldSys
}

void sys3() // signal
{
    // signal on sem s
    // p = removeBlocked()
    // insertProcQ(p,ready)

    // turn on interrupts
    // load oldSys
}

void sys4() // wait
{

    // update currentProc cpu time
    // insertBlocked(p, sem)

    // turn on interrupts
    // scheduler()
}

void sys5() // set custom handler
{
    // if old/new states are NULL
        // set old/new in address array
        // turn on interrupts
        // reload state and continue execution
    // else 
        // GLASS 'EM
}

int sys6() // get CPU time
{
    // set oldSys's A1 to cpu time

    // turn on interrupts
    // load oldSys
}

void sys7() // wait 100 ms
{
    // update currentProc cpu time
    // call sys 4 on psudo-timer for requresting process
}

void sys8() // wait for I/O
{
    // update currentProc cpu time
    // softblocked += 1
    // call sys 4 on IO's semaphore
}

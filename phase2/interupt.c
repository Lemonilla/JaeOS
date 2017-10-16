// interrupts.c

sysCall()
{
    // turn off interrupts

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
        case default: // - GLASS 'EM
            sys2();
    }
}

// BIRTH NEW PROC
void sys1()
{
    // make new proc

    // turn on interrupts
}

void sys2()
{
    // kill a proc & all it's children
    // freePCB()

    // turn on interrupts
}

void sys3()
{
    // signal on sem s
    // p = removeBlocked()
    // insertProcQ(p,ready)
    // turn on interrupts
    // return to program
}

void sys4()
{
    // turn off interrupts
    // insertBlocked(p, sem)
    // turn on interrupts
    // turn on interrupts
    // scheduler()
}

void sys5()
{
    // set exception address array
    // turn on interrupts
    // reload state and continue execution
}

int sys6() // get CPU time
{
    // return cpu time
}

void sys7()
{

}

void sys8() // wait for I/O
{
    // interrupts off
    // sys4 on IO's semaphore
}







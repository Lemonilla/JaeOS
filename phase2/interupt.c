// interrupts.c
// Neal Troscinski & Timmy Wright

void InterruptHandler()
{
    // figure out highest priority device alert
    // if end of quantum
    //      turn off alarm (optional)
    //      copy oldInt into currentProc->p_state
    //      increment currentProc cpu time
    //      put currentProc on readyQ
    //      scheduler()
    // get address of device register [status,command,data1,data2]
    // calculate index of device semaphore
    // copy status
    // set command to acknowledged
    // sys3 on device semaphore
    // if IO, decrement softblocked
    // put status in pcb gotten
    // put pcb on readyQ
    // load oldInt
}
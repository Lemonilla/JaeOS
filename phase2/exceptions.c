// exceptions.c

void exceptionHandlers()
{
    // figure out what caused the issue
    // if programTrap, call programTrapHandler
    // if TLBexception, call TLBExceptionHandler
    // it is an interrupt, call InterruptHandler
}

void programTrapHandler()
{
    // if pcb_t->trapHander[w/e we need] == NULL 
    //      Glass 'em
    // otherwise
    // copy state into old state
    // copy handlerState into new
    // load new state
}

void TLBExceptionHandler()
{
    // if pcb_t->trapHander[w/e we need] == NULL 
    //      Glass 'em  
    // otherwise
    // copy state into old state
    // copy handlerState into new
    // load new state
}

void InterruptHandler()
{
    // turn off alarm
    // call sys3 on the sem address of the device that caused alarm
    // return to current proc
}
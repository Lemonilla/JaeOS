// scheduler.c

void scheduler()
{
    // if list is empty
        // check if processCount = 0
            // is so, HALT()
            // if not:
                // check softblock count = 0
                    // if so, PANIC();
                    // if not, WAIT();
    // if list is not empty
        // p = removeProcQ()
        // set timer
        // load state in p
}
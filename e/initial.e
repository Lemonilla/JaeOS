#ifndef INITIAL
#define INITIAL

#include "../h/types.h"

extern int processCount;
extern int softBlockCount;
extern pcb_t* readyQ;
extern pcb_t* currentProc;

// index of device semaphore is (Line# - 1) * 16 + Device#
extern int devSem[16*NUMOFDEVICELINES];

#endif
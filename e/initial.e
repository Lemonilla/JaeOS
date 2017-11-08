#ifndef INITIAL
#define INITIAL

#include "../h/types.h"

extern int processCount;
extern int softBlockCount;
extern pcb_t* readyQ;
extern pcb_t* currentProc;
extern int startTime_Hi;
extern int startTime_Lo;

// index of device semaphore is (Line# - 1) * 16 + Device#
extern int devSem[16*NUMOFDEVICELINES];

extern void copyState(state_t* copy, state_t* initial);
extern int getTimeRunning();


extern void debug(int a, int b, int c, int d);

#endif
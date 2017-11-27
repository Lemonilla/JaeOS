#ifndef INITIAL
#define INITIAL

#include "../h/types.h"

extern int processCount;
extern int softBlockCount;
extern pcb_t* readyQ;
extern pcb_t* currentProc;
extern uint startTime_Hi;
extern uint startTime_Lo;

extern int Sys7WakeupTimestamp;
extern int QuantomPart2;

// index of device semaphore is Line# * DEVICESPERLINE + Device#
extern int devSem[DEVICESPERLINE*NUMOFDEVICELINES];
extern int devStat[DEVICESPERLINE*NUMOFDEVICELINES];

extern void copyState(state_t* copy, state_t* initial);
extern void updateTime();
extern void resetStopwatch();

#endif
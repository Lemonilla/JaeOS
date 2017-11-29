#ifndef INITIAL
#define INITIAL

#include "../h/types.h"

extern volatile int processCount;
extern volatile int softBlockCount;
extern volatile pcb_t* readyQ;
extern volatile pcb_t* currentProc;
extern volatile uint startTime_Hi;
extern volatile uint startTime_Lo;

extern volatile int Sys7WakeupTimestamp;
extern volatile int QuantomPart2;

// index of device semaphore is Line# * DEVICESPERLINE + Device#
extern volatile int devSem[DEVICESPERLINE*NUMOFDEVICELINES];
extern volatile int devStat[DEVICESPERLINE*NUMOFDEVICELINES];

extern void copyState(state_t* copy, state_t* initial);
extern void updateTime();
extern void resetStopwatch();

#endif
/******************************TYPES.H*******************************
 * Written by Neal Troscinski, and Timothy Wright
 *
 * This header file contains utility constants & macro definitions.
 *******************************************************************/

#ifndef CONSTS
#define CONSTS



#include "/usr/include/uarm/arch.h"
#include "/usr/include/uarm/uARMconst.h"


#define MAXPROC                 20
#define NUMOFDEVICELINES        8
#define MAXINT                  0xFFFFFFFF
#define HIDDEN                  static
#define TRUE                    1
#define FALSE                   0

#define QUANTOM                 5000
#define PAGESIZE                4000

#define RAMTOP                  0x00000204
#define ALLOFF                  0x00000000
#define SYS_MODE                0x0000001F
#define USR_MODE                0x00000010
#define INT_DISABLED            0x000000C0
#define INT_ENABLED             0x00000000

#define INTNEW                  0x00007058
#define INTOLD                  0x00007000
#define TLBNEW                  0x00007108
#define TLBOLD                  0x000070B0
#define PGMTNEW                 0x000071B8
#define PGMTOLD                 0x00007160
#define SYSNEW                  0x00007268
#define SYSOLD                  0x00007210

#define CUSTOM_HANDLER_NEW_OFFSET 3
#define CUSTOM_PGM_OLD            0
#define CUSTOM_TLB_OLD            1
#define CUSTOM_SYS_OLD            2
#define CUSTOM_PGM_NEW            3
#define CUSTOM_TLB_NEW            4
#define CUSTOM_SYS_NEW            5


#endif


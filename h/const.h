/******************************CONST.H*******************************
 * Written by Neal Troscinski, and Timothy Wright
 *
 * This header file contains utility constants & macro definitions.
 *******************************************************************/

#ifndef CONSTS
#define CONSTS

#include "../h/types.h"
#include "/usr/include/uarm/arch.h"
#include "/usr/include/uarm/uARMconst.h"

// Coding Standards
#define MAXINT                      0xFFFFFFFF
#define HIDDEN                      static
#define TRUE                        1
#define FALSE                       0

// Global Defines
#define MAXPROC                     20
#define QUANTOM                     5000
#define WORDSIZE                    4
#define PAGESIZE                    FRAME_SIZE //4096
#define RAMTOP                      RAM_TOP //0x00000204

// CPSR Masks
#define ALLOFF                      0x00000000
#define SYS_MODE                    0x0000001F
#define USR_MODE                    0x00000010
#define INT_DISABLED                0x000000C0
#define INT_ENABLED                 0x00000000 

// Exception Vector
#define INTOLD                      0x00007000
#define INTNEW                      0x00007058
#define TLBOLD                      0x000070B0
#define TLBNEW                      0x00007108
#define PGMTOLD                     0x00007160
#define PGMTNEW                     0x000071B8
#define SYSOLD                      0x00007210
#define SYSNEW                      0x00007268

// custom handler indexes
#define CUSTOM_TLB                  0
#define CUSTOM_PGM                  1
#define CUSTOM_SYS                  2
#define CUSTOM_HANDLER_NEW_OFFSET   3
 
#define CUSTOM_TLB_OLD              0
#define CUSTOM_PGM_OLD              1
#define CUSTOM_SYS_OLD              2
#define CUSTOM_PGM_NEW              4
#define CUSTOM_TLB_NEW              3
#define CUSTOM_SYS_NEW              5

// Device Register addresses
#define DEV_FLAG_PTR                0x00006fe0
#define DEV_REG_TABLE               0x000002d8

// Device lines for interrupts
#define NUMOFDEVICELINES            8
#define DEVICESPERLINE              16
#define CLOCK_LINE                  2
#define DISK_LINE                   3
#define TAPE_LINE                   4
#define NETWORK_LINE                5
#define PRINTER_LINE                6
#define TERMINAL_LINE               7
#define PSUDOTIMER_SEM_INDEX        CLOCK_LINE * DEVICESPERLINE + 0 // Line 2 Device 0

// DEVICE REGISTER COMMANDS
#define CMD_RESET                   0
#define CMD_ACK                     1
#define CMD_SEEKCYL                 2
#define CMD_READBLK                 3
#define CMD_WRITEBLK                4

// PGMTrap Codes
#define RESERVED_INSTRUCTION_CODE   20

#endif


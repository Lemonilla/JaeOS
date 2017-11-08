/******************************TYPES.H*******************************
 * Written by Neal Troscinski, and Timothy Wright
 *
 * This header file contains utility constants & macro definitions.
 *******************************************************************/

#ifndef CONSTS
#define CONSTS


#include "/usr/include/uarm/arch.h"
#include "/usr/include/uarm/uARMconst.h"

#define MAXPROC                     20
#define NUMOFDEVICELINES            8
#define MAXINT                      0xFFFFFFFF
#define HIDDEN                      static
#define TRUE                        1
#define FALSE                       0

#define QUANTOM                     5000
#define WORDSIZE                    4
#define PAGESIZE                    4000

#define RAMTOP                      0x00000204
#define ALLOFF                      0x00000000
#define SYS_MODE                    0x0000001F
#define USR_MODE                    0x00000010
//#define INT_DISABLED                0x000000C0 // | with w/e to turn them off
//#define INT_ENABLED                 0xFFFFFF3F // & with w/e to turn them on
#define INT_ENABLED                 0x000000C0 // | with w/e to turn them off
#define INT_DISABLED                0x00000000 // & with w/e to turn them on




#define INTNEW                      0x00007058
#define INTOLD                      0x00007000
#define TLBNEW                      0x00007108
#define TLBOLD                      0x000070B0
#define PGMTNEW                     0x000071B8
#define PGMTOLD                     0x00007160
#define SYSNEW                      0x00007268
#define SYSOLD                      0x00007210

#define CUSTOM_HANDLER_NEW_OFFSET   3
#define CUSTOM_PGM_OLD              0
#define CUSTOM_TLB_OLD              1
#define CUSTOM_SYS_OLD              2
#define CUSTOM_PGM_NEW              3
#define CUSTOM_TLB_NEW              4
#define CUSTOM_SYS_NEW              5

#define PSUDOTIMER_SEM_INDEX        2 * 16 + 0 // Line 2 Device 0

#define MEMEORY_ERROR_CODE          1   // Memory Error
#define BUS_ERROR_CODE              2   // Bus Error on Load/Store
#define INTERRUPT_EXCEPTION_CODE    3   // Interrupt Exception
#define SYS_INTERRUPT_CODE          4   // Software Interrupt Exception
#define UNDEFINED_INSTRUCTION_CODE  5   // Undefined Instr.  Exception
#define COPROCESSOR_EXCEPTION_CODE  6   // Coprocessor Exception
#define BREAKPOINT_CODE             7   // Breakpoint Exception
#define SEGMENT_ERROR_CODE          8   // Segment Error
#define PAGE_ERROR_CODE             9   // Page Error
#define INVALID_PAGE_TABLE_CODE     10  // Bad/Invalid Page Table
#define PAGE_TABLE_MISS_CODE        11  // Page Table Miss
#define TLB_INVALID_LD_CODE         14  // TLB Invalid Exception on a Load Instr.  or Instr.  Fetch
#define TLB_INVALID_ST_CODE         15  // TLB Invalid Exception on a Store Instr.
#define ADDRESS_ERROR_LD_CODE       16  // Address Error Exception on a Load or Instr.  Fetch
#define ADDRESS_ERROR_ST_CODE       17  // Address Error Exception on a Store Instr.
#define TLB_MOD_ERROR               18  // TLB Modification Error
#define RESERVED_INSTRUCTION_CODE   20  // Reserved Instruction

#define FIRST_BIT_ON                0x80000000
#define SECOND_BIT_ON               0x40000000
#define THIRD_BIT_ON                0x20000000
#define FOURTH_BIT_ON               0x10000000
#define FIFTH_BIT_ON                0x08000000
#define SIXTH_BIT_ON                0x04000000
#define SEVENTH_BIT_ON              0x02000000
#define EIGTH_BIT_ON                0x01000000

#define DEV_FLAG_PTR                0x00006fe0
#define DEV_REG_TABLE               0x000002d8


// DEVICE REGISTER COMMANDS
#define CMD_RESET                   0
#define CMD_ACK                     1
#define CMD_SEEKCYL                 2
#define CMD_READBLK                 3
#define CMD_WRITEBLK                4

#endif


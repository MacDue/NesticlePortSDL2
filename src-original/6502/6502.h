/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                          6502.h                         **/
/**                                                         **/
/** This file contains declarations relevant to emulation   **/
/** of 6502 CPU.                                            **/
/**                                                         **/
/** Copyright (C) Alex Krasivsky  1996                      **/
/**               Marat Fayzullin 1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/   
/**     changes to this file.                               **/
/*************************************************************/

#define INTERRUPTS             /* Compile interrupts code    */
/* #define DEBUG */            /* Compile debugging version  */
 #define LSB_FIRST         /* Compile for low-endian CPU */

#define INT_IRQ	1              /* Interrupt() return values  */
#define INT_NMI	2

#define	C_FLAG	0x01           /* Carry flag [1 = true]      */
#define	Z_FLAG	0x02           /* Zero flag                  */
#define	I_FLAG	0x04           /* Interrupts [1 = disabled]  */
#define	D_FLAG	0x08           /* Decimal mode [1 = true]    */
#define	B_FLAG	0x10           /* Break [0 on stk after int] */
#define	R_FLAG	0x20           /* Always 1                   */
#define	V_FLAG	0x40           /* Overflow [1 = true]        */
#define	N_FLAG	0x80           /* Negative [1 = negative]    */

/**********************************************************/
/*** NOTICE: sizeof(byte)=1 and sizeof(word)=2          ***/
/**********************************************************/
typedef unsigned char byte;
typedef unsigned short word;
typedef signed char offset;

/**********************************************************/
/*** #define LSB_FIRST for machines where least         ***/
/*** signifcant byte goes first.                        ***/
/**********************************************************/
typedef union
{
#ifdef LSB_FIRST
  struct { byte l,h; } B;
#else
  struct { byte h,l; } B;
#endif
  word W;
} pair;

typedef struct
{
  byte A,P,X,Y,S;
  pair PC;
} reg;

extern reg R;

/*** Interrupts *******************************************/
/*** Interrupt-related variables.                       ***/
/**********************************************************/
#ifdef INTERRUPTS
extern int  IPeriod; /* Number of cmds between int. intrpts */
extern byte IntSync; /* 1 to generate internal interrupts   */
extern byte IFlag;   /* If IFlag==1, gen. int. and set to 0 */
#endif

/*** Trace and Trap ***************************************/         
/*** Switches to turn tracing on and off in DEBUG mode. ***/
/**********************************************************/
#ifdef DEBUG
extern byte Trace;  /* Tracing is on if Trace==1  */
extern word Trap;   /* When PC==Trap, set Trace=1 */
#endif

/*** TrapBadOps *******************************************/
/*** When 1, print warnings of illegal 6502 instructions***/
/**********************************************************/
extern byte TrapBadOps;

/*** CPURunning *******************************************/
/*** When 0, execution terminates.                      ***/
/**********************************************************/
extern byte CPURunning;

/*** Reset 6502 registers: ********************************/
/*** This function can be used to reset the register    ***/
/*** file before starting execution with M6502(). It    ***/
/*** sets the registers to their initial values.        ***/
/**********************************************************/
void Reset6502(); //(reg *Regs);

/*** Interpret 6502 code: *********************************/
/*** Registers have initial values from Regs. PC value  ***/
/*** at which emulation stopped is returned by this     ***/
/*** function.                                          ***/
/**********************************************************/
word M6502(); //(reg Regs);

/*** RDMEM/WRMEM ******************************************/
/*** These functions are called when access to RAM      ***/
/*** occurs. They allow to control memory access.       ***/
/*** Replace with macros for no checks.                 ***/
/**********************************************************/
void M_WRMEM(word A,byte V);
byte M_RDMEM(word A);

#ifdef DEBUG
/*** Single-step debugger *********************************/
/*** This function should exist if DEBUG is #defined.   ***/
/*** If Trace==1 it is called after each command        ***/
/*** executed by the CPU and given address of the       ***/
/*** address space and the register file.               ***/
/**********************************************************/
void Debug(reg *R);
#endif

#ifdef INTERRUPTS
/*** Interrupt() ******************************************/
/*** This function should exist if INTERRUPTS is        ***/
/*** #defined. It is called on each suspected interrupt ***/
/*** and should return either 0 [no interrupt], or      ***/
/*** INT_IRQ [maskable interrupt], or INT_NMI [non-     ***/
/*** maskable interrupt].                               ***/
/**********************************************************/
byte Interrupt(void);
#endif

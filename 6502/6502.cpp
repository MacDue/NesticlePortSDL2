/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                          6502.c                         **/
/**                                                         **/
/** This file contains implementation for 6502 CPU. It can  **/
/** be used separately to emulate any 6502-based machine.   **/
/** In this case you will need: 6502.c, 6502.h, Codes.h.    **/
/** Don't forget to edit M_RDMEM() and M_WRMEM() definit-s  **/
/** to accomodate the emulated machine architecture.        **/
/**                                                         **/
/** Copyright (C) Alex Krasivsky  1996                      **/
/**               Marat Fayzullin 1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/   
/**     changes to this file.                               **/
/*************************************************************/

#include <stdio.h>
#include "6502.h"
#include "Tables.h"

#include "message.h"
/*** Registers ***********************************************/
/*** 6502 registers, and running flag.                     ***/
/*************************************************************/
reg R;
byte CPURunning;

/*** Interrupts **********************************************/
/*** Interrupt-related variables.                          ***/
/*************************************************************/
#ifdef INTERRUPTS
int  IPeriod = 10000; /* Number of cmds between int. intrpts */
byte IntSync = 1;     /* 1 to generate internal interrupts   */
byte IFlag = 0;       /* If IFlag==1, gen. int. and set to 0 */
#endif

/*** Trace and Trap ******************************************/
/*** Switches to turn tracing on and off in DEBUG mode.    ***/
/*************************************************************/
#ifdef DEBUG
byte Trace=0;       /* Tracing is on if Trace==1  */
word Trap=0xFFFF;   /* When PC==Trap, set Trace=1 */
#endif

/*** TrapBadOps **********************************************/
/*** When 1, print warnings of illegal 6502 instructions.  ***/
/*************************************************************/
byte TrapBadOps=0;

/*************************************************************/
/*** These macros calculate and return effective address.  ***/
/*************************************************************/
#define MC_Ab(Rg)	M_LDWORD(Rg)
#define MC_Zp(Rg)	Rg.B.l=M_RDMEM(R.PC.W++);Rg.B.h=0
#define MC_Zx(Rg)	Rg.B.l=M_RDMEM(R.PC.W++)+R.X;Rg.B.h=0
#define MC_Zy(Rg)	Rg.B.l=M_RDMEM(R.PC.W++)+R.Y;Rg.B.h=0
#define MC_Ax(Rg)	M_LDWORD(Rg);Rg.W+=R.X
#define MC_Ay(Rg)	M_LDWORD(Rg);Rg.W+=R.Y
#define MC_Ix(Rg)	K.B.l=M_RDMEM(R.PC.W++)+R.X;K.B.h=0; \
			Rg.B.l=M_RDMEM(K.W++);Rg.B.h=M_RDMEM(K.W)
#define MC_Iy(Rg)	K.B.l=M_RDMEM(R.PC.W++);K.B.h=0; \
			Rg.B.l=M_RDMEM(K.W++);Rg.B.h=M_RDMEM(K.W); \
			Rg.W+=R.Y

/*************************************************************/
/*** Macros to read memory.                                ***/
/*************************************************************/
#define MR_Ab(Rg)	MC_Ab(J);Rg=M_RDMEM(J.W)
#define MR_Im(Rg)	Rg=M_RDMEM(R.PC.W++)
#define	MR_Zp(Rg)	MC_Zp(J);Rg=M_RDMEM(J.W)
#define MR_Zx(Rg)	MC_Zx(J);Rg=M_RDMEM(J.W)
#define MR_Zy(Rg)	MC_Zy(J);Rg=M_RDMEM(J.W)
#define	MR_Ax(Rg)	MC_Ax(J);Rg=M_RDMEM(J.W)
#define MR_Ay(Rg)	MC_Ay(J);Rg=M_RDMEM(J.W)
#define MR_Ix(Rg)	MC_Ix(J);Rg=M_RDMEM(J.W)
#define MR_Iy(Rg)	MC_Iy(J);Rg=M_RDMEM(J.W)

/*************************************************************/
/*** Macros to write memory.                               ***/
/*************************************************************/
#define MW_Ab(Rg)	MC_Ab(J);M_WRMEM(J.W,Rg)
#define MW_Zp(Rg)	MC_Zp(J);M_WRMEM(J.W,Rg)
#define MW_Zx(Rg)	MC_Zx(J);M_WRMEM(J.W,Rg)
#define MW_Zy(Rg)	MC_Zy(J);M_WRMEM(J.W,Rg)
#define MW_Ax(Rg)	MC_Ax(J);M_WRMEM(J.W,Rg)
#define MW_Ay(Rg)	MC_Ay(J);M_WRMEM(J.W,Rg)
#define MW_Ix(Rg)	MC_Ix(J);M_WRMEM(J.W,Rg)
#define MW_Iy(Rg)	MC_Iy(J);M_WRMEM(J.W,Rg)

/*************************************************************/
/*** Macros to modify memory.                              ***/
/*************************************************************/
#define MM_Ab(Cmd)	MC_Ab(J);I=M_RDMEM(J.W);Cmd(I);M_WRMEM(J.W,I)
#define MM_Zp(Cmd)	MC_Zp(J);I=M_RDMEM(J.W);Cmd(I);M_WRMEM(J.W,I)
#define MM_Zx(Cmd)	MC_Zx(J);I=M_RDMEM(J.W);Cmd(I);M_WRMEM(J.W,I)
#define MM_Ax(Cmd)	MC_Ax(J);I=M_RDMEM(J.W);Cmd(I);M_WRMEM(J.W,I)

#define M_FL(Rg)	R.P=(R.P&~(Z_FLAG|N_FLAG))|ZNTable[Rg]
#define M_LDWORD(Rg)	Rg.B.l=M_RDMEM(R.PC.W++);Rg.B.h=M_RDMEM(R.PC.W++)

#define M_PUSH(Rg)	M_WRMEM(0x0100|R.S,Rg);R.S--
#define M_POP(Rg)	R.S++;Rg=M_RDMEM(0x0100|R.S)

#ifdef INTERRUPTS
#define M_JR		R.PC.W+=(offset)M_RDMEM(R.PC.W)+1;ICount--
#else
#define M_JR		R.PC.W+=(offset)M_RDMEM(R.PC.W)+1
#endif

#define M_ADC(Rg) \
  K.W=R.A+Rg+(R.P&C_FLAG); \
  if(R.P&D_FLAG) msg.error("Decimal ADC not implemented\n"); \
  R.P&=~(N_FLAG|V_FLAG|Z_FLAG|C_FLAG); \
  R.P|=(~(R.A^Rg)&(R.A^K.B.l)&0x80? V_FLAG:0)| \
       (K.B.h? C_FLAG:0)|ZNTable[K.B.l]; \
  R.A=K.B.l

/* Warning! C_FLAG is inverted before SBC and after it */
#define M_SBC(Rg) \
  K.W=R.A-Rg-(~R.P&C_FLAG); \
  if(R.P&D_FLAG) msg.error("Decimal SBC not implemented\n"); \
  R.P&=~(N_FLAG|V_FLAG|Z_FLAG|C_FLAG); \
  R.P|=((R.A^Rg)&(R.A^K.B.l)&0x80? V_FLAG:0)| \
  (K.B.h? 0:C_FLAG)|ZNTable[K.B.l]; \
  R.A=K.B.l

#define M_CMP(Rg1,Rg2) \
  K.W=Rg1-Rg2; \
  R.P&=~(N_FLAG|Z_FLAG|C_FLAG); \
  R.P|=ZNTable[K.B.l]|(K.B.h? 0:C_FLAG)
#define M_BIT(Rg) \
  R.P&=~(N_FLAG|V_FLAG|Z_FLAG); \
  R.P|=(Rg&(N_FLAG|V_FLAG))|(Rg&R.A? 0:Z_FLAG)

#define M_AND(Rg)	R.A&=Rg;M_FL(R.A)
#define M_ORA(Rg)	R.A|=Rg;M_FL(R.A)
#define M_EOR(Rg)	R.A^=Rg;M_FL(R.A)
#define M_INC(Rg)	Rg++;M_FL(Rg)
#define M_DEC(Rg)	Rg--;M_FL(Rg)

#define M_ASL(Rg)	R.P&=~C_FLAG;R.P|=Rg>>7;Rg<<=1;M_FL(Rg)
#define M_LSR(Rg)	R.P&=~C_FLAG;R.P|=Rg&C_FLAG;Rg>>=1;M_FL(Rg)
#define M_ROL(Rg)	K.B.l=(Rg<<1)|(R.P&C_FLAG); \
			R.P&=~C_FLAG;R.P|=Rg>>7;Rg=K.B.l; \
			M_FL(Rg)
#define M_ROR(Rg)	K.B.l=(Rg>>1)|(R.P<<7); \
			R.P&=~C_FLAG;R.P|=Rg&C_FLAG;Rg=K.B.l; \
			M_FL(Rg)

/*** Reset 6502 registers: ********************************/
/*** This function can be used to reset the register    ***/
/*** file before starting execution with M6502(). It    ***/
/*** sets the registers to their initial values.        ***/
/**********************************************************/
void Reset6502() //(reg *Regs)
{
  reg *Regs=&R;
  Regs->A=Regs->X=Regs->Y=0x00;
  Regs->P=Z_FLAG|R_FLAG;
  Regs->S=0xFF;
  Regs->PC.B.l=M_RDMEM(0xFFFC);
  Regs->PC.B.h=M_RDMEM(0xFFFD);

  msg.printf(1,"6502 initialized PC=%X",Regs->PC.W);   
}

extern int blah;

/*** Interpret 6502 code: *********************************/
/*** Registers have initial values from Regs. PC value  ***/
/*** at which emulation stopped is returned by this     ***/
/*** function.                                          ***/
/**********************************************************/
word M6502() //(reg Regs)
{
  static byte Cycles[256] =
  {
    7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    2,5,2,5,4,4,4,4,2,4,2,5,4,4,4,4,
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7
  };

  register byte I;
  register pair J,K;

#ifdef INTERRUPTS
  register int ICount=IPeriod;
  IFlag=0;
#endif

  //R=Regs;
//  Reset6502(&R);
  CPURunning=1;

  for(;;)
  {
#ifdef DEBUG
    if(R.PC.W==Trap) Trace=1;  /*** Turn tracing on if trapped ***/
    if(Trace) Debug(&R);       /*** Call single-step debugger  ***/
#endif

    I=M_RDMEM(R.PC.W++);

#ifdef INTERRUPTS
    ICount-=Cycles[I];
#endif
    blah+=Cycles[I];

    switch(I)
    {
#include "Codes.h"
      default:
        if(TrapBadOps)
        {
          msg.error
          (
            "Unrecognized instruction: $%02X at PC=$%04X\n",
            M_RDMEM(R.PC.W-1),(word)(R.PC.W-1)
          );
          return 0;
        }
    }

#ifndef INTERRUPTS
    if(!CPURunning) break;
#else
    if(ICount<=0)
    {
      if(!CPURunning) break;
      ICount+=IPeriod;
      if(IntSync) IFlag=1;
    }
    if(IFlag)
    {
      IFlag=0;I= Interrupt();
      if((I==INT_NMI)||((I==INT_IRQ)&&!(R.P&I_FLAG)))
      {
        ICount-=7;
        R.PC.W--;
        M_PUSH(R.PC.B.h);
        M_PUSH(R.PC.B.l);
        M_PUSH(R.P);
        R.P&=~D_FLAG;
        if(I==INT_IRQ) R.P|=I_FLAG;
        J.W=(I==INT_NMI)? 0xFFFA:0xFFFE;
        R.PC.B.l=M_RDMEM(J.W++);
        R.PC.B.h=M_RDMEM(J.W);
      }
    }
#endif
  }

  return(R.PC.W);
}

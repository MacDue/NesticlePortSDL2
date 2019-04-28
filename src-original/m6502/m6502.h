/* Multi-6502 32 Bit emulator */

/* Copyright 1996, Neil Bradley, All rights reserved
 *
 * License agreement:
 *
 * The M6502 emulator may be distributed in unmodified form to any medium.
 *
 * M6502 May not be sold, or sold as a part of a commercial package without
 * the express written permission of Neil Bradley (neil@synthcom.com). This
 * includes shareware.
 *
 * Modified versions of M6502 may not be publicly redistributed without author
 * approval (neil@synthcom.com). This includes distributing via a publicly
 * accessible LAN. You may make your own source modifications and distribute
 * M6502 in object only form.
 *
 * M6502 Licensing for commercial applications is available. Please email
 * neil@synthcom.com for details.
 *
 * Synthcom Systems, Inc, and Neil Bradley will not be held responsible for
 * any damage done by the use of M6502. It is purely "as-is".
 *
 * If you use M6502 in a freeware application, credit in the following text:
 *
 * "Multi-6502 CPU emulator by Neil Bradley (neil@synthcom.com)"
 *
 * must accompany the freeware application within the application itself or
 * in the documentation.
 *
 * Legal stuff aside:
 *
 * If you find problems with M6502, please email the author so they can get
 * resolved. If you find a bug and fix it, please also email the author so
 * that those bug fixes can be propogated to the installed base of M6502
 * users. If you find performance improvements or problems with M6502, please
 * email the author with your changes/suggestions and they will be rolled in
 * with subsequent releases of M6502.
 *
 * The whole idea of this emulator is to have the fastest available 32 bit
 * Multi-6502 emulator for the PC, giving maximum performance. 
 */ 

/* General 6502 related goodies */

#ifndef DWORD
#define	DWORD	unsigned long int
#endif

#ifndef WORD
#define	WORD	unsigned short int
#endif

#ifndef BYTE
#define	BYTE	unsigned char
#endif

/* Some bitwise defines for the m6502GetInfo call */

#define		MULTI_CPU			0x00010000	/* Set if multi CPU support */
#define		TIMING_INFO			0x00020000	/* Set if timing info present */
#define		SYNC					0x00040000	/* Set if SYNC support present */
#define		BOUNDS_CHECK		0x00080000	/* Set if bounds checking on */
extern "C" {
#define		EXEC_METHOD			0x00100000	/* Set if we're counting cycles */

extern DWORD m6502clockticks;
extern BYTE *m6502Base;
extern WORD m6502pc;
extern WORD m6502af;
extern BYTE m6502x;
extern BYTE m6502y;
extern BYTE m6502s;
extern DWORD __cdecl m6502nmi(void);
extern DWORD __cdecl m6502int(void);
extern DWORD __cdecl m6502exec(DWORD);
extern void __cdecl m6502reset(void);
extern DWORD __cdecl m6502GetInfo(void);
extern DWORD cyclesRemaining;
extern BYTE syncCycle;

struct m6502MemoryWrite
{
	WORD lowAddr;
	WORD highAddr;
	int (*memoryCall)(WORD, BYTE);
};

struct m6502MemoryRead
{
	WORD lowAddr;
	WORD highAddr;
	WORD (*memoryCall)(WORD);
};

extern struct m6502MemoryRead *m6502MemoryRead;
extern struct m6502MemoryWrite *m6502MemoryWrite;

/* WARNING: Do not use a sizeof() operator on this structure. The assembly
 * code will pack other context relevant data after this space. Use the
 * GetContextSize() call to obtain the specifics of the context.
 *
 * Creating a context is not necessary if you are only running a single CPU
 * emulation.
 */

struct m6502context
{
	BYTE *m6502Base;				/* 32 Bit pointer to base memory address */

	/* 32 Bit pointers to user trap address structure array */

	struct m6502MemoryRead *m6502MemoryRead;
	struct m6502MemoryWrite *m6502MemoryWrite;

	/* Number of clockticks */

	DWORD m6502clockticks;

	/* This context's register dump */

	WORD m6502af;			/* A & Flags */
	WORD m6502pc;			/* Program counter */
	BYTE m6502x;			/* X register */
	BYTE m6502y;			/* Y register */
	BYTE m6502s;			/* S register */
	BYTE syncCycle;		/* Non-zero if this is a sync-cycle instruction */
	BYTE inNmi;				/* Non-zero if we're in an NMI */
};

typedef struct m6502context CONTEXT;

extern void m6502SetContext(CONTEXT *);
extern void m6502GetContext(CONTEXT *);
extern DWORD m6502GetContextSize();

}

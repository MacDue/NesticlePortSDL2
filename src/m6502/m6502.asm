;************************************************************************
;*                                                                      *
;* Multi-6502 Emulation Engine                                          *
;*                                                                      *
;* Copyright 1997, Neil Bradley                                         *
;* Version 1.0								*
;*                                                                      *
;* This code is free to distribute and use in unmodified form for your  *
;* own projects requiring a 6502 emulator. This emulator is designed to *
;* give the best emulation performance on a PC as possible.             *
;*                                                                      *
;* This code, and no part of this code, may not be used in any          *
;* commercial or for-profit venture without the express written         *
;* permission of Neil Bradley. Credit must be given within the program  *
;* that uses this emulator OR in accompanying documentation.            *
;*                                                                      *
;* Contact information:                                                 *
;*                                                                      *
;* Neil Bradley      - neil@synthcom.com - Author & architect           *
;*                                                                      *
;************************************************************************
;*                                                                      *
;* Release history:                                                     *
;*                                                                      *
;* 02/24/97 - Release 1.0						*
;*                                                                      *
;************************************************************************

.486p
.model flat,c

;************************************************************************
;*                                                                      *
;* Philosophy of operation                                              *
;*                                                                      *
;* M6502 Is very macro based. This is to reduce the number of calls &   *
;* jumps that is critical to its operation. One could further modulize  *
;* this emulator by providing a very large call table for all adressing *
;* modes in addition to each call, but that would take significantly    *
;* longer to execute. The goal of this emulator is to be as fast as     *
;* possible for the 486 and higher CPUs.                                *
;*                                                                      *
;* All of the most commonly used 6502 registers are kept in x86         *
;* registers for increased speed. They are:                             *
;*                                                                      *
;* EAX - Instruction fetch, general purpose, etc... be sure to ensure   *
;*       that the upper 24 bits of this register are clear when         *
;*       returning from an emulated function. It uses this to compute   *
;*       an offset to where to jump for the next instruction.           *
;*                                                                      *
;* EBX - BX=The 6502's X register. Leave the upper 24 bits clear so it  *
;*       can quickly access information in the emulated RAM. Not doing  *
;*       so will give unpredictable results.                            *
;*                                                                      *
;* ECX - CX=The 6502's Y register. As with the BX register, leave the   *
;*       upper 24 bits of ECX ZERO.                                     *
;*                                                                      *
;* EDX - DH=The 6502's flags, DL=The 6502's accumulator                 *
;*       The upper 16 bits are not used. Leave at zero.                 *
;*                                                                      *
;* ESI - The 6502's program counter + emulation address space base addr.*
;*       Used in instruction fetching. All 32 bits are used as the      *
;*       currently executing address.                                   *
;*                                                                      *
;* EDI - Temporary address computation register.                        *
;*                                                                      *
;* EBP - The base address of the virtual 64K 6502 machine. 32 Bits!     *
;*                                                                      *
;* All registers persist while executing, and are saved when execution  *
;* is complete.                                                         *
;*                                                                      *
;* To use:                                                              *
;*                                                                      *
;* * From your C program, allocate 64K of memory space for the 6502     *
;*   emulator to run it. Set _m6502Memory to that address.              *
;*                                                                      *
;* * Call _m6502Reset to cause the 6502 to reset itself                 *
;*                                                                      *
;* * Set _m6502MemoryRead and _m6502MemoryWrite to your structures 	*
;*   dealing  with read/writes to various places in memory. (See	*
;*   m6502.H for more info                                              *
;*                                                                      *
;* * Set _m6502Base to the 6502's logical address of 0 (64K buffer)     *
;*                                                                      *
;* * Call _exec6502() to begin emulation execution. See the _exec6502() *
;*   comment header for more information on this call.                  *
;*                                                                      *
;* Optional operational calls:                                          *
;*                                                                      *
;* * _m6502int() - Call this when you wish to create a virtual 6502     *
;*   interrupt.                                                         *
;*                                                                      *
;* * _m6502nmi() - Call this when you wish to create a virtual 6502 non-*
;*   maskable interrupt.                                                *
;*                                                                      *
;* All variables within the CONTEXT comments need to be swapped out     *
;* when doing multi-6502 processing.                                    *
;*                                                                      *
;* To assemble:                                                         *
;*                                                                      *
;* For Watcom's WASM:    WASM /DWATCOM m6502.asm                        *
;* For MASM         :    ML /C /Cx /DMASM m6502.asm                     *
;* For TASM	    :	 TASM /DTASM m6502.asm				*
;*                                                                      *
;************************************************************************

;************************************************************************
;*                                                                      *
;* Equates, publics and options                                         *
;*                                                                      *
;************************************************************************

TASM equ 1

ifdef WATCOM
		public  _m6502clockticks
		public  _m6502Base
		public  _m6502MemoryRead
		public  _m6502MemoryWrite
		public  _m6502pc
		public  _m6502af
		public  _m6502x
		public  _m6502y
		public  _m6502s
		public	_cyclesRemaining
		public	_syncCycle

		public  m6502nmi_
		public  m6502int_
		public  m6502GetInfo_
		public  m6502exec_
		public  m6502reset_
endif

ifdef MASM
		public  m6502clockticks
		public  m6502Base
		public  m6502MemoryRead
		public  m6502MemoryWrite
		public  m6502pc
		public  m6502af
		public  m6502x
		public  m6502y
		public  m6502s
		public	cyclesRemaining
		public	syncCycle

		public  m6502nmi
		public  m6502int
		public  m6502GetInfo
		public  m6502exec
		public  m6502reset
endif

ifdef TASM
		public  m6502clockticks
		public  m6502Base
		public  m6502MemoryRead
		public  m6502MemoryWrite
		public  m6502pc
		public  m6502af
		public  m6502x
		public  m6502y
		public  m6502s
		public	cyclesRemaining
		public	syncCycle

		public  m6502nmi
		public  m6502int
		public  m6502GetInfo
		public  m6502exec
		public  m6502reset
endif

MAJOR_REV       equ     1       ; Major revision
MINOR_REV       equ     0       ; Minor revision
MULTI_CPU       equ     0       ; Enable multi-CPU support
INST_COUNT	equ	0	; We're counting down by instructions
CYCLE_COUNT	equ	1	; We're counting down by virtual clock time

ifdef MULTI_CPU

ifdef WATCOM
		public  m6502GetContextSize_
		public  m6502SetContext_
		public  m6502GetContext_
endif

ifdef MASM
		public  m6502GetContextSize
		public  m6502SetContext
		public  m6502GetContext
endif

ifdef TASM
		public  m6502GetContextSize
		public  m6502SetContext
		public  m6502GetContext
endif

endif

; M6502.ASM OPTIONS:

; Comment this out to not include timing information in emulation

TICKS           equ     1       ; Uncomment if we want a clock count

; Comment this out if you don't need to monitor the SYNC line

;SYNC_WANTED    equ     1       ; Uncomment if we want SYNC output

; Comment this out when you have released your code. This does a check to
; make sure that each instruction is fetched within the 64K segment in
; question. It will slow execution significantly, so comment it back out when
; you're ready to release your emulation.

;BOUNDS_CHECK    equ     1      ; Uncomment if you want bounds checking for
				; address space. Comment this back out when
				; your emulation is working properly

; This controls what the value passed to m6502exec actually means. If this
; is set to the INST_COUNT method, then it will consider the value input to be
; the number of virtual instructions left to execute. If this is set to
; CYCLE_COUNT then it is considered to be the number of virtual cycles left
; to execute.
;
; Be warned that changing this from INST_COUNT to CYCLE_COUNT will give
; approximately a 9% performance hit. If the image being emulated doesn't
; need exact timing, then keep it at INST_COUNT and approximate the instructions
; that pass between each cycle.

CYCLE_METHOD	equ	CYCLE_COUNT ; Count instructions

; ** DO NOT MUCK WITH THESE LABELS **

; FLAG REGISTER BIT FIELD NAMES
;

CARRY           equ     01h     ; Carry flag
ZERO            equ     02h     ; Zero flag
INTERRUPT       equ     04h     ; Interrupt flag
DECIMAL         equ     08h     ; That dreaded decimal mode
BREAK           equ     10h     ; Break flag

; Note that 20h is not used

OVERFLOW        equ     40h     ; Overflow flag
SIGN            equ     80h     ; Sign flag

;************************************************************************
;*                                                                      *
;* Macros                                                               *
;*                                                                      *
;************************************************************************

; Used when fetching instructions. It fetches the byte into AL from what is
; pointed to by ESI and leaves all other registers undisturbed

FetchInstructionByte    macro

; If you're using a 486, use the LODSW. If you're using a Pentium, use the
; instructions following it. It gives about a 7% increase in speed.

;		lodsb                   ; Fetch the next instruction into AL

		mov	al, [esi]
		inc	esi

		endm

; Used when fetching part of instructions (addresses, offsets, etc...). It
; fetches the two byte data into AX from ESI and leaves the other registers
; undisturbed

FetchInstructionWord    macro

; If you're using a 486, use the LODSW. If you're using a Pentium, use the
; instructions following it. It gives about a 7% increase in speed.

;		lodsw                   ; Fetch the next instruction word into AX

		mov	ax, [esi]
		add	esi, 2

		endm

; Used when attempting to do a memory read from any address. The read address
; is expected to be in AX - the 6502's 64K memory space. Any fixups are done
; by routines that may have it covered. AL=Byte that just got read.

ReadMemoryByte  macro
		local   checkLoop, nextAddr, memoryRead, readMacroExit, callRoutine

ifdef WATCOM
		mov     edi, [_m6502MemoryRead] ; Point to the read array
endif

ifdef MASM
		mov     edi, [m6502MemoryRead] ; Point to the read array
endif

ifdef TASM
		mov     edi, [m6502MemoryRead] ; Point to the read array
endif

checkLoop:
		cmp     [edi], word ptr -1 ; Is it the end of the list?
		je      memoryRead      ; Yes, go read it

		cmp     ax, [edi]       ; Are we smaller?
		jb      nextAddr        ; Yes... go to the next address

		cmp     ax, [edi+2]     ; Are we bigger?
		jbe     callRoutine

nextAddr:
		add     edi, 8          ; To the next structure
		jmp     short checkLoop ; Keep looping until we're done

callRoutine:
		push    esi
		push    ebx
		push    ecx
		push    edx
		push    ebp

		call    dword ptr [edi + 4]     ; Go call our handler!

		pop     ebp
		pop     edx
		pop     ecx
		pop     ebx
		pop     esi

		jmp     short readMacroExit

memoryRead:
		mov     al, [ebp + eax] ; Go get it!

readMacroExit:
		and     eax, 0ffh       ; Only the lower byte
		xor     edi, edi        ; Zero EDI
		endm

; Used when writing a memory byte to an address space. The address is
; expected to be in AX, and the byte to write in DL. Any fixups are done by
; the routines that have covered it.

WriteMemoryByte macro
		local   checkLoop, nextAddr, callRoutine, memoryWrite, writeMacroExit, noReloadPc

ifdef WATCOM
		mov     edi, [_m6502MemoryWrite] ; Point to the write array
endif

ifdef MASM
		mov	edi, [m6502MemoryWrite] ; Point to the write array
endif

ifdef TASM
		mov	edi, [m6502MemoryWrite] ; Point to the write array
endif

checkLoop:
		cmp     [edi], word ptr -1 ; Is it the end of the list?
		je      memoryWrite     ; Yes, go write it

		cmp     ax, [edi]       ; Are we smaller?
		jb      nextAddr        ; Yes... go to the next address

		cmp     ax, [edi+2]     ; Are we bigger?
		jbe     callRoutine

nextAddr:
		add     edi, 8          ; To the next structure
		jmp     short checkLoop ; Keep looping until we're done

callRoutine:
		push    ebx
		push    ecx
		push    edx
		push    esi
		push    ebp

		call    dword ptr [edi + 4]     ; Go call our handler!

		pop     ebp
		pop     esi

		or	al, al
		jz	noReloadPc

; Reload the program counter

		sub	esi, ebp	; Get PC back

ifdef WATCOM
		mov	ebp, _m6502Base	; Get our base address
endif

ifdef MASM
		mov	ebp, m6502Base	; Get our base address
endif

ifdef TASM
		mov	ebp, m6502Base	; Get our base address
endif

		add	esi, ebp	; Add it back in!		

noReloadPc:
		pop     edx
		pop     ecx
		pop     ebx

		and     eax, 0ffffh
		jmp     short writeMacroExit

memoryWrite:
		mov     [ebp + eax], dl ; Go get it!

writeMacroExit:
		xor     edi, edi        ; Zero EDI
		endm

; This is the macro to use to add something to our clock.

Clocktime       macro   addedTime

ifdef TICKS

ifdef WATCOM
		add     _m6502clockticks, addedTime
endif

ifdef MASM
		add     m6502clockticks, addedTime
endif

ifdef TASM
		add     m6502clockticks, addedTime
endif

endif

ifdef SYNC_WANTED

if addedTime GT 5

ifdef WATCOM
		mov     byte ptr _syncCycle, 1
endif

ifdef MASM
		mov     byte ptr syncCycle, 1
endif

ifdef TASM
		mov     byte ptr syncCycle, 1
endif

else

ifdef WATCOM
		mov     byte ptr _syncCycle, 0
endif

ifdef MASM
		mov     byte ptr syncCycle, 0
endif

ifdef TASM
		mov     byte ptr syncCycle, 0
endif

endif

endif ; SYNC_WANTED

		endm

; This macro is used at the end of an emulated function. It will return to the
; main execution loop to get the next instruction. IT IS SUPPOSED TO BE A
; JUMP AND NOT A CALL!

ExecuteNextInstruction  macro	clkTiming
		local   noMoreStuff, noHosed

; If we're doing a countdown of the # of instructions, then use this code

if CYCLE_METHOD EQ INST_COUNT

ifdef WATCOM
		dec     _cyclesRemaining         ; Next!
endif

ifdef MASM
		dec	cyclesRemaining		; Next !
endif

ifdef TASM
		dec	cyclesRemaining		; Next !
endif

		jz	noMoreStuff		; If we have nothing left...
endif

; If we're doing a countdown of virtual CPU clock cycles, then use THIS code

if CYCLE_METHOD EQ CYCLE_COUNT

ifdef WATCOM
		sub	_cyclesRemaining, clkTiming ; Subtract the virtual clock ticks
endif

ifdef MASM
		sub	cyclesRemaining, clkTiming ; Subtract the virtual clock ticks
endif

ifdef TASM
		sub	cyclesRemaining, clkTiming ; Subtract the virtual clock ticks
endif

		jc	noMoreStuff		; If we have underflowed...
endif


ifdef BOUNDS_CHECK
		mov     eax, esi                ; Get our naturalized PC
		sub     eax, ebp                ; Subtract base memory address
		test    eax, 0ffff0000h         ; Are we out of range?
		jnz     OutOfRange

		xor     eax, eax                ; Zero here for index lookup
endif
		FetchInstructionByte
		jmp     dword ptr m6502regular[eax*4] ; Jump to our instruction!

noMoreStuff:

ifdef WATCOM
		jmp     short instReturn
endif

; MASM Is brain-dead about automatically extending shorts to longs, so it
; needs everything as a long

ifdef MASM
		jmp     instReturn
endif

ifdef TASM
		jmp	instReturn
endif

		endm

; This macro is used at the beginning of every emulated function, and is
; different between assemblers

Alignment       macro

ifdef WATCOM
align 16
endif

ifdef MASM
align 
endif

ifdef TASM
align
endif
		endm

; Macro used when all flags need to be set (value in AL)

FlagSet         macro
		and     dh, 07dh        ; Knock out zero and SIGN
		xor     ah, ah          ; Zero this just in case
		or      dh, byte ptr flagTable[eax] ; Go get our flag

		endm

FlagSetX        macro

		and     dh, 07dh        ; Knock out zero and SIGN
		or      dh, byte ptr flagTable[ebx] ; Go get our flag

		endm

FlagSetY        macro

		and     dh, 07dh        ; Knock out zero and SIGN
		or      dh, byte ptr flagTable[ecx] ; Go get our flag

		endm

; Macro used when all flags need to be set from value in ACCUMULATOR

FlagSetAcc      macro

		mov     al, dl          ; Move accumulator into AL
		and     dh, 07dh        ; Knock out ZERO and SIGN
		xor	ah, ah		; Zero this for offset
		or      dh, byte ptr flagTable[eax] ; OR it in!

		endm

; Macro used when flags need to be set EXCEPT overflow (value in AL)

FlagSetNoOv     macro

		and	dh, 07dh	; No sign - just zero
		xor	ah, ah		; Zero this for lookup
		or	dh, byte ptr flagNoOvTable[eax]

		endm

; Macro used when flags need to be set EXCEPT overflow value in ACCUMULATOR

FlagSetNoOvAcc  macro
		local   l1

		and     dh, 07dh        ; Now we're zero
		xor	ah, ah		; Zero this for table lookup
		mov	al, dl		; Make it the accumulator lookup
		or	dh, byte ptr flagNoOvTable[eax]

		endm

; This is the ADC macro used by various instances of ADC. The value to add
; to the accumulator is in AL.

AdcMacro        macro
		local   addIt, noCarry, noSign, noAdcCarry, adcCont
		local   doDecAdc, noCarrySetDecAdc, skipTensUpgrade
		local   noCarryDecAdc, setFlags, adcOut

		push    cx

		test    dh, DECIMAL     ; Are we in decimal mode?
		jnz     doDecAdc

addIt:
		mov     ch, al          ; Store our original data
		add     al, dl          ; Add in A
		adc     ah, 0           ; If we have a carry...
		test    dh, CARRY       ; Carry set?
		jz      noCarry
		inc     ax              ; Increment A
noCarry:

; Let's check out and see if we've overflowed

		and     dh, 0bfh        ; No overflow
		xor     ch, dl          ; XOR A with our data
		test    ch, 80h         ; Sign extended?
		jnz     noSign
		
		mov     ch, dl          ; Get A
		xor     ch, al          ; XOR It with our new resultant
		jz      noSign

		or      dh, OVERFLOW    ; Indicate we've overflowed

noSign:
		mov     dl, al          ; Put A away for later

		or      ah, ah          ; Shall we carry?
		jz      noAdcCarry

		or      dh, 1           ; We've got a carry!
		jmp     short adcCont

noAdcCarry:
		and     dh, 0ffh - CARRY ; No carry! We didn't overflow

adcCont:
		FlagSet
		jmp     adcOut

; Here we need to add two decimal #'s together

doDecAdc:
		push    bx

		xor     bx, bx          ; Zero BX for later

; Add the first nibble together

		mov     ch, al          ; Source nibble
		mov     cl, dl          ; Destination nibble
		and     cx, 0f0fh       ; Only lower nibble allowed!
		add     cl, ch          ; Add them together
		test    dh, CARRY       ; Carry set?
		jz      noCarrySetDecAdc
		inc     cl              ; Increment!
noCarrySetDecAdc:
		cmp     cl, 0ah         ; Are we up there?
		jb      skipTensUpgrade
		mov     bh, 10h         ; Indicate we need to INC the upper nibble
		sub     cl, 0ah         ; Just the 10's location

; At this point, CL contains the lower BCD, and BH is non-zero if we've
; overflowed

skipTensUpgrade:
		mov     bl, cl          ; Put it in BL for later adding
		mov     ch, al          ; Source nibble
		mov     cl, dl          ; Source nibble
		and     cx, 0f0f0h      ; Only upper nibble this time
		add     cl, ch          ; Get it!
		add     cl, bh          ; Add in our lower BCD (if applicable)
		and     dh, 0beh        ; Clear carry and overflow
		cmp     cl, 0a0h        ; Are we too big?
		jb      noCarryDecAdc   ; Nope!

		or      dh, CARRY       ; Set virtual carry
		sub     cl, 0a0h        ; Make it be in range
noCarryDecAdc:
		or      bl, cl          ; Put our new value in BL
		mov     al, bl          ; Put it in AL

setFlags:
		pop     bx              ; Restore BX
		
		mov     dl, al          ; Put it in accumulator
		FlagSet
adcOut:
		pop     cx              ; Restore CX

		endm

; This is the SBC macro used by various instances of SBC. The value to subtract
; to the accumulator is in AL.

SbcMacro        macro
		local   noCarry, noSign, noSbcCarry, sbcCont
		local   sbcHandleDec, noSbcDec, noUnderFlow, noUnderFlowMSB
		local   skipIt, sbcOut

		push    cx              ; Save this away for later

		test    dh, DECIMAL     ; Are we in decimal mode?
		jnz     sbcHandleDec

		xor     al, 0ffh        ; Invert what we're adding
		mov     ch, al          ; Store our original data
		add     al, dl          ; Add in A
		adc     ah, 0           ; If we have a carry...
		test    dh, CARRY       ; Carry set?
		jz      noCarry
		inc     ax              ; Increment A
noCarry:

; Let's check out and see if we've overflowed

		and     dh, 0bfh        ; No overflow
		xor     ch, dl          ; XOR A with our data
		test    ch, 80h         ; Sign extended?
		jnz     noSign
		
		mov     ch, dl          ; Get A
		xor     ch, al          ; XOR It with our new resultant
		jz      noSign

		or      dh, OVERFLOW    ; Indicate we've overflowed

noSign:
		mov     dl, al          ; Put A away for later

		or      ah, ah          ; Shall we carry?
		jz      noSbcCarry

		or      dh, 1           ; We've got a carry!
		jmp     short sbcOut

noSbcCarry:
		and     dh, 0ffh - CARRY ; No carry! We didn't overflow
		jmp     short sbcOut

; Here we handle decimal mode

sbcHandleDec:
		push    bx              ; Save it because we wipe it out
		xor     bx, bx          ; Zero our borrow for later

		mov     ch, dl          ; A register in CH
		mov     cl, al          ; Data in CL
		and     cx, 0f0fh       ; Lower nibble only this time

		sub     ch, cl          ; Get our first digit's results
		test    dh, 01h         ; Carry set? (borrow)?
		jnz     noSbcDec
		dec     ch              ; Back up one for carry!
noSbcDec:
		cmp     ch, 0ah         ; Have we overflowed? (underflowed?)
		jb      noUnderFlow
		add     ch, 0ah         ; Add it in
		mov     bh, 10h         ; We take it from the most significant BCD

noUnderFlow:
		mov     bl, ch          ; Put final in BL

; We now have the lower BCD in BL, and any borrow in BH

		and     dh, 0beh        ; Clear carry and overflow flag

		mov     ch, dl          ; A register in CH
		mov     cl, al          ; Data in CL
		and     cx, 0f0f0h      ; Upper nibble this time
		sub     ch, cl          ; Get our second digit's results
		sub     ch, bh          ; Subtract our borrow (if applicable)
		cmp     ch, 0a0h        ; Did we underflow?
		jae     noUnderFlowMSB

		or      dh, 01h         ; Set carry if we underflowed
		jmp     short skipIt

noUnderFlowMSB:
		add     ch, 0a0h        ; Put it back to normal

skipIt:
		or      bl, ch          ; Store it in BL        

; Need to put it back into A, here!

		mov     al, bl
		mov     dl, bl
		pop     bx
sbcOut:
		FlagSet
		pop     cx
		endm


;************************************************************************
;*                                                                      *
;* Emulation data                                                       *
;*                                                                      *
;************************************************************************


ifdef WATCOM
_DATA           segment         para public 'DATA'
endif

ifdef MASM
_DATA           segment         para public 'DATA'
endif

ifdef TASM
_DATA           segment         dword public 'DATA'
endif

Alignment

; CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT

ifdef WATCOM
ContextBegin:
endif

ifdef MASM
ContextBegin::
endif

ifdef TASM
ContextBegin:
endif

ifdef WATCOM
_m6502Base      dd      0       ; Base address for 6502 stuff
_m6502MemoryRead dd     0       ; Offset of memory read structure array
_m6502MemoryWrite dd    0       ; Offset of memory write structure array
_m6502clockticks dd     0       ; # Of clock tips that have elapsed
_m6502af        dw      0       ; A register and flags
_m6502pc        dw      0       ; PC Register
_m6502x         db      0       ; X register
_m6502y         db      0       ; Y register
_m6502s         db      0       ; S register
_syncCycle      db      0       ; Non-zero if this is a sync cycle
_inNmi		db	0	; Non-zero if we're in an NMI
endif

ifdef MASM
m6502Base      dd      0       ; Base address for 6502 stuff
m6502MemoryRead dd     0       ; Offset of memory read structure array
m6502MemoryWrite dd    0       ; Offset of memory write structure array
m6502clockticks dd     0       ; # Of clock tips that have elapsed
m6502af        dw      0       ; A register and flags
m6502pc        dw      0       ; PC Register
m6502x         db      0       ; X register
m6502y         db      0       ; Y register
m6502s         db      0       ; S register
syncCycle      db      0       ; Non-zero if this is a sync cycle
inNmi		db	0	; Non-zero if we're in an NMI
endif

ifdef TASM
m6502Base      dd      0       ; Base address for 6502 stuff
m6502MemoryRead dd     0       ; Offset of memory read structure array
m6502MemoryWrite dd    0       ; Offset of memory write structure array
m6502clockticks dd     0       ; # Of clock tips that have elapsed
m6502af        dw      0       ; A register and flags
m6502pc        dw      0       ; PC Register
m6502x         db      0       ; X register
m6502y         db      0       ; Y register
m6502s         db      0       ; S register
syncCycle      db      0       ; Non-zero if this is a sync cycle
inNmi		db	0	; Non-zero if we're in an NMI
endif

Alignment

ifdef WATCOM
ContextEnd:
endif

ifdef MASM
ContextEnd::
endif

ifdef TASM
ContextEnd:
endif

; CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT CONTEXT

; This is general purpose non-persistent data

Alignment

ifdef WATCOM
_cyclesRemaining dd      0       ; # Of virtual CPU cycles left to execute
endif

ifdef MASM
cyclesRemaining dd      0       ; # Of virtual CPU cycles left to execute
endif

ifdef TASM
cyclesRemaining dd      0       ; # Of virtual CPU cycles left to execute
endif

Alignment

; This is the jump table for the regular instructions

m6502regular    dd      inst00                    ; 00h - BRK
		dd      inst01                    ; 01h - ORA A, INDX
		dd      invalidInsByte            ; 02h - Invalid
		dd      invalidInsByte            ; 03h - Invalid
		dd      invalidInsByte	          ; 04h - Invalid
		dd      inst05                    ; 05h - ORA $xx
		dd      inst06                    ; 06h - ASL $xx
		dd      invalidInsByte            ; 07h - Invalid
		dd      inst08                    ; 08h - PHP
		dd      inst09                    ; 09h - ORA #xxh
		dd      inst0a                    ; 0ah - ASL A
		dd      invalidInsByte            ; 0bh - Invalid
		dd	invalidInsByte		  ; 0ch - Invalid
		dd      inst0d                    ; 0dh - ORA $xxxxh
		dd      inst0e                    ; 0eh - ASL $xxxx
		dd      invalidInsByte            ; 0fh - Invalid
		dd      inst10                    ; 10h - BPL $xxxx
		dd      inst11		          ; 11h - ORA ($xx), Y
		dd      invalidInsByte            ; 12h - Invalid
		dd      invalidInsByte            ; 13h - Invalid
		dd      invalidInsByte            ; 14h - Invalid
		dd      inst15                    ; 15h - ORA $xx, X
		dd      inst16		          ; 16h - ASL $xx, X
		dd      invalidInsByte            ; 17h - Invalid
		dd      inst18                    ; 18h - CLC
		dd      inst19		          ; 19h - ORA $xxxx, Y
		dd      invalidInsByte            ; 1ah - Invalid
		dd      invalidInsByte            ; 1bh - Invalid
		dd      invalidInsByte            ; 1ch - Invalid
		dd      inst1d                    ; 1dh - ORA $xxxx, X
		dd      inst1e		          ; 1eh - ASL $xxxx, X
		dd      invalidInsByte            ; 1fh - Invalid
		dd      inst20                    ; 20h - JSR $xxxx
		dd      inst21		          ; 21h - AND ($xx), X
		dd      invalidInsByte            ; 22h - Invalid
		dd      invalidInsByte            ; 23h - Invalid
		dd      inst24                    ; 24h - BIT $xx
		dd      inst25                    ; 25h - AND $xx
		dd      inst26                    ; 26h - ROL $xx
		dd      invalidInsByte            ; 27h - Invalid
		dd      inst28                    ; 28h - PLP
		dd      inst29                    ; 29h - AND #xxh
		dd      inst2a                    ; 2ah - ROL A
		dd      invalidInsByte            ; 2bh - Invalid
		dd      inst2c                    ; 2ch - BIT $xxxx
		dd      inst2d                    ; 2dh - AND $xxxx
		dd      inst2e		          ; 2eh - ROL $xxxx
		dd      invalidInsByte            ; 2fh - Invalid
		dd      inst30                    ; 30h - BMI $xxxx
		dd      inst31		          ; 31h - AND ($xx), Y
		dd      invalidInsByte            ; 32h - Invalid
		dd      invalidInsByte            ; 33h - Invalid
		dd      invalidInsByte            ; 34h - Invalid
		dd      inst35		          ; 35h - AND $xx, X
		dd      inst36		          ; 36h - ROL $xx, X
		dd      invalidInsByte            ; 37h - Invalid
		dd      inst38                    ; 38h - SEC
		dd      inst39		          ; 39h - AND $xxxx, Y
		dd      invalidInsByte            ; 3ah - Invalid
		dd      invalidInsByte            ; 3bh - Invalid
		dd      invalidInsByte            ; 3ch - Invalid
		dd      inst3d                    ; 3dh - AND $xxxx, X
		dd      inst3e		          ; 3eh - ROL $xxxx, X
		dd      invalidInsByte            ; 3fh - Invalid
		dd      inst40                    ; 40h - RTI
		dd      inst41		          ; 41h - EOR ($xx), X
		dd      invalidInsByte            ; 42h - Invalid
		dd      invalidInsByte            ; 43h - Invalid
		dd      invalidInsByte            ; 44h - Invalid
		dd      inst45                    ; 45h - EOR $xx
		dd      inst46                    ; 46h - LSR $xx
		dd      invalidInsByte            ; 47h - Invalid
		dd      inst48                    ; 48h - PHA
		dd      inst49                    ; 49h - EOR #$xx
		dd      inst4a                    ; 4ah - LSR A
		dd      invalidInsByte            ; 4bh - Invalid
		dd      inst4c                    ; 4ch - JMP $xxxx
		dd      inst4d		          ; 4dh - EOR $xxxx
		dd      inst4e                    ; 4eh - LSR $xxxx
		dd      invalidInsByte            ; 4fh - Invalid
		dd      inst50                    ; 50h - BVC $xxxx
		dd      inst51		          ; 51h - EOR ($xx), Y
		dd      invalidInsByte            ; 52h - Invalid
		dd      invalidInsByte            ; 53h - Invalid
		dd      invalidInsByte            ; 54h - Invalid
		dd      inst55		          ; 55h - EOR $xx, X
		dd      inst56		          ; 56h - LSR $xx, X
		dd      invalidInsByte            ; 57h - Invalid
		dd      inst58		          ; 58h - CLI
		dd      inst59		          ; 59h - AND $xxxx, Y
		dd      invalidInsByte            ; 5ah - Invalid
		dd      invalidInsByte            ; 5bh - Invalid
		dd      invalidInsByte            ; 5ch - Invalid
		dd      inst5d                    ; 5dh - EOR $xxxx, X
		dd      inst5e		          ; 5eh - LSR $xxxx, X
		dd      invalidInsByte            ; 5fh - Invalid
		dd      inst60                    ; 60h - RTS
		dd      inst61		          ; 61h - ADC ($xx,X)
		dd      invalidInsByte            ; 62h - Invalid
		dd      invalidInsByte            ; 63h - Invalid
		dd      invalidInsByte            ; 64h - Invalid
		dd      inst65                    ; 65h - ADC ($02)
		dd      inst66                    ; 66h - ROR $xxh
		dd      invalidInsByte            ; 67h - Invalid
		dd      inst68                    ; 68h - PLA
		dd      inst69                    ; 69h - ADC #xxh
		dd      inst6a                    ; 6ah - ROR A
		dd      invalidInsByte            ; 6bh - Invalid
		dd      inst6c		          ; 6ch - JMP ($xxxx)
		dd      inst6d                    ; 6dh - ADC $xxxx
		dd      inst6e                    ; 6eh - ROR $xxxxh
		dd      invalidInsByte            ; 6fh - Invalid
		dd      inst70                    ; 70h - BVS $xxxx
		dd      inst71                    ; 71h - ADC ($xx),Y
		dd      invalidInsByte            ; 72h - Invalid
		dd      invalidInsByte            ; 73h - Invalid
		dd      invalidInsByte            ; 74h - Invalid
		dd      inst75                    ; 75h - ADC $xx, X
		dd      inst76		          ; 76h - ROR $xx, X
		dd      invalidInsByte            ; 77h - Invalid
		dd      inst78		          ; 78h - SEI
		dd      inst79                    ; 79h - ADC $xxxx,Y
		dd      invalidInsByte            ; 7ah - Invalid
		dd      invalidInsByte            ; 7bh - Invalid
		dd      invalidInsByte            ; 7ch - Invalid
		dd      inst7d                    ; 7dh - ADC $xxxx,X
		dd      inst7e		          ; 7eh - ROR $xxxx, X
		dd      invalidInsByte            ; 7fh - Invalid
		dd      invalidInsByte            ; 80h - Invalid
		dd      inst81		          ; 81h - STA ($xx),Y
		dd      invalidInsByte            ; 82h - Invalid
		dd      invalidInsByte            ; 83h - Invalid
		dd      inst84                    ; 84h - STY $xx
		dd      inst85                    ; 85h - STA $xx
		dd      inst86                    ; 86h - STX $xx
		dd      invalidInsByte            ; 87h - Invalid
		dd      inst88                    ; 88h - DEY
		dd      invalidInsByte            ; 89h - Invalid
		dd      inst8a                    ; 8ah - TXA
		dd      invalidInsByte            ; 8bh - Invalid
		dd      inst8c                    ; 8ch - STY $xxxx
		dd      inst8d                    ; 8dh - STA $xxxx
		dd      inst8e                    ; 8eh - STX $xxxx
		dd      invalidInsByte            ; 8fh - Invalid
		dd      inst90                    ; 90h - BCC $xxxx
		dd      inst91                    ; 91h - STA $xx, Y
		dd      invalidInsByte            ; 92h - Invalid
		dd      invalidInsByte            ; 93h - Invalid
		dd      inst94		          ; 94h - STY $xx, X
		dd      inst95                    ; 95h - STA $00, X
		dd      inst96 		          ; 96h - STX $xx, Y
		dd      invalidInsByte            ; 97h - Invalid
		dd      inst98                    ; 98h - TYA
		dd      inst99                    ; 99h - STA $xxxx, Y
		dd      inst9a                    ; 9ah - TXS
		dd      invalidInsByte            ; 9bh - Invalid
		dd      invalidInsByte            ; 9ch - Invalid
		dd      inst9d                    ; 9dh - STA $xxxx, X
		dd      invalidInsByte            ; 9eh - Invalid
		dd      invalidInsByte            ; 9fh - Invalid
		dd      insta0                    ; a0h - LDY, ##h
		dd      insta1                    ; a1h - LDA ($xx), X
		dd      insta2                    ; a2h - LDX, ##h
		dd      invalidInsByte            ; a3h - Invalid
		dd      insta4                    ; a4h - LDY $xx
		dd      insta5                    ; a5h - LDA $xx
		dd      insta6                    ; a6h - LDX $xx
		dd      invalidInsByte            ; a7h - Invalid
		dd      insta8                    ; a8h - TAY
		dd      insta9                    ; a9h - LDA #xxh
		dd      instaa                    ; aah - TAX
		dd      invalidInsByte            ; abh - Invalid
		dd      instac                    ; ach - LDY $xxxx
		dd      instad                    ; adh - LDA $xxxx
		dd      instae                    ; aeh - LDX $xxxx
		dd      invalidInsByte            ; afh - Invalid
		dd      instb0                    ; b0h - BCS $xxxx
		dd      instb1                    ; b1h - LDA ($xx), Y
		dd      invalidInsByte            ; b2h - Invalid
		dd      invalidInsByte            ; b3h - Invalid
		dd      instb4                    ; b4h - LDY $xx,X
		dd      instb5                    ; b5h - LDA $xx,X
		dd      instb6		          ; b6h - LDX $xx,Y
		dd      invalidInsByte            ; b7h - Invalid
		dd      instb8		          ; b8h - CLV
		dd      instb9                    ; b9h - LDA $xxxxh, Y
		dd      instba		          ; bah - TSX
		dd      invalidInsByte            ; bbh - Invalid
		dd      instbc		          ; bch - LDY $xxxxh, X
		dd      instbd                    ; bdh - LDA $xxxxh, X
		dd      instbe                    ; beh - LDX $xxxxh, Y
		dd      invalidInsByte            ; bfh - Invalid
		dd      instc0                    ; c0h - CPY #xxh
		dd      instc1		          ; c1h - CMP ($xx), X
		dd      invalidInsByte            ; c2h - Invalid
		dd      invalidInsByte            ; c3h - Invalid
		dd      instc4                    ; c4h - CPY $xx
		dd      instc5                    ; c5h - CMP $xx
		dd      instc6                    ; c6h - DEC $xx
		dd      invalidInsByte            ; c7h - Invalid
		dd      instc8                    ; c8h - INY
		dd      instc9                    ; c9h - CMP #xxh
		dd      instca                    ; cah - DEX
		dd      invalidInsByte            ; cbh - Invalid
		dd      instcc		          ; cch - CPY $xxxx
		dd      instcd                    ; cdh - CMP $xxxx
		dd      instce                    ; ceh - DEC $xxxx
		dd      invalidInsByte            ; cfh - Invalid
		dd      instd0                    ; d0h - BNE $xxxx
		dd      instd1		          ; d1h - CMP ($xx), Y
		dd      invalidInsByte            ; d2h - Invalid
		dd      invalidInsByte            ; d3h - Invalid
		dd      invalidInsByte            ; d4h - Invalid
		dd      instd5                    ; d5h - CMP $xx, X
		dd      instd6                    ; d6h - DEC $xx, X
		dd      invalidInsByte            ; d7h - Invalid
		dd      instd8                    ; d8h - CLD
		dd      instd9		          ; d9h - CMP $xxxx, Y
		dd      invalidInsByte            ; dah - Invalid
		dd      invalidInsByte            ; dbh - Invalid
		dd      invalidInsByte            ; dch - Invalid
		dd      instdd		          ; ddh - CMP $xxxx, X
		dd      instde                    ; deh - DEC $xxxx, X
		dd      invalidInsByte            ; dfh - Invalid
		dd      inste0                    ; e0h - CPX #$xx
		dd      inste1		          ; e1h - SBC ($xx), X
		dd      invalidInsByte            ; e2h - Invalid
		dd      invalidInsByte            ; e3h - Invalid
		dd      inste4                    ; e4h - CPX $xx
		dd      inste5                    ; e5h - SBC $xx
		dd      inste6                    ; e6h - INC $xx
		dd      invalidInsByte            ; e7h - Invalid
		dd      inste8                    ; e8h - INX
		dd      inste9                    ; e9h - SBC #$xx
		dd      instea		          ; eah - NOP
		dd      invalidInsByte            ; ebh - Invalid
		dd      instec		          ; ech - CPX $xxxx
		dd      insted                    ; edh - SBC $xxxx
		dd      instee                    ; eeh - INC $xxxx
		dd      invalidInsByte            ; efh - Invalid
		dd      instf0                    ; f0h - BEQ $xxxx
		dd      instf1		          ; f1h - SBC ($xx), Y
		dd      invalidInsByte            ; f2h - Invalid
		dd      invalidInsByte            ; f3h - Invalid
		dd      invalidInsByte            ; f4h - Invalid
		dd      instf5                    ; f5h - SBC $xx, X
		dd      instf6                    ; f6h - INC $xx, X
		dd      invalidInsByte            ; f7h - Invalid
		dd      instf8                    ; f8h - SED
		dd      instf9		          ; f9h - SBC $xxxx, Y
		dd      invalidInsByte            ; fah - Invalid
		dd      invalidInsByte            ; fbh - Invalid
		dd      invalidInsByte            ; fch - Invalid
		dd      instfd                    ; fdh - SBC $xxxx, X
		dd      instfe		          ; feh - INC $xxxx, X
		dd      invalidInsByte            ; ffh - Invalid

; This is the flag table for all 255 bytes

flagTable       db      02h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h

		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h

		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h

		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
		db      80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h

; This is the flag table for all 255 bytes - no overflow or sign

flagNoOvTable   db      02h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
		db      00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h

_DATA           ends

; Support routines go here

;************************************************************************
;*                                                                      *
;* Execution points for instructions                                    *
;*                                                                      *
;************************************************************************

ifdef WATCOM
_TEXT           segment         para public 'CODE'
endif

ifdef MASM
_TEXT           segment         para public 'CODE'
endif

ifdef TASM
_TEXT           segment         dword public 'CODE'
endif

		assume          CS:_TEXT

;************************************************************************
;*                                                                      *
;* Main routines                                                        *
;*                                                                      *
;************************************************************************

; 00 - BRK

Alignment

inst00          proc

		Clocktime       7

		sub     esi, ebp        ; Get our REAL PC
		add     si, 2           ; Add 2

		or      dh, BREAK OR INTERRUPT  ; OR In our BREAK flag & Interrupt

ifdef WATCOM
		mov     di, word ptr _m6502s
endif

ifdef MASM
		mov	di, word ptr m6502s
endif

ifdef TASM
		mov	di, word ptr m6502s
endif

		and     di, 0ffh        ; Only the lower part we care about
		add     di, 0feh        ; Stack segment is 100-1ffh
		mov     [edi + ebp], si ; Store our offset
		dec     edi             ; Decrement!
		mov     [edi + ebp], dh ; Store our flags
		dec     edi
		mov     ax, di          ; Get it for byte storage

ifdef WATCOM
		mov     byte ptr _m6502s, al ; Store it!
endif

ifdef MASM
		mov     byte ptr m6502s, al ; Store it!
endif

ifdef TASM
		mov     byte ptr m6502s, al ; Store it!
endif

		mov     di, 0fffeh      ; Place to get break address
		mov     si, [edi + ebp] ; Go get it!
		add     esi, ebp        ; Add in our offset
		
		xor     eax, eax        ; Zero EAX for the return loop
		ExecuteNextInstruction 7        ; Return to the main loop

inst00          endp

; 01 - ORA indx

Alignment

inst01          proc

		Clocktime       6

		FetchInstructionByte    ; Get our offset
		mov     al, [ebp+eax]   ; Get the value it points to
		add     al, bl          ; Add in X to it
		mov     ax, [ebp+eax]   ; Go get the address
		ReadMemoryByte          ; Go read it!
		or      dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Accumulator flag set
		ExecuteNextInstruction 6         ; Return to the main loop

inst01          endp

; 05 - ORA $xx

Alignment

inst05          proc

		Clocktime 3
		FetchInstructionByte    ; Get address to OR with
		mov     al, [ebp+eax]   ; Zero page - go get it
		or      dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 3

inst05          endp

; 06 - ASL $xx

Alignment

inst06          proc

		Clocktime 5

		and     dh, 0ffh - CARRY ; No carry!
		FetchInstructionByte    ; Get 0 page access address
		mov	di, ax		; Get our address in DI
		mov	al, [ebp+edi]	; Go get our data
		shl     al, 1           ; Left shift!
		adc	dh, 0		; Set carry (if applicable)
		mov	[ebp+edi], al	; Store our data back into zp
		FlagSet                 ; Set flags based on AL
		ExecuteNextInstruction 5

inst06          endp

; 08 - PHP

Alignment

inst08          proc

		Clocktime 3

ifdef WATCOM
		mov     al, _m6502s     ; Get our stack pointer
endif

ifdef MASM
		mov     al, m6502s     ; Get our stack pointer
endif

ifdef TASM
		mov     al, m6502s     ; Get our stack pointer
endif


		inc     ah              ; AH = 1 (stack page)
		mov     [ebp+eax], dh   ; Store flags

ifdef WATCOM
		dec     _m6502s         ; Back our stack pointer down one!
endif

ifdef MASM
		dec     m6502s         ; Back our stack pointer down one!
endif

ifdef TASM
		dec     m6502s         ; Back our stack pointer down one!
endif


		dec     ah              ; Zero AH
		ExecuteNextInstruction 3

inst08          endp

; 09 - ORA #xxh

Alignment

inst09          proc

		Clocktime 2
		FetchInstructionByte    ; Get value to and with A
		or      dl, al          ; And it in!
		FlagSetAcc              ; Set flags based on accumulator
		ExecuteNextInstruction 2

inst09          endp

; 0a - ASL A

Alignment

inst0a          proc

		Clocktime 2

		and     dh, 0ffh - CARRY ; No carry!
		shl     dl, 1           ; Left shift!
		adc	dh, 0		; Set carry (if applicable)
		FlagSetAcc              ; Set flags based on accumulator
		ExecuteNextInstruction 2

inst0a          endp

; 0d - ORA $xxxx

Alignment

inst0d          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to OR with
		ReadMemoryByte          ; Go read the data @ that address
		or      dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		xor     ah, ah          ; Don't hose up the main loop
		ExecuteNextInstruction 4

inst0d          endp

; 0e - ASL $xxxx

Alignment 

inst0e          proc

		Clocktime 6

		and     dh, 0ffh - CARRY ; No carry!
		shl     edx, 16         ; Protect DX
		shl     ebx, 16         ; Save this, too

		FetchInstructionWord    ; Get absolute address
		mov     bx, ax          ; Save address in BX
		ReadMemoryByte          ; Go get the data

		shl     al, 1           ; Left shift!
		jnc     noMain0e

		or      edx, CARRY SHL 24 ; OR In our carry flag

noMain0e:
		mov     dl, al          ; Put it here for the write
		mov     ax, bx          ; Get our address back
		WriteMemoryByte
		mov     al, dl          ; Get our byte back
		shr     edx, 16         ; Restore Flags & Accumulator
		FlagSet                 ; Set flags based on AL
		shr     ebx, 16         ; Restore EBX
		xor     ah, ah          ; Zero MSB
		ExecuteNextInstruction 6

inst0e          endp

; 10 - BPL $xxxx

Alignment

inst10          proc

		Clocktime 2
		test    dh, SIGN        ; Are we positive?
		jz      doTakeRelativeJump ; Yup! Go do it!
		inc     esi             ; Skip over offset
		ExecuteNextInstruction 2

inst10          endp

; 11 - ORA ($xx), Y

Alignment

inst11          proc

		Clocktime 5

		FetchInstructionByte    ; Go get our address
		mov	ax, [ebp+eax]	; Get zero page address
		add	ax, cx		; Add in Y
		or     	dl, byte ptr [ebp+eax]   ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 5

inst11          endp

; 15 - ORA $xx, X

Alignment

inst15          proc

		Clocktime 4
		FetchInstructionByte    ; Get address to OR with
		add     al, bl          ; Add in X, too
		mov     al, [ebp+eax]   ; Go get it!
		or      dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst15          endp

; 16 - ASL $xx, X

Alignment

inst16          proc

		Clocktime 6

		and     dh, 0ffh - CARRY ; No carry!
		FetchInstructionByte	; Go get our zero page
		add	al, bl		; Add X
		mov	di, ax		; Store offset for later
		mov	al, [ebp+edi]	; Go get the zero page byte

		shl     al, 1           ; Left shift!
		adc	dh, 0		; Add in carry (if present)

		mov	[ebp+edi], al	; Store the new byte
		FlagSet                 ; Set flags based on AL
		xor     ah, ah          ; Zero MSB
		ExecuteNextInstruction 6

inst16          endp

; 18 - CLC

Alignment

inst18          proc

		Clocktime 2
		and     dh, 0ffh - CARRY ; No more carry!
		ExecuteNextInstruction 2

inst18          endp

; 19 - OR $xxxx, Y

Alignment

inst19          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to AND with
		add     ax, cx          ; Add in Y
		ReadMemoryByte          ; Go read the data @ that address
		or      dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst19          endp

; 1d - OR $xxxx, X

Alignment

inst1d          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to AND with
		add     ax, bx          ; Add in X
		ReadMemoryByte          ; Go read the data @ that address
		or      dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst1d          endp

; 1e - ASL $xxxx, X

Alignment

inst1e          proc

		Clocktime 7
		FetchInstructionWord    ; Get absolute address
		add	ax, bx		; Add in X
		and     dh, 0ffh - CARRY ; No carry!
		shl     edx, 16         ; Protect DX
		shl     ebx, 16         ; Save this, too
		mov     bx, ax          ; Save address in BX
		ReadMemoryByte          ; Go get the data

		shl     al, 1           ; Left shift!
		jnc     noMain1e

		or      edx, CARRY SHL 24 ; OR In our carry flag

noMain1e:
		mov     dl, al          ; Put it here for the write
		mov     ax, bx          ; Get our address back
		WriteMemoryByte
		mov     al, dl          ; Get our byte back
		shr     edx, 16         ; Restore Flags & Accumulator
		FlagSet                 ; Set flags based on AL
		shr     ebx, 16         ; Restore EBX
		ExecuteNextInstruction 7

inst1e          endp

; 20 - JSR $xxxx

Alignment

inst20          proc

		Clocktime 6

		FetchInstructionWord    ; Get our "to call" address
		mov     di, ax          ; Store our "to call" address in DI
		sub     esi, ebp        ; Get our current address
		dec     si              ; Our offset to return to

ifdef WATCOM
		sub     _m6502s, 2      ; Back up 2 bytes for stack push
		mov     ax, word ptr _m6502s ; Our stack area
endif

ifdef MASM
		sub     m6502s, 2      ; Back up 2 bytes for stack push
		mov     ax, word ptr m6502s ; Our stack area
endif

ifdef TASM
		sub     m6502s, 2      ; Back up 2 bytes for stack push
		mov     ax, word ptr m6502s ; Our stack area
endif

		xor     ah, ah          ; Store it!
		mov     [eax + ebp + 101h], si ; Store our return address

		mov     esi, edi        ; Put it in ESI
		add     esi, ebp        ; Add in the base address
		ExecuteNextInstruction 6         ; Return to our main fetch loop

inst20          endp

; 21 - AND ($xx), X

Alignment

inst21          proc

		Clocktime 6

		FetchInstructionByte    ; Go get our address
		mov	ax, [ebp+eax]	; Get zero page address
		add	ax, bx		; Add in X
		and    	dl, byte ptr [ebp+eax]   ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 6

inst21          endp

; 24 - BIT $xx

Alignment

inst24          proc

		Clocktime 3

		FetchInstructionByte    ; Get address of data to do
		ReadMemoryByte          ; Go get the data

		mov     ah, al          ; A copy here, too
		and     ah, 0c0h        ; Bits 6 & 7
		and     al, dl          ; And AL with DL
		jnz     zeroFlag24
		
		or      dh, ZERO        ; Non-zero!
		jmp     short bitCont24

zeroFlag24:
		and     dh, 0ffh - ZERO ; We're Zero!

bitCont24:
		and     dh, 03fh        ; Knock out two upper bits
		or      dh, ah          ; OR In bits 6 & 7!
		xor     ah, ah          ; Zero this for the return
		ExecuteNextInstruction 3 ; Return to main loop!

inst24          endp

; 25 - AND $xx

Alignment

inst25          proc

		Clocktime 3
		FetchInstructionByte    ; Get address to OR with
		and     dl, [ebp+eax]   ; AND it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 3

inst25          endp

; 26 - ROL $xx

Alignment

inst26          proc

		Clocktime 5

		FetchInstructionByte    ; Get 0 page access address
		mov	di, ax		; Store our address
		mov	al, [ebp+edi]	; Get the data!

		test    dh, CARRY	; Is carry set?
		jz      noHighSet
		inc     ah              ; We do shift in a 1
noHighSet:
		and	dh, 0ffh - CARRY ; Get rid of carry
		shl     al, 1
		adc	dh, 0	 	; Add in carry (if applicable)
		or      al, ah          ; OR In our rotated bit
		mov	[ebp+edi], al	; Store our ZP byte again
		FlagSet                 ; Set flags based on AL
		ExecuteNextInstruction 5

inst26          endp

; 28 - PLP

Alignment

inst28          proc

		Clocktime 4

ifdef WATCOM
		inc     _m6502s         ; Increment stack
		mov     al, _m6502s     ; Get our stack address
endif

ifdef MASM
		inc     m6502s         ; Increment stack
		mov     al, m6502s     ; Get our stack address
endif

ifdef TASM
		inc     m6502s         ; Increment stack
		mov     al, m6502s     ; Get our stack address
endif

		inc     ah              ; Make it in the stack arena
		mov     dh, [ebp + eax] ; Get our flag register
		dec     ah              ; Zero AH again
		ExecuteNextInstruction 4

inst28          endp

; 29 - AND #xxh

Alignment

inst29          proc

		Clocktime 2
		FetchInstructionByte    ; Get value to and with A
		and     dl, al          ; And it in!
		FlagSetAcc              ; Set flags based on accumulator
		ExecuteNextInstruction 2

inst29          endp

; 2a - ROL A

Alignment

inst2a          proc
	
		Clocktime 2

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet           ; Guess not!
		stc                     ; Set carry for the rotate in
noSet:
		rcl     dl, 1           ; Rotate DL!
		jc      setCarryRola

		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSetAcc
		ExecuteNextInstruction 2

setCarryRola:
		or      dh, CARRY       ; Set carry
		FlagSetAcc
		ExecuteNextInstruction 2

inst2a          endp

; 2c - BIT $xxxx

Alignment

inst2c          proc

		Clocktime 4

		FetchInstructionWord    ; Get address of data to do
		ReadMemoryByte          ; Go get the data

		mov     ah, al          ; A copy here, too
		and     ah, 0c0h        ; Bits 6 & 7
		and     al, dl          ; And AL with DL
		jnz     zeroFlag
		
		or      dh, ZERO        ; Non-zero!
		jmp     short bitCont

zeroFlag:
		and     dh, 0ffh - ZERO ; We're Zero!

bitCont:
		and     dh, 03fh        ; Knock out two upper bits
		or      dh, ah          ; OR In bits 6 & 7!
		xor	ah, ah		; Zero this so things can run!

		ExecuteNextInstruction 4 ; Return to main loop!

inst2c          endp

; 2D - AND $xxxx

Alignment

inst2d          proc

		Clocktime 4
		FetchInstructionWord    ; Go get our absolute address
		ReadMemoryByte          ; Get value to and in AL
		and     dl, al          ; And it with accumulator
		FlagSetAcc              ; Accumulator flag setting
		ExecuteNextInstruction 4

inst2d          endp

; 2e - ROL $xxxx

Alignment

inst2e          proc
	
		Clocktime 6

		FetchInstructionWord	; Get our address
		shl	ebx, 16
		shl	edx, 16		; Save this for later, too
		mov	bx, ax		; Store our address away for later
		ReadMemoryByte		; Go read our data
		clc
		test    edx, CARRY SHL 16 ; Carry set?
		jz      noSet2e         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet2e:
		rcl     al, 1           ; Rotate AL!
		jc      setCarryRola2e

		mov	dl, al		; Byte to write in DL
		mov	ax, bx		; Address to write to in AX
		WriteMemoryByte		; Go write it!
		shr	edx, 16
		shr	ebx, 16
		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSet
		ExecuteNextInstruction 6

setCarryRola2e:
		mov	dl, al		; Byte to write in DL
		mov	ax, bx		; Address to write to in AX
		WriteMemoryByte		; Go write it!
		shr	edx, 16		; Restore AF
		shr	ebx, 16		; Restore X
		or      dh, CARRY       ; Set carry
		FlagSet
		ExecuteNextInstruction 6

inst2e          endp

; 30 - BMI $xxxx

Alignment

inst30          proc

		Clocktime 2

		test    dh, SIGN        ; Signed?
		jnz     doTakeRelativeJump

		inc     esi             ; Don't take the jump
		ExecuteNextInstruction 2

inst30          endp

; 31 - AND ($xx), Y

Alignment

inst31          proc

		Clocktime 5

		FetchInstructionByte    ; Go get our address
		mov	ax, [ebp+eax]	; Get zero page address
		add	ax, cx		; Add in Y
		and     dl, byte ptr [ebp+eax]   ; AND It!
		FlagSetAcc
		ExecuteNextInstruction 5

inst31          endp

; 35 - AND $xx, X

Alignment

inst35          proc

		Clocktime 4
		FetchInstructionByte    ; Get address to OR with
		add     al, bl          ; Add in X, too
		mov     al, [ebp+eax]   ; Go get it!
		and     dl, al          ; OR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst35          endp

; 36 - ROL $xx, X

Alignment

inst36          proc
	
		Clocktime 6

		FetchInstructionByte	; Go get our address
		add	al, bl		; Add X to it
		mov	di, ax		; Put it in DI for safekeeping
		mov	al, [ebp+eax]	; Go get the value

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet36         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet36:
		rcl     al, 1           ; Rotate AL!
		mov	[ebp+edi], al	; Store our byte back
		jc      setCarryRola36

		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSet
		ExecuteNextInstruction 6

setCarryRola36:
		mov	[ebp+edi], al	; Store our byte back
		or      dh, CARRY       ; Set carry
		FlagSet
		ExecuteNextInstruction 6

inst36          endp

; 38 - SEC

Alignment

inst38          proc

		Clocktime 2             ; Timing information
		
		or      dh, CARRY       ; Set carry!
		ExecuteNextInstruction 2 ; Return to the main loop!

inst38          endp

; 39 - AND $xxxx, Y

Alignment

inst39          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to AND with
		add     ax, cx          ; Add in Y
		ReadMemoryByte          ; Go read the data @ that address
		and      dl, al         ; AND it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst39          endp

; 3d - AND $xxxx, X

Alignment

inst3d          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to AND with
		add     ax, bx          ; Add in X
		ReadMemoryByte          ; Go read the data @ that address
		and     dl, al          ; AND it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst3d          endp

; 3e - ROL $xxxx, X

Alignment

inst3e          proc
	
		Clocktime 7

		FetchInstructionWord	; Get our address
		add	ax, bx		; Add in X
		shl	ebx, 16
		shl	edx, 16		; Save this for later, too
		mov	bx, ax		; Store our address away for later
		ReadMemoryByte		; Go read our data
		clc
		test    edx, CARRY SHL 16 ; Carry set?
		jz      noSet3e         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet3e:
		rcl     al, 1           ; Rotate AL!
		jc      setCarryRola3e

		mov	dl, al		; Byte to write in DL
		mov	ax, bx		; Address to write to in AX
		WriteMemoryByte		; Go write it!
		shr	edx, 16
		shr	ebx, 16
		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSet
		ExecuteNextInstruction 7

setCarryRola3e:
		mov	dl, al		; Byte to write in DL
		mov	ax, bx		; Address to write to in AX
		WriteMemoryByte		; Go write it!
		shr	edx, 16		; Restore AF
		shr	ebx, 16		; Restore X
		or      dh, CARRY       ; Set carry
		FlagSet
		ExecuteNextInstruction 7

inst3e          endp

; 40 - RTI

Alignment

inst40          proc

		Clocktime 6

		xor     esi, esi        ; Zero ESI for later

ifdef WATCOM
		mov     ax, word ptr _m6502s    ; Get our stack, man
		add     _m6502s, 3      ; Pop off a word & a byte
endif

ifdef MASM
		mov     ax, word ptr m6502s    ; Get our stack, man
		add     m6502s, 3      ; Pop off a word & a byte
endif

ifdef TASM
		mov     ax, word ptr m6502s    ; Get our stack, man
		add     m6502s, 3      ; Pop off a word & a byte
endif

		mov     ah, 1h          ; Stack "page"
		inc     eax             ; Go get it!
		mov     dh, [ebp + eax] ; Get our flags
		inc     eax             ; And now our address to return to
		mov     si, [ebp + eax] ; Get address to return to (+ 1)
		add     esi, ebp        ; Add in our base address
		xor     ah, ah          ; Zero AH

ifdef WATCOM
		mov	byte ptr _inNmi, ah ; Make sure we're not in an NMI anymore
endif

ifdef MASM
		mov	byte ptr inNmi, ah ; Make sure we're not in an NMI anymore
endif

ifdef TASM
		mov	byte ptr inNmi, ah ; Make sure we're not in an NMI anymore
endif

		ExecuteNextInstruction 6

inst40          endp

; 41 - EOR ($xx), X

Alignment

inst41          proc

		Clocktime 6

		FetchInstructionByte    ; Go get our address
		mov	ax, [ebp+eax]	; Get zero page address
		add	ax, bx		; Add in X
		xor     dl, byte ptr [ebp+eax]   ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 6

inst41          endp

; 45 - EOR $xx

Alignment

inst45          proc

		Clocktime 3

		FetchInstructionByte    ; Get value to XOR with
		mov     al, [ebp+eax]   ; Go get our address
		xor     dl, al          ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 3

inst45          endp

; 46 - LSR $xx

Alignment

inst46          proc

		Clocktime 5
		FetchInstructionByte    ; Go get the new address to shift
		mov	di, ax		; Our zero page address in EDI
		mov	al, [ebp+edi]	; Go get it!

		and     dh, 07fh - CARRY ; Get rid of carry & negative
		test    al, 1           ; How about carry?
		jz      noSetCarry

		or      dh, CARRY       ; Turn on carry!

noSetCarry:
		shr     al, 1           ; Shift right!
		FlagSet
		mov	[ebp+edi], al	; Store our new shifted value

		ExecuteNextInstruction 5

inst46          endp

; 48 - PHA

Alignment

inst48          proc

		Clocktime 3

ifdef WATCOM
		mov     al, _m6502s     ; Get our stack pointer
		dec     _m6502s         ; Back our stack pointer down one!
endif

ifdef MASM
		mov     al, m6502s     ; Get our stack pointer
		dec     m6502s         ; Back our stack pointer down one!
endif

ifdef TASM
		mov     al, m6502s     ; Get our stack pointer
		dec     m6502s         ; Back our stack pointer down one!
endif

		mov     ah, 01h         ; Stack "segment"
		mov     [ebp+eax], dl   ; Store accumulator
		dec     ah              ; Zero AH
		ExecuteNextInstruction 3

inst48          endp

; 49 - EOR xxh

Alignment

inst49          proc

		Clocktime 2

		FetchInstructionByte    ; Get value to XOR with
		xor     dl, al          ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 2

inst49          endp

; 4a - LSR A

Alignment

inst4a          proc

		Clocktime 2

		and     dh, 07fh - CARRY ; Get rid of carry
		shr	dl, 1		; LSR A!
		adc	dh, 0		; Set carry (if applicable)

		FlagSetNoOvAcc          ; Set the flags!
		ExecuteNextInstruction 2

inst4a          endp

; 4c - JMP $xxxx

Alignment

inst4c          proc

		Clocktime 3
		FetchInstructionWord    ; Go get new address to jump to
		mov     esi, eax        ; Move in our new address
		add     esi, ebp        ; Add in the base offset
		xor     ah, ah          ; Zero AH for later
		ExecuteNextInstruction 3         ; New address!

inst4c          endp

; 4d - EOR $xxxx

Alignment

inst4d		proc

		Clocktime 4

		FetchInstructionWord	; Get our data address
		ReadMemoryByte		; Go read it!
		xor	dl, al		; XOR It with A
		FlagSetAcc		; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst4d		endp

; 4e - LSR $xxxx

Alignment

inst4e          proc

		Clocktime 6
		FetchInstructionWord    ; Go get the new address to shift
		shl     ebx, 16         ; Save BX for later
		mov     bx, ax          ; Store address in DI
		ReadMemoryByte          ; Go read the memory byte

		and     dh, 07fh - CARRY ; Get rid of carry & negative
		test    al, 1           ; How about carry?
		jz      noSetCarry2

		or      dh, CARRY       ; Turn on carry!

noSetCarry2:
		shr     al, 1           ; Shift right!
		shl     edx, 16         ; Save DX
		mov     dl, al          ; Byte to write
		mov     ax, bx          ; Get address to write
		WriteMemoryByte
		FlagSetAcc		; Need to BEFORE the restore
		shr     edx, 16         ; Restore DX
		shr     ebx, 16         ; Move EBX back
		ExecuteNextInstruction 6

inst4e          endp

; 50 - BVC $xxxx

Alignment

inst50          proc

		Clocktime 2
		test    dh, OVERFLOW    ; Is overflow set?
		jz      doTakeRelativeJump ; Nope!! Jump!
				
		inc     esi             ; Skip relative offset
		ExecuteNextInstruction 2

inst50          endp

; 51 - EOR ($xx), Y

Alignment

inst51          proc

		Clocktime 5

		FetchInstructionByte    ; Go get our address
		mov	ax, [ebp+eax]	; Get zero page address
		add	ax, cx		; Add in Y
		xor     dl, byte ptr [ebp+eax]   ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 5

inst51          endp

; 55 - EOR $xx, X

Alignment

inst55          proc

		Clocktime 4

		FetchInstructionByte    ; Go get our address
		add     al, bl          ; Add in X, too
		xor     dl, byte ptr [ebp+eax]   ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 4

inst55          endp

; 56 - LSR $xx,X

Alignment

inst56          proc

		Clocktime 6
		FetchInstructionByte    ; Go get the new address to shift
		add	al, bl		; Add in X
		mov	di, ax		; Get our offset
		mov	al, [ebp+edi]	; Go get our data - zero page

		and     dh, 07fh - CARRY ; Get rid of carry & negative
		test    al, 1           ; How about carry?
		jz      noSetCarry56

		or      dh, CARRY       ; Turn on carry!

noSetCarry56:
		shr     al, 1           ; Shift right!
		FlagSet
		mov     dl, al          ; Byte to write
		mov	[ebp+edi], al	; Store it back!
		ExecuteNextInstruction 6

inst56          endp

; 58 - CLI

Alignment

inst58		proc

		Clocktime 2

		and	dh, 0ffh - INTERRUPT ; Allow interrupts again
		ExecuteNextInstruction 2

inst58		endp

; 59 - EOR $xxxx, Y

Alignment

inst59          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to AND with
		add     ax, cx          ; Add in Y
		ReadMemoryByte          ; Go read the data @ that address
		xor      dl, al         ; XOR it in with the accumulator
		FlagSetAcc              ; Set flags based upon accumulator
		ExecuteNextInstruction 4

inst59          endp

; 5d - EOR $xxxx, X

Alignment

inst5d          proc

		Clocktime 4

		FetchInstructionWord    ; Go get our address
		add     ax, bx          ; Add in X
		ReadMemoryByte          ; Go get the byte
		xor     dl, al          ; XOR It!
		FlagSetAcc
		ExecuteNextInstruction 4

inst5d          endp

; 5e - LSR $xxxx, X

Alignment

inst5e          proc

		Clocktime 7
		FetchInstructionWord    ; Go get the new address to shift
		add	ax, bx		; Add in X
		shl     ebx, 16         ; Save BX for later
		mov     bx, ax          ; Store address in DI
		ReadMemoryByte          ; Go read the memory byte

		and     dh, 07fh - CARRY ; Get rid of carry & negative
		test    al, 1           ; How about carry?
		jz      noSetCarry3

		or      dh, CARRY       ; Turn on carry!

noSetCarry3:
		shr     al, 1           ; Shift right!
		shl     edx, 16         ; Save DX
		mov     dl, al          ; Byte to write
		mov     ax, bx          ; Get address to write
		WriteMemoryByte
		FlagSetAcc		; Need to BEFORE the restore
		shr     edx, 16         ; Restore DX
		shr     ebx, 16         ; Move EBX back
		ExecuteNextInstruction 7

inst5e          endp

; 60 - RTS

Alignment

inst60          proc

		Clocktime 6

		xor     esi, esi        ; Zero ESI for later

ifdef WATCOM
		mov     ax, word ptr _m6502s    ; Get our stack, man
		add     _m6502s, 2      ; Pop off a word
endif

ifdef MASM
		mov     ax, word ptr m6502s    ; Get our stack, man
		add     m6502s, 2      ; Pop off a word
endif

ifdef TASM
		mov     ax, word ptr m6502s    ; Get our stack, man
		add     m6502s, 2      ; Pop off a word
endif

		mov     ah, 1h          ; Stack "page"
		inc     ax
		mov     si, [ebp + eax] ; Get address to return to (+ 1)
		inc     si              ; To the next address
		add     esi, ebp        ; Add in our base address
		xor     ah, ah          ; Zero MSB of AX
		ExecuteNextInstruction 6

inst60          endp

; 61 - ADC $(xx,X)

Alignment

inst61          proc

		Clocktime 6
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, bx          ; Add X
		ReadMemoryByte          ; Go read a memory byte
		AdcMacro
		ExecuteNextInstruction 6

inst61          endp

; 65 - ADC $xx

Alignment

inst65          proc

		Clocktime 3

		FetchInstructionByte    ; Get address
		mov     al, [ebp + eax] ; Get byte to add to accumulator
		AdcMacro                ; Add with carry!
		ExecuteNextInstruction 3

inst65          endp

; 66 - ROR $xx

Alignment

inst66          proc
	
		Clocktime 5

		FetchInstructionByte    ; Get next byte
		mov     di, ax
		xor     ah, ah          ; Zero this part for later
		mov     al, [ebp + eax] ; Get data...

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet66         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet66:
		rcr     al, 1           ; Rotate DL!
		mov     [ebp + edi], al
		jc      setCarryRola66

		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSet
		ExecuteNextInstruction 5

setCarryRola66:
		or      dh, CARRY       ; Set carry
		FlagSet
		ExecuteNextInstruction 5

inst66          endp

; 68 - PLA

Alignment

inst68          proc

		Clocktime 4

ifdef WATCOM
		inc     _m6502s         ; Increment stack
		mov     al, _m6502s     ; Get our stack address
endif

ifdef MASM
		inc     m6502s         ; Increment stack
		mov     al, m6502s     ; Get our stack address
endif

ifdef TASM
		inc     m6502s         ; Increment stack
		mov     al, m6502s     ; Get our stack address
endif


		inc     ah              ; Make it in the stack arena
		mov     dl, [ebp + eax] ; Get our A register
		FlagSetAcc              ; Go set its flag
		ExecuteNextInstruction 4

inst68          endp

; 69 - ADC #$xx

Alignment

inst69          proc

		Clocktime 2
		FetchInstructionByte
		AdcMacro                ; Do ADC!
		ExecuteNextInstruction 2

inst69          endp

; 6a - ROR A

Alignment

inst6a          proc
	
		Clocktime 2

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet6a         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet6a:
		rcr     dl, 1           ; Rotate DL!
		jc      setCarryRora

		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSetAcc
		ExecuteNextInstruction 2

setCarryRora:
		or      dh, CARRY       ; Set carry
		FlagSetAcc
		ExecuteNextInstruction 2

inst6a          endp

; 6c - JMP ($xxxx)

Alignment

inst6c		proc

		Clocktime 5
		FetchInstructionWord    ; Go get new address to jump to
		mov	ax, [ebp+eax]	; Go get our new indirect address
		mov     esi, eax        ; Move in our new address
		add     esi, ebp        ; Add in the base offset
		xor     ah, ah          ; Zero AH for later
		ExecuteNextInstruction 5 ; New address!

inst6c		endp

; 6d - ADC $xxxx

Alignment

inst6d          proc

		Clocktime 4
		FetchInstructionWord
		ReadMemoryByte          ; Go read in value to add
		AdcMacro                ; Add with carry!
		ExecuteNextInstruction 4

inst6d          endp

; 6e - ROR $xxxxh

Alignment

inst6e          proc
	
		Clocktime 6

		FetchInstructionWord    ; Get next word
		shl     ebx, 16         ; Save it!
		mov     bx, ax          ; Store our address
		ReadMemoryByte          ; Read our byte
		mov     di, bx          ; Get our address in DI
		shr     ebx, 16         ; Restore EBX

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet6e         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet6e:
		rcr     al, 1           ; Rotate DL!
		jc      setCarryRola6e

		and     dh, 0ffh - CARRY ; Get rid of carry
		shl     edx, 16         ; Save it for later
		mov     dl, al
		mov     ax, di
		WriteMemoryByte         ; Go write our new byte
		FlagSetAcc
		shr     edx, 16         ; Put it back
		ExecuteNextInstruction 6

setCarryRola6e:
		or      dh, CARRY       ; Set carry
		shl     edx, 16         ; Save it for later
		mov     dl, al
		mov     ax, di
		WriteMemoryByte         ; Go write our new byte
		FlagSetAcc
		shr     edx, 16         ; Put it back
		ExecuteNextInstruction 6

inst6e          endp

; 70 - BVS $xxxx

Alignment

inst70          proc

		Clocktime 2
		test    dh, OVERFLOW    ; Is overflow set?
		jnz     doTakeRelativeJump ; Yup! Jump!
				
		inc     esi             ; Skip relative offset
		ExecuteNextInstruction 2

inst70          endp

; 71 - ADC $(xx,Y)

Alignment

inst71          proc

		Clocktime 6
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, cx          ; Add Y
		ReadMemoryByte          ; Go read a memory byte
		AdcMacro
		ExecuteNextInstruction 6

inst71          endp

; 75 - ADC $xx, X

Alignment

inst75          proc

		Clocktime 4
		FetchInstructionByte
		add     al, bl          ; Add in X!
		mov     al, [ebp+eax]   ; Go get the data!
		AdcMacro                ; Add with carry!
		ExecuteNextInstruction 4

inst75          endp

; 76 - ROR $xx, X

Alignment

inst76          proc
	
		Clocktime 6

		FetchInstructionByte    ; Get next byte
		add	al, bl		; Add in X
		mov     di, ax
		xor     ah, ah          ; Zero this part for later
		mov     al, [ebp + eax] ; Get data...

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet76         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet76:
		rcr     al, 1           ; Rotate DL!
		mov     [ebp + edi], al
		jc      setCarryRola76

		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSet
		ExecuteNextInstruction 6

setCarryRola76:
		or      dh, CARRY       ; Set carry
		FlagSet
		ExecuteNextInstruction 6

inst76          endp

; 78 - SEI

Alignment

inst78		proc

		Clocktime 2

		or	dh, INTERRUPT	; Set interrupt disable
		ExecuteNextInstruction 2

inst78		endp

; 79 - ADC $xxxx,Y

Alignment

inst79          proc

		Clocktime 4
		FetchInstructionWord
		add     ax, cx          ; Add in Y!
		ReadMemoryByte          ; Go read in value to add
		AdcMacro                ; Add with carry!
		ExecuteNextInstruction 4

inst79          endp

; 7d - ADC $xxxx,X

Alignment

inst7d          proc

		Clocktime 4
		FetchInstructionWord
		add     ax, bx          ; Add in X!
		ReadMemoryByte          ; Go read in value to add
		AdcMacro                ; Add with carry!
		ExecuteNextInstruction 4

inst7d          endp

; 7e - ROR $xxxxh, X

Alignment

inst7e          proc
	
		Clocktime 7

		FetchInstructionWord    ; Get next word
		add	ax, bx		; Add in X
		shl     ebx, 16         ; Save it!
		mov     bx, ax          ; Store our address
		ReadMemoryByte          ; Read our byte
		mov     di, bx          ; Get our address in DI
		shr     ebx, 16         ; Restore EBX

		clc
		test    dh, CARRY       ; Carry set?
		jz      noSet7e         ; Guess not!
		stc                     ; Set carry for the rotate in
noSet7e:
		rcr     al, 1           ; Rotate DL!
		jc      setCarryRola7e

		and     dh, 0ffh - CARRY ; Get rid of carry
		FlagSet
		shl     edx, 16         ; Save it for later
		mov     dl, al
		mov     ax, di
		WriteMemoryByte         ; Go write our new byte
		shr     edx, 16         ; Put it back
		xor     ah, ah          ; Zero MSB
		ExecuteNextInstruction 7

setCarryRola7e:
		or      dh, CARRY       ; Set carry
		FlagSet
		shl     edx, 16         ; Save it for later
		mov     dl, al
		mov     ax, di
		WriteMemoryByte         ; Go write our new byte
		shr     edx, 16         ; Put it back
		xor     ah, ah          ; Zero MSB
		ExecuteNextInstruction 7

inst7e          endp

; 81 - STA $(xx,X)

Alignment

inst81          proc

		Clocktime 6
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, bx          ; Add X
		WriteMemoryByte		; Go store A away
		xor	ah, ah		; Zero this for the next instruction
		ExecuteNextInstruction 6

inst81          endp

; 84 - STY $xx

Alignment

inst84          proc
		
		Clocktime 3
		FetchInstructionByte    ; Get instruction byte
		mov	[ebp+eax], cl	; Store Y in our zero page
		ExecuteNextInstruction 3

inst84          endp

; 85 - STA $xx

Alignment

inst85          proc
		
		Clocktime 3
		FetchInstructionByte    ; Get instruction byte
		mov	[ebp+eax], dl	; Store the accumulator (zero page)
		ExecuteNextInstruction 3

inst85          endp

; 86 - STX $56

Alignment

inst86          proc

		Clocktime 3
		FetchInstructionByte    ; Get our zero page address to write
		mov	[ebp+eax], bl	; Store X in zero page
		ExecuteNextInstruction 3

inst86          endp

; 88 - DEY

Alignment

inst88          proc

		Clocktime 2

		dec     cl              ; Decrement Y
		FlagSetY
		ExecuteNextInstruction 2

inst88          endp

; 8a - TXA

Alignment

inst8a          proc

		Clocktime 2
		mov     dl, bl          ; A = X
		FlagSetAcc              ; Go set flags from accumulator

		ExecuteNextInstruction 2

inst8a          endp

; 8c - STY $xxxx

Alignment

inst8c          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to store
		xchg    cl, dl          ; Swap for the sake of write
		WriteMemoryByte         ; Go write the data
		xchg    cl, dl          ; Swap for the sake of restoration
		xor     ah, ah          ; Zero upper part
		ExecuteNextInstruction 4

inst8c          endp

; 8d - STA $xxxx

Alignment

inst8d          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to store
		WriteMemoryByte         ; Go write the data
		xor     ah, ah          ; Zero upper part
		ExecuteNextInstruction 4

inst8d          endp

; 8E - STX $xxxx

Alignment

inst8e          proc

		Clocktime 4
		FetchInstructionWord    ; Get address to store
		xchg    bl, dl          ; Swap X & A
		WriteMemoryByte         ; Go write the data
		xchg    bl, dl          ; Swap them back
		xor     ah, ah          ; Zero MSB
		ExecuteNextInstruction 4

inst8e          endp

; 90 - BCC $xxxx

Alignment

inst90          proc

		Clocktime 2
		test    dh, CARRY       ; Is carry set?
		jz      doTakeRelativeJump
				
		inc     esi             ; Skip relative offset
		ExecuteNextInstruction 2

inst90          endp

; 91 - STA $xx, Y

Alignment

inst91          proc

		Clocktime 6

		FetchInstructionByte    ; Get our offset in AX (zero page)
		mov     ax, [ebp + eax] ; Get our address
		add     ax, cx          ; Put address in AX
		WriteMemoryByte         ; Go write the data
		xor     ah, ah          ; Zero upper part of AX
		ExecuteNextInstruction 6

inst91          endp

; 94 - STY $xx, X

Alignment

inst94          proc

		Clocktime 4

		FetchInstructionByte    ; Get our offset in AX (zero page)
		add     al, bl          ; Put address in AX
		mov	[ebp+eax], cl	; Store Y
		ExecuteNextInstruction 4

inst94          endp

; 95 - STA $xx, X

Alignment

inst95          proc

		Clocktime 4

		FetchInstructionByte    ; Get our offset in AX (zero page)
		add     al, bl          ; Put address in AX
		mov	[ebp+eax], dl	; Store A
		ExecuteNextInstruction 4

inst95          endp

; 96 - STX $xx, Y

Alignment

inst96          proc

		Clocktime 4

		FetchInstructionByte    ; Get our offset in AX (zero page)
		add     al, cl          ; Put address in AX
		mov	[ebp+eax], bl	; Store X
		ExecuteNextInstruction 4

inst96          endp

; 98 - TYA

Alignment

inst98          proc

		Clocktime 2
		mov     dl, cl          ; A = Y
		FlagSetAcc              ; Go set flags from accumulator

		ExecuteNextInstruction 2

inst98          endp

; 99 - STA $xxxx, Y

Alignment

inst99          proc

		Clocktime 5

		FetchInstructionWord
		add     ax, cx          ; Add Absolute addr to Y
		WriteMemoryByte         ; Go write accumulator!
		xor     ah, ah          ; Zero upper part of AX
		ExecuteNextInstruction 5

inst99          endp

; 9a - TXS

Alignment

inst9a          proc

		Clocktime 2

ifdef WATCOM
		mov     _m6502s, bl     ; Store our new stack pointer
endif

ifdef MASM
		mov     m6502s, bl     ; Store our new stack pointer
endif

ifdef TASM
		mov     m6502s, bl     ; Store our new stack pointer
endif


		ExecuteNextInstruction 2

inst9a          endp

; 9d - STA $xxxx, X

Alignment

inst9d          proc

		Clocktime 5

		FetchInstructionWord
		add     ax, bx          ; Add Absolute addr to X
		WriteMemoryByte         ; Go write accumulator!
		xor     ah, ah          ; Zero upper part of AX
		ExecuteNextInstruction 5

inst9d          endp

; a2 - LDX #xxh

Alignment

insta0          proc

		Clocktime 2

		FetchInstructionByte    ; Go get our immediate data
		mov     cl, al          ; Put it in Y
		FlagSet                 ; Set our flags according to AL
		ExecuteNextInstruction 2

insta0          endp

; a1 - LDA ($08),X

Alignment

insta1          proc

		Clocktime 6
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, bx          ; Add X
		ReadMemoryByte          ; Go read a memory byte
		mov     dl, al          ; Go read it in!
		FlagSetAcc              ; Flags based on accumulator
		ExecuteNextInstruction 6

insta1          endp

; a2 - LDX #xxh

Alignment

insta2          proc

		Clocktime 2

		FetchInstructionByte    ; Go get our immediate data
		mov     bl, al          ; Put it in X
		FlagSet                 ; Set our flags according to AL
		ExecuteNextInstruction 2

insta2          endp

; a4 - LDY $xx

Alignment

insta4          proc

		Clocktime 3

		FetchInstructionByte    ; Get zero page address
		mov	cl, byte ptr [ebp+eax] ; Go get our new Y
		FlagSetY                ; Set flags based on the Y register
		ExecuteNextInstruction 3

insta4          endp

; a5 - LDA $xx

Alignment

insta5          proc

		Clocktime 3

		FetchInstructionByte    ; Get zero page address
		mov	dl, byte ptr [ebp+eax] ; Go get our new accumulator
		FlagSetAcc              ; Set flags based on accumulator
		ExecuteNextInstruction 3

insta5          endp

; a6 - LDX $xx

Alignment

insta6          proc

		Clocktime 3

		FetchInstructionByte    ; Go get zero page address
		mov     bl, byte ptr [ebp + eax] ; Go get our data
		FlagSetX
		ExecuteNextInstruction 3

insta6          endp


; a8 - TAY

Alignment

insta8          proc

		Clocktime 2

		mov     cl, dl          ; Y=A now
		FlagSetAcc
		ExecuteNextInstruction 2

insta8          endp

; a9 - LDA #xx

Alignment

insta9          proc

		Clocktime 2
		
		FetchInstructionByte
		mov     dl, al          ; Put it in DL!
		FlagSet                 ; Go set our flags
		ExecuteNextInstruction 2

insta9          endp            

; aa - TAX

Alignment

instaa          proc

		Clocktime 2

		mov     bl, dl          ; X = A
		FlagSetAcc
		ExecuteNextInstruction 2

instaa          endp

; ac - LDY $xxxx

Alignment

instac          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		ReadMemoryByte          ; Go get byte at address
		mov     cl, al          ; Y = gotten byte
		FlagSetY                ; Go set flags based on Y
		ExecuteNextInstruction 4

instac          endp

; ad - LDA $xxxx

Alignment

instad          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		ReadMemoryByte          ; Go get byte at address
		mov     dl, al          ; Get it in DL for accumulator load
		FlagSetAcc              ; Go set flags based on A
		ExecuteNextInstruction 4

instad          endp

; ae - LDX $xxxx

Alignment

instae          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		ReadMemoryByte          ; Go get byte at address
		mov     bl, al          ; X = gotten byte
		FlagSetX                ; Go set flags based on X
		ExecuteNextInstruction 4

instae          endp

; b0 - BCS $xxxx

Alignment

instb0          proc

		Clocktime 2
		test    dh, CARRY       ; Is carry set?
		jnz     doTakeRelativeJump
				
		inc     esi             ; Skip relative offset
		ExecuteNextInstruction 2

instb0          endp

; b1 - LDA ($xx), Y

Alignment

instb1          proc

		Clocktime 5
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, cx          ; Add Y
		ReadMemoryByte          ; Go read a memory byte
		mov     dl, al          ; Put it in the accumulator
		FlagSetAcc              ; Set the flags
		ExecuteNextInstruction 5

instb1          endp

; b4 - LDY $xx, X

Alignment

instb4          proc

		Clocktime 4

		FetchInstructionByte    ; Get the address
		add     al, bl          ; Add in the value of X
		mov     cl, [ebp + eax] ; Get our 16 bit address
		FlagSetY                ; Go set flags based on Y
		ExecuteNextInstruction 4

instb4          endp

; b5 - LDA $xx, X

Alignment

instb5          proc

		Clocktime 4

		FetchInstructionByte    ; Get the address
		add     al, bl          ; Add in the value of X
		mov     dl, [ebp + eax] ; Get our 16 bit address
		FlagSetAcc              ; Go set flags based on A
		ExecuteNextInstruction 4

instb5          endp

; b6 - LDX $xx, Y

Alignment

instb6		proc

		Clocktime 4

		FetchInstructionByte    ; Get the address
		add     al, cl          ; Add in the value of Y
		mov     bl, [ebp + eax] ; Get our 16 bit address
		FlagSetX                ; Go set flags based on X
		ExecuteNextInstruction 4

instb6		endp

; b8 - CLV

Alignment

instb8		proc

		Clocktime 2

		and	dh, 0ffh - OVERFLOW ; Get rid of overflow
		ExecuteNextInstruction 2

instb8		endp


; b9 - LDA $xxxx, Y

Alignment

instb9          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		add     ax, cx          ; Add in the value of Y
		ReadMemoryByte          ; Go get byte at address
		mov     dl, al          ; Get it in DL for accumulator load
		FlagSetAcc              ; Go set flags based on A
		ExecuteNextInstruction 4

instb9          endp

; ba - TSX

Alignment

instba		proc

		Clocktime 2

ifdef WATCOM
		mov	bl, _m6502s	; Get our S register
endif

ifdef MASM
		mov	bl, m6502s	; Get our S register
endif

ifdef TASM
		mov	bl, m6502s	; Get our S register
endif

		FlagSetX
		ExecuteNextInstruction 2

instba		endp

; bc - LDY $xxxx, X

Alignment

instbc          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		add     ax, bx          ; Add in the value of X
		ReadMemoryByte          ; Go get byte at address
		mov     cl, al          ; Get it in CL for Y
		FlagSet	                ; Go set flags based on A
		ExecuteNextInstruction 4

instbc          endp

; bd - LDA $xxxx, X

Alignment

instbd          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		add     ax, bx          ; Add in the value of X
		ReadMemoryByte          ; Go get byte at address
		mov     dl, al          ; Get it in DL for accumulator load
		FlagSetAcc              ; Go set flags based on A
		ExecuteNextInstruction 4

instbd          endp

; be - LDX $xxxx, Y

Alignment

instbe          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		add     ax, cx          ; Add in the value of Y
		ReadMemoryByte          ; Go get byte at address
		mov     bl, al          ; Get it in BL for X load
		FlagSet                 ; Go set flags based on A
		ExecuteNextInstruction 4

instbe          endp

; c0 - CPY #xxh

Alignment

instc0          proc

		Clocktime 2
		FetchInstructionByte    ; Get byte to subtract
		mov     ah, cl          ; Get a copy of the Y register
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!

		jc      noSetCarryc0

		or      dh, CARRY       ; Set carry!
noSetCarryc0:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 2

instc0          endp

; c1 - CMP ($xx), X

Alignment

instc1          proc

		Clocktime 6
		FetchInstructionByte	; Go get our zero page address
		mov	ax, [ebp+eax]	; Go get our offset
		add	ax, bx		; Add in X
		ReadMemoryByte		; Go read the data into AL
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarryc1

		or      dh, CARRY       ; Set carry!
noSetCarryc1:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 6

instc1          endp

; c4 - CPY $xx

Alignment

instc4          proc

		Clocktime 3
		FetchInstructionByte    ; Get byte to subtract
		mov     al, [ebp+eax]   ; Go get the byte!
		mov     ah, cl          ; Get a copy of the Y register
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!

		jc      noSetCarryc4

		or      dh, CARRY       ; Set carry!
noSetCarryc4:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 3

instc4          endp

; c5 - CMP $xxh

Alignment

instc5          proc

		Clocktime 3
		FetchInstructionByte    ; Get byte to subtract
		mov     al, [ebp+eax]   ; Go get our byte (in zp)
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarryc5

		or      dh, CARRY       ; Set carry!
noSetCarryc5:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 3

instc5          endp

; c6 - DEC $xx

Alignment

instc6          proc

		Clocktime 5

		FetchInstructionByte    ; Go get address to deal with
		dec	byte ptr [ebp+eax] ; Decrement the byte
		mov	al, [ebp+eax]	; Go get the data
		FlagSet                 ; Set the flags
		ExecuteNextInstruction 5

instc6          endp

; c8 - INY

Alignment

instc8          proc

		Clocktime 2
		inc     cl              ; Increment Y
		FlagSetY                ; Set flags according to Y
		ExecuteNextInstruction 2         ; Return for the next instruction

instc8          endp

; c9 - CMP #xxh

Alignment

instc9          proc

		Clocktime 2
		FetchInstructionByte    ; Get byte to subtract
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!

		jc      noSetCarryc9

		or      dh, CARRY       ; Set carry!
noSetCarryc9:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 2

instc9          endp

; ca - DEX

Alignment

instca          proc

		Clocktime 2

		dec     bl              ; Decrement X
		FlagSetX
		ExecuteNextInstruction 2

instca          endp

; cc - CPY $xxxx

Alignment

instcc          proc

		Clocktime 4
		FetchInstructionWord    ; Get word to subtract
		ReadMemoryByte
		mov     ah, cl          ; Get a copy of Y

		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarrycc

		or      dh, CARRY       ; Set carry!
noSetCarrycc:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 4

instcc          endp

; cd - CMP $xxxx

Alignment

instcd          proc

		Clocktime 4
		FetchInstructionWord    ; Get word to subtract
		ReadMemoryByte
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarrycd

		or      dh, CARRY       ; Set carry!
noSetCarrycd:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 4

instcd          endp

; ce - DEC $xxxx

Alignment

instce          proc

		Clocktime 6

		FetchInstructionWord    ; Go get address to deal with
		shl     ebx, 16         ; Save BX for later
		mov     bx, ax          ; Store our target address
		ReadMemoryByte          ; Go get the byte to increment
		dec     al              ; Decrement our byte
		shl     edx, 16         ; Save flags & A away
		mov     dl,  al         ; Put it in DL for the memory byte write
		mov     ax, bx          ; Get target address back
		WriteMemoryByte         ; Write it back
		mov     al, dl          ; Get our byte to set flags
		shr     edx, 16         ; Restore AF
		shr     ebx, 16         ; Restore BX (X)
		FlagSet                 ; Go set the flags
		ExecuteNextInstruction 6

instce          endp

; d0 - BNE $xxxx

Alignment

instd0          proc

		Clocktime 2
		test    dh, ZERO        ; Not equal?
		jz      doTakeRelativeJump

		inc     esi             ; Don't take the jump
		ExecuteNextInstruction 2

ifdef WATCOM
doTakeRelativeJump:
endif

ifdef MASM
doTakeRelativeJump::
endif

ifdef TASM
doTakeRelativeJump:
endif
		Clocktime 1		; Plus one for a taken jump
		FetchInstructionByte    ; Fetch it!
		sub     esi, ebp        ; Get our offset
		or      al, al          ; Are we negative?
		jns     noHighSetd0
		dec     ah              ; Make it 0ffh!
noHighSetd0:
		add     si, ax          ; Add in our offset
		add     esi, ebp        ; Add back in our base
		xor     ah, ah          ; Zero upper part of AX

		ExecuteNextInstruction 1

instd0          endp

; d1 - CMP ($xx), Y

Alignment

instd1          proc

		Clocktime 3
		FetchInstructionByte	; Go get our zero page address
		mov	ax, [ebp+eax]	; Go get our offset
		add	ax, cx		; Add in Y
		ReadMemoryByte		; Go read the data into AL
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarryd1

		or      dh, CARRY       ; Set carry!
noSetCarryd1:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 3

instd1          endp

; d5 - CMP $xx, X

Alignment

instd5          proc

		Clocktime 4
		FetchInstructionByte    ; Get byte to subtract
		add     al, bl          ; Add in X
		mov     al, [ebp+eax]   ; Go get our byte (in zp)
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarryd5

		or      dh, CARRY       ; Set carry!
noSetCarryd5:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 4

instd5          endp

; d6 - DEC $xx, X

Alignment

instd6          proc

		Clocktime 6
		FetchInstructionByte    ; Get offset within zero page
		add     al, bl          ; Add in the value of X
		dec     byte ptr [ebp + eax]    ; Decrement it!
		mov     al, [ebp + eax] ; Get the value
		FlagSet                 ; Go set the flags
		ExecuteNextInstruction  6 ; Return to the main loop 

instd6          endp

; d8 - CLD

Alignment

instd8          proc

		Clocktime 2

		and     dh, 0ffh - DECIMAL ; Knock out decimal flag
		ExecuteNextInstruction 2

instd8          endp

; d9 - CMP $xxxx, Y

Alignment

instd9          proc

     		Clocktime 4
		FetchInstructionWord	; Go get our address
		add	ax, cx		; Add in Y
		ReadMemoryByte		; Go read the data into AL
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarryd9

		or      dh, CARRY       ; Set carry!
noSetCarryd9:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 4

instd9          endp

; dd - CMP $xxxx, X

Alignment

instdd          proc

		Clocktime 4
		FetchInstructionWord	; Go get our address
		add	ax, bx		; Add in X
		ReadMemoryByte		; Go read the data into AL
		mov     ah, dl          ; Get a copy of the accumulator
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarrydd

		or      dh, CARRY       ; Set carry!
noSetCarrydd:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 4

instdd          endp

; de - DEC $xxxx, X

Alignment

instde          proc

		Clocktime 7
		FetchInstructionWord    ; Go get address to deal with
		add     ax, bx          ; Add in X
		shl     ebx, 16         ; Save BX for later
		mov     bx, ax          ; Store our target address
		ReadMemoryByte          ; Go get the byte to decrement
		dec     al              ; Increment our byte
		FlagSet                 ; Go set the flags
		shl     edx, 16         ; Save flags & A away
		mov     dl, al          ; Put byte to write in DL
		mov     ax, bx          ; Get target address back
		WriteMemoryByte         ; Write it back
		shr     edx, 16         ; Restore AF
		shr     ebx, 16         ; Restore BX (X)
		xor     ah, ah          ; Zero upper part of byte
		ExecuteNextInstruction 7

instde          endp

; e0 - CPX #xxh

Alignment

inste0          proc

		Clocktime 2
		FetchInstructionByte    ; Get byte to subtract
		mov     ah, bl          ; Get a copy of the X register
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!

		jc      noSetCarrye0

		or      dh, CARRY       ; Set carry!
noSetCarrye0:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		xor     ah, ah          ; Zero MSB of accumulator
		ExecuteNextInstruction 2

inste0          endp

; e1 - SBC $(xx,X)

Alignment

inste1          proc

		Clocktime 6
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, bx          ; Add X
		ReadMemoryByte          ; Go read a memory byte
		SbcMacro
		ExecuteNextInstruction 6

inste1          endp

; e4 - CPX $xx

Alignment

inste4          proc

		Clocktime 3
		FetchInstructionByte    ; Get byte to subtract
		mov     al, [ebp+eax]   ; Go get the data
		mov     ah, bl          ; Get a copy of the X register
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarrye4

		or      dh, CARRY       ; Set carry!
noSetCarrye4:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 3

inste4          endp

; e5 - SBC $xx

Alignment

inste5          proc

		Clocktime 3
		FetchInstructionByte    ; Get zero page address
		mov     al, [ebp+eax]   ; Go get the value to subtract
		SbcMacro                ; Do the subtract
		ExecuteNextInstruction 3

inste5          endp

; e6 - INC $xx

Alignment

inste6          proc

		Clocktime 5

		FetchInstructionByte    ; Go get address to deal with
		inc	byte ptr [ebp+eax]
		mov	al, [ebp+eax]
		FlagSet                 ; Set the flags
		ExecuteNextInstruction 5

inste6          endp

; e8 - INX

Alignment

inste8          proc

		Clocktime 2

		inc     bl              ; Increment X
		FlagSetX
		ExecuteNextInstruction 2

inste8          endp

; e9 - SBC #$xx

Alignment

inste9          proc

		Clocktime 2

		FetchInstructionByte    ; Get byte to subtract
		SbcMacro                ; Do SBC
		ExecuteNextInstruction 2

inste9          endp

; ea - NOP

Alignment

instea		proc

		Clocktime 2
		ExecuteNextInstruction 2	; Do nothing!

instea		endp

; ec - CPX $xxxx

Alignment

instec          proc

		Clocktime 4
		FetchInstructionWord    ; Get byte to subtract
		ReadMemoryByte		; Go read the data
		mov     ah, bl          ; Get a copy of the X register
		and     dh, 0ffh - CARRY ; No carry!
		sub     ah, al          ; Subtract!
		jc      noSetCarryec

		or      dh, CARRY       ; Set carry!
noSetCarryec:
		mov     al, ah          ; Get our value in AL for compare
		FlagSet
		ExecuteNextInstruction 4

instec          endp

; ed - SBC $xxxx

Alignment

insted          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		ReadMemoryByte          ; Go get byte at address
		SbcMacro                ; Subtract with carry
		ExecuteNextInstruction 4

insted          endp

; ee - INC $xxxx

Alignment

instee          proc

		Clocktime 6

		FetchInstructionWord    ; Go get address to deal with
		shl     ebx, 16         ; Save BX for later
		mov     bx, ax          ; Store our target address
		ReadMemoryByte          ; Go get the byte to increment
		inc     al              ; Increment our byte
		FlagSet                 ; Go set the flags
		shl     edx, 16         ; Save flags & A away
		mov     dl, al          ; Put byte to write in DL
		mov     ax, bx          ; Get target address back
		WriteMemoryByte         ; Write it back
		shr     edx, 16         ; Restore AF
		shr     ebx, 16         ; Restore BX (X)
		xor     ah, ah          ; Zero upper part of byte
		ExecuteNextInstruction 6

instee          endp

; f0 - BEQ $xxxx

Alignment

instf0          proc

		Clocktime 2
		test    dh, ZERO        ; Not equal?
		jnz     doTakeRelativeJump

		inc     esi             ; Don't take the jump
		ExecuteNextInstruction 2

instf0          endp

; f1 - SBC $(xx,Y)

Alignment

instf1          proc

		Clocktime 5
		FetchInstructionByte    ; Get our address
		mov     ax, [ebp + eax] ; Get our offset
		add     ax, cx          ; Add Y
		ReadMemoryByte          ; Go read a memory byte
		SbcMacro
		ExecuteNextInstruction 5

instf1          endp

; f5 - SBC $xx, X

Alignment

instf5          proc

		Clocktime 4

		FetchInstructionByte    ; Get our zero page address
		add     al, bl          ; Add in X
		mov     al, [ebp + eax] ; Go get it!
		SbcMacro                ; Subtract!
		ExecuteNextInstruction 4

instf5          endp

; f6 - INC $xx, X

Alignment

instf6          proc

		Clocktime 6

		FetchInstructionByte    ; Get our zero page address
		add     al, bl          ; Add in X
		inc     byte ptr [ebp + eax] ; Increment it!
		mov     al, [ebp + eax] ; Go get the incremented value
		FlagSet                 ; Go set flags
		ExecuteNextInstruction 6 ; Return to the main loop

instf6          endp

; f8 - SED

Alignment

instf8          proc

		Clocktime 2

		or      dh, DECIMAL     ; Set Decimal mode
		ExecuteNextInstruction 2

instf8          endp

; f9 - SBC $xxxx, Y

Alignment

instf9          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		add     ax, cx          ; Add in the value of Y
		ReadMemoryByte          ; Go get byte at address
		SbcMacro                ; Subtract with carry
		ExecuteNextInstruction 4

instf9          endp

; fd - SBC $xxxx, X

Alignment

instfd          proc

		Clocktime 4

		FetchInstructionWord    ; Get the address
		add     ax, bx          ; Add in the value of X
		ReadMemoryByte          ; Go get byte at address
		SbcMacro                ; Subtract with carry
		ExecuteNextInstruction 4

instfd          endp


; fe - INC $xxxx, X

Alignment

instfe          proc

		Clocktime 7

		FetchInstructionWord    ; Go get address to deal with
		add	ax, bx		; Add in X
		shl     ebx, 16         ; Save BX for later
		mov     bx, ax          ; Store our target address
		ReadMemoryByte          ; Go get the byte to increment
		inc     al              ; Increment our byte
		FlagSet                 ; Go set the flags
		shl     edx, 16         ; Save flags & A away
		mov     dl, al          ; Put byte to write in DL
		mov     ax, bx          ; Get target address back
		WriteMemoryByte         ; Write it back
		shr     edx, 16         ; Restore AF
		shr     ebx, 16         ; Restore BX (X)
		xor     ah, ah          ; Zero upper part of byte
		ExecuteNextInstruction 7

instfe          endp

; Support routines go here

;************************************************************************
;                                                                       *
; Name  : m6502reset()                                                  *
;                                                                       *
; Entry : Nothing                                                       *
;                                                                       *
; Exit  : Nothing                                                       *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine will reset the 6502's flags, registers, & interrupt modes *
;                                                                       *
;************************************************************************

Alignment

ifdef WATCOM
m6502reset_     proc
endif

ifdef MASM
m6502reset	proc
endif
	
ifdef TASM
m6502reset	proc
endif
			push    ebp

		xor     eax, eax        ; Zero AX

ifdef WATCOM
		mov     ebp, _m6502Base         ; Get the base address
		mov	_inNmi, al	; We're not in an NMI
		mov     _m6502x, al
		mov     _m6502y, al
		mov     _m6502s, 0ffh
		mov     _m6502clockticks, eax
endif

ifdef MASM
		mov     ebp, m6502Base         ; Get the base address
		mov	inNmi, al	; We're not in an NMI
		mov     m6502x, al
		mov     m6502y, al
		mov     m6502s, 0ffh
		mov     m6502clockticks, eax
endif

ifdef TASM
		mov     ebp, m6502Base         ; Get the base address
		mov	inNmi, al	; We're not in an NMI
		mov     m6502x, al
		mov     m6502y, al
		mov     m6502s, 0ffh
		mov     m6502clockticks, eax
endif

		mov     ah, 20h         ; Reset value for 6502

ifdef WATCOM
		mov     _m6502af, ax    ; Store it and the accumulator
endif

ifdef MASM
		mov     m6502af, ax    ; Store it and the accumulator
endif

ifdef TASM
		mov     m6502af, ax    ; Store it and the accumulator
endif


		mov     ax, 0fffch      ; Reset vector
		mov     ax, [ebp + eax] ; Get our reset vector address

ifdef WATCOM
		mov     _m6502pc, ax    ; Store our new PC
endif

ifdef MASM
		mov     m6502pc, ax    ; Store our new PC
endif

ifdef TASM
		mov     m6502pc, ax    ; Store our new PC
endif

		pop     ebp

		ret

ifdef WATCOM
m6502reset_     endp
endif

ifdef MASM
m6502reset	endp
endif

ifdef TASM
m6502reset	endp
endif

;************************************************************************
;                                                                       *
; Name  : m6502nmi_                                                     *
;                                                                       *
; Entry : Nothing                                                       *
;                                                                       *
; Exit  : 0=NMI was taken. NMI Is always taken                          *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine simulates a virtual NMI to the emulated 6502. No code    *
; execution actually takes place - only the PC is reset and the current *
; PC gets pushed to the stack.                                          *
;                                                                       *
;************************************************************************

Alignment

ifdef WATCOM
m6502nmi_       proc
endif

ifdef MASM
m6502nmi	proc
endif

ifdef TASM
m6502nmi	proc
endif

		xor	eax, eax		; Zero EAX

ifdef WATCOM
		cmp	byte ptr _inNmi, 0	; Are we in an nmi?
endif

ifdef MASM
		cmp	byte ptr inNmi, 0	; Are we in an nmi?
endif

ifdef TASM
		cmp	byte ptr inNmi, 0	; Are we in an nmi?
endif


		jnz	notTaken

ifdef WATCOM
		inc	_inNmi
endif

ifdef MASM
		inc	inNmi
endif

ifdef TASM
		inc	inNmi
endif

		push    edi
		push    ebx

		Clocktime 7 			; We eat up 7 clock ticks

		mov     ebx, 0100h              ; Point to stack page

ifdef WATCOM
		mov     edi, [_m6502Base]       ; Get pointer to game image
		mov     bl, [_m6502s]           ; Get our S reg pointer
		mov     ax, [_m6502pc]          ; Get our PC
endif

ifdef MASM
		mov     edi, [m6502Base]       ; Get pointer to game image
		mov     bl, [m6502s]           ; Get our S reg pointer
		mov     ax, [m6502pc]          ; Get our PC
endif

ifdef TASM
		mov     edi, [m6502Base]       ; Get pointer to game image
		mov     bl, [m6502s]           ; Get our S reg pointer
		mov     ax, [m6502pc]          ; Get our PC
endif



		mov     [edi + ebx], ah         ; Store it!
		dec     bl
		mov     [edi + ebx], al         ; Store it!
		dec     bl

ifdef WATCOM
		mov     al, byte ptr [_m6502af+1] ; Get our flags
endif

ifdef MASM
		mov     al, byte ptr [m6502af+1] ; Get our flags
endif

ifdef TASM
		mov     al, byte ptr [m6502af+1] ; Get our flags
endif


		mov     [edi + ebx], al         ; Store flags
		dec     bl
		mov     ax, [edi+0fffah]        ; Get our start vector!

ifdef WATCOM
		mov     _m6502s, bl             ; Store S reg pointer
		and     _m6502af + 1, 0efh      ; Knock out source of interrupt bit
		or      _m6502af + 1, 24h       ; Turn on something
		mov     _m6502pc, ax            ; Store our new PC
endif

ifdef MASM
		mov     m6502s, bl             ; Store S reg pointer
		and     m6502af + 1, 0efh      ; Knock out source of interrupt bit
		or      m6502af + 1, 24h       ; Turn on something
		mov     m6502pc, ax            ; Store our new PC
endif


ifdef TASM
		mov     m6502s, bl             ; Store S reg pointer
		and     m6502af + 1, 0efh      ; Knock out source of interrupt bit
		or      m6502af + 1, 24h       ; Turn on something
		mov     m6502pc, ax            ; Store our new PC
endif

		pop     ebx
		pop     edi

		mov     eax, -1			; Indicate that we took the NMI

notTaken:
		ret

ifdef WATCOM
m6502nmi_	endp
endif

ifdef MASM
m6502nmi	endp
endif

ifdef TASM
m6502nmi	endp
endif

;************************************************************************
;                                                                       *
; Name  : m6502int_                                                     *
;                                                                       *
; Entry : Nothing                                                       *
;                                                                       *
; Exit  : 0=IRQ was taken, Non-zero=already in interrupt                *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine simulates a virtual IRQ to the emulated 6502. No code    *
; execution actually takes place - only the PC is reset and the current *
; PC gets pushed to the stack.                                          *
;                                                                       *
;************************************************************************

Alignment

ifdef WATCOM
m6502int_       proc
endif

ifdef MASM
m6502int	proc
endif

ifdef TASM
m6502int	proc
endif

		xor	eax, eax		; Zero our result code
		
ifdef WATCOM
		test	byte ptr _m6502af + 1, INTERRUPT	; Are interrupts disabled?
endif

ifdef MASM
		test	byte ptr m6502af + 1, INTERRUPT	; Are interrupts disabled?
endif

ifdef TASM
		test	byte ptr m6502af + 1, INTERRUPT	; Are interrupts disabled?
endif

		jnz	notTakenNmi

		Clocktime 7 			; We eat up 7 clock ticks

		push    edi
		push    ebx
		push    ax

		mov     ebx, 0100h              ; Point to stack page

ifdef WATCOM
		mov     edi, [_m6502Base]       ; Get pointer to game image
		mov     bl, [_m6502s]           ; Get our S reg pointer
		mov     ax, [_m6502pc]          ; Get our PC
endif

ifdef MASM
		mov     edi, [m6502Base]       ; Get pointer to game image
		mov     bl, [m6502s]           ; Get our S reg pointer
		mov     ax, [m6502pc]          ; Get our PC
endif

ifdef TASM
		mov     edi, [m6502Base]       ; Get pointer to game image
		mov     bl, [m6502s]           ; Get our S reg pointer
		mov     ax, [m6502pc]          ; Get our PC
endif

		mov     [edi + ebx], ah         ; Store it!
		dec     bl
		mov     [edi + ebx], al         ; Store it!
		dec     bl

ifdef WATCOM
		mov     al, byte ptr [_m6502af+1] ; Get our flags
endif

ifdef MASM
		mov     al, byte ptr [m6502af+1] ; Get our flags
endif

ifdef TASM
		mov     al, byte ptr [m6502af+1] ; Get our flags
endif

		mov     [edi + ebx], al         ; Store flags
		dec     bl
		mov     ax, [edi+0fffeh]        ; Get our start vector!

ifdef WATCOM
		mov     _m6502s, bl             ; Store S reg pointer
		and	byte ptr _m6502af + 1, 0efh		; Knock out source of interrupt bit
		or	byte ptr _m6502af + 1, 24h		; Turn on something
		mov     _m6502pc, ax            ; Store our new PC
endif

ifdef MASM
		mov     m6502s, bl             ; Store S reg pointer
		and	byte ptr m6502af + 1, 0efh		; Knock out source of interrupt bit
		or	byte ptr m6502af + 1, 24h		; Turn on something
		mov     m6502pc, ax            ; Store our new PC
endif

ifdef TASM
		mov     m6502s, bl             ; Store S reg pointer
		and	byte ptr m6502af + 1, 0efh		; Knock out source of interrupt bit
		or	byte ptr m6502af + 1, 24h		; Turn on something
		mov     m6502pc, ax            ; Store our new PC
endif

		pop     ax                      ; Restore used registers
		pop     ebx
		pop     edi

		mov     eax, -1			; Indicate IRQ was taken

notTakenNmi:
		ret

ifdef WATCOM
m6502int_       endp
endif

ifdef MASM
m6502int	endp
endif

ifdef TASM
m6502int	endp
endif

;************************************************************************
;                                                                       *
; Name  : m6502GetContextSize                                           *
;                                                                       *
; Entry : Nothing                                                       *
;                                                                       *
; Exit  : Size of context array (in bytes)                              *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine returns the # of bytes needed for a context switch for   *
; multi-CPU operation.                                                  *
;                                                                       *
;************************************************************************

ifdef MULTI_CPU

Alignment

ifdef WATCOM
m6502GetContextSize_    proc
endif

ifdef MASM
m6502GetContextSize	proc
endif

ifdef TASM
m6502GetContextSize	proc
endif

		mov     eax, ((offset ContextEnd - offset ContextBegin) AND 0FFFFFFFCh) + 4 ; DWORD Align it!
		ret

ifdef WATCOM
m6502GetContextSize_    endp
endif

ifdef MASM
m6502GetContextSize	endp
endif

ifdef TASM
m6502GetContextSize	endp
endif

endif

;************************************************************************
;                                                                       *
; Name  : m6502SetContext                                               *
;                                                                       *
; Entry : Address of context in EAX                                     *
;                                                                       *
; Exit  : Nothing                                                       *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine will set a context. On input, a source address for the   *
; context is given.                                                     *
;                                                                       *
;************************************************************************

ifdef MULTI_CPU

Alignment

ifdef WATCOM
m6502SetContext_ proc
endif

ifdef MASM
m6502SetContext	proc	contextAddr:DWORD

		mov	eax, contextAddr
endif

ifdef TASM
m6502SetContext	proc	contextAddr:DWORD

		mov	eax, contextAddr
endif

		push    esi             ; Save registers we use
		push    edi
		push    ecx

		mov     ecx, (((offset ContextEnd - offset ContextBegin) AND 0fffffffch)) SHR 2
		mov     esi, eax        ; Source address in ESI
		mov     edi, offset ContextBegin
		rep     movsd           ; Move it as fast as we can!

		pop     ecx
		pop     edi
		pop     esi

		ret                     ; No return code                

ifdef WATCOM
m6502SetContext_ endp
endif

ifdef MASM
m6502SetContext	endp
endif

ifdef TASM
m6502SetContext	endp
endif

endif	; MULTI_CPU

;************************************************************************
;                                                                       *
; Name  : m6502GetContext                                               *
;                                                                       *
; Entry : Address of context in EAX                                     *
;                                                                       *
; Exit  : Nothing                                                       *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine will get a context. On input, a dest. address for the    *
; context is given.                                                     *
;                                                                       *
;************************************************************************

ifdef MULTI_CPU

Alignment

ifdef WATCOM
m6502GetContext_        proc
endif

ifdef MASM
m6502GetContext	proc
endif

ifdef TASM
m6502GetContext	proc
endif


		push    esi             ; Save registers we use
		push    edi
		push    ecx

		mov     edi, eax        ; Destination address in EDI

		mov     ecx, (((offset ContextEnd - offset ContextBegin) AND 0fffffffch)) SHR 2
		mov     esi, offset ContextBegin
		rep     movsd           ; Move it as fast as we can!

		pop     ecx
		pop     edi
		pop     esi

		ret                     ; No return code

ifdef WATCOM
m6502GetContext_        endp
endif

ifdef MASM
m6502GetContext		endp
endif

ifdef TASM
m6502GetContext		endp
endif

endif ; MULTI_CPU

;************************************************************************
;                                                                       *
; Name  : m6502GetInfo                                                  *
;                                                                       *
; Entry : Nothing                                                       *
;                                                                       *
; Exit  : EAX Contains bitwise information:                             *
;                                                                       *
; Bits      31-22           21  20  19  18  17  16  15-8   7-0          *
;          Unused           |   |   |   |   |   |   Major  Minor        *
;                           |   |   |   |   |   |   Rev    Rev          *
;                           |   |   |   |   |   |                       *
;                           |   |   |   |   |   +-0=UniCPU, 1=MultiCPU  *
;                           |   |   |   |   |                           *
;                           |   |   |   |   +-1=Timing info available   *
;                           |   |   |   |                               *
;                           |   |   |   +-1=SYNC emulation enabled      *
;                           |   |   |                                   *
;                           |   |   +-1=Bounds checking enabled         *
;                           |   |                                       *
;                           |   +-1=Instr. count, 0=Cycle count	        *
;                           |                                           *
;                           +-Unused - set to 0 			*
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine will get current information regarding the emulator.     *
;                                                                       *
;************************************************************************

Alignment

ifdef WATCOM
m6502GetInfo_   proc
endif

ifdef MASM
m6502GetInfo	proc
endif

ifdef TASM
m6502GetInfo	proc
endif

		xor     eax, eax        ; Zero our return code

		mov     al, MINOR_REV   ; Minor revision
		mov     ah, MAJOR_REV   ; Major revision

ifdef MULTI_CPU
		or      eax, 10000h     ; Multiple CPU support
endif

ifdef TICKS
		or      eax, 20000h     ; Timer information available
endif

ifdef SYNC_WANTED
		or      eax, 40000h     ; Sync emulation enabled
endif

ifdef BOUNDS_CHECK
		or      eax, 80000h     ; Bounds checking enabled
endif

if CYCLE_METHOD EQ INST_COUNT
		or	eax, 100000h	; We're counting instructions
endif
		ret

ifdef WATCOM
m6502GetInfo_   endp
endif

ifdef MASM
m6502GetInfo	endp
endif

ifdef TASM
m6502GetInfo	endp
endif

;************************************************************************
;                                                                       *
; Name  : m6502exec_(instructions)                                      *
;                                                                       *
; Entry : # Of 6502 instructions to execute.                            *
;                                                                       *
; Exit  : 010000h If Execution was valid                                *
;         010001h If attempted access outside of 64K area.              *
;        <010000h If invalid instruction - address of invalid instr.    *
;                                                                       *
; Description:                                                          *
;                                                                       *
; This routine will cause the virtual 6502 code to be emulated. Read the*
; header notes at the beginning of this file for programming            *
; information on setting up the 6502 for execution.                     *
;                                                                       *
;************************************************************************

Alignment

ifdef WATCOM
m6502exec_      proc
endif

ifdef MASM
m6502exec	proc	cycles:DWORD
	
		mov	eax, cycles
endif

ifdef TASM
m6502exec	proc	cycles:DWORD
	
		mov	eax, cycles
endif

		push    ebp
		push    ebx                     ; Save all registers we use
		push    ecx
		push    edx
		push    esi
		push    edi

ifdef WATCOM
		mov     _cyclesRemaining, eax    ; Store # of cycles to exec
endif

ifdef MASM
		mov	cyclesRemaining, eax	; Store our cycles remaining
endif

ifdef TASM
		mov	cyclesRemaining, eax	; Store our cycles remaining
endif

		cld                             ; Go forward!

		xor     eax, eax                ; Zero EAX 'cause we use it!
		xor     ebx, ebx                ; Zero EBX, too
		xor     ecx, ecx                ; Zero ECX
		xor     edx, edx                ; And EDX
		xor     edi, edi                ; Zero EDI as well
		xor     esi, esi                ; Zero our source address

ifdef WATCOM
		mov     bl, _m6502x             ; Get our X value
		mov     cl, _m6502y             ; And our Y value
		mov     dx, _m6502af            ; Accumulator & flags
		mov     si, _m6502pc            ; Get our program counter
		mov     ebp, _m6502Base         ; Get the base address
endif

ifdef MASM
		mov     bl, m6502x             ; Get our X value
		mov     cl, m6502y             ; And our Y value
		mov     dx, m6502af            ; Accumulator & flags
		mov     si, m6502pc            ; Get our program counter
		mov     ebp, m6502Base         ; Get the base address
endif

ifdef TASM
		mov     bl, m6502x             ; Get our X value
		mov     cl, m6502y             ; And our Y value
		mov     dx, m6502af            ; Accumulator & flags
		mov     si, m6502pc            ; Get our program counter
		mov     ebp, m6502Base         ; Get the base address
endif

		add     esi, ebp                ; Add in our base address

		FetchInstructionByte		; Get our first instruction
		jmp     dword ptr m6502regular[eax*4] ; Jump to our instruction!

; We get to invalidInsByte if it's a single byte invalid opcode

ifdef WATCOM
invalidInsByte:
endif

ifdef MASM
invalidInsByte::
endif

ifdef TASM
invalidInsByte:
endif
		dec     esi                     ; Back up one instruction...
		mov     eax, esi                ; Get our address in EAX
		sub     eax, ebp                ; And subtract our base for
						; an invalid instruction
		jmp     short emulateEnd

ifdef WATCOM
instReturn:
endif

ifdef MASM
instReturn::
endif

ifdef TASM
instReturn:
endif

		mov     eax, 010000h            ; Indicate successful exec

; Now let's tuck away the virtual registers for next time

emulateEnd:

ifdef WATCOM
		mov     _m6502x, bl             ; Store X
		mov     _m6502y, cl             ; Store Y
		mov     _m6502af, dx            ; Store A & flags
		sub     esi, _m6502Base         ; Knock off physical address
		mov     _m6502pc, si            ; And store virtual address
endif

ifdef MASM
		mov     m6502x, bl             ; Store X
		mov     m6502y, cl             ; Store Y
		mov     m6502af, dx            ; Store A & flags
		sub     esi, m6502Base         ; Knock off physical address
		mov     m6502pc, si            ; And store virtual address
endif

ifdef TASM
		mov     m6502x, bl             ; Store X
		mov     m6502y, cl             ; Store Y
		mov     m6502af, dx            ; Store A & flags
		sub     esi, m6502Base         ; Knock off physical address
		mov     m6502pc, si            ; And store virtual address
endif

		pop     edi                     ; Restore registers
		pop     esi
		pop     edx
		pop     ecx
		pop     ebx
		pop     ebp

		ret

ifdef WATCOM
OutOfRange:
endif

ifdef MASM
OutOfRange::
endif

ifdef TASM
OutOfRange:
endif
		mov     eax, 10001h             ; Indicates a boundary
						; violation (outside 64K)
		jmp     short emulateEnd

ifdef WATCOM
m6502exec_      endp
endif

ifdef MASM
m6502exec	endp
endif

ifdef TASM
m6502exec	endp
endif

_TEXT           ends
		end



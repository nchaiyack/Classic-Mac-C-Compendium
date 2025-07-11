From: Stuart Cheshire <cheshire@cs.stanford.edu>
Subject: WriteThrough INIT source code

// Disk cache bypass INIT
// Copyright (C) April 1994 Stuart Cheshire
// Should be compiled as an INIT resource
// (use the "custom header" option in Think C)

#include <Traps.h>

// I know an address should be defined as a pointer type (eg 'void *'),
// not a 'long', but Apple's Get & Set TrapAddress calls are defined to
// use longs, so it's not my fault

typedef struct { unsigned short opcode; long addr; } JmpInstruction;

typedef void ShowIconFamily(short iconId);

void main(void);

static void entry_point(void)	/* no stack frame please ! */
	{
	asm	{	
	@0		move.l	a4, -(sp)
			move.l	a0, -(sp)
			DetachResource
			lea		@0, a0
			move.l	a0, a4
			bsr		main
	exit:	move.l (sp)+, a4
		}
	}

void my_write(void);  void sys_write(void);
static void my_write(void)
	{
	asm {
		tst.w	IOParam.ioRefNum(a0)			; Is this a file write?
		bmi.s	@sys_write
		cmp.l	#1024, IOParam.ioReqCount(a0)	; Is it at leask 1K?
		blo.s	@sys_write
		ori.w	#0x20, IOParam.ioPosMode(a0)	; Set "Don't cache" bit
	extern sys_write:
		jmp		0x12345678						; Resume the system call
		}
	}

static Boolean TrapAvailable(unsigned long trap)
	{
	TrapType tType = (trap & 0x800 ? ToolTrap : OSTrap);
	if (trap & 0x800)				// if it is a ToolBox Trap
		{
		unsigned long n = 0x400;	// number of toolbox traps
		if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E,
ToolTrap))
			n = 0x200;
		if ((trap &= 0x7FF) >= n) trap = _Unimplemented;
		}
	return(NGetTrapAddress(trap, tType) != NGetTrapAddress(_Unimplemented,
ToolTrap));
	}

void main(void)
	{
	register long *p = &((JmpInstruction*)sys_write)->addr;
	*p = GetOSTrapAddress(_Write);
	SetOSTrapAddress((long)&my_write,_Write);
	if (TrapAvailable(_HWPriv)) FlushInstructionCache();
	}


Stuart Cheshire <cheshire@cs.stanford.edu>
 * <A HREF="file://brubeck.stanford.edu/www/cheshire-bio.html">WWW</A>
 * Stanford Distributed Systems Group Research Assistant
 * Escondido Village Resident Computer Coordinator
 * Macintosh Programmer

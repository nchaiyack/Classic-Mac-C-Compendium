#include <GestaltEqu.h>
#include <Traps.h>
//
// MacJmp seems to be the way to determine whether MacsBug is installed.
// On a IIx with 8MB RAM it contained the value '0x80000000' when MacsBug
// was not installed. If MacsBug is installed it seems to contain the
// address of an entry point to MacsBug.
//
#define MacJmp 0x120

void main()
{
	asm
	{
		move.l	MacJmp,D0
		and.l	#0x7FFFFFFF,D0
		beq.s	@noDebuggerPresent
		dc.w	_Debugger
		rts
	
	noDebuggerPresent:
		Move.w	#0x09,-(sp)
		dc.w	_SysBeep
//		rts		automatically included at end by Think C
	}
}

/**********************************************************************\

File:		NMRP.c

Purpose:	This module handles cleaning up after a Notification
			Manager note has been displayed.
			
Known bugs:	This code only compiles under THINK C 6.x.  I think the
			syntax for assembler is slightly different under THINK C 5,
			but I don't have my version 5 user manual handy to research
			the problem.  There is a already-compiled version of the
			NMRP resource in gsitc.<pi>.rsrc, so you don't really need
			to compile this anyway.  I'll figure it out someday and fix it.


This code was originally written by Dave Blumenthal, 12/92
Modified for use in GSITC by Mark Pilgrim, 1/94


Go Sit In The Corner -=- not you, just the cursor
Copyright ©1994, Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

extern long		ToolScratch : 0x09CE;

void FlushCodeCache(void) = 0xA0BD;

#define			_HwPriv			0xA198
#define			_CacheFlush		0xA0BD
#define			_Moof			0xA89F

pascal void main(NMRecPtr note)
{
	long			saveReturn;
	Boolean			Caches040;
	Boolean			Caches020;
	
	Caches040 = (GetOSTrapAddress(_HwPriv) != GetToolTrapAddress(_Moof));
	Caches020 = (GetOSTrapAddress(_CacheFlush) != GetToolTrapAddress(_Moof));
	
	asm
	{
		move.l		note, a0
		_NMRemove
		
		move.l		note, a0
		move.l		OFFSET(NMRec, nmStr)(a0), a0
		beq.s		@1
		_DisposePtr
		
@1:		move.l		note, a0
		_DisposePtr
		
		/* I don't like this - it uses low mem globals and assumes an     */
		/* implicit link, not to mention that it's self-modifying code.   */
		/* But, it seems to work.  Well, at least it compiles...          */
		
		/* ADDENDUM - this code assumes the following:                    */
		/*  1) The compiler has put a LINK A6 at the start of the routine */
		/*  2) The UNLK and RTS code at the end does not modify registers */
		/*     D0 or A1, or ToolScratch                                   */
		/*  3) _DisposeHandle does not modify register A1
		/*  4) ToolScratch is not modified by any of the following traps: */
		/*     _DisposeHandle, _RecoverHandle, _CacheFlush, and _HwPriv   */
		
		move.l		4(a6), saveReturn
		move.l		#ToolScratch, a0
		move.l		a0, 4(a6)
		move.l		#0x2040A023, (a0)	; movea.l	d0, a0 / _DisposeHandle
		move.w		#0x4ED1, 4(a0)		; jmp		(a1)
	}
	
	if(Caches040)
	{
		FlushDataCache();
		FlushInstructionCache();
	}
	else if(Caches020)
	{
		FlushCodeCache();
	}
	
	asm
	{
		lea			main, a0
		_RecoverHandle
		move.l		a0, d0
		move.l		saveReturn, a1
	}
}

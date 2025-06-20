/*	NAME:
		Standalone Code.h

	WRITTEN BY:
		Eric Shapiro (from July 1993 BYTE article, and the supporting source code)
				
	DESCRIPTION:
		Misc. utilities for THINK C stand-alone 'CODE' resources.

	___________________________________________________________________________
*/
#ifndef __STANDALONECODE__
#define __STANDALONECODE__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
// To be called from main() in an INIT resource that wants to lock itself in
// memory and stay around after its resource file is closed.
#define	LockSelf()			asm {													\
								lea			main, A0		; &main into A0			\
								dc.w		_RecoverHandle	; get handle to self	\
								move.l		A0, -(SP)		; save handle to self	\
								dc.w		_HLock			; lock self 			\
								dc.w		_DetachResource	; detach from rez fork	\
								}


// Saves the current value of A4 on the stack, and puts a copy of the address of main
// into A4. Should be called at start of stand-alone code resource to access globals.
#define GetGlobals()		asm { 													\
								move.l 		A4, -(SP)		; save old A4			\
								lea 		main, A4 		; get globals			\
								}


// The converse of GetGlobals(). Restore the value of A4						
#define UngetGlobals()		asm {													\
								move.l 		(SP)+, A4		; restore A4 from stack	\
								}


// Save some registers, and retrieve access to our globals via A4. Remove your
// paramaters from the stack into local registers before PatchGetGlobals()
#define PatchGetGlobals()	asm { 													\
								movem.l 	A0-A5/D0-D7, -(SP)	; save registers	\
								lea 		main, A4 			; get globals		\
								}


// The converse of PatchGetGlobals. If your return value is held in a global,
// make sure you copy it into a local variable before calling this macro - when
// A4 has been restored, your globals are now longer accessible.
#define PatchUngetGlobals()	asm {													\
								movem.l 	(SP)+, A0-A5/D0-D7	; restore registers	\
								}


#endif

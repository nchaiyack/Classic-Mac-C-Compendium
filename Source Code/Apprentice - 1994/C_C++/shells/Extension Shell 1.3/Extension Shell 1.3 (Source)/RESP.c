/*	NAME:
		RESP.c

	WRITTEN BY:
		Joe Holt (from the Usenet Macintosh Programmer's Guide).
	
	MODIFIED BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a 'RESP' code resource. It's job is to unload a
		Notification Manager request from memory, and then dispose of itself.

	NOTES:
		�	Compiled with THINK C 6.0.

		�	This code resource is generic and can be pasted into any INIT that
			uses the NotifyMsg.c code.

	___________________________________________________________________________

	VERSION HISTORY:
		(June 1990, jhh)
			�	Response to Usenet news posting. Archived in Usenet Macintosh
				Programmer's Guide.

		(Oct 1993, dg)
			�	Added call to _FlushInstructionCache and __FlushDataCache.
				Otherwise, the code fell over on 040s.

		(Jan 1994, dg)
			�	Changed type and ID of Resp code for use with Extension Shell.
			
			
	___________________________________________________________________________
*/
//=============================================================================
//		Function prototypes																	 
//-----------------------------------------------------------------------------
pascal void	main(QElemPtr nmReqPtr);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
extern long	ToolScratch : 0x09CE;		// ToolScratch is an 8 byte low-memory
										// global used as a temporary holding
										// place.










//=============================================================================
//		main : Entry point to our 'RESP' code resource.																 
//-----------------------------------------------------------------------------
//		Note :	This code is called when the user closes the Notification
//				Manager's dialog. The NM passes to us the address of the NM
//				record which was set up by NotifyMsg.c. We dispose of the
//				record, and then ourselves.
//
//				This is written in assembly for size. If you have problems
//				with assembly, I s'pose it could be written in C, but the trick
//				at the end would be hard to duplicate...
//-----------------------------------------------------------------------------
pascal void main(QElemPtr nmReqPtr)
{
	asm {
			// First remove the note from the Notification Manager's
			// Notification queue.
			move.l		nmReqPtr, A0
			_NMRemove


			// Grab the string's address from the NM record and dispose
			// of it. Then get rid of the NM record itself.
			// Notification queue.
			move.l		nmReqPtr, A0
			move.l		OFFSET(NMRec,nmStr)(A0), A0
			_DisposPtr
			move.l		nmReqPtr, A0
			_DisposPtr


			// Now the tricky part. This code lives within a small block
			// in the System Heap which we want to get rid of. But it's
			// not safe to dispose of the very block you're calling from.
			// So, we create a little bit of code in ToolScratch which
			// does the dispose for us, and execute it last.
			//
			//    dg	- Added call to _FlushInstructionCache for 040s.
			//			- Added call to _FlushDataCache for 040s.
			move.l		4(A7), A1
			move.l		#ToolScratch, A0
			move.l		A0, 4(A7)
			move.l		#0x2040A01F, (A0)				; movea.l D0, A0 / _DisposPtr
			move.w		#0x4ED1, 4(A0)					; jmp (A1)
			jsr			FlushDataCache(A4)				; flush the cache for 040s
			jsr			FlushInstructionCache(A4)		; flush the cache for 040s
			lea			main, A0
			move.l		A0, D0							; Pascal clobbers A0 on exit
	}
}

/*	NAME:
		MyFilter.c

	WRITTEN BY:
		James Thomson
		
	MODIFIED BY:
		Dair Grant.
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a jGNE filter.

	NOTES:
		�	It relies on some inline assembly to provide a wrapper for some
			higher level code.

		�	Causes the character for each keyDown event to be displayed next to
			the Apple menu.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, jt)
			�	First version.

		(Feb 1994, dg)
			�	Converted to Extension Shell.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include "StandaloneCode.h"
#include "AddrsTable.h"
#include "ESConstants.h"
#include "CodeConstants.h"





//=============================================================================
//		Global Variables																 
//-----------------------------------------------------------------------------
void		*gOldJGNE;						// The address of the next filter
Boolean		gAlreadyRan=false;				// Have we already been called?
GrafPort	gMyPortRec;						// A port to draw in
GrafPtr		gMyPort;						// Pointer to gMyPortRect





//=============================================================================
//		Private function prototypes							 
//-----------------------------------------------------------------------------
void	myJGNE(void);
void	main(void);










//=============================================================================
//		main : Entry point to our code resource.																 
//-----------------------------------------------------------------------------
//		Note :	If this is the first time we've been called we call the
//				address table to get the address of the next thing on the
//				chain.
//
//				We rely on some assembly stubs to call our C filter routine.
//-----------------------------------------------------------------------------
void main(void)
{	AddressTable	*theAddressTable;
	GrafPtr			savePort;




	// Save some registers and get the globals. This would normally be
	// done by PatchGetGlobals() - however, we need to leave d7 alone.
	asm	{
		movem.l	a0-a5/d0-d6, -(SP)				;Save some registers
		lea		main, a4						;Get the globals
	}
	

	
	// If this is the first time this code has been called, we
	// do our first-time-through processing.
	if (!gAlreadyRan)
		{
		// Get the address of the next thing on the chain
		Gestalt(kWhatKeyAddressTable, &theAddressTable);		
		gOldJGNE = (ProcPtr) ((long) theAddressTable->theTable[kWhatKey]);
		
		
		// Save the current port, then open ours to intialise it
		GetPort(&savePort);
		gMyPort = &gMyPortRec;
		OpenPort(gMyPort);
		SetPort(&savePort);

		
		// We don't need to execute this code again
		gAlreadyRan = true;
		}
	
	
	
	// Use James' assembly glue to call our routine and set
	// things up so that we exit into the next filter.
	asm {
		jsr		myJGNE							;Call our routine
		move.w	d0, 8(a6)						;Modify the result on the stack
		move.l	gOldJGNE, d7						;Copy the old filter into d7
		movem.l	(SP)+, a0-a5/d0-d6				;Restore the registers & trash globals
		move.l	d7,-(sp)						;Put address of original GNE filter on stack
	}
}










//=============================================================================
//		myJGNE : Our jGNE Filter.																 
//-----------------------------------------------------------------------------
//		Note :	If the event is a keyDown event, we draw the right character
//				up beside the Apple menu.
//-----------------------------------------------------------------------------
void myJGNE()
{	EventRecord		*theEvent;
	long			theResult;
	Rect			theRect;
	GrafPtr			savePort;
	char			key;




	
	// Load the registers into our local variables
	asm {
		move.l	a1, theEvent
		move.l	d0, theResult
	}
	
	
	
	// Examine the event - we only keyDown and autoKey events. If we
	// wanted to catch other events, this is where we would do it.
	// If we wanted to swallow events, we would set theEvent->what to
	// a nullEvent.
	switch (theEvent->what) {
		case keyDown:
		case autoKey:
			// Set things up for drawing
			GetPort(&savePort);
			SetPort(gMyPort);
			TextFont(geneva);
			TextSize(9);
			
			
			// Wipe out anything that's there already
			SetRect(&theRect, 5, 5, 15, 15);
			EraseRect(&theRect);
			
			
			// Draw the character
			MoveTo(5, 12);
			DrawChar(theEvent->message & charCodeMask);
			
			
			// Restore things
			SetPort(savePort);
			break;
			
		default:
			break;
	}



	// Move the result into the right register
	asm {
		move.l theResult, d0
	}	
}

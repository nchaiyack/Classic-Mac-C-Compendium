/*	NAME:
		QuadraGestalt.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a Gestalt
		Selector.

	NOTES:
		�	Causes your Mac to think its a Quadra. :-)

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			�	First publicly distributed version.


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
pascal OSErr	(*gOldGestaltProc) (OSType gestaltSelector, long *gestaltResponse);
Boolean			gAlreadyRan=false;





//=============================================================================
//		Private function prototypes							 
//-----------------------------------------------------------------------------
pascal OSErr main(OSType gestaltSelector, long *gestaltResponse);










//=============================================================================
//		main : Entry point to our code resource.																 
//-----------------------------------------------------------------------------
//		Note :	We call the original Gestalt selector first then fudge the
//				result to be a Quadra. Actually, we don't need to call the
//				original routine, because the information returned isn't
//				used to store other things apart from the machine type. If
//				the result was some sort of bit structure, we would have
//				to get the original result and mask on/off whatever was
//				needed.
//-----------------------------------------------------------------------------
pascal OSErr main(OSType gestaltSelector, long *gestaltResponse)
{	AddressTable	*theAddressTable;
	long			theVal;
	OSErr			theErr;




	// Get access to our globals, and save off A4
	PatchGetGlobals();

	
	
	// If we've not already been called, call the Gestalt selector to get
	// the address of the old routine. Cast the table entry to a long
	// first so we get what's pointed to by it, rather than a pointer to it.
	if (!gAlreadyRan)
		{
		Gestalt(kMacTypeAddressTable, &theAddressTable);		
		gOldGestaltProc	= (ProcPtr) ((long) theAddressTable->theTable[kMacType]);
		gAlreadyRan		= true;
		}
	
	
	
	// Call the original routine
	theErr = (*gOldGestaltProc)(gestaltSelector, &theVal);
	
	

	// Fudge the response to what we want
	*gestaltResponse = gestaltQuadra950;
	theErr = noErr;
	
	
	
	// Restore A4 for our caller and return
	PatchUngetGlobals();
	return(theErr);
}

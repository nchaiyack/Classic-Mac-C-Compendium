/*	NAME:
		AddrsTable.c

	WRITTEN BY:
		Dair Grant

	DESCRIPTION:
		This file contains a a code resource that acts as a Gestalt Selector
		function to be installed as an address table. When called, it will
		return the address of its internal address table. This is the most
		flexible mechanism for code installed from an Extension to find out
		the address of the thing it replaced.

	NOTES:
		¥	Compiled with THINK C 6.0.

		¥	This code resource is generic and can be pasted into any INIT that
			uses ExtensionShell.
		
		¥	This code relies on a fixed size table - if the table changes
			(see ESConstants.h.h), the code resource will need recompiled.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			¥	First publicly distributed version.
		
			
	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include "StandaloneCode.h"
#include "AddrsTable.h"





//=============================================================================
//		Function prototypes																	 
//-----------------------------------------------------------------------------
pascal OSErr main(OSType theSelector, long *theResponse);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
AddressTable	gTheAddressTable;










//=============================================================================
//		main : Entry point to our Gestalt Selector function.																 
//-----------------------------------------------------------------------------
//		Note :	This code is used to store a table of ProcPtrs. This is set up
//				by Extension Shell so that code installed into the System Heap
//				can find out the address of the thing that replaced them.
//
//				We do not initialise gTheAddressTable. We assume that the first
//				person to call us is ExtensionShell, and it initialises it.
//-----------------------------------------------------------------------------
pascal OSErr main(OSType theSelector, long *theResponse)
{




	// Set up A4 to get access to our globals
	GetGlobals();

	
			
	// Return the address of gTheAddressTable. We treat theResponse as a pointer
	// to a variable that's a pointer to an address table, and write to it
	// accordingly.
	*theResponse = (long) &gTheAddressTable;



	// Restore A4 for our caller.
	UngetGlobals();
}

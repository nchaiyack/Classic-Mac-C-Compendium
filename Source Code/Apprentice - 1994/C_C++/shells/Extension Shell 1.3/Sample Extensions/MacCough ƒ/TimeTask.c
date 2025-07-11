/*	NAME:
		TimeTask.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a Time Manager task.

	NOTES:
		�	We retrieve the handle to sound to play from the address table, and play it.

	___________________________________________________________________________

	VERSION HISTORY:
		(Mar 1994, dg)
			�	First publicly distributed version.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include <Sound.h>
#include "StandaloneCode.h"
#include "MCAddrsTable.h"
#include "ESConstants.h"
#include "CodeConstants.h"





//=============================================================================
//		Global Variables																 
//-----------------------------------------------------------------------------
TMTask		*gOurTask;
NMRec		gTheNMRec;
Boolean		gAlreadyRan=false;





//=============================================================================
//		Private function prototypes							 
//-----------------------------------------------------------------------------
pascal	void main(void);










//=============================================================================
//		main : Entry point to our code resource.																 
//-----------------------------------------------------------------------------
//		Note :	We get hold of the handle to the sound from our address table,
//				and play it.
//-----------------------------------------------------------------------------
pascal void main(void)
{	MCAddressTable	*theAddressTable;
	OSErr			theErr;




	// Get access to our globals, and save off A4
	PatchGetGlobals();

	
	
	// If we've not already been called, call the Gestalt selector to get
	// the address of our task record, and a handle to the sound to play.
	if (!gAlreadyRan)
		{
		// Call gestalt and get the address of our task
		Gestalt(kMacCoughAddressTable, &theAddressTable);		
		gOurTask		= (ProcPtr) ((long) theAddressTable->theTable[kTimeTask]);
		
		
		// Make up a Notification Manager request
		gTheNMRec.qType		= nmType;						// NM request type
		gTheNMRec.nmMark	= 0;							// No mark in the menu
		gTheNMRec.nmIcon	= nil;							// No icon in the menu
		gTheNMRec.nmSound	= theAddressTable->theSound;	// Play this sound
		gTheNMRec.nmStr		= nil;							// Don't show a dialog
		gTheNMRec.nmResp	= (ProcPtr) -1;					// Remove the note afterwards
		
		
		// This code doesn't need to be executed again
		gAlreadyRan = true;
		}
	

	
	// Install the Notification Manager request (which plays
	// the sound) and requeue ourselves for 10 seconds from now.
	theErr = NMInstall(&gTheNMRec);
	PrimeTime(gOurTask, 10000);



	// Restore A4 for our caller
	PatchUngetGlobals();
}

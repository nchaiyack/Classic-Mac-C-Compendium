/*	NAME:
		PlaySound.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a code block in
		the System Heap.

	NOTES:
		�	We retrieve the handle to sound to play from the address table, and play it.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			�	First publicly distributed version.

		(Mar 1994, dg)
			�	Extended address table, uses code block to play sound.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include <Sound.h>
#include "StandaloneCode.h"
#include "BTAddrsTable.h"
#include "ESConstants.h"
#include "CodeConstants.h"





//=============================================================================
//		Global Variables																 
//-----------------------------------------------------------------------------
Handle		gTheSoundToPlay;
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
{	BTAddressTable	*theAddressTable;
	OSErr			theErr;




	// Get access to our globals, and save off A4
	PatchGetGlobals();

	
	
	// If we've not already been called, call the Gestalt selector to get
	// the handle to the sound to play.
	if (!gAlreadyRan)
		{
		Gestalt(kBellTestAddressTable, &theAddressTable);		
		gTheSoundToPlay	= theAddressTable->theSound;
		gAlreadyRan		= true;
		}
	

	
	// Play the sound
	theErr = SndPlay(nil, gTheSoundToPlay, true);



	// Restore A4 for our caller
	PatchUngetGlobals();
}

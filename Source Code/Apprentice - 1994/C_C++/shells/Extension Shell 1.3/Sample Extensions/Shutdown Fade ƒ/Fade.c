/*	NAME:
		Fade.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be installed as a Shutdown task.

	NOTES:
		¥	Causes all monitors to fade to black when the Mac shuts down.

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
#include "ESConstants.h"
#include "CodeConstants.h"
#include "Gamma.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
pascal	void main(void);










//=============================================================================
//		main : Entry point to our code resource.																 
//-----------------------------------------------------------------------------
//		Note :	We fade all the monitors down to black.
//-----------------------------------------------------------------------------
pascal void main(void)
{	int					i;




	// Get access to our globals, and save off A4
	PatchGetGlobals();

	

	// Initialise the Gamma Tools library, and do nothing if we can't use it
	if (SetupGammaTools() == noErr)
		{
		if (IsGammaAvailable())
			{
			// If it all went OK, fade down slowly
			for (i = 100; i >= 0; i -= 3)
				DoGammaFade(i); 
	
	
			// And tear down the library
			DisposeGammaTools();
			}
		}
		
	
	
	// Restore A4 for our caller
	PatchUngetGlobals();
}

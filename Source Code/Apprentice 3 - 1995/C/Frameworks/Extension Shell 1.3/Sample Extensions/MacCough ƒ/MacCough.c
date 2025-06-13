/*	NAME:
		MacCough.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be used as a handler by
		Extension Shell.

	___________________________________________________________________________

	VERSION HISTORY:
		(Mar 1994, dg)
			¥	First publicly distributed version.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include <Traps.h>
#include "ParamBlock.h"
#include "StandaloneCode.h"
#include "ESConstants.h"
#include "CodeConstants.h"
#include "MacCough.h"
#include "MCAddrsTable.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
void	main(short theMsg, ESParamBlock *theParamBlock);
void	InitialiseParamBlock(void);
void	InitialiseAddrsTable(void);
void	HandleTheError(void);
void	SetUpIcons(int animDelay, int numIcons, int firstIcon);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
ESParamBlock	*gTheParamBlock;










//=============================================================================
//		main : Entry point to our code resource.																 
//-----------------------------------------------------------------------------
//		Note :	Extension Shell communicates with us via a message constant,
//				and a pointer to a structure it owns. Our job is to fill in
//				the details, depending on what it wants us to do. It takes
//				care of the rest.
//-----------------------------------------------------------------------------
void main(short theMsg, ESParamBlock *theParamBlock)
{




	// Set up A4 so that we can access our globals, and initialise them.
	GetGlobals();
	gTheParamBlock = theParamBlock;



	// Case out on what we have to do
	switch(theMsg) {
		case kInitialiseParamBlock:
		     InitialiseParamBlock();
		     break;
		     
		case kInitialiseAddrsTable:
		     InitialiseAddrsTable();
		     break;

		case kHandleError:
		     HandleTheError();
		     break;
	
		default:
			 ;
	}



	// Restore A4.
	UngetGlobals();
}










//=============================================================================
//		InitialiseParamBlock : Initialises the ParamBlock.																 
//-----------------------------------------------------------------------------
//		Note :	We have three things we want to do:
//					¥ Check to see if we can still run
//					¥ Set up the icons we want to display
//					¥ Set up the code we want installed
//-----------------------------------------------------------------------------
void InitialiseParamBlock(void)
{	int		i;




	// Check for System 7. We depend on having System 7, and won't
	// run if we don't have it. We beep, post an error message,
	// and show our disabled icon(s).
	if (gTheParamBlock->systemVersion < 0x0700)
		{
		// Error details
		gTheParamBlock->beepNow				= true;
		gTheParamBlock->postError			= true;
		gTheParamBlock->errorStringsID		= kErrorStrings;
		gTheParamBlock->errorStringIndex	= kNeedSystemSeven;


		// Icon details
		SetUpIcons(kDisabledAnimDelay, kMyNumDisabledIcons, kMyFirstDisabledIcon);
		}
	
	
	
	// If a shift key, or the mouse button, is down, we don't load either.
	// We don't post an error, but we do show our disabled icon(s).
	else if ((*gTheParamBlock->UserForcedDisable)(kShiftKey, true))
		{
		// Icon details
		SetUpIcons(kDisabledAnimDelay, kMyNumDisabledIcons, kMyFirstDisabledIcon);
		}
	
	
	
	// Otherwise, we're allowed to run so we show our icon(s) as normal,
	// and fill in the details for the code we want installed.
	else
		{
		// Icon details
		SetUpIcons(kEnabledAnimDelay, kMyNumEnabledIcons, kMyFirstEnabledIcon);
		
		
		// We install one time manager task, and request an address table
		gTheParamBlock->installAddressTable		= true;
		gTheParamBlock->addressTableSelector	= kMacCoughAddressTable;
		gTheParamBlock->numCodeResources		= 1;


		// Details for a Time Manager Task. We prime it to go off in 60 seconds,
		// remembering that the Time Manager is fully functioning when we're
		// called. 60 seconds should be long enough for the Finder to start up.
		gTheParamBlock->theCodeResources[kTimeTask].resType		= kTimeTaskResType;
		gTheParamBlock->theCodeResources[kTimeTask].resID		= kTimeTaskResID;
		gTheParamBlock->theCodeResources[kTimeTask].codeType	= kTimeManagerTaskType;
		gTheParamBlock->theCodeResources[kTimeTask].theCodeThing.theTimeManagerTask.theDelay = 60000;
		}
}










//=============================================================================
//		InitialiseAddrsTable : Initialise the address table.													 
//-----------------------------------------------------------------------------
//		Note :	If we are being used in a Control Panel, we will probably have
//				implemented the address table code with a custom code resource
//				that returns a structure with an address table embedded at the
//				start. This function's job is to correctly initialise the
//				extended fields of that structure. If we're not using a
//				(custom) address table then we don't do anything.
//
//				The message for this routine will only arrive if we've
//				requested an address table.
//-----------------------------------------------------------------------------
void InitialiseAddrsTable(void)
{	MCAddressTable	*theAddressTable;
	Handle			theHnd;

	


	// Our custom address table holds one ProcPtrs, and a Handle. The ProcPtr
	// is used by Extension Shell for Time Manager task, and the last field
	// is a handle to the noise we will play. First we call Gestalt to get the
	// address of the table.
	Gestalt(kMacCoughAddressTable, &theAddressTable);

	
	
	// Then we load the sound, detach it, and store the handle in the table.
	theHnd = GetResource('snd ', kTheCoughNoise);
	DetachResource(theHnd);
	theAddressTable->theSound = theHnd;
}










//=============================================================================
//		HandleTheError : Handle any errors															 
//-----------------------------------------------------------------------------
//		Note :	If any error occurs, we beep, post an error, and remove our
//				code. We also have to reset the icon details to show our
//				disabled icons.
//-----------------------------------------------------------------------------
void HandleTheError(void)
{




	// General error handling settings
	gTheParamBlock->removeInstalledCode	= true;
	gTheParamBlock->beepNow				= true;
	gTheParamBlock->postError			= true;
	gTheParamBlock->errorStringsID		= kErrorStrings;



	// Icon details
	SetUpIcons(kDisabledAnimDelay, kMyNumDisabledIcons, kMyFirstDisabledIcon);



	// Just give a general error.
	gTheParamBlock->errorStringIndex = kUnknownError;
}










//=============================================================================
//		SetUpIcons : Set up our icons accordingly.														 
//-----------------------------------------------------------------------------
//		Note :	We are passed in the resource ID of the first icon, the number
//				of icons, and a delay for animation. We just fill in the fields
//				in the paramBlock.
//-----------------------------------------------------------------------------
void SetUpIcons(int animDelay, int numIcons, int firstIcon)
{	int		i;




	// Fill in the fields
	gTheParamBlock->animationDelay	= animDelay;
	gTheParamBlock->numIcons		= numIcons;
	for (i = 1; i <= numIcons; i++)
		gTheParamBlock->theIcons[i] = firstIcon + i - 1;
}

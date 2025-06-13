/*	NAME:
		ES Handler.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains a CODE resource to be used as a handler by
		Extension Shell.

	NOTES:
		¥	Provides a basic shell for creating ES Handler CODE resources.
			You will have to fill in the more Extension specific details,
			and alter the #defines in the .h file to suit.

	___________________________________________________________________________

	VERSION HISTORY:
		(Jan 1994, dg)
			¥	First publicly distributed version.


	___________________________________________________________________________
*/
//=============================================================================
//		Include files 
//-----------------------------------------------------------------------------
#include "ParamBlock.h"
#include "StandaloneCode.h"
#include "ESConstants.h"
#include "ES Handler.h"
#include "CodeConstants.h"





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
//				care of all actual work.
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
{




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
		
		
		// From this point on, the code to be installed is Extension specific, and is
		// therefore shown as pseudo-code. Actual code will be very similar, and can
		// be based on what is shown here. All you will have to do is cut out the
		// irrelevent code, and supply expressions for the right hand side of the
		// statements. Obviously, the indexes into theCodeResources will need to be
		// changed, and you may need more of one given type of code. The indexes
		// should correspond to the error handling code later on, by some #defines
		// in a common .h file ('CodeConstants.h' by default). By placing them in
		// a common .h file, your standalone code can use the #define as an index into
		// the Extension Shell address table to find out what it replaced (if anything).
		// This way, a given code resource will have the same index in the theCodeResources
		// array, the errorIndex value, and the address table. If you need to use an
		// address table, you will have to provide a value for the addressTableSelector
		// field.
/*
		// General details
		gTheParamBlock->installAddressTable		= // Install an address table?
		gTheParamBlock->addressTableSelector	= // Selector of any address table.
		gTheParamBlock->numCodeResources		= // Number of code resources to be installed.


		// Details for a trap patch
		gTheParamBlock->theCodeResources[kCodeThingOne].resType		= // Resource type for code
		gTheParamBlock->theCodeResources[kCodeThingOne].resID		= // Resource ID for code
		gTheParamBlock->theCodeResources[kCodeThingOne].codeType	= kTrapPatchType;
		gTheParamBlock->theCodeResources[kCodeThingOne].theCodeThing.theTrapPatch.trapNum = // Trap number


		// Details for a Gestalt selector
		gTheParamBlock->theCodeResources[kCodeThingTwo].resType		= // Resource type for code
		gTheParamBlock->theCodeResources[kCodeThingTwo].resID		= // Resource ID for code
		gTheParamBlock->theCodeResources[kCodeThingTwo].codeType	= kGestaltSelectorType;
		gTheParamBlock->theCodeResources[kCodeThingTwo].theCodeThing.theGestaltSelector.theSelector = // New selector
		gTheParamBlock->theCodeResources[kCodeThingTwo].theCodeThing.theGestaltSelector.overwriteExistingSelector = // Replace any existing selector?


		// Details for a shutdown task
		gTheParamBlock->theCodeResources[kCodeThingThree].resType	= // Resource type for code
		gTheParamBlock->theCodeResources[kCodeThingThree].resID		= // Resource ID for code
		gTheParamBlock->theCodeResources[kCodeThingThree].codeType	= kShutdownTaskType;
		gTheParamBlock->theCodeResources[kCodeThingThree].theCodeThing.theShutdownTask.theFlags = // Task flags


		// Details for a VBL task
		gTheParamBlock->theCodeResources[kCodeThingFour].resType	= // Resource type for code
		gTheParamBlock->theCodeResources[kCodeThingFour].resID		= // Resource ID for code
		gTheParamBlock->theCodeResources[kCodeThingFour].codeType	= kVBLTaskType;
		gTheParamBlock->theCodeResources[kCodeThingFour].theCodeThing.theVBLTask.vblCount = // Count
		gTheParamBlock->theCodeResources[kCodeThingFour].theCodeThing.theVBLTask.vblPhase = // Phase


		// Details for a low-memory filter
		gTheParamBlock->theCodeResources[kCodeThingFive].resType	= // Resource type for code
		gTheParamBlock->theCodeResources[kCodeThingFive].resID		= // Resource ID for code
		gTheParamBlock->theCodeResources[kCodeThingFive].codeType	= kLowMemFilterType;
		gTheParamBlock->theCodeResources[kCodeThingFive].theCodeThing.theLowMemFilter.theEntryPoint = // Entry point for filter


		// Details for a block of code
		gTheParamBlock->theCodeResources[kCodeThingSix].resType		= // Resource type for code
		gTheParamBlock->theCodeResources[kCodeThingSix].resID		= // Resource ID for code
		gTheParamBlock->theCodeResources[kCodeThingSix].codeType	= kCodeBlockType;
		gTheParamBlock->theCodeResources[kCodeThingSix].theCodeThing.theCodeBlock.reserved = // Dummy field
*/	
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
{



}










//=============================================================================
//		HandleTheError : Handle any errors.														 
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



	// Case out on the error, and post a relevent message. Only a skeleton
	// is shown here, and you should fill in the relevent details for the
	// errors you support. You may also want to take account of the value
	// of gTheParamBlock->theErr when deciding what string to post. The
	// values of errorIndex will depend on what's being installed (from
	// above). The #defines used to describe errorIndex will need to be
	// tailored to your Extension's needs - they are given arbitrary names
	// in this example. See CodeConstants.h for the definitions.
	switch(gTheParamBlock->errorIndex) {
		case kCodeThingOne:
		     gTheParamBlock->errorStringIndex = kErrorWithOne;
		     break;
		     
		case kCodeThingTwo:
		     gTheParamBlock->errorStringIndex = kErrorWithTwo;
		     break;
		     
		case kCodeThingThree:
		     gTheParamBlock->errorStringIndex = kErrorWithThree;
		     break;
		     
		case kCodeThingFour:
		     gTheParamBlock->errorStringIndex = kErrorWithFour;
		     break;
		     
		default:
		     gTheParamBlock->errorStringIndex = kUnknownError;
	}
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

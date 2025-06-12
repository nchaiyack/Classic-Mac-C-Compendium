/*	NAME:
		ESConstants.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Holds various resource IDs and related constants that Extension Shell
		depends upon. Your THINK C projects should be built accordingly.

	___________________________________________________________________________
*/
#ifndef __ESCONSTANTS__
#define __ESCONSTANTS__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
#define kMaxNumIcons			20					// NumIcons ranges from 1..20
#define kMaxNumCodeResources	10					// NumCodeResources ranges from 1..10

#define kESHandlerCodeType  	'CODE'				// Type of the ES Handler code
#define kRespCodeType		   	'CODE'				// Type of the RESP code
#define kAddrsTableCodeType   	'CODE'				// Type of the Address Table code
#define kESHandlerCodeID   		5000				// ID of the ES Handler code
#define kRespCodeID			   	5001				// ID of the RESP code
#define kAddrsTableCodeID  		5002				// ID of the Address Table code

#define kInitialiseParamBlock	0					// Message for the ES Handler code resource
#define kInitialiseAddrsTable	1					// Message for the ES Handler code resource
#define kHandleError			2					// Message for the ES Handler code resource

#define kShiftKey				0x38				// Keycode for the shift key

		
#endif

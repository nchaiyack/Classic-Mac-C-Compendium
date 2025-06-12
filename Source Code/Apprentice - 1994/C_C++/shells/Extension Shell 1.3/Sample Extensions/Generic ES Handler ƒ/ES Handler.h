/*	NAME:
		ES Handler.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Header file for ES Handler.c

	NOTES:
		¥	The defines below are for a generic Extension. You will need to alter
			them for your Extension. You will have to change:
			
				¥	The indexes, names, and contents of the error strings
				
				¥	The details for the disabled icons
				
				¥	The details for the enabled icons

	___________________________________________________________________________
*/
#ifndef __ESHANDLER__
#define __ESHANDLER__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
// Notification Manager error strings
#define kErrorStrings				128							// Resource ID of 'STR#' resource
#define kNeedSystemSeven			1							// Need System 7 index
#define kUnknownError				2							// Unknown error index
#define kErrorWithOne				3							// Error with the 1st code resource
#define kErrorWithTwo				4							// Error with the 2nd code resource
#define kErrorWithThree				5							// Error with the 3rd code resource
#define kErrorWithFour				6							// Error with the 4th code resource
#define kErrorWithFive				7							// Error with the 5th code resource
#define kErrorWithSix				8							// Error with the 6th code resource


// Enabled icons
#define kMyNumEnabledIcons			1							// Number of enabled icons
#define kEnabledAnimDelay			0							// Delay for enabled icon(s)
#define kMyFirstEnabledIcon			128							// First enabled icon


// Disabled icons
#define kMyNumDisabledIcons			1							// Number of disabled icons
#define kDisabledAnimDelay			0							// Delay for disabled icon(s)
#define kMyFirstDisabledIcon 		129							// First disabled icon


#endif

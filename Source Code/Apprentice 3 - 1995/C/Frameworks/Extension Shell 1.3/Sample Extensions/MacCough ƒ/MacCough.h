/*	NAME:
		MacCough.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Header file for MacCough.c

	___________________________________________________________________________
*/
#ifndef __MACCOUGH__
#define __MACCOUGH__
//=============================================================================
//		Defines																 
//-----------------------------------------------------------------------------
// Notification Manager error strings
#define kErrorStrings				128							// Resource ID of 'STR#' resource
#define kNeedSystemSeven			1							// Need System 7 index
#define kUnknownError				2							// Unknown error index


// Enabled icons
#define kMyNumEnabledIcons			1							// Number of enabled icons
#define kEnabledAnimDelay			0							// Delay for enabled icon(s)
#define kMyFirstEnabledIcon			128							// First enabled icon


// Disabled icons
#define kMyNumDisabledIcons			1							// Number of disabled icons
#define kDisabledAnimDelay			0							// Delay for disabled icon(s)
#define kMyFirstDisabledIcon 		129							// First disabled icon


// General resources
#define kTheCoughNoise				128							// The coughing noise


#endif

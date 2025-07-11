/*
	File:			VoicePicker.h

	Contains:		Header file for VoicePicker library routines.
				
	Written by:	Luigi Belverato
				P.O. Box 19,
				20080 Basiglio MI,
				Italy

	Copyright:	�1995 Luigi Belverato
	
	Change History (most recent first):
				06/23/95	0.0.1

	Notes: 		This code uses Apple's Universal Interfaces for C version 2.01f.
	
				Send bug reports to lbelvera@micronet.it
				(if I don't reply within a few days use snail mail as I might have changed internet provider)
*/

//-----------------------------------------------------------------------
// this text document was formatted with Geneva 9, tab size 4

#ifndef	_VOICEPICKERLIB_
#define	_VOICEPICKERLIB_


#include <Speech.h>

// Public interfaces

struct VoiceData
{
	VoiceSpec	voice;
	Fixed	pitch;
	Fixed	rate;
};
typedef struct VoiceData	VoiceData,*VoiceDataPtr,**VoiceDataHandle;

//-----------------------------------------------------------------------
extern Boolean VoicePicker(	ConstStr255Param	passedStr,
						VoiceData			*passedVoiceData);
//-----------------------------------------------------------------------
//	VoicePicker brings up a modal dialog where the user may choose among
//	currently installed speech manager voices, view their characteristics
//	such as gender, age, language and region, change the rate and speed and
//	try them on some editable sample text (that defaults to the built in voice
//	description).
//
//	Returns a Boolean indicating whether the user dismissed the dialog with
//	OK or Cancel
//		
//	Parameters:
//
//	ConstStr255Param	passedStr
//		the prompt to be displayed or nil
//
//	VoiceData			*passedVoiceData
//		the address of a VoiceData structure containg in VoiceSpec the
//		initial voice to be selected in the popup menu and its pitch and rate
//		settings. On return will contain updated values.
//-----------------------------------------------------------------------

#endif _VOICEPICKERLIB_

/* end of file: VoicePicker.h */
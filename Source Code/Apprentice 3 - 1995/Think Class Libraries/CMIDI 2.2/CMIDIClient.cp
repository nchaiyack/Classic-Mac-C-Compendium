/*
 *——— CMIDIClient.cp ————————————————————————————————————————————————————————————————————
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * Superclass:  CObject
 * Subclasses:  None
 *
 * Description:
 *	CMIDIClient.c defines a MIDI Manager client object.
 *
 *	For use with Symantec C++ 6.0, the accompanying THINK Class Library (TCL), and MIDI
 *	Manager 2.0. Refer to the accompanying Microsoft Word document for complete
 *	details about MIDI Manager objects.
 *
 *	If you have comments or questions about this code, you can reach me on
 *	CompuServe at 70441,3055.
 *
 *——————————————————————————————————————————————————————————————————————————————————————
 *———— NOTE ——— NOTE ——— NOTE ——— NOTE ——— NOTE ——— NOTE ——— NOTE ——— NOTE ——— NOTE ————
 *——————————————————————————————————————————————————————————————————————————————————————
 *	If you are not familiar with programming the Apple MIDI Manager, refer to the
 *	"MIDI Management Tools" Version 2.0, available from APDA.  You MUST have the
 *	software (MIDI.H and the library) from this package in order to use these objects.
 *	It will not work without this.
 *——————————————————————————————————————————————————————————————————————————————————————
 *	REVISION HISTORY:
 *		August ??, 1990			- Original release (1.0).
 *		November 5, 1990		- Added checks for midiMgrVer to most methods.
 *		August 1991				- updated for THINK C 5.0 as version 2.0
 *		July 1993				- updated for Symanctec C++ 6.0.
 *——————————————————————————————————————————————————————————————————————————————————————
 */

#include "CMIDIClient.h"			// This code's header file

extern	OSType	gSignature;			// Used to register client

/*
 *——— gMIDIClient (global variable) ——————————————————————————————————————
 * Because these objects are designed only for a single MIDI client, we
 * define this global to hold it.
 *————————————————————————————————————————————————————————————————————————
 */
 
CMIDIClient * gMIDIClient = (CMIDIClient *) 0;


/*
 *——— CMIDIClient::IMIDIClient ———————————————————————————————————————————
 * Initialize this object.
 *
 * Sign into the MIDI Manager.  Save the midiMgrVerNum.  It signs
 * in with the application's name, and tries to find it's BNDL type.
 * Returns an error code indicating result of initializing and signing
 * into MIDI Manager.
 *
 * NOTE:	Only one instance of a MIDI Manager Client can be made, with
 *			the current design of these objects.  Other methods assume
 *			that the 'gMIDIClient' variable (defined above) points to a
 *			valid CMIDIClient object.
 *————————————————————————————————————————————————————————————————————————
 */
OSErr CMIDIClient::IMIDIClient(short theIconID)
{
	OSErr			err;
	Handle			theIconHndl;		// Used by MIDISignIn
	Str255			theName;			// me too...
	short			i;					// Junk variables
	Handle			h;
	unsigned long	version;

	// Make sure MIDIMgr is installed and save version number.
 
	midiMgrVerNum   	= SndDispVersion(midiToolNum);
	shortMidiMgrVerNum	= 0;

	if (midiMgrVerNum == 0)				// Damn!  Things were going so well...
	{
		return ErrNoMIDI;				// No MIDI driver seems to be loaded
	}

	// Sign in to the MIDI Manager.

	theIconHndl = GetResource('ICN#',theIconID);	// Let’s get our icon
	CheckResource(theIconHndl);			// Just checkin’...
	GetAppParms(theName, &i, &h);		// Get the name of this application

	err = MIDISignIn(gSignature, 		// Use application signature
						0L,				// Don’t need refCon
						theIconHndl,
						theName);		// Use theName for this MIDI client
	ReleaseResource(theIconHndl);
	if (err)							// If unable to sign into MM, treat
		midiMgrVerNum = 0;				// same as if no driver present.

	version = midiMgrVerNum;
	version >>= 16;
	shortMidiMgrVerNum = (unsigned short) version;

	return err;
}

/*
 *——— CMIDIClient::Dispose ————————————————————————————————————————
 * Save connections, sign out of MIDI Manager.
 *—————————————————————————————————————————————————————————————————
 */

void CMIDIClient::Dispose(void)
{
 	if (midiMgrVerNum)					// If we are signed in to MM
 		MIDISignOut(gSignature);
	inherited::Dispose();				// Call CObject method
}										// It’s Miller Time…


#ifndef __cplusplus

	// if using THINK C compiler, not Symantec C++, these are non-inline methods.

	MIDIIDListHdl	CMIDIClient::GetPorts(void)
	{
		return ((midiMgrVerNum) ? MIDIGetPorts(gSignature) : (MIDIIDListHdl) 0);
	}
	
	Boolean			CMIDIClient::WorldChanged(void)
	{
		return ( (midiMgrVerNum) ? MIDIWorldChanged(gSignature) : FALSE );
	}
	
	unsigned long	CMIDIClient::GetVerNum(void)
	{
		return midiMgrVerNum;
	}
	
	unsigned short	CMIDIClient::GetShortVerNum(void)
	{
		return shortMidiMgrVerNum;
	}
#endif
// end of CMIDIClient.cp

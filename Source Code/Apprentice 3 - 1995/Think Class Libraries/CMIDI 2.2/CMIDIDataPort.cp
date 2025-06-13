/*
 *ÑÑÑ CMIDIDataPort.cp ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * Superclass:  CMIDIPort
 * Subclasses:  CInputPort
 *				COutputPort
 *
 * Description:
 *	CMIDIDataPort.c defines a MIDI Manager port object.  CMIDIDataPort is an abstract
 *	type, containing variables and methods common to input and output port types.
 *
 *	For use with Symantec C++ 6.0, the accompanying THINK Class Library (TCL), and MIDI
 *	Manager 2.0. Refer to the accompanying Microsoft Word document for complete
 *	details about MIDI Manager objects.
 *
 *	If you have comments or questions about this code, you can reach me on
 *	CompuServe at 70441,3055.
 *
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 *ÑÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑ NOTE ÑÑÑÑ
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 *	If you are not familiar with programming the Apple MIDI Manager, refer to the
 *	"MIDI Management Tools" Version 2.0, available from APDA.  You MUST have the
 *	software (MIDI.H and the library) from this package in order to use these objects.
 *	It will not work without this.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 *	REVISION HISTORY:
 *		August ??, 1990			- Original release (1.0).
 *		November 5, 1990		- Added checks for midiMgrVer to most methods.
 *		August 1991				- updated for THINK C 5.0 as version 2.0
 *		July 1993				- updated for Symantec C++ 6.0.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

#include "CMIDIDataPort.h"					// This code's header file

extern	OSType			gSignature;			// Used to register client
extern CMIDIClient * 	gMIDIClient;

/*
 *ÑÑÑ NOTE ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Note that there is no initialization method for this class.
 * The subclasses CMIDIInputPort and CMIDIOutputPort initialization
 * methods call IMIDIPort() directly.  There is not really any
 * need for an initializer here.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */


/*
 *ÑÑÑ CMIDIDataPort::LoadPatches ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Read in patch information from a specified resource. This may
 * also return resNotFound error, which should be handled by the
 * application.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr CMIDIDataPort::LoadPatches(ResType theResType, short theResID)
{
	MIDIPortInfoHdl	rsrcPortInfoH, ourPortInfoH;
	short			i;
	OSErr			theErr = noErr;

	if ( ! itsVersion )
		return ErrNoMIDI;

	ourPortInfoH = GetPortInfo();
	
	switch (itsResult)		// This was set during initialization
	{
	case midiVConnectMade:
		if ((**ourPortInfoH).numConnects > 0)
			break;
									// else fall through to next case...
									// (only have a time base connection)
		itsResult = noErr;			//  cheat a little bit, and ... FALL THROUGH!
	case noErr:
	
		// Check for virtual connections stored in the specified resource.

		rsrcPortInfoH = (MIDIPortInfoHdl) GetResource(theResType, theResID);
		if ( rsrcPortInfoH == (MIDIPortInfoHdl) 0 )
			return resNotFound;

		// Lock resource and call MIDIConnectData

		HLock((Handle) rsrcPortInfoH);
		if ((**ourPortInfoH).type == (**rsrcPortInfoH).type)
		{
			for (i = 0; i < (**rsrcPortInfoH).numConnects; i++)
			{
				theErr = MIDIConnectData(gSignature,
							itsPortID,
							(**rsrcPortInfoH).cList[i].clientID,
							(**rsrcPortInfoH).cList[i].portID);
			}
		}
		HUnlock((Handle) rsrcPortInfoH);
		ReleaseResource((Handle) rsrcPortInfoH);
		break;
	default:						// Not likely to see this case
		break;
	}
	return theErr;
}


/*
 *ÑÑÑ CMIDIDataPort::Get/SetTCFormat ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Retrieve or set the time code format.  Constants for valid
 * formats are in MIDI.h
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

short CMIDIDataPort::GetTCFormat(void)
{
	return (itsVersion ? MIDIGetTCFormat(itsRefNum) : -1);
}

void CMIDIDataPort::SetTCFormat(short theFormat)
{
	if (itsVersion)
		MIDISetTCFormat(itsRefNum, theFormat);
}

// end of CMIDIDataPort.cp

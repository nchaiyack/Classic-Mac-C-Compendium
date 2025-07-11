/*
 *ΡΡΡ CMIDITimePort.cp ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * Description:
 *	CMIDITimePort.c defines a MIDI Manager time port object.
 *
 *	For use with Symantec C++ 6.0, the accompanying THINK Class Library (TCL), and MIDI
 *	Manager 2.0. Refer to the accompanying Microsoft Word document for complete
 *	details about MIDI Manager objects.
 *
 *	If you have comments or questions about this code, you can reach me on
 *	CompuServe at 70441,3055.
 *
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 *ΡΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡ NOTE ΡΡΡΡ
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 *	If you are not familiar with programming the Apple MIDI Manager, refer to the
 *	"MIDI Management Tools" Version 2.0, available from APDA.  You MUST have the
 *	software (MIDI.H and the library) from this package in order to use these objects.
 *	It will not work without this.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 *	REVISION HISTORY:
 *		August ??, 1990			- Original release (1.0).
 *		November 5, 1990		- Added checks for midiMgrVer to most methods.
 *		August, 1991			- Updated for THINK C 5.0 as version 2.0
 *		July, 1993				- Updated for Symantec C++ 6.0.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */

#include "CMIDITimePort.h"					// This code's header file
#include <CApplication.h>

extern CApplication *	gApplication;		// Used in SetConnection
extern OSType			gSignature;			// Used to register client
extern CMIDIClient * 	gMIDIClient;		// Defined in CMIDIClient.c


/*
 *ΡΡΡ CMIDITimePort::IMIDITimePort ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Initialize a time port object. This method sets up the MIDIPortParams
 * data structure and calls CMIDIPort::IMIDIPort().
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */
 
OSErr CMIDITimePort::IMIDITimePort(StringPtr	theName,
								   OSType		thePortID,
								   Boolean		theVisibleFlag,
								   short		theFormat)
{
	MIDIPortParams	portParams;
	OSErr			theResult;

	portParams.portID				= thePortID;
	portParams.portType				= midiPortTypeTime;
	if (theVisibleFlag == FALSE)
	{
		if (this->itsVersion >= 0x0200)
			portParams.portType		|= midiPortInvisible;		// Use 2.0 preferred syntax
		else
			portParams.portType		= midiPortTypeTimeInv;		// Use 1.x syntax
	}
	portParams.timeBase				= 0;
	portParams.offsetTime			= 0;
	portParams.readHook				= (Ptr) 0;
	portParams.refCon				= 0;
	portParams.initClock.sync		= midiInternalSync;		// Always start out internal
	portParams.initClock.curTime	= 0;
	portParams.initClock.format		= theFormat;
	BlockMove(theName, portParams.name, theName[0]+1);

	theResult = IMIDIPort(&portParams, 0);
	if (theResult == noErr)
	{
		UpdateSync();		// Check for time base connections
	}

	return theResult;
}

/*
 *ΡΡΡ CMIDITimePort::LoadPatches ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Load any time patches, and make connections.
 * Patches are stored in the resource specified by the parameters. This may
 * also return resNotFound error, which should be handled by the application.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */
OSErr CMIDITimePort::LoadPatches(ResType theResType, short theResID)
{
	OSErr				theErr = noErr;
	MIDIPortInfoHdl		rsrcPortInfoH, ourPortInfoH;
	short				i;
	Str255				theName;

	if (this->itsVersion == 0)		// CouldnΥt open the MIDI Manager for some reason
		return ErrNoMIDI;

	if (this->itsResult != noErr)	// MM initialization had failed.
		return this->itsResult;
		
	// Check for virtual connections stored in specified resource.	
	// If resource not found, or wrong size return resource error.

	rsrcPortInfoH = (MIDIPortInfoHdl) GetResource(theResType, theResID);
	if ( rsrcPortInfoH == (MIDIPortInfoHdl) 0 )
		return resNotFound;

	// Make MIDI time connections.

	HLock((Handle) rsrcPortInfoH);
	ourPortInfoH = GetPortInfo();
	GetPortName(theName);
	if ((**ourPortInfoH).type == (**rsrcPortInfoH).type)	// Is this resource a time port?
	{
		if ((**rsrcPortInfoH).timeBase.clientID != kNoClient)	// Were we supposed to be synchronized
		{														// to another client?
			theErr = MIDIConnectTime(
						(**rsrcPortInfoH).timeBase.clientID, 	// Yes, make that client our time base
						(**rsrcPortInfoH).timeBase.portID,
						gSignature,
						itsPortID);
			if (theErr != midiVConnectErr)		// Is client still signed in?
				SetExternalSync();
		}

		// Were we someone else's time base?

		for (i = 0; i < (**rsrcPortInfoH).numConnects; ++i)
		{
			theErr = MIDIConnectTime(gSignature, // We are the time base for them
						itsPortID,
						(**rsrcPortInfoH).cList[i].clientID,
						(**rsrcPortInfoH).cList[i].portID);
			if (theErr) break;
		}
	}
	HUnlock((Handle) rsrcPortInfoH);
	ReleaseResource((Handle) rsrcPortInfoH);

	return theErr;
}


/*
 *ΡΡΡ CMIDITimePort::UpdateSync ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Set internal or external time base synchronization depending on
 * current connections.  It returns the current synchronization setting
 * (same as GetSync).
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */

short CMIDITimePort::UpdateSync(void)
{
	MIDIPortInfoHdl		thePortInfoH;
	register short		theResult;

	thePortInfoH = GetPortInfo();

	if (thePortInfoH)
	{
		theResult = GetSync();
		if ( (**thePortInfoH).timeBase.clientID != kNoClient )
		{
			if (theResult != midiExternalSync)
				SetExternalSync();
		}
		else
		{
			if (theResult != midiInternalSync)
				SetInternalSync();
		}
		DisposHandle((Handle) thePortInfoH);
	}
	return (itsVersion ? MIDIGetSync(itsRefNum) : -1);
}


/*
 *ΡΡΡ CMIDITimePort::Perform ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * This turns the time port into a CChore, so that you can call
 * gApplication->AssignIdleChore() with this object.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */

void CMIDITimePort::Perform(long * maxSleep)
{
	if ( gMIDIClient->WorldChanged() )
		UpdateSync();
}


/*
 *ΡΡΡ CMIDITimePort::SetConnection ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * This sets the connection procedure. Note that for pre-2.0 or
 * a NULL connectionProc, the previous method, Perform(), will
 * called at idle time, checking for a change in the wind.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */

void CMIDITimePort::SetConnection(ProcPtr theConnectionProc)
{
	if ((gMIDIClient->GetShortVerNum() < 0x200 )		// If pre-2.0
		|| (theConnectionProc == NULL) )				// or a null pointer
		gApplication->AssignIdleChore( (CChore *) this );
	else												// else use connection proc
		SetConnectionProc(theConnectionProc, GetRefCon());
}


/*
 *ΡΡΡ CMIDITimePort:: trivial methods ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * These are not inlined, because they are not likely to be
 * called in a time critical situation--time base synchronization
 * doesn't change often.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */

short CMIDITimePort::GetSync(void)
{
	return (itsVersion ? MIDIGetSync(itsRefNum) : -1);
}

void CMIDITimePort::SetExternalSync(void)
{
	if (itsVersion)
		MIDISetSync(itsRefNum, midiExternalSync);
}

void CMIDITimePort::SetInternalSync(void)
{
	if (itsVersion)
		MIDISetSync(itsRefNum, midiInternalSync);
}

#ifndef __cplusplus

	// if using THINK C compiler, not Symantec C++, these are non-inline methods.

	long CMIDITimePort::GetCurTime(void)
	{
		return (itsVersion ? MIDIGetCurTime(itsRefNum) : 0);
	}
	
	void CMIDITimePort::SetCurTime(long theTime)
	{
		if (itsVersion) MIDISetCurTime(itsRefNum, theTime);
	}
	
	void CMIDITimePort::StartTime(void)
	{
		if (itsVersion) MIDIStartTime(itsRefNum); 
	}
	void CMIDITimePort::StopTime(void)
	{
		if (itsVersion) MIDIStopTime(itsRefNum);
	}
	
	long CMIDITimePort::GetOffsetTime(void)
	{
		return (itsVersion ? MIDIGetOffsetTime(itsRefNum) : 0);
	}
	
	void CMIDITimePort::SetOffsetTime(long theOffset)
	{
		if (itsVersion) MIDISetOffsetTime(itsRefNum, theOffset);
	}
	
	void CMIDITimePort::WakeUp(long theBaseTime, long thePeriod, ProcPtr theTimeProc)
	{
		if (itsVersion) MIDIWakeUp(itsRefNum, theBaseTime, thePeriod, theTimeProc);
	}
#endif
// end of CMIDITimePort.cp

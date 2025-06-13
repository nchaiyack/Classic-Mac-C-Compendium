/*
 *ÑÑÑ CMIDIPort.cp ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * Superclass:  CObject
 * Subclasses:  CDataPort CTimePort
 *
 * Description:
 *	CMIDIPort.c defines a MIDI Manager port object.  CMIDIPort is an abstract type,
 *	containing variables and methods common to all three port types.
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
 *		July 1993				- updated for Symantec C++ 6.0.  Some simple methods
 * 									were inlined.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

#include "CMIDIPort.h"

extern OSType		gSignature;			// Used to register client
extern CMIDIClient	* gMIDIClient;


/*
 *ÑÑÑ CMIDIPort::IMIDIPort ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * This method is called by IMIDIInputPort(), IMIDIOutputPort(), and IMIDITimePort().
 * When this method is called, the portParams structure should be completely filled in.
 * 
 * Note that this method checks for gMIDIClient == NULL, which might indicate that the
 * gMIDIClient object was never created. This provides another check against programmer
 * stupidity (present company exceptedÉ).
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr CMIDIPort::IMIDIPort(MIDIPortParamsPtr portParams, short bufSize)
{
	short			theRefNum;
	unsigned char *	theLen;

	if ((gMIDIClient == 0) || ((itsVersion = gMIDIClient->GetShortVerNum()) == 0))
	{
		itsVersion = itsPortID = itsRefNum = 0;
		return (ErrNoMIDI);
	}

	// Otherwise, call MIDIAddPort

	theLen = &(portParams->name[0]);		// Truncate name if necessary
	if (*theLen > midiMaxNameLen) *theLen = midiMaxNameLen;

	itsPortID = portParams->portID;			// Save our port ID in the object
	itsResult = MIDIAddPort(gSignature,		// Call MIDI Manager
							bufSize, &theRefNum, portParams);
	itsRefNum = theRefNum;					// Save the port reference number
	return itsResult;
}

/*
 *ÑÑÑ CMIDIPort::Dispose ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Dispose of this MIDI port.  Note that we do NOT call MIDIRemovePort()
 * here.  This has been seen to cause problems with the MIDI Manager.
 *
 * Normally, this isn't a problem, since you typically would only call
 * this when you are quitting.  If you specifically need to call
 * MIDIRemovePort(), then you can override this method.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

void CMIDIPort::Dispose(void)
{
	inherited::Dispose();
}


/*
 *ÑÑÑ CMIDIDataPort::LoadPatches ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * This is a virtual function which is overloaded in subclasses
 * CMIDIDataPort and CMIDITimePort.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr CMIDIPort::LoadPatches(ResType theResType, short theResID)
{
	this->SubclassResponsibility();
	return noErr;
}


/*
 *ÑÑÑ CMIDIPort::SavePatches ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Saves the current port connections in a specified resource.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr CMIDIPort::SavePatches(ResType theResType, short theResID)
{
	Str255				theName;
	Handle				h;
	MIDIPortInfoHdl		portInfo;

	if ( ! itsVersion )
		return (ErrNoMIDI);

	if (itsResult != noErr)			// May have been a virtual connection,
		return(itsResult);			// so don't do anything.

	GetPortName(theName);
	h = GetResource(theResType, theResID);	// Delete resource if it exists
	if (h)
	{
		RmveResource(h);
		DisposHandle(h);
		UpdateResFile(CurResFile());
	}
	portInfo = (MIDIPortInfoHdl) MIDIGetPortInfo(gSignature, itsPortID);
	if (portInfo)
	{
		if ((**portInfo).numConnects > 0)	// No connection, no record
		{
			AddResource((Handle) portInfo, theResType, theResID, theName);
			WriteResource((Handle) portInfo);
			UpdateResFile(CurResFile());
		}
		ReleaseResource((Handle) portInfo);
	}
	return (noErr);
}


/*
 *ÑÑÑ Misc, trivial methods ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * These aren't individually commented because frankly, I didnÕt feel
 * like it, nor do they really need any comments besides this one.
 * These are not inlined because of the infrequency of their being
 * called.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

void CMIDIPort::GetPortName(StringPtr theName)
{
	if (itsVersion)
		MIDIGetPortName(gSignature, itsPortID, theName);
}

void CMIDIPort::SetPortName(StringPtr theName)
{
	if (itsVersion)
	{
		if (theName[0] > midiMaxNameLen)
			theName[0] = midiMaxNameLen;
		MIDISetPortName(gSignature, itsPortID, theName);
	}
}

MIDIPortInfoHdl	CMIDIPort::GetPortInfo(void)
{
	return(itsVersion ? MIDIGetPortInfo(gSignature, itsPortID) : 0);
}

void CMIDIPort::SetConnectionProc(ProcPtr theConnectProc, long theRefCon)
{
	if (itsVersion >= 0x200)		// Version 2.0 or later
		MIDISetConnectionProc(itsRefNum, theConnectProc, theRefCon);
}

void CMIDIPort::GetConnectionProc(ProcPtr * theConnectProc, long * theRefCon)
{
	if (itsVersion >= 0x200)		// Version 2.0 or later
		MIDIGetConnectionProc(itsRefNum, theConnectProc, theRefCon);
}

#ifndef __cplusplus

	// if using THINK C compiler, not Symantec C++, these are non-inline methods.

	short CMIDIPort::GetRefNum(void)
	{
		return itsRefNum;
	}
	
	long CMIDIPort::GetRefCon(void)
	{
		return (itsVersion ? MIDIGetRefCon(itsRefNum) : 0);
	}
	
	void CMIDIPort::SetRefCon(long theRefCon)
	{
		if (itsVersion) MIDISetRefCon(itsRefNum, theRefCon);
	}
#endif

// end of CMIDIPort.cp

/*
 *ΡΡΡ CMIDIInputPort.cp ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 *	Superclass:	 CDataPort
 *	Subclasses:	 None
 *
 * Description:
 *	CMIDIInputPort.c defines a MIDI Manager port object.
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
 *		August 1991				- updated for THINK C 5.0 as version 2.0
 *		July 1993				- updated for Symantec C 6.0. Made simple methods inline
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */
 
#include "CMIDIInputPort.h"
#include "CMIDITimePort.h"

static const int midiPoll = 104;			// From MIDI.a
static const int midiDiscardPacket = 176;

static	pascal void (*CMIDIInputPort::midiDiscardProc) (short theRefNum, MIDIPacketPtr thePacket);
static	pascal void (*CMIDIInputPort::midiPollProc) (short theRefNum, long theOffsetTime);
static	pascal void (*CMIDIInputPort::midiFlushProc) (short theRefNum);

/*
 *ΡΡΡ CMIDIInputPort::IMIDIInputPort ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Initialize Input Port. This method sets up the MIDIPortParams data structure
 * and calls CMIDIPort::IMIDIPort().  If version is >= 2.0, then save any
 * direct ProcPtrs.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */

OSErr CMIDIInputPort::IMIDIInputPort(StringPtr			theName,
									 OSType				thePortID,
								  	 Boolean			theVisibleFlag,
									 CMIDITimePort *	theTimePort,
									 long   			theOffset,
									 short  			theBufSize,
									 ProcPtr  			theReadHook)
{
	MIDIPortParams	portParams;		// MIDI Mgr Init data structure
	OSErr			theResult;
	
	portParams.portID			= thePortID;
	portParams.portType			= midiPortTypeInput;
	if ( (theVisibleFlag == FALSE) && (itsVersion >= 0x0200) )	// Invisible input port, in 2.x
		portParams.portType		|= midiPortInvisible;

	portParams.timeBase			= theTimePort ? theTimePort->GetRefNum() : 0;
	portParams.offsetTime		= theOffset;
	portParams.readHook			= (Ptr) theReadHook;
	portParams.refCon			= SetCurrentA5();
	BlockMove(theName, portParams.name, theName[0]+1);

	theResult = IMIDIPort(&portParams, theBufSize);
	if (itsVersion >= 0x0200)
	{
		midiDiscardProc	= (pascal void (*) (short, MIDIPacketPtr)) MIDICallAddress(midiDiscardPacket);
		midiPollProc	= (pascal void (*) (short, long)) MIDICallAddress(midiPoll);
	}
	return theResult;
}

#ifndef __cplusplus

	// if using THINK C compiler, not Symantec C++, these are non-inline methods.
	
	ProcPtr	CMIDIInputPort::GetReadHook(void)
	{
		return (itsVersion ? MIDIGetReadHook(itsRefNum) : 0);
	}
	
	void	CMIDIInputPort::SetReadHook(ProcPtr theReadHook)
	{
		if (itsVersion)
			MIDISetReadHook(itsRefNum, theReadHook);
	}
	
	void	CMIDIInputPort::Flush(void)
	{
		if (itsVersion >= 0x0200)
			(*midiFlushProc) (itsRefNum);
		else
			if (itsVersion) MIDIFlush(itsRefNum);
	}
	
	void	CMIDIInputPort::Poll(long offsetTime)
	{
		if (itsVersion >= 0x0200)
			(*midiPollProc) (itsRefNum, offsetTime);
		else
			if (itsVersion) MIDIPoll(itsRefNum, offsetTime);
	}
	
	void	CMIDIInputPort::DiscardPacket(MIDIPacketPtr thePacket)
	{
		if (itsVersion >= 0x0200)		// Only valid on MM 2.0 or later!
			(*midiDiscardProc) (itsRefNum, thePacket);
	}
#endif

// end of CMIDIInputPort.cp

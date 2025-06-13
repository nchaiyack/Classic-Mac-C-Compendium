/*
 *ÑÑÑ CMIDIOutputPort.cp ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * Superclass:  CDataPort
 * Subclasses:  None
 *
 * Description:
 *	CMIDIOutputPort.c defines a MIDI Manager port object.
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
 *		July 1993				- updated for Symantec C++ 6.0.  Inlined simple methods.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

#include "CMIDIOutputPort.h"
#include "CMIDITimePort.h"

static const int midiWritePacket = 108;		// EQU from MIDI.a

static pascal OSErr (*CMIDIOutputPort::midiWriteProc) (short refnum, MIDIPacketPtr packet);

/*
 *ÑÑÑ IMIDIOutputPort ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Initialize the Output Port object.  This method sets up the MIDIPortParams data
 * structure and calls CMIDIPort::IMIDIPort().  If version is >= 2.0, then save
 * any direct ProcPtrs.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr CMIDIOutputPort::IMIDIOutputPort(StringPtr		theName,
									   OSType			thePortID,
									   Boolean			theVisibleFlag,
									   CMIDITimePort *	theTimePort,
									   long				theOffset)
{
	MIDIPortParams		portParams;		// MIDI Mgr Init data structure
	OSErr				theResult;

	portParams.portID			= thePortID;
	portParams.portType			= midiPortTypeOutput;
	if ( (theVisibleFlag == FALSE) && (itsVersion >= 0x0200) )	// Invisible output port, in 2.x
		portParams.portType		|= midiPortInvisible;

	portParams.timeBase			= theTimePort ? theTimePort->GetRefNum() : 0;
	portParams.offsetTime		= theOffset;
	portParams.readHook			= (Ptr) 0;
	portParams.refCon			= SetCurrentA5();
	BlockMove(theName, portParams.name, theName[0]+1);

	theResult = IMIDIPort(&portParams, 0);

	if (itsVersion >= 0x200)
	{
		midiWriteProc = (pascal OSErr (*) (short, MIDIPacketPtr)) MIDICallAddress(midiWritePacket);
	}

	return theResult;
}


/*
 *ÑÑÑ CMIDIOutputPort::DoMIDIWrite ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * This local function does all the real work of sending MIDI data.
 *
 * This method is classified as 'private' in CMIDI.h.  If you wish to call
 * this method directly, simply change the header file.
 *
 * If you know your application is going to write a lot of short or fixed
 * length packets, you may wish to override or modify this method to remove the
 * BlockMove trap and multi-length packet logic overhead.
 *
 * It is called by CMIDIOutputPort::Write() and CMIDIOutputPort::WriteTS().
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr CMIDIOutputPort::DoMIDIWrite(	char *			theData,
									short			theDataLen,
									unsigned char	theFlags,
									long			theTimeStamp)
{
	MIDIPacket				midiPacket;		// MIDI Manager packet to be transmitted
	register MIDIPacketPtr	midiPPtr = &midiPacket;
	
	if (itsVersion == 0) return (ErrNoMIDI);
	
	if (theDataLen < 250)			// Handle short packets directly
	{
		midiPPtr->flags = theFlags | midiMsgType | midiNoCont;
		midiPPtr->len   = theDataLen + 6;
		if (theFlags == midiTimeStampValid)
			midiPPtr->tStamp = theTimeStamp;
		BlockMove(theData, &(midiPPtr->data[0]), theDataLen);
		if (midiWriteProc)
			return (*midiWriteProc) (itsRefNum, &midiPacket);		// ¥¥¥ Call MIDI Manager
		else
			return MIDIWritePacket(itsRefNum, &midiPacket);
	}
	else					// Multiple packet send	(more than 249 bytes)
	{
		register char *	source = theData;
		register short	thisPacketLen;
		OSErr			err;

		do
		{
			midiPPtr->flags  = theFlags | midiMsgType;		// Re-init each time
			if (theFlags == midiTimeStampValid)
				midiPPtr->tStamp = theTimeStamp;
			thisPacketLen = (theDataLen < 249) ? theDataLen : 249;
			BlockMove(source, &(midiPPtr->data[0]), thisPacketLen);	// Data to write
			
			// Set the continuation bits field in the flags byte.
			
			theDataLen -= thisPacketLen;
			if (source == theData)							// the first piece of the message
			{
				if (theDataLen > 0)							// Multiple writes required?
					midiPPtr->flags |= midiStartCont;		// Yes
				else										// Everything fits in one packet
					midiPPtr->flags |= midiNoCont;			// SHOULD NEVER SEE THIS CASE HERE!
			}
			else
			{
				if (theDataLen > 0)							// a middle piece
					midiPPtr->flags |= midiMidCont;
				else										// Last piece of the message
					midiPPtr->flags |= midiEndCont;
			}
			midiPPtr->len = thisPacketLen + 6;				// Set the packet length
			if (midiWriteProc)
				err = (*midiWriteProc) (itsRefNum, &midiPacket);	// ¥¥¥ Call MIDI Manager
			else
				err = MIDIWritePacket(itsRefNum, &midiPacket);
			if (err) break;

			// You may want to add more sophisticated error handling here.  Also, if you are
			// sending large packets like system exclusive messages, you may need to add a
			// timing delay here so you don't overflow the output buffer or the device.

			source += thisPacketLen;						// Bump pointer

		} while (theDataLen > 0);		// Loop for next portion of data
		return err;						// Indicates last result
	}
}


/*
 *ÑÑÑ WritePacket ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Call the MIDI Manager to send the specified packet.  This is
 * called by the inline methods Write and WriteTS.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

OSErr	CMIDIOutputPort::WritePacket(MIDIPacketPtr theMIDIPacket)
{
	if (itsVersion >= 0x200)
		return (*midiWriteProc) (itsRefNum, theMIDIPacket);
	else
		return (itsVersion > 0 ? MIDIWritePacket (itsRefNum, theMIDIPacket) : ErrNoMIDI);
}

#ifndef __cplusplus

	// if using THINK C compiler, not Symantec C++, these are non-inline methods.
	
	OSErr CMIDIOutputPort::Write(char * theData, short theDataLen)
	{
		return DoMIDIWrite(theData, theDataLen, midiTimeStampCurrent, 0);
	}
	
	OSErr CMIDIOutputPort::WriteTS(char * theData, short theDataLen, long theTimeStamp)
	{
		return DoMIDIWrite(theData, theDataLen, midiTimeStampValid, theTimeStamp);
	}

#endif

// end of CMIDIOutputPort.cp

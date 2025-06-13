/*
 *ÑÑÑ CMIDIOutputPort.h ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 *	For use with Symantec C++ 6.0 and the THINK Class Library.
 *
 * Description:
 *	This class defines a MIDI Manager output port object.
 *
 *	Refer to the CMIDI Programmer's Manual for a complete desciption of
 *	this class.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */
#pragma once
#include "CMIDIDataPort.h"

CLASS CMIDITimePort;

class CMIDIOutputPort : public CMIDIDataPort
{

public:

	OSErr			IMIDIOutputPort(
						StringPtr		theName, 		// Pascal format, 31 char max
						OSType			thePortID,
						Boolean			theVisibleFlag,
						CMIDITimePort *	theTimePort,
						long			theOffset);
	OSErr			WritePacket(MIDIPacketPtr theMIDIPacket);
#ifdef __cplusplus
	OSErr			Write(char * theData, short theDataLen)
						{ return DoMIDIWrite(theData, theDataLen, midiTimeStampCurrent, 0); };
	OSErr			WriteTS(char * theData, short theDataLen, long theTimeStamp)
						{ return DoMIDIWrite(theData, theDataLen, midiTimeStampValid, theTimeStamp); };
#else
	OSErr			Write(char * theData, short theDataLen);
	OSErr			WriteTS(char * theData, short theDataLen, long theTimeStamp);
#endif
private:

	OSErr			DoMIDIWrite(
						char *			theData,
						short			theDataLen,
						unsigned char	theFlags,
						long			theTimeStamp);

	static pascal OSErr	(*midiWriteProc) (short refnum, MIDIPacketPtr packet);
};

// end of CMIDIOutputPort.h

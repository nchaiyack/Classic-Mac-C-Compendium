/*
 *ΡΡΡ CMIDIInputPort.h ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * For use with Symantec C++ 6.0 and the THINK Class Library.
 *
 * Description:
 *	This class defines a MIDI Manager Input port object.
 *
 *	Refer to the CMIDI Programmer's Manual for a complete desciption of
 *	this class.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */
#pragma once
#include "CMIDIDataPort.h"

CLASS CMIDITimePort;

class CMIDIInputPort : public CMIDIDataPort
{

public:

	OSErr 	IMIDIInputPort(
				StringPtr		theName, 		// Pascal format, 31 char max
				OSType			thePortID,
				Boolean			theVisibleFlag,
				CMIDITimePort *	theTimePort,
				long			theOffset,		// Current, nothing, everything
				short			theBufSize,
				ProcPtr			theReadHook);
#ifdef __cplusplus
	ProcPtr	GetReadHook(void)
			{
				return (itsVersion ? MIDIGetReadHook(itsRefNum) : 0);
			};
	void	SetReadHook(ProcPtr theReadHook)
			{
				if (itsVersion)
					MIDISetReadHook(itsRefNum, theReadHook);
			};
	void	Flush(void)
			{
				if (itsVersion >= 0x0200)
					(*midiFlushProc) (itsRefNum);
				else
					if (itsVersion) MIDIFlush(itsRefNum);
			};
	void	Poll(long offsetTime)
			{
				if (itsVersion >= 0x0200)
					(*midiPollProc) (itsRefNum, offsetTime);
				else
					if (itsVersion) MIDIPoll(itsRefNum, offsetTime);
			};
	void	DiscardPacket(MIDIPacketPtr thePacket)
			{
				if (itsVersion >= 0x0200)		// Only valid on MM 2.0 or later!
					(*midiDiscardProc) (itsRefNum, thePacket);
			};
#else
	ProcPtr	GetReadHook(void);
	void	SetReadHook(ProcPtr theReadHook);
	void	Flush(void);
	void	Poll(long offsetTime);
	void	DiscardPacket(MIDIPacketPtr thePacket);
#endif
private:

	static pascal void	(*midiDiscardProc) (short theRefNum, MIDIPacketPtr thePacket);
	static pascal void	(*midiPollProc) (short theRefNum, long theOffsetTime);
	static pascal void	(*midiFlushProc) (short theRefNum);
};

// end of CMIDIInputPort.h

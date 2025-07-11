/*
 *ΡΡΡ CMIDIClient.h ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * For use with Symantec C++ 6.0 and the THINK Class Library.
 *
 * Description:
 *	This class defines a MIDI Manager client object.  Every application that
 *	creates CMIDI objects must first create a CMIDIClient object, gMIDIClient
 *	(defined in CMIDIClient.c).  You should not create any additional
 *	CMIDIClient objects.
 *
 *	Refer to the CMIDI Programmer's Manual for a complete desciption of
 *	this class.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */
#pragma once
#include <CObject.h>
#include <MIDI.h>				// ₯₯₯Κ!!! DANGER, WILL ROBINSON !!! ₯₯₯
								// You must use 2.0 version of MIDI.h, not the
								// one included with THINK C 5.0 (ver 1.2).
								// Otherwise, you will get compile and link errors.

#define ErrNoMIDI	-249		// MIDI MANAGER DRIVER NOT PRESENT. (It happens more
								// often than you thinkΙ)  Minus 249 is one more than
								// the first real MIDI Manager error code, and does
								// not appear to be used presently.

extern OSType gSignature;

class CMIDIClient : public CObject
{

public:

	OSErr			IMIDIClient(short theIconID);
	virtual	void	Dispose(void);
#ifdef __cplusplus
	MIDIIDListHdl	GetPorts(void)
					{
						return ((midiMgrVerNum) ? MIDIGetPorts(gSignature) : (MIDIIDListHdl) 0);
					};
	Boolean			WorldChanged(void)
					{
						return ( (midiMgrVerNum) ? MIDIWorldChanged(gSignature) : FALSE );
					};
	unsigned long	GetVerNum(void)
					{
						return midiMgrVerNum;
					};
	unsigned short	GetShortVerNum(void)
					{
						return shortMidiMgrVerNum;
					};
#else
	MIDIIDListHdl	GetPorts(void);
	Boolean			WorldChanged(void);
	unsigned long	GetVerNum(void);
	unsigned short	GetShortVerNum(void);
#endif

private:

	unsigned long	midiMgrVerNum;		// Version number.  Also used to
										// determine whether signed in (if non-zero)
	unsigned short	shortMidiMgrVerNum;
};

// end of CMIDIClient.h

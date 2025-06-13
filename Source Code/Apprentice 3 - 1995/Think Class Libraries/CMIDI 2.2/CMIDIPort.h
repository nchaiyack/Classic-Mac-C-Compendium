/*
 *ÑÑÑ CMIDIPort.h ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 *	For use with Symantec C++ 6.0 and the THINK Class Library.
 *
 * Description:
 *	An abstract class for all MIDI Manager port classes.
 *
 *	Refer to the CMIDI Programmer's Manual for a complete desciption of
 *	this class.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */

#pragma once
#include "CMIDIClient.h"

class CMIDIPort : public CObject		// An abstract port, with methods and instance
{										// variables common to all port objects.
protected:								// only called by subclass initializers

	OSErr			IMIDIPort(MIDIPortParams * portParams, short bufSize);

public:

	virtual	void	Dispose(void);
	MIDIPortInfoHdl	GetPortInfo(void);
#ifdef __cplusplus
	short			GetRefNum(void) { return itsRefNum; };
	long			GetRefCon(void) { return (itsVersion ? MIDIGetRefCon(itsRefNum) : 0); };
	void			SetRefCon(long theRefCon) { if (itsVersion) MIDISetRefCon(itsRefNum, theRefCon); };
#else
	short			GetRefNum(void);
	long			GetRefCon(void);
	void			SetRefCon(long theRefCon);
#endif
	void			GetPortName(StringPtr theName);
	void			SetPortName(StringPtr theName);
	virtual	OSErr	LoadPatches(ResType theResType, short theResID);
	virtual	OSErr	SavePatches(ResType theResType, short theResID);
	void			SetConnectionProc(ProcPtr theConnectProc, long theRefCon);
	void			GetConnectionProc(ProcPtr *	theConnectProc, long * theRefCon);

protected:

	short			itsRefNum;			// From AddPort()
	OSType			itsPortID;			// Identifier in MIDI Manager
	OSErr			itsResult;			// Used by SavePatches()
	unsigned short	itsVersion;			// Save upper word of client version
};

// end of CMIDIPort.h
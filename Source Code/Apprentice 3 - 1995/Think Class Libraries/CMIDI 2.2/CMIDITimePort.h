/*
 *ÑÑÑ CMIDITimePort.h ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 * Copyright © Paul Ferguson, 1990-1994.  All rights reserved.
 *
 * For use with Symantec C++ 6.0 and the THINK Class Library.
 *
 * Description:
 *	This class defines a MIDI Manager time port object.
 *
 *	Refer to the CMIDI Programmer's Manual for a complete desciption of
 *	this class.
 *ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
 */
#pragma once
#include "CMIDIPort.h"

#define kNoClient	'    '				// The UnClient

class CMIDITimePort : public CMIDIPort
{

public:

	OSErr			IMIDITimePort(
						StringPtr	theName, 		// Pascal format, 31 char max
						OSType		thePortID,		// Used when signing into MM
						Boolean		theVisibleFlag,
						short		theFormat);		// Initial time format

	virtual	OSErr	LoadPatches(ResType theResType, short theResID);
	short			GetSync(void);
	void			SetExternalSync(void);
	void			SetInternalSync(void);
	short			UpdateSync(void);
#ifdef __cplusplus
	long			GetCurTime(void)			{ return (itsVersion ? MIDIGetCurTime(itsRefNum) : 0); };
	void			SetCurTime(long theTime)	{ if (itsVersion) MIDISetCurTime(itsRefNum, theTime); };
	void			StartTime(void)				{ if (itsVersion) MIDIStartTime(itsRefNum); };
	void			StopTime(void)				{ if (itsVersion) MIDIStopTime(itsRefNum); };
	long			GetOffsetTime(void)			{ return (itsVersion ? MIDIGetOffsetTime(itsRefNum) : 0); };
	void			SetOffsetTime(long theOffset) { if (itsVersion) MIDISetOffsetTime(itsRefNum, theOffset); };
	void			WakeUp(long theBaseTime, long thePeriod, ProcPtr theTimeProc) { if (itsVersion) MIDIWakeUp(itsRefNum, theBaseTime, thePeriod, theTimeProc); };
#else
	long			GetCurTime(void);
	void			SetCurTime(long theTime);
	void			StartTime(void);
	void			StopTime(void);
	long			GetOffsetTime(void);
	void			SetOffsetTime(long theOffset);
	void			WakeUp(long theBaseTime, long thePeriod, ProcPtr theTimeProc);
#endif
	void			SetConnection(ProcPtr theConnectionProc);
	virtual	void	Perform(long * maxSleep);		// Makes this object a CChore
};

// end of CMIDITimePort.h

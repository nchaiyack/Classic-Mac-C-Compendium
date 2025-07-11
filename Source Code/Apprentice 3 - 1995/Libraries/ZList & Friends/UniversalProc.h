#pragma once

/*
	UniversalProc.h
	�1995 Chris K. Thomas.  All Rights Reserved.
*/

struct UniversalProc
{
	UniversalProcPtr	us;

	UniversalProc(ProcPtr proc, ProcInfoType procInfo)
	{
		us = NewRoutineDescriptor(proc, procInfo, GetCurrentISA());
	}
	
	~UniversalProc()
	{
		DisposeRoutineDescriptor(us);
	}
	
	operator UniversalProcPtr()
	{
		return us;
	}

#if !GENERATINGPOWERPC	
	operator ControlActionUPP()
	{
		return (ControlActionUPP)us;
	}
#endif
};
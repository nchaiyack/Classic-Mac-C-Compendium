/*
 *ΡΡΡ CMIDIDataPort.h ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 * Copyright © Paul Ferguson, 1990-94.  All rights reserved.
 *
 * For use with Symantec C++ 6.0 and the THINK Class Library.
 *
 * Description:
 *	An abstract class for MIDI Manager Input and Output port classes.
 *
 *	Refer to the CMIDI Programmer's Manual for a complete desciption of
 *	this class.
 *ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
 */
#pragma once
#include "CMIDIPort.h"

class CMIDIDataPort : public CMIDIPort
{

public:

	virtual OSErr	LoadPatches(ResType theResType, short theResID);

	short			GetTCFormat(void);
	void			SetTCFormat(short theFormat);
};

// end of CMIDIDataPort.h
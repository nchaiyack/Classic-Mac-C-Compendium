/*
** File:		Failure.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright � 1995 Nikol Software
** All rights reserved.
*/


#include "StringUtils.h"
#include <stdio.h>
#include <stdarg.h>
#include "Failure.h"
#include "error.h"



//==================================================================




void showFailure(ConstStr255Param unixStr)
{
	Str255		macStr;
	short		i;

	// Disgusting hackery to convert LFs to CRs for the Mac error string.

	macStr[0] = unixStr[0];

	for (i = 1; i <= unixStr[0]; ++i)
		macStr[i] = (unixStr[i] == 10) ? 13 : unixStr[i];

	SetCursor(&qd.arrow);
	ParamText(macStr, nil, nil, nil);
	Alert(32765, nil);
}



void fatal_error(char *fmt, ...)
{
	va_list	args;
	char	buf[256];

	va_start(args, fmt);
	buf[0] = vsprintf(buf+1, fmt, args);
	va_end(args);

	showFailure((StringPtr)buf);
	ExitToShell();
}


void Fail(OSErr ErrorID, char *FileName, short LineNum, Boolean Fatal)
{
	Str255	theErrNum,theLineNum;
	
	NumToString(LineNum, theLineNum);
	NumToString(ErrorID, theErrNum);
	
	c2p(FileName);
	
	SetCursor(&qd.arrow);
	ParamText(theErrNum, theLineNum, (StringPtr)FileName, "\p");
	if ( Fatal )
		{
		StopAlert(912, nil);
		ExitToShell();
		}
	else
		StopAlert(911, nil);
}


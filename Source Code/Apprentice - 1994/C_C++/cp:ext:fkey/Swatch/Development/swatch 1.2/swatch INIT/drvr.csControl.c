/**

	csControl.c
	Copyright (c) 1990, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 ** Headers
 **
 **/

#include "drvr.globals.h"
#include "drvr.csControl.h"


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

OSErr csControl( register CntrlParam *cpb, DCtlPtr dce )
{
	register OSErr err;

	err = noErr;
	switch (cpb->csCode) {
	case csGetGlobals:
		cpb->ioNamePtr = (StringPtr) &Globals;
		break;

	default :
		break;
	}

	return err;
}

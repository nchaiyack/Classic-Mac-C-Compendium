/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"InitCursorBalloon.h"
#include	"CursorBalloon.h"

#if	!defined(FabNoSegmentDirectives)
#pragma segment Init
#endif

/* Initializes the manager; should be called after initializing QuickDraw, etc.
should be put into the initialization segment of the application */

OSErr InitCursorBalloonManager(void)
{
register OSErr err = appMemFullErr;

mouseRgn = NewRgn();
wideOpenRgn = NewRgn();
if (mouseRgn && wideOpenRgn) {
	ForceMouseMovedEvent();
	SetRectRgn(wideOpenRgn, -32765, -32765, 32765, 32765);
	err = noErr;
	}
return err;
}


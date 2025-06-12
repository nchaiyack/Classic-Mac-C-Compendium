/*********************************************************************
 * UtilFuncs.c
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/

#include "HexEdit.h"

void SetControl(DialogPtr dial, int item, int value)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	SetCtlValue((ControlHandle) handle,value);
}

// Copy a pascal string to a dialog's text item (static text or edit text)
//
void SetText(DialogPtr dial, int item, StringPtr text)
{
	Handle	handle;
	short	type;
	Rect	r;

	GetDItem(dial,item,&type,&handle,&r);
	SetIText(handle,text);
}

// Copy a dialog's text item to a pascal string.
//
void GetText(DialogPtr dial, int item, StringPtr text)
{
	Handle	handle;
	short	type;
	Rect	r;

	GetDItem(dial,item,&type,&handle,&r);
	GetIText(handle,(StringPtr) text);
}

void MyOutlineButton(DialogPtr dp, int bid, Pattern pat)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;

	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	PenSize(3,3);
	PenPat(pat);
	InsetRect(&r,-4,-4);
	FrameRoundRect(&r,16,16);
	PenNormal();
	SetPort(gp);
}

// Gray out a disabled button
//
void MyDisableButton(DialogPtr dp, int bid)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;
	
	MyOutlineButton(dp,bid,white);
	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	HiliteControl((ControlHandle) h, 255);
	PenPat(gray);
	PenMode(patBic);
	PaintRect(&r);
	PenNormal();
	SetPort(gp);
}

// Restore a previously disabled button
//
void MyEnableButton(DialogPtr dp, int bid)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;
	
	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	HiliteControl((ControlHandle) h,0);
	InvalRect(&r);
	SetPort(gp);
}

// Simulate the user pressing a button.  This is used to give the user some
// visual feedback when they use the keyboard as a shortcut to press a dialog button.
//
void MySimulateButtonPress(DialogPtr dp, int bid)
{
	Handle	h;
	short	t;
	Rect	r;
	GrafPtr	gp;
	long	dmy;

	GetPort(&gp);
	SetPort(dp);
	GetDItem(dp,bid,&t,&h,&r);
	InvertRoundRect(&r,4,4);
	Delay(10,&dmy);
	InvertRoundRect(&r,4,4);
	Delay(10,&dmy);
	SetPort(gp);
}

short ErrorAlert(short severity, char *str,...)
{
	char 	tbuf[128];
	short	itemHit;
	va_list args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);
	CtoPstr(tbuf);
	ParamText((StringPtr) tbuf,"\p","\p","\p");
	InitCursor();
	switch (severity) {
	case ES_Message:
		itemHit = Alert(StdMessageALRT,NULL);
		break;
	case ES_Note:
		itemHit = NoteAlert(StdErrorALRT,NULL);
		break;
	case ES_Caution:
		itemHit = CautionAlert(StdErrorALRT,NULL);
		break;
	case ES_Stop:
		itemHit = StopAlert(StdErrorALRT,NULL);
		break;
	}

	if (itemHit == 2)
		DebugStr((StringPtr) tbuf);

	if (severity == ES_Stop)
		ExitToShell();

	return itemHit;
}

short OSErrorAlert(short severity, char *str, short oe)
{
	return ErrorAlert(severity, "OS Error in %s: %d", str, oe);
}

short MyRandom(short limit)
{
	unsigned long r;
	r = (unsigned) Random();
	r = (r*(long)limit)/65536L;
	return (short) r;
}

void MySetCursor(short n)
{
	extern Cursor gWatchCursor,gIBeamCursor;
	static short lastN=-1;
	if (n != lastN) {
		lastN = n;
		switch (n) {
		case C_Arrow:	SetCursor(&arrow);			break;
		case C_Watch:	SetCursor(&gWatchCursor);	break;
		case C_IBeam:	SetCursor(&gIBeamCursor);	break;
		}
	}
}
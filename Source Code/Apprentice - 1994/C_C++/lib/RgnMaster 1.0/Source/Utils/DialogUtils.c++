#include "DialogUtils.h"

void PushButton(DialogPtr theDialog, short theItem) {
	HiliteControl((ControlHandle)GetDItemHdl(theDialog, theItem), inButton);
	Delay(7, nil);
	HiliteControl((ControlHandle)GetDItemHdl(theDialog, theItem), 0);
} // END PushButton

void GetDItemRect(DialogPtr theDialog, short theItem, Rect *theRect) {
	Handle itemH; short itemT;
	GetDItem(theDialog, theItem, &itemT, &itemH, theRect);
} // END GetDItemRect

void SetDItemText(DialogPtr theDialog, short theItem, Str255 iText) {
	Handle itemH; short itemT; Rect itemR;
	GetDItem(theDialog, theItem, &itemT, &itemH, &itemR);
	SetIText(itemH, iText);
} // END SetDItemText

void SetUserProc(DialogPtr theDialog, short theItem, ProcPtr theProc) {
	Handle itemH; short itemT; Rect itemR;
	GetDItem(theDialog, theItem, &itemT, &itemH, &itemR);
	SetDItem(theDialog, theItem, itemT, (Handle)theProc, &itemR);
} // END SetUserProc;

pascal void FrameItemRect(DialogPtr theDialog, short theItem) {
	Rect itemRect;
	Pattern savePat;
	GrafPtr savePort;

	GetPort(&savePort);
	BlockMove(&theDialog->pnPat, &savePat, sizeof(Pattern));

	SetPort(theDialog);	// Important!
	GetDItemRect(theDialog, theItem, &itemRect);
	PenPat(black);
	FrameRect(&itemRect);
	PenPat(savePat);
	SetPort(savePort);
} // END FrameItemRect

pascal void FrameGrayRect(DialogPtr theDialog, short theItem) {
	Rect itemRect;
	Pattern savePat;
	GrafPtr savePort;

	GetPort(&savePort);
	BlockMove(&theDialog->pnPat, &savePat, sizeof(Pattern));

	SetPort(theDialog);	// Important!
	GetDItemRect(theDialog, theItem, &itemRect);
	PenPat(gray);
	FrameRect(&itemRect);
	PenPat(savePat);
	SetPort(savePort);
} // END FrameGrayRect

Handle GetDItemHdl(DialogPtr theDialog, short theItem) {
	Handle itemH; short itemT; Rect itemR;
	GetDItem(theDialog, theItem, &itemT, &itemH, &itemR);
	return(itemH);
} // END GetItemHdl

void OutlineDefaultButton(DialogPtr theDialog, short theItem) {
	Rect buttonRect;
	GrafPtr savePort;

	GetPort(&savePort);
	SetPort(theDialog);
	GetDItemRect(theDialog, theItem, &buttonRect);

	InsetRect(&buttonRect, -4, -4);
	PenSize(3, 3);
	FrameRoundRect(&buttonRect, 16, 16);
	PenNormal();
	SetPort(savePort);
} // END OutlineDefaultButton
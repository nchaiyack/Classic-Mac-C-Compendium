/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"UtilsSys7.h"
#include	"Globals.h"
#include	"Animation.h"
#include	"ControlStore.h"
#include	"DoEditDialog.h"
#include	"DoMenu.h"
#include	"Main.h"
#include	"Registers.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

GrafPtr	offScr;		/* offscreen GrafPort for animation */
PicHandle	images[kLAST_PICT - kFIRST_PICT + 1];	/* Handles to Pictures */
Rect	rval[kP_MIR - kP_ALATCH + kMIRSubboxes];	/* in these rects we write the values */
RgnHandle	mirUpdRgn;

static Boolean PtInObj(Point, RectPtr, short obj);

/* PtInObj: finds out if thePt is in the obj object, returning the object
Rect in neededRect */

static Boolean PtInObj(Point thePt, RectPtr neededRect, short obj)
{
register PicHandle	tempPH;

tempPH = images[obj];
if (*tempPH == nil)
	LoadResource((Handle)tempPH);
*neededRect = (*tempPH)->picFrame;
neededRect->right--;
neededRect->bottom--;
return(PtInRect(thePt, neededRect));
}

/* UpdateMir: takes care of updating the mir image in the Animation window */

void UpdateMir(un_mir newmir)
{
Str255	tempS;
GrafPtr	savePort;
register RectPtr	RectScan = &rval[kP_MIR - kP_ALATCH];

GetPort(&savePort);
SetPort(offScr);
MyNumToString((newmir).bits.amux, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.cond, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.alu, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.shift, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.mbr, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.rd, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.wr, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.mar, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.dsc, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
ShortToHexString((newmir).bits.a, tempS);
TETextBox(&tempS[4], 1, RectScan++, teJustCenter);
ShortToHexString((newmir).bits.b, tempS);
TETextBox(&tempS[4], 1, RectScan++, teJustCenter);
ShortToHexString((newmir).bits.c, tempS);
TETextBox(&tempS[4], 1, RectScan++, teJustCenter);
MyNumToString((newmir).bits.map, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustCenter);
MyNumToString((newmir).bits.addr, tempS);
TETextBox(&tempS[1], StrLength(tempS), RectScan++, teJustRight);
SetPort(gWPtr_Animation);
InvalRgn(mirUpdRgn);
SetPort(savePort);
}

/* ChangedBox: does all the housekeeping when an on-screen object is modified */

void ChangedBox(short obj)
{
Str255	tempS;
GrafPtr	savePort;

GetPort(&savePort);
SetPort(offScr);
ShortToHexString(gParts[obj], tempS);
if((obj >= kP_INCR - kFIRST_PICT)&&(obj <= kP_MPC - kFIRST_PICT)) {
	*(Byte *)(&gParts[obj]) = 0;
	MyNumToString(gParts[obj], tempS);
	}
else {
	if( obj == kP_ALU - kFIRST_PICT ) {
		register unsigned char *mslstring = (unsigned char *)&tempS + 8;
		register StringPtr ZeroandOne = *GetString(kSTR_BINALLOWED);

		*mslstring++ = (gParts[kP_ALU - kFIRST_PICT] < 0 ? *(ZeroandOne+2) : *(ZeroandOne+1));
		*mslstring++ = 13;
		*mslstring++ = (gParts[kP_ALU - kFIRST_PICT] == 0 ? *(ZeroandOne+2) : *(ZeroandOne+1));
		TextSize(9);
		TETextBox((unsigned char *)&tempS + 8, 3, &rval[kP_MSL - kFIRST_PICT], teJustCenter);
		TextSize(12);
		}
	}
TETextBox(&tempS[1], StrLength(tempS), &rval[obj], teJustCenter);
SetPort(gWPtr_Animation);
InvalRect(&rval[obj]);
if (obj == kP_ALU - kFIRST_PICT)
	InvalRect(&rval[kP_MSL - kFIRST_PICT]);
SetPort(savePort);
}

void ChangedAllBoxes(void)
{
register short	i;

for (i = kP_ALATCH - kFIRST_PICT; i <= kP_ALU - kFIRST_PICT; ChangedBox(i++))
	;
}

/* ActivateObjs: activates (blackens) a list of objects */

void ActivateObjs(const short *objs)
{
Rect	tempRect;
GrafPtr	savePort;
register short	pos;

GetPort(&savePort);
SetPort(offScr);
PenNormal();

for( ; pos = *objs++; ) {
	register PicHandle	tempPicH;
	
	tempPicH = images[pos - kFIRST_PICT];
	if( *tempPicH == nil ) /* Resource purged */
		LoadResource((Handle)tempPicH);
	tempRect = (*tempPicH)->picFrame;
	SetPort(offScr);
	DrawPicture(tempPicH, &tempRect);
	SetPort(gWPtr_Animation);
	InvalRect(&tempRect);
	}
SetPort(savePort);
}

/* DeactivateObjs: deactivates (grays) a list of objects */

void DeactivateObjs(const short *objs)
{
Rect	tempRect;
GrafPtr	savePort;
register RgnHandle	tempRgn = NewRgn();
register RgnHandle	tempRgn2 = NewRgn();
register short	pos;

GetPort(&savePort);
SetPort(offScr);
PenPat(&qd.gray);
PenMode(notPatBic);

for( ; pos = *objs++; ) {
	register PicHandle	tempPicH;
	
	tempPicH = images[pos - kFIRST_PICT];
	if( *tempPicH == nil ) /* Resource purged */
		LoadResource((Handle)tempPicH);
	tempRect = (*tempPicH)->picFrame;
	SetPort(offScr);
	DrawPicture(tempPicH, &tempRect);
	tempRect.right--;
	tempRect.bottom--;
	if(pos <= kP_MIR) {
		if(pos == kP_MIR) {
			CopyRgn(mirUpdRgn, tempRgn2);
			InvertRgn(mirUpdRgn);
			}
		else
			RectRgn(tempRgn2, &rval[pos - kP_ALATCH]);
		RectRgn(tempRgn, &tempRect);
		DiffRgn(tempRgn, tempRgn2, tempRgn);
		}
	else
		RectRgn(tempRgn, &tempRect);
	PaintRgn(tempRgn);
	SetPort(gWPtr_Animation);
	InvalRect(&tempRect);
	}
DisposeRgn(tempRgn2);
DisposeRgn(tempRgn);
SetPort(savePort);
}

/* Update_Animation: handler for the update event */

void Update_Animation(WindowPtr w)
{
//Str15	tempS;

CopyBits(&offScr->portBits, &w->portBits, &offScr->portBits.bounds,
			&w->portRect, srcCopy, nil);

/*
MyNumToString(timingInfo, tempS);
MoveTo(40,40);
DrawString(&tempS);
*/
}

/* Do_Animation: someone has clicked in content ╔ */

void Do_Animation(WindowPtr /*w*/, EventRecord *theEvent)
{
//#pragma unused (w)

Rect	tempRect;
Point	myPt;
register short	code;

myPt = theEvent->where;
GlobalToLocal(&myPt);
if (PtInObj(myPt, &tempRect, code = kP_ALATCH - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_BLATCH - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_MAR - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_MBR - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_AMUX - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_ALU - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_SHIFTER - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_MMUX - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_INCR - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_MPC - kFIRST_PICT) ||
	PtInObj(myPt, &tempRect, code = kP_REGISTERS - kFIRST_PICT) ||
	(PtInObj(myPt, &tempRect, code = kP_CONTSTORE - kFIRST_PICT) && DocIsOpen) ) {
	if (TrackObject(&tempRect)) {
		if (code == kP_REGISTERS - kFIRST_PICT)
			DoMenuWindows(kMItem_Registers);
		else if (code == kP_CONTSTORE - kFIRST_PICT)
			DoMenuWindows(kMItem_Microprogram);
		else {
			(void)DoEditDialog(code, kPART_NAMES,
								(code >= kP_INCR - kFIRST_PICT) &&
									(code <= kP_MPC - kFIRST_PICT) ? kPOP_DEC : kPOP_HEX);
			UnloadSeg(DoEditDialog);
			}
		}
	}
}

/* TrackObject: like all the TrackThing in the Mac OS */

Boolean	TrackObject(RectPtr	r)
{
Point	myPt;
register Boolean	inrect;

InvertRect(r);
inrect = true;
do {
	register Boolean tempB;

	GetMouse(&myPt);
	if ((tempB = PtInRect(myPt, r)) != inrect) {
		InvertRect(r);
		inrect = tempB;
		}
	}
while( StillDown() );
if (inrect) {
	InvertRect(r);
	}
return inrect;
}

/* procedure called when closing the Animation window */

void CloseAnimation(WindowPtr w)
{
DoCloseWindow(w, kMItem_Animation);
}

#endif


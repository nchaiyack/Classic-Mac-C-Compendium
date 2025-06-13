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
#include	"Disasm.h"
#include	"DoEditDialog.h"
#include	"Globals.h"
#include	"Main.h"
#include	"Registers.h"
#include	"SimUtils.h"
#include	"Independents.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

/* CLRRECT: makes a Rect an empty Rect */
#define	CLRRECT(r)	(*(long *)&r = 0, *(long *)((Ptr)&r + 2) = 0)

ControlHandle	Ctrl_Base;
short	regLineHeight;
short	regDistFromTop;
short	regVertSepLine;
short	oldChoice;

static void Do_A_Button(ControlHandle theControl);


void Update_Registers(WindowPtr w)
{
Str255	sTemp;
register Handle	tempH;
register short i,j;

EraseRect(&w->portRect);
HLock(tempH = Get1Resource('STR#', kREG_NAMES));
for(i = 0, j = regDistFromTop; i <= kDISP_REGS - 1; i++, j += regLineHeight) {
	MoveTo(REG_LEFT + kDIST_FROMLEFT, j);
	DrawString((ConstStr255Param)GetPtrIndHString(tempH, i));
	MoveTo(REG_LEFT + regVertSepLine + kDIST_FROMVERTSEP + 1, j);
	FromNumToString(sTemp, gRegs[i], oldChoice);
	DrawString(sTemp);
	}
HUnlock(tempH);
/*  Draw the horizontal line  */
MoveTo(REG_LEFT, kREG_HORZSEPLINE);
LineTo(REG_RIGHT, kREG_HORZSEPLINE);
/*  Draw the vertical line  */
MoveTo(regVertSepLine, REG_TOP);
LineTo(regVertSepLine, REG_BOTTOM - 1);
UpdateControls(w, w->visRgn);
}

void Do_Registers(WindowPtr w, EventRecord *theEvent)
{
Rect	tempRect, oldRect, shiftingRect;
Point	myPt;
ControlHandle	theControl;
register short	code;
register Boolean	inbigrect;

myPt = theEvent->where;
GlobalToLocal(&myPt);
if (FindControl(myPt, w, &theControl) != 0) {
	if (TrackControl(theControl, myPt, (ControlActionUPP)-1L) == kInLabelControlPart)
		if ((code = GetControlValue(Ctrl_Base)) != oldChoice) {
			oldChoice = code;
			tempRect.top = REG_TOP;
			tempRect.left = REG_LEFT + regVertSepLine + 1;
			tempRect.bottom = REG_BOTTOM;
			tempRect.right = REG_RIGHT;
			InvalRect(&tempRect);
			}
	}
else {
	tempRect.top = REG_TOP;
	tempRect.left = REG_LEFT;
	tempRect.bottom = REG_BOTTOM;
	tempRect.right = REG_RIGHT;
	if (PtInRect(myPt, &tempRect)){
		CLRRECT(oldRect);
		do {
			GetMouse(&myPt);
			if (!PtInRect(myPt, &oldRect)) {
				if (inbigrect = PtInRect(myPt, &tempRect)) {
					shiftingRect.top = REG_TOP;
					shiftingRect.left = REG_LEFT;
					shiftingRect.bottom = REG_TOP + regLineHeight;
					shiftingRect.right = REG_RIGHT;
					for(code = kREG_PC;
						!(PtInRect(myPt, &shiftingRect)&&(code <= kREG_LOW8));
						code++, shiftingRect.top += regLineHeight,
						shiftingRect.bottom += regLineHeight)
							;
					InvertRect(&oldRect);
					InvertRect(&shiftingRect);
					oldRect = shiftingRect;
					}
				else {
					InvertRect(&oldRect);
					CLRRECT(oldRect);
					}
				}
			}
		while( StillDown() );
		if (inbigrect) {
			InvertRect(&oldRect);
			if(DoEditDialog(code, kREG_EXTNAMES, oldChoice)) {
				oldRect.left += regVertSepLine+1;
				InvalRect(&oldRect);
				if (code == kREG_PC)
					InvalDisasm();
				}
			UnloadSeg(DoEditDialog);
			}
		}
	}
}

/* ChangedRegister: to tell us that a register has changed its value */

void ChangedRegister(short whichreg)
{
Rect	tempRect;
GrafPtr	savePort;

GetPort(&savePort);
SetPort(gWPtr_Registers);
tempRect.top = REG_TOP + (regLineHeight * whichreg);
tempRect.left = REG_LEFT + regVertSepLine+1;
tempRect.bottom = tempRect.top + regLineHeight;
tempRect.right = REG_RIGHT;
InvalRect(&tempRect);
if (whichreg == kREG_PC)
	InvalDisasm();
SetPort(savePort);
}

/* ChangedAllRegisters: to tell us that _all_ registers have changed */

void ChangedAllRegisters(void)
{
Rect	tempRect;
GrafPtr	savePort;

GetPort(&savePort);
SetPort(gWPtr_Registers);
tempRect.top = REG_TOP;
tempRect.left = REG_LEFT + regVertSepLine+1;
tempRect.bottom = tempRect.top + (regLineHeight * kREG_ZERO);
tempRect.right = REG_RIGHT;
InvalRect(&tempRect);
InvalDisasm();
SetPort(savePort);
}

/* procedure called when closing the Registers window */

void CloseRegisters(WindowPtr w)
{
DoCloseWindow(w, kMItem_Registers);
}

void getDragRectRegs(WindowPtr w, RectPtr r)
{
*r = w->portRect;
}

#endif


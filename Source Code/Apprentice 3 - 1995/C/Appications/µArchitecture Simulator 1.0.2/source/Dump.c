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
#include	"CursorBalloon.h"
#include	"Disasm.h"
#include	"Dump.h"
#include	"DoEditDialog.h"
#include	"DoMenu.h"
#include	"Globals.h"
#include	"Main.h"
#include	"Input.h"
#include	"Registers.h"
#include	"Scroll.h"
#include	"TrackThumb.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

ControlHandle	dumpVScroll;
short	dumpLineHeight, dumpFromTop, dumpCWidMax;
static WindowPtr	curPosW;

static void NoRealTimeActionProc(ControlHandle control, short value);
static void RealTimeActionProc(ControlHandle control, short value);
static pascal void DumpActionProc(ControlHandle control, short part);
static void DrawLine(Ptr start);
static short TrackDumpObject(RectPtr r, short theLoc);

void DumpHome(void)
{
SetControlValue(dumpVScroll, GetControlMinimum(dumpVScroll));
InvalDump();
}

void DumpEnd(void)
{
SetControlValue(dumpVScroll, GetControlMaximum(dumpVScroll));
InvalDump();
}

void DumpPgUp(void)
{
DumpActionProc(dumpVScroll, kInPageUpControlPart);
}

void DumpPgDn(void)
{
DumpActionProc(dumpVScroll, kInPageDownControlPart);
}

void Activate_Dump(EventRecord *evt, WindowPtr w, Boolean becomingActive)
{
Rect	growRect;

/* the growbox needs to be redrawn on activation: */
growRect = w->portRect;
/* adjust for the scrollbars */
growRect.top = growRect.bottom - kScrollbarAdjust + 1;
growRect.left = growRect.right - kScrollbarAdjust + 1;
if (becomingActive) {
	InvalRect(&growRect);	/* we cannot avoid grow box flicker */
	if ((*dumpVScroll)->contrlVis == 0) {
		ShowControl(dumpVScroll);
		ValidRect(&(*dumpVScroll)->contrlRect);
		}
	}
else {
/* the control must be redrawn on deactivation: */
	HideControl(dumpVScroll);
	InvalRect(&growRect);
	}
} /*Activate*/

void Grow_Dump(WindowPtr w, EventRecord *event)
{
Rect	tempRect, updateRect;
register long	growResult;

tempRect.right = tempRect.left = PRCT_R(w) + 1;
tempRect.bottom = 0x7FFF;		/* set up limiting values */
tempRect.top = kMinDocDim + 10;
updateRect = w->portRect;
updateRect.top = updateRect.bottom - kScrollbarAdjust;
/* see if it really changed size */
if (growResult = GrowWindow(w, event->where, &tempRect)) {
	SizeWindow(w, LoWrd(growResult),
				(HiWrd(growResult) / dumpLineHeight) * dumpLineHeight, true);
	SizeControl(dumpVScroll, kScrollbarWidth, PRCT_B(w) - PRCT_T(w) - 13);
	if (HiWrd(growResult) > updateRect.bottom)
		/* enlarged */
		InvalRect(&updateRect);
	else /* reduced */ {
		updateRect.bottom = PRCT_B(w);
		updateRect.right = PRCT_R(w);
		updateRect.top = updateRect.bottom - kScrollbarAdjust + 1;
		updateRect.left = updateRect.right - kScrollbarAdjust + 1;
		InvalRect(&updateRect);
		}
	ValidRect(&(*dumpVScroll)->contrlRect);
	SetupDumpCtlMax(dumpVScroll);
	}
}

void Update_Dump(WindowPtr w)
{
Rect	growRect;
register RgnHandle	oldClip;

if (EmptyRgn(w->visRgn) == false) {	/* draw if updating needs to be done */
	DrawDump(w);
	oldClip = NewRgn();
	GetClip(oldClip);
	growRect = w->portRect;
	growRect.left = growRect.right - kScrollbarAdjust;
	ClipRect(&growRect);
	DrawGrowIcon(w);
	SetClip(oldClip);
	DisposeRgn(oldClip);
	UpdateControls(w, w->visRgn);
	}
}

void Do_Dump(WindowPtr w, EventRecord *event)
{
enum {
kCurPosWindow = 128
};

Rect	tempRect;
GrafPtr	savePort;
ControlHandle	control;
Point	mouse;
register unsigned long	clickAddr;
register short	part, offset, popItem;

mouse = event->where;		/* get the click position */
GlobalToLocal(&mouse);
/* see if we are in the dump area; if so, we won╒t check the controls */
tempRect = w->portRect;
tempRect.right -= kScrollbarAdjust;
if (PtInRect(mouse, &tempRect)) {
	/* handle editing click */
	if ((offset = mouse.h - dumpCWidMax * 6 - kDIST_FROMLEFT) >= 0)
		if ((offset /= dumpCWidMax) % 5 != 4) {
			tempRect.top = (mouse.v /= dumpLineHeight) * dumpLineHeight;
			tempRect.left = ((offset / 5) * 5) * dumpCWidMax + dumpCWidMax * 6 + kDIST_FROMLEFT;
			tempRect.bottom = tempRect.top + dumpLineHeight;
			tempRect.right = tempRect.left + (dumpCWidMax << 2);
			clickAddr = ((unsigned long)(GetControlValue(dumpVScroll) + mouse.v) << 4)
						+ ((offset / 5) << 1);
			if (popItem = TrackDumpObject(&tempRect, *(unsigned short *)(gMMemory + clickAddr))) {
				if (popItem == kD_Disasm) {
					SetControlValue(disasmVScroll, clickAddr >> 2);
					InvalDisasm();
					DoMenuWindows(kMItem_Disasm);
					}
				else if ((event->modifiers & optionKey)||(popItem == kD_DisasmFrom)) {
					SetControlValue(disasmVScroll, *(unsigned short *)(gMMemory + clickAddr) >> 1);
					InvalDisasm();
					DoMenuWindows(kMItem_Disasm);
					}
				else if ((event->modifiers & cmdKey)||(popItem == kD_DumpFrom)) {
					SetControlValue(dumpVScroll, *(unsigned short *)(gMMemory + clickAddr) >> 3);
					InvalDump();
					}
				else {
					if (DoEditDump((short *)(gMMemory + clickAddr), clickAddr >> 1)) {
						InvalDump();
						InvalDisasm();
						}
					UnloadSeg(DoEditDump);
					}
				}
			}
	}
else {
	part = FindControl(mouse, w, &control);
	switch ( part ) {
		case 0:		/* do nothing for viewRect case */
			break;
		case kInIndicatorControlPart:
			if (gPrefs.NeXTScroll)
				(void)TrackThumb(control, mouse, RealTimeActionProc);
			else {
				GetPort(&savePort);
				SetPort(curPosW = GetNewWindow(kCurPosWindow, nil, (WindowPtr)-1L));
				TextMode(srcCopy);
				TextFont(monaco);
				TextSize(9);
				SetPort(savePort);
				savePort = (GrafPtr)LMGetGhostWindow();
				LMSetGhostWindow(curPosW);
				ShowWindow(curPosW);
				(void)TrackThumb(control, mouse, NoRealTimeActionProc);
				DisposeWindow(curPosW);
				LMSetGhostWindow(savePort);
				InvalDump();
				}
			break;
		default:	/* clicked in an arrow, so track & scroll */
			{
			ControlActionUPP DumpActionProcUPP = NewControlActionProc(DumpActionProc);

			(void)TrackControl(control, mouse, DumpActionProcUPP);
			if (DumpActionProcUPP)
				DisposeRoutineDescriptor(DumpActionProcUPP);
			}
			break;
		}
	}
}

/* NoRealTimeActionProc: only updates the number in the little window up there */

static void NoRealTimeActionProc(ControlHandle , short value)
{
Str15	tempS;
GrafPtr	savePort;

ShortToHexString(value << 3, tempS);
GetPort(&savePort);
SetPort(curPosW);
MoveTo(kDIST_FROMLEFT,12);
DrawString(tempS);
SetPort(savePort);
}

/* RealTimeActionProc: updates all of the window contents while dragging the scroll box */

static void RealTimeActionProc(ControlHandle control, short )
{
DrawDump((*control)->contrlOwner);
}

static pascal void DumpActionProc(ControlHandle control, short part)
{
Rect	tempRect;
register WindowPtr	w;
register Ptr	addr;
register short	amount, oldAmount, newAmount, vc;
register Boolean	doScrollRect = false;

if ( part ) {		/* if it was actually in the control */
	w = (*control)->contrlOwner;
	switch ( part ) {
		case kInUpButtonControlPart:
			amount = -1;
			doScrollRect = true;
			break;
		case kInDownButtonControlPart:
			amount = 1;
			doScrollRect = true;
			break;
		case kInPageUpControlPart:
			amount = (PRCT_T(w) - PRCT_B(w)) / dumpLineHeight + 1;
			break;
		case kInPageDownControlPart:
			amount = (PRCT_B(w) - PRCT_T(w)) / dumpLineHeight - 1;
			break;
		}
	SetControlValue(control, (oldAmount = GetControlValue(control)) + amount);
	if (doScrollRect && (newAmount = oldAmount - GetControlValue(control))) {
		tempRect = w->portRect;
		tempRect.right -= kScrollbarAdjust;
		VScrollRect(&tempRect, newAmount * dumpLineHeight);
		addr = &gMMemory[(unsigned long)GetControlValue(control) << 4];
		vc = dumpFromTop;
		if (newAmount < 0) {
			vc += PRCT_B(w) - PRCT_T(w) - dumpLineHeight;
			addr += ((PRCT_B(w) - PRCT_T(w)) / dumpLineHeight - 1) << 4;
			}
		MoveTo(PRCT_L(w) + kDIST_FROMLEFT, vc);
		DrawLine(addr);
		}
	else
		DrawDump(w);
	}
} /* DumpActionProc */

void DrawDump(WindowPtr w)
{
Rect	tempRect;
GrafPtr	savePort;
Point	tempPoint;
register Ptr	addr;
register short	j;

GetPort(&savePort);
SetPort(w);
tempRect = w->portRect;
tempRect.bottom += dumpLineHeight;
addr = &gMMemory[(unsigned long)GetControlValue(dumpVScroll) << 4];
for(tempPoint.h = kDIST_FROMLEFT, j = dumpFromTop;
	tempPoint.v = j, PtInRect(tempPoint, &tempRect);
	j += dumpLineHeight, addr += 16) {

	MoveTo(PRCT_L(w) + kDIST_FROMLEFT, j);
	DrawLine(addr);
	}
SetPort(savePort);
}

/* DrawLine: draws a "line" of memory in the Dump window */

static void DrawLine(Ptr start)
{
Str63	tempS;
register short *hexPtr, *endPtr;
register Ptr	textPtr;

endPtr = (short *)(start + 16);
textPtr = ShortToHexText(PTR2MEMWORD(start), (Ptr)&tempS);
*textPtr++ = ':';
for (hexPtr = (short *)start; hexPtr < endPtr; ) {
	*textPtr++ = ' ';
	textPtr = ShortToHexText(*hexPtr++, textPtr);
	}
DrawText(&tempS, 0, 45);
}

/* SetupDumpCtlMax: sets up the CtlMax value of our scroll bar */

void SetupDumpCtlMax(ControlHandle theControl)
{
enum {
kAdjustForPleasantGrow = 3
};

register WindowPtr	wind;
register short	newmax;

wind = (*theControl)->contrlOwner;
newmax = 8191 - (PRCT_B(wind) - PRCT_T(wind) - kAdjustForPleasantGrow) / dumpLineHeight;
if (newmax != GetControlMaximum(theControl)) {
	SetControlMaximum(theControl, newmax);
	InvalDump();
	}
}

void InvalDump(void)
{
Rect	tempRect;
GrafPtr	savePort;

GetPort(&savePort);
SetPort(gWPtr_Dump);
tempRect = gWPtr_Dump->portRect;
tempRect.right -= kScrollbarWidth;
InvalRect(&tempRect);
SetPort(savePort);
}

/* TrackDumpObject: like all the TrackThing in the Mac OS, plus the popUp */

short	TrackDumpObject(RectPtr	r, short theLocation)
{
enum {
kSimpleClick = -1,
kDelay = 1 * 60 // one second
};

Str32	itemStr;
Str15	numStr;
Point	myPt;
MenuRef	tempMenu;
register long		timeout = TickCount();
register long		chosen = kSimpleClick;
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
	if (inrect == false)
		timeout = TickCount();
	if ((TickCount() - timeout) > GetDblTime()) {
		tempMenu = gPopMenu;
		if (noErr == HandToHand((Handle *)&tempMenu)) {
			GetMenuItemText(tempMenu, kD_DisasmFrom, itemStr);
			ShortToHexString(theLocation, numStr);
			PLstrcat(itemStr, numStr);
			SetMenuItemText(tempMenu, kD_DisasmFrom, itemStr);
			GetMenuItemText(tempMenu, kD_DumpFrom, itemStr);
			PLstrcat(itemStr, numStr);
			SetMenuItemText(tempMenu, kD_DumpFrom, itemStr);
			InsertMenu(tempMenu, hierMenu);
			LocalToGlobal(&myPt);
			chosen = PopUpMenuSelect(tempMenu, myPt.v, myPt.h, kD_Edit);
			DeleteMenu(kRes_Menu_PopDump);
			DisposeMenu(tempMenu);
			}
		}
	}
while( StillDown() );
if (inrect)
	InvertRect(r);
if (chosen == kSimpleClick)
	chosen = inrect ? toMenu(kRes_Menu_PopDump, kD_Edit) : 0L;
return HiWrd(chosen) ? LoWrd(chosen) : 0;
}

/* procedure called when closing the Dump window */

void CloseDump(WindowPtr w)
{
DoCloseWindow(w, kMItem_Dump);
}

void RecalcDump(FabWindowPtr w, RgnBalloonCursPtr theObj)
{
Rect	tempRect;

tempRect = ((WindowPtr)w)->portRect;
tempRect.right -= kScrollbarWidth;
RectRgn(theObj->zoneLocal, &tempRect);
}

void getDragRectDump(WindowPtr w, RectPtr r)
{
*r = w->portRect;
r->right -= kScrollbarAdjust;
}

#endif


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

#include	<stdlib.h>

#include	"UtilsSys7.h"
#include	"Assembler.h"
#include	"Disasm.h"
#include	"Globals.h"
#include	"Main.h"
#include	"Microprogram_Ed.h"
#include	"Input.h"
#include	"Registers.h"
#include	"TrackThumb.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

ControlHandle	disasmVScroll;
static WindowPtr	curPosW;
short	disasmLineHeight, disasmFromTop, disasmCWidMax;

static short *DisasmInstruction(short *inst, Ptr dest, ROpcDisasmPtr instrTbl, Size TblSize);
static void ThumbActionProc(ControlHandle control, short value);
static pascal void DisasmActionProc(ControlHandle control, short part);
static int CompareFirstLast(const void *opc1, const void *opc2);

void DisasmHome(void)
{
SetControlValue(disasmVScroll, GetControlMinimum(disasmVScroll));
InvalDisasm();
}

void DisasmEnd(void)
{
SetControlValue(disasmVScroll, GetControlMaximum(disasmVScroll));
InvalDisasm();
}

void DisasmPgUp(void)
{
DisasmActionProc(disasmVScroll, kInPageUpControlPart);
}

void DisasmPgDn(void)
{
DisasmActionProc(disasmVScroll, kInPageDownControlPart);
}

/* DisasmInstruction: disassembles an instruction, returns a pointer to
the next one. */

static short *DisasmInstruction(short *inst, Ptr dest, ROpcDisasmPtr instrTbl, Size TblSize)
{
Str15	tempS;
register ROpcDisasmPtr	found;
register char *tempcPtr;
register short *newpos;
register short	theOperand;

newpos = inst + 1;	/* at least two bytes per instruction */
tempcPtr = (char *)bsearch(inst, &instrTbl->first, TblSize, sizeof(ROpcDisasm), CompareFirstLast);
if (tempcPtr) {
	tempcPtr -= ((char *)&instrTbl->first - (char *)instrTbl);
	found = (ROpcDisasmPtr)tempcPtr;
	*(OSType *)dest = found->instr;
	if (found->class != kCLASS_16_0) {	/* there is an operand */
		/* extra word ? */
		theOperand = ((found->class == kCLASS_16_16) || (found->class == kCLASS_16_16_REL)) ?
						*newpos++ : *inst & (found->last - found->first);
		switch (found->class) {	/* sign extensions for 12 & 11 bit operands */
			case kCLASS_4_12:
			case kCLASS_4_12_REL:
				theOperand <<= 4;
				theOperand >>= 4;
				break;
			case kCLASS_5_11:
				theOperand <<= 5;
				theOperand >>= 5;
				break;
			}
		if (found->class == kCLASS_4_12)
			ShortToHexString(theOperand, tempS);
		else
			MyNumToString(theOperand, tempS);
		tempcPtr = dest + 5;
		if (found->class == kCLASS_4_12)	/* hex value */
			*tempcPtr++ = '$';
		if (found->class >= kCLASS_16_16_REL) {	/* relative jumps */
			*tempcPtr++ = '*';
			if (theOperand > 0)	/* wanna a plus sign before positive values */
				*tempcPtr++ = '+';
			}
		BlockMoveData(&tempS[1], tempcPtr, tempS[0]);
		tempcPtr += tempS[0];
		if (found->class >= kCLASS_16_16_REL) {	/* relative jumps */
			*tempcPtr++ = ' ';
			*tempcPtr++ = '(';
			*tempcPtr++ = '$';
			tempcPtr = ShortToHexText(PTR2MEMWORD(newpos) + theOperand, tempcPtr);
			*tempcPtr++ = ')';
			}
		}
	}
else {
	*(OSType *)dest = 'ILL*';
	}
return newpos;
}


/* Activate_Disasm: handles activate events */

void Activate_Disasm(EventRecord *evt, WindowPtr w, Boolean becomingActive)
{
Rect	growRect;

/* the growbox needs to be redrawn on activation: */
growRect = w->portRect;
/* adjust for the scrollbars */
growRect.top = growRect.bottom - kScrollbarAdjust + 1;
growRect.left = growRect.right - kScrollbarAdjust + 1;
if (becomingActive) {
	InvalRect(&growRect);	/* we cannot avoid grow box flicker */
	if ((*disasmVScroll)->contrlVis == 0) {
		ShowControl(disasmVScroll);
		ValidRect(&(*disasmVScroll)->contrlRect);
		}
	}
else {
/* the control must be redrawn on deactivation: */
	HideControl(disasmVScroll);
	InvalRect(&growRect);
	}
} /*Activate*/

/* Grow_Disasm: handles resizing */

void Grow_Disasm(WindowPtr w, EventRecord *event)
{
Rect	tempRect, updateRect;
register long	growResult;

tempRect.right = tempRect.left = PRCT_R(w) + 1;
tempRect.bottom = 0x7FFF;		/* set up limiting values */
tempRect.top = kMinDocDim;
updateRect = w->portRect;
updateRect.top = updateRect.bottom - kScrollbarAdjust;
/* see if it really changed size */
if (growResult = GrowWindow(w, event->where, &tempRect)) {
	SizeWindow(w, LoWrd(growResult),
				(HiWrd(growResult) / disasmLineHeight) * disasmLineHeight, true);
	SizeControl(disasmVScroll, kScrollbarWidth, PRCT_B(w) - PRCT_T(w) - 13);
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
	ValidRect(&(*disasmVScroll)->contrlRect);
	SetupDisasmCtlMax(disasmVScroll);
	}
}

/* Update_Disasm: handles window updates. */

void Update_Disasm(WindowPtr w)
{
Rect	growRect;
register RgnHandle	oldClip;

if (EmptyRgn(w->visRgn) == false) {	/* draw if updating needs to be done */
	DrawDisasm(w);
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

/* Do_Disasm: we handle mouse clicks in scrollbars */

void Do_Disasm(WindowPtr w, EventRecord *event)
{
enum {
kCurPosWindow = 128
};

Rect	tempRect;
GrafPtr	savePort;
ControlHandle	control;
Point	mouse;
register short	part;

mouse = event->where;		/* get the click position */
GlobalToLocal(&mouse);
/* see if we are in the disasm area; if so, we won╒t check the controls */
tempRect = w->portRect;
tempRect.right -= kScrollbarAdjust;
if (PtInRect(mouse, &tempRect)) {
	/* handle editing click */
/*	if ((offset = mouse.h - disasmCWidMax * 6 - kDIST_FROMLEFT) >= 0)
		if ((offset /= disasmCWidMax) % 5 != 4) {
			tempRect.top = (mouse.v /= dumpLineHeight) * dumpLineHeight;
			tempRect.left = ((offset / 5) * 5) * dumpCWidMax + dumpCWidMax * 6 + kDIST_FROMLEFT;
			tempRect.bottom = tempRect.top + dumpLineHeight;
			tempRect.right = tempRect.left + (dumpCWidMax << 2);
			if (TrackObject(&tempRect)) {
				clickAddr = ((unsigned long)(GetControlValue(dumpVScroll) + mouse.v) << 4)
							+ ((offset / 5) << 1);
				if (DoEditDump((short *)(mMemory + clickAddr), clickAddr >> 1))
					DrawDump();
				UnloadSeg(DoEditDump);
				}
			}*/
	}
else {
	part = FindControl(mouse, w, &control);
	switch ( part ) {
		case 0:		/* do nothing for viewRect case */
			break;
		case kInIndicatorControlPart:
			GetPort(&savePort);
			SetPort(curPosW = GetNewWindow(kCurPosWindow, nil, (WindowPtr)-1L));
			TextMode(srcCopy);
			TextFont(monaco);
			TextSize(9);
			SetPort(savePort);
			savePort = (GrafPtr)LMGetGhostWindow();
			LMSetGhostWindow(curPosW);
			ShowWindow(curPosW);
			part = TrackThumb(control, mouse, ThumbActionProc);
			DisposeWindow(curPosW);
			LMSetGhostWindow(savePort);
			InvalDisasm();
			break;
		default:	/* clicked in an arrow, so track & scroll */
			{
			ControlActionUPP DisasmActionProcUPP = NewControlActionProc(DisasmActionProc);

			part = TrackControl(control, mouse, DisasmActionProcUPP);
			if (DisasmActionProcUPP)
				DisposeRoutineDescriptor(DisasmActionProcUPP);
			}
			break;
		}
	}
}

/* ThumbActionProc: continuously called while we drag the scroll box */

static void ThumbActionProc(ControlHandle , short value)
{
Str15	tempS;
GrafPtr	savePort;

ShortToHexString(value << 1, tempS);
GetPort(&savePort);
SetPort(curPosW);
MoveTo(kDIST_FROMLEFT,12);
DrawString(tempS);
SetPort(savePort);
}

/* DisasmActionProc: called when we hold mouse button down in some
parts of the scroll bar. */

static pascal void DisasmActionProc(ControlHandle control, short part)
{
register WindowPtr	w;
register long	templong = 0L;
register short	amount, oldAmount;

if ( part ) {		/* if it was actually in the control */
	w = (*control)->contrlOwner;
	switch ( part ) {
		case kInUpButtonControlPart:
			amount = -1;
			break;
		case kInDownButtonControlPart:
			amount = 1;
			break;
		case kInPageUpControlPart:
			amount = (PRCT_T(w) - PRCT_B(w)) / (disasmLineHeight * 2) + 1;
			break;
		case kInPageDownControlPart:
			amount = (PRCT_B(w) - PRCT_T(w)) / (disasmLineHeight * 2) - 1;
			break;
		}
	SetControlValue(control, (oldAmount = GetControlValue(control)) + amount);
	if (oldAmount != GetControlValue(control))
		DrawDisasm(w);
	}
} /* DisasmActionProc */

/* DrawDisasm: redraws the entire Disassembler window */

void DrawDisasm(WindowPtr w)
{
Str27	disassembledText;
Rect	tempRect, ToBeInverted;
GrafPtr	savePort;
Handle	tH;
Point	tempPoint;
short *addr;
register long *clrptr;
register short *textPtr;
register long	spaces = 0L;
short	j, HilitePC;
SignedByte	savedState;

GetPort(&savePort);
SetPort(w);
savedState = WantThisHandleSafe(tH = Get1Resource(krInstructions, kOPCODES));
tempRect = w->portRect;
tempRect.bottom += disasmLineHeight;
spaces = GetControlValue(disasmVScroll);
addr = (short *)&gMMemory[spaces << 2];
for(tempPoint.h = kDIST_FROMLEFT, j = disasmFromTop;
	tempPoint.v = j, PtInRect(tempPoint, &tempRect);
	j += disasmLineHeight) {

	MoveTo(PRCT_L(w) + kDIST_FROMLEFT, j);
	clrptr = (long *)&disassembledText;
	spaces = '    ';
	*clrptr++ = spaces;
	*clrptr++ = spaces;
	*clrptr++ = spaces;
	*clrptr++ = spaces;
	*clrptr++ = spaces;
	*clrptr++ = spaces;
	*clrptr = spaces;
	textPtr = (short *)ShortToHexText(HilitePC = PTR2MEMWORD(addr), (Ptr)&disassembledText);
	*textPtr++ = ': ';
	addr = DisasmInstruction(addr, (Ptr)textPtr, (ROpcDisasmPtr)((*tH) + 2), *(unsigned short *)*tH + 1);

	DrawText(&disassembledText, 0L, 28L);
	if (HilitePC == gRegs[kREG_PC]) {
		ToBeInverted.top = j - disasmLineHeight + 1;
		ToBeInverted.left = PRCT_L(w) + kDIST_FROMLEFT;
		ToBeInverted.bottom = j + 1;
		ToBeInverted.right = ToBeInverted.left + disasmCWidMax * kMaxCharsInOneDisasmLine;
		LMSetHiliteMode(LMGetHiliteMode() & 0x7F);
		InvertRect(&ToBeInverted);
		}
	}
HSetState(tH, savedState);
SetPort(savePort);
}

/* SetupDisasmCtlMax: sets up the CtlMax value of our scroll bar */

void SetupDisasmCtlMax(ControlHandle theControl)
{
enum {
kAdjustForPleasantGrow = 3
};

register WindowPtr	wind;
register short	newmax;

wind = (*theControl)->contrlOwner;
newmax = SHRT_MAX - (PRCT_B(wind) - PRCT_T(wind) - kAdjustForPleasantGrow) / (disasmLineHeight << 1);
if (newmax != GetControlMaximum(theControl)) {
	SetControlMaximum(theControl, newmax);
	InvalDisasm();
	}
}

/* InvalDisasm: invalidates the entire window contents
(excluding the scrollbar) */

void InvalDisasm(void)
{
Rect	tempRect;
GrafPtr	savePort;

GetPort(&savePort);
SetPort(gWPtr_Disasm);
tempRect = gWPtr_Disasm->portRect;
tempRect.right -= kScrollbarWidth;
InvalRect(&tempRect);
SetPort(savePort);
}

/* procedure called when closing the Disasm window */

void CloseDisasm(WindowPtr w)
{
DoCloseWindow(w, kMItem_Disasm);
}

static int CompareFirstLast(const void *opc1, const void *opc2)
{
register unsigned short *tempP = (unsigned short *)opc2;
unsigned short	d1, d2;
int	d0;

d2 = *(short *)opc1;
d0 = d2;
d1 = *tempP++;
if ((d0 -= d1) > 0) {
	d0 = d2;
	d1 = *tempP;
	if ((d0 -= d1) <= 0)
		d0 = 0;
	}

return d0;
}

void getDragRectDisasm(WindowPtr w, RectPtr r)
{
*r = w->portRect;
r->right -= kScrollbarAdjust;
}


#endif


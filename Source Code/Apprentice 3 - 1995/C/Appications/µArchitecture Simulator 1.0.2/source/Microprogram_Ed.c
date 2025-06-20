/*
Copyright � 1993,1994,1995 Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
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
#include	"CursorBalloon.h"
#include	"DoMenu.h"
#include	"Main.h"
#include	"Microprogram_Ed.h"
#include	"SimAsm.h"
#include	"SoundHandling.h"
#include	"Independents.h"

#if defined(FabSystem7orlater)


enum {
kLEN_MIR = 4
};

/* Cursor keys */
enum cursorkeys {
kcursLeft = 0x1c,
kcursRight,
kcursUp,
kcursDown
};

enum microedstrs {
kSTR_ALUINPUT = 1,
kSTR_TO
};

enum {
kW_STRINGS = 259
};

enum busrequests {
kREQ_NO = 1,
kREQ_READ,
kREQ_WRITE
};

TEHandle TEs[2];
ListHandle	Lists[2];

RectPtr	keyrects[kN_RECTS];
ControlHandle	controls[kNUM_CONTROLS];
ControlHandle	RadioSelected;
short	keyDownDest;
short	theSelection[2] = { 0, 0 };
short	maxLLine[2];

static union u_mir editmir;
Boolean	arrowDrawn = false;
Boolean	draggedOnComments = false;

void ChangeAnim_mar(short);
void ChangeAnim_mbr(short);
void ChangeAnim_map(short);
void ChangeAnim_amux(short);
void ChangeAnim_alu(short new, short old);
void ChangeAnim_cond(short new, short old);
void ChangeAnim_sh(short new, short old);
void ChangeAnim_busreq(short new, short old);
void ChangeAnim_cbus(Boolean new, Boolean old);

/* ===============static prototypes=============== */
static void SettheInputTo(TEHandle thisHandle);
#define	Frame()	UnFrame()
static void UnFrame(void);
static void DoChar(short which, unsigned char theChar);
static short StartsWith(short which, unsigned char c);
static void HandleClick(TEHandle theTE, short obj);
static void ChangedComment(void);
static void ChangedBranchTo(void);
static void ResetMirAndComment(void);
static void GotDragOnComments(void);

/* =============================================== */

/* EraseArrowRect: erases the rectangle containing the arrow */

void EraseArrowRect(void)
{
Rect	tempRect;

tempRect.right = keyrects[kKEY_LIST]->left - klateralCellTweek + 1;
tempRect.top = PRCT_T(gWPtr_Microprogram_Ed);
tempRect.left = tempRect.right - karrowDistFromList - 1;
tempRect.bottom = PRCT_B(gWPtr_Microprogram_Ed);
EraseRect(&tempRect);
arrowDrawn = false;
}

/* RefreshTE: refreshes a textedit field in the window */

void RefreshTE(short whichTE)
{
Rect	tempRect = *keyrects[whichTE];
GrafPtr	savePort;

GetPort(&savePort);
SetPort(gWPtr_Microprogram_Ed);
InsetRect(&tempRect, 1, 1);
EraseRect(&tempRect);
TEUpdate(keyrects[whichTE], TEs[whichTE]);
SetPort(savePort);
}

/* SetControlsFromMir: sets up the controls to reflect the
micro instruction register */

void SetControlsFromMir(union u_mir oldmir)
{
Str255 tempS;
GrafPtr	savePort;

lastCommand = kST_STOPPED;
GetPort(&savePort);
SetPort(gWPtr_Microprogram_Ed);
SetControlValue(controls[kPOPUP_ABUS], editmir.bits.a + 1);
SetControlValue(controls[kPOPUP_BBUS], editmir.bits.b + 1);
if (editmir.bits.c != oldmir.bits.c) {
	SetControlValue(controls[kPOPUP_CBUS], (editmir.bits.dsc ? 0 : editmir.bits.c + 2));
	ChangeAnim_cbus(editmir.bits.dsc, oldmir.bits.dsc);
	}
if (editmir.bits.mar != oldmir.bits.mar) {
	SetControlValue(controls[kCHECK_MAR], editmir.bits.mar);
	ChangeAnim_mar(editmir.bits.mar);
	}
if (editmir.bits.mbr != oldmir.bits.mbr) {
	SetControlValue(controls[kCHECK_MBR], editmir.bits.mbr);
	ChangeAnim_mbr(editmir.bits.mbr);
	}
if ((1 + editmir.bits.rd + (editmir.bits.wr << 1)) !=
	(1 + oldmir.bits.rd + (oldmir.bits.wr << 1))) {
	SetControlValue(controls[kPOPUP_BUSREQ], 1 + editmir.bits.rd + (editmir.bits.wr << 1));
	ChangeAnim_busreq(1 + editmir.bits.rd + (editmir.bits.wr << 1),
						1 + oldmir.bits.rd + (oldmir.bits.wr << 1));
	}
if (editmir.bits.shift != oldmir.bits.shift) {
	SetControlValue(controls[kPOPUP_SHIFTER], editmir.bits.shift + 1);
	ChangeAnim_sh(editmir.bits.shift, oldmir.bits.shift);
	}
if (editmir.bits.cond != oldmir.bits.cond) {
	SetControlValue(controls[kPOPUP_BRANCH], editmir.bits.cond + 1);
	ChangeAnim_cond(editmir.bits.cond, oldmir.bits.cond);
	}
if (editmir.bits.alu != oldmir.bits.alu) {
	SetControlValue(controls[kPOPUP_ALU], editmir.bits.alu + 1);
	ChangeAnim_alu(editmir.bits.alu, oldmir.bits.alu);
	}
if (editmir.bits.amux != oldmir.bits.amux) {
	SetControlValue(controls[kRADIO_ABUS], 1 - editmir.bits.amux);
	SetControlValue(controls[kRADIO_MBR], editmir.bits.amux);
	RadioSelected = controls[kRADIO_ABUS + editmir.bits.amux];
	ChangeAnim_amux(editmir.bits.amux);
	}
if (editmir.bits.map != oldmir.bits.map) {
	SetControlValue(controls[kCHECK_ACTMAP], editmir.bits.map);
	ChangeAnim_map(editmir.bits.map);
	}
//if (editmir.bits.addr != oldmir.bits.addr) {
	MyNumToString(editmir.bits.addr, tempS);
	TESetText(&tempS[1], StrLength(tempS), TEs[kKEY_BRTO]);
	RefreshTE(kKEY_BRTO);
//	}
SetPort(savePort);
}

/* da schiaffare dentro la routine successiva */
const short martoActDeact[] = { kC_MAR1, kC_MAR2, kC_MAR3, kC_MAR4, 0 };
const short marifnotSubc[] = { kP_MAR, kP_BLTCH2MAR1, kP_BLTCH2MAR2, 0 };

void ChangeAnim_mar(short curVal)
{
if (curVal) {
	ActivateObjs(martoActDeact);
	if (gRstatus < kST_STEPSUBCYC)
		ActivateObjs(marifnotSubc);
	}
else {
	DeactivateObjs(martoActDeact);
	if (gRstatus < kST_STEPSUBCYC)
		DeactivateObjs(marifnotSubc);
	}
}

/* da schiaffare dentro la routine successiva */
const short mbrtoActDeact[] = { kC_MBR1, kC_MBR2, kC_MBR3, 0 };
const short mbrifnotSubc[] = { kP_MBR, kP_SH2MBR1, kP_SH2MBR2, 0 };

void ChangeAnim_mbr(short curVal)
{
if (curVal) {
	ActivateObjs(mbrtoActDeact);
	if (gRstatus < kST_STEPSUBCYC)
		ActivateObjs(mbrifnotSubc);
	}
else {
	DeactivateObjs(mbrtoActDeact);
	if (gRstatus < kST_STEPSUBCYC)
		DeactivateObjs(mbrifnotSubc);
	}
}

/* da schiaffare dentro la routine successiva */
const short maptoActDeact[] = { kP_MAP, kP_MAPREGS, 0 };

void ChangeAnim_map(short curVal)
{
if (gRstatus < kST_STEPSUBCYC)
	if (curVal)
		ActivateObjs(maptoActDeact);
	else
		DeactivateObjs(maptoActDeact);
}

/* da schiaffare dentro la routine successiva */
const short amuxtoActDeact[] = { kC_AMUX1, kC_AMUX2, kC_AMUX3, 0 };
const short amuxifnotSubc[] = { kP_AMUX, kP_MBR2AMUX, 0 };	
const short amuxtoDeact[] = { kP_ALTCH2AMUX, 0 };
const short amux2ifnotSubc[] = { kP_MBR2AMUX, 0 };
const short amuxtoAct[] = { kP_ALTCH2AMUX, kP_AMUX, 0 };

void ChangeAnim_amux(short curVal)
{
if (curVal) {
	ActivateObjs(amuxtoActDeact);
	if (gRstatus < kST_STEPSUBCYC) {
		DeactivateObjs(amuxtoDeact);
		ActivateObjs(amuxifnotSubc);
		}
	}
else {
	DeactivateObjs(amuxtoActDeact);
	if (gRstatus < kST_STEPSUBCYC) {
		ActivateObjs(amuxtoAct);
		DeactivateObjs(amux2ifnotSubc);
		}
	}
}

/* da schiaffare dentro la routine successiva */
const short alutoActDeact[] = { kC_ALU1, kC_ALU2, 0 };
const short aluifnotSubc[] = { kP_ALU, 0 };

void ChangeAnim_alu(short curVal, short oldVal)
{
if (curVal) {
	if (oldVal == 0) {
		ActivateObjs(alutoActDeact);
		if (gRstatus < kST_STEPSUBCYC)
			ActivateObjs(aluifnotSubc);
		}
	}
else {
	DeactivateObjs(alutoActDeact);
	if (gRstatus < kST_STEPSUBCYC)
		DeactivateObjs(aluifnotSubc);
	}
}

/* da schiaffare dentro la routine successiva */
const short condtoActDeact[] = { kP_MSL, kC_COND1, kC_COND2, kC_MSL2MMUX1, kC_MSL2MMUX2, 0 };

void ChangeAnim_cond(short curVal, short oldVal)
{
if (curVal) {
	if (oldVal == 0)
		ActivateObjs(condtoActDeact);
	}
else
	DeactivateObjs(condtoActDeact);
}

/* da schiaffare dentro la routine successiva */
const short shtoActDeact[] = { kC_SHFT1, kC_SHFT2, 0 };
const short shifnotSubc[] = { kP_SHIFTER, 0 };

void ChangeAnim_sh(short curVal, short oldVal)
{
if (curVal) {
	if (oldVal == 0) {
		ActivateObjs(shtoActDeact);
		if (gRstatus < kST_STEPSUBCYC)
			ActivateObjs(shifnotSubc);
		}
	}
else {
	DeactivateObjs(shtoActDeact);
	if (gRstatus < kST_STEPSUBCYC)
		DeactivateObjs(shifnotSubc);
	}
}

/* da schiaffare dentro la routine successiva */
const short busreqNOtoActDeact[] = { kP_MAR2MEM, kP_MBRMEM, 0 };
const short busreqREADtoActDeact[] = { kC_READ1, kC_READ2, kC_READ3, 0 };
const short busreqWRITEtoActDeact[] = { kC_WRITE1, kC_WRITE2, kC_WRITE3, 0 };

void ChangeAnim_busreq(short curVal, short oldVal)
{
switch(oldVal) {
	case kREQ_NO:
		if (gRstatus < kST_STEPSUBCYC)
			ActivateObjs(busreqNOtoActDeact);
		break;
	case kREQ_READ:
		DeactivateObjs(busreqREADtoActDeact);
		break;
	case kREQ_WRITE:
		DeactivateObjs(busreqWRITEtoActDeact);
	}
switch(curVal) {
	case kREQ_NO:
		if (gRstatus < kST_STEPSUBCYC)
			DeactivateObjs(busreqNOtoActDeact);
		break;
	case kREQ_READ:
		ActivateObjs(busreqREADtoActDeact);
		break;
	case kREQ_WRITE:
		ActivateObjs(busreqWRITEtoActDeact);
	}
}

/* da schiaffare dentro la routine successiva */
const short cbustoActDeact3[] = { kP_SH2REGS1, kP_SH2REGS2, kP_SH2REGS3, 0 };
const short cbustoActDeact1[] = { kC_DSC, 0 };

void ChangeAnim_cbus(Boolean curVal, Boolean oldVal)
{
if (curVal) {
	if (oldVal == false) {
		if (gRstatus < kST_STEPSUBCYC)
			DeactivateObjs(cbustoActDeact3);
		ActivateObjs(cbustoActDeact1);
		}
	}
else if (oldVal) {
	if (gRstatus < kST_STEPSUBCYC)
		ActivateObjs(cbustoActDeact3);
	DeactivateObjs(cbustoActDeact1);
	}
}

/* SettheInputTo: the input TextEdit field has changed */

static void SettheInputTo(TEHandle thisHandle)
{
if (gTheInput)
	TEDeactivate(gTheInput);
if (gTheInput = thisHandle)
	TEActivate(gTheInput);
}

/* UnFrame: handles the standard bordering for active lists */

static void UnFrame(void)
{
Rect	selectRect;
RgnHandle	insideRgn, outsideRgn;

selectRect = *keyrects[keyDownDest];
insideRgn = NewRgn();
if (insideRgn) {
	outsideRgn = NewRgn();
	if (outsideRgn) {
		RectRgn(insideRgn, &selectRect);
		InsetRect(&selectRect, -4, -4);
		RectRgn(outsideRgn, &selectRect);
		DiffRgn(outsideRgn, insideRgn, outsideRgn);
		InvalRgn(outsideRgn);
		DisposeRgn(outsideRgn);
		}
	DisposeRgn(insideRgn);
	}
}

/* HandleClick: the user has clicked in a list or editable text */

static void HandleClick(TEHandle theTE, short obj)
{
if (keyDownDest != obj) {
	if (keyDownDest >= kKEY_LIST)
		UnFrame();
	SettheInputTo(theTE);
	if ((keyDownDest = obj) >= kKEY_LIST)
		Frame();
	}
}

void Update_Microprogram_Ed(WindowPtr w)
{
PenState	curPen;
Rect	tempRect;
Rect	tempRectInstr;
WindowPtr	f = FrontWindow();
register Handle	tempH;

TextFont(systemFont);
TextSize(12);
HLock(tempH = Get1Resource('STR#', kW_STRINGS));
MoveTo(keyrects[kKEY_STRINGS]->left, keyrects[kKEY_STRINGS]->top);
DrawString((ConstStr255Param)GetPtrIndHString(tempH, kSTR_ALUINPUT - 1));
MoveTo(keyrects[kKEY_STRINGS]->right, keyrects[kKEY_STRINGS]->bottom);
DrawString((ConstStr255Param)GetPtrIndHString(tempH, kSTR_TO - 1));
HUnlock(tempH);
FrameRect(keyrects[kKEY_COMMENT]);
FrameRect(keyrects[kKEY_BRTO]);
tempRect = *keyrects[kKEY_LIST];
tempRect.right -= kScrollbarAdjust;
InsetRect(&tempRect, -1, -1);
FrameRect(&tempRect);
tempRectInstr = *keyrects[kKEY_INSTR];
tempRectInstr.right -= kScrollbarAdjust;
InsetRect(&tempRectInstr, -1, -1);
FrameRect(&tempRectInstr);

GetPenState(&curPen);
PenNormal();
PenSize(2, 2);
if (keyDownDest != kKEY_INSTR || w != f)
	PenMode(patBic);
tempRectInstr.right += kScrollbarAdjust;
InsetRect(&tempRectInstr, -3, -3);
FrameRect(&tempRectInstr);
PenMode(keyDownDest == kKEY_LIST && w == f ? patCopy : patBic);
tempRect.right += kScrollbarAdjust;
InsetRect(&tempRect, -3, -3);
FrameRect(&tempRect);
SetPenState(&curPen);

TextFont(geneva);
TextSize(9);
LUpdate(w->visRgn, Lists[kL_INSTR]);
LUpdate(w->visRgn, Lists[kL_COMMENTS]);
RefreshTE(kKEY_BRTO);
RefreshTE(kKEY_COMMENT);
UpdateControls(w, w->visRgn);
}

/* PrepareOpenMicroprogram: we hilite some objects before
showing up the window */

/* da schiaffare dentro la routine successiva */
const short toBeMoreClear[] = { kP_AMUX2ALU, kP_ALU2SH, kP_REG2LTCH1, kP_REG2LTCH2,
					kP_BLTCH2ALU, kP_CST2MIR, kC_ABC1, kC_ABC2, kC_ABC3, kC_ABC4, 0 };

void PrepareOpenMicroprogram(void)
{
if (((WindowPeek)gWPtr_Microprogram_Ed)->visible == false)
	ActivateObjs(toBeMoreClear);
}

void Activate_Microprogram_Ed(EventRecord *, WindowPtr , Boolean Do_An_Activate)
{

LActivate(Do_An_Activate, Lists[kL_COMMENTS]);
LActivate(Do_An_Activate, Lists[kL_INSTR]);
if (Do_An_Activate) {
	if (keyDownDest < kKEY_LIST)
		SettheInputTo(TEs[keyDownDest]);
	else
		Frame();
	}
else {
	if (keyDownDest >= kKEY_LIST)
		UnFrame();
	else
		SettheInputTo(nil);
	}
}

/* Microprog_TextWasModified: tells us that a text field has been modified */

void Microprog_TextWasModified(void)
{
if (gTheInput == TEs[kKEY_COMMENT])
	ChangedComment();
else if (gTheInput == TEs[kKEY_BRTO])
	ChangedBranchTo();
DocumentIsDirty(true);
}

/* ChangedComment: the comment field has changed */

static void ChangedComment(void)
{
register Handle	typedText;
register Point	tempCell;
register SignedByte	savedState;

tempCell.h = 0;
tempCell.v = theSelection[kL_COMMENTS];
savedState = WantThisHandleSafe(typedText = (Handle)TEGetText(TEs[kKEY_COMMENT]));
LSetCell(*typedText, InlineGetHandleSize(typedText), tempCell, Lists[kL_COMMENTS]);
HSetState(typedText, savedState);
}

/* ChangedBranchTo: the branch to� field has changed */

static void ChangedBranchTo(void)
{
Str255	tempS;
Size	tSize = 0L;
register union u_mir	tempMir;

GetDialogItemText(TEGetText(TEs[kKEY_BRTO]), tempS);
if (tempS[0] != 0)
	StringToNum(tempS, &tSize);
/* �stuff it into the micro instruction */
tempMir = editmir;
tempMir.bits.addr = tSize;
SetMir(tempMir.cstore);
}

/* ChangedListSelection: the selection in a list has changed */

void ChangedListSelection(Point newCell, short which, Boolean inCkLoop)
{
Str255	tempS;
GrafPtr	savePort;
register ROpcodePtr	myOpcodePtr;
Point instr;
short	length;

GetPort(&savePort);
SetPort(gWPtr_Microprogram_Ed);
if (which == kL_COMMENTS) {
	if (inCkLoop) {
/* we must update the associative memory� */
		*(long *)&instr = 0L;
		if (LGetSelect(true, &instr, Lists[kL_INSTR])) {
			register Byte i;

			myOpcodePtr = ((ROpcodePtr)(((short *)*Get1Resource(krInstructions, kOPCODES)) + 1))
							+ instr.v;
			for (i = myOpcodePtr->offsetHB; i <= myOpcodePtr->lastHB; i++)
				*(gAssMemory + i) = newCell.v;
			}
		}
/* �and update the control store memory */
/*	*(csMemory + theSelection[kL_COMMENTS]) = editmir;*/
/* get cell contents and put into TextEdit field */
	length = 255;	/* maximum length of text */
	LGetCell(&tempS, &length, newCell, Lists[kL_COMMENTS]);
	TESetText(&tempS, length, TEs[kKEY_COMMENT]);
	RefreshTE(kKEY_COMMENT);
	theSelection[which] = newCell.v;
/* load the micro instruction we must edit from control store memory */
	SetMir((*(gCsMemory + newCell.v)).cstore);
	}
else {	/* instruction list */
	myOpcodePtr = ((ROpcodePtr)(((short *)*Get1Resource(krInstructions, kOPCODES))+1))
					+ newCell.v;
	if (*(gAssMemory + myOpcodePtr->offsetHB) != theSelection[kL_COMMENTS])
/* the selected instruction is associated to a "comment" not selected */
		SelectLLine(kL_COMMENTS, *(gAssMemory + myOpcodePtr->offsetHB));
	}
theSelection[which] = newCell.v;
SetPort(savePort);
}

void Do_Microprogram_Ed(WindowPtr w, EventRecord *theEvent)
{
Rect	tempRect;
union u_mir	oldmir;
Point	myPt;
ControlHandle	theControl;
register Boolean	DoubleClick, selectionFound;

myPt = theEvent->where;
GlobalToLocal(&myPt);
/* click in comment TE */
if (PtInRect(myPt, keyrects[kKEY_COMMENT])) {
	HandleClick(TEs[kKEY_COMMENT], kKEY_COMMENT);
	TEClick(myPt, (theEvent->modifiers & shiftKey) != 0, TEs[kKEY_COMMENT]);
	}
/* click in branch to line TE */
else if (PtInRect(myPt, keyrects[kKEY_BRTO])) {
	HandleClick(TEs[kKEY_BRTO], kKEY_BRTO);
	TEClick(myPt, (theEvent->modifiers & shiftKey) != 0, TEs[kKEY_BRTO]);
	}
else {
	tempRect = *keyrects[kKEY_LIST];
	InsetRect(&tempRect, -1, -1);
/* click in comment list */
	if (PtInRect(myPt, &tempRect)) {
		HandleClick(nil, kKEY_LIST);
		if (theEvent->modifiers & cmdKey)
			SetCursor(*GetCursor(plusCursor));
		DoubleClick = LClick(myPt, theEvent->modifiers, Lists[kL_COMMENTS]);
		EraseArrowRect();
		*(long *)(&myPt) = 0L;
		selectionFound = LGetSelect(true, &myPt, Lists[kL_COMMENTS]);
		if (theEvent->modifiers & cmdKey) {
			oldmir = editmir;
			oldmir.bits.addr = myPt.v;
			SetMir(oldmir.cstore);
			DocumentIsDirty(true);
			InitCursor();
			}
		if (DoubleClick) {
			if (editmir.bits.cond)
				SelectLLine(kL_COMMENTS, editmir.bits.addr);
			}
		else {
			if (selectionFound) {
				if (myPt.v != theSelection[kL_COMMENTS])
					/* new item has been selected */
					ChangedListSelection(myPt, kL_COMMENTS, false);
				}
			}
		}
	else {
		tempRect = *keyrects[kKEY_INSTR];
		InsetRect(&tempRect, -1, -1);
/* click in instruction list */
		if (PtInRect(myPt, &tempRect)) {
			HandleClick(nil, kKEY_INSTR);
			(void)LClick(myPt, theEvent->modifiers, Lists[kL_INSTR]);
			if (draggedOnComments) {
				GotDragOnComments();
				}
			tempRect = *keyrects[kKEY_INSTR];
			tempRect.right -= kScrollbarAdjust;
			if (PtInRect(myPt, &tempRect)) {
				*(long *)(&myPt) = 0L;
				if (LGetSelect(true, &myPt, Lists[kL_INSTR]))
					/* new item has been selected */
					ChangedListSelection(myPt, kL_INSTR, draggedOnComments);
				}
			draggedOnComments = false;
			ForceMouseMovedEvent();
			}
		else {
/* click in controls */
			if (FindControl(myPt, w, &theControl)) {
				oldmir = editmir;
				switch(TrackControl(theControl, myPt, (ControlActionUPP)-1L)) {
					register short newval;
		
					case kInLabelControlPart:
						if (theControl == controls[kPOPUP_ABUS])
							editmir.bits.a = GetControlValue(theControl) - 1;
						else if (theControl == controls[kPOPUP_BBUS])
							editmir.bits.b = GetControlValue(theControl) - 1;
						else if (theControl == controls[kPOPUP_CBUS]) {
							if (editmir.bits.c != (newval = (GetControlValue(theControl)-2))) {
								ChangeAnim_cbus(newval < 0, editmir.bits.dsc);
								editmir.bits.c = newval;
								editmir.bits.dsc = newval < 0;
								}
							}
						else if (theControl == controls[kPOPUP_BUSREQ]) {
							register short newvalrd, newvalwr;
		
							newvalrd = ((newval = GetControlValue(theControl)) == kREQ_READ);
							newvalwr = (newval == kREQ_WRITE);
							if ((editmir.bits.rd != newvalrd)||(editmir.bits.wr != newvalwr)) {
								ChangeAnim_busreq(newval, 1 + editmir.bits.rd + (editmir.bits.wr << 1));
								editmir.bits.rd = newvalrd;
								editmir.bits.wr = newvalwr;
								}
							}
						else if (theControl == controls[kPOPUP_SHIFTER]) {
							if (editmir.bits.shift != (newval = (GetControlValue(theControl)-1))) {
								ChangeAnim_sh(newval, editmir.bits.shift);
								editmir.bits.shift = newval;
								}
							}
						else if (theControl == controls[kPOPUP_BRANCH]) {
							if (editmir.bits.cond != (newval = (GetControlValue(theControl)-1))) {
								ChangeAnim_cond(newval, editmir.bits.cond);
								editmir.bits.cond = newval;
								}
							}
						else if (theControl == controls[kPOPUP_ALU]) {
							if (editmir.bits.alu != (newval = (GetControlValue(theControl)-1))) {
								ChangeAnim_alu(newval, editmir.bits.alu);
								editmir.bits.alu = newval;
								}
							}
						break;
					case kInCheckBoxControlPart:
						if (theControl == controls[kRADIO_ABUS] || theControl == controls[kRADIO_MBR]) {
							if (RadioSelected != theControl) {
								SetControlValue(RadioSelected, 0);
								SetControlValue(theControl, 1);
								RadioSelected = theControl;
								ChangeAnim_amux(editmir.bits.amux = GetControlValue(controls[kRADIO_MBR]));
								}
							}
						else {	
							SetControlValue(theControl, newval = (1 - GetControlValue(theControl)));
							if (theControl == controls[kCHECK_MAR]) {
								editmir.bits.mar = newval;
								ChangeAnim_mar(newval);
								}
							else if (theControl == controls[kCHECK_MBR]) {
								editmir.bits.mbr = newval;
								ChangeAnim_mbr(newval);
								}
							else if (theControl == controls[kCHECK_ACTMAP]) {
								editmir.bits.map = newval;
								ChangeAnim_map(newval);
								}
							}
					}
				if (editmir.cstore != oldmir.cstore) {
					UpdateMir(editmir);
					/* �and update the control store memory */
					*(gCsMemory + theSelection[kL_COMMENTS]) = editmir;
					DocumentIsDirty(true);
					}
				}
			}
		}
	}
}

void Key_Microprogram_Ed(EventRecord *theEvent, Boolean ignoreChar)
{
enum {
kTabKey = 9
};

GrafPtr	savePort;
register unsigned char	ch;

GetPort(&savePort);
SetPort(gWPtr_Microprogram_Ed);
if ((ch = CHARFROMMESSAGE(theEvent->message)) == kTabKey) {
	if (keyDownDest >= kKEY_LIST)
		UnFrame();
	keyDownDest = (theEvent->modifiers & shiftKey ?
					(keyDownDest == kKEY_INSTR ? kKEY_BRTO : keyDownDest + 1) :
					(keyDownDest == kKEY_BRTO ? kKEY_INSTR : keyDownDest - 1));
	if (keyDownDest < kKEY_LIST) {
		SettheInputTo(TEs[keyDownDest]);
		TESetSelect(0, SHRT_MAX, gTheInput);
		}
	else {
		SettheInputTo(nil);
		Frame();
		}
	}
else if (gTheInput) {
	if (ignoreChar == false)
		TEKey(ch, gTheInput);
	if ((ch < kcursLeft)||(ch > kcursDown))
		Microprog_TextWasModified();
	}
else if (keyDownDest >= kKEY_LIST)
	DoChar(keyDownDest - 2, ch);
SetPort(savePort);
}

static void DoChar(short item, unsigned char theChar)
{
register short	newSelect;

/* Take the char, and find the first line that starts with this char. */
if ((newSelect = StartsWith(item, theChar)) != theSelection[item])
	/* We've moved. Unhighlight the old. */
	SelectLLine(item, newSelect);
}

static short StartsWith(short elem, unsigned char c)
{
/* Check for cursor up, down. */
if (c == kcursDown) {
	/* Go up one, clamp at the end. */
	return ((theSelection[elem] == maxLLine[elem]) ? theSelection[elem] :
				theSelection[elem] + 1);
	}
if (c == kcursUp) {
	/* Go down one, clamp at the bottom. */
	return((theSelection[elem] == 0) ? 0 : theSelection[elem] - 1);
	}
return (maxLLine[elem]);
}

/* ResetMirAndComment: reset the micro instruction register and comment field */

static void ResetMirAndComment(void)
{
short	null = 0;

TESetText(&null, 0, TEs[kKEY_COMMENT]);
RefreshTE(kKEY_COMMENT);
ChangedComment();
SetMir(0L);
DocumentIsDirty(true);
}

/* SendClipMsg: handle a clipboard request from the user */

OSErr SendClipMsg(short msg)
{
unsigned long	dataOffset;
register long	length;
register Handle	typedText;
register SignedByte	savedState;

switch (msg) {
	case kCLIPMSG_CUT:
		if ((length = ZeroScrap()) == noErr)
			if ((length = PutScrap(kLEN_MIR, kFCR_MINE, (Ptr)&editmir)) == noErr) {
				savedState = WantThisHandleSafe(typedText = (Handle)TEGetText(TEs[kKEY_COMMENT]));
				length = PutScrap(InlineGetHandleSize(typedText), 'TEXT', *typedText);
				HSetState(typedText, savedState);
				if (length == noErr) {
					(void)TEFromScrap();
					ResetMirAndComment();
					}
				}
		break;
	case kCLIPMSG_COPY:
		if ((length = ZeroScrap()) == noErr)
			if ((length = PutScrap(kLEN_MIR, kFCR_MINE, (Ptr)&editmir)) == noErr) {
				savedState = WantThisHandleSafe(typedText = (Handle)TEGetText(TEs[kKEY_COMMENT]));
				length = PutScrap(InlineGetHandleSize(typedText), 'TEXT', *typedText);
				HSetState(typedText, savedState);
				(void)TEFromScrap();
				}
		break;
	case kCLIPMSG_PASTE:
		typedText = NewHandle(0);
		if ((length = GetScrap(typedText, kFCR_MINE, (long *)&dataOffset)) == kLEN_MIR) {
			SetMir(*(unsigned long *)*typedText);
			DocumentIsDirty(true);
			if ((length = GetScrap(typedText, 'TEXT', (long *)&dataOffset)) >= 0L) {
				savedState = WantThisHandleSafe(typedText);
				TESetText(*typedText, length, TEs[kKEY_COMMENT]);
				HSetState(typedText, savedState);
				RefreshTE(kKEY_COMMENT);
				ChangedComment();
				}
			}
		DisposeHandle(typedText);
		break;
	case kCLIPMSG_CLEAR:
		ResetMirAndComment();
		length = 0L;
		break;
	}
return(((length < 0) && (length != noTypeErr)) ? length : noErr);
}

/* SelectLLine: selects a specified line in a list */

void SelectLLine(short whichList, short newSelect)
{
Point	cell;

cell.h = 0;
cell.v = theSelection[whichList];
LSetSelect(false, cell, Lists[whichList]);
cell.v = newSelect;
LSetSelect(true, cell, Lists[whichList]);
LAutoScroll(Lists[whichList]);
ChangedListSelection(cell, whichList, false);
}

/* SetMir: calls UpdateMir and updates the document (Microprogram Editor) window */

void SetMir(unsigned long newmir)
{
union u_mir	oldmir;

oldmir = editmir;
editmir.cstore = newmir;
UpdateMir(editmir);
/* �and update the control store memory */
*(gCsMemory + theSelection[kL_COMMENTS]) = editmir;
SetControlsFromMir(oldmir);
}


/* procedure called when closing the MPrg window */

void CloseMicroProg(WindowPtr w)
{
DoCloseWindow(w, kMItem_Microprogram);
}

void GotDragOnComments(void)
{
Rect	tempRect;
Point	curMouse;
register ListHandle	theList;
register ListClickLoopUPP	savedLCkLoopUPP;
register Boolean	wasLinked;

tempRect = *keyrects[kKEY_LIST];
tempRect.right -= kScrollbarAdjust;
SetCursor(*GetCursor(kSheetsCursor));
GetMouse(&curMouse);
theList = Lists[kL_COMMENTS];
savedLCkLoopUPP = (*theList)->lClickLoop;
(*theList)->lClickLoop = gSwitchCursClikLoopUPP;

(void)LClick(curMouse, gMyEvent.modifiers, theList);
(*theList)->lClickLoop = savedLCkLoopUPP;
GetMouse(&curMouse);
wasLinked = PtInRect(curMouse, &tempRect);
*(long *)&curMouse = 0L;
if(LGetSelect(true, &curMouse, theList)) {
	if (wasLinked) {
		SetCursor(*GetCursor(kLinkedSheetsCursor));
		DoSound(kSndPinzatrice);
		DocumentIsDirty(true);
		ChangedListSelection(curMouse, kL_COMMENTS, true);
		}
	else {
		LSetSelect(false, curMouse, theList);
		curMouse.h = 0;
		curMouse.v = theSelection[kL_COMMENTS];
		LSetSelect(true, curMouse, theList);
		SysBeep(30);
		}
	}
InitCursor();
}

#endif


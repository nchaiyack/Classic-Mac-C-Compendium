/*
Copyright � 1993,1994,1995 Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#include	"ControlStore.h"
#include	"Globals.h"
#include	"Microprogram_Ed.h"
#include	"SimAsm.h"


Boolean instrClikLoop(void)
{
Rect tempRect = *keyrects[kKEY_LIST];
Point	tempPt;

tempRect.right -= kScrollbarAdjust;
GetMouse(&tempPt);
draggedOnComments = PtInRect(tempPt, &tempRect);

#if GENERATINGCFM
return draggedOnComments;
#else
return draggedOnComments == false;
#endif
}

Boolean commentClikLoop(void)
{
union u_mir tmpmir;
Rect	clkRect, destRect;
Point	curPt;
register short	cellMid, temp;

curPt = LLastClick(Lists[kL_COMMENTS]);
tmpmir = *(gCsMemory + curPt.v);
if (tmpmir.bits.cond) {
	LRect(&clkRect, curPt, Lists[kL_COMMENTS]);
	GetMouse(&curPt);
	if (PtInRect(curPt, &clkRect)) {
		if (arrowDrawn == false) {
			cellMid = (clkRect.top + clkRect.bottom) >> 1;
			temp = clkRect.left - klateralCellTweek;
			MoveTo(temp, cellMid);
			temp -= karrowDistFromList;
			LineTo(temp, cellMid);
			curPt.h = 0;
			curPt.v = tmpmir.bits.addr;
			LRect(&destRect, curPt, Lists[kL_COMMENTS]);
			cellMid = (destRect.top + destRect.bottom) >> 1;
			LineTo(temp, cellMid);
			temp += karrowDistFromList;
			LineTo(temp, cellMid);
			LineTo(temp, cellMid);
			LineTo(temp - karrowHead, cellMid - karrowHead);
			MoveTo(temp, cellMid);
			LineTo(temp - karrowHead, cellMid + karrowHead);
			arrowDrawn = true;
			}
		}
	else if (arrowDrawn) {
		EraseArrowRect();
//		UnloadSeg(EraseArrowRect);
		}
	}
return kListClickLoopTrue;
}

#if defined(powerc) || defined (__powerc)

Boolean switchCursClikLoop(void)
{
Rect tempRect = *keyrects[kKEY_LIST];
Point	tempPt;
Boolean d7;

tempRect.right -= kScrollbarAdjust;
GetMouse(&tempPt);
d7 = PtInRect(tempPt, &tempRect);
if (d7 != draggedOnComments) {
	SetCursor(d7 ? *GetCursor(kSheetsCursor) : &qd.arrow);
	draggedOnComments = d7;
	}

return kListClickLoopTrue;
}

#else

asm Boolean switchCursClikLoop(void)
{
	LINK	A6,#-8
	MOVEA.L	keyrects+(kKEY_LIST*4),A0
	LEA		-8(A6),A1
	MOVE.L	(A0)+,(A1)+
	MOVE.L	(A0),(A1)+
	SUBI.W	#kScrollbarAdjust,-(A1)

	MOVE.L	D7,-(SP)
	SUBQ.L	#2,SP
	MOVE.L	D5,-(SP)	//we must not overwrite D5 (contains mouse location)
	PEA		-8(A6)
	_PtInRect
	MOVE.B	(SP)+,D7
	CMP.B	draggedOnComments,D7
	BEQ.S	exitasm
	
	TST.B	D7
	BEQ.S	restoreplus
	
	SUBQ.L	#4,SP
	MOVE.W	#kSheetsCursor,-(SP)
	_GetCursor
	MOVEA.L	(SP),A0
	MOVE.L	(A0),(SP)
	_SetCursor
	BRA.S	goodexit
restoreplus:
	PEA		qd.arrow
	_SetCursor
goodexit:
	MOVE.B	D7,draggedOnComments
exitasm:
	MOVE.L	(SP)+,D7
	MOVEQ	#1,D0
//	MOVE.B	D0,8(A6)	//	true	(actually is not needed)!
	
	UNLK	A6
	RTS
}

#endif

// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// XTypes.cp

#include "XTypes.h"
#include "Tools.h"

#include <stdio.h>

#pragma segment MyTools

void CopyHolToCStr255(HandleOffsetLength hol, CStr255 &text)
{
#if qDebug
	if (hol.fLength < 0 || hol.fLength > 255)
	{
		fprintf(stderr, "Invalid length (%ld) for hol->CStr255 conversion\n", hol.fLength);
		ProgramBreak(gEmptyString);
	}
#endif
	text.Length() = short(Min(250, hol.fLength));
	if (text.Length())
		BytesMove(*hol.fH + hol.fOffset, &text[1], text.Length());
}



// old select stuff 
	
/*
	CSelectionRange sr(theRange);
	sr.Sort();
	if (sr.fStart.IsZero() || !Focus())
		return;
	SetPortTextStyle(fGridViewTextStyle.fTextStyle);
	PenNormal();
	VQD vqd(this);
	VPoint startVP, endVP;
	SelectionPointToViewPt(sr.fStart, startVP);
	SelectionPointToViewPt(sr.fEnd, endVP);
	VCoordinate inset = (changeHL == hlOnDim) ? 1 : 0;
	if (theRange.IsEmpty())
	{
		vqd.VMoveTo(startVP.h, startVP.v + inset);
		vqd.VInvertLineToV(startVP.v + fGridViewTextStyle.fRowHeight - inset - 1);
		return;
	}
	if (sr.fEnd.fCharOffset == 0 && sr.fEnd.fLineNum > sr.fStart.fLineNum)
	{
#if qDebugHighlight
		fprintf(stderr, "Last line empty, keeps previous completely\n");
#endif
		// If the last line is empty, delete it, but keep complete previous
		sr.fEnd.fLineNum--;
		endVP.h = 10000;
	}
	else
	{
		// add the height to it
		endVP.v += fGridViewTextStyle.fRowHeight;
	}
#if qDebugHighlight
	fprintf(stderr, "  startVP = %s, ", (char*)startVP);
	fprintf(stderr, "  endVP = %s, inset = %ld\n", (char*)endVP, long(inset));
#endif
	if (sr.fStart.fLineNum == sr.fEnd.fLineNum)
	{
		startVP.h += inset; startVP.v += inset;
		endVP.h += inset; endVP.v += inset;
		if (changeHL == hlOffDim)
		{
			PenMode(patXor);
			vqd.VFrameRect(startVP, endVP);
		}
		else
			vqd.VInvertRect(startVP, endVP);
		return;
	}
	VCoordinate height = fGridViewTextStyle.fRowHeight;
	VCoordinate height1 = height - 1;
	long numBodyLines = sr.fEnd.fLineNum - sr.fStart.fLineNum - 1;
	if (changeHL != hlOffDim)
	{
		vqd.VInvertRect(startVP, VPoint(10000, startVP.v + height));
		vqd.VInvertRect(VPoint(0, startVP.v + height), VPoint(10000, endVP.v - height));
		vqd.VInvertRect(VPoint(0, endVP.v - height), endVP);
		return;
	}
	// First Line
	if (startVP.h < fVisibleExtent.right)
	{
		vqd.VMoveTo(startVP.h, startVP.v + height1);
		vqd.VInvertLineToV(startVP.v); // left of first line
		vqd.VInvertLineToH(fVisibleExtent.right - 1); // top of first line
		vqd.VInvertLineToV(endVP.v - height - 1); // right of first line and body
	}
	// top of bottom or body
	vqd.VMoveTo(0, startVP.v + height);
	if (numBodyLines)
		vqd.VInvertLineToH(startVP.h);
	else
		vqd.VInvertLineToH(Min(startVP.h, endVP.h));
	// bottom of First line or body
	if (numBodyLines)
		vqd.VMoveTo(endVP.h, endVP.v - height - 1);
	else
		vqd.VMoveTo(Max(startVP.h, endVP.h), endVP.v - height - 1);
	vqd.VInvertLineToH(fVisibleExtent.right - 1);
	// Last Line
	vqd.VMoveTo(0, startVP.v + height);
	vqd.VInvertLineToV(endVP.v - 1); // this draws too the left line of the body
	vqd.VInvertLineToH(Min(fVisibleExtent.right, endVP.h) - 1); // bottom of line
	vqd.VInvertLineToV(endVP.v - height); // right of bottom line
*/

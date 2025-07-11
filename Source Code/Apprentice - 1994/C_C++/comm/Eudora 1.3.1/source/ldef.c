#define FILE_NUM 19
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#include <Types.h>
#include <Lists.h>
#include <Quickdraw.h>
#include <SysEqu.h>
#include <ToolUtils.h>
void ListDraw(Boolean lSelect,Rect *lRect,Cell lCell,ListHandle lHandle);
/************************************************************************
 * my list definition
 ************************************************************************/
pascal void ListDef(short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle)
{
#pragma unused(lDataOffset,lDataLen)
	switch (lMessage)
	{
		case lDrawMsg:
			ListDraw(lSelect,lRect,lCell,lHandle);
			break;
		case lHiliteMsg:
			BitClr((Ptr)HiliteMode, pHiliteBit);
			InvertRect(lRect);
			break;
	}
}

void ListDraw(Boolean lSelect,Rect *lRect,Cell lCell,ListHandle lHandle)
{
	Str63 myString;
	short junk=sizeof(myString);
	EraseRect(lRect);
	LGetCell(myString,&junk,lCell,lHandle);
	MoveTo(lRect->left+(*lHandle)->indent.h,lRect->top+(*lHandle)->indent.v);
	DrawString(myString+1);
	if (myString[0])
	{
		Point pt;
		PolyHandle pH;
		
		pt.h = lRect->right - 2;
		pt.v = (lRect->top + lRect->bottom)/2;
		MoveTo(pt.h,pt.v);
		PenNormal();
		if (pH=OpenPoly())
		{
			Line(-4,-4);
			Line(0,9);
			LineTo(pt.h,pt.v);
			ClosePoly();
			PaintPoly(pH);
			KillPoly(pH);
		}
	}
	if (lSelect)
	{
		BitClr((Ptr)HiliteMode, pHiliteBit);
		InvertRect(lRect);
	}
}

// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FontTools.cp

#include "FontTools.h"

#include <Script.h>

#pragma segment MyTools

#define qDebugLogMyGetFontInfo qDebug & 0

void CalcStandardGridViewFont(StandardGridViewTextStyle &gvts)
{
	GrafPtr savePort;
	GetPort(&savePort);
	SetPort(gWorkPort);
	SetPortTextStyle(gvts.fTextStyle);
	FontInfo info;
	GetFontInfo(&info);
	gvts.fRowHeight = info.ascent + info.descent + info.leading;
	gvts.fHorzOffset = 2 * CharWidth(' ');
	gvts.fVertOffset = info.leading + info.ascent;
	SetPort(savePort);
}

void GetFontInfo(const StandardGridViewTextStyle &gvts, FontInfo &info)
{
	GrafPtr savePort;
	GetPort(&savePort);
	SetPort(gWorkPort);
	SetPortTextStyle(gvts.fTextStyle);
	GetFontInfo(&info);
	SetPort(savePort);
}

void TruncSystemFontString(short width,CStr255& theString, short truncWhere)
{
	GrafPtr savePort;
	GetPort(&savePort);
	SetPort(gWorkPort);
	SetPortTextStyle(gSystemStyle);
	TruncString(width, theString, truncWhere);
	SetPort(savePort);	
}

MyFontInfo *MyGetFontInfo(const CStr255 &fontname)
{
	static MyFontInfo *sMyFontInfoListHeadP = nil;
	MyFontInfo *fiP = sMyFontInfoListHeadP;
	while (fiP)
	{
		if (fiP->fName == fontname)
			return fiP;
		fiP = fiP->fNext;
	}
#if qDebugLogMyGetFontInfo
	fprintf(stderr, "MyGetFontInfo(%s)\n", (char*)fontname);
#endif
	fiP = (MyFontInfo*)NewPermPtr(sizeof(MyFontInfo));
	fiP->fNext = sMyFontInfoListHeadP;
	sMyFontInfoListHeadP = fiP;
	fiP->fName = fontname;
	GetFNum(fontname, &fiP->fFontNum);
#if qDebug
	if (!fiP->fFontNum)
		fprintf(stderr, "The font '%s' is missing in this mac\n", (char*)fontname);
#endif
	fiP->fScript = Font2Script(fiP->fFontNum);
	return fiP;
}

void MATextStyle2TextStyle(const MATextStyle& mats, TextStyle &ts)
{
	ts.tsFont = MyGetFontInfo(mats.tsFont)->fFontNum;
	ts.tsFace = mats.tsFace;
	ts.tsSize = mats.tsSize;
	ts.tsColor = mats.tsColor;
}


// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FontTools.cp

#include "FontTools.h"

#include <Script.h>

#pragma segment MyTools

void CalcStandardGridViewFont(StandardGridViewTextStyle &gvts)
{
	GrafPtr savePort;
	GetPort(savePort);
	SetPort(gWorkPort);
	SetPortTextStyle(gvts.fTextStyle);
	FontInfo info;
	GetFontInfo(info);
	gvts.fRowHeight = info.ascent + info.descent + info.leading;
	gvts.fHorzOffset = 2 * CharWidth(' ');
	gvts.fVertOffset = info.leading + info.ascent;
	SetPort(savePort);
}

void TruncSystemFontString(short width,CStr255& theString, short truncWhere)
{
	GrafPtr savePort;
	GetPort(savePort);
	SetPort(gWorkPort);
	SetPortTextStyle(gSystemStyle);
	TruncString(width, theString, truncWhere);
	SetPort(savePort);	
}


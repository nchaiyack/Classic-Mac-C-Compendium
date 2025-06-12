// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FontTools.h

#define __FONTTOOLS__

typedef struct
{
	TextStyle fTextStyle;
	short fRowHeight;
	short fHorzOffset;
	short fVertOffset;
} StandardGridViewTextStyle;
		
		
void CalcStandardGridViewFont(StandardGridViewTextStyle &gvts);

void TruncSystemFontString(short width,CStr255& theString, short truncWhere);

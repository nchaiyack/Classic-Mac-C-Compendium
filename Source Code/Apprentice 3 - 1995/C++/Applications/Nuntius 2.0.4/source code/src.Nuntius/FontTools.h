// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
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
void GetFontInfo(const StandardGridViewTextStyle &gvts, FontInfo &info);

void TruncSystemFontString(short width,CStr255& theString, short truncWhere);

struct MyFontInfo
{
	MyFontInfo *fNext;
	short fFontNum;
	short fScript;
	CStr255 fName;
};
MyFontInfo *MyGetFontInfo(const CStr255 &fontname);
void MATextStyle2TextStyle(const MATextStyle& mats, TextStyle &ts);

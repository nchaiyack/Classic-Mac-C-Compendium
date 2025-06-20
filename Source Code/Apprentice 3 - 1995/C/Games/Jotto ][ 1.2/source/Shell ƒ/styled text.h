#ifndef __STYLED_TEXT_H__
#define __STYLED_TEXT_H__

typedef unsigned char	**CharHandle;

typedef struct
{
	long			offset;
	short			lineHeight;
	short			fontDescent;
	short			fontNum;
	unsigned char	fontStyle;
	unsigned char	unused1;
	short			fontSize;
	short			unused2;
	short			unused3;
	short			unused4;
} OneStyle;

typedef struct
{
	short		numStyles;
	OneStyle	theStyle[31];
} StylRec, *StylPtr, **StylHandle;

enum
{
	kLeft=0,
	kCenter
};

void DrawTheText(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect);
void DrawThePartialText(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect, short startLine, short endLinePlusOne);
short CountLines(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect);

#endif

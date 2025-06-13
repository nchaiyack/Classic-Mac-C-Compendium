#pragma once

#define kPopupMenuID		256	// STR# ID
#define kArrowPictID		256 // arrow pict ID
#define kHelpStringIndex	1	// index of the help string
#define	kIconWidth			16	// width of the ics8

#define width(rect)			((rect).right - (rect).left)	// macro for rect width
#define height(rect)		((rect).bottom - (rect).top)	// macro for rect height
#define PicFrame(what)		((**MyGlobals.what).picFrame)	// macro for picture frame rect
#define pstrcpy(dst, src)	(BlockMove((src), (dst), (src)[0] + 1))

#include <Icons.h>
#include <ControlStrip.h>

typedef struct
{
	Handle			iconSuite;
	MenuHandle		myMenuH;
	Handle			myStrings;
	PicHandle		myArrowPict;
} MyGlobals, *MyGlobalPtr, **MyGlobalHandle;

void DoCSTrack(MyGlobalHandle myGlobals, Rect *statusRect, GrafPtr statusPort);
void DoCSDraw(MyGlobalHandle myGlobals, Rect *statusRect, GrafPtr statusPort);
unsigned long DoCSTickle(MyGlobalHandle myGlobals, Rect *statusRect, GrafPtr statusPort);
void DoCSClose(MyGlobalHandle myGlobals);
long DoCSInit(void);
void DoCSClick(MyGlobalHandle myGlobals, const Rect	*statusRect);

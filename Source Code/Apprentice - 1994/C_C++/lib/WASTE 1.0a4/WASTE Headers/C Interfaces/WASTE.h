/*
 *	WASTE.h
 *
 *	C interface to the WASTE text engine
 *	version 1.0a3 -- March/April 1994
 *
 *	Copyright (c) 1993-1994 Merzwaren
 *	All Rights Reserved
 */

#ifndef _WASTE_
#define _WASTE_

#ifndef _LongCoords_
#include "LongCoords.h"
#endif

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

/*	alignment styles */

enum {
	weFlushLeft 		=	-2,		/* flush left */
	weFlushRight		=	-1,		/* flush right */
	weFlushDefault		=	 0,		/* flush according to system direction */
	weCenter			=	 1,		/* centered */
	weJustify			=	 2		/* fully justified */
};

/*	values for the mode parameter in WESetStyle and WEContinuousStyle */

enum {
	weDoFont			=	0x0001,
	weDoFace			=	0x0002,
	weDoSize			=	0x0004,
	weDoColor			=	0x0008,
	weDoAll				=	weDoFont + weDoFace + weDoSize + weDoColor,
	weDoAddSize			=	0x0010,
	weDoToggleFace		=	0x0020,
	weDoReplaceFace		=	0x0040
};

/*	values for the edge parameter in WEGetOffset etc. */

enum {
	kLeadingEdge = -1,
	kTrailingEdge = 0
};

/*	values for WEFeatureFlag feature parameter */

enum {
	weFAutoScroll		=	0,		/* automatically scroll the selection range into view */
	weFOutlineHilite	=	2,		/* frame selection when deactivated */
	weFInhibitRecal		=	9,		/* don't recalculate line starts and don't redraw text */
	weFUseTempMem		=	10,		/* use temporary memory for main data structures */
	weFDrawOffscreen	=	11		/* draw text offscreen for smoother visual results */
};

/*	values for WENew flags parameter */

enum {
	weDoAutoScroll		=	1 << weFAutoScroll,
	weDoOutlineHilite	=	1 << weFOutlineHilite,
	weDoInhibitRecal	=	1 << weFInhibitRecal,
	weDoUseTempMem		=	1 << weFUseTempMem,
	weDoDrawOffscreen	=	1 << weFDrawOffscreen
};

/*	values for WEFeatureFlag action parameter */

enum {
	weBitTest = -1,				/* return the current setting of the specified feature */
	weBitClear,					/* disables the specified feature */
	weBitSet					/* enables the specified feature */
};

/*	selectors for WEGetInfo and WESetInfo */

enum {
	weClickLoop		=	'clik',		/* click loop callback */
	wePort			=	'port',		/* graphics port */
	weRefCon		=	'refc',		/* reference constant for use by application */
	weScrollProc	=	'scrl',		/* auto-scroll callback */
	weText			=	'text',		/* text handle */
	weTSMDocumentID	=	'tsmd',		/* Text Services Manager document ID */
	weTSMPreUpdate	=	'pre ',		/* Text Services Manager pre-update callback */
	weTSMPostUpdate	=	'post'		/* Text Services Manager post-update callback */
};

typedef struct WERunInfo {
	long 		runStart;
	long 		runEnd;
	short 		runHeight;
	short 		runAscent;
	TextStyle 	runStyle;
} WERunInfo;

typedef Handle WEHandle;

/*	callback prototypes */

typedef pascal Boolean (*WEClickLoopProcPtr)(WEHandle hWE);
typedef pascal void (*WEScrollProcPtr)(WEHandle hWE);
typedef pascal void (*WETSMPreUpdateProcPtr)(WEHandle hWE);
typedef pascal void (*WETSMPostUpdateProcPtr)(WEHandle hWE,
		long fixLength, long inputAreaStart, long inputAreaEnd,
		long pinRangeStart, long pinRangeEnd);

/*	WASTE public calls */

#ifdef __cplusplus
extern "C" {
#endif

/*	creation and destruction */

pascal OSErr WENew(const LongRect *destRect, const LongRect *viewRect, short flags, WEHandle *hWE);
pascal void WEDispose(WEHandle hWE);

/*	getting variables */

pascal Handle WEGetText(WEHandle hWE);
pascal short WEGetChar(long offset, WEHandle hWE);
pascal long WEGetTextLength(WEHandle hWE);
pascal long WECountLines(WEHandle hWE);
pascal long WEGetHeight(long startLine, long endLine, WEHandle hWE);
pascal void WEGetSelection(long *selStart, long *selEnd, WEHandle hWE);
pascal void WEGetDestRect(LongRect *destRect, WEHandle hWE);
pascal void WEGetViewRect(LongRect *viewRect, WEHandle hWE);
pascal Boolean WEIsActive(WEHandle hWE);

/*	setting variables */

pascal void WESetSelection(long selStart, long selEnd, WEHandle hWE);
pascal void WESetDestRect(const LongRect *destRect, WEHandle hWE);
pascal void WESetViewRect(const LongRect *viewRect, WEHandle hWE);

/*	accessing style run information */

pascal Boolean WEContinuousStyle(short *mode, TextStyle *ts, WEHandle hWE);
pascal void WEGetRunInfo(long offset, WERunInfo *runInfo, WEHandle hWE);

/*	converting byte offsets to screen position and vice versa */

pascal long WEGetOffset(const LongPoint *thePoint, char *edge, WEHandle hWE);
pascal void WEGetPoint(long offset, LongPoint *thePoint, short *lineHeight, WEHandle hWE);

/*	finding words and lines */

pascal void WEFindWord(long offset, char edge, long *wordStart, long *wordEnd, WEHandle hWE);
pascal void WEFindLine(long offset, char edge, long *lineStart, long *lineEnd, WEHandle hWE);

/*	making a copy of a text range */

pascal OSErr WECopyRange(long rangeStart, long rangeEnd, Handle hText, StScrpHandle hStyles, WEHandle hWE);

/*	getting and setting the alignment style */

pascal char WEGetAlignment(WEHandle hWE);
pascal void WESetAlignment(char alignment, WEHandle hWE);

/*	recalculating line breaks, drawing and scrolling */

pascal OSErr WECalText(WEHandle hWE);
pascal void WEUpdate(RgnHandle updateRgn, WEHandle hWE);
pascal void WEScroll(long hOffset, long vOffset, WEHandle hWE);
pascal void WESelView(WEHandle hWE);

/*	handling activate / deactivate events */

pascal void WEActivate(WEHandle hWE);
pascal void WEDeactivate(WEHandle hWE);

/* 	handling key-down events */

pascal void WEKey(short key, short modifiers, WEHandle hWE);

/*	handling mouse-down events and mouse tracking */

pascal void WEClick(Point hitPt, short modifiers, long clickTime, WEHandle hWE);

/*	adjusting the cursor shape */

pascal Boolean WEAdjustCursor(Point mouseLoc, RgnHandle mouseRgn, WEHandle hWE);

/*	blinking the caret */

pascal void WEIdle(long *maxSleep, WEHandle hWE);

/*	modifying the text and the styles */

pascal OSErr WEInsert(const void *textPtr, long textLength, StScrpHandle hStyles, WEHandle hWE);
pascal OSErr WEDelete(WEHandle hWE);
pascal OSErr WESetStyle(short mode, const TextStyle *ts, WEHandle hWE);
pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEHandle hWE);
pascal OSErr WEUseText(Handle hText, WEHandle hWE);

/*	clipboard operations */

pascal OSErr WECut(WEHandle hWE);
pascal OSErr WECopy(WEHandle hWE);
pascal OSErr WEPaste(WEHandle hWE);

/*	Script Manager utilities */

pascal short WECharByte(long offset, WEHandle hWE);
pascal short WECharType(long offset, WEHandle hWE);

/*	Text Services Manager support */

pascal OSErr WEInstallTSMHandlers(void);
pascal void WEStopInlineSession(WEHandle hWE);

/*	additional features */

pascal short WEFeatureFlag(short feature, short action, WEHandle hWE);
pascal OSErr WEGetInfo(OSType selector, void *info, WEHandle hWE);
pascal OSErr WESetInfo(OSType selector, const void *info, WEHandle hWE);

/*	long coordinate utilities */

pascal void WELongPointToPoint(const LongPoint *lp, Point *p);
pascal void WEPointToLongPoint(Point p, LongPoint *lp);
pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom);
pascal void WELongRectToRect(const LongRect *lr, Rect *r);
pascal void WERectToLongRect(const Rect *r, LongRect *lr);
pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset);
pascal Boolean WELongPointInLongRect(const LongPoint *lp, const LongRect *lr);

#ifdef __cplusplus
}
#endif

#endif _WASTE_

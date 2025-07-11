/*----------------------------------------------------------------------------

	ldef.c

	This module contains the LDEF for NewsWatcher.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "ldef.h"
#include "util.h"


static RGBColor gHighlightColors[5] = {			/* hilight color table */
	{0xDDDD, 0x0000, 0x0000},	/* red */
	{0x0000, 0x9999, 0x0000},	/* green */
	{0x0000, 0x0000, 0xDDDD},	/* blue */
	{0x9999, 0x0000, 0x9999},	/* purple */
	{0xDDDD, 0x7777, 0x0000},	/* orange */
};

static RGBColor gLightBlue = {0x9999, 0x9999, 0xFFFF};	/* light blue for filling triangles */

static Rect *gRect;					/* cell rect */
static FontInfo gFontInfo;			/* font information */
static TWindow **gInfo;				/* window information */
static short gIndex;				/* index in group or subject array */
static Handle gStrings;				/* handle to strings */
static short gH, gV;				/* h and v coords for drawing */
static Rect gEraseRect;				/* rectangle erased and redrawn */
static TSubject **gSubjectArray; 	/* handle to subject array */
static TSubject gTheSubject;		/* subject record to be drawn */



/*----------------------------------------------------------------------------
	DrawFullOrNewGroupCell
	
	Draws a single cell in the full group list window or the new groups
	list window.
----------------------------------------------------------------------------*/

static void DrawFullOrNewGroupCell (void)
{			
	TGroup **groupArray;
	char *theString;
	short strLen;

	groupArray = (**gInfo).groupArray;
	gEraseRect = *gRect;
	EraseRect(&gEraseRect);
	theString = *gStrings + (*groupArray)[gIndex].nameOffset;
	strLen = strlen(theString);
	MoveTo(gH, gV);
	DrawText(theString, 0, strLen);
}



/*----------------------------------------------------------------------------
	DrawUserGroupCell
	
	Draws a single cell in a user group list window.
----------------------------------------------------------------------------*/

static void DrawUserGroupCell (void)
{			
	TGroup **groupArray, theGroup;
	long numUnread;
	char *theString;
	short strLen;
	Str255 numStr;

	groupArray = (**gInfo).groupArray;
	theGroup = (*groupArray)[gIndex];
	gEraseRect = *gRect;
	if (theGroup.onlyRedrawCount) gEraseRect.right = gH + (**gInfo).numUnreadHCoord;
	EraseRect(&gEraseRect);
	numUnread = theGroup.numUnread;
	if (numUnread != 0) {
		if (numUnread > 9999) numUnread = 9999;
		NumToString(numUnread, numStr);
		MoveTo(gH + (**gInfo).numUnreadHCoord - StringWidth(numStr), gV);
		DrawString(numStr);
	}
	if (!theGroup.onlyRedrawCount) {
		theString = *gStrings + theGroup.nameOffset;
		strLen = strlen(theString);
		MoveTo(gH + (**gInfo).groupNameHCoord, gV);
		DrawText(theString, 0, strLen);
	}
}



/*----------------------------------------------------------------------------
	DrawThreadHeadInfo
	
	Draws the thread head info for a thread head cell in a subject list 
	window (the thread control and number of articles in the thread).
----------------------------------------------------------------------------*/

static void DrawThreadHeadInfo (void)
{
	PolyHandle poly;
	short th, tv;
	Str255 numStr;
	short threadLength;
	RGBColor saveFG;
	Rect tempRect;

	if (gTheSubject.threadLength == 1) {
		MoveTo(gH + (**gInfo).minusSignHCoord, gV);
		DrawChar('-');
	} else {
		if (gTheSubject.collapsed) {
			poly = (**gInfo).collapseTriangle;
			th = gH + ((**poly).polyBBox.right >> 1) + 1; 
			tv = gRect->top + 1;
		} else {
			poly = (**gInfo).expandTriangle;
			th = gH + 1; 
			tv = gRect->top + ((**poly).polyBBox.bottom >> 1) + 1;
		}
		OffsetPoly(poly, th, tv);
		if (gTheSubject.drawTriangleFilled) {
			FillPoly(poly, qd.black);
		} else {
			tempRect = (**poly).polyBBox;
			if (GetPixelDepth(&tempRect) > 2) {
				GetForeColor(&saveFG);
				RGBForeColor(&gLightBlue);
				FillPoly(poly, qd.gray);
				RGBForeColor(&saveFG);
			}
		}
		FramePoly(poly);
		OffsetPoly(poly, -th, -tv);

		if (!gTheSubject.onlyRedrawTriangle) {
			threadLength = gTheSubject.threadLength;
			if (threadLength > 99) threadLength = 99;
			NumToString(threadLength, numStr);
			MoveTo(gH + (**gInfo).threadCountHCoord - StringWidth(numStr), gV);
			DrawString(numStr);
		}
	}
}



/*----------------------------------------------------------------------------
	DrawCheckMark
	
	Draws the article read check mark in a cell in a subject list window.
	For collapsed thread head cells, the mark is drawn iff the entire
	thread has been read. 
----------------------------------------------------------------------------*/

static void DrawCheckMark (void)
{
	Boolean read;
	short nextInThread;

	if (gTheSubject.collapsed) {
		read = true;
		nextInThread = gIndex;
		while (true) {
			if (!(*gSubjectArray)[nextInThread].read) {
				read = false;
				break;
			}
			nextInThread = 
				(*gSubjectArray)[nextInThread].nextInThread;
			if (nextInThread == -1) break;
		}
	} else {
		read = gTheSubject.read;
	}
	if (read) {
		MoveTo(gH + (**gInfo).checkHCoord, gV);
		DrawChar('�');
	}
}



/*----------------------------------------------------------------------------
	DrawAuthorAndSubject
	
	Draws the author and subject in a cell in a subject list window.
----------------------------------------------------------------------------*/

static void DrawAuthorAndSubject (void)
{
	RGBColor saveFG;
	char *theString;
	short strLen;
	short width;

	if (gHasColorQD && gTheSubject.highlight > 0) {
		GetForeColor(&saveFG);
		RGBForeColor(&gHighlightColors[gTheSubject.highlight-1]);
	}
	if ((**gInfo).authorsShown && gTheSubject.authorOffset >= 0) {
		theString = *gStrings + gTheSubject.authorOffset;
		strLen = strlen(theString);
		while (true) {
			width = TextWidth(theString, 0, strLen);
			if (width <= (**gInfo).authorWidth) break;
			if (strLen == 1) break;
			strLen--;
		}
		MoveTo(gH + (**gInfo).authorHCoord, gV);
		DrawText(theString, 0, strLen);
	}
	theString = *gStrings + gTheSubject.subjectOffset;
	strLen = strlen(theString);
	MoveTo(gH + (**gInfo).subjectHCoord, gV);
	DrawText(theString, 0, strLen);
	if (gHasColorQD && gTheSubject.highlight > 0) {
		RGBForeColor(&saveFG);
	}
}



/*----------------------------------------------------------------------------
	DrawSubjectCell
	
	Draws a single cell in a subject list window.
----------------------------------------------------------------------------*/

static void DrawSubjectCell (void)
{	
	gSubjectArray = (**gInfo).subjectArray;
	gTheSubject = (*gSubjectArray)[gIndex];
	gEraseRect = *gRect;
	if (gTheSubject.onlyRedrawTriangle) {
		gEraseRect.right = gH + gFontInfo.ascent + 2;
	} else if (gTheSubject.onlyRedrawCheck) {
		gEraseRect.left = gH + (**gInfo).checkHCoord;
		gEraseRect.right = gH + (**gInfo).authorHCoord;
	}
	EraseRect(&gEraseRect);
	
	if (gTheSubject.threadOrdinal == 1 && !gTheSubject.onlyRedrawCheck) 
		DrawThreadHeadInfo();
	
	if (!gTheSubject.onlyRedrawTriangle) 
		DrawCheckMark();
		
	if (!gTheSubject.onlyRedrawTriangle && !gTheSubject.onlyRedrawCheck) 
		DrawAuthorAndSubject();
}



/*----------------------------------------------------------------------------
	ListDefFunc
	
	This is the list definition function used for group and subject list 
	windows. It is called by the stub code in the standalone LDEF 128 code 
	resource.
----------------------------------------------------------------------------*/

void ListDefFunc (short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle)
{	
	char *cellDataPtr;
	EWindowKind kind;
	
	switch (lMessage) {

	  case lDrawMsg:
	  
		(**lHandle).port->txFace = 0;
		GetFontInfo(&gFontInfo);
		gRect = lRect;
		gH = lRect->left + (**lHandle).indent.h;
		gV = lRect->top + gFontInfo.ascent;
		gInfo = (TWindow**)GetWRefCon((WindowPtr)(**lHandle).port);
		kind = (**gInfo).kind;
		gStrings = (**gInfo).strings;
		cellDataPtr = *(**lHandle).cells + lDataOffset;
		gIndex = *(unsigned short*)cellDataPtr;
		
		HLock(gStrings);
		switch (kind) {
			case kFullGroup:
			case kNewGroup:
				DrawFullOrNewGroupCell();
				break;
			case kUserGroup:
				DrawUserGroupCell();
				break;
			case kSubject:
				DrawSubjectCell();
				break;
		}
		HUnlock(gStrings);

		if (lSelect) {
			BitClr(&HiliteMode, pHiliteBit);
			InvertRect(&gEraseRect);
		}
		
	  	break;

	  case lHiliteMsg:
	  
		BitClr(&HiliteMode, pHiliteBit);
	  	InvertRect(lRect);
	  	break;
	  	
	}
	
}

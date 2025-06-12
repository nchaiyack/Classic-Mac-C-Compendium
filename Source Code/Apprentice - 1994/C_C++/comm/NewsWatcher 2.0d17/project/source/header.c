/*----------------------------------------------------------------------------

	header.c

	This module handles the Show/Hide Header command, and
	contains some utilities for dealing with headers.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "header.h"
#include "menus.h"
#include "resize.h"
#include "scroll.h"
#include "util.h"
#include "wind.h"


/*	FindHeader - search the header part of a message for some header.
*/

Boolean FindHeader (Handle text, const char *hdr, char *contents, 
	short maxLength)
{
	long hdrLen, hdrEnd;
	char *p, *pEnd, *q;
	short len;

	*contents = 0;
	hdrLen = strlen(hdr);
	hdrEnd = Munger(text,0,CRCR,2,nil,0);
	if (hdrEnd < 0) return false;
	p = *text;
	pEnd = *text + hdrEnd;
	while (p < pEnd) {
		if (strncasecmp(p, hdr, hdrLen) == 0) {
			p += hdrLen;
			while (p < pEnd && *p == ' ') p++;
			q = p;
			while (q < pEnd && *q != CR) q++;
			q--;
			while (q > p && *q == ' ') q--;
			q++;
			len = q-p;
			if (len == 0 || len >= maxLength) return false;
			strncpy(contents, p, len);
			contents[len] = 0;
			return true;
		}
		while (p < pEnd && *p != CR) p++;
		p++;
	}
	return false;
}

/* PrettifyName extracts the author's name from the "From" text.
 *	The new "prettified" name replaces the old one.
 *	Returns the length of the prettified name. 
 */
short PrettifyName (char *name)
{
	char *p, *start = 0, *end = 0;
	short len, oldlen;

	oldlen = strlen(name);
	end = name + oldlen - 1;

	/* Find the first < or ( in the name */
	p = name;
	while (*p && *p != '(' && *p != '<') p++;
	if (*p == '(') { 				/* format = "address (name)" */
		while (*p) {
			if (*p == '(') {
				start = p+1;		/* Handle (illegal) nested parens */
			} else if (*p == ')') {
				end = p - 1;
				break;
			}
			p++;
		}
	} else { 						/* format = "name <address>" or "address" */
		if (*p == '<')
			p++;
		start = p;
		while (*p && *p != '>') p++;
		end = p - 1;
	}
	if (start && end >= start) {
		while (start < end && *start == ' ') start++;
		while (start < end && *end == ' ') end--;
		len = end - start + 1;
		BlockMove(start, name, len);
		name[len] = 0;
		return len;
	}
	return oldlen;
}


/*	GetAuthorFromHeader extracts the author's name from an article.
*/

void GetAuthorFromHeader (Handle text, char *author)
{
	FindHeader(text, "From:", author, 255);
	PrettifyName(author);
}



/*	FindBody returns the offset in article or message text of the first character of the 
	article or message body. It skips past the header and any empty lines following the
	header.
*/

long FindBody (Handle text)
{
	char *p, *pEnd;
	long length;
	
	length = GetHandleSize(text);
	p = *text;
	pEnd = p + length;
	while (p < pEnd) {
		if (*p == CR && *(p+1) == CR) break;
		p++;
	}
	p += 2;
	return p < pEnd ? p-*text : length;
}


/*	DoShowHideHeader shows or hides the header in an article or message window.
*/

void DoShowHideHeader (WindowPtr wind)
{
	TWindow **info;
	ControlHandle scrollBar;
	Boolean headerShown;
	short numSections;
	TEHandle theTE;
	Handle text;
	long length, headerLength;
	EWindowKind kind;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	theTE = (**info).theTE;
	headerShown = (**info).headerShown = !(**info).headerShown;
	SetItem(GetMHandle(kEditMenu),kShowHideHeaderItem,
		headerShown ? "\pHide Header" : "\pShow Header");
	if (kind == kArticle || kind == kMiscArticle) { /* article window */
		numSections = (**info).numSections;
		scrollBar = numSections > 1 ? HScrollCont(wind) : VScrollCont(wind);
		SetCtlValue(scrollBar, 0);
		MoveText(wind, scrollBar);
		if (numSections > 1) return;
		text = (**info).fullText;
		if (headerShown) {
			length = GetHandleSize(text);
			HLock(text);
			TESetText(*text,length,theTE);
			HUnlock(text);
			TESetSelect(0,0,theTE);
			AdjustScrollBar(wind);
			(**theTE).destRect = (**theTE).viewRect;
			EraseRect(&(**theTE).destRect);
			TEUpdate(&(**theTE).destRect,theTE);
		} else {
			TESetSelect(0, FindBody(text), theTE);
			TEDelete(theTE);
			AdjustScrollBar(wind);
		}
	} else { /* message window */
		scrollBar = VScrollCont(wind);
		SetCtlValue(scrollBar, 0);
		MoveText(wind, scrollBar);
		if (headerShown) {
			text = (**info).headerText;
			length = GetHandleSize(text);
			TESetSelect(0, 0, theTE);
			HLock(text);
			TEInsert(*text, length, theTE);
			TESetSelect(length, length, theTE);
			HUnlock(text);
		} else {
			text = (Handle)TEGetText(theTE);
			headerLength = FindBody(text);
			if ((**info).headerText == nil) (**info).headerText = MyNewHandle(0);
			HLock(text);
			PtrToXHand(*text, (**info).headerText, headerLength);
			HUnlock(text);
			TESetSelect(0, headerLength, theTE);
			TEDelete(theTE);
		}
		AdjustScrollBar(wind);
	}
	if (headerShown && gPrefs.zoomWindows && GetCtlMax(scrollBar) > 0) {
		DoZoom(wind,inZoomOut);
	} else {
		SetWindowNeedsZooming(wind);
	}
}

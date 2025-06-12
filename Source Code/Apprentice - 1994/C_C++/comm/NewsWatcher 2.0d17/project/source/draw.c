/*----------------------------------------------------------------------------

	draw.c

	This module draws the contents of windows.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "draw.h"
#include "header.h"
#include "util.h"
#include "datetime.h"


/*	DrawSectionMessage draws the "Section x of y" message in a window. */

void DrawSectionMessage (WindowPtr wind)
{
	TWindow **info;
	short fontNum,savedFontNum,savedFontSize;
	CStr255 msg,num;
	
	info = (TWindow**) GetWRefCon(wind);
	GetFNum("\pMonaco",&fontNum);
	savedFontNum = wind->txFont;
	savedFontSize = wind->txSize;
	TextFont(fontNum);
	TextSize(9);
	strcpy(msg, "Section ");
	NumToString((**info).curSection+1, (StringPtr)num);
	p2cstr((StringPtr)num);
	strcat(msg, num);
	strcat(msg, " of ");
	NumToString((**info).numSections, (StringPtr)num);
	p2cstr((StringPtr)num);
	strcat(msg, num);
	MoveTo(kTextMargin, wind->portRect.bottom-4);
	c2pstr(msg);
	DrawString((StringPtr)msg);
	TextFont(savedFontNum);
	TextSize(savedFontSize);
}


/*	DrawArticlePanel draws the panel below the window title bar for
	article windows.
*/

static void DrawArticlePanel (WindowPtr wind)
{
	WindowPtr parent;
	TWindow **info, **parentInfo;
	TSubject subject, **subjectArray;
	Handle text;
	short panelHeight,windWidth;
	CStr255 author, org, date;
	char msg[522];
	FontInfo fontInfo;
	short len, width1, width2, v;
	char c;
	
	info = (TWindow**)GetWRefCon(wind);
	panelHeight = (**info).panelHeight;
	text = (**info).fullText;
	GetFontInfo(&fontInfo);
	
	windWidth = wind->portRect.right - wind->portRect.left;
	MoveTo(0,panelHeight-3);
	LineTo(windWidth,panelHeight-3);
	MoveTo(0,panelHeight-1);
	LineTo(windWidth,panelHeight-1);
	
	GetAuthorFromHeader(text, author);
	strcpy(msg, "Author: ");
	strcat(msg, author);
	v = fontInfo.ascent + 3;
	MoveTo(kTextMargin, v);
	len = strlen(msg);
	DrawText(msg, 0, len);
	
	
	FindHeader(text, "Organization:", org, sizeof(org));
	strcpy(msg, "Organization: ");
	strcat(msg, org);
	v += fontInfo.ascent + fontInfo.descent + fontInfo.leading;
	MoveTo(kTextMargin, v);
	len = strlen(msg);
	DrawText(msg, 0, len);
	
	FindHeader(text, "Date:", date, sizeof(date));
	Cleanup822Date(date);
	v += fontInfo.ascent + fontInfo.descent + fontInfo.leading;
	MoveTo(kTextMargin, v);
	len = strlen(date);
	DrawText(date, 0, len);
	width1 = TextWidth(date, 0, len);
	
	if ((**info).kind == kArticle) {
		parent = (**info).parentWindow;
		parentInfo = (TWindow**)GetWRefCon(parent);
		subjectArray = (**parentInfo).subjectArray;
		subject = (*subjectArray)[(**info).parentSubject];
		if (subject.threadLength > 1) {
			c = subject.threadOrdinal == subject.threadLength ? '¥' : 'É';
			sprintf(msg, "Article %d of %d in thread %c", 
				subject.threadOrdinal, subject.threadLength, c);
			len = strlen(msg);
			width2 = TextWidth(msg, 0, len);
			if (width1 + width2 + 2*kTextMargin + 10 < windWidth) {
				MoveTo(windWidth - kTextMargin - width2, v);
				DrawText(msg, 0, len);
			}
		}
	}
}


/*	DrawMessagePanel draws the panel below the window title bar for
	message windows.
*/

static void DrawMessagePanel (WindowPtr wind)
{
	TWindow **info;
	short panelHeight, windWidth;
	Handle text;
	CStr255 line, temp;
	CStr255 msg;
	FontInfo fontInfo;
	short len;
	EWindowKind kind;
	short maxWidth;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	panelHeight = (**info).panelHeight;
	windWidth = wind->portRect.right - wind->portRect.left;
	MoveTo(0,panelHeight-3);
	LineTo(windWidth,panelHeight-3);
	MoveTo(0,panelHeight-1);
	LineTo(windWidth,panelHeight-1);
	
	text = (**info).headerShown ? (Handle)TEGetText((**info).theTE) : 
		(**info).headerText;
	if (kind == kPostMessage) {
		FindHeader(text, "Newsgroups:", line, 200);
	} else {
		FindHeader(text, "To:", line, 200);
		PrettifyName(line);
	}
	strcpy(msg, kind == kPostMessage ? "Posting to: " : "Mail message to: ");
	strcat(msg, line);
	
	GetFontInfo(&fontInfo);
	MoveTo(kTextMargin, 
		((panelHeight - 4 - fontInfo.ascent - fontInfo.descent) >> 1) +
			fontInfo.ascent);
	len = strlen(msg);
	maxWidth = wind->portRect.right - 70;
	while (TextWidth(msg, 0, len) > maxWidth) len--;
	DrawText(msg, 0, len);
}


/*	DrawWindow is called to re-draw window contents in response
	to update events.
*/

static void DrawWindow (WindowPtr wind)
{
	TWindow **info;
	EWindowKind kind;
	Rect clipRect;

	info = (TWindow**) GetWRefCon(wind);
	kind = (**info).kind;	
	
	clipRect = wind->portRect;
	clipRect.top += (**info).panelHeight;
	ClipRect(&clipRect);
	DrawGrowIcon(wind);
	ClipRect(&wind->portRect);
	switch (kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
		case kSubject:
			LUpdate(wind->visRgn,(**info).theList);
			break;
		case kArticle:
		case kMiscArticle:
		case kPostMessage:
		case kMailMessage:
			UpdtControl(wind,wind->visRgn);
			TEUpdate(&wind->portRect,(**info).theTE);
			if ((**info).numSections > 1) DrawSectionMessage(wind);
			if (kind == kArticle || kind == kMiscArticle) {
				DrawArticlePanel(wind);
			} else {
				DrawMessagePanel(wind);
			}
			break;
	}
}


/* HandleUpdate: Handles update events */

void HandleUpdate (WindowPtr wind)
{
	GrafPtr savePort;
	
	if (((WindowPeek)wind)->windowKind < 0 || 
		((WindowPeek)wind)->windowKind == dialogKind) return;
	GetPort(&savePort);
	SetPort(wind);
	BeginUpdate(wind);
	if (!IsStatusWindow(wind)) {
		EraseRect(&wind->portRect);
		DrawWindow(wind);
	} else {
		UpdateStatus();
	}
	EndUpdate(wind);
	SetPort(savePort);
}

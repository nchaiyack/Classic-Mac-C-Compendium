/*----------------------------------------------------------------------------

	article.c

	This module handles the creation of article windows.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "article.h"
#include "child.h"
#include "close.h"
#include "dlgutil.h"
#include "header.h"
#include "mark.h"
#include "nntp.h"
#include "open.h"
#include "resize.h"
#include "scroll.h"
#include "util.h"


/*	CalcSectionBreaks calculates section breaks for an article 
	larger than 32K. */

static void CalcSectionBreaks (WindowPtr wind)
{
	TWindow **info;
	Handle text;
	long **breaks;
	long len;
	short numSections;
	char *p,*textEnd,*qStart,*qEnd,*q,*r,*qCR,*qCRCR;

	info = (TWindow**)GetWRefCon(wind);
	text = (**info).fullText;
	len = GetHandleSize(text);
	breaks = (long**)MyNewHandle((len/25000 + 2)<<2);
	p = *text;
	textEnd = *text + len;
	numSections = 0;
	while (true) {
		(*breaks)[numSections] = p - *text;
		qStart = p+31999;
		if (qStart >= textEnd) break;
		qCR = qCRCR = qEnd = p+25010;
		for (q = qStart; q > qEnd; q--) {
			if (*q == CR) {
				qCR = q;
				if (*(q-1) == CR) {
					qCRCR = q;
					for (r = q-2; *r == '-' && r > qEnd; r--);
					if (*r == CR) break;
				}
			}
		}
		if (q <= qEnd) q = qCRCR;
		if (q <= qEnd) q = qCR;
		if (q <= qEnd) q = qStart;
		p = q+1;
		numSections++;
	}
	(*breaks)[++numSections] = len;
	(**info).numSections = numSections;
	(**info).curSection = 0;
	(**info).sectionBreaks = breaks;
	MakeHScroller(wind,numSections-1);
}

/*	OpenArticle gets the text of an article, given the newsgroup,
	and article number or message-id.  The article is fetched
	and placed in a new window created by the procedure call.

	Returns as function result:
		0 if article retrieved and window opened, or existing open window brought to front.
		1 if article does not exist on server.
		2 if some other error.
*/

static short OpenArticle (char *newsGroup, char *number, char *title,
	WindowPtr parent, EWindowKind kind)
{
	Handle text;
	long length;
	WindowPtr wind;
	TWindow **newInfo;
	GrafPtr savePort;
	TEHandle theTE;
	Point firstOffset;
	OSErr err = noErr;
	short result;
	long offset;
	Handle msgId;
	char subject[250];
	CStr255 miscTitle;
	CStr255 wTitle;
	WindowPtr w;
	short i;
	
	result = GetArticle(newsGroup,number,&text,&length);
	if (result != 0) return result;

	SetPt(&firstOffset,0,0);
	GetPort(&savePort);
		
	SetPort(parent == nil ? FrontWindow() : parent);	
	LocalToGlobal(&firstOffset);
	SetPort(savePort);

	if (kind == kMiscArticle) {
		FindHeader(text, "Subject:", subject, sizeof(subject));
		i = 1;
		while (true) {
			sprintf(miscTitle, "%d.%s", i, subject); 
			w = FrontWindow();
			while (w != nil) {
				GetWTitle(w, (StringPtr)wTitle);
				p2cstr((StringPtr)wTitle);
				if (strcmp(wTitle, miscTitle) == 0) break;
				w = (WindowPtr)((WindowPeek)w)->nextWindow;
			}
			if (w == nil) break;
			i++;
		}
		c2pstr(miscTitle);
		wind = MakeNewWindow(kind, firstOffset, (StringPtr)miscTitle);
	} else {
		c2pstr(title);
		wind = MakeNewWindow(kind, firstOffset, (StringPtr)title);
		p2cstr((StringPtr)title);
	}

	newInfo = (TWindow**) GetWRefCon(wind);
	(**newInfo).parentWindow = parent;
	(**newInfo).fullText = text;
	
	if (kind == kMiscArticle) {
		PtrToHand(number, &msgId, strlen(number)+1);
		(**newInfo).msgId = msgId;
	}
	
	if (parent != nil) AddChild(parent, wind);
	SetPort(wind);
	
	if (length > kMaxLength) {
		CalcSectionBreaks(wind);
		length = (*(**newInfo).sectionBreaks)[1];
	}
	theTE = (**newInfo).theTE;
	
	(**newInfo).headerShown = gPrefs.showHeaders;
	
	offset = gPrefs.showHeaders ? 0 : FindBody(text);
	HLock(text);
	TESetText(*text+offset, length-offset, theTE);
	HUnlock(text);
	
	TESetSelect(0,0,theTE);
	if (!DoZoom(wind,inZoomOut)) {
		result = 2;
		goto exit;
	}
	AdjustScrollBar(wind);
	ShowWindow(wind);
	SetPort(savePort);
	return 0;

exit:
	
	DoCloseWindow(wind);
	SetPort(savePort);
	return result;
}


/*	OpenSubjectCell opens one article for a cell in a subject list window.
	
	Entry:	wind = pointer to subject list window.
			theCell = the cell in the subject list window to be opened.
			threadOrdinal = ordinal of article within thread to
				be opened.
			
	Exit:	function result = 
				0 if article window opened.
				1 if article no longer on server.
				2 if some other error.
*/

short OpenSubjectCell (WindowPtr wind, Cell theCell, short threadOrdinal)
{
	TWindow **info,**childInfo;
	ListHandle theList;
	TSubject **subjectArray, subject;
	CStr255 newsGroup, numStr;
	char title[512];
	short cellData, cellDataLen;
	WindowPtr child;
	short result;
	Handle strings;
	
	info = (TWindow**) GetWRefCon(wind);
	theList = (**info).theList;
	subjectArray = (**info).subjectArray;
	strings = (**info).strings;
	
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen ,theCell, theList);
	while (threadOrdinal-- > 1) 
		cellData = (*subjectArray)[cellData].nextInThread;
	
	if ((child = FindChildByCellData(wind, cellData)) != nil) {
		SelectWindow(child);
		return 0;
	}

	subject = (*subjectArray)[cellData];
	
	GetWTitle(wind, (StringPtr)newsGroup);
	p2cstr((StringPtr)newsGroup);
	
	*title = '�';
	strcpy(title+1, *strings + subject.subjectOffset);
	if (subject.threadLength > 1) {
		NumToString(subject.threadOrdinal, (StringPtr)numStr);
		p2cstr((StringPtr)numStr);
		strcat(title, " - ");
		strcat(title, numStr);
	}
	title[255] = 0;
	
	NumToString(subject.number, (StringPtr)numStr);
	p2cstr((StringPtr)numStr);
	
	result = OpenArticle(newsGroup,numStr,title,wind,kArticle);
	if (result != 0) return result;
	
	child = FrontWindow();
	childInfo = (TWindow**)GetWRefCon(child);
	(**childInfo).parentSubject = cellData;
	MarkArticle(FrontWindow(),true);

	return 0;
}


/*  CheckMiscArticleAlreadyOpen checks to see if a miscellaneous article
	window is already open. If it is already open, it is brought to the front
	and the function returns true. If not, the function returns false.
*/

static Boolean CheckMiscArticleAlreadyOpen (char *msgId)
{
	WindowPtr wind;
	TWindow **info;
	
	wind = FrontWindow();
	while (wind != nil) {
		if (IsAppWindow(wind)) {
			info = (TWindow**)GetWRefCon(wind);
			if ((**info).kind == kMiscArticle) {
				if (strcmp(msgId, *(**info).msgId) == 0) {
					SelectWindow(wind);
					return true;
				}
			}
		}
		wind = (WindowPtr)(((WindowPeek)wind)->nextWindow);
	}
	return false;
}


/*	OpenReferences opens up all articles which are referred to in an article 
	or message.  The "References:" field is used to determine which articles 
	should be fetched.
*/

void OpenReferences (WindowPtr wind)
{
	TWindow **info;
	long startReference,endReference;
	CStr255 msgId;
	char refs[4096];
	short totalRefs=0, totalOpenedRefs=0;
	CStr255 msg;
	Handle text;
	EWindowKind kind;
	short msgIdLen;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	
	if (kind == kArticle || kind == kMiscArticle) {
		text = (**info).fullText;
	} else if ((**info).headerShown) {
		text = (Handle)TEGetText((**info).theTE);
	} else {
		text = (**info).headerText;
	}

	if(!FindHeader(text,"References:",refs,sizeof(refs)))
	{
		SysBeep(1);
		return;
	}
	endReference = strlen(refs);
	while (endReference > 0) {
		while(refs[endReference] != '>' && endReference >= 0) endReference--;
		startReference = endReference;
		while(refs[startReference] != '<' && startReference >= 0) 
			startReference--;
		msgIdLen = endReference - startReference + 1;
		strncpy(msgId, &refs[startReference], msgIdLen);
		msgId[msgIdLen] = 0;
		totalRefs++;
		if (CheckMiscArticleAlreadyOpen(msgId)) {
			totalOpenedRefs++;
		} else {
			switch (OpenArticle(nil,msgId,"",nil,kMiscArticle)) {
				case 0:
					totalOpenedRefs++;
					break;
				case 1:
					break;
				case 2:
					return;
			}
		}
		endReference = startReference;
	}
	if (totalOpenedRefs < totalRefs) {
		if (totalOpenedRefs == 0) {
			strcpy(msg,"None of the referenced articles could be opened");
		} else {
			strcpy(msg,"Some of the referenced articles could not be opened");
		}
		strcat(msg,", because they no longer exist on the news server.");
		ErrorMessage(msg);
	}
}


/*	OpenSelectedReference opens article corresponding to selected message ID.
*/

void OpenSelectedReference (WindowPtr wind)
{
	TWindow **info;
	TEHandle te;
	Handle theText;
	CStr255 msgId;
	short selStart, selEnd;
	char *start,*end;
	short teLen;
		
	info = (TWindow**)GetWRefCon(wind);
	
	te = (**info).theTE;
	teLen = (**te).teLength;
	theText = (Handle) TEGetText(te);
	selStart = (*te)->selStart;
	selEnd = (*te)->selEnd;
	
	HLock(theText);
	
	start = *theText + selStart;
	end = *theText + selEnd - 1;
	while (start >= *theText && *start != '<' && *start != CR) start--;
	while (end < *theText + teLen && *end != '>' && *end != CR) end++;
	if (*start != '<' || *end != '>' || end-start > 254) {
		HUnlock(theText);
		SysBeep(1);
		return;
	}
	strncpy(msgId, start, end-start+1);
	msgId[end-start+1] = 0;

	HUnlock(theText);
	
	if (CheckMiscArticleAlreadyOpen(msgId)) return;
	if (OpenArticle(nil, msgId, "", nil, kMiscArticle) == 1) 
		ErrorMessage("The referenced article no longer exists on the news server.");
}

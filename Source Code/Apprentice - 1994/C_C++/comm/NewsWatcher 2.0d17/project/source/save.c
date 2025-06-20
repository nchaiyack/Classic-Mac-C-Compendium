/*----------------------------------------------------------------------------

	save.c

	This module handles saving article and message windows.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "dlgutil.h"
#include "glob.h"
#include "save.h"
#include "util.h"
#include "sfutil.h"



/*	This routine removes colons and returns from the titles of
	articles.  This insures that the string returned is a legal filename.
	It also removes any leading check mark.
*/

static void TitleFilter (StringPtr title)
{
	short i;
	
	if (*title > 0 && title[1] == (unsigned char)'�') {
		BlockMove(title+2, title+1, *title-1);
		(*title)--;
	}
	
	for (i=1; i<=*title; i++) {
		if (title[i] == CR || title[i] == ':') title[i] = ' ';
	}
	if (*title > 31) *title = 31;
}


/*	GetFullMessageText gets the full text for a message window, 
	including the header.
*/

void GetFullMessageText (WindowPtr wind, Handle *fullText, 
	Boolean *mustDispose)
{
	TWindow **info;
	TEHandle theTE;
	Handle text, headerText;
	long textLength, headerLength;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;
	text = (Handle)TEGetText(theTE);
	if ((**info).headerShown) {
		*fullText = text;
		*mustDispose = false;
	} else {
		headerText = (**info).headerText;
		headerLength = GetHandleSize(headerText);
		textLength = GetHandleSize(text);
		*fullText = MyNewHandle(headerLength + textLength);
		BlockMove(*headerText, **fullText, headerLength);
		BlockMove(*text, **fullText+headerLength, textLength);
		*mustDispose = true;
	}
}


/*	DoSaveWindow is called when the user issues a Save command for an
	article or message window.
	
	Exit:	function result = true if file saved, false if canceled
				or error.
*/

Boolean DoSaveWindow (WindowPtr wind)
{
	StandardFileReply reply;
	Str255 defaultName;
	TWindow **info;
	Handle text = nil;
	OSErr err;
	long length;
	short fRefNum = 0, vRefNum;
	EWindowKind kind;
	Boolean isArticle, mustDispose=false;
	CStr255 msg;
	
	GetWTitle(wind,defaultName);
	TitleFilter(defaultName);
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	isArticle = kind == kArticle || kind == kMiscArticle;

	/* Force Standard File to start in the right place */
	
	if (gPrefs.textDefaultDir) {
		if (VolNameToVRefNum(gPrefs.textVolName, &vRefNum) == noErr) {
			SFSaveDisk = -vRefNum;
			CurDirStore = gPrefs.textDirID;
		}
	}
	
	if (isArticle) {
		MyStandardPutFile("\pSave article as:", defaultName, &reply);
	} else {
		MyStandardPutFile("\pSave message as:", defaultName, &reply);
	}
    if (!reply.sfGood) return false;
	
	err = FSpOpenDF(&reply.sfFile, fsRdWrPerm, &fRefNum);
	if (err == noErr) {
		err = SetEOF(fRefNum, 0);
		if (err != noErr) goto exit1;
	} else if (err = fnfErr) {
		err = FSpCreate(&reply.sfFile, gPrefs.textCreator, 'TEXT', reply.sfScript);
		if (err != noErr) goto exit1;
		err = FSpOpenDF(&reply.sfFile, fsRdWrPerm, &fRefNum);
		if (err != noErr) goto exit1;
	} else {
		goto exit1;
	}
	
	if (isArticle) {
		text = (**info).fullText;
	} else {
		GetFullMessageText(wind, &text, &mustDispose);
	}
	length = GetHandleSize(text);
	
	HLock(text);
	err = FSWrite(fRefNum,&length,*text);
	if (err != noErr) goto exit1;
	if (*(*text + length - 1) != CR) {
		length = 1;
		err = FSWrite(fRefNum, &length, CRSTR);
		if (err != noErr) goto exit1;
	}
	HUnlock(text);
	if (mustDispose) MyDisposHandle(text);
	FSClose(fRefNum);
	FlushVol(NULL, reply.sfFile.vRefNum);
	return true;
	
exit1:
	sprintf(msg, "Unexpected error %d while trying to save file �%#s�", 
		err, reply.sfFile.name);
	ErrorMessage(msg);
	if (text != nil) HUnlock(text);
	MyDisposHandle(text);
	if (fRefNum != 0) FSClose(fRefNum);
	return false;
}

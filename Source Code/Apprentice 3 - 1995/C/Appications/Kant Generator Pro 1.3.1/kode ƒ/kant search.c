#include "kant search.h"
#include "text layer.h"
#include "dialog layer.h"
#include "memory layer.h"
#include "prefs.h"
#include "main.h"
#include "window layer.h"
#include "program globals.h"

extern	Boolean			gCustomCursor;	/* see environment.c */

#define findDialog	500

Boolean			gStartFromTop;
Boolean			gIgnoreCase;
Str255			gFindString;
Str255			gReplaceString;
short			gLastFindPosition;

static	Boolean SearchDoneQQ(short strPos, short strLen);

void SetLastFindPosition(short val)
{
	gLastFindPosition=val;
}

short SearchForwards(TEHandle hTE, short *foundLength)
{
	short			offset;
	short			max;
	short			strPos;
	short			strLen;
	unsigned char	*a;
	unsigned char	theChar, matchChar;
	
	max=(**hTE).teLength;
	HLock((**hTE).hText);
	a=(unsigned char*)(*((**hTE).hText));
	strPos=1;
	strLen=gFindString[0];
	
	offset=gLastFindPosition;
	while ((offset<max) && (!SearchDoneQQ(strPos, strLen)))
	{
		if (gIgnoreCase)
		{
			theChar=a[offset];
			if ((theChar>='A') && (theChar<='Z'))
				theChar|=0x20;
			matchChar=gFindString[strPos];
			if ((matchChar>='A') && (matchChar<='Z'))
				matchChar|=0x20;
			if (theChar==matchChar)
				strPos++;
			else
				strPos=1;
		}
		else
		{
			if (a[offset]==gFindString[strPos])
				strPos++;
			else
				strPos=1;
		}
		offset++;
	}
	
	HUnlock((**hTE).hText);
	if ((strLen>0) && (strPos>strLen))
	{
		SetLastFindPosition(offset+1);
		*foundLength=strLen;
		return offset-strLen;
	}
	
	return -1;
}

Boolean SearchDoneQQ(short strPos, short strLen)
{
	Boolean			done;
	
	done=FALSE;
	if (gFindString[0]!=0x00)
		done=(strPos>strLen);
	
	return done;
}

Boolean DoFindDialog(void)
{
	DialogPtr		theDialog;
	UniversalProcPtr	modalFilterProc, userItemProc;
	Boolean			ignoreCase, startFromTop;
	Boolean			found;
	short			itemSelected;
	
	SetCursor(&qd.arrow);
	gCustomCursor=FALSE;
	
	theDialog=SetupTheDialog(findDialog, 3, "\pFind", (UniversalProcPtr)TwoButtonFilter,
		&modalFilterProc, &userItemProc);
	ignoreCase=gIgnoreCase;
	startFromTop=gStartFromTop;
	SetDialogItemString(theDialog, 7, gFindString);
	SetDialogItemString(theDialog, 8, gReplaceString);
	HighlightDialogTextItem(theDialog, 7, 0, 32767);
	SetCheckboxState(theDialog, 9, ignoreCase);
	SetCheckboxState(theDialog, 10, startFromTop);
	DisplayTheDialog(theDialog, TRUE);
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2) && (itemSelected!=4))
	{
		itemSelected=GetOneDialogEvent(theDialog, modalFilterProc);
		
		switch (itemSelected)
		{
			case 9:	/* ignore case checkbox */
				ignoreCase=!ignoreCase;
				SetCheckboxState(theDialog, 9, ignoreCase);
				break;
			case 10:	/* start from top checkbox */
				startFromTop=!startFromTop;
				SetCheckboxState(theDialog, 10, startFromTop);
				break;
		}
	}
	
	if (itemSelected!=2)
	{
		GetDialogItemString(theDialog, 7, gFindString);
		GetDialogItemString(theDialog, 8, gReplaceString);
		gIgnoreCase=ignoreCase;
		gStartFromTop=startFromTop;
	}

	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
	
	while (HandleSingleEvent(FALSE)) {};
	
	if (itemSelected!=2)
	{
		SaveThePrefs();
		if (gStartFromTop)
			SetWindowLastFindPosition(GetFrontDocumentWindow(), 0);
		if (itemSelected==1)
		{
			found=DoFindAgain();
			if (!found)
				SysBeep(7);
			return found;
		}
		else
		{
			DoReplaceAll();
			return TRUE;
		}
	}
	
	return FALSE;
}

Boolean DoFindAgain(void)
{
	TEHandle		hTE;
	short			newOffset;
	short			len;
	WindowRef		theWindow;
	
	if ((theWindow=GetFrontDocumentWindow())==0L)
		return FALSE;
	
	if ((hTE=GetWindowTE(theWindow))==0L)
		return FALSE;
	
	SetLastFindPosition(GetWindowLastFindPosition(theWindow));
	newOffset=SearchForwards(hTE, &len);
	if (newOffset!=-1)
	{
		TESetSelect(newOffset, newOffset+len, hTE);
		TESelView(hTE);
		AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
		SetWindowLastFindPosition(theWindow, newOffset+len);
		return TRUE;
	}
	
	return FALSE;
}

void DoReplace(void)
{
	WindowRef		theWindow;
	TEHandle		hTE;
	
	if ((theWindow=GetFrontDocumentWindow())==0L)
		return;
	if ((hTE=GetWindowTE(theWindow))==0L)
		return;
	
	TEDelete(hTE);
	InsertBeforeStart(theWindow, gReplaceString);
	AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	SetWindowLastFindPosition(theWindow, (**hTE).selEnd);
}

void DoReplaceAll(void)
{
	SetWindowLastFindPosition(GetFrontDocumentWindow(), 0);
	while (DoFindAgain())
		DoReplace();
	SysBeep(7);
// maybe show alert here saying how many were found?
}

void DoEnterString(Boolean isFindString)
{
	WindowRef		theWindow;
	
	if ((theWindow=GetFrontDocumentWindow())==0L)
		return;
	GetSelectionString(theWindow, isFindString ? gFindString : gReplaceString);
}

Boolean AnythingToFindQQ(void)
{
	return (gFindString[0]!=0x00);
}

Boolean AnythingToReplaceQQ(void)
{
	return (gReplaceString[0]!=0x00);
}

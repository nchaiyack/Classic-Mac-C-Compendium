#include "jotto main window.h"
#include "jotto meat.h"
#include "jotto endgame.h"
#include "jotto graphics.h"
#include "jotto keys.h"
#include "jotto environment.h"
#include "jotto load-save.h"
#include "environment.h"
#include "util.h"
#include "dialogs.h"
#include "program globals.h"
#include "pict utilities.h"
#include "wipe dispatch.h"
#include "graphics.h"
#include "window layer.h"

static	short			gOldForegroundTime;		/* stored foreground wait time */
static	Boolean			gSetupDone=FALSE;

CIconHandle		gColorIcons[3];
Handle			gBWIcons[3];
PicHandle		gBackgroundPict=0L;

void SetupTheMainWindow(WindowPtr theWindow)
{
	unsigned char	*titleStr="\pJotto ][";
	short			i;
	FSSpec			theFS;
	
	SetWindowMaxDepth(theWindow, 8);
	SetWindowDepth(theWindow, 8);
	SetWindowType(theWindow, noGrowDocProc);
	SetWindowHasCloseBox(theWindow, TRUE);
	SetWindowTitle(theWindow, titleStr);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowAutoCenter(theWindow, TRUE);
	SetWindowWidth(theWindow, 300);
	SetWindowHeight(theWindow, 300);
	theFS.name[0]=0x00;
	SetWindowFS(theWindow, theFS);
	SetWindowIsModified(theWindow, FALSE);
	
	if (gSetupDone)
		return;
	
	for (i=0; i<3; i++)
	{
		if (gHasColorQD)
			gColorIcons[i]=GetCIcon(128+i);
		gBWIcons[i]=GetIcon(128+i);
	}
	
	gSetupDone=TRUE;
}

void ShutDownTheMainWindow(void)
{
	short			i;
	
	for (i=0; i<3; i++)
	{
		if (gHasColorQD)
			DisposeCIcon(gColorIcons[i]);
		ReleaseResource(gBWIcons[i]);
	}

	gBackgroundPict=ReleaseThePict(gBackgroundPict);
}

Boolean CloseTheMainWindow(WindowPtr theWindow)
{
	ModalFilterUPP	procFilter = NewModalFilterProc(ThreeButtonFilter);
	short			result;
	
	if (WindowIsModifiedQQ(theWindow))
	{
		SetCursor(&qd.arrow);
		PositionDialog('ALRT', saveAlert);
		ParamText(GetWindowTitle(theWindow), "\p", "\p", "\p");
		result=Alert(saveAlert, procFilter);
		DisposeRoutineDescriptor(procFilter);
		switch (result)
		{
			case 1:	/* save */
				LoadSaveDispatch(FALSE, TRUE, 0L);
				if (WindowIsModifiedQQ(theWindow))	/* save didn't work for some reason */
					return FALSE;
				break;
			case 2:	 /* cancel */
				return FALSE;
				break;
			case 3:	/* don't save */
				break;
		}
	}
	
	gCustomCursor=FALSE;
	
	return TRUE;
}

void KeyPressedInMainWindow(WindowPtr theWindow, unsigned char theChar)
{
	ObscureCursor();
	
	if (GameInProgressQQ())
	{
		if ((theChar==0x0d) || (theChar==0x03))
			UserHitReturn(theWindow);
		else if ((theChar==0x1c) || (theChar==0x08))
			UserHitLeftArrow(theWindow, theChar);
		else if (theChar==0x1d)
			UserHitRightArrow(theWindow);
		else if ((((theChar&0xdf)>='A') && ((theChar&0xdf)<='Z')) || (theChar==' '))
			UserHitLetter(theWindow, theChar);
	}
	else if (EndGameShowingQQ())
	{
		SetEndGame(FALSE);
		UpdateTheWindow(theWindow);
	}
	
	SetWindowIsModified(theWindow, TRUE);
}

void MouseClickedInMainWindow(WindowPtr theWindow, Point thePoint)
{
	Rect		theRect;
	short		i,target;
	
	if (GameInProgressQQ())
	{
		theRect.top=31;
		theRect.bottom=theRect.top+38;
		theRect.left=(GetWindowWidth(theWindow))/2-99-((gNumLetters==6) ? 20 : 0);
		theRect.right=theRect.left+38;
		target=-1;
		for (i=0; i<gNumLetters; i++)
		{
			if (PtInRect(thePoint, &theRect))
			{
				target=i;
				i=gNumLetters;
			}
			theRect.left+=40;
			theRect.right+=40;
		}
		
		if (target!=-1)
		{
			DrawOneChar(theWindow);
			gWhichChar=target;
			HighlightChar(theWindow);
		}
	}
	else if (EndGameShowingQQ())
	{
		SetEndGame(FALSE);
		UpdateTheWindow(theWindow);
	}
}

void ActivateTheMainWindow(WindowPtr theWindow)
{
	gOldForegroundTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
}

void DeactivateTheMainWindow(WindowPtr theWindow)
{
	gForegroundWaitTime=gOldForegroundTime;
}

void DrawTheMainWindow(WindowPtr theWindow, short theDepth)
{
	RGBColor		oldForeColor, oldBackColor;
	GrafPtr			curPort;
	Rect			theRect;
	short			i;
	Str255			tempStr;
	char			oldChar;
	short			endi;
	unsigned char	*temp="\pYOU ARE A";
	unsigned char	*temp2="\pWINNER!";
	PicHandle		thePic;
	
	if (theDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
	}
	
	GetPort(&curPort);
	if (!EndGameShowingQQ())
		gBackgroundPict=DrawThePicture(gBackgroundPict, 200, 0, 0);
	else
	{
		thePic=GetPicture((theDepth>2) ? 134 : 135);
		FillRect(&(curPort->portRect), &qd.black);
		TextFont(144);
		TextSize(24);
		TextMode(srcXor);
		MoveTo(150-StringWidth(temp)/2, 140);
		DrawString(temp);
		SetRect(&theRect, 0, 150, 300, 220);
		DrawPicture(thePic, &theRect);
		MoveTo(150-StringWidth(temp2)/2, 250);
		DrawString(temp2);
		ReleaseResource((Handle)thePic);
	}	
	DrawCurrentWord(theWindow);
	
	if (!EndGameShowingQQ())
	{
		if (GameInProgressQQ())
			HighlightChar(theWindow);
		
		endi=(GameInProgressQQ()) ? gNumTries : (gNumTries==MAX_TRIES) ? gNumTries : gNumTries+1;
		for (i=0; i<endi; i++)
		{
			MoveTo(20+(GetWindowWidth(theWindow))/2-4*CharWidth(' '),105+i*12);
			Mymemcpy((Ptr)((long)tempStr+1), gHumanWord[i], gNumLetters);
			tempStr[0]=gNumLetters;
			DrawString(tempStr);
			DrawString("\p    ");
			NumToString(gNumRight[i], tempStr);
			DrawChar(tempStr[1]);
		}
	
		theRect.top=90;
		theRect.bottom=122;
		theRect.left=43;
		theRect.right=theRect.left+32;
		for (i=0; i<3; i++)
		{
			if (theDepth>2)
				PlotCIcon(&theRect, gColorIcons[i]);
			else
				PlotIcon(&theRect, gBWIcons[i]);
			theRect.top+=35;
			theRect.bottom+=35;
		}
		if (theDepth>2)
			PlotCIcon(&theRect, gColorIcons[2]);
		else
			PlotIcon(&theRect, gBWIcons[2]);
		theRect.top+=35;
		theRect.bottom+=35;
		if (theDepth>2)
			PlotCIcon(&theRect, gColorIcons[1]);
		else
			PlotIcon(&theRect, gBWIcons[1]);
		
		oldChar=gWhichChar;
		gWhichChar=0x0a;
		DrawOneChar(theWindow);
		gWhichChar=oldChar;
	}
	
	if (theDepth>2)
	{
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);
	}
}

Boolean CopybitsTheMainWindow(WindowPtr theWindow, WindowPtr offscreenWindowPtr)
/* returns TRUE if copybits has been dealt with, FALSE if it still needs to be done */
{
	if ((gAnimation) && (gWhichWipe!=0))
	{
		SetPort(theWindow);
		FillRect(&(theWindow->portRect), &qd.black);
		DoTheWipe(offscreenWindowPtr, theWindow);
		
		return TRUE;
	}
	else gWhichWipe=0;
	
	return FALSE;
}

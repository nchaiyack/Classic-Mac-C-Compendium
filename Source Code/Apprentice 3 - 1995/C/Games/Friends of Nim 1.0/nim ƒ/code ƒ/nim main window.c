#include "nim main window.h"
#include "nim globals.h"
#include "nim endgame.h"
#include "nim dispatch.h"
#include "window layer.h"
#include "memory layer.h"
#include "text layer.h"
#include "pict utilities.h"
#include "turbo peano.h"
#include "program globals.h"

#define kNimWindowWidth			510
#define kNimWindowHeight		287
#define kNimWindowTitleResID	301
#define kCalligraphyFont		36

MyOffscreenPtr	gTheOtherWorld=0L;

static	void GetNimWindowTitle(short game, Str255 titleStr);
static	void DrawStringCentered(Str255 theString, short leftx, short rightx, short y);

void SetupTheMainWindow(WindowRef theWindow)
{
	Str255			titleStr;
	FSSpec			fs;
	
	SetWindowHeight(theWindow, kNimWindowHeight);
	SetWindowWidth(theWindow, kNimWindowWidth);
	SetWindowAttributes(theWindow, kHasDocumentTitlebarMask+kHasCloseBoxMask);
	SetWindowMaxDepth(theWindow, 8);
	GetNimWindowTitle(gGameType, titleStr);
	SetWindowTitle(theWindow, titleStr);
	SetWindowAutoCenter(theWindow, TRUE);
	SetWindowIsModified(theWindow, FALSE);
	fs.name[0]=0x00;
	SetWindowFS(theWindow, fs);
}

void OpenTheMainWindow(WindowRef theWindow)
{
	Boolean			depthChanged;
	
	if (gTheOtherWorld==0L)
	{
		gTheOtherWorld=(MyOffscreenPtr)NewPtr(sizeof(MyOffscreenRec));
		gTheOtherWorld=AllocateOffscreenWorld(theWindow, &depthChanged);
		SetPortToOnscreenWindow(theWindow, gTheOtherWorld);
	}
	SetWindowHasPermanentOffscreenWorld(theWindow, TRUE);
	SetWindowPermanentOffscreenWorld(theWindow, gTheOtherWorld);
	SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
}

void IdleInMainWindow(WindowRef theWindow)
{
	if (gGameStatus!=kGameInProgress)
		return;

	if (IsComputersTurnQQ())
		NimIdleDispatch(gGameType, theWindow);
}

void KeyPressedInMainWindow(WindowRef theWindow)
{
	if (gGameStatus==kShowingEndGame)
	{
		gGameStatus=kGameDone;
		SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
		UpdateTheWindow(theWindow);
	}
}

void MouseClickedInMainWindow(WindowRef theWindow, Point thePoint)
{
	Boolean			humanMove;
	
	if (gGameStatus==kShowingEndGame)
	{
		gGameStatus=kGameDone;
		SetWindowOffscreenNeedsUpdate(theWindow, TRUE);
		UpdateTheWindow(theWindow);
	}

	if (gGameStatus!=kGameInProgress)
		return;
	
	humanMove=(gPlayerMode==kHumanHumanMode) || ((gPlayerMode==kHumanMacMode) && (gPlayer==kFirstPlayer)) ||
		((gPlayerMode==kMacHumanMode) && (gPlayer==kSecondPlayer));
	if (!humanMove)
		return;
	
	NimClickDispatch(gGameType, theWindow, thePoint);
}

void DrawTheMainWindow(WindowRef theWindow, short theDepth)
{
	GrafPtr			curPort;
	Str255			theStr;
	
	GetPort(&curPort);
	DrawPeano(theDepth);
	if (gGameStatus==kShowingEndGame)
	{
		GetWinnerString(theStr);
		SetTheDrawingFont(kCalligraphyFont, 12, 0, srcXor);
		DrawStringCentered(theStr, 0, GetWindowWidth(theWindow), GetWindowHeight(theWindow)/2);
		UseDefaultDrawingFont();
	}
	else NimDrawDispatch(gGameType, theWindow, theDepth);
}

Boolean CloseTheMainWindow(void)
{
	gGameStatus=kNoGameStarted;
	return TRUE;
}

void UndoInMainWindow(WindowRef theWindow)
{
	#pragma unused(theWindow)
// more here if we want to support undo
}

static	void GetNimWindowTitle(short game, Str255 titleStr)
{
	GetIndString(titleStr, kNimWindowTitleResID, game+1);
}

static	void DrawStringCentered(Str255 theString, short leftx, short rightx, short y)
{
	MoveTo(leftx+((rightx-leftx)/2)-(StringWidth(theString)/2), y);
	DrawString(theString);
}

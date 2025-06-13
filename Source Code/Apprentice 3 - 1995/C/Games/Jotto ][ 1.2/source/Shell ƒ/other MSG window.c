#include "other MSG window.h"
#include "environment.h"
#include "menus.h"
#include "util.h"
#include "file utilities.h"
#include "main.h"
#include "text twiddling.h"
#include "generic window handlers.h"
#include "graphics.h"
#include "window layer.h"
#include "program globals.h"
#include <Folders.h>

#define kGrowBoxSize		15
#define PROGRAMS_LIST_NAME	"\pMSG programs list"
#define kListResourceID		151
#define LIST_CREATOR		'ttxt'
#define LIST_TYPE			'ttro'

static	Boolean GetListFromDisk(void);
static	Boolean SetupNewListFile(short fileID);
static	Boolean GetListFromResource(void);
static	unsigned long GetModificationDate(FSSpec *theFS);
static	void PutProductListIntoTE(WindowPtr theWindow);

static	short			gOldForegroundTime;		/* stored foreground wait time */
static	Boolean			gIsActive=FALSE;
static	Handle			gTheList=0L;
static	Boolean			gSetupDone=FALSE;

void SetupTheOtherMSGWindow(WindowPtr theWindow)
{
	unsigned char	*titleStr="\pOther MSG products";
	Point			topLeft;
	
	SetWindowHeight(theWindow,
		qd.screenBits.bounds.bottom-qd.screenBits.bounds.top-LMGetMBarHeight()-28);
	SetWindowWidth(theWindow, qd.screenBits.bounds.right-qd.screenBits.bounds.left-70);
	SetWindowType(theWindow, zoomDocProc);
	topLeft.v=qd.screenBits.bounds.top+LMGetMBarHeight()+20;
	topLeft.h=qd.screenBits.bounds.left+10;
	SetWindowTopLeft(theWindow, topLeft);
	SetWindowHasCloseBox(theWindow, TRUE);
	SetWindowMaxDepth(theWindow, 1);
	SetWindowDepth(theWindow, 1);
	SetWindowAutoCenter(theWindow, FALSE);
	SetWindowTitle(theWindow, titleStr);
	SetWindowIsFloat(theWindow, FALSE);
	
	if (gSetupDone)
		return;
	
	if (!GetListFromDisk())
		GetListFromResource();
}

void OpenTheOtherMSGWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	FontInfo		theFontInfo;
	Rect			vScrollBarRect, hScrollBarRect;
	Rect			destRect, viewRect;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
	{
		SetRect(&vScrollBarRect, GetWindowWidth(theWindow)-kGrowBoxSize, -1,
			GetWindowWidth(theWindow)+1, GetWindowHeight(theWindow)+1-kGrowBoxSize);
		SetRect(&hScrollBarRect, -1, GetWindowHeight(theWindow)-kGrowBoxSize,
			GetWindowWidth(theWindow)-kGrowBoxSize+1, GetWindowHeight(theWindow)+1);
		SetWindowVScrollBar(theWindow,
			NewControl(theWindow, &vScrollBarRect, "\p", TRUE, 0, 0, 0, scrollBarProc, 0));
		SetWindowHScrollBar(theWindow,
			NewControl(theWindow, &hScrollBarRect, "\p", TRUE, 0, 0, 0, scrollBarProc, 0));
		
		GetTERect(theWindow, &destRect, TRUE);
		viewRect=destRect;
		hTE=TENew(&destRect, &viewRect);
		SetWindowTE(theWindow, hTE);
		TextFont((**hTE).txFont=36);
		TextSize((**hTE).txSize=12);
		TextFace((**hTE).txFace=0);
		GetFontInfo(&theFontInfo);
		(**hTE).fontAscent=theFontInfo.ascent;
		(**hTE).lineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
		AdjustViewRect(hTE);
		TEAutoView(TRUE, hTE);
		TESetClickLoop((TEClickLoopUPP)MyClikLoop, hTE);
		PutProductListIntoTE(theWindow);
	}
	
	gIsActive=TRUE;
	AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	AdjustMenus();
}

void DisposeTheOtherMSGWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE!=0L)
	{
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
}

void ActivateTheOtherMSGWindow(WindowPtr theWindow)
{
	gOldForegroundTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
	gIsActive=TRUE;
}

void DeactivateTheOtherMSGWindow(WindowPtr theWindow)
{
	gForegroundWaitTime=gOldForegroundTime;
	gCustomCursor=FALSE;
}

void CopybitsTheOtherMSGWindow(WindowPtr theWindow, WindowPtr offscreenWindowPtr)
{
	GenericCopybits(theWindow, offscreenWindowPtr, gIsActive);
}

/* ---------------------------------------------------- */
/* the rest of these are internal to other MSG window.h */

static	void PutProductListIntoTE(WindowPtr theWindow)
{
	long			listEOF;
	
	if (gTheList==0L)
		return;
	
	listEOF=GetHandleSize(gTheList);
	HLock(gTheList);
	SetTheText(theWindow, *gTheList, listEOF);
	HUnlock(gTheList);
	TESetSelect(0, 0, GetWindowTE(theWindow));
}

static	Boolean GetListFromDisk(void)
{
	Boolean			isNewFile;
	unsigned char	*name=PROGRAMS_LIST_NAME;
	OSErr			isHuman;
	short			vRefNum;
	long			dirID;
	FSSpec			listFileFS, progFS;
	short			listFile;
	long			listEOF;
	unsigned long	diskListModDate, ourListModDate;
	
	isNewFile=FALSE;
	/* find vRefNum and dirID of preferences folder, creating it if necessary */
	isHuman=FindFolder(kOnSystemDisk, 'pref', kCreateFolder, &vRefNum, &dirID);
	
	if (isHuman!=noErr)		/* screwed up already?!? */
		return FALSE;
	
	isHuman=FSMakeFSSpec(vRefNum, dirID, name, &listFileFS);	/* make FSSpec out of it */
	if (isHuman!=noErr)
	{
		if (isHuman==fnfErr)	/* FSSpec is valid, but list file does not exist */
		{
			isHuman=FSpCreate(&listFileFS, LIST_CREATOR, LIST_TYPE, 0);	/* so create it */
			if (isHuman!=noErr)										/* or not */
				return FALSE;
			isNewFile=TRUE;		/* signal that list file is new */
		}
		else return FALSE;
	}
	isHuman=FSpOpenDF(&listFileFS, fsRdWrPerm, &listFile);	/* open list file */
	if (isHuman!=noErr)
		return FALSE;
	
	diskListModDate=GetModificationDate(&listFileFS);
	GetApplicationFSSpec(&progFS);
	ourListModDate=GetModificationDate(&progFS);
	if (diskListModDate<ourListModDate)
		isNewFile=TRUE;
	
	if (isNewFile)
	{
		if (!SetupNewListFile(listFile))
		{
			FSClose(listFile);
			FSpDelete(&listFileFS);
			return FALSE;
		}
		SetModificationDate(&listFileFS, ourListModDate);
	}
	else
	{
		GetEOF(listFile, &listEOF);
		gTheList=NewHandle(listEOF);
		if (gTheList==0L)
		{
			FSClose(listFile);
			FSpDelete(&listFileFS);
			return FALSE;
		}
		HLock(gTheList);
		SetFPos(listFile, 1, 0L);
		if (FSRead(listFile, &listEOF, *gTheList)!=noErr)
		{
			FSClose(listFile);
			FSpDelete(&listFileFS);
			DisposeHandle(gTheList);
			gTheList=0L;
			return FALSE;
		}
		HUnlock(gTheList);
	}
	
	FSClose(listFile);
//	FlushVol(dirID, vRefNum);
	
	return TRUE;
}

static	Boolean SetupNewListFile(short fileID)
{
	long			listEOF;
	
	if (!GetListFromResource())
		return FALSE;
	
	listEOF=GetHandleSize(gTheList);
	if (SetEOF(fileID, listEOF)!=noErr)
		return FALSE;
	
	SetFPos(fileID, 1, 0L);
	HLock(gTheList);
	if (FSWrite(fileID, &listEOF, *gTheList)!=noErr)
		return FALSE;
	HUnlock(gTheList);
	
	return TRUE;
}

static	Boolean GetListFromResource(void)
{
	gTheList=(Handle)Get1Resource('TEXT', kListResourceID);
	if (gTheList==0L)
		return FALSE;
	if (*gTheList==0L)
		LoadResource(gTheList);
	if (*gTheList==0L)
	{
		DisposeHandle(gTheList);
		gTheList=0L;
		return FALSE;
	}
	
	DetachResource(gTheList);
	
	return TRUE;
}

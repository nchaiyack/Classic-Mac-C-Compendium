#include "kant build lists.h"
#include "list layer.h"
#include "window layer.h"
#include "text layer.h"
#include "resource layer.h"
#include "memory layer.h"
#include <Icons.h>

enum { kReferenceList=0, kInstantList };

#define kReferenceListLineHeight	19
#define kInstantListLineHeight		14

#define kInterestingIconID			650
#define kReferenceLDEF				128
#define kInstantLDEF				129
#define kGrowBoxSize				15

static	short			gWhichListActive;
static	ListHandle		gReferenceListHandle=0L;
static	ListHandle		gInstantListHandle=0L;
static	Rect			gReferenceListRect;
static	Rect			gInstantListRect;
static	short			gMaxRefsToDisplay;
static	Handle			gInterestingIconHandle=0L;

void InitTheBuildLists(void)
/* don't call this more than once */
{
	GetIconSuite(&gInterestingIconHandle, kInterestingIconID, svAllSmallData);
	gReferenceListHandle=gInstantListHandle=0L;
	gWhichListActive=kReferenceList;
}

ListHandle GetReferenceListHandle(void)
{
	return gReferenceListHandle;
}

ListHandle GetInstantListHandle(void)
{
	return gInstantListHandle;
}

void GetReferenceListRect(Rect *theRect)
{
	*theRect=gReferenceListRect;
}

void GetInstantListRect(Rect *theRect)
{
	*theRect=gInstantListRect;
}

Boolean ReferenceListActiveQQ(void)
{
	return (gWhichListActive==kReferenceList);
}

Boolean InstantListActiveQQ(void)
{
	return (gWhichListActive==kInstantList);
}

Boolean HighlightedReferenceInterestingQQ(void)
{
	Cell			theCell;
	Str255			itemStr;
	Handle			iconHandle;
	
	if (MyGetFirstSelectedCell(gReferenceListHandle, &theCell))
	{
		MyGetCellData(gReferenceListHandle, theCell, itemStr);
		Mymemcpy((Ptr)&iconHandle, (Ptr)&itemStr[1], 4);
		return (iconHandle!=0L);
	}
	
	return FALSE;
}

Boolean SetHighlightedReferenceInteresting(Boolean isInteresting)
{
	Cell			theCell;
	Str255			itemStr;
	
	if (MyGetFirstSelectedCell(gReferenceListHandle, &theCell))
	{
		MyGetCellData(gReferenceListHandle, theCell, itemStr);
		if (isInteresting)
		{
			Mymemcpy((Ptr)&itemStr[1], (Ptr)&gInterestingIconHandle, 4);
		}
		else
		{
			Mymemset((Ptr)&itemStr[1], 0, 4);
		}
		MySetCellData(gReferenceListHandle, theCell, itemStr);
	}
	
	return FALSE;
}

void SetReferenceListActive(Boolean redraw)
{
	if (gWhichListActive!=kReferenceList)
	{
		gWhichListActive=kReferenceList;
		if (redraw)
		{
			UpdateBuildLists((**gReferenceListHandle).port);
		}
	}
}

void SetInstantListActive(Boolean redraw)
{
	if (gWhichListActive!=kInstantList)
	{
		gWhichListActive=kInstantList;
		if (redraw)
		{
			UpdateBuildLists((**gInstantListHandle).port);
		}
	}
}

void SetMaxReferenceDisplay(short numRefs, Boolean redraw)
{
	gMaxRefsToDisplay=numRefs;
	if (redraw)
	{
// more stuff here to resize and display (not currently an option)
	}
}

void SetHighlightedReference(short refNum)
{
	Cell			theCell;
	
	SetPt(&theCell, 0, refNum);
	MySelectOneCell(gReferenceListHandle, theCell);
}

void SetHighlightedInstant(short instantNum)
{
	Cell			theCell;
	
	SetPt(&theCell, 0, instantNum);
	MySelectOneCell(gInstantListHandle, theCell);
}

short GetNumberOfReferences(void)
{
	return (**gReferenceListHandle).dataBounds.bottom;
}

short GetNumberOfInstants(void)
{
	return (**gInstantListHandle).dataBounds.bottom;
}

short GetNumberOfVisibleReferences(void)
{
	return (**gReferenceListHandle).visible.bottom-(**gReferenceListHandle).visible.top;
}

short GetNumberOfVisibleInstants(void)
{
	return (**gInstantListHandle).visible.bottom-(**gInstantListHandle).visible.top;
}

short GetHighlightedReference(void)
{
	Cell			theCell;
	
	if (MyGetFirstSelectedCell(gReferenceListHandle, &theCell))
		return theCell.v;
	else
		return -1;
}

short GetHighlightedInstant(void)
{
	Cell			theCell;
	
	if (MyGetFirstSelectedCell(gInstantListHandle, &theCell))
		return theCell.v;
	else
		return -1;
}

void GetIndReferenceName(Str255 theStr, short index)
{
	Cell			theCell;
	Str255			itemStr;
	
	if ((index<0) || (index>=GetNumberOfReferences()))
	{
		theStr[0]=0x00;
		return;
	}
	
	SetPt(&theCell, 0, index);
	MyGetCellData(gReferenceListHandle, theCell, itemStr);
	Mymemcpy((Ptr)&theStr[1], (Ptr)&itemStr[5], itemStr[5]-4);
	theStr[0]=itemStr[0]-4;
}

void GetIndInstantName(Str255 theStr, short index)
{
	Cell			theCell;
	
	if ((index<0) || (index>=GetNumberOfInstants()))
	{
		theStr[0]=0x00;
		return;
	}
	
	SetPt(&theCell, 0, index);
	MyGetCellData(gInstantListHandle, theCell, theStr);
}

void GetHighlightedReferenceName(Str255 theStr)
{
	Cell			theCell;
	Str255			itemStr;
	
	if (MyGetFirstSelectedCell(gReferenceListHandle, &theCell))
	{
		MyGetCellData(gReferenceListHandle, theCell, itemStr);
		Mymemcpy((Ptr)&theStr[1], (Ptr)&itemStr[5], itemStr[5]-4);
		theStr[0]=itemStr[0]-4;
	}
	else theStr[0]=0x00;
}

void GetHighlightedInstantName(Str255 theStr)
{
	Cell			theCell;
	
	if (MyGetFirstSelectedCell(gInstantListHandle, &theCell))
	{
		MyGetCellData(gInstantListHandle, theCell, theStr);
	}
	else theStr[0]=0x00;
}

Boolean DeleteHighlightedReference(void)
{
	Cell			theCell;
	
	if (MyGetFirstSelectedCell(gReferenceListHandle, &theCell))
	{
		MyDeleteItemFromList(gReferenceListHandle, theCell.v);
		BuildInstantListHandle(GetWindowFS((**gReferenceListHandle).port), TRUE);
		return TRUE;
	}
	
	return FALSE;
}

Boolean DeleteHighlightedInstantiation(void)
{
	Cell			theCell;
	
	if (MyGetFirstSelectedCell(gInstantListHandle, &theCell))
	{
		MyDeleteItemFromList(gInstantListHandle, theCell.v);
		return TRUE;
	}
	
	return FALSE;
}

void AddReferenceToList(Str255 theStr, Boolean isInteresting, Boolean autoSelect)
{
	Str255			itemStr;
	Cell			theCell;
	
	itemStr[0]=0x04;
	if (isInteresting)
	{
		Mymemcpy((Ptr)&itemStr[1], (Ptr)&gInterestingIconHandle, 4);
	}
	else
	{
		Mymemset((Ptr)&itemStr[1], 0, 4);
	}
	Mymemcpy((Ptr)&itemStr[5], (Ptr)&theStr[1], theStr[0]);
	itemStr[0]+=theStr[0];
	if ((!ReferenceListActiveQQ()) && autoSelect)
		SetReferenceListActive(TRUE);
	
	MyAddStr255ToList(gReferenceListHandle, itemStr);
	if (autoSelect)
	{
		SetPt(&theCell, 0, GetNumberOfReferences()-1);
		MySelectOneCell(gReferenceListHandle, theCell);
		BuildInstantListHandle(GetWindowFS((**gReferenceListHandle).port), TRUE);
	}
}

void AddInstantToList(Str255 theStr, Boolean autoSelect)
{
	Cell			theCell;
	
	if ((!InstantListActiveQQ()) && autoSelect)
		SetInstantListActive(TRUE);
	
	MyAddStr255ToList(gInstantListHandle, theStr);
	if (autoSelect)
	{
		SetPt(&theCell, 0, GetNumberOfInstants()-1);
		MySelectOneCell(gInstantListHandle, theCell);
	}
}

void EditReferenceInList(short index, Str255 newStr, Boolean isInteresting)
/* index is 0-based */
{
	Str255			itemStr;
	Cell			theCell;
	
	itemStr[0]=0x04;
	if (isInteresting)
	{
		Mymemcpy((Ptr)&itemStr[1], (Ptr)&gInterestingIconHandle, 4);
	}
	else
	{
		Mymemset((Ptr)&itemStr[1], 0, 4);
	}
	Mymemcpy((Ptr)&itemStr[5], (Ptr)&newStr[1], newStr[0]);
	itemStr[0]+=newStr[0];
	SetPt(&theCell, 0, index);
	MySetCellData(gReferenceListHandle, theCell, itemStr);
}

void EditInstantInList(short index, Str255 newStr)
/* index is 0-based */
{
	Cell			theCell;
	
	SetPt(&theCell, 0, index);
	MySetCellData(gInstantListHandle, theCell, newStr);
}

void SetBuildListRects(WindowRef theWindow, short hSpace, short vSpace, short headerHeight)
{
	short			refBottom;
	
	SetPort(theWindow);
	refBottom=kReferenceListLineHeight*gMaxRefsToDisplay+headerHeight+vSpace;
	SetRect(&gReferenceListRect, hSpace, headerHeight+vSpace, GetWindowWidth(theWindow)-hSpace,
		refBottom);
	SetRect(&gInstantListRect, hSpace, refBottom+vSpace,
		GetWindowWidth(theWindow)-hSpace, GetWindowHeight(theWindow)-kGrowBoxSize-vSpace);
}

void CreateBuildLists(WindowRef theWindow)
{
	gReferenceListHandle=MyCreateVerticalScrollingList(theWindow, gReferenceListRect, 1,
		kReferenceLDEF, kReferenceListLineHeight);
	MySetListSelectionFlags(gReferenceListHandle, lOnlyOne);
	gInstantListHandle=MyCreateVerticalScrollingList(theWindow, gInstantListRect, 1,
		kInstantLDEF, kInstantListLineHeight);
	MySetListSelectionFlags(gInstantListHandle, lOnlyOne);
}

OSErr BuildReferenceListHandle(FSSpec theFS, Boolean redraw)
{
	OSErr			oe;
	short			oldRefNum, refNum, numStringResources, resID, i;
	Boolean			alreadyOpen;
	Str255			resName;
	ResType			resType;
	Handle			resHandle;
	short			resAttrs;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, TRUE))!=noErr)
		return oe;
	
	LSetDrawingMode(FALSE, gReferenceListHandle);
	MyClearAllCells(gReferenceListHandle);
	
	numStringResources=Count1Resources('STR#');
	for (i=1; i<=numStringResources; i++)
	{
		resHandle=Get1IndResource('STR#', i);
		if (resHandle==0L)
		{
			LSetDrawingMode(TRUE, gReferenceListHandle);
			return ResError();
		}
		
		GetResInfo(resHandle, &resID, &resType, resName);
		oe=ResError();
		if (oe!=noErr)
		{
			ReleaseResource(resHandle);
			CloseTheResFile(oldRefNum, refNum, alreadyOpen);
			LSetDrawingMode(TRUE, gReferenceListHandle);
			return oe;
		}
		resAttrs=GetResAttrs(resHandle);
		oe=ResError();
		ReleaseResource(resHandle);
		if (oe!=noErr)
		{
			CloseTheResFile(oldRefNum, refNum, alreadyOpen);
			LSetDrawingMode(TRUE, gReferenceListHandle);
			return oe;
		}
		
		AddReferenceToList(resName, (resAttrs&0x00ff) & resLocked, FALSE);
	}
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	LSetDrawingMode(TRUE, gReferenceListHandle);
	
	if (redraw)
		MyUpdateList(gReferenceListHandle);
	
	return noErr;
}

OSErr BuildInstantListHandle(FSSpec theFS, Boolean redraw)
{
	OSErr			oe;
	short			oldRefNum, refNum, numStringsInResource, resID, j;
	Boolean			alreadyOpen;
	Str255			resName, theName;
	ResType			resType;
	Handle			resHandle;
	Str255			refName;
	Rect			eraseRect;
	
	if (GetHighlightedReference()<0)
	{
		EraseRect(&gInstantListRect);
		return noErr;
	}
	
	GetHighlightedReferenceName(refName);
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, TRUE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', refName);
	if (resHandle==0L)
		return ResError();
	
	LSetDrawingMode(FALSE, gInstantListHandle);
	MyClearAllCells(gInstantListHandle);
	
	GetResInfo(resHandle, &resID, &resType, resName);
	
	numStringsInResource=**((short**)resHandle);
	for (j=1; j<=numStringsInResource; j++)
	{
		GetIndString(theName, resID, j);
		AddInstantToList(theName, FALSE);
	}
	
	ReleaseResource(resHandle);
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	LSetDrawingMode(TRUE, gInstantListHandle);
	
	if (redraw)
	{
		eraseRect=gInstantListRect;
		eraseRect.right-=16;
		EraseRect(&eraseRect);
		MyUpdateList(gInstantListHandle);
	}
	
	return noErr;
}

void UpdateBuildLists(WindowRef theWindow)
{
	LUpdate(theWindow->visRgn, gReferenceListHandle);
	MyDrawListBorder(gReferenceListHandle);
	MyDrawActiveListBorder(gReferenceListHandle, ReferenceListActiveQQ());
	LUpdate(theWindow->visRgn, gInstantListHandle);
	MyDrawListBorder(gInstantListHandle);
	MyDrawActiveListBorder(gInstantListHandle, InstantListActiveQQ());
}

void ResizeBuildLists(WindowRef theWindow, short hSpace, short vSpace, short headerHeight)
{
	short			refBottom;
	
	LSetDrawingMode(FALSE, gReferenceListHandle);
	LSetDrawingMode(FALSE, gInstantListHandle);
	refBottom=kReferenceListLineHeight*gMaxRefsToDisplay+headerHeight+vSpace;
	SetRect(&gReferenceListRect, hSpace, headerHeight+vSpace, GetWindowWidth(theWindow)-hSpace,
		refBottom);
	SetRect(&gInstantListRect, hSpace, refBottom+vSpace,
		GetWindowWidth(theWindow)-hSpace, GetWindowHeight(theWindow)-kGrowBoxSize-vSpace);
	MySetListRect(gReferenceListHandle, gReferenceListRect);
	MySetListRect(gInstantListHandle, gInstantListRect);
	MySetListSize(gReferenceListHandle, gReferenceListRect.right-gReferenceListRect.left-16,
		gReferenceListRect.bottom-gReferenceListRect.top, kReferenceListLineHeight);
	MySetListSize(gInstantListHandle, gInstantListRect.right-gInstantListRect.left-16,
		gInstantListRect.bottom-gInstantListRect.top, kInstantListLineHeight);
	MyDrawListBorder(gReferenceListHandle);
	MyDrawListBorder(gInstantListHandle);
	if (ReferenceListActiveQQ())
		MyDrawActiveListBorder(gReferenceListHandle, TRUE);
	if (InstantListActiveQQ())
		MyDrawActiveListBorder(gInstantListHandle, TRUE);
	LSetDrawingMode(TRUE, gReferenceListHandle);
	LSetDrawingMode(TRUE, gInstantListHandle);
	InvalRect(&(theWindow->portRect));
}

void GetGrowSizeTheLists(Rect *sizeRect, short hSpace, short vSpace, short headerHeight)
{
	#pragma unused(hSpace)
	
	SetRect(sizeRect, 294, 48+kGrowBoxSize+1+headerHeight+kReferenceListLineHeight*gMaxRefsToDisplay+vSpace*5,
		32766, 32767);
}

void DisposeBuildLists(void)
{
	if (gReferenceListHandle!=0L)
		LDispose(gReferenceListHandle);
	if (gInstantListHandle!=0L)
		LDispose(gInstantListHandle);
	gReferenceListHandle=gInstantListHandle=0L;
}

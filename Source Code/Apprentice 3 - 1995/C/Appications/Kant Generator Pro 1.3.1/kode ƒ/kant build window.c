#include "kant build window.h"
#include "kant build lists.h"
#include "kant build dispatch.h"
#include "program globals.h"
#include "button layer.h"
#include "environment.h"
#include "key layer.h"
#include "window layer.h"
#include "menus.h"
#include "main.h"
#include "text layer.h"
#include "resource layer.h"
#include "memory layer.h"
#include "list layer.h"
#include <Icons.h>

#define DEAD_SPACE_H		10
#define DEAD_SPACE_V		10
#define BUTTON_GAP			1
#define BUTTON_WIDTH		24
#define BUTTON_HEIGHT		24
#define MESSAGE_HEIGHT		19
#define HEADER_SPACE		(DEAD_SPACE_V*3+BUTTON_HEIGHT+MESSAGE_HEIGHT)
#define HEADER_NO_MESSAGE	(DEAD_SPACE_V*2+BUTTON_HEIGHT)
#define HEADER_NO_TOOLBAR	0
#define kHeaderHeight		(gShowToolbar ? (gShowMessageBox ? HEADER_SPACE : HEADER_NO_MESSAGE) : HEADER_NO_TOOLBAR)
#define kGrowBoxSize		15

#define kFirstBuildButtonID	600
#define kBuildMessagesID	500
#define NUM_BUTTONS			7

enum { kNewRefButton=0, kNewInstantButton, kEditRefButton, kDeleteRefButton,
		kMarkInterestingButton, kShowMessageButton, kHideToolbarButton };

enum { kNewRefMessage=1, kNewInstantMessage, kEditRefMessage, kDeleteRefMessage,
		kMarkInterestingMessage, kHideMessageMessage, kHideToolbarMessage,
		kEditInstantMessage, kDeleteInstantMessage, kMarkUninterestingMessage,
		kBlankMessage, kNewInstantDimmedMessage, kEditRefDimmedMessage,
		kDeleteRefDimmedMessage, kMarkInterestingDimmedMessage, kEditInstantDimmedMessage,
		kDeleteInstantDimmedMessage };

static	Boolean ButtonIsDimmedQQ(WindowRef theWindow, short buttonNum);
static	void SetTheBuildMessage(WindowRef theWindow, short messageIndex, Boolean redraw);
static	void DrawTheBuildMessage(WindowRef theWindow, Boolean drawFrame);
static	void DrawBuildHeader(WindowRef theWindow, short theDepth);

static	Str255			gBuildWindowTitle;
static	Rect			gButtonRect[NUM_BUTTONS];
static	Handle			gButtonIcon[NUM_BUTTONS];	/* handle to icon suite */
static	Str255			gTheMessage;
static	short			gTheMessageID;
static	Boolean			gOldButtonState[NUM_BUTTONS];
static	Boolean			gSetupDone=FALSE;

void SetupTheBuildWindow(WindowRef theWindow)
{
	unsigned char	*titleStr="\puntitled";
	Point			topLeft;
	short			i;
	
	SetWindowHeight(theWindow, qd.screenBits.bounds.bottom-qd.screenBits.bounds.top-LMGetMBarHeight()-48);
	SetWindowWidth(theWindow, qd.screenBits.bounds.right-qd.screenBits.bounds.left-70);
	SetWindowAttributes(theWindow, kHasCloseBoxMask+kHasZoomBoxMask+kHasGrowBoxMask+kHasDocumentTitlebarMask);
	topLeft.v=qd.screenBits.bounds.top+LMGetMBarHeight()+40;
	topLeft.h=qd.screenBits.bounds.left+30;
	SetWindowTopLeft(theWindow, topLeft);
	SetWindowMaxDepth(theWindow, 8);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowIsZoomable(theWindow, TRUE);
	SetWindowIsPrintable(theWindow, TRUE);
	if (gBuildWindowTitle[0]==0x00)
	{
		SetWindowTitle(theWindow, titleStr);
	}
	else
	{
		SetWindowTitle(theWindow, gBuildWindowTitle);
	}
	gBuildWindowTitle[0]=0x00;
	SetWindowAutoCenter(theWindow, FALSE);
	SetWindowIsModified(theWindow, FALSE);
	SetWindowDrawGrowIconLines(theWindow, FALSE);
	
	if (gSetupDone)
		return;
	
	gSetupDone=TRUE;
	gNeedToOpenWindow=FALSE;
	for (i=0; i<NUM_BUTTONS; i++)
	{
		SetRect(&gButtonRect[i], DEAD_SPACE_H+i*(BUTTON_WIDTH+BUTTON_GAP), DEAD_SPACE_V,
			DEAD_SPACE_H+i*(BUTTON_WIDTH+BUTTON_GAP)+BUTTON_WIDTH, DEAD_SPACE_V+BUTTON_HEIGHT);
		GetIconSuite(&gButtonIcon[i], kFirstBuildButtonID+i, svAllSmallData);
	}
	SetTheBuildMessage(theWindow, 0, FALSE);
	InitTheBuildLists();
}

void ShutDownTheBuildWindow(void)
{
	short			i;
	
	for (i=0; i<NUM_BUTTONS; i++)
		gButtonIcon[i]=SafeDisposeIconSuite(gButtonIcon[i]);
}

void OpenTheBuildWindow(WindowRef theWindow)
{
	theWindow->txFont=geneva;
	theWindow->txSize=9;
	
	SetMaxReferenceDisplay(5, FALSE);
	SetBuildListRects(theWindow, DEAD_SPACE_H, DEAD_SPACE_V, kHeaderHeight);
	CreateBuildLists(theWindow);
	SetReferenceListActive(FALSE);
	SetWindowIsActive(theWindow, TRUE);
	AdjustMenus();
}

void IdleInBuildWindow(WindowRef theWindow, Point mouseLoc)
{
	short			i;
	Boolean			gotone;
	short			id;
	
	if (!gShowToolbar || !gShowMessageBox)
		return;
	
	for (i=0, gotone=FALSE; ((i<NUM_BUTTONS) && (!gotone)); i++)
	{
		if (PtInRect(mouseLoc, &gButtonRect[i]))
		{
			gotone=TRUE;
			switch (i)
			{
				case kNewRefButton:
					id=kNewRefMessage;
					break;
				case kNewInstantButton:
					id=(ButtonIsDimmedQQ(theWindow, i)) ? kNewInstantDimmedMessage : kNewInstantMessage;
					break;
				case kEditRefButton:
					id=(ReferenceListActiveQQ()) ?
						(ButtonIsDimmedQQ(theWindow, i)) ? kEditRefDimmedMessage : kEditRefMessage :
						(ButtonIsDimmedQQ(theWindow, i)) ? kEditInstantDimmedMessage : kEditInstantMessage;
					break;
				case kDeleteRefButton:
					id=(ReferenceListActiveQQ()) ?
						(ButtonIsDimmedQQ(theWindow, i)) ? kDeleteRefDimmedMessage : kDeleteRefMessage :
						(ButtonIsDimmedQQ(theWindow, i)) ? kDeleteInstantDimmedMessage : kDeleteInstantMessage;
					break;
				case kMarkInterestingButton:
					id=(ButtonIsDimmedQQ(theWindow, i)) ? kMarkInterestingDimmedMessage :
						(HighlightedReferenceInterestingQQ()) ? kMarkUninterestingMessage :
						kMarkInterestingMessage;
					break;
				case kShowMessageButton:
					id=kHideMessageMessage;
					break;
				case kHideToolbarButton:
					id=kHideToolbarMessage;
					break;
			}
			if (id!=gTheMessageID)
				SetTheBuildMessage(theWindow, id, TRUE);
		}
	}
	
	if ((!gotone) && (gTheMessageID!=kBlankMessage))
		SetTheBuildMessage(theWindow, kBlankMessage, TRUE);
}

void KeyPressedInBuildWindow(WindowRef theWindow, unsigned char theChar)
{
	Boolean			refActive;
	short			highlightedIndex, newIndex;
	short			max, numVisible;
	ListHandle		theList;
	Cell			theCell;
	
	refActive=ReferenceListActiveQQ();
	theList=refActive ? GetReferenceListHandle() : GetInstantListHandle();
	highlightedIndex=refActive ? GetHighlightedReference() : GetHighlightedInstant();
	max=refActive ? GetNumberOfReferences() : GetNumberOfInstants();
	numVisible=refActive ? GetNumberOfVisibleReferences() : GetNumberOfVisibleInstants();
	newIndex=-2;
	
	RememberBuildButtonState(theWindow);
	
	switch (theChar)
	{
		case key_Tab:
			if (refActive)
				SetInstantListActive(TRUE);
			else
				SetReferenceListActive(TRUE);
			break;
		case key_UpArrow:
		case key_LeftArrow:
			newIndex=(highlightedIndex<=0) ? max-1 : highlightedIndex-1;
			/* no break intentionally */
		case key_DownArrow:
		case key_RightArrow:
			if (newIndex==-2)
				newIndex=((highlightedIndex==-1) || (highlightedIndex==max-1)) ? 0 : highlightedIndex+1;
			SetPt(&theCell, 0, newIndex);
			MySelectOneCell(theList, theCell);
			if (refActive)
				BuildInstantListHandle(GetWindowFS(theWindow), TRUE);
			break;
		case key_Home:
			MyScrollList(theList, -max);
			break;
		case key_End:
			MyScrollList(theList, max);
			break;
		case key_PageUp:
			MyScrollList(theList, -numVisible);
			break;
		case key_PageDown:
			MyScrollList(theList, numVisible);
			break;
		case key_Return:
		case key_Enter:
			if (highlightedIndex>=0)
				DoEditDispatch(theWindow);
			break;
		case key_Delete:
			if (highlightedIndex>=0)
				DoDeleteDispatch(theWindow);
			break;
	}

	UpdateBuildButtons(theWindow, TRUE);
}

Boolean MouseClickedInBuildWindow(WindowRef theWindow, Point thePoint, Boolean dynamicScroll)
{
	#pragma unused(dynamicScroll)

	Boolean			gotone;
	short			i;
	short			theDepth;
	Rect			refRect, instantRect;
	ListHandle		refList, instantList;
	Boolean			doubleClick;
	short			oldReference;
	
	if (gInProgress)
		return TRUE;
	
	theDepth=GetWindowDepth(theWindow);
	RememberBuildButtonState(theWindow);
	
	for (i=0, gotone=FALSE; ((i<NUM_BUTTONS) && (!gotone)); i++)
	{
		if ((gShowToolbar) && (PtInRect(thePoint, &gButtonRect[i])) && (!ButtonIsDimmedQQ(theWindow, i)))
		{
			gotone=TRUE;
			if (Track3DButton(&gButtonRect[i], 0L, gButtonIcon[i], theDepth, kSquareButton))
			{
				switch (i)
				{
					case kNewRefButton:
						DoNewDispatch(theWindow, TRUE);
						break;
					case kNewInstantButton:
						DoNewDispatch(theWindow, FALSE);
						break;
					case kEditRefButton:
						DoEditDispatch(theWindow);
						break;
					case kDeleteRefButton:
						DoDeleteDispatch(theWindow);
						break;
					case kMarkInterestingButton:
						DoMarkInterestingDispatch(theWindow);
						break;
					case kShowMessageButton:
						DoShowMessageDispatch(theWindow);
						break;
					case kHideToolbarButton:
						DoShowToolbarDispatch(theWindow);
						break;
				}
			}
		}
	}
	
	if (!gotone)
	{
		refList=GetReferenceListHandle();
		instantList=GetInstantListHandle();
		GetReferenceListRect(&refRect);
		GetInstantListRect(&instantRect);
		
		if (PtInRect(thePoint, &refRect))
		{
			oldReference=GetHighlightedReference();
			SetReferenceListActive(TRUE);
			doubleClick=MyHandleMouseDownInList(refList, thePoint, GetTheModifiers());
			if (oldReference!=GetHighlightedReference())
				BuildInstantListHandle(GetWindowFS(theWindow), TRUE);
			if (doubleClick)
				DoEditDispatch(theWindow);
		}
		else if (PtInRect(thePoint, &instantRect))
		{
			SetInstantListActive(TRUE);
			if (MyHandleMouseDownInList(instantList, thePoint, GetTheModifiers()))
				DoEditDispatch(theWindow);
		}
	}
	
	UpdateBuildButtons(theWindow, TRUE);
	return TRUE;
}

void DisposeTheBuildWindow(WindowRef theWindow)
{
	#pragma unused(theWindow)

	DisposeBuildLists();
}

void CopybitsTheBuildWindow(WindowRef theWindow, WindowRef offscreenWindowRef, Boolean active,
	Boolean drawGrowIconLines)
{
	Rect			tempRect;
	RgnHandle		copyRgn, listRgn;
	ListHandle		refList, instantList;
	Rect			refRect, instantRect;
	
	refList=GetReferenceListHandle();
	instantList=GetInstantListHandle();
	GetReferenceListRect(&refRect);
	GetInstantListRect(&instantRect);
	
	copyRgn=NewRgn();
	listRgn=NewRgn();
	RectRgn(copyRgn, &(theWindow->portRect));
	RectRgn(listRgn, &refRect);
	InsetRgn(listRgn, -4, -4);
	DiffRgn(copyRgn, listRgn, copyRgn);
	SetEmptyRgn(listRgn);
	RectRgn(listRgn, &instantRect);
	InsetRgn(listRgn, -4, -4);
	DiffRgn(copyRgn, listRgn, copyRgn);
	
	CopyBits(	&(offscreenWindowRef->portBits),
				&(theWindow->portBits),
				&(theWindow->portRect),
				&(theWindow->portRect), 0, copyRgn);
	
	if (active)
	{
		DrawGrowIconNoLines(theWindow, drawGrowIconLines);
    }
	else
	{
		tempRect.bottom=theWindow->portRect.bottom;
		tempRect.right=theWindow->portRect.right;
		tempRect.left=tempRect.right-kGrowBoxSize+1;
		tempRect.top=tempRect.bottom-kGrowBoxSize+1;
		EraseRect(&tempRect);
	}
	
	UpdateBuildLists(theWindow);
	
	DisposeRgn(copyRgn);
	DisposeRgn(listRgn);
}

void DrawTheBuildWindow(WindowRef theWindow, short theDepth)
{
	RGBColor		oldForeColor, oldBackColor;
	GrafPtr			curPort;
	
	if (theDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
	}
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	
	if (kHeaderHeight>0)
	{
		DrawBuildHeader(theWindow, theDepth);
	}
	
	if (theDepth>2)
	{
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);
	}
}

void ActivateTheBuildWindow(WindowRef theWindow, Boolean drawGrowIconLines)
{
	ListHandle		refList, instantList;
	
	refList=GetReferenceListHandle();
	instantList=GetInstantListHandle();
	LActivate(TRUE, refList);
	LActivate(TRUE, instantList);
	MyDrawActiveListBorder(refList, ReferenceListActiveQQ());
	MyDrawActiveListBorder(instantList, InstantListActiveQQ());
	DrawGrowIconNoLines(theWindow, drawGrowIconLines);
}

void DeactivateTheBuildWindow(WindowRef theWindow)
{
	Rect			tempRect;
	ListHandle		refList, instantList;
	
	refList=GetReferenceListHandle();
	instantList=GetInstantListHandle();
	LActivate(FALSE, refList);
	LActivate(FALSE, instantList);
	MyDrawActiveListBorder(refList, FALSE);
	MyDrawActiveListBorder(instantList, FALSE);
	tempRect.bottom=theWindow->portRect.bottom;
	tempRect.right=theWindow->portRect.right;
	tempRect.left=tempRect.right-kGrowBoxSize+1;
	tempRect.top=tempRect.bottom-kGrowBoxSize+1;
	EraseRect(&tempRect);
}

void ResizeTheBuildWindow(WindowRef theWindow)
{
	ResizeBuildLists(theWindow, DEAD_SPACE_H, DEAD_SPACE_V, kHeaderHeight);
}

void GetGrowSizeTheBuildWindow(WindowRef theWindow, Rect *sizeRect)
{
	#pragma unused(theWindow)

	GetGrowSizeTheLists(sizeRect, DEAD_SPACE_H, DEAD_SPACE_V, HEADER_SPACE);
}

static	Boolean ButtonIsDimmedQQ(WindowRef theWindow, short buttonNum)
{
	#pragma unused(theWindow)

	switch (buttonNum)
	{
		case kNewRefButton:
		case kShowMessageButton:
		case kHideToolbarButton:
			return FALSE;
		case kNewInstantButton:
			return (GetHighlightedReference()<0);
		case kEditRefButton:
		case kDeleteRefButton:
			return (ReferenceListActiveQQ() ?
				(GetHighlightedReference()<0) : (GetHighlightedInstant()<0));
		case kMarkInterestingButton:
			return (GetHighlightedReference()<0);
	}
	
	return TRUE;
}

static	void SetTheBuildMessage(WindowRef theWindow, short messageIndex, Boolean redraw)
{
	if (messageIndex>0)
		GetIndString(gTheMessage, kBuildMessagesID, messageIndex);
	gTheMessageID=messageIndex;
	if (redraw)
		DrawTheBuildMessage(theWindow, FALSE);
}

static	void DrawBuildHeader(WindowRef theWindow, short theDepth)
{
	Rect			tempRect;
	RGBColor		background4={32767, 32767, 32767};
	RGBColor		background8={30802, 30802, 30802};
	PixPatHandle	backgroundppat;
	short			width;
	short			headerHeight;
	GrafPtr			curPort;
	
	headerHeight=kHeaderHeight;
	
	GetPort(&curPort);
	tempRect=curPort->portRect;
	tempRect.bottom=tempRect.top+headerHeight-1;
	
	if (theDepth>2)
	{
		backgroundppat=NewPixPat();
		MakeRGBPat(backgroundppat, (theDepth==4) ? &background4: &background8);
		FillCRect(&tempRect, backgroundppat);
		DisposePixPat(backgroundppat);
	}
	else
	{
		EraseRect(&tempRect);
	}
	
	width=curPort->portRect.right-curPort->portRect.left;
	MoveTo(0, headerHeight-1);
	Line(width, 0);
	
	UpdateBuildButtons(theWindow, FALSE);
	
	if (gShowMessageBox)
		DrawTheBuildMessage(theWindow, TRUE);
}

static	void DrawTheBuildMessage(WindowRef theWindow, Boolean drawFrame)
{
	Rect			tempRect;
	short			headerHeight;
	
	headerHeight=kHeaderHeight;
	tempRect=theWindow->portRect;
	tempRect.right-=DEAD_SPACE_H;
	tempRect.left+=DEAD_SPACE_H;
	tempRect.bottom=tempRect.top+headerHeight-DEAD_SPACE_H;
	tempRect.top=tempRect.bottom-MESSAGE_HEIGHT;
	if (drawFrame)
	{
		DrawTheShadowBox(tempRect, TRUE);
		InsetRect(&tempRect, 3, 3);
	}
	else
	{
		InsetRect(&tempRect, 3, 3);
		EraseRect(&tempRect);
	}
	
	if (gTheMessageID>0)
	{
		MoveTo(tempRect.left+5, tempRect.bottom-4);
		TextFont(geneva);
		TextSize(9);
		TextMode(srcOr);
		DrawString(gTheMessage);
	}
}

void UpdateBuildButtons(WindowRef theWindow, Boolean onlyIfChanged)
{
	short			i;
	short			theDepth;
	
	if (!gShowToolbar)
		return;
	
	theDepth=GetWindowDepth(theWindow);
	for (i=0; i<NUM_BUTTONS; i++)
	{
		if ((!onlyIfChanged) || (ButtonIsDimmedQQ(theWindow, i)!=gOldButtonState[i]))
			Draw3DButton(&gButtonRect[i], 0L, gButtonIcon[i], theDepth, kSquareButton+
				(ButtonIsDimmedQQ(theWindow, i) ? kButtonIsDimmed : 0));
	}
}

short GetBuildHeaderHeight(void)
{
	return kHeaderHeight;
}

void RememberBuildButtonState(WindowRef theWindow)
{
	short			i;
	
	for (i=0; i<NUM_BUTTONS; i++)
		gOldButtonState[i]=ButtonIsDimmedQQ(theWindow, i);
}

void SetBuildWindowTitle(Str255 theTitle)
{
	Mymemcpy((Ptr)gBuildWindowTitle, (Ptr)theTitle, theTitle[0]+1);
}

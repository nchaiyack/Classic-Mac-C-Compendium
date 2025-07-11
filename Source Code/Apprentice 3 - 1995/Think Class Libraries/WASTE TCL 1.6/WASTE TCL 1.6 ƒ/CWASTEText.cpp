/********************************************************\
 CWASTEText.cpp

	by Dan Crevier
	version 1.6
	5/24/95

\********************************************************/

#ifdef TCL_PCH
#include <TCLHeaders>
#endif

#include "CWASTEText.h"
#ifdef WASTE11
#include "CWASTETask.h"
#else
#include "CWASTEEditTask.h"
#include "CWASTEStyleTask.h"
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif
#ifndef __SCRAP__
#include <Scrap.h>
#endif
#ifndef __FONTS__
#include <Fonts.h>
#endif
#ifndef __DRAG__
#include <Drag.h>
#endif
#include <Sound.h>
#include <Icons.h>
#include <Script.h>
#include "CClipboard.h"
#include "CBartender.h"
#include "Commands.h"
#include "TCLUtilities.h"
#include "Constants.h"
#include "CScrollPane.h"
#include "Global.h"
#include <TextServices.h>
#include "CPrinter.h"
#include "CWindow.h"

// Global Variables
extern CClipboard	*gClipboard;	// Copies and Pastes data
extern EventRecord  gLastMouseUp;
extern CBureaucrat	*gGopher;
extern CBartender	*gBartender;

extern long			gSleepTime;
extern short		gUsingTSM;
extern Boolean		gHasDragAndDrop;



#ifdef WASTE_OBJECT_ARCHIVE
#include "WASTE_Objects.h"
#endif

#ifndef WASTE11
// undocumented WASTE routine
#define CWASTE
#ifdef CWASTE
extern "C"
{
	long _WEOffsetToLine(long offset, WEHandle hWE);
}
#else
	pascal long _WEOffsetToLine(long offset, WEHandle hWE);
#endif
#endif

#define WASTE_TABS

#ifdef WASTE_TABS
#include "WETabs.h"
#endif

// undocumented WASTE feature flag
const short weFActive = 20;

#ifdef WASTE11
// for drag and drop
static DragTrackingHandler trackingUPP = NULL;
static DragReceiveHandler receiveUPP = NULL;
static pascal OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow,
	void *handlerRefCon, DragReference theDrag);
static pascal OSErr MyReceiveHandler(WindowPtr theWindow, void *handlerRefCon, DragReference theDrag);

#define kTypePicture		'PICT'
#define kTypeSound			'snd '

#ifndef WASTE_OBJECT_ARCHIVE
// handlers for pictures and sounds
#define kSoundIconNumber	777
static pascal OSErr HandleNewPicture(Point *defaultObjectSize, WEObjectReference objectRef);
static pascal OSErr HandleDisposePicture(WEObjectReference objectRef);
static pascal OSErr HandleDrawPicture(Rect *destRect, WEObjectReference objectRef);
static pascal OSErr HandleNewSound(Point *defaultObjectSize, WEObjectReference objectRef);
static pascal OSErr HandleDisposeSound(WEObjectReference objectRef);
static pascal OSErr HandleClickSound(Point hitPt, short modifiers, long clickTime,
	WEObjectReference objectRef);
static pascal OSErr HandleDrawSound(Rect *destRect, WEObjectReference objectRef);
#endif
#endif

TCL_DEFINE_CLASS_D1(CWASTEText, CAbstractText);

RGBColor RGBBlack={0,0,0};
RGBColor RGBRed={65535,0,0};
RGBColor RGBGreen={0,65535,0};
RGBColor RGBBlue={0,0,65535};
RGBColor RGBCyan={0,65535,65535};
RGBColor RGBMagenta={65535,0,65535};
RGBColor RGBYellow={65535,65535,0};

CWASTEText *CWASTEText::curWASTEText = NULL;

/********************************************************\
 CWASTEText() - default constructor
 	You must call IWASTEText if you use this
\********************************************************/

CWASTEText::CWASTEText()
{
	macWE = NULL;
	spacingCmd = cmdSingleSpace;
	alignCmd = cmdAlignLeft;

	wholeLines = false;
	
	fixedLineHeights = false;
	
	cFirstTaskIndex = 1;  // so we get undo strings
	
	TCL_END_CONSTRUCTOR
}

/********************************************************\
 CWASTEText - constructor
 	You cannot call IWASTEText if you use this
\********************************************************/

CWASTEText::CWASTEText(
	CView			*anEnclosure,
	CBureaucrat		*aSupervisor,
	short			aWidth,
	short			aHeight,
	short			aHEncl,
	short			aVEncl,
	SizingOption	aHSizing,
	SizingOption	aVSizing,
	short			aLineWidth,
	Boolean			aScrollHoriz,
	TextStyle 		*tStyle)

	:	CAbstractText(anEnclosure, aSupervisor,
					 aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing,
					 aLineWidth, aScrollHoriz)
{
	macWE = NULL;
	spacingCmd = cmdSingleSpace;
	alignCmd = cmdAlignLeft;

	wholeLines = false;
	fixedLineHeights = false;

	SetPort(macPort);

	if (tStyle)
	{
		TextFont(tStyle->tsFont);
		TextSize(tStyle->tsSize);
		TextFace(tStyle->tsFace);
		if (gSystem.hasColorQD)
			RGBForeColor(&tStyle->tsColor);
	}

	
	IWASTETextX();

	cFirstTaskIndex = 1;  // so we get undo strings
	
	TCL_END_CONSTRUCTOR
}

/********************************************************\
 ~CWASTEText - destructor
\********************************************************/

CWASTEText::~CWASTEText()
{
	TCL_START_DESTRUCTOR
	if (macWE)
	{
		WEDispose(macWE);
		macWE = NULL;
#ifdef WASTE11
		RemoveDragHandlers();
#endif
	}
}

/********************************************************\
 IWASTEText - initialize CWASTEText
 	Use this only if you use the default constructor
\********************************************************/

void CWASTEText::IWASTEText(
	CView			*anEnclosure,
	CBureaucrat		*aSupervisor,
	short			aWidth,
	short			aHeight,
	short			aHEncl,
	short			aVEncl,
	SizingOption	aHSizing,
	SizingOption	aVSizing,
	short			aLineWidth,
	TextStyle 		*tStyle)
{

	CAbstractText::IAbstractText(anEnclosure, aSupervisor,
					 aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing,
					 aLineWidth);
	if (tStyle)
	{
		TextFont(tStyle->tsFont);
		TextSize(tStyle->tsSize);
		TextFace(tStyle->tsFace);
		if (gSystem.hasColorQD)
			RGBForeColor(&tStyle->tsColor);
	}

	IWASTETextX();
}

/********************************************************\
 IViewTemp - initialize from View resource
\********************************************************/

void CWASTEText::IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
						Ptr viewData)
{
	CAbstractText::IViewTemp(anEnclosure, aSupervisor, viewData);

	macWE = NULL;
	spacingCmd = cmdSingleSpace;
	alignCmd = cmdAlignLeft;
	fixedLineHeights = FALSE;

	IWASTETextX();
}

/********************************************************\
 IWASTETextX - initialization common to above routines
\********************************************************/

void CWASTEText::IWASTETextX()
{
	LongRect			tLongFrame;
	Boolean				saveAllocState;
	OSErr				err;
	WEHandle 			newMacWE;
#ifdef WASTE11
	static WEClickLoopUPP		clickLoop = NULL;
	static WETSMPreUpdateUPP	preProc = NULL;
	static WETSMPostUpdateUPP	postProc = NULL;
#ifndef WASTE_OBJECT_ARCHIVE
	static WENewObjectUPP		newPICTUPP = NULL;
	static WEDisposeObjectUPP	disposePICTUPP = NULL;
	static WEDrawObjectUPP		drawPICTUPP = NULL;
	static WENewObjectUPP		newSndUPP = NULL;
	static WEDisposeObjectUPP	disposeSndUPP = NULL;
	static WEDrawObjectUPP		drawSndUPP = NULL;
	static WEClickObjectUPP		clickSndUPP = NULL;
#endif
#else
	WEClickLoopProcPtr	clickLoop;
	WETSMPreUpdateProcPtr preProc;
	WETSMPostUpdateProcPtr postProc;
#endif

	dragHandlersInstalled = false;
	textMargin = 0;

	ForceNextPrepare();

	UseLongCoordinates(TRUE);

	// Figure out size of of rectangle
	GetAperture(&tLongFrame);

	// create WEHandle
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
    err = WENew(&tLongFrame, &tLongFrame, weDoOutlineHilite + weDoDrawOffscreen + weDoUndo,
     			&newMacWE);
#else
    err = WENew(&tLongFrame, &tLongFrame, weDoOutlineHilite + weDoDrawOffscreen,
     			&newMacWE);
#endif
    SetAllocation(saveAllocState);
    FailOSErr(err);
	macWE = newMacWE;

	SetWholeLines(wholeLines);

	AdjustBounds();

	autoRefresh = lineWidth <= 0;

	// �� Install handlers ��
#ifdef WASTE11
	// if first time, make UPP's
	if (clickLoop == NULL)
	{
		clickLoop = NewWEClickLoopProc(WEClickLoop);
		preProc = NewWETSMPreUpdateProc(WEPreUpdate);
		postProc = NewWETSMPostUpdateProc(WEPostUpdate);
	}
#else
	preProc = &WEPreUpdate;
	postProc = &WEPostUpdate;
	clickLoop = &WEClickLoop;
#endif

	WESetInfo(weTSMPreUpdate, (Ptr)&preProc, macWE);
	WESetInfo(weTSMPostUpdate, (Ptr)&postProc, macWE);
	WESetInfo(weClickLoop, (Ptr)&clickLoop, macWE);

	CalcWERects();
	if (lineWidth < 0)
	{
		saveAllocState = SetAllocation(kAllocCanFail);
		err = WECalText(macWE);
		SetAllocation(saveAllocState);
	}

#ifdef WASTE_AUTO_TABS
	InstallTabHandlers();
#endif

#ifdef WASTE11

#ifdef WASTE_OBJECT_ARCHIVE
	FailOSErr(InstallAllWASTEObjHandlers());
#else
	if (newPICTUPP == NULL)
	{
		newPICTUPP = NewWENewObjectProc(HandleNewPicture);
		disposePICTUPP = NewWEDisposeObjectProc(HandleDisposePicture);
		drawPICTUPP = NewWEDrawObjectProc(HandleDrawPicture);
	
		newSndUPP = NewWENewObjectProc(HandleNewSound);
		disposeSndUPP = NewWEDisposeObjectProc(HandleDisposeSound);
		drawSndUPP = NewWEDrawObjectProc(HandleDrawSound);
		clickSndUPP = NewWEClickObjectProc(HandleClickSound);
	}
	
	// ���Install Picture Handlers ��
	FailOSErr(WEInstallObjectHandler(kTypePicture, weNewHandler, (UniversalProcPtr)newPICTUPP, NULL));
	FailOSErr(WEInstallObjectHandler(kTypePicture, weDisposeHandler, (UniversalProcPtr)disposePICTUPP, NULL));
	FailOSErr(WEInstallObjectHandler(kTypePicture, weDrawHandler, (UniversalProcPtr)drawPICTUPP, NULL));

	// ���Install Sound Handlers ��
	FailOSErr(WEInstallObjectHandler(kTypeSound, weNewHandler, (UniversalProcPtr)newSndUPP, NULL));
	FailOSErr(WEInstallObjectHandler(kTypeSound, weDisposeHandler, (UniversalProcPtr)disposeSndUPP, NULL));
	FailOSErr(WEInstallObjectHandler(kTypeSound, weDrawHandler, (UniversalProcPtr)drawSndUPP, NULL));
	FailOSErr(WEInstallObjectHandler(kTypeSound, weClickHandler, (UniversalProcPtr)clickSndUPP, NULL));
#endif // WASTE_OBJECT_ARCHIVE
#endif // WASTE11
}

#ifdef WASTE_TABS
/********************************************************\
 InstallTabHandlers -- with WASTE 1.1r7 or later, will
 	install the hooks to support tabs in WASTE.  If
 	WASTE_AUTO_TABS, this will be called for you.
\********************************************************/

void CWASTEText::InstallTabHandlers(void)
{
	WEInstallTabHooks(macWE);
}
#endif

/********************************************************\
 SetTextMargin -- set a margin that will be left between
	the destRect and viewRect in WASTE
\********************************************************/

void CWASTEText::SetTextMargin(short margin)
{
	textMargin = margin;
	CalcWERects();
}

#ifdef WASTE11
/********************************************************\
 MyTrackingHandler - handles drag tracking
\********************************************************/

pascal OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow,
	void *handlerRefCon, DragReference theDrag)
{
	if (theWindow!=NULL)
	{
		((CWASTEText *)handlerRefCon)->Prepare();
		ClipRect(&theWindow->portRect);
		return WETrackDrag(theMessage, theDrag, ((CWASTEText *)handlerRefCon)->macWE);	
	}
	else
	{
		return noErr;
	}
}

/********************************************************\
 MyTrackingHandler - handles drag tracking
\********************************************************/

pascal OSErr MyReceiveHandler(WindowPtr theWindow, void *handlerRefCon, DragReference theDrag)
{
	OSErr err = noErr;
	CWASTETask *wasteTask;
	
	if (theWindow!=NULL)
	{
		((CWASTEText *)handlerRefCon)->Prepare();
		ClipRect(&theWindow->portRect);
		err = WEReceiveDrag(theDrag, ((CWASTEText *)handlerRefCon)->macWE);	
		((CWASTEText *)handlerRefCon)->AdjustBounds();
		wasteTask = new CWASTETask((CWASTEText *)handlerRefCon);
		((CWASTEText *)handlerRefCon)->itsSupervisor->Notify(wasteTask);
	}
	return err;
}

#ifndef WASTE_OBJECT_ARCHIVE

/********************************************************\
 HandleNewPicture - Handler for creating pictures
 	from WASTE Demo source
\********************************************************/

pascal OSErr HandleNewPicture(Point *defaultObjectSize, WEObjectReference objectRef)
{
	PicHandle thePicture;
	Rect frame;

	// { get handle to object data (in this case, a picture handle) }
	thePicture = (PicHandle)WEGetObjectDataHandle(objectRef);

	// { figure out the default object size by looking at the picFrame record }
	frame = (*thePicture)->picFrame;
	OffsetRect(&frame, -frame.left, -frame.top);
	defaultObjectSize->h = frame.right;
	defaultObjectSize->v = frame.bottom;

	return noErr;
}

/********************************************************\
 HandleDisposePicture - handles disposing a picture
 	From WASTE Demo source
\********************************************************/

pascal OSErr HandleDisposePicture(WEObjectReference objectRef)
{
	PicHandle thePicture;

	// { get handle to object data (in this case, a picture handle) }
	thePicture = (PicHandle)WEGetObjectDataHandle(objectRef);

	// { kill the picture }
	KillPicture(thePicture);

	return MemError();
}

/********************************************************\
 HandleDrawPicture - handle drawing a picture
 	From WASTE Demo source
\********************************************************/

pascal OSErr HandleDrawPicture(Rect *destRect, WEObjectReference objectRef)
{
	PicHandle thePicture;

	// { get handle to object data (in this case, a picture handle) }
	thePicture = (PicHandle)WEGetObjectDataHandle(objectRef);

	// { draw the picture }
	DrawPicture(thePicture, destRect);

	return noErr;
}

/********************************************************\
 HandleNewSound - Handler for creating sounds
\********************************************************/

pascal OSErr HandleNewSound(Point *defaultObjectSize, WEObjectReference objectRef)
{
	// icon is 32x32

	defaultObjectSize->h = 32;
	defaultObjectSize->v = 32;

	return noErr;
}

/********************************************************\
 HandleDisposeSound - handles disposing a sound
\********************************************************/

pascal OSErr HandleDisposeSound(WEObjectReference objectRef)
{
	Handle theSound;

	// { get handle to object data (in this case, a sound handle) }
	theSound = WEGetObjectDataHandle(objectRef);

	DisposeHandle(theSound);

	return MemError();
}

/********************************************************\
 HandleClickSound - handle clicking a sound (play it)
\********************************************************/

pascal OSErr HandleClickSound(Point hitPt, short modifiers, long clickTime,
	WEObjectReference objectRef)
{
	OSErr err;
	Handle theSound;

	// { get handle to object data (in this case, a sound handle) }
	theSound = WEGetObjectDataHandle(objectRef);

	// seems to depend on Universal Headers version -- there's probably a good way to check,
	// but...
#ifndef __MWERKS__
	err = SndPlay(NULL, theSound, false);
#else
	err = SndPlay(NULL, (SndListHandle)theSound, false);
#endif

	return err;
}

/********************************************************\
 HandleDrawSound -- draw an icon for the sound
\********************************************************/

pascal OSErr HandleDrawSound(Rect *destRect, WEObjectReference objectRef)
{
	PlotIconID(destRect, atNone, ttNone, kSoundIconNumber);
	return noErr;
}

#endif // WASTE_OBJECT_ARCHIVE

/********************************************************\
 InstallDragHandlers - installs the handlers for
 	receiving drags
\********************************************************/

void CWASTEText::InstallDragHandlers(void)
{
	WindowPtr dragWindow = GetWindow()->macPort;

	if (gHasDragAndDrop && !dragHandlersInstalled)
	{
		if (trackingUPP == NULL) // need to create UPP
		{
			trackingUPP = NewDragTrackingHandlerProc(MyTrackingHandler);
		}
		FailOSErr(InstallTrackingHandler(trackingUPP, dragWindow, this));

		if (receiveUPP == NULL) // need to create UPP
		{
			receiveUPP = NewDragReceiveHandlerProc(MyReceiveHandler);
		}
		FailOSErr(InstallReceiveHandler(receiveUPP, dragWindow, this));

		dragHandlersInstalled = true;
	}
}

/********************************************************\
 RemoveDragHandlers -- removes the drag handlers
\********************************************************/

void CWASTEText::RemoveDragHandlers(void)
{
	WindowPtr dragWindow = GetWindow()->macPort;

	if (gHasDragAndDrop && dragHandlersInstalled)
	{
		FailOSErr(RemoveTrackingHandler(trackingUPP, dragWindow));
		FailOSErr(RemoveReceiveHandler(receiveUPP, dragWindow));
		dragHandlersInstalled = false;
	}
}
#endif

/********************************************************\
 CheckInsertion - sees if the specified amount of text
 	can be inserted
\********************************************************/

void CWASTEText::CheckInsertion(long numChars, long styleSize, Boolean useSelection)
{
	long	selStart, selEnd;
	long	growSize = numChars;
	Handle	h;
	OSErr	err;

	if (useSelection)
	{
		GetSelection(&selStart, &selEnd);
		growSize -= Abs(selEnd - selStart);
	}

	/*	The problem with WASTE is that it can succeed in adding the text to its content
	 *	structure, and then fail to expand its style tables or line start tables.  The way
	 *	I address this is to add a fudge factor to the amount of data being added, and
	 *	hope that its enough to cover the WASTE overhead.
	 */

	if (growSize > 0)
	{
		/*	Add fudge factor for new line starts (1/5th of numChars)	*/
		
		growSize += growSize / 5;
		
		/*	Add fudge factor for style information (1/2 of styleSize)	*/

		growSize += styleSize + styleSize / 2;

		if (WEFeatureFlag(weFUseTempMem, weBitTest, macWE))
		{
			h = TempNewHandle(growSize, &err);
			if (!h && err == noErr)
				err = memFullErr;
			DisposeHandle(h);
			FailOSErr(err);
		}
		else
		{
			h = NewHandleCanFail(growSize);
			FailNIL(h);
			DisposeHandle(h);
		}
	}
}

#ifdef WASTE11
static long sOldModCount = -1; // for keeping track of modifications by inline input
#endif

/********************************************************\
 WEPreUpdate -- prepare the port
\********************************************************/

pascal void CWASTEText::WEPreUpdate(WEHandle hWE)
{
	if (curWASTEText != NULL)
	{
#ifdef WASTE11
		sOldModCount = WEGetModCount(hWE);
#endif
		curWASTEText->Prepare();
	}
}

/********************************************************\
 WEPostUpdate -- keep typing visible -- by M. Sladok
\********************************************************/

pascal void CWASTEText::WEPostUpdate(WEHandle hWE, long fixLength, long inputAreaStart,
	long inputAreaEnd, long pinRangeStart, long pinRangeEnd)
{
#ifdef WASTE11
	long newModCount;
	CWASTETask *wasteTask;
#endif

	if (curWASTEText != NULL)
	{
		curWASTEText->AdjustBounds();
		curWASTEText->ScrollToSelection();
#ifdef WASTE11
		newModCount = WEGetModCount(hWE);
		if (newModCount != sOldModCount) // inline input occured
		{
			wasteTask = new CWASTETask(curWASTEText);
			curWASTEText->itsSupervisor->Notify(wasteTask);
		}
#endif
	}
}

/********************************************************\
 DoClick - handle mouse clicks
\********************************************************/

void CWASTEText::DoClick(Point hitPt, short modifierKeys, long when)
{
#ifdef WASTE11
	long oldModCount, newModCount;
	CWASTETask *wasteTask;

	oldModCount = WEGetModCount(macWE);
#endif

	WEClick(hitPt, modifierKeys, when, macWE);
#ifdef WASTE11
	newModCount = WEGetModCount(macWE);
#endif
	
		// CSwitchboard will never see the mouse up that ended
		// the drag, so we stuff gLastMouseUp here to allow
		// multi-click counting to work.
		
	gLastMouseUp.what = mouseUp;
	gLastMouseUp.when = TickCount();
	gLastMouseUp.where = hitPt;
	LocalToGlobal( &gLastMouseUp.where);
	gLastMouseUp.modifiers = modifierKeys;

	SelectionChanged();
	
	// make task if a drag occured (so it can be undone)
#ifdef WASTE11
	if (oldModCount != newModCount) // drag occured
	{
		wasteTask = new CWASTETask(this);
		itsSupervisor->Notify(wasteTask);
	}
#endif
#ifndef WASTE11
	if (!editable && (gGopher == this))
	{
		long selStart, selEnd;
		
		GetSelection( &selStart, &selEnd);	
		if (selStart == selEnd)
			itsSupervisor->BecomeGopher( TRUE);
	}
#endif
}

#ifdef WASTE11

/******************************************************************************
 DoKeyDown - handles key strokes and the undoing of them
 ******************************************************************************/

void CWASTEText::DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent)
{
	CWASTETask *wasteTask;

	// Don't try to insert and command keys into the text,
    // pass them along the command chain

    if (macEvent->modifiers & cmdKey)
    {
    	inherited::DoKeyDown(theChar, keyCode, macEvent);
    	return;
    }

    switch (keyCode)
    {

        case KeyHome:
        case KeyPageUp:
        case KeyPageDown:
        case KeyEnd:
        case KeyLeftCursor:
        case KeyRightCursor:
        case KeyUpCursor:
        case KeyDownCursor:
    		inherited::DoKeyDown(theChar, keyCode, macEvent);
        	break;

        default:
        	if (editable)
        	{
        		TypeChar(theChar, macEvent->modifiers);
        		wasteTask = new CWASTETask(this);
        		itsSupervisor->Notify(wasteTask);
	        }
            break;
    }
}

/********************************************************\
 DoCommand - version that uses WASTE 1.1's undo features
\********************************************************/

void CWASTEText::DoCommand(long theCommand)
{
	CWASTETask *wasteTask;
	Boolean makeTask = false;
	Str255			itemName;
	long			number;

	if (theCommand < 0)
	{
		switch (HiShort(-theCommand))
		{
			// ���font menu ��
			case MENUfont:
				gBartender->GetCmdText(theCommand, itemName);
				SetFontName(itemName);
				makeTask = true;
				break;
			
			// ���Size menu ��
			case MENUsize:
				gBartender->GetCmdText(theCommand, itemName);
				StringToNum(itemName, &number);
				SetFontSize(number);
				makeTask = true;
				break;

			default:
				CPanorama::DoCommand(theCommand);
				break;
		}

	}
	else
	{
		switch (theCommand)
		{
			case cmdCut:
			case cmdPaste:
			case cmdClear:
				if (!editable)
				{
					SysBeep(3);
					break;
				}
				PerformEditCommand(theCommand);
				makeTask = true;
				break;

			case cmdCopy:
				PerformEditCommand(theCommand);
				break;

			case cmdSelectAll:
				Prepare();
				SelectAll(TRUE);
				SelectionChanged();
				break;

			case cmdPlain:
				SetFontStyle(NOTHING);
				makeTask = true;
				break;

			case cmdBold:					// Styles are set by flipping bits
			case cmdItalic:					//   Bold is bit 0, Italic is bit 1
			case cmdUnderline:				//   Underline is bit 2, etc.
			case cmdOutline:				//   By cleverly numbering these
			case cmdShadow:					//   commands, we can just shift
			case cmdCondense:				//   a bit to get the proper style
			case cmdExtend:
				SetFontStyle(1 << (theCommand - cmdBold));
				makeTask = true;
				break;

			case cmdBlack:
					SetFontColor(&RGBBlack);
					makeTask = true;
					break;
			case cmdRed:
					SetFontColor(&RGBRed);
					makeTask = true;
					break;
			case cmdGreen:
					SetFontColor(&RGBGreen);
					makeTask = true;
					break;
			case cmdBlue:
					SetFontColor(&RGBBlue);
					makeTask = true;
					break;
			case cmdCyan:
					SetFontColor(&RGBCyan);
					makeTask = true;
					break;
			case cmdMagenta:
					SetFontColor(&RGBMagenta);
					makeTask = true;
					break;
			case cmdYellow:
					SetFontColor(&RGBYellow);
					makeTask = true;
					break;

			case cmdAlignLeft:
			case cmdAlignCenter:
			case cmdAlignRight:
			case cmdJustify:
				SetAlignCmd(theCommand);
				// not undoable at this time
				break;

			case cmdSingleSpace:
			case cmd1HalfSpace:
			case cmdDoubleSpace:
				SetSpacingCmd(theCommand);
				// not undoable at this time
				break;

			default:
				CPanorama::DoCommand(theCommand);
				break;
		}
	}
	
	if (makeTask)
	{
        wasteTask = new CWASTETask(this);
        itsSupervisor->Notify(wasteTask);
	}
}

#else
/********************************************************\
 DoCommand - AbstractText does not deal with justify so
 	we have to special case it. And color too
\********************************************************/

void	CWASTEText::DoCommand(long theCommand)
{
	CTextStyleTask	*styleTask = NULL;
	Boolean		makeStyleTask = FALSE;

	if (stylable)
	{
		switch (theCommand)
		{
			case cmdJustify:
			case cmdBlack:
			case cmdRed:
			case cmdBlue:
			case cmdGreen:
			case cmdCyan:
			case cmdMagenta:
			case cmdYellow:
				makeStyleTask =TRUE;
				break;
			default: ;
		}
	}
		
	if (makeStyleTask)
	{
		itsTypingTask = NULL;
		styleTask = MakeStyleTask(theCommand);
		itsLastTask = styleTask;
		itsSupervisor->Notify(styleTask);
		styleTask->Do();
	}
	else
		inherited::DoCommand(theCommand);
}
#endif

/********************************************************\
 UpdateMenus - handle WASTE specific menu enabling
\********************************************************/

void CWASTEText::UpdateMenus()
{
	long	selStart, selEnd;
	TextStyle		style;
	short			styleFlags;

	inherited::UpdateMenus();
	
		// Copy and Cut are only possible if the selection is small (< 32K).
		// The danger here is that a piece of styled text > 32K will pass
		// through the clipboard to another applciation which cannot handle
		// it (like the Finder).
		//
		// Should probably override DoCommand as well to make sure a command
		// does not get past the Menu Manager -- can be done later.
	
	GetSelection(&selStart, &selEnd);
	if (Abs(selEnd - selStart) >= 32767)
	{
		gBartender->DisableCmd(cmdCopy);
		gBartender->DisableCmd(cmdCut);
	}
#ifdef WASTE11
	if (WECanPaste(macWE))
		gBartender->EnableCmd(cmdPaste);
	else
		gBartender->DisableCmd(cmdPaste);
#endif

	gBartender->EnableCmd(cmdBlack);  //mfcolor
	gBartender->EnableCmd(cmdRed);  //mfcolor
	gBartender->EnableCmd(cmdGreen);  //mfcolor
	gBartender->EnableCmd(cmdBlue);  //mfcolor
	gBartender->EnableCmd(cmdCyan);  //mfcolor
	gBartender->EnableCmd(cmdMagenta);  //mfcolor
	gBartender->EnableCmd(cmdYellow);  //mfcolor

	// figure out the continuous color
	styleFlags = doColor;
	GetTextStyle (&styleFlags,  &style);

	if (styleFlags & doColor)
	{
		if (style.tsColor.red == 0)
		{
			if (style.tsColor.green == 0)
			{
				if (style.tsColor.blue == 0)
				{
					gBartender->CheckMarkCmd(cmdBlack, true);
				}
				else if (style.tsColor.blue == 65535)
				{
					gBartender->CheckMarkCmd(cmdBlue, true);
				}
			}
			else if (style.tsColor.green == 65535)
			{
				if (style.tsColor.blue == 0)
				{
					gBartender->CheckMarkCmd(cmdGreen, true);
				}
				else if (style.tsColor.blue == 65535)
				{
					gBartender->CheckMarkCmd(cmdCyan, true);
				}
			}
		}
		else if (style.tsColor.red == 65535)
		{
			if (style.tsColor.green == 0)
			{
				if (style.tsColor.blue == 0)
				{
					gBartender->CheckMarkCmd(cmdRed, true);
				}
				else if (style.tsColor.blue == 65535)
				{
					gBartender->CheckMarkCmd(cmdMagenta, true);
				}
			}
			else if (style.tsColor.green == 65535)
			{
				if (style.tsColor.blue == 0)
				{
					gBartender->CheckMarkCmd(cmdYellow, true);
				}
				else if (style.tsColor.blue == 65535)
				{
					// no white
				}
			}
		}
	}
	
}

#ifndef WASTE11
/********************************************************\
 MakeEditTask -- use WASTE specific one
\********************************************************/

CTextEditTask *CWASTEText::MakeEditTask( long editCmd)
{
	CWASTEEditTask *volatile editTask = NULL;
	
	try_
	{
		editTask = new CWASTEEditTask(this, editCmd, cFirstTaskIndex);
	}

	catch_all_()
	{
		TCLForgetObject(editTask);

		throw_same_();
	}
	end_try_
	
	return editTask;
}

/********************************************************\
 MakeStyleTask -- use WASTE specific one
\********************************************************/

CTextStyleTask *CWASTEText::MakeStyleTask( long styleCmd)
{

	CWASTEStyleTask *volatile newTask = NULL;
	short			taskIndex;
	
	try_
	{
		taskIndex = cFirstTaskIndex > 0 ? cFirstTaskIndex + undoFormatting : 0;
		newTask = new CWASTEStyleTask(this, styleCmd, taskIndex);
	}

	catch_all_()
	{
		TCLForgetObject( newTask);

		throw_same_();
	}
	end_try_
	
	return newTask;
}
#endif

/********************************************************\
 PerformEditCommand - handle cut, copy, paste, and clear
\********************************************************/

void CWASTEText::PerformEditCommand(long theCommand)
{
	Boolean		saveAllocState;
	OSErr		err = noErr;

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);

	switch( theCommand)
	{		
		case cmdCut:
			gClipboard->EmptyGlobalScrap();
			saveAllocState = SetAllocation(kAllocCanFail);		
			err = WECut(macWE);
			SetAllocation(saveAllocState);
			if (err == noErr)
				gClipboard->UpdateDisplay();
			break;
			
		case cmdCopy:
			gClipboard->EmptyGlobalScrap();
			saveAllocState = SetAllocation(kAllocCanFail);		
			err = WECopy(macWE);
			SetAllocation(saveAllocState);
			if (err == noErr)
				gClipboard->UpdateDisplay();
			break;
			
		case cmdPaste:
			CheckInsertion(gClipboard->DataSize('TEXT'),
						   gClipboard->DataSize('styl'), TRUE);
			saveAllocState = SetAllocation(kAllocCanFail);
			err = WEPaste(macWE);
			SetAllocation(saveAllocState);
			break;
			
		case cmdClear:
			saveAllocState = SetAllocation(kAllocCanFail);		
			err = WEDelete(macWE);
			SetAllocation(saveAllocState);
			break;
			
	}
	AdjustBounds();
	ScrollToSelection();
	FailOSErr(err);
}


/********************************************************\
 Draw - draw the text
\********************************************************/

void CWASTEText::Draw(Rect *area)
{
	RgnHandle	updateRgn;
	LongRect	fr, dr;
	Rect		r;
	
	if (!ReallyVisible())
		return;

	updateRgn = NewRgn();
	RectRgn(updateRgn, area);
	
	if (!printing)
	{
		SectRgn(updateRgn, macPort->visRgn, updateRgn);
		CalcWERects();
	}
	else
	{
		// Prepare the destination and view rectangles

		WEGetDestRect(&dr, macWE);
		GetAperture(&fr);
		OffsetLongRect(&dr, -(dr.left + fr.left), -(dr.top + fr.top));
		FrameToQDR(&fr, &r);
		OffsetLongRect(&dr, r.left, r.top);
		dr.right = (lineWidth > 0) ? dr.left + lineWidth : printPageWidth;
		WESetDestRect(&dr, macWE);
		QDToLongRect(area, &fr);
		if (wholeLines)
			fr.bottom = fr.top + vScale * ((area->bottom - area->top) / vScale);
		WESetViewRect(&fr, macWE);
	}

	WEUpdate(updateRgn, macWE);
	// not necessary -- err = WESetInfo(wePort, &macPort, macWE);
	DisposeRgn(updateRgn);

}


/********************************************************\
 Activate - activate the pane
\********************************************************/

void CWASTEText::Activate()
{
#ifndef WASTE11
	TSMDocumentID aTSMDocument;
#endif

	if (curWASTEText==this) // already active
	{
		return;
	}
	
	CAbstractText::Activate();
	
	// deactivate old WASTEText
	if (curWASTEText!=NULL) curWASTEText->Deactivate();
	Prepare();
	CalcWERects();
	WEActivate(macWE);
	curWASTEText = this;

#ifndef WASTE11
	if (!editable)
	{
		// if not editable, deactivate TSM so inline input won't work
		// WASTE 1.1a5 adds a readonly flag so we don't have to do this
		if (gUsingTSM)
		{
			if (WEGetInfo(weTSMDocumentID, (Ptr)&aTSMDocument, macWE) == noErr)
			{
				if (aTSMDocument != NULL)
					DeactivateTSMDocument(aTSMDocument);
			}
		}
	}
#endif
}

/********************************************************\
 Deactivate - deactivate the pane
\********************************************************/

void CWASTEText::Deactivate()
{
	CAbstractText::Deactivate();

	Prepare();
	WEStopInlineSession(macWE);
	if (macWE)
	{
		CalcWERects();
		WEDeactivate(macWE);
	}
	curWASTEText = NULL;
}

/********************************************************\
 SetSelection - set what text is selected
\********************************************************/

void CWASTEText::SetSelection(long selStart, long selEnd, Boolean fRedraw)
{
	short wasActive, wasOutline;

	Prepare();	
	if (!fRedraw)
	{
//		InhibitRecal(true);
		wasActive = WEFeatureFlag(weFActive, weBitClear, macWE);
		wasOutline = WEFeatureFlag(weFOutlineHilite, weBitClear, macWE);
	}
	WESetSelection(selStart, selEnd, macWE);
	if (!fRedraw)
	{
//		InhibitRecal(false);
		WEFeatureFlag(weFActive, wasActive, macWE);
		WEFeatureFlag(weFOutlineHilite, wasOutline, macWE);
	}
}

/********************************************************\
 Clear - clear the text
	Deactivates text before clearing to prevent
	highlighting from showing up
\********************************************************/

void CWASTEText::Clear(void)
{
	Boolean	saveAllocState;
	OSErr	err;
#ifdef WASTE11
	Boolean wasReadOnly;
#endif

	SetSelection(0, 0x7FFFFFFF, false);
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
#endif
	err = WEDelete(macWE);
#ifdef WASTE11
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
#endif
	SetAllocation(saveAllocState);
	
	//RestoreSelection();
	AdjustBounds();
	ScrollToSelection();
	itsSupervisor->Notify(NULL); // can't undo
	FailOSErr(err);
}

/********************************************************\
 SetTextPtr - set the text to a block of memory
\********************************************************/

void CWASTEText::SetTextPtr(Ptr textPtr, long numChars)
{
	Boolean		saveAllocState;
	OSErr		err;
#ifdef WASTE11
	Boolean		wasReadOnly;
#endif

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	
	SetSelection(0, 0x7FFFFFFF, false);
#ifdef WASTE11
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
#endif
	FailOSErr(WEDelete(macWE));
	CheckInsertion(numChars, 0, FALSE);
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	err = WEInsert(textPtr, numChars, NULL, NULL, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
#else
	err = WEInsert(textPtr, numChars, NULL, macWE);
#endif
	SetAllocation(saveAllocState);

	AdjustBounds();
	Refresh();

	itsSupervisor->Notify(NULL); // can't undo
	
	FailOSErr(err);
}

/********************************************************\
 StopInlineSession - stop inline session, confirm text
\********************************************************/

void CWASTEText::StopInlineSession(void)
{
	WEStopInlineSession(macWE);
}

/********************************************************\
 SetOutlineHighliting - turns on or off outline
 	highliting and returns old setting
\********************************************************/

Boolean CWASTEText::SetOutlineHighliting(Boolean hilite)
{
	Boolean oldValue;
	
	oldValue = (WEFeatureFlag(weFOutlineHilite, 
		hilite ? weBitSet : weBitClear, macWE) == weBitSet);
		
	return oldValue;
}

/********************************************************\
 GetTextHandle - get a handle to the text
 	This is not a copy of the handle, but the real thing
\********************************************************/

Handle CWASTEText::GetTextHandle()
{
	return( (Handle) WEGetText(macWE));
}

/********************************************************\
 CopyTextRange - return a handle to a copy of the
 	indicated range of text
\********************************************************/

Handle CWASTEText::CopyTextRange(long start, long end)
{
	Handle	h;
	long	len;
	
	end = Min(end, WEGetTextLength(macWE));
	
	len = Max(end - start, 0);
	h = NewHandleCanFail(len);
	FailNIL(h);
	if (len > 0)
		BlockMove( (char*)*(WEGetText(macWE)) + start, *h, len);
	
	return h;
}

/********************************************************\
 CopyRangeWithStyle - return a handle to the text and
 	styles in a range.  Handles must be previously
 	created with NewHandle();
\********************************************************/

void CWASTEText::CopyRangeWithStyle(long start, long end, Handle hText,
	StScrpHandle hStyles)
{
	Boolean		saveAllocState;
	OSErr		err;
	
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	err = WECopyRange(start, end, hText, hStyles, NULL, macWE);
#else
	err = WECopyRange(start, end, hText, hStyles, macWE);
#endif
	SetAllocation(saveAllocState);
	FailOSErr(err);
}

#ifdef WASTE11
/********************************************************\
 CopyRangeWithStyleSoup - return a handle to the text and
 	styles and soup in a range.  Handles must be previously
 	created with NewHandle();
\********************************************************/

void CWASTEText::CopyRangeWithStyleSoup(long start, long end, Handle hText,
	StScrpHandle hStyles, WESoupHandle hSoup)
{
	Boolean		saveAllocState;
	OSErr		err;
	
	saveAllocState = SetAllocation(kAllocCanFail);
	err = WECopyRange(start, end, hText, hStyles, hSoup, macWE);
	SetAllocation(saveAllocState);
	FailOSErr(err);
}
#endif

/********************************************************\
 InsertTextPtr - insert a block of text
\********************************************************/

void CWASTEText::InsertTextPtr(Ptr text, long length, Boolean fRedraw)
{
	Boolean		saveAllocState;
	OSErr		err;
	Boolean		wasReadOnly;

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);

	CheckInsertion(length, 0, TRUE);
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);

	err = WEInsert(text, length, NULL, NULL, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
#else
	err = WEInsert(text, length, NULL, macWE);
#endif
	SetAllocation(saveAllocState);

	if (fRedraw)
	{
		AdjustBounds(); // if not redrawing, will have to call AdjustBounds later
		Refresh();
	}
	itsSupervisor->Notify(NULL); // can't undo

	FailOSErr(err);
}

/********************************************************\
 InsertWithStyle -- insert text along with style
\********************************************************/

void CWASTEText::InsertWithStyle(Ptr text, long length, StScrpHandle hStyles,
		Boolean fRedraw)
{
	Boolean		saveAllocState;
	OSErr		err;
#ifdef WASTE11
	Boolean		wasReadOnly;
#else
	long		i, numStyles;
	short		fontSize;
	StScrpPtr	styles;
#endif

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	CheckInsertion(length, hStyles ? GetHandleSize((Handle) hStyles) : 0, TRUE);

#ifndef WASTE11 // WASTE 1.1 does this itself
	/*	Fix style entries which have a font size of 0	*/
	if (hStyles)
	{
		//fontSize = LMGetSysFontSize(); // for some reason this is returning 0 also
		fontSize = 12;
		styles = *hStyles;
		
		numStyles = styles->scrpNStyles;
		for (i = 0; i < numStyles; i++)
			if (styles->scrpStyleTab[i].scrpSize == 0)
				styles->scrpStyleTab[i].scrpSize = fontSize;
	}
#endif

	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
	err = WEInsert(text, length, hStyles, NULL, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
#else
	err = WEInsert(text, length, hStyles, macWE);
#endif
	SetAllocation(saveAllocState);

	AdjustBounds();
	if (fRedraw) Refresh();
	itsSupervisor->Notify(NULL); // can't undo
	FailOSErr(err);
}

#ifdef WASTE11
/********************************************************\
 InsertWithStyleSoup -- insert text along with style
	and soup
\********************************************************/

void CWASTEText::InsertWithStyleSoup(Ptr text, long length, StScrpHandle hStyles,
		WESoupHandle hSoup, Boolean fRedraw)
{
	Boolean		saveAllocState;
	OSErr		err;
	Boolean		wasReadOnly;

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	CheckInsertion(length, hStyles ? GetHandleSize((Handle) hStyles) : 0, TRUE);


	saveAllocState = SetAllocation(kAllocCanFail);
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
	err = WEInsert(text, length, hStyles, hSoup, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
	SetAllocation(saveAllocState);

	AdjustBounds();
	if (fRedraw) Refresh();
	itsSupervisor->Notify(NULL); // can't undo
	FailOSErr(err);
}
#endif


/********************************************************\
 TypeChar - type a character
\********************************************************/

void CWASTEText::TypeChar(char theChar, short theModifers)
{
	Boolean		saveAllocState;

	Prepare();

	CheckInsertion(1, 0, TRUE);
	saveAllocState = SetAllocation(kAllocCanFail);

	WEKey(theChar, theModifers, macWE);
	SetAllocation(saveAllocState);

	AdjustBounds();
	ScrollToSelection();
}

/********************************************************\
 CalcWERects - Sets the DestRect and ViewRect fields
 	used by WASTE
\********************************************************/

void CWASTEText::CalcWERects()
{
	LongRect	fr, dr;
	Rect		r;

		// Prepare the destination and view rectangles

	WEGetDestRect(&dr, macWE);
	GetAperture(&fr);
	OffsetLongRect(&dr, -(dr.left + fr.left), -(dr.top + fr.top));
	FrameToQDR(&fr, &r);
	OffsetLongRect(&dr, r.left, r.top);
	dr.right = (lineWidth > 0) ? dr.left + lineWidth : fr.right;
	dr.top += textMargin;
	dr.bottom -= textMargin;
	dr.left += textMargin;
	dr.right -= textMargin;
	WESetDestRect(&dr, macWE);
	QDToLongRect(&r, &fr);
	WESetViewRect(&fr, macWE);
}

/********************************************************\
 ResizeFrame - resize the frame when the size of the
 	pane changes
\********************************************************/

void CWASTEText::ResizeFrame(Rect *delta)
{
	Boolean		saveAllocState;
	OSErr		err;

	CAbstractText::ResizeFrame(delta);

	CalcWERects();	
	if (lineWidth < 0)
	{
		saveAllocState = SetAllocation(kAllocCanFail);
		err = WECalText( macWE);
		SetAllocation(saveAllocState);
		CalcWERects();
	}
	AdjustBounds();
}

/********************************************************\
 AdjustBounds - change the size of the CWASTEText to
 	match that in the WASTE record
\********************************************************/

void CWASTEText::AdjustBounds()
{
	LongRect	oldBounds;
	long  		newHeight;
	long		hFix = 0, vFix = 0;

	oldBounds = bounds;
	newHeight = GetHeight(0, MAXINT);
	
	bounds.left = bounds.top = 0;
	bounds.bottom = newHeight;

	if (lineWidth > 0) 
	{
		bounds.right = lineWidth;
	} else {
		bounds.right = frame.right - frame.left;
	}
        
    bounds.right = (bounds.right - 1) / hScale + 1;

	if (itsScrollPane != NULL)
	{
		itsScrollPane->AdjustScrollMax();
		itsScrollPane->Calibrate();
	}
}

/********************************************************\
 FindLine - return the line on which the character
 	at charPos is on.
\********************************************************/

long CWASTEText::FindLine(long charPos)
{
	long	lineNo;
	
#ifdef WASTE11
	lineNo = WEOffsetToLine(charPos, macWE);
#else
	lineNo = _WEOffsetToLine(charPos, macWE);
#endif
	if (charPos == WEGetTextLength(macWE) && 
		((char*)(*GetTextHandle()))[GetLength()-1] == 13)
		lineNo++;

	return (lineNo);
	
}

/********************************************************\
 GetLength - return the length of the text
\********************************************************/

long CWASTEText::GetLength()
{
	return WEGetTextLength(macWE);
}

/********************************************************\
 SetFontNumber - set the font of the selection
\********************************************************/

void CWASTEText::SetFontNumber(short aFontNumber)
{
	TextStyle	style;
	
	style.tsFont = aFontNumber;
	SetStyle( doFont, &style, TRUE);
	itsSupervisor->Notify(NULL); // can't undo

}

/********************************************************\
 SetFontNumberAll - set the font of all of the text
\********************************************************/

void CWASTEText::SetFontNumberAll(short aFontNumber)
{
	TempSelectAll();
	SetFontNumber(aFontNumber);
	RestoreSelection();
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetFontNameAll - set the font of the selection
\********************************************************/

void CWASTEText::SetFontNameAll(Str255 aFontName)
{
	TempSelectAll();
	SetFontName(aFontName);
	RestoreSelection();
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetFontStyle - set the font style of the selection
\********************************************************/

void CWASTEText::SetFontStyle(short aStyle)
{
	TextStyle	style;
	short		mode = doFace;
	
	style.tsFace = aStyle;
	if (aStyle != NOTHING)
		mode += doToggle;
	SetStyle( mode, &style, TRUE);
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetFontStyleAll - set the font style of all of the text
\********************************************************/

void CWASTEText::SetFontStyleAll(short aStyle)
{
	TempSelectAll();
	SetFontStyle(aStyle);
	RestoreSelection();
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetFontSize - set the font size of the selection
\********************************************************/

void CWASTEText::SetFontSize(short aSize)
{
	TextStyle	style;
	
	style.tsSize = aSize;
	SetStyle( doSize, &style, TRUE);
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetFontSizeAll - set the font size of all of the text
\********************************************************/

void CWASTEText::SetFontSizeAll(short aSize)
{
	TempSelectAll();
	SetFontSize(aSize);
	RestoreSelection();
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetFontColor - set the color of the selection
\********************************************************/

void CWASTEText::SetFontColor(RGBColor *aFontColor)
{
	TextStyle	style;
	
	style.tsColor = (*aFontColor);
	SetStyle(doColor, &style, TRUE);
	itsSupervisor->Notify(NULL); // can't undo

}

/********************************************************\
 SetFontColorAll - set the font of all of the text
\********************************************************/

void CWASTEText::SetFontColorAll(RGBColor *aFontColor)
{
	TempSelectAll();
	SetFontColor(aFontColor);
	RestoreSelection();
	itsSupervisor->Notify(NULL); // can't undo
}

/********************************************************\
 SetTextMode - currently not implemented
\********************************************************/

void CWASTEText::SetTextMode(short aMode)
{
	return;
}

/********************************************************\
 SetAlignment - set the alignment
\********************************************************/

void CWASTEText::SetAlignment(short anAlignment)
{
	WESetAlignment(anAlignment, macWE);
	
	Refresh();
}

/********************************************************\
 SetAlignCmd - set the alignment
\********************************************************/

void CWASTEText::SetAlignCmd(long anAlignCmd)
{
	short teAlign;
	
	alignCmd = anAlignCmd;
	switch( alignCmd)
	{
		case cmdAlignLeft:
			teAlign = weFlushLeft;
			break;
		case cmdAlignCenter:
			teAlign = weCenter;
			break;
		case cmdAlignRight:
			teAlign = weFlushRight;
			break;
		case cmdJustify:		//mf
			teAlign = weJustify;
			break;
		default:
			teAlign = weFlushDefault;
	}
	SetAlignment(teAlign);
}

/********************************************************\
 SetSpacingCmd - not really supported
\********************************************************/

void CWASTEText::SetSpacingCmd(long aSpacingCmd)
{
	spacingCmd = cmdSingleSpace;
	
	// only single-spaced text is supported.
	
	SetWholeLines(wholeLines);
 	CalcAperture();
	AdjustBounds();

}

/********************************************************\
 SetTheStyleScrap - set the style of a given range
 	caller disposes handle
\********************************************************/

void CWASTEText::SetTheStyleScrap(long rangeStart, long rangeEnd,
			StScrpHandle styleScrap, Boolean redraw)
{
	Boolean		saveAllocState;
	OSErr		err;
	long		selStart, selEnd;
	
	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	saveAllocState = SetAllocation(kAllocCanFail);
	WEGetSelection(&selStart, &selEnd, macWE);
	SetSelection(rangeStart, rangeEnd, FALSE);
	err = WEUseStyleScrap(styleScrap, macWE);
	if (err == noErr)
	{
		SetSelection(selStart, selEnd, false);
		err = WECalText( macWE);
	}

	SetAllocation(saveAllocState);
	AdjustBounds();
	itsSupervisor->Notify(NULL); // can't undo

	FailOSErr(err);
}

/********************************************************\
 SetStyle - set the style of the current selection
\********************************************************/

void CWASTEText::SetStyle(short mode, TextStyle *newStyle, Boolean redraw)
{
	Boolean		saveAllocState;
	OSErr		err;
#ifdef WASTE11
	Boolean		wasReadOnly;
#endif

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
#endif
	err = WESetStyle( mode, newStyle, macWE);
#ifdef WASTE11
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
#endif
	SetAllocation(saveAllocState);

	if (err == noErr)
	{
		SetSpacingCmd(spacingCmd);
		SetWholeLines(wholeLines);
	}
	if (redraw) AdjustBounds();

	itsSupervisor->Notify(NULL); // can't undo
	FailOSErr(err);
}

/********************************************************\
 GetHeight - get the height of the indicated lines
\********************************************************/

long CWASTEText::GetHeight(long startLine, long endLine)
{
	long height;
	long nLines;
	
	nLines = GetNumLines();
	height = WEGetHeight( startLine - 1, endLine, macWE);
	if (endLine >= nLines && ((char*)(*GetTextHandle()))[GetLength()-1] == 13)
		height += WEGetHeight(nLines - 2, nLines - 1, macWE);

	return height;
}

/********************************************************\
 GetCharOffset - return offset of character at point in
 	frame coords
\********************************************************/

long CWASTEText::GetCharOffset(LongPt *aPt)
{
	Point	qdPt;
	LongPt lPt;
	char edge;
	
	Prepare();
	FrameToQD( aPt, &qdPt);
	lPt.v = qdPt.v;
	lPt.h = qdPt.h;
	
	return WEGetOffset( &lPt, &edge, macWE);
	
}

/********************************************************\
 GetCharPoint - return the position of the character in
 	Frame coordinates
\********************************************************/

void CWASTEText::GetCharPoint( long offset, LongPt *aPt)
{
	Point	qdPt;
	LongPt lPt;
	short lineHeight;
	
	ASSERT( offset <= MAXLONG);
	
	Prepare();
	WEGetPoint(offset, &lPt, &lineHeight, macWE);
	qdPt.h = lPt.h;
	qdPt.v = lPt.v;
	QDToFrame( qdPt, aPt);
}

/********************************************************\
 GetTextStyle - return style of selection
\********************************************************/

void CWASTEText::GetTextStyle(short *whichAttributes, TextStyle *aStyle)
{
	WEContinuousStyle(whichAttributes, aStyle, macWE);
}

StScrpHandle CWASTEText::GetTextStyles(void)
{
	OSErr			err;
	StScrpHandle	styles;
	Boolean			saveAllocState;
	
	styles = (StScrpHandle) NewHandleCanFail(0);
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	err = WECopyRange(0, WEGetTextLength(macWE), NULL, styles, NULL, macWE);
#else
	err = WECopyRange(0, WEGetTextLength(macWE), NULL, styles, macWE);
#endif
	SetAllocation(saveAllocState);
	if (err != noErr)
	{
		DisposeHandle((Handle) styles);
		FailOSErr(err);
	}

	return (styles);
}

/********************************************************\
 GetCharStyle - return style of one character
\********************************************************/

void CWASTEText::GetCharStyle(long charOffset, TextStyle *theStyle)
{
	WERunInfo runInfo;

	WEGetRunInfo(charOffset, &runInfo, macWE);
	*theStyle = runInfo.runStyle;
}

/********************************************************\
 GetSpacingCmd - not really supported
\********************************************************/

long CWASTEText::GetSpacingCmd(void)
{
	return spacingCmd;
}

/********************************************************\
 GetAlignCmd - not really supported
\********************************************************/

long CWASTEText::GetAlignCmd(void)
{
	return alignCmd;
}

/********************************************************\
 GetTheStyleScrap -- get a scrap handle of the styles
 	of the selection
\********************************************************/

StScrpHandle CWASTEText::GetTheStyleScrap(void)
{
	Boolean		saveAllocState;
	OSErr		err;
	long		selStart, selEnd;
	StScrpHandle h;

	WEGetSelection(&selStart, &selEnd, macWE);

	h=(StScrpHandle)NewHandleCanFail(1);
	FailNIL(h);
	saveAllocState = SetAllocation(kAllocCanFail);
#ifdef WASTE11
	err = WECopyRange(selStart, selEnd, (Handle)NULL, h, NULL, macWE);
#else
	err = WECopyRange(selStart, selEnd, (Handle)NULL, h, macWE);
#endif
	SetAllocation(saveAllocState);
	FailOSErr(err);
	
	return h;
}

#ifdef WASTE11
/********************************************************\
 GetTheSoup - return the soup of the text
\********************************************************/

WESoupHandle CWASTEText::GetTheSoup(void)
{
	Boolean		saveAllocState;
	OSErr		err;
	long		selStart, selEnd;
	WESoupHandle h;

	WEGetSelection(&selStart, &selEnd, macWE);

	h=(WESoupHandle)NewHandleCanFail(1);
	FailNIL(h);
	saveAllocState = SetAllocation(kAllocCanFail);
	err = WECopyRange(selStart, selEnd, (Handle)NULL, NULL, h, macWE);
	SetAllocation(saveAllocState);
	FailOSErr(err);
	
	return h;
}
#endif

/********************************************************\
 GetNumLines - return the number of lines
\********************************************************/

long CWASTEText::GetNumLines(void)
{
	long nLines = WECountLines(macWE);

	if (!GetLength()) return 0;
	if (((char*)(*GetTextHandle()))[GetLength()-1] == 13)
		nLines++;
	return nLines;
}

/********************************************************\
 GetSelection - get the position of the start and end
 	of the selection
\********************************************************/

void CWASTEText::GetSelection(long *selStart, long *selEnd)
{
	WEGetSelection(selStart, selEnd, macWE);
}

/********************************************************\
 HideSelection - not implemented
\********************************************************/

void CWASTEText::HideSelection(Boolean hide, Boolean redraw)
{
	return;
}

/********************************************************\
 GetSteps - get the size of the scrolling steps
 	Step sizes hard coded in
\********************************************************/

void CWASTEText::GetSteps(short *hStep, short *vStep)
{
	long		nLines,
				height;

	nLines = GetNumLines();
	height = GetHeight(0, MAXINT);
	*hStep = 20;
	if (nLines)
		*vStep = Min(height / nLines, 30);
	else
		*vStep = height;
}

/********************************************************\
 AboutToPrint -- called right before printing
\********************************************************/

void CWASTEText::AboutToPrint(short *firstPage, short *lastPage)
{
	if (active)
	{
		Prepare();
		HidePen();
		WEDeactivate(macWE);
		ShowPen();
	}

	CAbstractText::AboutToPrint(firstPage, lastPage);
}


void CWASTEText::Paginate(CPrinter *aPrinter, short pageWidth, short pageHeight)
{
	Boolean		saveAllocState;
	OSErr		err;
	LongRect	dr;

	WEGetDestRect(&dr, macWE);
	dr.right = dr.left + pageWidth;
	WESetDestRect(&dr, macWE);
	saveAllocState = SetAllocation(kAllocCanFail);
	err = WECalText(macWE);
	SetAllocation(saveAllocState);
	FailOSErr(err);
	printPageWidth = pageWidth;
	
	CAbstractText::Paginate(aPrinter, pageWidth, pageHeight);
}


/********************************************************\
 PrintPage -- called to print page
\********************************************************/

void CWASTEText::PrintPage(short pageNum, short pageWidth, short pageHeight,
						   CPrinter *aPrinter)
{
	OSErr		err;
	short		oldOffscreen,
				oldOutline;

	err = WESetInfo(wePort, (Ptr)&qd.thePort, macWE);
	// temporarily turn off off screen drawing and outline hiliting
	oldOffscreen = WEFeatureFlag(weFDrawOffscreen, weBitClear, macWE);
	oldOutline = WEFeatureFlag(weFOutlineHilite, weBitClear, macWE);

//	if (printClip == clipPAGE) {		/* Expand viewRect to the size of a	*/
//										/*   page. It will be restored by	*/
//										/*   the DonePrinting() method.		*/
//		WEGetViewRect(&viewRect, macWE);
//		viewRect.right = viewRect.left + pageWidth;
//		
//		if (wholeLines) {
//			viewRect.bottom = viewRect.top +
//											vScale * (pageHeight / vScale);
//		} else {
//			viewRect.bottom = viewRect.top + pageHeight;
//		}
//		WESetViewRect(&viewRect, macWE);
//	}

	itsPrinter = aPrinter;
	CAbstractText::PrintPage(pageNum, pageWidth, pageHeight, aPrinter);

	err = WESetInfo(wePort, (Ptr)&macPort, macWE);
	WEFeatureFlag(weFDrawOffscreen, oldOffscreen, macWE);
	WEFeatureFlag(weFOutlineHilite, oldOutline, macWE);
}

/********************************************************\
 DonePrinting - called when printing is finished
\********************************************************/

void CWASTEText::DonePrinting()
{
	Boolean		saveAllocState;
	OSErr		err;

	CAbstractText::DonePrinting();

	if (active)
	{
		Prepare();
		HidePen();
		WEActivate(macWE);
		ShowPen();
	}

	TCL_ASSERT(itsPrinter);
	itsPrinter->ResetPagination();
	itsPrinter = NULL;

	CalcWERects();
	if (lineWidth < 0)
	{
		saveAllocState = SetAllocation(kAllocCanFail);
		err = WECalText( macWE);
		SetAllocation(saveAllocState);
	}
}

/********************************************************\
 Dawdle - do idle stuff
\********************************************************/

void CWASTEText::Dawdle(long *maxSleep)
{
	if (visible)
	{
		Prepare();
		WEIdle(NULL, macWE);
		*maxSleep = GetCaretTime();
	}
}

/********************************************************\
 PutTo - not implemented
\********************************************************/

void CWASTEText::PutTo(CStream& stream)
{
	return;
}

/********************************************************\
 GetFrom - not implemented
\********************************************************/

void CWASTEText::GetFrom(CStream& stream)
{
	return;
}

/********************************************************\
 WEClickLoop - click loop routine
\********************************************************/

pascal Boolean CWASTEText::WEClickLoop(WEHandle hWE)
{
	Point		mouseLoc;
	LongPt		longMouse;

	if (curWASTEText != NULL) {
		GetMouse(&mouseLoc);
		curWASTEText->QDToFrame(mouseLoc, &longMouse);
		curWASTEText->AutoScroll( &longMouse);
	}
	return(TRUE);
}

// static variables for the following routines
static Boolean wasActive, outlineHilite;
static long selStart, selEnd;

/********************************************************\
 TempSelectAll -- temporarily selects all of the text
 	while deactivating the text and turning off 
 	outline hilighting so that the selection won't show
 	up.  It can be restored by Restore selection.  This
 	is useful when you want to apply something to all
 	of the text without seeing all of the text flash.
 	
 	Note: Calls to TempSelectAll/RestoreSelection cannot
 	be nested
\********************************************************/

void CWASTEText::TempSelectAll(void)
{
	// turn off outline highlighting temporarily
	outlineHilite = SetOutlineHighliting(false);

	// deactivate text
	wasActive = active;
	if (wasActive) Deactivate();
	
	// select all
	GetSelection(&selStart, &selEnd);
	SetSelection(0, 0x7FFFFFF, false);
}
	
/********************************************************\
 RestoreSelection - restores things to the way they
 	were before a call to TempSelectAll()
\********************************************************/

void CWASTEText::RestoreSelection(void)
{
	if (wasActive) Activate();
	SetOutlineHighliting(outlineHilite);
	SetSelection(selStart, selEnd, false);
}

/******************************************************************************
 Specify

	If editable is changed and it's active, enable/disable TSMInput
 ******************************************************************************/

void CWASTEText::Specify(Boolean fEditable, Boolean fSelectable, Boolean fStylable)
{
	Boolean wasEditable = editable;
#ifndef WASTE11
	TSMDocumentID aTSMDocument;
#endif
	
	inherited::Specify(fEditable, fSelectable, fStylable);

#ifdef WASTE11
	// use weFReadOnly flag
	if (macWE)
	{
		if (!fEditable) WEFeatureFlag(weFReadOnly, weBitSet, macWE);
		else WEFeatureFlag(weFReadOnly, weBitClear, macWE);
	}
#else
	if (active && wasEditable && !editable)
	{
		// if not editable, deactivate TSM so inline input won't work
		if (gUsingTSM)
		{
			if (WEGetInfo(weTSMDocumentID, (Ptr)&aTSMDocument, macWE) == noErr)
			{
				if (aTSMDocument != NULL)
					DeactivateTSMDocument(aTSMDocument);
			}
		}
	}

	if (active && !wasEditable && editable)
	{
		// if editable, activate TSM so inline input will work
		if (gUsingTSM)
		{
			if (WEGetInfo(weTSMDocumentID, (Ptr)&aTSMDocument, macWE) == noErr)
			{
				if (aTSMDocument != NULL)
					ActivateTSMDocument(aTSMDocument);
			}
		}
	}
#endif
}

/******************************************************************************
 AdjustCursor

	Use WEAdjustCursor to set the cursor
 ******************************************************************************/

void CWASTEText::AdjustCursor(Point where, RgnHandle mouseRgn)
{
	Point		wTopLeft;

	if (wantsClicks)					// TCL 2.0
	{
		Prepare();
	
		// convert back to global coords
		wTopLeft = topLeft((**((WindowPeek)GetWindow()->macPort)->contRgn).rgnBBox);
		where.h += wTopLeft.h;
		where.v += wTopLeft.v;

		WEAdjustCursor(where, mouseRgn, macWE);
	}
}

/******************************************************************************
 InhibitRecal
	turn on or off inhibition of recalibration
 ******************************************************************************/

void CWASTEText::InhibitRecal(Boolean inhibit)
{
	Boolean oldInhibit;
	
	oldInhibit = WEFeatureFlag(weFInhibitRecal, inhibit ? weBitSet : weBitClear, macWE);
	if (oldInhibit && !inhibit)
	{
		FailOSErr(WECalText(macWE));
		AdjustBounds();
//		Refresh();
	}
}

#ifdef WASTE11
/******************************************************************************
 InsertPicture
	Insert a picture
 ******************************************************************************/

void CWASTEText::InsertPicture(PicHandle pHandle, Boolean fRedraw)
{
	Point zeroPoint = {0, 0};
	Boolean		saveAllocState;
	OSErr		err;
	Boolean		wasReadOnly;

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	CheckInsertion((*pHandle)->picSize, 0, TRUE);
	saveAllocState = SetAllocation(kAllocCanFail);
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
	err = WEInsertObject(kTypePicture, (Handle)pHandle, zeroPoint, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
	SetAllocation(saveAllocState);

	AdjustBounds();
	if (fRedraw) Refresh();
	FailOSErr(err);
}

/******************************************************************************
 InsertSound
	Insert a sound
 ******************************************************************************/

void CWASTEText::InsertSound(Handle sHandle, Boolean fRedraw)
{
	Point zeroPoint = {0, 0};
	Boolean		saveAllocState;
	OSErr		err;
	Boolean		wasReadOnly;

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	CheckInsertion(GetHandleSize(sHandle), 0, TRUE);
	saveAllocState = SetAllocation(kAllocCanFail);
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
	err = WEInsertObject(kTypeSound, sHandle, zeroPoint, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
	SetAllocation(saveAllocState);

	AdjustBounds();
	if (fRedraw) Refresh();
	FailOSErr(err);
}

#ifdef WASTE_OBJECT_ARCHIVE
/******************************************************************************
 InsertFSSpec
	Insert a file
 ******************************************************************************/

void CWASTEText::InsertFSSpec(FSSpec *theFSSpec, Boolean fRedraw)
{
	Point zeroPoint = {0, 0};
	Boolean		saveAllocState;
	OSErr		err;
	Boolean		wasReadOnly;
	Handle		fsHandle;

	Prepare();
	if (!ReallyVisible())
		SetOrigin(-10000, -10000);
	fsHandle = NewHandle(sizeof(FSSpec));
	FailNIL(fsHandle);
	*((FSSpec *)*fsHandle) = *theFSSpec;
	saveAllocState = SetAllocation(kAllocCanFail);
	wasReadOnly = WEFeatureFlag(weFReadOnly, weBitClear, macWE);
	err = WEInsertObject(flavorTypeHFS, fsHandle, zeroPoint, macWE);
	WEFeatureFlag(weFReadOnly, wasReadOnly, macWE);
	SetAllocation(saveAllocState);

	AdjustBounds();
	if (fRedraw) Refresh();
	FailOSErr(err);
}
#endif // WASTE_OBJECT_ARCHIVE

#endif

#ifdef WASTE11

/******************************************************************************
 DoDrag
	
	See if a drag is started, and if so, handle click-through properly
 ******************************************************************************/

Boolean CWASTEText::DoDrag(EventRecord *theEvent, Point where)
{
	long selStart, selEnd;
	RgnHandle selRgn;

	if (!gHasDragAndDrop) return false;
	
	Prepare();

	WEGetSelection(&selStart, &selEnd, macWE);
	selRgn = WEGetHiliteRgn(selStart, selEnd, macWE);
	if (PtInRgn(where, selRgn) && WaitMouseMoved(theEvent->where))
	{
		// doing a drag -- just call DoClick now...
		DoClick(where, theEvent->modifiers, theEvent->when);
		return true;
	}
	return false;

}

/******************************************************************************
 GetUndoInfo - get information about what's in WASTE's undo stack
 ******************************************************************************/

WEActionKind CWASTEText::GetUndoInfo(Boolean *redoFlag)
{
	return WEGetUndoInfo(redoFlag, macWE);
}

/******************************************************************************
 DoUndo - perform undo operation
 ******************************************************************************/

void CWASTEText::DoUndo(void)
{
	FailOSErr(WEUndo(macWE));
}

#endif

/* Copyright (C) 1993 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* macEvent.c */
/* Macintosh graphics window event loop driver for Ghostscript library */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <console.h>
#include "gdevmac.h"
#include "gserrors.h"
#include "gp_mac.h"
#include "gmacprefs.h"
#include "macPrint.h"
#include "gp_macui.h"
#include "drvr.h"


static void DoInit			(MenuHandle hGUIMenu, short menuOffset);
static void DoCursor		(void);
static void DoCut			(void);
static void DoCopy			(void);
static void DoClear			(void);
static void DoMenu			(short menuItem);
static void DoActivate		(Boolean fDoActive, CWindowPtr windowPtr);
static void DoUpdate		(CWindowPtr windowPtr);
static void DoMouseDown		(EventRecord *pEvent);
static void DoKeyEvent		(EventRecord *pEvent);
static void DoConsolePuts	(void);
static void DoOpenFilePre	(void);
static void DoOpenFilePost	(void);
static char DoOptionsProc	(short actionValue);
static void DoTerminate		(void);


GUIProcRec macGUIProcs =
{
	DoInit,
	DoCursor,
	DoCut,
	DoCopy,
	(GUINoArgFunc) NULL,	//	DoPaste
	DoClear,
	DoMenu,
	DoActivate,
	DoUpdate,
	DoMouseDown,
	DoKeyEvent,
	DoConsolePuts,
	DoOpenFilePre,
	DoOpenFilePost,
	DoOptionsProc,	//	(GUIOptionsProc) NULL,	//	DoOptionsProc
	DoTerminate
};


extern gx_device_mac	gs_mac_device;
extern Boolean			gDoCheckInterrupts;


typedef struct
{
	short			graphicsRefNum;			/*	window driver reference number	*/

	RgnHandle		saveClipRgn;			/*	original clipping region 		*/
	RgnHandle		clipRgn;				/*	clipping region for scrollbars	*/
	RgnHandle		updRgn;					/*	update region for scrolling		*/
	Rect			updRect;
	Boolean			fIsDirty;

	Point			origin;

	Rect			viewRect;
	short			viewWidth;
	short			viewHeight;

	PicHandle		hPicture;				/*	current (possibly open) picture	*/
	Size			pictLen;				/*	current picture size			*/
	Rect			pictRect;
	CWindowPtr		pictWin;
	Boolean			fOpenPicture;

	Rect			growRect;				/*	GrowWindow () limits			*/

	ControlHandle	hScrollBar;
	short			hValue;
	ControlHandle	vScrollBar;
	short			vValue;
	short			delScroll;

	ResType			fCreator;

	GrafPtr			savePort;

} MacWinRec, *MacWinPtr, **MacWinH;


#define mac_white_color_index		(gx_color_index) 0x00FFFFFF

enum
{
	iSavePicture = 1,
	iSaveSelection,
	iCopySelection,
	_iLine1,
	iPageSetUp,
	iPrint,
	_iLine2,
	iCmdWindow,
	iResume,
	iStopProc,

	__iLastMacGSMenuItem
};


void		macCreatePicture (gx_device *dev);
void		macClosePicture  (gx_device *dev);
void		macClearPicture  (gx_device *dev);
void		macCopyPicture   (gx_device *dev);
PicHandle	macGetPicture	 (gx_device *dev);
Boolean		macSavePicture	 (gx_device *dev, PicHandle hPicture);
short		macCreateWindow	 (gx_device *dev, short pictWidth, short pictHeight);
void		macClearWindow   (gx_device *dev);
short		macCloseWindow   (gx_device *dev);
void		macInvalWindow   (gx_device *dev, Boolean fResetOrigin);
void		macRedrawWindow  (gx_device *dev);
void		macResizeWindow  (gx_device *dev);
void 		macBeginDraw	 (gx_device *dev);
void 		macEndDraw  	 (gx_device *dev);
void 		macBeginPictDraw (gx_device *dev);
short 		macEndPictDraw 	 (gx_device *dev);
CWindowPtr	macPictWindow	 (gx_device *dev);
Rect	   *macViewRect		 (gx_device *dev);


static MenuHandle	hGUIMenu		 = (MenuHandle) NULL;
static short		numGUIMenuItems;
static Str255		cmdWindowStr;
static Str255		graphicsWindowStr;
static Boolean		fMenuSavePicture = FALSE;
static Boolean		fIsInConfirm	 = FALSE;
static short		old_mac_pause_atexit;
static GrafPtr		openFileSavePort = (GrafPtr) NULL;


static void		macConfirmSetup (short state);
static void		doMacConfirmCB	(Boolean fIsActive);
static void		swapConsoleControl (Boolean fSaveConsoleControl);


#define MENUAPPENDSTR(index)							\
{														\
	GetIndString (string, MACMENUSTRS_RES_ID, index);	\
	AppendMenu (hGUIMenu, string);						\
}


/*
 *
 */

	static void
DoInit (MenuHandle theGUIMenuH, short menuOffset)

{	
	Str255		string;


	hGUIMenu		= theGUIMenuH;
	numGUIMenuItems = menuOffset;

	MENUAPPENDSTR (iSavePictureMenuStr);
	MENUAPPENDSTR (iPrintMenuStr);
	MENUAPPENDSTR (iCommandMenuStr);

	GetItem (hGUIMenu, iCmdWindow + numGUIMenuItems, cmdWindowStr);
	GetIndString ((StringPtr) graphicsWindowStr, MACMENUSTRS_RES_ID, iGraphicsWindowStr);
	SetItem (hGUIMenu, iCmdWindow + numGUIMenuItems, graphicsWindowStr);

	(void) doRunIndString (iShowpageMac);
}


/*
 *
 */

	static void
DoCursor (void)

{
	Point		where;
	CWindowPtr	windowPtr = gs_mac_device.windowPtr;
	MacWinH		hMacWin   = (MacWinH) GetWRefCon ((WindowPtr) windowPtr);
	Rect 	   *pViewRect = &((*hMacWin)->viewRect);
	Boolean		gp_in_check_interrupts (void);
	void		macInitCursor (void);


	if (gp_in_check_interrupts ())
		return;

	SetPort ((GrafPtr) windowPtr);
	GetMouse (&where);

	if (PtInRect (where, pViewRect))
		SetCursor (*GetCursor (fIsInConfirm ? plusCursor : crossCursor));
	else
		macInitCursor ();
}


	static void
DoCut (void)

{
	macClosePicture ((gx_device *) &gs_mac_device);
	macCopyPicture  ((gx_device *) &gs_mac_device);
	macClearWindow  ((gx_device *) &gs_mac_device);
}


	static void
DoCopy (void)

{
	macClosePicture ((gx_device *) &gs_mac_device);
	macCopyPicture  ((gx_device *) &gs_mac_device);
}


	static void
DoClear (void)

{
	if (fIsInConfirm)
		macConfirmSetup (2);
	else
		macClearWindow ((gx_device *) &gs_mac_device);
}


/*
 *	DoMenu -- handles MacGS menu events
 *
 */

	static void
DoMenu (short menuItem)

{
	switch (menuItem - numGUIMenuItems)
	{
		case iSavePicture:
		{
			PicHandle	hPicture = macGetPicture ((gx_device *) &gs_mac_device);


			macClosePicture ((gx_device *) &gs_mac_device);
			fMenuSavePicture = macSavePicture ((gx_device *) &gs_mac_device, hPicture);

			break;
		}

		case iSaveSelection:
		{
			Boolean		macSavePictureSelection (gx_device *dev);


			macClosePicture	((gx_device *) &gs_mac_device);
			fMenuSavePicture = macSavePictureSelection ((gx_device *) &gs_mac_device);

			break;
		}

		case iCopySelection:
		{
			void	macCopyPictureSelection (gx_device *dev);


			macClosePicture	((gx_device *) &gs_mac_device);
			macCopyPictureSelection ((gx_device *) &gs_mac_device);

			break;
		}

		case iResume:
		{
			macConfirmSetup (2);

			break;
		}

		case iStopProc:
		{
			macConfirmSetup (3);

			break;
		}

		case iPageSetUp:
		{
			MacDoPrStyleDlg ();

			break;
		}

		case iPrint:
		{
			void	doPrint (gx_device *dev);


			doPrint ((gx_device *) &gs_mac_device);

			break;
		}

		case iCmdWindow:
		{
			SendBehind (FrontWindow (), (WindowPtr) NULL);
			break;
		}
	}

	HiliteMenu (0);
}


	static void
DoActivate (Boolean fDoActive, CWindowPtr windowPtr)

{
	MacWinH			hMacWin		= (MacWinH) GetWRefCon ((WindowPtr) windowPtr);
	CWindowPtr		pictWin		= (*hMacWin)->pictWin;
	MenuHandle		hEditMenu	= GetMHandle (EditMenuID);
	Boolean			fDoPost		= TRUE;
	short			cClear;


	if (fDoActive)
	{
		DisableItem (hEditMenu, iUndo);
		DisableItem (hEditMenu, iPaste);
		cClear = (short) 'K';
		HiliteControl ((*hMacWin)->hScrollBar, 0);
		HiliteControl ((*hMacWin)->vScrollBar, 0);
	}
	else
	{
		if (!fMenuSavePicture)
		{
			EnableItem (hEditMenu, iUndo);
			EnableItem (hEditMenu, iPaste);
			cClear = (short) '\0';
		}
		else
		{
			fMenuSavePicture = FALSE;
			fDoPost			 = FALSE;
		}

		HiliteControl ((*hMacWin)->hScrollBar, 255);
		HiliteControl ((*hMacWin)->vScrollBar, 255);
	}

	macConfirmSetup (0);

	if (fDoPost)
	{
		SetItemCmd  (hEditMenu, iClear, cClear);
		DrawMenuBar ();
		DrawGrowIcon ((WindowPtr) windowPtr);
	}

	SetItem (hGUIMenu, iCmdWindow + numGUIMenuItems,
			 fDoActive ? cmdWindowStr : graphicsWindowStr);
}


	static void
DoUpdate (CWindowPtr windowPtr)

{
	CWindowPeek	pWindowPeek = (CWindowPeek) windowPtr;
	MacWinH		hMacWin		= (MacWinH) GetWRefCon ((WindowPtr) pWindowPeek);
	CWindowPtr	pictWin		= (*hMacWin)->pictWin;


	if (!(*hMacWin)->fOpenPicture &&
		(*hMacWin)->hPicture /* != (PicHandle) NULL */)
	{
		gx_device   *dev = (gx_device *) &gs_mac_device;


		macBeginDraw (dev);
			DrawPicture ((*hMacWin)->hPicture, &((*hMacWin)->pictRect));
		macEndDraw (dev);

		(*hMacWin)->fIsDirty = FALSE;
	}

	DrawControls ((WindowPtr) pWindowPeek);
	DrawGrowIcon ((WindowPtr) pWindowPeek);
}


enum
{
	iPostDoNothing,
	iPostCmdWindow,
	iPostScrollKeys
};


/*
 *	DoKeyEvent -- handles keyboard events
 *
 */

	static void
DoKeyEvent (EventRecord *pEvent)

{
	gx_device  *dev = (gx_device *) &gs_mac_device;


	if (pEvent->modifiers & cmdKey)
	{
		char			aChar		= pEvent->message & charCodeMask;
		short			flashMenuID = 0;
		char			iPostAction = iPostDoNothing;


		if ('a' <= aChar && aChar <= 'z')
			aChar += 'A' - 'a';

		switch (aChar)
		{
			case 'R':
			{
				macConfirmSetup (2);
				flashMenuID = MacGSMenuID;

				break;
			}

			case 'P':
			{
				void	doPrint (gx_device *dev);


				FlashMenuBar (MacGSMenuID);
				flashMenuID = MacGSMenuID;
				doPrint (dev);

				break;
			}

			case 'S':
			{
				FlashMenuBar (MacGSMenuID);
				flashMenuID = MacGSMenuID;

				fMenuSavePicture = macSavePicture (dev, macGetPicture (dev));
				break;
			}

			case 'W':
			{
				flashMenuID = MacGSMenuID;
				iPostAction = iPostCmdWindow;
				break;
			}

			case 'C':
			{
				DoCopy ();
				flashMenuID = EditMenuID;
				break;
			}

			case 'X':
			{
				DoCut ();
				flashMenuID = EditMenuID;
				break;
			}

			case 'K':
			{
				macClosePicture (dev);
				if (fIsInConfirm)
					macConfirmSetup (2);
				else
					macClearWindow (dev);
				flashMenuID = EditMenuID;
				break;
			}

			case 'Q':
			{
				macClosePicture (dev);
				break;
			}
		}

		if (flashMenuID /* != 0 */)
		{
			FlashMenuBar (flashMenuID);
			if (flashMenuID > 0)
				FlashMenuBar (flashMenuID);
			HiliteMenu (0);
		}

		switch (iPostAction)
		{
			case iPostCmdWindow:
				SendBehind (FrontWindow (), (WindowPtr) NULL);
				break;

			default:
				break;
		}
	}
	else
	{
		char			aChar		= pEvent->message & charCodeMask;
		short			flashMenuID = 0;
		char			iPostAction = iPostDoNothing;
		ControlHandle	hControl	= (ControlHandle) NULL;
		short			iCtlPart;


		if ('a' <= aChar && aChar <= 'z')
			aChar += 'A' - 'a';

		switch (aChar)
		{
			case 0x01:				/*	home		*/
			case 0x04:				/*	end			*/
			case 0x0B:				/*	page up		*/
			case 0x0C:				/*	page down	*/
			case 0x1C:				/*  left arrow	*/
			case 0x1D:				/*  right arrow */
			case 0x1E:				/*  up arrow	*/
			case 0x1F:				/*  down arrow  */
			{
				MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


				switch (aChar)
				{
					case 0x1C:
					case 0x1D:
						hControl = (*hMacWin)->hScrollBar;
						break;

					case 0x1E:
					case 0x1F:
						hControl = (*hMacWin)->vScrollBar;
						break;

					default:
						hControl = (pEvent->modifiers & optionKey)
									   ? (*hMacWin)->hScrollBar
									   : (*hMacWin)->vScrollBar;
						break;
				}

				switch (aChar)
				{
					case 0x1C:
					case 0x1E:
						iCtlPart = inUpButton;
						break;

					case 0x1D:
					case 0x1F:
						iCtlPart = inDownButton;
						break;

					case 0x0B:
						iCtlPart = inPageUp;
						break;

					case 0x0C:
						iCtlPart = inPageDown;
						break;

					case 0x01:
					case 0x04:
					{
						iCtlPart = inThumb;

						if (aChar == 0x01)
							SetCtlValue (hControl, GetCtlMin (hControl));
						else
							SetCtlValue (hControl, GetCtlMax (hControl));

						break;
					}
				}

				iPostAction = iPostScrollKeys;

				break;
			}

			default:
				break;
		}

		if (flashMenuID /* != 0 */)
		{
			FlashMenuBar (flashMenuID);
			FlashMenuBar (flashMenuID);
		}

		switch (iPostAction)
		{
			case iPostScrollKeys:
			{
				if (hControl /* != (ControlHandle) NULL */)
				{
					pascal void	doScrollProc (ControlHandle	whichCtl, short iCtlPart);


					macClosePicture (dev);

					doScrollProc (hControl, iCtlPart);
				}

				break;
			}

			default:
				break;
		}
	}
}


/*
 *	DoMouseDown -- handles mouse down events
 *
 */

	static void
DoMouseDown (EventRecord *pEvent)

{
	gx_device  *dev		= (gx_device *) &gs_mac_device;
	MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	WindowPtr	whichWin;
	short		iWinPart;


	((CWindowPeek) xdev->windowPtr)->windowKind = userKind;
	iWinPart = FindWindow (pEvent->where, &whichWin);
	((CWindowPeek) xdev->windowPtr)->windowKind = (*hMacWin)->graphicsRefNum;

	if (FrontWindow () != whichWin)
	{
		SelectWindow (whichWin);
	}
	else
	{
		GetPort (&(*hMacWin)->savePort);
		SetPort (whichWin);

		switch (iWinPart)
		{
			case inZoomIn:
			case inZoomOut:
			{
				Rect   *pRect = &(((WindowPeek) whichWin)->port.portRect);


				InitCursor ();
				SetPort ((GrafPtr) whichWin);

				if (TrackBox (whichWin, pEvent->where, iWinPart))
				{
					macClosePicture (dev);

					if (iWinPart == inZoomOut)
					{
						macInvalWindow (dev, TRUE);
					}

					ZoomWindow (whichWin, iWinPart, FALSE);

					macRedrawWindow (dev);
				}
				
				break;
			}

			case inGrow:
			{
				long	newSize;


				InitCursor ();
				SetPort ((GrafPtr) whichWin);

				if ((newSize = GrowWindow (whichWin, pEvent->where,
										   &(*hMacWin)->growRect))
						/* != 0L */)
				{
					macClosePicture (dev);

					macInvalWindow (dev, FALSE);

					SizeWindow (whichWin,
								LoWord (newSize), HiWord (newSize), TRUE);

					macRedrawWindow (dev);
				}
				
				break;
			}

			case inDrag:
			{
				Rect	limitRect;


				BlockMove ((Ptr) &screenBits.bounds, (Ptr) &limitRect,
						   (Size) sizeof (Rect));
				InsetRect (&limitRect, 4, 4);
				DragWindow (whichWin, pEvent->where, &limitRect);

				break;
			}

			case inContent:
			{
				ControlHandle	whichCtl;
				short			iCtlPart;


				GlobalToLocal (&pEvent->where);

				if ((iCtlPart = FindControl (pEvent->where, whichWin, &whichCtl))
						/* > 0 */)
				{
					pascal void	doScrollProc (ControlHandle	whichCtl,
											  short iCtlPart);


					macClosePicture (dev);

					iCtlPart = TrackControl (whichCtl, pEvent->where,
											 (iCtlPart != inThumb)
											 	 ? doScrollProc
											 	 : (ProcPtr) NULL);

					if (iCtlPart == inThumb)
						doScrollProc (whichCtl, iCtlPart);
				}

				break;
			}
		}

		SetPort ((*hMacWin)->savePort);
	}
}


	static void
DoConsolePuts (void)

{
	if (openFileSavePort /* != (GrafPtr) NULL */)
		SetPort (openFileSavePort);
}


	static void
DoOpenFilePre (void)

{
	macClosePicture ((gx_device *) &gs_mac_device);

	SetDoMacOpenFile (TRUE);

	openFileSavePort = (GrafPtr) FrontWindow ();
	gDoCheckInterrupts	= TRUE;
	swapConsoleControl (TRUE);
}


	static void
DoOpenFilePost (void)

{
	swapConsoleControl (FALSE);
	gDoCheckInterrupts	= FALSE;
	SetPort (openFileSavePort);

	SetDoMacOpenFile (FALSE);

	macClosePicture ((gx_device *) &gs_mac_device);

	openFileSavePort = (GrafPtr) NULL;
}


	static char
DoOptionsProc (short actionValue)

{
	char	retVal = TRUE;


	switch (actionValue)
	{
		case iInitDialogAction:
			retVal = FALSE;
			break;

		case iDoDialogAction:
			break;

		case iCommitAction:
			(void) doRunIndString (iShowpageMac);
			break;

		case iRetractAction:
			break;

		default:
			break;
	}

	return retVal;
}


	static void
DoTerminate (void)

{
	(void) doRunIndString (iShowpageOrig);
}


/*
 *	doScrollProc -- handles immediate feedback for scrollbars
 *
 */

	static pascal void
doScrollProc (ControlHandle	whichCtl, short iCtlPart)

{
	gx_device  *dev			= (gx_device *) GetCRefCon (whichCtl);
	MacWinH		hMacWin		= (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	Boolean		fIsVertical = (whichCtl == (*hMacWin)->vScrollBar);
	short	   *pCtlVal		= fIsVertical
								  ? &((*hMacWin)->vValue)
								  : &((*hMacWin)->hValue);
	short		ctlVal		= *pCtlVal;
	short		ctlMin		= GetCtlMin (whichCtl);
	short		ctlMax		= GetCtlMax (whichCtl);
	short		offset		= 0;
	short		dx			= 0;
	short		dy			= 0;


	if (iCtlPart == 0)
		return;

	switch (iCtlPart)
	{
		case inUpButton:
		{
			offset = -(*hMacWin)->delScroll;

			break;
		}

		case inDownButton:
		{
			offset =  (*hMacWin)->delScroll;

			break;
		}

		case inThumb:
		{
			offset = (*pCtlVal = GetCtlValue (whichCtl)) - ctlVal;

			break;
		}

		case inPageUp:
		{
			if (fIsVertical)
				offset = -(*hMacWin)->viewHeight;
			else
				offset = -(*hMacWin)->viewWidth;

			break;
		}

		case inPageDown:
		{
			if (fIsVertical)
				offset = (*hMacWin)->viewHeight;
			else
				offset = (*hMacWin)->viewWidth;

			break;
		}
	}

	if		(ctlMin > ctlVal + offset)
		offset = ctlMin - ctlVal;
	else if (ctlMax < ctlVal + offset)
		offset = ctlMax - ctlVal;

	if (fIsVertical)
		dy = -offset;
	else
		dx = -offset;

	if (offset /* != 0 */)
	{
		RgnHandle	hUpdRgn = (*hMacWin)->updRgn;


		if (iCtlPart != inThumb)
		{
			ctlVal = GetCtlValue (whichCtl) + offset;
			SetCtlValue (whichCtl, *pCtlVal = ctlVal);
		}

		GetPort (&(*hMacWin)->savePort);
		SetPort ((GrafPtr) xdev->windowPtr);

		/*...Scroll the previous update region...*/

		if ((*hMacWin)->fIsDirty)
		{
			Rect	invalRect = (*hUpdRgn)->rgnBBox;


			OffsetRect (&(*hMacWin)->updRect, dx, dy);
			UnionRect  (&(*hMacWin)->updRect, &invalRect, &(*hMacWin)->updRect);
			InvalRect  (&(*hMacWin)->updRect);
		}

		/*...Scroll the image...*/

		ScrollRect (&((*hMacWin)->viewRect), dx, dy, hUpdRgn);

		InvalRgn (hUpdRgn);

		SetPort ((*hMacWin)->savePort);

		/*...Offset the origin...*/

		(*hMacWin)->origin.h -= dx;
		(*hMacWin)->origin.v -= dy;

		if ((*hMacWin)->origin.h < 0)
			(*hMacWin)->origin.h = 0;
		if ((*hMacWin)->origin.v < 15)
			(*hMacWin)->origin.v = 15;

		if (!(*hMacWin)->fIsDirty)
		{
			(*hMacWin)->fIsDirty = TRUE;
			(*hMacWin)->updRect  = (*hUpdRgn)->rgnBBox;
		}
	}
}


	static void
doPrint (gx_device *dev)

{
	MacWinH		hMacWin	= (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


	macClosePicture (dev);
	MacPrintOnePage ((*hMacWin)->hPicture, &(*hMacWin)->pictRect);
}


static Boolean		fDoConfirm = TRUE;
static char			fDoReturn;


	Boolean
doMacConfirm (void)

{
	Boolean	retVal = TRUE;


	if (fDoConfirm)
	{
		extern void		macForceCursorInit (void);


		SetPort (openFileSavePort);
		swapConsoleControl (FALSE);

		doMacConfirmCB (TRUE);
		macForceCursorInit ();

		fDoReturn = 0;

		do
		{
			EventRecord		event;
			WindowPtr		wp;
			long			choice;
			Boolean			status;


			SystemTask ();

			if (gWNEPresent)
				status = WaitNextEvent (everyEvent, &event, 0xFFFFFFFF, (RgnHandle) NULL);
			else
				status = GetNextEvent (everyEvent, &event);

			if (status)
			{
				if (!SystemEvent (&event))
					goto doEvent;
			}
			else if (event.what == nullEvent)
			{
				if (FrontWindow () == 0)
					InitCursor ();
			}

			continue;

		/*  handle event  */

		doEvent:

			if (event.what == mouseDown)
			{
				switch (FindWindow (event.where, &wp))
				{
					case inMenuBar:
					{
						InitCursor ();
						choice = MenuSelect (event.where);
						goto doMenu;
					}

					case inSysWindow:
					{
						SystemClick (&event, wp);
						break;
					}
				}
			}

			continue;
		
				/*  handle menu choice  */
		
		doMenu:

			switch (HiWord (choice))
			{
				case AppleMenuID:		/*  Apple  */
				{
					MenuHandle	appleMenu = GetMHandle (AppleMenuID);
					Str255		buf;


					GetItem (appleMenu, LoWord (choice), buf);
					OpenDeskAcc (buf);

					break;
				}

				case FileMenuID:		/*  File  */
				{
					extern short old_mac_pause_atexit;
					extern void	 gs_exit (int code);


					console_options.pause_atexit = old_mac_pause_atexit;

					gs_exit (0);

					/* unreachable */
				}

				case EditMenuID:		/*  Edit  */
				{
					SystemEdit (LoWord (choice) - 1);

					break;
				}
			}

			HiliteMenu (0);

		} while (fDoReturn == 0);

		doMacConfirmCB (FALSE);

		if (GetDoMacOpenFile ())
			swapConsoleControl (TRUE);

		retVal = (fDoReturn == 1);
	}

	return retVal;
}


	static void
macConfirmSetup (short state)

{
	switch (state)
	{
		case 0:
			fDoConfirm = TRUE;
			break;

		case 1:
			fDoConfirm = FALSE;
			break;

		case 2:
			fDoReturn  = 1;
			break;

		case 3:
			fDoReturn  = -1;
			break;
	}

	macForceCursorInit ();
	InitCursor ();
}


	static void
doMacConfirmCB (Boolean fIsActive)

{
	CWindowPtr	windowPtr = gs_mac_device.windowPtr;
	MacWinH		hMacWin   = (MacWinH) GetWRefCon ((WindowPtr) windowPtr);


	HiliteMenu (0);

	if (fIsInConfirm = fIsActive)
	{
		EnableItem  (hGUIMenu, iResume	  + numGUIMenuItems);
		EnableItem  (hGUIMenu, iStopProc  + numGUIMenuItems);
		DisableItem (hGUIMenu, iOpenFile);
		DisableItem (hGUIMenu, iLaserPrep);
	}
	else
	{
		DisableItem (hGUIMenu, iResume	  + numGUIMenuItems);
		DisableItem (hGUIMenu, iStopProc  + numGUIMenuItems);
		EnableItem  (hGUIMenu, iOpenFile);
		EnableItem  (hGUIMenu, iLaserPrep);
	}

	if (!(*hMacWin)->fOpenPicture &&
		(*hMacWin)->hPicture /* != (PicHandle) NULL */ &&
		fIsInConfirm)
	{
		EnableItem  (hGUIMenu, iSavePicture	  + numGUIMenuItems);
		EnableItem  (hGUIMenu, iSaveSelection + numGUIMenuItems);
		EnableItem  (hGUIMenu, iCopySelection + numGUIMenuItems);
	}
	else
	{
		DisableItem (hGUIMenu, iSavePicture	  + numGUIMenuItems);
		DisableItem (hGUIMenu, iSaveSelection + numGUIMenuItems);
		DisableItem (hGUIMenu, iCopySelection + numGUIMenuItems);
	}

	SetCursor (*GetCursor (watchCursor));
}


#define RESET_SCROLLBAR_ORIGINS()					\
{													\
	/*...Define a drawing origin that avoids the	\
	 *	 horizontal scrollbar...*/					\
													\
	(*hMacWin)->origin.h =  0;						\
	(*hMacWin)->origin.v = 15;						\
													\
	/*...Initialize scrollbar current values...*/	\
													\
	(*hMacWin)->vValue = 0;							\
	(*hMacWin)->hValue = 0;							\
													\
	if (hScrollBar /* != (ControlHandle) NULL */)	\
	{												\
		SetCtlValue (hScrollBar, 0);				\
		SetCtlValue (vScrollBar, 0);				\
	}												\
}


	static void
macCreatePicture (gx_device *dev)

{
	MacWinH			hMacWin	   = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	CWindowPtr		pictWin	   = (*hMacWin)->pictWin;
	Rect		   *pRect	   = &(*hMacWin)->pictRect;
	ControlHandle	vScrollBar = (*hMacWin)->vScrollBar;
	ControlHandle	hScrollBar = (*hMacWin)->hScrollBar;


	/*...Create a new picture...*/

	GetPort (&(*hMacWin)->savePort);
		SetPort ((GrafPtr) pictWin);
		ClipRect (pRect);
		(*hMacWin)->hPicture	 = OpenPicture (pRect);
		(*hMacWin)->fOpenPicture = TRUE;
	SetPort ((*hMacWin)->savePort);

	RESET_SCROLLBAR_ORIGINS ();
}


	void
macClearPicture (gx_device *dev)

{
	MacWinH	hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


	if ((*hMacWin)->hPicture /* != (PicHandle) NULL */)
	{
		/*...Out with the old...*/

		KillPicture ((*hMacWin)->hPicture);
	
		/*...In with the new...*/
	
		macCreatePicture (dev);
	}
}


	static void
macCopyPicture (gx_device *dev)

{
	PicHandle	hPicture;


	ZeroScrap ();

	if ((hPicture = macGetPicture (dev)) /* != (PicHandle) NULL */)
	{
		HLock ((Handle) hPicture);
		PutScrap (GetHandleSize ((Handle) hPicture), (ResType) 'PICT', (Ptr) *hPicture);
		HUnlock ((Handle) hPicture);
	}
}


	void
macClosePicture (gx_device *dev)

{
	MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	CWindowPtr	pictWin = (*hMacWin)->pictWin;


	if ((*hMacWin)->fOpenPicture &&
		(*hMacWin)->hPicture /* != (PicHandle) NULL */)
	{
		GetPort (&(*hMacWin)->savePort);
			SetPort ((GrafPtr) pictWin);
			ClosePicture ();
			(*hMacWin)->fOpenPicture = FALSE;
		SetPort ((*hMacWin)->savePort);
	}

	if (openFileSavePort /* != (GrafPtr) NULL */)
		SetPort (openFileSavePort);
}


	PicHandle
macGetPicture (register gx_device *dev)

{
	MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


	return (*hMacWin)->hPicture;
}


	Boolean
macSavePicture (register gx_device *dev, PicHandle hPicture)

{
	FILE	   *fd	  = (FILE *) NULL;
	char		string[256];
	short		i;
	char		aChar;
	MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	Boolean		retVal	= FALSE;


	/*...Create a new output file named "Picture<nn>"...*/

	for (i = 0; i < 999; i++)
	{
		(void) sprintf (string, "%s%d", gSFFName, i);

		if ((fd = fopen (string, "r")) /* != (FILE *) NULL */)
		{
			(void) fclose (fd);
			fd = (FILE *) NULL;

			continue;
		}
		else
		{
			SFReply		aReply;
			Point		where = {50, 50};
			Str255		pPrompt;
			Str255		pName;
			Rect	   *pRect = &(((CWindowPeek) xdev->windowPtr)->port.portRect);
			StringPtr	PfromCStr (StringPtr pPstr, char *pCst);


			GetIndString (pPrompt, MACSTRS_RES_ID, iSFPutPromptStr);

			SFPutFile (where,
					   pPrompt, PfromCStr (pName, string),
					   (ProcPtr) NULL, &aReply);

			if (aReply.good)
			{
				SetVol ((StringPtr) NULL, aReply.vRefNum);

				(void) CfromPStr (string, (char *) &(aReply.fName));

				fd = fopen (string, "wb");
			}

			/*...Refresh the screen...*/

			macBeginDraw (dev);
				DrawPicture ((*hMacWin)->hPicture, &((*hMacWin)->pictRect));
			macEndDraw (dev);

			break;
		}
	}

	if (fd /* != (FILE *) NULL */)
	{
		long	  **hLong	 = (long **) NewHandle ((Size) 512);
		short		refnum	 = fd->refnum;
		void		SetFileTypeCreator (char *fName,
										OSType fileType, OSType creator);


		if (hLong /* != (Handle) NULL */)
		{
			long   *pLong;


			/*...Write a 512 byte picture file header...*/

			for (i = 512 / sizeof (long), pLong = *hLong;
				 i /* > 0 */;
				 i--, pLong++)
			{
				*pLong = 0L;
			}

			HLock ((Handle) hLong);

			(void) fwrite (*hLong, GetHandleSize ((Handle) hLong), 1, fd);

			DisposHandle ((Handle) hLong);

			/*...Write the picture...*/

			HLock ((Handle) hPicture);

			(void) fwrite ((char *) *hPicture,
						   GetHandleSize ((Handle) hPicture), 1, fd);

			HUnlock ((Handle) hPicture);
		}

		fclose (fd);

		SetFileTypeCreator (string, (OSType) 'PICT', (*hMacWin)->fCreator);

		retVal = TRUE;
	}

	return retVal;
}


	short
macCreateWindow (gx_device *dev, short pictWidth, short pictHeight)

{
	short		retVal		= 0;


	if (xdev->windowPtr == (CWindowPtr) NULL)
	{
	    Rect			windowRect;
	    WindowPtr		SavePort;
		Rect 		   *pRect;
		Str255			string;
		short			width		= pictWidth;
		short			height		= pictHeight;
		MacPrefsH		hPrefs		= (MacPrefsH) GetResource (MACPREFS_RES_TYPE,
															   MACPREFS_RES_ID);
		ResType			fCreator	= MACPREFS_DEFAULT_FCREATOR;
		short			delScroll	= MACPREFS_DEFAULT_DELSCROLL;
		short			gWidth;
		short			gHeight;
		short			screenDepth;
		short			gp_VMError (char *msg, short index);


		if (gHasColorQD)
		{
			GDHandle		hGD	   = GetMainDevice ();
			PixMapHandle	gdPMap = (*hGD)->gdPMap;


			gWidth 	= (*hGD)->gdRect.right  - (*hGD)->gdRect.left;
			gHeight	= (*hGD)->gdRect.bottom - (*hGD)->gdRect.top;

			screenDepth = (*gdPMap)->pixelSize;
		}
		else
		{
			Rect   *pScreenRect = &(*GrayRgn)->rgnBBox;


			gWidth 	= pScreenRect->right  - pScreenRect->left;
			gHeight	= pScreenRect->bottom - pScreenRect->top + 20;

			screenDepth = 1;
		}

		if (screenDepth == 1)
		{
			gx_device_color_info   *pColorInfo = &dev->color_info;


			pColorInfo->num_components = 1;
			pColorInfo->depth		   = 1;
			pColorInfo->max_gray	   = 1;
			pColorInfo->max_rgb		   = 0;
			pColorInfo->dither_gray	   = 2;
			pColorInfo->dither_rgb	   = 0;
		}

		/*...Get favorite values from the preferences resource...*/

		if (hPrefs /* != (MacPrefsH) NULL */)
		{
			char	fFlags = (*hPrefs)->flags;


			if (fFlags & iUseWinRes)
			{
				width  = (*hPrefs)->winWidth;
				height = (*hPrefs)->winHeight;
			}
			else
			{
				width  = gWidth;
				height = gHeight;
			}

			if (fFlags & iUsePageRes)
			{
				pictWidth  = (*hPrefs)->pageWidth;
				pictHeight = (*hPrefs)->pageHeight;
			}

			fCreator  = (*hPrefs)->fCreator;
			delScroll = (*hPrefs)->delScroll;

			ReleaseResource ((Handle) hPrefs);
		}

		xdev->width		= pictWidth;
		xdev->height	= pictHeight;

		/*...Create a color graphics window...*/

		GetIndString (string, MACSTRS_RES_ID, iGraphicsWinTitleStr);

		SetRect (&windowRect, 3, 40, width, height);

		GetPort (&SavePort);

		if (gHasColorQD)
			xdev->windowPtr = (CWindowPtr) NewCWindow (
											   (Ptr) NULL, &windowRect,
											   string,
											   TRUE, zoomDocProc, (WindowPtr) NULL,
											   FALSE, 0L);
		else
			xdev->windowPtr = (CWindowPtr) NewWindow (
											   (Ptr) NULL, &windowRect,
											   string,
											   TRUE, zoomDocProc, (WindowPtr) NULL,
											   FALSE, 0L);

		if (xdev->windowPtr /* != (CWindowPtr) NULL */)
		{
			MacWinH		hMacWin   = (MacWinH) NewHandle ((Size) sizeof (MacWinRec));
			Rect	   *pRect	  = &(*hMacWin)->pictRect;
			Boolean		fGrowPict = FALSE;


			if (hMacWin == (MacWinH) NULL)
			{
				DisposeWindow ((WindowPtr) xdev->windowPtr);
				xdev->windowPtr = (CWindowPtr) NULL;
				SetPort (SavePort);

				gp_VMError ("graphics window", iWindowCreationFailed);
			}

			/*...Stow away the keys to the kingdom...*/

			SetWRefCon ((WindowPtr) xdev->windowPtr, (long) hMacWin);

			SetPort ((GrafPtr) xdev->windowPtr);

			BackPat	  (white);			/*	Hmm.  Is A5 still good?	*/
			BackColor (whiteColor);

			/*...Create a clipping region that excludes the scrollbars...*/

			(*hMacWin)->clipRgn		= NewRgn ();
			if ((*hMacWin)->clipRgn == (RgnHandle) NULL)
				gp_VMError ("clip region", iClipRegionCreationFailed);

			/*...Hold on to the original clipping region...*/

			(*hMacWin)->saveClipRgn = NewRgn ();
			if ((*hMacWin)->saveClipRgn == (RgnHandle) NULL)
				gp_VMError ("save clip region", iSaveRegionCreationFailed);

			GetClip ((*hMacWin)->saveClipRgn);

			/*...Create a region to hold the areas vacated by scrolling...*/

			(*hMacWin)->updRgn = NewRgn ();
			if ((*hMacWin)->updRgn == (RgnHandle) NULL)
				gp_VMError ("update region", iUpdRegionCreationFailed);

			/*...Initialize a few state variables and preferences...*/

			(*hMacWin)->fIsDirty = FALSE;

			(*hMacWin)->fCreator   = fCreator;
			(*hMacWin)->delScroll  = delScroll;
			(*hMacWin)->hScrollBar = (ControlHandle) NULL;
			(*hMacWin)->vScrollBar = (ControlHandle) NULL;

			SetRect (pRect, 0, 0, pictWidth, pictHeight);

			/*...Create the scrollbars...*/

			(*hMacWin)->hScrollBar = NewControl ((WindowPtr) xdev->windowPtr,
												 pRect, "\p",
												 FALSE, 0, 0, 0,
												 scrollBarProc, (long) dev);
			(*hMacWin)->hValue = 0;
			(*hMacWin)->vScrollBar = NewControl ((WindowPtr) xdev->windowPtr,
												 pRect, "\p",
												 FALSE, 0, 0, 0,
												 scrollBarProc, (long) dev);
			(*hMacWin)->vValue = 0;

			/*...Position the scrollbars and calculate the view port's size...*/

			macResizeWindow (dev);

			/*...Show, but do not activate, the scrollbars...*/

			ShowControl   ((*hMacWin)->hScrollBar);
			HiliteControl ((*hMacWin)->hScrollBar, 255);
			ShowControl   ((*hMacWin)->vScrollBar);
			HiliteControl ((*hMacWin)->vScrollBar, 255);

			/*...Establish the GrowWindow () limits...*/

			height -= 40;
			width  -=  3;
			(*hMacWin)->growRect.top	= height >> 1;
			(*hMacWin)->growRect.bottom = height;
			(*hMacWin)->growRect.left	= width  >> 1;
			(*hMacWin)->growRect.right  = width;

			/*...Make the drawing at least as large as the initial output
			 *	 window's view port...*/

			if ((*hMacWin)->viewWidth - 1 > pictWidth)
			{
				pictWidth  = (*hMacWin)->viewWidth - 1;
				fGrowPict  = TRUE;
			}
			if ((*hMacWin)->viewHeight - 1 > pictHeight)
			{
				pictHeight = (*hMacWin)->viewHeight - 1;
				fGrowPict  = TRUE;
			}

			/*...If necessary, reset the scrollbar's values...*/

			if (fGrowPict)
			{
				SetRect (pRect, 0, 0, pictWidth, pictHeight);
				macResizeWindow (dev);

				xdev->width	 = pictWidth;
				xdev->height = pictHeight;
			}

			/*...Create an "offscreen" window used to capture the rendered picture...*/

			SetRect (&windowRect, 0, gHeight, pictWidth, gHeight + pictHeight);

			if (gHasColorQD)
				(*hMacWin)->pictWin = (CWindowPtr) NewCWindow (
													   (Ptr) NULL, &windowRect,
													   "\p",
													   FALSE, plainDBox, (WindowPtr) NULL,
													   FALSE, 0L);
			else
				(*hMacWin)->pictWin = (CWindowPtr) NewWindow (
													   (Ptr) NULL, &windowRect,
													   "\p",
													   FALSE, plainDBox, (WindowPtr) NULL,
													   FALSE, 0L);
			if ((*hMacWin)->pictWin == (CWindowPtr) NULL)
				gp_VMError ("picture window", iPictWinCreationFailed);

			SetPort ((GrafPtr) (*hMacWin)->pictWin);

			BackPat	  (white);			/*	Hmm.  Is A5 still good?	*/
			BackColor (whiteColor);

			/*...Finally, create a picture for backing store...*/

			macCreatePicture (dev);

			/*...Install the window driver...*/

			(*hMacWin)->graphicsRefNum = macOpenGUIDriver (gs_mac_device.dname);
			((CWindowPeek) xdev->windowPtr)->windowKind = (*hMacWin)->graphicsRefNum;
		}
		else
		{
			retVal = gs_error_VMerror;
		}

		SetPort (SavePort);
	}

	if (retVal /* != 0 */)
		return_error (retVal);
	else
		return retVal;
}


	static void
macClearWindow (gx_device *dev)

{
	(void) (*dev->procs->fill_rectangle) (dev,
										  0, 0, xdev->width, xdev->height,
										  mac_white_color_index);
//	gDoCheckInterrupts = FALSE;
	macForceCursorInit ();
	InitCursor ();
}


	short
macCloseWindow (gx_device *dev)

{
	short	retVal = gs_error_rangecheck;


	EnableItem  (hGUIMenu, iOpenFile);
	EnableItem  (hGUIMenu, iLaserPrep);

	if (xdev->windowPtr /* != (CWindowPtr) NULL */)
	{
		MacWinH	hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


		if ((*hMacWin)->hPicture /* != (PicHandle) NULL */)
			KillPicture ((*hMacWin)->hPicture);

		if ((*hMacWin)->clipRgn /* != (RgnHandle) NULL */)
			DisposeRgn ((*hMacWin)->clipRgn);

		if ((*hMacWin)->updRgn /* != (RgnHandle) NULL */)
			DisposeRgn ((*hMacWin)->updRgn);

		if ((*hMacWin)->pictWin /* != (CWindowPtr) NULL */)
			DisposeWindow ((WindowPtr) (*hMacWin)->pictWin);

		DisposHandle ((Handle) hMacWin);

		DisposeWindow ((WindowPtr) xdev->windowPtr);
		xdev->windowPtr = (CWindowPtr) NULL;

		retVal = 0;
	}

	if (retVal /* != 0 */)
		return_error (retVal);
	else
		return retVal;
}


/*
 *	Invalidates the scrollbar and drag icon areas.
 */
 
	static void
macInvalWindow (gx_device *dev, Boolean fResetOrigin)

{
	CWindowPtr		pCWin	   = xdev->windowPtr;
	Rect			invalRect  = pCWin->portRect;
	MacWinH			hMacWin	   = (MacWinH) GetWRefCon ((WindowPtr) pCWin);
	ControlHandle	vScrollBar = (*hMacWin)->vScrollBar;
	ControlHandle	hScrollBar = (*hMacWin)->hScrollBar;


	GetPort (&(*hMacWin)->savePort);
	SetPort ((GrafPtr) pCWin);
		HideControl (hScrollBar);
		HideControl (vScrollBar);

		if (!fResetOrigin)
		{
			invalRect.left = invalRect.right  - 15;
			invalRect.top  = invalRect.bottom - 15;
			EraseRect (&invalRect);
		}
		InvalRect (&invalRect);
		EraseRect (&invalRect);
	SetPort ((*hMacWin)->savePort);

	if (fResetOrigin)
	{
		RESET_SCROLLBAR_ORIGINS ();
	}
}


/*
 *	Redraws the grow icon and scrollbars.
 */

	static void
macRedrawWindow (gx_device *dev)

{
	MacWinH			hMacWin	   = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	ControlHandle	vScrollBar = (*hMacWin)->vScrollBar;
	ControlHandle	hScrollBar = (*hMacWin)->hScrollBar;


	macResizeWindow (dev);

	GetPort (&(*hMacWin)->savePort);
	SetPort ((GrafPtr) xdev->windowPtr);
		DrawGrowIcon ((WindowPtr) xdev->windowPtr);
		ShowControl (vScrollBar);
		ShowControl (hScrollBar);
	SetPort ((*hMacWin)->savePort);
}


	static void
macResizeWindow (gx_device *dev)

{
	CWindowPtr		pCWin	   = xdev->windowPtr;
	Rect			portRect   = pCWin->portRect;
	MacWinH			hMacWin	   = (MacWinH) GetWRefCon ((WindowPtr) pCWin);
	ControlHandle	vScrollBar = (*hMacWin)->vScrollBar;
	ControlHandle	hScrollBar = (*hMacWin)->hScrollBar;
	short			wide;
	short			high;
	short			newCtlMax;


	/*...Reposition and resize the scrollbars...*/

	MoveControl (vScrollBar, portRect.right - 15, portRect.top - 1);
	wide = 16;
	high = (portRect.bottom - portRect.top) - 14;
	SizeControl (vScrollBar, wide, high);

	MoveControl (hScrollBar, portRect.left - 1, portRect.bottom - 15);
	wide = (portRect.right - portRect.left) - 14;
	high = 16;
	SizeControl (hScrollBar, wide, high);

	/*...Recalculate the view port's rectangle...*/

	portRect.right  -= 15;			/*	adjustment for scrollbars	*/
	portRect.bottom -= 15;
	(*hMacWin)->viewRect = portRect;

	(*hMacWin)->viewWidth  = portRect.right	 - portRect.left + 1;
	(*hMacWin)->viewHeight = portRect.bottom - portRect.top	 + 1;

	/*...Reset the scrollbars' maxima...*/

	newCtlMax = (*hMacWin)->pictRect.right - (*hMacWin)->viewWidth;
	SetCtlMax ((*hMacWin)->hScrollBar, (newCtlMax > 0) ? newCtlMax : 0);

	newCtlMax = (*hMacWin)->pictRect.bottom - (*hMacWin)->viewHeight;
	SetCtlMax ((*hMacWin)->vScrollBar, (newCtlMax > 0) ? newCtlMax : 0);
}


	void
macBeginDraw (gx_device *dev)

{
	MacWinH	hMacWin  = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	Rect	clipRect = (*hMacWin)->viewRect;
	Point	origin	 = (*hMacWin)->origin;


	GetPort (&(*hMacWin)->savePort);
	SetPort ((GrafPtr) xdev->windowPtr);
		OffsetRect (&clipRect, origin.h, origin.v);
		RectRgn ((*hMacWin)->clipRgn, &clipRect);
		SetClip ((*hMacWin)->clipRgn);

		SetOrigin (origin.h, origin.v);
}


	void
macEndDraw (gx_device *dev)

{
	MacWinH	hMacWin  = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


		SetOrigin (0, 0);

		SetClip ((*hMacWin)->saveClipRgn);
	SetPort ((*hMacWin)->savePort);

	if (openFileSavePort /* != (GrafPtr) NULL */)
		SetPort (openFileSavePort);
}


	void
macBeginPictDraw (gx_device *dev)

{
	MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	CWindowPtr	pictWin = (*hMacWin)->pictWin;
	PicHandle	hPicture;


	if (pictWin /* != (CWindowPtr) NULL */ &&
		(hPicture = (*hMacWin)->hPicture) /* != (PicHandle) NULL */)
	{
		(*hMacWin)->pictLen = GetHandleSize ((Handle) hPicture);

		GetPort (&(*hMacWin)->savePort);
		SetPort ((GrafPtr) pictWin);
	}
}


#define	MINIMUM_PICT_LEN	10L


	short
macEndPictDraw (gx_device *dev)

{
	MacWinH		hMacWin  = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);
	PicHandle	hPicture = (*hMacWin)->hPicture;
	short		retVal	= 0;


		SetPort ((*hMacWin)->savePort);

		if (openFileSavePort /* != (GrafPtr) NULL */)
			SetPort (openFileSavePort);

	/*...See if we ran out of memory rendering the picture...*/

	if (hPicture /* != (PicHandle) NULL */)
	{
		Size	pictLen = GetHandleSize ((Handle) hPicture);


		if ((*hMacWin)->pictLen >= pictLen && pictLen == MINIMUM_PICT_LEN)
		{
#if 0
			macInitCursor ();
			Alert (MACVMERROR, (ProcPtr) NULL);
#endif
			retVal = gs_error_VMerror;
		}
	}

	return retVal;
}


	CWindowPtr
macPictWindow (gx_device *dev)

{
	MacWinH		hMacWin = (MacWinH) GetWRefCon ((WindowPtr) xdev->windowPtr);


	return (*hMacWin)->pictWin;
}


	static StringPtr
PfromCStr (StringPtr pPstr, char *pCstr)

{
    short		lenstr = strlen (pCstr);


	pPstr[0] = lenstr;

	if (lenstr /* > 0 */)
		BlockMove ((char *) pCstr, &(pPstr[1]), lenstr);

	return pPstr;
}


	static void
SetFileTypeCreator (char *fName, OSType fdType, OSType fdCreator)

{
	OSErr		status = noErr;
    Str255		pName;
    FInfo   	fndrInfo;
    FileParam	pBlock;
    short		oldVRefNum;
	StringPtr	PfromCStr (StringPtr pPstr, char *pCstr);


	if (GetVol (pName, &oldVRefNum) /* != noErr */)
		return;

	/*...Obtain the name for the open output file using its refnum...*/

	pBlock.ioNamePtr   = PfromCStr (pName, fName);
	pBlock.ioFVersNum  = 0;
	pBlock.ioVRefNum   = oldVRefNum;
	pBlock.ioFDirIndex = 0;

	if (PBGetFInfo ((ParmBlkPtr) &pBlock, FALSE) /* != noErr */)
		return;

    /*...Set file's type and creator...*/

    if (GetFInfo (pName, oldVRefNum, &fndrInfo) == noErr)
    {
        fndrInfo.fdType    = fdType;
        fndrInfo.fdCreator = fdCreator;

        (void) SetFInfo (pName, oldVRefNum, &fndrInfo);
    }
}


static int	(*pConsolePBControl) (void);
static int	gsConsolePBControl (void);


	static void
swapConsoleControl (Boolean fSaveConsoleControl)

{
	short				consoleRefNum = ~((WindowPeek) gConsoleWinPtr)->windowKind;
	DCtlHandle			dceH 		  = UTableBase[consoleRefNum];
	DrvrH				hConsoleDrvr  = (DrvrH) (*dceH)->dCtlDriver;


	if (fSaveConsoleControl)
	{
		pConsolePBControl			= (*hConsoleDrvr)->vCtl.vCode;
		(*hConsoleDrvr)->vCtl.vCode = gsConsolePBControl;
	}
	else
	{
		if (pConsolePBControl /* != (int (*) (void)) NULL */)
		{
			(*hConsoleDrvr)->vCtl.vCode = pConsolePBControl;
			pConsolePBControl			= (int (*) (void)) NULL;
		}
	}
}


	static int
gsConsolePBControl (void)

{
	register CntrlParam	   *pb;
	DCtlPtr					dce;
	long					oldA5  = SetCurrentA5 ();
	int						retVal = 0;


	asm
	{
		move.l	a0,pb
		move.l	a1,dce
	}

	switch (pb->csCode)
	{
		case accCursor:
		/*	do nothing	*/
			break;

		default:
		{
			asm
			{
				move.l	pb,a0
				move.l	dce,a1
				move.l	gsConsolePBControl,a2	;	retVal = (*gsConsolePBControl) ();
				jsr		(a2)
				move.w	d0,retVal
			}

			break;
		}
	}

	HUnlock (RecoverHandleSys ((Ptr) dce));
	SetA5 (oldA5);

	return retVal;
}


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

#include <stdio.h>
#include <stddef.h>
#include "gx.h"
#include "gxdevice.h"
#include "gp_mac.h"
#include "gmacres.h"
#include "gp_macui.h"


enum
{
	iOKOutline = Cancel + 1,
	iPicture,

	__iLastDialogItem__
};


static PicHandle	hPicture;		//	used by drawPreview
static Rect			pictBounds;		//	used by MyDlgFilter
static Rect			selRect;		//	used by MyDlgFilter
static Boolean		fHaveSelRect;	//	used by MyDlgFilter


extern PicHandle	macGetPicture			(gx_device *dev);
Boolean				macSavePictureSelection	(gx_device *dev);
void				macCopyPictureSelection (gx_device *dev);

static Boolean		DoGetPictureSelection (gx_device *dev, Rect *pRect, Boolean *pFIsAll);
static PicHandle	DoMakeClippedPicture  (gx_device *dev, Rect *pRect);


	Boolean
macSavePictureSelection (gx_device *dev)

{
	Rect		rect;
	Boolean		fIsAll;
	Boolean		retVal;


	if (retVal = DoGetPictureSelection (dev, &rect, &fIsAll))
	{
		PicHandle	hPicture;


		if (fIsAll)
			hPicture = macGetPicture (dev);
		else
			hPicture = DoMakeClippedPicture (dev, &rect);

		retVal = (hPicture != (PicHandle) NULL) && macSavePicture (dev, hPicture);

		if (!fIsAll && hPicture /* != (PicHandle) NULL */)
			KillPicture (hPicture);
	}

	return retVal;
}


	void
macCopyPictureSelection (gx_device *dev)

{
	Rect		rect;
	Boolean		fIsAll;


	if (DoGetPictureSelection (dev, &rect, &fIsAll))
	{
		PicHandle	hPicture;


		if (fIsAll)
			hPicture = macGetPicture (dev);
		else
			hPicture = DoMakeClippedPicture (dev, &rect);

		if (hPicture /* != (PicHandle) NULL */)
		{
			ZeroScrap ();

			HLock ((Handle) hPicture);
			PutScrap (GetHandleSize ((Handle) hPicture), (ResType) 'PICT',
					  (Ptr) *hPicture);
			HUnlock ((Handle) hPicture);

			if (!fIsAll)
				KillPicture (hPicture);
		}
	}
}


	static Boolean
DoGetPictureSelection (gx_device *dev, Rect *pRect, Boolean *pFIsAll)

{
	DialogPtr		dialog;
	Boolean			retVal = FALSE;
	pascal Boolean	MyDlgFilter (DialogPtr theDlg, EventRecord *pEvent, short *itemHit);


	if ((hPicture = macGetPicture (dev)) == (PicHandle) NULL ||
		pRect == (Rect *) NULL ||
		pFIsAll == (Boolean *) NULL)
	{
		return retVal;
	}

	dialog = GetNewDialog (MACSAVESELECTIONDLOG, (char *) NULL, (WindowPtr) -1);

	if (dialog /* != (DialogPtr) NULL */)
	{
		short		item;
		short		iType;
		Handle		hItem;
		Rect		rect;
		Rect		pictRect;
		long		widthP;
		long		heightP;


		fHaveSelRect = FALSE;

		/*...Prep outline field...*/

		GetDItem (dialog, iOKOutline, &iType, &hItem, &rect);

		if (iType == userItem | itemDisable)
			SetDItem (dialog, iOKOutline, iType, (Handle) drawOutline, &rect);

		/*...Determine the picture's geometry...*/

		rect	= (*hPicture)->picFrame;
		widthP	= rect.right  - rect.left;
		heightP	= rect.bottom - rect.top;

		/*...Prep the preview...*/

		GetDItem (dialog, iPicture, &iType, &hItem, &pictRect);

		if (iType == userItem | itemDisable)
		{
			long		widthU	= pictRect.right  - pictRect.left;
			long		heightU = pictRect.bottom - pictRect.top;
			long		wPhU	= widthP * heightU;
			long		wUhP	= widthU * heightP;
			pascal void	drawPreview (WindowPtr dialog, short item);


			/*...Make the preview image have the same aspect ratio as the picture...*/

			if (wPhU != wUhP)
			{
				long	center;
				long	half;


				if (wPhU > wUhP)
				{
					center = pictRect.top + (heightU >> 1);
					half   = ((heightP * widthU) / widthP) >> 1;

					pictRect.top	= center - half;
					pictRect.bottom = center + half;
				}
				else
				{
					center = pictRect.left + (widthU >> 1);
					half   = ((heightU * widthP) / heightP) >> 1;

					pictRect.left  = center - half;
					pictRect.right = center + half;
				}
			}

			SetDItem (dialog, iPicture, iType, (Handle) drawPreview, &pictRect);
		}

		/*...Accommodate the one-pixel border...*/

		pictBounds = pictRect;
		InsetRect (&pictBounds, 1, 1);

		/*...Run the modal dialog...*/

		ShowWindow (dialog);

		ModalDialog ((ProcPtr) MyDlgFilter, &item);

		DisposDialog (dialog);

		/*...Calculate the new picture viewport...*/

		if (item == OK)
		{
			if (fHaveSelRect && !EmptyRect (&selRect))
			{
				short	widthU	= pictBounds.right  - pictBounds.left;
				short	heightU = pictBounds.bottom - pictBounds.top;
				short	xOrigin	= rect.left;
				short	yOrigin = rect.top;


				rect.left	= xOrigin +
							  (selRect.left   - pictBounds.left) * widthP  / widthU;
				rect.right	= xOrigin +
							  (selRect.right  - pictBounds.left) * widthP  / widthU;
				rect.top	= yOrigin +
							  (selRect.top	  - pictBounds.top ) * heightP / heightU;
				rect.bottom	= yOrigin +
							  (selRect.bottom - pictBounds.top ) * heightP / heightU;

				*pFIsAll = FALSE;
			}
			else
			{
				rect	 = (*hPicture)->picFrame;
				*pFIsAll = TRUE;
			}

			*pRect = rect;

			retVal = TRUE;
		}
	}

	return retVal;
}


	static pascal void
drawPreview (WindowPtr dialog, short item)

{
	short	iType;
	Handle	hItem;
	Rect	rect;
	GrafPtr	savePort;


	GetPort (&savePort);
	SetPort ((GrafPtr) dialog);
		GetDItem (dialog, item, &iType, &hItem, &rect);
		PenSize (1, 1);
		FrameRect (&rect);
		DrawPicture (hPicture, &pictBounds);
	SetPort (savePort);
}


	static pascal Boolean
MyDlgFilter (DialogPtr pDialog, EventRecord *pEvent, short *itemHit)

{
	Boolean	retVal;


	if (pEvent->what == mouseDown)
	{
		Point	origin;
		GrafPtr	savePort;


		GetPort (&savePort);
		SetPort ((GrafPtr) pDialog);

		GetMouse (&origin);

		if (PtInRect (origin, &pictBounds))
		{
			Point	lastPoint = origin;


			PenSize (1, 1);
			PenMode (notPatXor);
			PenPat (black);

			if (fHaveSelRect)
				FrameRect (&selRect);				//	erase

			SetRect (&selRect, origin.h, origin.v, origin.h, origin.v);
			FrameRect (&selRect);

			fHaveSelRect = TRUE;

			/*...Rubberband a rectangular region...*/

			while (StillDown ())
			{
				Point	aPoint;
				short	top;
				short	left;
				short	bottom;
				short	right;


				SystemTask ();

				GetMouse (&aPoint);

				if		(pictBounds.left > aPoint.h)
					aPoint.h = pictBounds.left;
				else if (pictBounds.right < aPoint.h)
					aPoint.h = pictBounds.right;

				if		(pictBounds.top > aPoint.v)
					aPoint.v = pictBounds.top;
				else if (pictBounds.bottom < aPoint.v)
					aPoint.v = pictBounds.bottom;

				if (EqualPt (aPoint, lastPoint))
					continue;

				lastPoint = aPoint;

				FrameRect (&selRect);		//	erase

				top		= (origin.v <= aPoint.v) ? origin.v : aPoint.v;
				left	= (origin.h <= aPoint.h) ? origin.h : aPoint.h;
				bottom	= (origin.v >= aPoint.v) ? origin.v : aPoint.v;
				right	= (origin.h >= aPoint.h) ? origin.h : aPoint.h;

				SetRect (&selRect, left, top, right, bottom);

				FrameRect (&selRect);		//	draw
			}

			PenMode (patCopy);
		}

		SetPort (savePort);
	}

	else if (pEvent->what == keyDown)
	{
		*itemHit = 0;

		switch ((pEvent->message) & charCodeMask)
		{
			case 0x0D:	// Return pressed or ...
			case 0x03:	// ... Enter pressed
			{
				*itemHit = OK;
				retVal	 = TRUE;
				break;
			}

			case '.':
			{
				short	modifiers = pEvent->modifiers & 0xFF00;


				if (modifiers == cmdKey)
				{
					*itemHit = Cancel;	// Command-. pressed
					retVal	 = TRUE;
				}
				break;
			}

			case 0x1B:
			{
				*itemHit = Cancel;	// Esc pressed
				retVal	 = TRUE;
				break;
			}
		}

		if (*itemHit /* != 0 */)
		{
			short		item;
			short		iType;
			Handle		hItem;
			Rect		rect;


			GetDItem (pDialog, *itemHit, &iType, &hItem, &rect);

			if (iType == ctrlItem | btnCtrl)
			{
				long	targetTick = Ticks + 10L;


				HiliteControl ((ControlHandle) hItem, inButton);

				while (Ticks <= targetTick)		//	wait for 1/6 of a second
				{}

				HiliteControl ((ControlHandle) hItem, inButton);
			}
		}
	}

	return retVal;
}


	static PicHandle
DoMakeClippedPicture (gx_device *dev, Rect *pRect)

{
	PicHandle	hPicture   = macGetPicture (dev);
	Rect		windowRect;
	short		pictWidth  = pRect->right  - pRect->left;
	short		pictHeight = pRect->bottom - pRect->top;
	CWindowPtr	pictWin;
	PicHandle	hNewPict   = (PicHandle) NULL;
	GrafPtr		savePort;
	short		gp_VMError (char *msg, short index);


	SetRect (&windowRect, 0, 0, pictWidth, pictHeight);

	if (gHasColorQD)
		pictWin = (CWindowPtr) NewCWindow ((Ptr) NULL, &windowRect,
										   "\p",
										   FALSE, plainDBox, (WindowPtr) NULL,
										   FALSE, 0L);
	else
		pictWin = (CWindowPtr) NewWindow ((Ptr) NULL, &windowRect,
										   "\p",
										   FALSE, plainDBox, (WindowPtr) NULL,
										   FALSE, 0L);

	if (pictWin == (CWindowPtr) NULL)
		gp_VMError ("picture window", iPictWinCreationFailed);

	GetPort (&savePort);
		SetPort ((GrafPtr) pictWin);
		ClipRect (pRect);
		if ((hNewPict = OpenPicture (&windowRect)) /* != (PicHandle) NULL */)
		{
			SetOrigin (pRect->left, pRect->top);
			DrawPicture (hPicture, &(*hPicture)->picFrame);
			ClosePicture ();
		}
		DisposeWindow ((WindowPtr) pictWin);
	SetPort (savePort);

	return hNewPict;
}

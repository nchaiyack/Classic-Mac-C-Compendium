// *****************************************************************************
//
//	FILE:
//		InfinityWindoid.c
//
//	WRITTEN BY:
//		Troy Gaul
//		Infinity Systems
//
//		� 1991-94 Infinity Systems
//		All rights reserved.
//
//	DESCRIPTION:
//		This file contains the main source for a WDEF (Window Definition)
//		resource. It provides a 'windoid' appearance use on floating windows. 
//				
//		See the file 'About Infinity Windoid' for more information and a list
//		of features this WDEF supports.
//
//	HOW TO CONTACT THE AUTHOR:
//		Send e-mail to: t-gaul@i-link.com
//
// -----------------------------------------------------------------------------

// *****************************************************************************
//	Include files
// -----------------------------------------------------------------------------

#include "WindoidDefines.h"			// must be included before Apple interfaces

#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <Windows.h>
#include <Palettes.h>
#include <ToolUtils.h>
#include <Desk.h>
#include <Fonts.h>
#ifdef UNIV_HEADERS
#include <MixedMode.h>
#endif

#include "WindoidTypes.h"
#include "WindoidUtil.h"

// *****************************************************************************
//	Function Prototypes	for main							 
// -----------------------------------------------------------------------------

void DoWInit(WindowPeek window, long param, short varCode);
void DoWDispose(WindowPeek window, long param);
long DoWHit(WindowPeek window, long param);
void DoWDraw(WindowPeek window, long param);
void DoWCalcRgns(WindowPeek window, long param);
void DoWGrow(WindowPeek window, long param);
void DoWDrawGIcon(WindowPeek window, long param);

////// *************************************************************************
/////
////	Windoid Main Function													 
///
// -----------------------------------------------------------------------------
//	This is the main entry point for all calls to this code resource. It
//	dispatches to routines that correspond to the message it is given.
// -----------------------------------------------------------------------------

pascal long 
main(short varCode, WindowPeek window, short message, long param) {
	GrafPtr		savePort;
	long		result;
	Boolean		needSyncPorts;
	
	// This sets up the appropriate drawing environment, but only for those
	// messages for which we actually need to draw.

	needSyncPorts = (message == wDraw 
				  || message == wHit 
				  || message == wGrow
				  || message == wDrawGIcon) && HasCQDraw();
	if (needSyncPorts) {
		GetPort(&savePort);
		SyncPorts();
	}
	
	switch (message) {
		case wNew:			DoWInit(window, param, varCode);
							break;
		
		case wDispose:		DoWDispose(window, param);
							break;

		case wDraw:			DoWDraw(window, param & 0xFFFF);
							break;
			// There's a tech note that says that for the draw message, only
			// the low-order word of param is set correctly, so we should do
			// this (AND with 0xFFFF) to be sure we're looking at the correct 
			// value.
			
		case wHit:			result = DoWHit(window, param);		
							break;
		
		case wCalcRgns:		DoWCalcRgns(window, param);
							break;
		
		case wGrow:			DoWGrow(window, param);
							break;
							
		case wDrawGIcon:	DoWDrawGIcon(window, param);
							break;
	}
	
	if (needSyncPorts)
		SetPort(savePort);
	
	return result;
}

////// *************************************************************************
/////
////	Routines to get Rects for title bar parts													 
///
// -----------------------------------------------------------------------------

static void 
GetTitleBar(WindowPeek window, Rect *titleBar) {
	*titleBar = (**(window->strucRgn)).rgnBBox;

	if (WindData.isHoriz) {	
	
		// title bar on top
		
		titleBar->bottom = titleBar->top + kTitleHeight;
		titleBar->right -= 1;		// shadow compensation
	} else {			
	
		// title bar on left
		
		titleBar->right = titleBar->left + kTitleHeight;
		titleBar->bottom -= 1; 		// shadow compensation
	}
}

// -----------------------------------------------------------------------------

static void 
GetCloseBox(WindowPeek window, const Rect *titleRect, Rect *theRect) {
	#pragma unused(window)

	*theRect = *titleRect;
	if (WindData.isHoriz)
		InsetRect(theRect, kGadgetMargin, kGadgetInset);	// titlebar on top
	else
		InsetRect(theRect, kGadgetInset, kGadgetMargin);	// titlebar on left

	theRect->bottom = theRect->top  + kGadgetSize;
	theRect->right  = theRect->left + kGadgetSize;
}

// -----------------------------------------------------------------------------
#ifdef ALLOW_ZOOM

static void 
GetZoomBox(WindowPeek window, const Rect *titleRect, Rect *theRect) {
	#pragma unused(window)

	*theRect = *titleRect;
	if (WindData.isHoriz) {
		InsetRect(theRect, kGadgetMargin, kGadgetInset);	// titlebar on top
#ifndef DONT_ALIGN_ZOOM
		if (IsEven(titleRect->right - titleRect->left))
			OffsetRect(theRect, -1, 0);
#endif
	} else {
		InsetRect(theRect, kGadgetInset, kGadgetMargin);	// titlebar on left
#ifndef DONT_ALIGN_ZOOM
		if (IsEven(titleRect->bottom - titleRect->top))
			OffsetRect(theRect, 0, -1);
#endif
	}
	
	theRect->top = theRect->bottom - kGadgetSize;
	theRect->left = theRect->right - kGadgetSize;
}

#endif
// -----------------------------------------------------------------------------
#ifdef ALLOW_GROW

static void 
GetGrowBox(WindowPeek window, Rect *theRect) {
	GetGlobalContentRect(window, theRect);

	theRect->left = ++theRect->right - kScrollBarPixels;
	theRect->top = ++theRect->bottom - kScrollBarPixels;
}

#endif

////// *************************************************************************
/////
////	Drawing routines													 
///
// -----------------------------------------------------------------------------

// *****************************************************************************
//	SetWFrameColor
// -----------------------------------------------------------------------------

static void
SetWFrameColor(WindowPeek window, short variation) {
	switch (variation) {
		case blackandwhite:
			ForeColor(blackColor);
			break;

#ifndef SYS7_OR_LATER
		case sys6color:
			WctbForeColor(window, wFrameColor);
			break;
#endif

		case sys7color:
			if (WindData.ignoreHilite || window->hilited)
				WctbForeColor(window, wFrameColor);
			else
				AvgWctbForeColor(window, wHiliteColorLight, wHiliteColorDark,
								 wInactiveFramePct);
			break;
	}
}

// *****************************************************************************
//	SetWTitleColor
// -----------------------------------------------------------------------------
#ifdef TITLE_STRING

static void
SetWTitleColor(WindowPeek window, short variation, Boolean active) {
	switch (variation) {
		case blackandwhite:
			break;

#ifndef SYS7_OR_LATER
		case sys6color:
			WctbForeColor(window, wTextColor);
			break;
#endif
			
		case sys7color:
			if (active)
				WctbForeColor(window, wTextColor);
			else		// -- set the color for inactive titlebar text
				AvgWctbForeColor(window, wHiliteColorLight, wHiliteColorDark,
								 wInactiveTextPct);
			break;
	}
}

#endif

// *****************************************************************************
//	SetWTitleBarColors
// -----------------------------------------------------------------------------

static void
SetWTitleBarColors(WindowPeek window, short variation, Boolean active) {

	//	Set the foreground and background for the drawing of the 
	//	titlebar pattern
	
	switch (variation) {
		case blackandwhite:
			ColorsNormal();
			break;
			
#ifndef SYS7_OR_LATER

		case sys6color:
			WctbForeColor(window, wHiliteColor);
			if (active)
				WctbBackColor(window, wTitleBarColor);
			else
				WctbBackColor(window, wContentColor);
			break;

#endif

		case sys7color:
			if (active) {
				AvgWctbForeColor(window, wHiliteColorLight, wHiliteColorDark, 
								 wTitleBarDarkPct);
				AvgWctbBackColor(window, wHiliteColorLight, wHiliteColorDark, 
								 wTitleBarLightPct);
			} else {
				WctbForeColor(window, wContentColor);
				WctbBackColor(window, wContentColor);
			}
			break;
	}
}

// *****************************************************************************
//	SetGadgetFrameEraseColors
// -----------------------------------------------------------------------------

static void
SetGadgetFrameEraseColors(WindowPeek window, short variation) {

	//	Set the foreground and background for the drawing of the 
	//	titlebar pattern, in inverse so we can erase some of the
	//	background by using normal drawing routines

	switch (variation) {
		case blackandwhite:
			ForeColor(whiteColor);
			BackColor(blackColor);
			break;
			
#ifndef SYS7_OR_LATER
		case sys6color:
			WctbForeColor(window, wTitleBarColor);
			WctbBackColor(window, wHiliteColor);
			break;
#endif

		case sys7color:
			AvgWctbBackColor(window, wHiliteColorLight, wHiliteColorDark, 
							 wTitleBarDarkPct);
			AvgWctbForeColor(window, wHiliteColorLight, wHiliteColorDark, 
							 wTitleBarLightPct);
			break;
	}
}

// *****************************************************************************
//	DrawTitlebarTinges
// -----------------------------------------------------------------------------

static void
DrawTitlebarTinges(WindowPeek window, short variation, const Rect *bounds) {
#ifdef THICK_TITLEBAR
	Rect tempRect = *bounds;
	InsetRect(&tempRect, 1, 1);
	
	switch (variation) {
		case blackandwhite:
			ForeColor(whiteColor);
			BackColor(blackColor);
			FrameRect(&tempRect);
			break;

#ifndef SYS7_OR_LATER
		case sys6color:
			WctbForeColor(window, wTitleBarColor);
			FrameRect(&tempRect);
			break;
#endif
			
		case sys7color:
			AvgWctbForeColor(window, wTingeLight, wTingeDark, 
							 wTitleBarTingeDarkPct);
			FrameBottomRightShading(tempRect);

			WctbForeColor(window, wTingeLight);
			tempRect.right--;
			tempRect.bottom--;
			FrameTopLeftShading(tempRect);
			break;
	}
#else
	#pragma unused (window, variation, bounds)
#endif
}

// *****************************************************************************
//	DrawCloseBox
// -----------------------------------------------------------------------------

static void 
DrawCloseBox(WindowPeek window, short variation, Rect *theRect) {
	Rect tempRect;

#ifdef THICK_TITLEBAR

	//	Paint the area on the edges out with the background color

	SetGadgetFrameEraseColors(window, variation);
	InsetRect(theRect, -kTingeInset, -kTingeInset);
	FrameRect(theRect);
	InsetRect(theRect, kTingeInset, kTingeInset);
#endif

	switch (variation) {
		case blackandwhite:
			ColorsNormal();
			FrameBox(theRect);
			break;
		
#ifndef SYS7_OR_LATER
		case sys6color:
			WctbForeColor(window, wHiliteColor);
			WctbBackColor(window, wTitleBarColor);
			FrameBox(theRect);
			break;
#endif
		
		case sys7color:
			WctbForeColor(window, wTingeDark);
			BackColor(whiteColor);
			FrameTopLeftShading(*theRect);
			
			tempRect = *theRect;
			tempRect.top++;
			tempRect.left++;
			WctbForeColor(window, wTingeLight);
			FrameRect(&tempRect);

			InsetRect(&tempRect, 1, 1);
			WctbForeColor(window, wTingeDark);
			FrameBottomRightShading(tempRect);

			tempRect.right--;
			tempRect.bottom--;
			AvgWctbForeColor(window, wTitleBarLight, wTitleBarDark, 
							 wCloseBoxColor);
			PaintRect(&tempRect);
			break;
	}
}

// *****************************************************************************
//	DrawZoomBox -- Draw zoom box
// -----------------------------------------------------------------------------
#ifdef ALLOW_ZOOM

static void 
DrawZoomBox(WindowPeek window, short variation, Rect *theRect) {
	Rect tempRect;

	DrawCloseBox(window, variation, theRect);
	tempRect = *theRect;
	tempRect.bottom -= 3;
	tempRect.right -= 3;

	switch (variation) {
		case blackandwhite:
#ifndef SYS7_OR_LATER
		case sys6color:
#endif
			FrameRect(&tempRect);
			break;
		
		case sys7color:
			WctbForeColor(window, wTingeDark);
			tempRect.left += 2;
			tempRect.top += 2;
			FrameBottomRightShading(tempRect);
			break;
	}
}

#endif
// *****************************************************************************
//	DrawXedBox -- Draw close or zoom box with an X in it (or inverted in B&W)
// -----------------------------------------------------------------------------

static void 
DrawXedBox(WindowPeek window, short variation, Rect *theRect) {
	switch (variation) {
		case blackandwhite:
			PaintRect(theRect);
			break;
			
#ifndef SYS7_OR_LATER
		case sys6color:
			WctbForeColor(window, wHiliteColor);
			PaintRect(theRect);
			break;
#endif
			
		case sys7color:
			AvgWctbForeColor(window, wTingeLight, wTingeDark, wXedBoxPct);
			PaintRect(theRect);
			
			WctbForeColor(window, wTitleBarDark);
			FrameRect(theRect);
			
			MoveTo(theRect->left,      theRect->top       );	// Draw the 'X'
			LineTo(theRect->right - 1, theRect->bottom - 1);
			MoveTo(theRect->right - 1, theRect->top       );
			LineTo(theRect->left,      theRect->bottom - 1);
			break;
	}
}

// *****************************************************************************
//	DrawGrowBox
// -----------------------------------------------------------------------------
#ifdef ALLOW_GROW

static void 
DrawGrow3DBox(WindowPeek window, Rect *theRect, Boolean light) {
	Rect tempRect = *theRect;
	
	WctbForeColor(window, wTingeDark);
	FrameRect(theRect);
	
	
	//	Add the top light outer border on the top-left edge
	
	tempRect.left++;
	tempRect.top++;
	WctbForeColor(window, wTingeLight);
	FrameTopLeftShading(tempRect);


	//	Finally, fill in the center.
	
	InsetRect(&tempRect, 1, 1);
	AvgWctbForeColor(window, wTitleBarLight, wTitleBarDark, 
					 light ? wGrowBoxColorLt : wGrowBoxColorDk);
	PaintRect(&tempRect);
}

// -----------------------------------------------------------------------------

static void 
DrawGrowBox(WindowPeek window, short variation, Rect *theRect) {
	Rect smallRect; 
	Rect largeRect;
	
	if (!(WindData.ignoreHilite || window->hilited)) { 

		switch (variation) {
#ifndef SYS7_OR_LATER
			case sys6color:
#endif
			case sys7color:
				WctbForeColor(window, wFrameColor);
				WctbBackColor(window, wContentColor);
				break;
		}
		FrameBox(theRect);
	} else {
	
		//	Add the size box chevrons.

#ifndef SMALL_GROW		// normal grow box
		SetRect(&smallRect, theRect->left + 3, theRect->top + 3,
						    theRect->left + 10, theRect->top + 10);
		SetRect(&largeRect, smallRect.left + 2, smallRect.top + 2, 
						    theRect->right - 2, theRect->bottom - 2);
#else
		SetRect(&smallRect, theRect->left + 2, theRect->top + 2,
						    theRect->left + 7, theRect->top + 7);
		SetRect(&largeRect, smallRect.left + 1, smallRect.top + 1, 
						    theRect->right - 2, theRect->bottom - 2);
#endif

		switch (variation) {
			case blackandwhite:
#ifndef SYS7_OR_LATER
			case sys6color:
				if (variation == sys6color)
					WctbForeColor(window, wFrameColor);
				else
#endif
					ColorsNormal();
				FrameBox(theRect);
				
				FrameRect(&largeRect);
				FrameBox(&smallRect);
				break;
			
			case sys7color:
				WctbForeColor(window, wFrameColor);
				AvgWctbBackColor(window, wHiliteColorLight, wHiliteColorDark, 
								 wGrowBoxBackground);
				FrameBox(theRect);
				

				//	Draw the dark border parts for the bottom rectangle

#ifndef SMALL_GROW
				OffsetRect(&largeRect, -1, -1);
#endif
				DrawGrow3DBox(window, &largeRect, false);
				

				//	Draw the dark border parts for the top rectangle

#ifndef SMALL_GROW
				smallRect.right--;
				smallRect.bottom--;
#endif
				DrawGrow3DBox(window, &smallRect, true);
				break;
		}
	}
}

#endif
// -----------------------------------------------------------------------------

static void
GetTitlebarPat(Boolean active, Point *corner, Pattern *titlePat) {

	// Choose correct pattern, depending on position of window in global
	// coordinates. (Concept of new (2.3) version taken from _Macintosh 
	// Programming Secrets_, Second Edition, by Scott Knaster and Keith 
	// Rollin, page 423.)

	long seed;

	if (active)
		seed = 0x00550055;
	else
		seed = 0x00000000;

#ifdef THICK_TITLEBAR
	if (IsOdd(corner->h))
		seed <<= 1;
	if (IsOdd(corner->v))
		seed <<= 8;
#else
	if (IsEven(corner->h))
		seed <<= 1;
	if (IsEven(corner->v))
		seed <<= 8;
#endif

	*((long*) titlePat + 1) = *((long*) titlePat) = seed;
}

//*****************************************************************************
//	SubtractGadgetRect
//-----------------------------------------------------------------------------

static void
SubtractGadgetRect(RgnHandle theRgn, const Rect *theRect) {
	Rect subRect = *theRect;
	RgnHandle subRgn = NewRgn();
	
#ifdef THICK_TITLEBAR
	InsetRect(&subRect, -kTingeInset, -kTingeInset);
		// to give the correct visual appearance
#endif
	
	RectRgn(subRgn, &subRect);
	DiffRgn(theRgn, subRgn, theRgn);
	
	DisposeRgn(subRgn);
}

// *****************************************************************************
//	DrawTitleString
// -----------------------------------------------------------------------------
	// when this routine is called, the background color will be set to the
	// color of the background of the titlebar

static void
DrawTitleString(WindowPeek window, short variation, 
				const Rect *titleRect, Boolean active, 
				Rect *stringRect) {
#ifdef TITLE_STRING
	short maxWidth;
	short titleWidth;
	short inset;
	short strAreaLeft;
	Rect titleStrBounds;
	RGBColor saveFore;
	RgnHandle saveClip = NewRgn();
	RgnHandle clipRgn = NewRgn();
	
	if (window->titleHandle != nil && (*window->titleHandle)[0] != 0
		&& WindData.isHoriz) {
		maxWidth = titleRect->right - titleRect->left - 2 * kGadgetMargin;
		strAreaLeft = titleRect->left + kGadgetMargin;
		if (window->goAwayFlag || window->spareFlag) {
			maxWidth -= 2 * (kGadgetSize + kGadgetMargin);
			strAreaLeft += kGadgetSize + kGadgetMargin;
		}
		
		if (maxWidth > 0) {

			//	Set up fonts, colors for text drawing

			TextFont(kTitleFont);
			TextSize(kTitleSize);
			TextFace(kTitleStyle);
			TextMode(srcOr);
			HLock((Handle) window->titleHandle);
			
			if (variation != blackandwhite)
				GetForeColor(&saveFore);
			SetWTitleColor(window, variation, active);
			
			//	Calculate the width of the title string
			
			titleWidth = StringWidth(*window->titleHandle)
						 + 2 * kTitleMargin;
			
			
			//	Limit its size to maxWidth
			
			titleWidth = (titleWidth > maxWidth) ? maxWidth : titleWidth;
			
			
			//	Determine where to position it
			
			inset = (short) (maxWidth - titleWidth) / 2;


			//	The following is done to make the title appear centered

			if (IsEven(titleWidth))				// We need an odd width or  
				titleWidth--;					// the overlap is wrong

#ifndef THICK_TITLEBAR
			inset -= IsOdd(inset);
				// This is done so that the title doesn't shift as the window's
				// width changes.
#else
			inset -= IsEven(inset);
#endif
				
			SetRect(&titleStrBounds, strAreaLeft + inset, 
									 titleRect->top + 1, 
									 strAreaLeft + inset + titleWidth, 
									 titleRect->bottom - 1); 

#ifdef THICK_TITLEBAR
			
			//	Inset the bounds so as not to erase part of the tinges
			
			if (active)
				InsetRect(&titleStrBounds, 0, kTingeInset);

#endif		

			//	Make sure this area is cleared to the titlebar background color
			
			EraseRect(&titleStrBounds);

#ifdef THICK_TITLEBAR

			//	Outset it so decenders may overwrite the bottom tinge
			
			if (active)
				InsetRect(&titleStrBounds, 0, -kTingeInset);

#endif		

			//	Return the title string area's boundry

			*stringRect = titleStrBounds;
			
			GetClip(saveClip);
			
			
			//	Get the region the title string should go into
			
			InsetRect(&titleStrBounds, kTitleMargin, 0);
			RectRgn(clipRgn, &titleStrBounds);
			
			
			//	Make sure we don't clobber other windows
			
			SectRgn(saveClip, clipRgn, clipRgn);
			SetClip(clipRgn);


			//	Draw the title
			
			MoveTo(titleStrBounds.left, titleStrBounds.bottom - kTitleVDelta);
			DrawString(*window->titleHandle);
	
	
			//	Clean up
			
			SetClip(saveClip);

			if (variation != blackandwhite)
				RGBForeColor(&saveFore);

			HUnlock((Handle) window->titleHandle);
			TextFont(systemFont);
			TextSize(0);
			TextFace(0);
		}
	}
	DisposeRgn(saveClip);
	DisposeRgn(clipRgn);
#else
	#pragma unused (window, variation, titleRect, active, stringRect)
#endif
}

// *****************************************************************************
//	DrawTitleBar -- Draw pattern/title into the title bar
// -----------------------------------------------------------------------------
	//	This routine actually draws the pattern into the titlebar. Note: it
	//	takes a Rect as a parameter (not by address) because it goes ahead and
	//	modifies it. I figured this was no worse than needing to copy it into
	//	a local variable, so I went ahead and did it this way.

static void 
DrawTitleBar(WindowPeek window, short variation, const Rect *titleRect,
			 Boolean active) {
	Rect		tempRect;
	Rect		insetTitleRect;
	Rect		stringRect;
	Pattern		pat;
	RgnHandle	titleRgn = NewRgn();
	
	SetRect(&stringRect, 0, 0, 0, 0);
	
	if (active)
		DrawTitlebarTinges(window, variation, titleRect);
	
	SetWTitleBarColors(window, variation, active);
	

	//	Set up the starting region to be the whole titlebar, 
	//	parts will then be 'punched out' of it.

	insetTitleRect = *titleRect;
	InsetRect(&insetTitleRect, 1, 1);
	
	if (active)
		InsetRect(&insetTitleRect, kTingeInset, kTingeInset);
	
	RectRgn(titleRgn, &insetTitleRect);
	
	DrawTitleString(window, variation, titleRect, active, &stringRect);
		// since the area affected by the title string is returned in
		// stringRect, we can use it's left and right to draw the pattern
	
	if (!EmptyRect(&stringRect)) {
		RgnHandle tempRgn = NewRgn();
		
		RectRgn(tempRgn, &stringRect);
		DiffRgn(titleRgn, tempRgn, titleRgn);
		
		DisposeRgn(tempRgn);
	}
	

	//	Subtract the Close Box

	if (active && window->goAwayFlag) {
		GetCloseBox(window, titleRect, &tempRect);
		SubtractGadgetRect(titleRgn, &tempRect);
	}

#ifdef ALLOW_ZOOM

	//	Subtract the Zoom Box

	if (active && window->spareFlag) {
		GetZoomBox(window, titleRect, &tempRect);
		SubtractGadgetRect(titleRgn, &tempRect);
	}
#endif
	
	GetTitlebarPat(active, (Point*) &titleRect->top, (Pattern*) &pat);
	FillRgn(titleRgn, (ConstPatternParam) &pat);
	
	DisposeRgn(titleRgn);
}

////// *************************************************************************
/////
////	Zoom handling													 
///
// -----------------------------------------------------------------------------
#ifdef ALLOW_ZOOM
	
static void 
SetZoomRects(WindowPeek window) {
	Rect contRect;
	
	if (window->spareFlag) {
		GetGlobalContentRect(window, &contRect);
		WindData.wState.stdState = contRect;
#ifdef MFI_ZOOM
		if (WindData.isHoriz)							// titlebar on top
			contRect.bottom = contRect.top + 12;
		else											// titlebar on left
			contRect.right = contRect.left + 12;
#endif
		WindData.wState.userState = contRect;
			//	The stdState and user state might be backwards from how they
			//	would normally be used by an application. Check this for your
			//	own use and change them if necessary.
	}
}

// -----------------------------------------------------------------------------

static long 
GetZoomHitType(WindowPeek window) {
	Rect contRect;
	Rect stdRect;
	
	contRect = (**(window->contRgn)).rgnBBox;
	stdRect = WindData.wState.stdState;
	OffsetRect(&stdRect, -stdRect.left + contRect.left, 
						 -stdRect.top + contRect.top);
		// make topLeft the same for content and standard rects, and compare

	if (EqualRect(&contRect, &stdRect))
		return wInZoomIn;		// go to user state (make small, MFI)
	else
		return wInZoomOut;		// go to standard state (make normal, MFI)
}

#endif
////// *************************************************************************
/////
////	DoWInit -- Windoid initialization													 
///
// -----------------------------------------------------------------------------

void 
DoWInit(WindowPeek window, long param, short varCode) {
	#pragma unused(param)
	Handle zoomDataHndl;
	WindoidDataPtr wdata;

	window->spareFlag = false;
	zoomDataHndl = NewHandle(sizeof(WindoidData));
	if (zoomDataHndl) {
		wdata = (WindoidDataPtr) *zoomDataHndl;		// make it easier to access
		
		wdata->closeToggle 		= 0;
		wdata->hasTitlebar		= true;
		wdata->ignoreHilite		= false;

#ifdef ALLOW_GROW

	#ifdef MACAPP_STYLE
		wdata->hasGrow			= (varCode & kMacApp_hasGrow) != 0;
	#else
		wdata->hasGrow			= (varCode & noGrowDocProc) == 0;
	#endif

#endif

#ifdef ALLOW_VERT
		wdata->isHoriz 			= !((varCode & 1) || (varCode & 2));
#else
		wdata->isHoriz 			= true;
#endif

#ifdef ALWAYS_HILITE
		wdata->ignoreHilite		= true;
#endif

#ifdef MACAPP_STYLE
		wdata->ignoreHilite		= !(kMacApp_toggleTBar & varCode);
#endif

#ifdef THINK_STYLE
		if ((varCode & 7) != 0 && (varCode & 7) != 2)
			wdata->hasTitlebar	= false;
#endif


		//	Set the window's data handle to the right value
		
		window->dataHandle 		= zoomDataHndl;


#ifdef ALLOW_ZOOM
		wdata->zoomToggle		= 0;
		window->spareFlag		= (zoomDocProc & varCode);
		SetZoomRects(window);
#endif

	}
}

////// *************************************************************************
/////
////	DoWDispose -- Windoid disposal													 
///
// -----------------------------------------------------------------------------

void 
DoWDispose(WindowPeek window, long param) {
	#pragma unused(param)

	if (window->dataHandle)
		DisposeHandle(window->dataHandle);
}

////// *************************************************************************
/////
////	DoWHit -- Windoid hit routine													 
///
// -----------------------------------------------------------------------------

long 
DoWHit(WindowPeek window, long param) {
	Rect	titleRect;
	Rect	theRect;
	Point	hitPt;
	long	result;
#ifdef ALLOW_ZOOM
	Rect	stdRect;
	Rect	usrRect;
	Rect	contentRect;
#endif

	hitPt.v = HiWord(param);
	hitPt.h = LoWord(param);
	
	result = wNoHit;
	if (PtInRgn(hitPt, window->contRgn)) {
		result = wInContent;

#ifdef ALLOW_GROW

		// Look for a Grow region hit
		
		if (WindData.hasGrow) {
			GetGrowBox(window, &theRect);
			InsetRect(&theRect, -1, -1);
			if (PtInRect(hitPt, &theRect))
				result = wInGrow; 
		}
		
#endif

	} else {
		GetTitleBar(window, &titleRect);
		if (WindData.hasTitlebar && PtInRect(hitPt, &titleRect)) {
			result = wInDrag;
			if (WindData.ignoreHilite || window->hilited) {

				if (window->goAwayFlag) {
					GetCloseBox(window, &titleRect, &theRect);
					InsetRect(&theRect, -kGadgetHitFudge, -kGadgetHitFudge);
					if (PtInRect(hitPt, &theRect))
						result = wInGoAway;
				}

#ifdef ALLOW_ZOOM
				if (window->spareFlag) {
					GetZoomBox(window, &titleRect, &theRect);
					InsetRect(&theRect, -kGadgetHitFudge, -kGadgetHitFudge);
					if (PtInRect(hitPt, &theRect)) {
						result = GetZoomHitType(window);
						
						//	Calculate Offset for Zoom Rects (make sure they
						//	are up to date)
						
						contentRect = (**(window->contRgn)).rgnBBox;
						stdRect = WindData.wState.stdState;
						usrRect = WindData.wState.userState;
						
#ifdef STAYPUT_ZOOM
						OffsetRect(&(WindData.wState.stdState), 
								   contentRect.left - stdRect.left,
								   contentRect.top  - stdRect.top);
						OffsetRect(&(WindData.wState.userState), 
								   contentRect.left - usrRect.left, 
								   contentRect.top  - usrRect.top);
#else
						if (result == wInZoomIn) {
							OffsetRect(&(WindData.wState.stdState), 
									   contentRect.left - stdRect.left,
									   contentRect.top  - stdRect.top);
						} else {
							OffsetRect(&(WindData.wState.userState), 
									   contentRect.left - usrRect.left, 
									   contentRect.top  - usrRect.top);
						}
#endif						
					}
				}
#endif
			}
		}
	}
	return result;
}

////// *************************************************************************
/////
////	DoWDraw -- Windoid drawing routines													 
///
// -----------------------------------------------------------------------------

typedef struct {
	WindowPeek	wdlWindow;
	long		wdlParam;
} WDLDataRec;
	// This information is used to communicate with DeviceLoop callback routine

// *****************************************************************************
//	Windoid drawing loop
// -----------------------------------------------------------------------------
	// This routine actually does the real work of the drawing of stuff into
	// the window.

static pascal void 
WindoidDrawLoop(short depth, short deviceFlags, 
				GDHandle targetDevice, WDLDataRec *userData) {
	WindowPeek window;
	Rect titleRect;
	Rect tempRect;
	short variation;

	window = userData->wdlWindow;	// make sure our macros work
	variation = CheckDisplay(depth, deviceFlags, targetDevice, window);
	
	switch (userData->wdlParam) {
		case wNoHit:
			BackColor(whiteColor);
			

			//	Frame titlebar

			if (WindData.hasTitlebar) {
				GetTitleBar(window, &titleRect);
				SetWFrameColor(window, variation);
				FrameRect(&titleRect);
			}
			
			if (WindData.ignoreHilite || window->hilited) {

				//	Draw titlebar interior

				if (WindData.hasTitlebar)
					DrawTitleBar(window, variation, &titleRect, true);
				

				//	Draw close box

				if (window->goAwayFlag) {	
					GetCloseBox(window, &titleRect, &tempRect);
					DrawCloseBox(window, variation, &tempRect);
				}
					// Note: I have seen at least one windoid WDEF that is used
					// by some applications that does not utilize this flag.
					// In that case, I think it always had a close box (or it
					// might even have used the varcode to determine this, but
					// that wouldn't seem to be a good thing to do).

#ifdef ALLOW_ZOOM

				//	Draw zoom box

				if (window->spareFlag) {
					GetZoomBox(window, &titleRect, &tempRect);
					DrawZoomBox(window, variation, &tempRect);
				}
#endif
			} else {

				//	Draw dimmed titlebar

				DrawTitleBar(window, variation, &titleRect, false);
			}
			
			
			//	Draw content frame and shadow

			tempRect = (**window->strucRgn).rgnBBox;
			tempRect.bottom--;
			tempRect.right--;
			
			SetWFrameColor(window, variation);
			FrameRect(&tempRect);
			

			//	Draw Shadow

			OffsetRect(&tempRect, 1, 1);
			FrameBottomRightShading(tempRect);
			break;


		//	Toggle Go-Away Box

		case wInGoAway:
			GetTitleBar(window, &titleRect);
			GetCloseBox(window, &titleRect, &tempRect);
			if (WindData.closeToggle)
				DrawCloseBox(window, variation, &tempRect);
			else
				DrawXedBox(window, variation, &tempRect);
			break;
		
#ifdef ALLOW_ZOOM

		//	Toggle Zoom Box

		default:
			if (window->spareFlag) {
				GetTitleBar(window, &titleRect);
				GetZoomBox(window, &titleRect, &tempRect);
				if (WindData.zoomToggle)
					DrawZoomBox(window, variation, &tempRect);
				else
					DrawXedBox(window, variation, &tempRect);
			}
			break;
#endif
	}
	ColorsNormal();
}

// -----------------------------------------------------------------------------

void 
DoWDraw(WindowPeek window, long param) {
	WDLDataRec theUserData;

	if (window->visible) {
		theUserData.wdlWindow = window;
		theUserData.wdlParam = param;

#ifdef UNIV_HEADERS
		{
			DeviceLoopDrawingUPP uppDrawProc;
			
			uppDrawProc = NewDeviceLoopDrawingProc(WindoidDrawLoop);
			OurDeviceLoop(window->strucRgn, uppDrawProc, 
						  (long) &theUserData, (DeviceLoopFlags) 0);
			
			DisposeRoutineDescriptor(uppDrawProc);
		}
#else
		OurDeviceLoop(window->strucRgn, 
					  (DeviceLoopDrawingProcPtr) WindoidDrawLoop, 
					  (long) &theUserData, (DeviceLoopFlags) 0);
#endif

		switch (param) {
			case wNoHit:
				//	This is so param 0 doesn't get interpreted as a hit in the
				//	zoom box, incorrectly.
				break;
				
			case wInGoAway:						// toggle go-away
				WindData.closeToggle = ! WindData.closeToggle;
				break;
			
#ifdef ALLOW_ZOOM
			default:							// toggle zoom box
				WindData.zoomToggle = ! WindData.zoomToggle;
				break;
#endif
		}
	}
}

////// *************************************************************************
/////
////	DoWCalcRgns -- Windoid region calculating routine													 
///
// -----------------------------------------------------------------------------

void 
DoWCalcRgns(WindowPeek window, long param) {
	#pragma unused(param)
	Rect theRect;
	RgnHandle tempRgn = NewRgn();


	//	Calculate the content Rect in global coordinates
	
	GetGlobalContentRect(window, &theRect);
	RectRgn(window->contRgn, &theRect);


	//	Start off with the structure equal to the content
	//	& make it include the window frame and titlebar
	
	InsetRect(&theRect, -1, -1);
	if (WindData.hasTitlebar) {
		if (WindData.isHoriz)
			theRect.top -= kTitleHeight - 1;
		else
			theRect.left -= kTitleHeight - 1;
	}
	RectRgn(window->strucRgn, &theRect);
	
	
	//	Add the shadow to the structure 
	
	OffsetRect(&theRect, 1, 1);
	RectRgn(tempRgn, &theRect);
	UnionRgn(tempRgn, window->strucRgn, window->strucRgn);
	
	DisposeRgn(tempRgn);
}

////// *************************************************************************
/////
////	DoWGrow -- Draw the growing outline													 
///
// -----------------------------------------------------------------------------

void 
DoWGrow(WindowPeek window, long param) {
#ifdef ALLOW_GROW
	#pragma unused(window)
	Rect growingRect = *(Rect*) param;
	
	if (WindData.isHoriz)
		growingRect.top  -= kTitleHeight - 1;	// add room for the title bar
	else
		growingRect.left -= kTitleHeight - 1;	// add room for the title bar
	InsetRect(&growingRect, -1, -1);
	
	
	//	Draw the window frame.
	
	FrameRect(&growingRect);
	
	if (WindData.isHoriz)
		growingRect.top  += kTitleHeight - 1;
	else
		growingRect.left += kTitleHeight - 1;
	
	
	//	Now mark the title bar area
	
	MoveTo(growingRect.left, growingRect.top);
	if (WindData.isHoriz)
		LineTo(growingRect.right - 2, growingRect.top);
	else
		LineTo(growingRect.left, growingRect.bottom - 2);
	
	
	//	Mark the scroll bars too
	
	MoveTo(growingRect.right - kScrollBarPixels, growingRect.top + 1);
	LineTo(growingRect.right - kScrollBarPixels, growingRect.bottom - 2);
	
	MoveTo(growingRect.left, growingRect.bottom - kScrollBarPixels);
	LineTo(growingRect.right - 2, growingRect.bottom - kScrollBarPixels);       
#else
	#pragma unused(window, param)
#endif
}

////// *************************************************************************
/////
////	DoWDrawGIcon -- Draw the grow icon and scroll frame in the lower right													 
///
// -----------------------------------------------------------------------------
#ifdef ALLOW_GROW

static pascal void 
GrowBoxDrawLoop(short depth, short deviceFlags, 
				GDHandle targetDevice, WDLDataRec *userData) {
	WindowPeek window;
	Rect theRect;
	short variation;

	window = userData->wdlWindow;	// make sure our macros work
	variation = CheckDisplay(depth, deviceFlags, targetDevice, window);
	
	GetGrowBox(window, &theRect);
	DrawGrowBox(window, variation, &theRect);
	
	ColorsNormal();
}

#endif
// -----------------------------------------------------------------------------

void 
DoWDrawGIcon(WindowPeek window, long param) {
#ifdef ALLOW_GROW
	#pragma unused(param)

	if (window->visible && WindData.hasGrow) {
		WDLDataRec theUserData;
		RgnHandle saveClip = NewRgn();
		RgnHandle tempRgn = NewRgn();
		Point mappingPoint;
		
		SectRgn(window->port.visRgn, window->port.clipRgn, tempRgn);

		GetClip(saveClip);
		
		GetGlobalMappingPoint(window, &mappingPoint);
		OffsetRgn(tempRgn, mappingPoint.h, mappingPoint.v);
		
		SetClip(tempRgn);
		
		theUserData.wdlWindow = window;

#ifdef UNIV_HEADERS
		{
			DeviceLoopDrawingUPP uppDrawProc;
			
			uppDrawProc = NewDeviceLoopDrawingProc(GrowBoxDrawLoop);
			OurDeviceLoop(window->strucRgn, uppDrawProc, 
						  (long) &theUserData, (DeviceLoopFlags) 0);
			
			DisposeRoutineDescriptor(uppDrawProc);
		}
#else
		OurDeviceLoop(window->strucRgn, 
					  (DeviceLoopDrawingProcPtr) GrowBoxDrawLoop, 
					  (long) &theUserData, (DeviceLoopFlags) 0);
#endif

		SetClip(saveClip);

		DisposeRgn(saveClip);
		DisposeRgn(tempRgn);
	}
#else
	#pragma unused(window, param)
#endif
}

// *****************************************************************************

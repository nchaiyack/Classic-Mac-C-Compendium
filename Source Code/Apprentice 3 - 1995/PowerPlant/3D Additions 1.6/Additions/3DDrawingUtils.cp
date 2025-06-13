// ===========================================================================
// 3DDrawingUtils.cp  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// C++ wrapper class for 3D drawing utilities.
// Dependent on PPlant UDrawingUtils
//
// This class acts solely as a wrapper class for the lower level, ANSI C,
// routines in 3DUtilities. This class takes care of drawing to different 
// bit depths.

#include "3DDrawingUtils.h"
#include "3DAttachments.h"

#include <LWindow.h>


// ===========================================================================
// ¥ St3DPenState												St3DPenState ¥
// ===========================================================================

St3DPenState::St3DPenState()
{
	PenState	the3DPnState;
		  
	::GetPenState(&mPenState);
	
	::Get3DPenState(&the3DPnState);
	::SetPenState(&the3DPnState);
}


// ===========================================================================
// ¥ U3DDrawingUtils										 U3DDrawingUtils ¥
// ===========================================================================

void U3DDrawingUtils::MakeWindow3D(LWindow* inWindow)
{
	const Pattern black = {0xFFFFFFF0};
	PenState thePenState;
	thePenState.pnSize.h = 1;
	thePenState.pnSize.v = 1;
	thePenState.pnMode = srcCopy;
	thePenState.pnPat = black;

	inWindow->AddAttachment(new C3DPanelAttachment(&thePenState));
}
	

// ===========================================================================
// ===========================================================================
void	U3DDrawingUtils::Draw3DHLine ( short vpos, short h1, short h2,
									   const RGBColor *inULColor,
									   const RGBColor *inLRColor)
{
	PenState thePenState;
	Get3DPenState(&thePenState);
	
	Rect	theRect;
	Point	pt1 = {	vpos, h1},
			pt2 = {	vpos + 2*thePenState.pnSize.v, h2};
	
	::Pt2Rect( pt1, pt2, &theRect);
	
	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  ::Draw3DHLine ( vpos, h1, h2, inULColor, inLRColor);
		} else {
		  St3DPenState	thePnState;

		  ::MoveTo( h1, vpos);
		  ::LineTo( h2, vpos);
		  ::BWShadowLine( h1, vpos+thePenState.pnSize.v,
		  				  h2, vpos+thePenState.pnSize.v);
		}
	}
}
	
// ===========================================================================
void	U3DDrawingUtils::Draw3DVLine ( short hpos, short v1, short v2,
									   const RGBColor *inULColor,
									   const RGBColor *inLRColor)
{
	PenState thePenState;
	Get3DPenState(&thePenState);
	
	Rect	theRect;
	Point	pt1 = {	v1,	hpos},
			pt2 = {	v2,	hpos + 2*thePenState.pnSize.h};
	
	::Pt2Rect( pt1, pt2, &theRect);
	
	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  ::Draw3DVLine ( hpos, v1, v2, inULColor, inLRColor);
		} else {
		  St3DPenState	thePnState;

		  ::MoveTo( hpos, v1);
		  ::LineTo( hpos, v2);
		  ::BWShadowLine( hpos+thePenState.pnSize.v, v1,
		  				  hpos+thePenState.pnSize.v, v2);
		}
	}
}
	

// ===========================================================================
// ===========================================================================
void	U3DDrawingUtils::DrawCLine ( const RGBColor *inColor, short h1, short v1,
															  short h2, short v2)
{
  ::MoveTo( h1, v1);
	CLineTo ( inColor, h2, v2);
}
	
// ===========================================================================
void	U3DDrawingUtils::CLineTo ( const RGBColor *inColor, short h, short v)
{
	Rect	theRect;
	Point	pt1 = { v, h},
			pt2 = { v+2, h+2};
	
	::Pt2Rect( pt1, pt2, &theRect);
	
	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  ::CLineTo ( inColor, h, v);
		} else {
		  ::LineTo( h, v);
		}
	}
}
	
// ===========================================================================
// ===========================================================================

void 	U3DDrawingUtils::DoDeviceLoop( RectArgProc Draw3DEffect,
									   RectArgProc DrawPlainEffect, Rect* inRect)
{
	St3DDeviceLoop	theLoop(*inRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  Draw3DEffect(inRect);
		} else {
		  St3DPenState	thePnState;

		  DrawPlainEffect(inRect);
		}
	}
}

// ===========================================================================

void 	U3DDrawingUtils::DoDeviceLoop(
	Rect2RGBColorArgProc	Draw3DEffect,
	RectArgProc				DrawPlainEffect,
	Rect*					inRect,
	const RGBColor*			inColor1,
	const RGBColor*			inColor2)
{
	St3DDeviceLoop	theLoop(*inRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  Draw3DEffect( inRect, inColor1, inColor2);
		} else {
		  St3DPenState	thePnState;

		  DrawPlainEffect(inRect);
		}
	}
}

// ===========================================================================

void 	U3DDrawingUtils::DoDeviceLoop(
	Rect2RGBColorBooleanArgProc Draw3DEffect,
	RectArgProc					DrawPlainEffect,
	Rect*						inRect,
	const RGBColor*				inColor1,
	const RGBColor*				inColor2,
	const Boolean 				inFrameIt)
{
	St3DDeviceLoop	theLoop(*inRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  Draw3DEffect( inRect, inColor1, inColor2, inFrameIt);
		} else {
		  St3DPenState	thePnState;

		  DrawPlainEffect(inRect);
		}
	}
}

// ===========================================================================

void 	U3DDrawingUtils::DoDeviceLoop(
	Rect3RGBColorBooleanArgProc	Draw3DEffect,
	RectArgProc					DrawPlainEffect,
	Rect*						inRect,
	const RGBColor*				inColor1,
	const RGBColor*				inColor2,
	const RGBColor*				inColor3,
	const Boolean				inFrameIt)
{
	St3DDeviceLoop	theLoop(*inRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		  Draw3DEffect( inRect, inColor1, inColor2, inColor3, inFrameIt);
		} else {
		  St3DPenState	thePnState;

		  DrawPlainEffect(inRect);
		}
	}
}

// ===========================================================================

void 	U3DDrawingUtils::DoDeviceLoopHLine(
	LineArgProc	Draw3DEffect,
	short		vpos,
	short		h1,
	short		h2)
{
	Rect	theRect;
	Point	pt1 = {	vpos,	h1},
			pt2 = {	vpos+2,	h2};
	
	::Pt2Rect( pt1, pt2, &theRect);
	
	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
			Draw3DEffect( vpos, h1, h2);
		} else {
		  St3DPenState	thePnState;

		  ::MoveTo( h1, vpos);
		  ::LineTo( h2, vpos);
		}
	}
}

// ===========================================================================

void 	U3DDrawingUtils::DoDeviceLoopVLine(
	LineArgProc	Draw3DEffect,
	short		hpos,
	short		v1,
	short		v2)
{
	Rect	theRect;
	Point	pt1 = {	v1,	hpos},
			pt2 = {	v2,	hpos+2};
	
	::Pt2Rect( pt1, pt2, &theRect);
	
	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
		 	Draw3DEffect( hpos, v1, v2);
		} else {
		  St3DPenState	thePnState;

		  ::MoveTo( hpos, v1);
		  ::LineTo( hpos, v2);
		}
	}
}


// ===========================================================================
// ===========================================================================

void	U3DDrawingUtils::DoFrameOval(Rect* inRect)
	{ ::FrameOval(inRect); }

void	U3DDrawingUtils::DoFrameRect(Rect* inRect)
	{ ::FrameRect(inRect); }

// ===========================================================================
// 3DUtilities.cp	  �1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// Utilities for drawing 3D effects.
//
// This is intended to look and act as much like a Toolbox Manager as possible.
// You should use the first set of procedures in general, but the second set 
// is directly available for flexibility.

#include "3DUtilities.h"

// ===========================================================================
// Local variables. Statics are used rather than a class to allow ANSI C usage.
// ===========================================================================

static RGBColor theLightColor	= kWhite;
static RGBColor theShadowColor	= kShadowGray;
static RGBColor theBkgndColor	= kBackgroundGray;
static PenState	thePnState		= {{0,0},{1,1},srcCopy,
									{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};

// ===========================================================================
// Static function prototypes.
// ===========================================================================

static PenState	theOldPnState;	// only used by static functions.

static void	GetCPen();
static void	GetBWPen();
static void	SavePenState();
static void	RestorePenState();


// ===========================================================================
// Accessor functions to 3D colors.
// ===========================================================================

// � Set 3D State
void Set3DBackColor( RGBColor *inBkgndColor)	{ theBkgndColor	 = *inBkgndColor; }

void Set3DLightColor( RGBColor *inLightColor)	{ theLightColor	 = *inLightColor; }

void Set3DShadowColor( RGBColor *inShadowColor)	{ theShadowColor = *inShadowColor; }

void Set3DPenState( PenState *inPnState)		{ thePnState	 = *inPnState; }


// � Get 3D State
void Get3DBackColor( RGBColor *outBkgndColor)	{ *outBkgndColor = theBkgndColor; }

void Get3DLightColor( RGBColor *outLightColor)	{ *outLightColor = theLightColor; }
	
void Get3DShadowColor( RGBColor *outShadowColor){ *outShadowColor= theShadowColor; }

void Get3DPenState( PenState *outPnState)		{ *outPnState	 = thePnState; }


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DOvalPanel
// ---------------------------------------------------------------------------
// Draw a 3D oval panel (raised or embedded) in the current grafPort
// using the current PenState.

void	Draw3DOvalPanel( Rect *inRect,
					 const RGBColor *inBKColor,
					 const RGBColor *inULColor,
					 const RGBColor *inLRColor,
					 Boolean inFrameIt)
{
	RGBColor	theForeColor;

	GetForeColor( &theForeColor);

	// Paint background color
	RGBForeColor(inBKColor);
	PaintOval(inRect);

	// Draw oval border
	Draw3DOvalBorder( inRect, inULColor, inLRColor, inFrameIt);

	RGBForeColor( &theForeColor);
}

// ===========================================================================

// � Draw3DInsetOvalPanel
void Draw3DInsetOvalPanel( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DOvalPanel(inRect, &theLightColor, &theShadowColor, &theLightColor, true);
	RestorePenState();
}

// � Draw3DRaisedOvalPanel
void Draw3DRaisedOvalPanel( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DOvalPanel(inRect, &theBkgndColor, &theLightColor, &theShadowColor, false);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DOvalBorder
// ---------------------------------------------------------------------------
// Draw a 3D oval border (raised or embedded) in the current grafPort
// using the current PenState.

void	Draw3DOvalBorder( Rect *inRect,
					 const RGBColor *inULColor,
					 const RGBColor *inLRColor,
					 Boolean inFrameIt)
{
	RGBColor	theForeColor;

	GetForeColor( &theForeColor);

	// Draw upper left color
	RGBForeColor(inULColor);
	FrameOval(inRect);
	
	// Draw lower right color
	RGBForeColor(inLRColor);
	FrameArc( inRect, 45, 180);

	// Draw inner oval (frame)
	if (inFrameIt) {
		InsetRect(inRect,thePnState.pnSize.h,thePnState.pnSize.v);
		RGBForeColor(&kBlack);
		FrameOval(inRect);
		InsetRect(inRect,-thePnState.pnSize.h,-thePnState.pnSize.v);
	}

	RGBForeColor( &theForeColor);
}

// ===========================================================================

// � Draw3DInsetOvalBorder
void Draw3DInsetOvalBorder( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DOvalBorder(inRect, &theShadowColor, &theLightColor, false);
	RestorePenState();
}

// � Draw3DRaisedOvalBorder
void Draw3DRaisedOvalBorder( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DOvalBorder(inRect, &theLightColor, &theShadowColor, false);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DOvalFrame
// ---------------------------------------------------------------------------
// Draw a 3D oval frame (raised or embedded) in the current grafPort
// using the current PenState.

void	Draw3DOvalFrame( Rect *inRect,
					 const RGBColor *inULColor,
					 const RGBColor *inLRColor)
{
	RGBColor	theForeColor;

	GetForeColor( &theForeColor);

	RGBForeColor(inULColor);
	FrameOval(inRect);
	
	OffsetRect(inRect,thePnState.pnSize.h, thePnState.pnSize.v);
						 
	RGBForeColor(inLRColor);
	FrameOval(inRect);

	OffsetRect(inRect,-thePnState.pnSize.h, -thePnState.pnSize.v);
						 
	RGBForeColor( &theForeColor);
}

// ===========================================================================

// � Draw3DInsetOvalFrame
void Draw3DInsetOvalFrame( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DOvalFrame(inRect, &theShadowColor, &theLightColor);
	RestorePenState();
}

// � Draw3DRaisedOvalFrame
void Draw3DRaisedOvalFrame( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DOvalFrame(inRect, &theLightColor, &theShadowColor);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DPanel
// ---------------------------------------------------------------------------
// Draw a 3D panel (raised or embedded) in the current grafPort
// using the current PenState.

void	Draw3DPanel( Rect *inRect,
					 const RGBColor *inBKColor,
					 const RGBColor *inULColor,
					 const RGBColor *inLRColor,
					 Boolean inFrameIt)
{
	RGBColor	theForeColor;

	GetForeColor( &theForeColor);

	// Paint background color
	RGBForeColor(inBKColor);
	PaintRect(inRect);

	Draw3DBorder( inRect, inULColor, inLRColor, inFrameIt);

	RGBForeColor( &theForeColor);
}

// ===========================================================================

// � Draw3DInsetPanel
void Draw3DInsetPanel( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DPanel(inRect, &theLightColor, &theShadowColor, &theLightColor, true);
	RestorePenState();
}

// � Draw3DRaisedPanel
void Draw3DRaisedPanel( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DPanel(inRect, &theBkgndColor, &theLightColor, &theShadowColor, false);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DBorder
// ---------------------------------------------------------------------------
// Draw a 3D panel border (raised or embedded) in the current grafPort
// using the current PenState.

void	Draw3DBorder( Rect *inRect,
					  const RGBColor *inULColor,
					  const RGBColor *inLRColor,
					  Boolean inFrameIt)
{
	PenState	thePnState;
	RGBColor	theForeColor;
	
	GetPenState( &thePnState);
	GetForeColor( &theForeColor);

	// Draw lower right (shadow) lines
	RGBForeColor(inLRColor);
	FrameRect(inRect);

	// Draw upper left (light source) lines
	RGBForeColor(inULColor);
	MoveTo( inRect->left, inRect->bottom - thePnState.pnSize.v);
	LineTo( inRect->left, inRect->top);
	LineTo( inRect->right - thePnState.pnSize.h, inRect->top);
	
	// Draw inner box (frame)
	if (inFrameIt) {
		InsetRect(inRect,thePnState.pnSize.h,thePnState.pnSize.v);
		RGBForeColor(&kBlack);
		FrameRect(inRect);
		InsetRect(inRect,-thePnState.pnSize.h,-thePnState.pnSize.v);
	}
	
	RGBForeColor(&theForeColor);
}

// ===========================================================================

// � Draw3DInsetBorder
void	Draw3DInsetBorder( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DBorder(inRect, &theShadowColor, &theLightColor, false);
	RestorePenState();
}

// � Draw3DRaisedBorder
void	Draw3DRaisedBorder( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DBorder(inRect, &theLightColor, &theShadowColor, false);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DFrame
// ---------------------------------------------------------------------------
// Draw a 3D rect (raised or embedded) in the current grafPort using the
// current PenState.

void
Draw3DFrame( Rect *inRect,
			const RGBColor *inULColor,
			const RGBColor *inLRColor)
{
	PenState	thePnState;
	RGBColor	theForeColor;
	
	GetPenState( &thePnState);
	GetForeColor( &theForeColor);

	inRect->right -= thePnState.pnSize.h;
	inRect->bottom -= thePnState.pnSize.v;
	
	RGBForeColor(inULColor);
	FrameRect(inRect);
	
	OffsetRect(inRect,thePnState.pnSize.h, thePnState.pnSize.v);
						 
	RGBForeColor(inLRColor);
	FrameRect(inRect);

	OffsetRect(inRect,-thePnState.pnSize.h, -thePnState.pnSize.v);
						 
	RGBForeColor( &theForeColor);
	
	inRect->left -= thePnState.pnSize.h;
	inRect->top -= thePnState.pnSize.v;
}

// ===========================================================================

// � Draw3DInsetFrame
void Draw3DInsetFrame( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DFrame(inRect, &theShadowColor, &theLightColor);
	RestorePenState();
}

// � Draw3DRaisedFrame
void Draw3DRaisedFrame( Rect *inRect)
{
	SavePenState();
	GetCPen();
	Draw3DFrame(inRect, &theLightColor, &theShadowColor);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DHLine
// ---------------------------------------------------------------------------
// Draw a 3D line (raised or embedded) horizontally accross the current grafPort
// using the current PenState.

void
Draw3DHLine( short vpos, short h1, short h2,
			 const RGBColor *inULColor, const RGBColor *inLRColor)
{
	PenState	thePnState;
	RGBColor	theForeColor;
	
	GetPenState( &thePnState);
	GetForeColor( &theForeColor);

	DrawCLine( inULColor, h1, vpos, h2, vpos);
	DrawCLine( inLRColor, h1, vpos+thePnState.pnSize.v,
						  h2, vpos+thePnState.pnSize.v);

	RGBForeColor( &theForeColor);
}

// ===========================================================================

// � Draw3DInsetHLine
void Draw3DInsetHLine( short vpos, short h1, short h2)
{
	SavePenState();
	GetCPen();
	Draw3DHLine( vpos, h1, h2, &theShadowColor, &theLightColor);
	RestorePenState();
}

// � Draw3DRaisedHLine
void Draw3DRaisedHLine( short vpos, short h1, short h2)
{
	SavePenState();
	GetCPen();
	Draw3DHLine( vpos, h1, h2, &theLightColor, &theShadowColor);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� Draw3DVLine
// ---------------------------------------------------------------------------
// Draw a 3D line (raised or embedded) vertically accross the current grafPort
// using the current PenState.

void
Draw3DVLine( short hpos, short v1, short v2,
			 const RGBColor *inULColor, const RGBColor *inLRColor)
{
	PenState	thePnState;
	RGBColor	theForeColor;
	
	GetPenState( &thePnState);
	GetForeColor( &theForeColor);

	DrawCLine( inULColor, hpos, v1, hpos, v2);
	DrawCLine( inLRColor, hpos+thePnState.pnSize.h, v1,
						  hpos+thePnState.pnSize.h, v2);

	RGBForeColor( &theForeColor);
}

// ===========================================================================

// � Draw3DInsetVLine
void Draw3DInsetVLine( short hpos, short v1, short v2)
{
	SavePenState();
	GetCPen();
	Draw3DVLine( hpos, v1, v2, &theShadowColor, &theLightColor);
	RestorePenState();
}

// � Draw3DRaisedVLine
void Draw3DRaisedVLine( short hpos, short v1, short v2)
{
	SavePenState();
	SavePenState();
	GetCPen();
	Draw3DVLine( hpos, v1, v2, &theLightColor, &theShadowColor);
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� DrawCLine
// ---------------------------------------------------------------------------
// Draw a colored line within the current grafPort using the current PenState.

void
DrawCLine( const RGBColor *theColor,
		   short h1, short v1, short h2, short v2)
{
	MoveTo( h1, v1);
	CLineTo( theColor, h2, v2);
}

// ---------------------------------------------------------------------------
//		� CLineTo
// ---------------------------------------------------------------------------
// Draw a colored line within the current grafPort using the current PenState.

void
CLineTo( const RGBColor *theColor, short h, short v)
{
	RGBForeColor(theColor);
	LineTo( h, v);
}


// ===========================================================================
// ===========================================================================

// ---------------------------------------------------------------------------
//		� BWShadowInset
// ---------------------------------------------------------------------------

void
BWShadowInset( Rect* inRect)
{
	if ( inRect != nil ) {
		SavePenState();
		GetBWPen();
	
		MoveTo( inRect->left, inRect->bottom);
		LineTo( inRect->left, inRect->top);
		LineTo( inRect->right, inRect->top);
	
		RestorePenState();
	}
}


// ---------------------------------------------------------------------------
//		� BWShadowRaised
// ---------------------------------------------------------------------------

void
BWShadowRaised( Rect* inRect)
{
	if ( inRect != nil ) {
		SavePenState();
		GetBWPen();
	
		MoveTo( inRect->left, inRect->bottom);
		LineTo( inRect->right, inRect->bottom);
		LineTo( inRect->right, inRect->top);
	
		RestorePenState();
	}
}


// ---------------------------------------------------------------------------
//		� BWShadowLine
// ---------------------------------------------------------------------------
// Draw a BW shadowed line within the current grafPort using the current PenState.

void
BWShadowLine( short h1, short v1, short h2, short v2)
{
	SavePenState();
	GetBWPen();
	
	MoveTo( h1, v1);
	LineTo( h2, v2);
	
	RestorePenState();
}


// ===========================================================================
// ===========================================================================

void GetCPen()
{	SetPenState( &thePnState); }

void GetBWPen()
{
	Pattern grey = {0xFFFFFFE8};
	GetCPen();
	PenPat(&grey);
}

void SavePenState()
{	GetPenState( &theOldPnState); }

void RestorePenState()
{	SetPenState( &theOldPnState); }


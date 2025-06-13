// ===========================================================================
// 3DUtilities.h	  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// Utilities for drawing 3D effects.
//
// This is intended to look and act as much like a Toolbox Manager as possible.
// You should use the first set of procedures in general, but the second set 
// is directly available for flexibility.

#pragma once

#include <3DGrays.h>

// ===========================================================================
// These wrapper functions provide the basic 3D appearance recommended in
// Develop issue 15.
// ===========================================================================

void Set3DBackColor( RGBColor *inBkgndColor);
void Set3DLightColor( RGBColor *inLightColor);
void Set3DShadowColor( RGBColor *inShadowColor);
void Set3DPenState( PenState *inPnState);

void Get3DBackColor( RGBColor *outBkgndColor);
void Get3DLightColor( RGBColor *outLightColor);
void Get3DShadowColor( RGBColor *outShadowColor);
void Get3DPenState( PenState *outPnState);

void Draw3DInsetOvalPanel( Rect *inRect);
void Draw3DRaisedOvalPanel( Rect *inRect);

void Draw3DInsetOvalBorder( Rect *inRect);
void Draw3DRaisedOvalBorder( Rect *inRect);

void Draw3DInsetOvalFrame( Rect *inRect);
void Draw3DRaisedOvalFrame( Rect *inRect);

void Draw3DInsetPanel( Rect *inRect);
void Draw3DRaisedPanel( Rect *inRect);

void Draw3DInsetBorder( Rect *inRect);
void Draw3DRaisedBorder( Rect *inRect);

void Draw3DInsetFrame( Rect *inRect);
void Draw3DRaisedFrame( Rect *inRect);

void Draw3DInsetHLine( short vpos, short h1, short h2);
void Draw3DInsetVLine( short hpos, short v1, short v2);

void Draw3DRaisedHLine( short vpos, short h1, short h2);
void Draw3DRaisedVLine( short hpos, short v1, short v2);


// ===========================================================================
// These are the functions that actually do the work, use these to customize
// color usage. For inset effects, use dark color for inULColor and light color
// for inLRColor. Reverse colors for raised appearance.
// ===========================================================================

void	Draw3DOvalPanel( Rect *inRect, const RGBColor *inBKColor, const RGBColor *inULColor,
					 const RGBColor *inLRColor, Boolean inFrameIt);

void	Draw3DOvalBorder( Rect *inRect, const RGBColor *inULColor, const RGBColor *inLRColor, 
					 Boolean inFrameIt);

void	Draw3DOvalFrame( Rect *inRect, const RGBColor *inULColor, const RGBColor *inLRColor);

void	Draw3DPanel( Rect *inRect, const RGBColor *inBKColor, const RGBColor *inULColor,
					 const RGBColor *inLRColor, Boolean inFrameIt);

void	Draw3DBorder( Rect *inRect, const RGBColor *inULColor, const RGBColor *inLRColor,
					  Boolean inFrameIt);

void	Draw3DFrame( Rect *inRect, const RGBColor *inULColor, const RGBColor *inLRColor);

void	Draw3DHLine( short vpos, short h1, short h2,
					 const RGBColor *inULColor, const RGBColor *inLRColor);
					 
void	Draw3DVLine( short hpos, short v1, short v2,
					 const RGBColor *inULColor, const RGBColor *inLRColor);

void	DrawCLine( const RGBColor *inColor, short h1, short v1, short h2, short v2);
void	CLineTo( const RGBColor *theColor, short h, short v);

void	BWShadowInset( Rect* inRect);
void	BWShadowRaised( Rect* inRect);
void	BWShadowLine( short h1, short v1, short h2, short v2);

// ===========================================================================
// Provide backward compatability (ONLY) with early releases of this package.

#define DrawH3DLine		Draw3DHLine
#define DrawV3DLine		Draw3DVLine

// ===========================================================================






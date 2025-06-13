// ===========================================================================
// 3DDrawingUtils.h	  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// C++ wrapper class for 3D drawing utilities.
// Dependent on PPlant UDrawingUtils
//
// This class acts solely as a wrapper class for the lower level, ANSI C,
// routines in 3DUtilities. This class takes care of drawing to different 
// bit depths.

#pragma once

#include <3DUtilities.h>
#include <UDrawingUtils.h>

class LWindow;

// ===========================================================================

typedef void (*RectArgProc)(Rect* inRect);
typedef void (*Rect2RGBColorArgProc)
			 (Rect* inRect, const RGBColor* inColor1, const RGBColor* inColor2);
typedef void (*Rect2RGBColorBooleanArgProc)
			 (Rect* inRect, const RGBColor* inColor1, const RGBColor* inColor2,
			  Boolean inBoolean);
typedef void (*Rect3RGBColorBooleanArgProc)
			 (Rect* inRect, const RGBColor* inColor1, const RGBColor* inColor2,
			  const RGBColor* inColor3, Boolean inBoolean);
typedef void (*LineArgProc)(short arg1, short arg2, short arg3);

// ===========================================================================

class St3DDeviceLoop : public StDeviceLoop {
public:
	St3DDeviceLoop(const Rect &inLocalRect);
	
	Boolean	Next();
	Boolean	CurrentDeviceIsGrayscale();
	Boolean	CurrentDeviceIs3DCapable();
	Boolean	CurrentDeviceIsGrayCapable();
};


// ===========================================================================

class St3DPenState {
public:
	St3DPenState();
	~St3DPenState();

private:
	PenState mPenState;
};


// ===========================================================================

class	U3DDrawingUtils {
public:
	static void MakeWindow3D( LWindow* inWindow);

	static void Set3DBackColor( RGBColor *inBkgndColor);
	static void Set3DLightColor( RGBColor *inLightColor);
	static void Set3DShadowColor( RGBColor *inShadowColor);
	static void Set3DPenState( PenState *inPnState);

	static void Get3DBackColor( RGBColor *outBkgndColor);
	static void Get3DLightColor( RGBColor *outLightColor);
	static void Get3DShadowColor( RGBColor *outShadowColor);
	static void Get3DPenState( PenState *outPnState);

	static void	Draw3DInsetOvalPanel( Rect *inRect);
	static void	Draw3DRaisedOvalPanel( Rect *inRect);

	static void	Draw3DInsetOvalBorder( Rect *inRect);
	static void	Draw3DRaisedOvalBorder( Rect *inRect);

	static void	Draw3DInsetOvalFrame( Rect *inRect);
	static void	Draw3DRaisedOvalFrame( Rect *inRect);

	static void Draw3DInsetPanel( Rect *inRect);
	static void Draw3DRaisedPanel( Rect *inRect);

	static void Draw3DInsetBorder( Rect *inRect);
	static void Draw3DRaisedBorder( Rect *inRect);

	static void Draw3DInsetFrame( Rect *inRect);
	static void Draw3DRaisedFrame( Rect *inRect);

	static void Draw3DInsetHLine( short vpos, short h1, short h2);
	static void Draw3DInsetVLine( short hpos, short v1, short v2);

	static void Draw3DRaisedHLine( short vpos, short h1, short h2);
	static void Draw3DRaisedVLine( short hpos, short v1, short v2);
	
	
	static void	Draw3DOvalPanel( Rect *inRect, const RGBColor *inBKColor,
											   const RGBColor *inULColor,
											   const RGBColor *inLRColor,
											   Boolean inFrameIt);

	static void	Draw3DOvalBorder( Rect *inRect, const RGBColor *inULColor,
												const RGBColor *inLRColor,
												Boolean inFrameIt);

	static void	Draw3DOvalFrame( Rect *inRect, const RGBColor *inULColor,
											   const RGBColor *inLRColor);

	static void	Draw3DPanel ( Rect *inRect, const RGBColor *inBKColor,
											const RGBColor *inULColor,
											const RGBColor *inLRColor,
											Boolean inFrameIt);

	static void	Draw3DBorder( Rect *inRect, const RGBColor *inULColor,
											const RGBColor *inLRColor,
											Boolean inFrameIt);

	static void	Draw3DFrame ( Rect *inRect, const RGBColor *inULColor,
											const RGBColor *inLRColor);

	static void	Draw3DHLine ( short vpos, short h1, short h2,
							  const RGBColor *inULColor, const RGBColor *inLRColor);
					 
	static void	Draw3DVLine ( short hpos, short v1, short v2,
							  const RGBColor *inULColor, const RGBColor *inLRColor);

	static void	DrawCLine ( const RGBColor *inColor, short h1, short v1, short h2, short v2);
	static void	CLineTo ( const RGBColor *inColor, short h, short v);

private:
	static void DoDeviceLoop( RectArgProc Draw3DEffect,
							  RectArgProc DrawPlainEffect, Rect* inRect);
							  
	static void DoDeviceLoop( Rect2RGBColorArgProc Draw3DEffect,
							  RectArgProc DrawPlainEffect, Rect *inRect,
							  const RGBColor *inColor1, const RGBColor *inColor2);
							  
	static void DoDeviceLoop( Rect2RGBColorBooleanArgProc Draw3DEffect,
							  RectArgProc DrawPlainEffect, Rect *inRect,
							  const RGBColor *inColor1, const RGBColor *inColor2,
							  const Boolean inFrameIt);
							  
	static void DoDeviceLoop( Rect3RGBColorBooleanArgProc Draw3DEffect,
							  RectArgProc DrawPlainEffect, Rect *inRect,
							  const RGBColor *inColor1, const RGBColor *inColor2,
							  const RGBColor *inColor3, const Boolean inFrameIt);
							  
	static void DoDeviceLoopHLine( LineArgProc Draw3DEffect, short vpos, short h1, short h2);
	static void DoDeviceLoopVLine( LineArgProc Draw3DEffect, short hpos, short v1, short v2);
							  
	static void DoFrameOval(Rect* inRect);
	static void DoFrameRect(Rect* inRect);
};

#include <3DDrawingUtils.inline.cp>
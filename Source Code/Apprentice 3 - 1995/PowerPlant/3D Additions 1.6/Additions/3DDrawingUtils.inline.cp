// ===========================================================================
//	3DDrawingUtils.inline.cp
//	©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// C++ wrapper class inlined procedures for 3D drawing utilities.
// Dependent on PPlant UDrawingUtils
//
// These routines were optimized for speed without sacrificing code bloat.
// The key to this is the multiple DoDeviceLoop routines.

#pragma once

// ===========================================================================
// ¥ St3DDeviceLoop											  St3DDeviceLoop ¥
// ===========================================================================

inline St3DDeviceLoop::St3DDeviceLoop(const Rect &inLocalRect)
	: StDeviceLoop(inLocalRect) {}

inline Boolean St3DDeviceLoop::Next()
	{ Int16 depth; return NextDepth(depth); }

inline Boolean St3DDeviceLoop::CurrentDeviceIsGrayscale()
	// This test came from Zig Zichterman's LDeviceLoop:
	// !!! I haven't seen this flag documented, so this might
	// !!! be a dangerous test. It's worked so far though...
	{ return (((**mCurrentDevice).gdFlags & 0x0001) == 0); }
	
inline Boolean St3DDeviceLoop::CurrentDeviceIs3DCapable()
	{ Int16 depth = (**((**mCurrentDevice).gdPMap)).pixelSize;
	  return ( (8 <= depth) || ((4 <= depth) && (CurrentDeviceIsGrayscale())) );
	}
	
inline Boolean St3DDeviceLoop::CurrentDeviceIsGrayCapable()
	{ Int16 depth = (**((**mCurrentDevice).gdPMap)).pixelSize;
	  return ( (4 <= depth) || ((2 <= depth) && (CurrentDeviceIsGrayscale())) );
	}


// ===========================================================================
// ¥ St3DPenState												St3DPenState ¥
// ===========================================================================

inline St3DPenState::~St3DPenState()
	{ ::SetPenState(&mPenState); }


// ===========================================================================
// ¥ U3DDrawingUtils										 U3DDrawingUtils ¥
// ===========================================================================

// ===========================================================================
// ---------------------------------------------------------------------------
// ¥ Accessor Routines									   Accessor Routines ¥
// ---------------------------------------------------------------------------

inline void U3DDrawingUtils::Set3DBackColor( RGBColor *inBkgndColor)
	{ ::Set3DBackColor(inBkgndColor); }
	
inline void U3DDrawingUtils::Set3DLightColor( RGBColor *inLightColor)
	{ ::Set3DLightColor(inLightColor); }
	
inline void U3DDrawingUtils::Set3DShadowColor( RGBColor *inShadowColor)
	{ ::Set3DShadowColor(inShadowColor); }
	
inline void U3DDrawingUtils::Set3DPenState( PenState *inPnState)
	{ ::Set3DPenState(inPnState); }
	
	
// ===========================================================================
// ---------------------------------------------------------------------------
// ¥ Accessing Routines									  Accessing Routines ¥
// ---------------------------------------------------------------------------

inline void U3DDrawingUtils::Get3DBackColor( RGBColor *outBkgndColor)
	{ ::Get3DBackColor(outBkgndColor); }
	
inline void U3DDrawingUtils::Get3DLightColor( RGBColor *outLightColor)
	{ ::Get3DLightColor(outLightColor); }
	
inline void U3DDrawingUtils::Get3DShadowColor( RGBColor *outShadowColor)
	{ ::Get3DShadowColor(outShadowColor); }
	
inline void U3DDrawingUtils::Get3DPenState( PenState *outPnState)
	{ ::Get3DPenState(outPnState); }
	
	
// ===========================================================================
// ---------------------------------------------------------------------------
// ¥ Ovals															   Ovals ¥
// ---------------------------------------------------------------------------

inline void	U3DDrawingUtils::Draw3DOvalPanel( Rect *inRect,
			const RGBColor *inBKColor, const RGBColor *inULColor,
			const RGBColor *inLRColor, Boolean inFrameIt)
	{ DoDeviceLoop( ::Draw3DOvalPanel, U3DDrawingUtils::DoFrameOval, inRect,
					  inBKColor, inULColor, inLRColor, inFrameIt); }

inline void U3DDrawingUtils::Draw3DInsetOvalPanel( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DInsetOvalPanel, U3DDrawingUtils::DoFrameOval, inRect); }
	
inline void U3DDrawingUtils::Draw3DRaisedOvalPanel( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DRaisedOvalPanel, U3DDrawingUtils::DoFrameOval, inRect); }

// ---------------------------------------------------------------------------

inline void	U3DDrawingUtils::Draw3DOvalBorder( Rect *inRect,
			const RGBColor *inULColor, const RGBColor *inLRColor, Boolean inFrameIt)
	{ DoDeviceLoop( ::Draw3DOvalBorder, U3DDrawingUtils::DoFrameOval, inRect,
					  inULColor, inLRColor, inFrameIt); }

inline void U3DDrawingUtils::Draw3DInsetOvalBorder( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DInsetOvalBorder, U3DDrawingUtils::DoFrameOval, inRect); }

inline void U3DDrawingUtils::Draw3DRaisedOvalBorder( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DRaisedOvalBorder, U3DDrawingUtils::DoFrameOval, inRect); }

// ---------------------------------------------------------------------------

inline void	U3DDrawingUtils::Draw3DOvalFrame( Rect *inRect,
			const RGBColor *inULColor, const RGBColor *inLRColor)
	{ DoDeviceLoop( ::Draw3DOvalFrame, U3DDrawingUtils::DoFrameOval, inRect,
					  inULColor, inLRColor); }

inline void U3DDrawingUtils::Draw3DInsetOvalFrame( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DInsetOvalFrame, U3DDrawingUtils::DoFrameOval, inRect); }

inline void U3DDrawingUtils::Draw3DRaisedOvalFrame( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DRaisedOvalFrame, U3DDrawingUtils::DoFrameOval, inRect); }


// ===========================================================================
// ---------------------------------------------------------------------------
// ¥ Frames															  Frames ¥
// ---------------------------------------------------------------------------

inline void	U3DDrawingUtils::Draw3DPanel ( Rect *inRect,
			const RGBColor *inBKColor, const RGBColor *inULColor,
			const RGBColor *inLRColor, Boolean inFrameIt)
	{ DoDeviceLoop( ::Draw3DPanel, U3DDrawingUtils::DoFrameRect, inRect,
					  inBKColor, inULColor, inLRColor, inFrameIt); }

inline void U3DDrawingUtils::Draw3DInsetPanel( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DInsetPanel, U3DDrawingUtils::DoFrameRect, inRect); }
	
inline void U3DDrawingUtils::Draw3DRaisedPanel( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DRaisedPanel, U3DDrawingUtils::DoFrameRect, inRect); }
	
// ---------------------------------------------------------------------------

inline void	U3DDrawingUtils::Draw3DBorder( Rect *inRect,
			const RGBColor *inULColor, const RGBColor *inLRColor, Boolean inFrameIt)
	{ DoDeviceLoop( ::Draw3DBorder, U3DDrawingUtils::DoFrameRect, inRect,
					  inULColor, inLRColor, inFrameIt); }
	
inline void U3DDrawingUtils::Draw3DInsetBorder( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DInsetBorder, U3DDrawingUtils::DoFrameRect, inRect); }
	
inline void U3DDrawingUtils::Draw3DRaisedBorder( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DRaisedBorder, U3DDrawingUtils::DoFrameRect, inRect); }
	
// ---------------------------------------------------------------------------

inline void	U3DDrawingUtils::Draw3DFrame ( Rect *inRect,
			const RGBColor *inULColor, const RGBColor *inLRColor)
	{ DoDeviceLoop( ::Draw3DFrame, U3DDrawingUtils::DoFrameRect, inRect,
					  inULColor, inLRColor); }
	
inline void U3DDrawingUtils::Draw3DInsetFrame( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DInsetFrame, U3DDrawingUtils::DoFrameRect, inRect); }
	
inline void U3DDrawingUtils::Draw3DRaisedFrame( Rect *inRect)
	{ DoDeviceLoop( ::Draw3DRaisedFrame, U3DDrawingUtils::DoFrameRect, inRect); }


// ===========================================================================
// ---------------------------------------------------------------------------
// ¥ Lines															   Lines ¥
// ---------------------------------------------------------------------------

inline void U3DDrawingUtils::Draw3DInsetHLine( short vpos, short h1, short h2)
	{ U3DDrawingUtils::DoDeviceLoopHLine( ::Draw3DInsetHLine, vpos, h1, h2); }
	
inline void U3DDrawingUtils::Draw3DRaisedHLine( short vpos, short h1, short h2)
	{ U3DDrawingUtils::DoDeviceLoopHLine( ::Draw3DRaisedHLine, vpos, h1, h2); }
	
// ---------------------------------------------------------------------------

inline void U3DDrawingUtils::Draw3DInsetVLine( short hpos, short v1, short v2)
	{ U3DDrawingUtils::DoDeviceLoopVLine( ::Draw3DInsetVLine, hpos, v1, v2); }
	
inline void U3DDrawingUtils::Draw3DRaisedVLine( short hpos, short v1, short v2)
	{ U3DDrawingUtils::DoDeviceLoopVLine( ::Draw3DRaisedVLine, hpos, v1, v2); }

// ===========================================================================
// ===========================================================================


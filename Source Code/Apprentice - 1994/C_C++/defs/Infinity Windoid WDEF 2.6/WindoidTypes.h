// *****************************************************************************
//
//	WindoidTypes.h
//
// *****************************************************************************
#ifndef __WindoidTypes__
#define __WindoidTypes__

// -----------------------------------------------------------------------------
#ifndef __WindoidDefines__
#include <WindoidDefines.h>
#endif

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __WINDOWS__
#include <Windows.h>
#endif

// *****************************************************************************
//	Constants
// -----------------------------------------------------------------------------
//	Titlebar and gadget sizes and offsets

#ifdef THICK_TITLEBAR

	#define	kTitleHeight	13		// height of the windoid's titlebar.
	#define kTingeInset		 1		// when insetting to adjust for window sides
	
	#define kGadgetInset	 3		// inset from top/bottom of titlebar 
									//    (with titlebar on top of window)
	#define kGadgetMargin	 8		// space between edge and gadget
	#define kGadgetSize		(kTitleHeight - (2 * kGadgetInset))

	#define kTitleVDelta	 2		// how far up from bottom of titlebar the
									// the baseline of the text is located
#else

	#define	kTitleHeight	11		// height of the windoid's titlebar.
	#define kTingeInset		 0		// when insetting to adjust for window sides
	
	#define kGadgetInset	 2		// inset from top/bottom of titlebar 
									//    (with titlebar on top of window)
	#define kGadgetMargin	 6		// space between edge and gadget
	#define kGadgetSize		(kTitleHeight - (2 * kGadgetInset))

	#define kTitleVDelta	 1		// how far up from bottom of titlebar the
									// the baseline of the text is located
#endif

	#define kGadgetHitFudge	 1
	
// -----------------------------------------------------------------------------
//	Scroll Bar width

#ifndef SMALL_GROW
	#define kScrollBarPixels	16
#else
	#define kScrollBarPixels	13
#endif

// -----------------------------------------------------------------------------
//	Font information for titlebar title

#define kTitleFont			applFont
#define kTitleSize			9
#define kTitleStyle			bold

#define kTitleMargin		5		// space between pattern and edges of text	

// -----------------------------------------------------------------------------
//	Color table tinge percentage constants	

#define	wTitleBarLightPct		0x1
#define	wTitleBarDarkPct		0x8
#define wTitleBarTingeDarkPct	0x4

#define wCloseBoxColor			0x5

#define wGrowBoxBackground		0x1
#define wGrowBoxColorLt			0x4
#define wGrowBoxColorDk			0x5

#define	wXedBoxPct				0x8
#define	wInactiveFramePct		0xA
#define	wInactiveTextPct		0x7

// -----------------------------------------------------------------------------
//	Color table constants	

enum {
	wHiliteColorLight = 5, 
	wHiliteColorDark,
	wTitleBarLight,
	wTitleBarDark,
	wDialogLight,
	wDialogDark,
	wTingeLight,
	wTingeDark
};
	// These are the constants defined in the Apple technical note regarding
	// Color, Windows, and System 7. Last I checked, they weren't in an Apple
	// header file. (But the ones < 5 are, from the previous, pre-System 7
	// coloring scheme.)

// -----------------------------------------------------------------------------
//	Style variation constants	
// -----------------------------------------------------------------------------

enum {
	blackandwhite = 0,
#ifndef SYS7_OR_LATER
	sys6color,
#endif
	sys7color
};
	// These constants represent the three types of window 'colorings' we
	// support.
	
// -----------------------------------------------------------------------------
//	MacApp style variations	
// -----------------------------------------------------------------------------

#define kMacApp_toggleTBar	1	// bit 0 tells us whether to hilite/unhilite 
								//       title bar
#define kMacApp_hasTallTBar	2	// bit 1 is tall title bar bit (unsupported)
#define kMacApp_hasGrow		4	// bit 2 is grow bit
#define kMacApp_hasZoom		8	// bit 3 is zoom bit (standard zoom bit)

// *****************************************************************************
//	Structures
// -----------------------------------------------------------------------------

typedef struct {
	WStateData		wState;
	unsigned char	closeToggle;
	unsigned char	zoomToggle;
	unsigned char	isHoriz;
	unsigned char	ignoreHilite;
	unsigned char	hasTitlebar;
#ifdef ALLOW_GROW
	unsigned char	hasGrow;
#endif
} WindoidData, *WindoidDataPtr, **WindoidDataHandle;

// -----------------------------------------------------------------------------

#define WindData (**(WindoidDataHandle)(window->dataHandle))

	// This macro is used so I can access the 'globals' easily. Note: the
	// variable containing the window must be named 'window', and it must be in
	// scope at the time of the usage of this macro.
	// Also, they aren't REALLY globals, becuase they're kept for EACH window,
	// which is why I use a bitfield, trying to keep this handle as small as
	// possible.)

// *****************************************************************************
#endif
// *****************************************************************************
//
//	WindoidDefines.h
//
// -----------------------------------------------------------------------------
//	This file contains only the #define's used to determine how to compile
//	the Infinity Windoid WDEF. By modifying only this file, you can choose
//	what capabilities will be included when the WDEF is compiled.
// *****************************************************************************
#ifndef __WindoidDefines__
#define __WindoidDefines__

// *****************************************************************************
//	Conditional Compilation Options
// -----------------------------------------------------------------------------
/*

The #define's you may make include:

	ALLOW_VERT -	This determines if the code that is compiled will support
					a palette that has a vertical title bar along the left
					side of the window (rather than the top). To create such
					a window, add one to the varcode that is used.

	ALLOW_GROW -	This option creates a WDEF that supports a grow box. This
					grow box behaves exactly like that of a standard document
					window. This addition to the code was provided to me by
					Jim Petrick.
	
	ALLOW_ZOOM -	This option creates a windoid that supports a zoombox.
					To add the zoombox to a windoid, add zoomDocProc to the
					procID when creating the window. 
					
					The application is responsible for setting the user state 
					and standard state Rects of the window for zooming. (When 
					a new window is created, both of these are initialized to 
					the bounds of the window. See the README for more 
					information).

	SMALL_GROW -	This option will cause the grow box to be drawn smaller
					than the standard size.

	STAYPUT_ZOOM -	This option will cause both the user and standard Rects to
					be changed together, causing the zoomed in and out state
					to have the same topLeft at all times. If this is not set,
					the standard and user states have their own 'memories'.
					(Note: this behavior is not fully tested.)

	MFI_ZOOM -		This will cause the WDEF to handle the zoom box in the way
					expected by MicroFrontier applications (the same way as
					version 2.2). That is, the other state that is used is one
					which has a title bar with a small area (in which the host
					program displays a label for the windoid).
					
	MACAPP_STYLE -	This determines if the varcodes that are supported are
					the one's I consider 'normal' or the ones MacApp's
					windoid WDEF knows about. Note, however, that this code
					only supports the 'smaller', title-less version that
					is available in MacApp's.

	THINK_STYLE -	This creates a version of the WDEF that is totally
					compatible with the varcodes used in the windoid WDEF 
					included with the THINK environments. 
					
					Two things are done: varcode 0 gives a normal titlebar, 
					varcode 2 gives a titlebar down the left side, and other 
					varcodes (i.e. 1,3,4,5,6,7) give no titlebar at all. 
					(Note that in my version, a zoom box may also be used 
					with this style.) Also, for the THINK style, ALWAYS_HILITE 
					is also set, as this is how theirs works.

	ALWAYS_HILITE -	This will cause a windoid to be created that will always
					draw its title bar with gadgets and all. Normally, the
					windoid will draw the titlebar and frame in gray and
					empty when the window is not hilighted (like normal
					windows). Some programs, however, don't keep their
					windoids properly hilighted, so this will make them 
					appear to always be active.

	VERS_2_2_COMPATIBLE - This creates a version of the WDEF that is 
					functionally compatible with the version 2.2 windoid that
					I released. The only issue involved is the way the zooming
					is handled.

	THICK_TITLEBAR - Defining this will cause the titlebar of the windoid to
					be a little taller and include tinges along the outside
					that match those of document windows. 
					
					This is similar to Adobe's recent windoids, and also 
					similar to (but better than in my opinion) the floating 
					windows put up by Casper on the AV Macs. 
					
					This option works especially well if the window has a 
					grow box (the overall appearance of the windoid is better) 
					or if title strings are being shown, as descenders are 
					cut off with the thinner titlebar.

	TITLE_STRING -	Due to popular request and several commercial applications
					with titles in their floaters, I have implemented this
					option. When enabled, the windoid's title will appear
					centered in the titlebar. 
					
					Currently, this feature does not look at the script and 
					change the size of the titlebar as is recommended for 
					regions with an application font that doesn't work at 
					9 points. 
					
					Now, a note: I personally don't like the title showing in 
					the titlebar of a floater, since it makes it look more 
					like a document window. If you like them, though, the ones 
					I have implemented take everything into account.

	SYS7_OR_LATER - If this flag is set, a version of the WDEF that doesn't
					support the System 6 coloring will be compiled. Note that
					this version may crash on systems earlier than 7.0, so 
					your application should make sure to check for System 7
					or later.
					
	USE_GESTALT -	Define this if you want to use Gestalt to determine the
					System and QuickDraw versions. Otherwise, SysEnvirons is
					used. (NOTE: using Gestalt in MPW will cause additional
					glue code to be attached to the WDEF resource, making it
					larger.)  With SYS7_OR_LATER, this is automatically
					defined.

	DONT_ALIGN_ZOOM - Prevents the zoom box from forcing itself from being
					aligned with the titlebar pattern so that it appears
					aligned the same as the close box (i.e. it would be an
					extra pixel from the edge of the window).

	UNIV_HEADERS -	Define this if you are compiling the WDEF with Universal
					headers (the only change made is that if you are using
					them, the ProcPtr for the DeviceLoop drawing routines
					get created with UniversalProcPtrs).

	NOT_PBOOK_AWARE - If this is NOT defined, the WDEF will act like the WDEF 
					that is included in the PowerBook enabler. That is, when 
					the window is on the main screen of a PowerBook with a 
					passive matrix display, the titlebar will always be drawn 
					in black and white (to avoid shimmering).  If it is defined,
					PowerBook passive displays are handled like any other 
					display.

------------------------------------------------------------------------------*/

#define THICK_TITLEBAR
// #define TITLE_STRING

#define ALLOW_ZOOM
#define ALLOW_VERT

// #define ALLOW_GROW
// #define SMALL_GROW

// #define SYS7_OR_LATER

// #define UNIV_HEADERS
#define VERS_2_2_COMPATIBLE

// *****************************************************************************
//	Combinations
//		These should pretty much remain untouched
// -----------------------------------------------------------------------------

#ifdef VERS_2_2_COMPATIBLE
	#ifndef MFI_ZOOM
	#define MFI_ZOOM
	#endif
	
	#ifndef STAYPUT_ZOOM
	#define STAYPUT_ZOOM
	#endif
#endif

// -----------------------------------------------------------------------------

#ifdef THINK_STYLE
	#ifndef ALLOW_VERT
	#define ALLOW_VERT
	#endif

	#ifndef ALWAYS_HILITE
	#define ALWAYS_HILITE
	#endif

	#ifdef MACAPP_STYLE
	#undef MACAPP_STYLE
	#endif
#endif

// -----------------------------------------------------------------------------

#ifdef MACAPP_STYLE
	#ifdef ALLOW_VERT
	#undef ALLOW_VERT
	#endif

	#ifdef ALWAYS_HILITE
	#undef ALWAYS_HILITE
	#endif

	#ifndef ALLOW_GROW
	#define ALLOW_GROW
	#endif
#endif

// -----------------------------------------------------------------------------

#ifdef SYS7_OR_LATER
	#ifndef USE_GESTALT
	#define USE_GESTALT
	#endif
#endif

// *****************************************************************************
//	System version define for the Apple Interfaces
// -----------------------------------------------------------------------------

#ifdef SYS7_OR_LATER
	#define SystemSevenOrLater 1	
#else
	#define SystemSixOrLater 1		
#endif

	// This is used so that we can cut down on the code size in MPW. If 
	// support for earlier systems is important, get rid of this. 
	// Note: for this define to work under THINK C, MacHeaders cannot be used.

// *****************************************************************************
#endif
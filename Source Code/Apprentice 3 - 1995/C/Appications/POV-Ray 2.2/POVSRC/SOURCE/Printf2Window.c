/*
==============================================================================
Project:	POV-Ray

Version:	2.2

File Name:	Printf2Window.c

Description:
	General-purpose printf-capturing routines that allow a console-like
	output window for c programs that otherwise prefer to use printf/fprintf.
	This code was "inspired heavily" from sources such as MacDTS'es TESample,
	MacApp's Transcript window, and previous code of mine.  It is fairly well
	self-contained, and works in MPW C 3.2 and Think C 5.0.

	This is the main source file, containing the private definitions and
	code to implement all the needed external and internal support functions.

Related Files:
	Stdio_p2w.h		- generic header for sources that would otherwise use <stdio.h>
	Printf2Window.h	- Mac-specific header for p2w routines
	Printf2Window.c	- the main source for the p2w routines
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.tech@applelink.apple.com
		AppleLink: jl.tech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920318	[esp]	Created.
	920325	[esp]	Added init/Terminate, major redesign of std c fns.
	920327	[esp]	Robustized AddCString code that handles TERec-is-full delete logic
	920329	[esp]	Added AdjustScrollBars call in AddCString to update scrollers as text is added
	920330	[esp]	Updated file header with copyright & related files info
	920401	[esp]	Added p2wSignature to window record for safety checking
	920402	[esp]	Fixed cr/lf bug in bottleneck routine
	920412	[esp]	Fixed potential integer overflow in comparison in AddCString routine
	920412	[esp]	Added most function header comments, added windBounds support in Newp2wWindow, fixed scroller activate bug
	920413	[esp]	Fixed misbehavin' scrollbars hilite upon activate/deactivate
	920521	[esp]	Made the non-resource based code work.
	920529	[esp]	Changed type defs to have trailing _t for ANSI consistency
	920529	[esp]	Added p2w_SetTextFont function to allow easy fontsize switching
	920603	[esp]	Initialized newly alloc'ed ctrls to VISIBLE
	920816	[esp]	Added p2w_SelectAll routine
	920901	[esp]	reduced kMaxTELength to 32000 so TeachText doesn't choke on >32000 sized files!
	920905	[esp]	Fixed scrollbars so they're inactive on window creation.
	920912	[esp]	Added windID parm to NewWindow call
	921011	[esp]	Updated bottleneck to handle fopen() file I/O too (pass-through)
	921128	[esp]	Fixed bug in p2w_fputs: the newline went to stdout, not stream!
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
	931119	[djh]	2.0.1 conditionally compiles for PPC machines, keyword __powerc
==============================================================================
*/

/* Standard C library headers */
#include <stdarg.h>		// ANSI C variable length argument support
#include <string.h>		// strlen

/* Macintosh-specific headers */
#include <Types.h>
#include <Controls.h>
#include <Dialogs.h>
#include <Files.h>
#include <Memory.h>
#include <OSUtils.h>
#include <Resources.h>
#include <Windows.h>
#include <Scrap.h>

#if !defined(THINK_C)
#include <strings.h>		/*p2cstr*/
#endif // THINK_C

#include "screenUtils.h"	/* GetMaxGrowRect */
#include "printf2window.h"	/* our defs AND stdio.h for sprintf, etc. */

#if defined(__powerc)
extern QDGlobals qd;
extern RoutineDescriptor gVScrollRD;
extern RoutineDescriptor gHScrollRD;
#endif

// ==== Constant definitions

// Comment USE_P2W_RESOURCES out to create the controls from scratch..
// Leave it in to create them from resources

// #define	USE_P2W_RESOURCES	true


// Resource IDs of supporting p2w resources

#if defined(USE_P2W_RESOURCES)
#define	kp2w_VScrollID		9601
#define	kp2w_HScrollID		9602
#endif // USE_P2W_RESOURCES


// Magic p2w window record signature value

#define	kp2wWindowSignature	'p2w '


// kTextMargin is the number of pixels we leave blank at the edge of the window.

#define kTextMargin				2


// kControlInvisible is used to 'turn off' controls (i.e., cause the control not
// to be redrawn as a result of some Control Manager call such as SetCtlValue)
// by being put into the contrlVis field of the record. kControlVisible is used
// the same way to 'turn on' the control.

#define kControlInvisible		0
#define kControlVisible			0xFF


// kControlHiliteActive is used to activate controls (contrlHilite)
// kControlHiliteInactive is used to deactivate controls (contrlHilite)

#define kControlHiliteActive		0
#define kControlHiliteInactive		0xFF


// kButtonScroll is how many pixels to scroll horizontally when the button part
// of the horizontal scrollbar is pressed.

#define kButtonScroll			4

// kScrollbarAdjust and kScrollbarWidth are used in calculating
// values for control positioning and sizing.

#define kScrollbarWidth			16
#define kScrollbarAdjust		(kScrollbarWidth - 1)


// kScrollTweek compensates for off-by-one requirements of the scrollbars
// to have borders coincide with the growbox.

#define kScrollTweek			2


// kMaxTELength is an arbitrary number used to limit the length of text in the TERec
// so that various errors won't occur from too many characters in the text.  When the
// string to be added exceeds kMaxTELength, then strlen+kTEDeleteChunkSize
// characters will be deleted from the beginning of the TERec.  This lets the TE
// buffer stay pretty full while characters "fall off the top" of the buffer, and
// insures that the performance delay of deleting from the front of the buffer
// only happens every once in awhile, instead of for every single additional
// string added.  kTEDeleteChunkSize should probably be between 1k and 20k..

#define	kMaxTELength			32000	// 32000 is pretty close to 32767, OK TeachText?
#define	kTEDeleteChunkSize		5000	// should be between 1000 and 20000


// kMinDocSize is the smallest horiz/vert size of a grown window.

#define	kMinDocSize				100


// kMaxStdIOBuffSize is the largest supported size of a single formatted
// stdio C string.  Note that internally, a relocatable buffer of this size
// will be allocated.

#define	kMaxStdIOBuffSize		1024


// internal macro definitions

// Define some structure accessing macros for efficiency.
#define GET_HIWORD(aLong)	(((aLong)>>16)&0x0FFFFL)
#define GET_LOWORD(aLong)	((aLong)&0x0FFFFL)
#define GET_TOPLEFT_POINT(aRect)	(*(Point*)&(aRect).top)
#define GET_BOTRIGHT_POINT(aRect)	(*(Point*)&(aRect).bottom)
#define GET_RECT_WIDTH(aRect)		((aRect).right - (aRect).left)
#define GET_RECT_HEIGHT(aRect)		((aRect).bottom - (aRect).top)
#define IS_P2W_WINDOW(p2wPtr) (((p2w_WindowPtr_t)p2wPtr)->p2wSignature == kp2wWindowSignature)

/*
----------------------------------
prototypes for internal routines
----------------------------------
*/

static void p2wi_GetTERect(const p2w_WindowPtr_t thep2wWindow, Rect *teRect);
static void p2wi_AdjustTE(const p2w_WindowPtr_t thep2wWindow);
static void p2wi_AdjustViewRect(TEHandle p2wTE);
static void p2wi_CommonAdjustScroller(const Boolean isVert, const p2w_WindowPtr_t thep2wWindow, ControlHandle control, TEHandle theTEHandle, const Boolean canRedraw);
static void p2wi_AdjustScrollValues(const p2w_WindowPtr_t thep2wWindow, const Boolean doRedraw);
static void p2wi_AdjustScrollSizes(const p2w_WindowPtr_t thep2wWindow);
static void p2wi_AdjustScrollbars(const p2w_WindowPtr_t thep2wWindow, const Boolean doResize);
static void p2wi_ResizeWindow(const p2w_WindowPtr_t thep2wWindow);
static void p2wi_GetLocalUpdateRegion(const p2w_WindowPtr_t thep2wWindow, RgnHandle localRgn);
static void p2wi_CommonScrollAction(ControlHandle control, short *amount);
pascal void p2wi_VScrollActionProc(ControlHandle control, short part);
pascal void p2wi_HScrollActionProc(ControlHandle control, short part);
static int p2wi_StdCOut_BottleNeck(FILE *stream);
static int p2wi_vfprintf(FILE *stream, const char *format, va_list va_args);



/*
----------------------------------
global variable definitions
----------------------------------
*/

static Handle			p2w_Stdio_OutBuf_Hdl = NULL;
static p2w_WindowPtr_t	local_p2wWindow = NULL;



/*---------------------------------------------------------------------*/
/*==== Main interface routines ====*/


/*
******************************************************************************
Name:
	p2w_Init
------------------------------------------------------------------------------
Purpose:
	Primary (one-time) initialization of the p2w routines
------------------------------------------------------------------------------
Description:
	pre-allocates internal storage & initializes variables
------------------------------------------------------------------------------
Parameters:
	void
------------------------------------------------------------------------------
When Used:
	Called once, when the application starts up
******************************************************************************
*/
OSErr p2w_Init(void)
{
	OSErr	anError = noErr;

	// allocate an output character stream buffer to use
	p2w_Stdio_OutBuf_Hdl = NewHandle(kMaxStdIOBuffSize);
	anError = MemError();
	local_p2wWindow = NULL;

	return anError;
} // p2w_Init



/*
******************************************************************************
Name:
	p2w_Terminate
------------------------------------------------------------------------------
Purpose:
	Primary (one-time) destruction of the p2w routines
------------------------------------------------------------------------------
Description:
	de-allocates internal storage & invalidates variables
------------------------------------------------------------------------------
Parameters:
	void
------------------------------------------------------------------------------
When Used:
	Called once, when the application is shutting down
******************************************************************************
*/
OSErr p2w_Terminate(void)
{
	OSErr	anError = noErr;

	if (p2w_Stdio_OutBuf_Hdl)
	{
		DisposeHandle(p2w_Stdio_OutBuf_Hdl);
		anError = MemError();
	}
	p2w_Stdio_OutBuf_Hdl = NULL;
	local_p2wWindow = NULL;

	return anError;
} // p2w_Terminate



/*
******************************************************************************
Name:
	p2w_NewWindow
------------------------------------------------------------------------------
Purpose:
	Creates a new p2w window structure for use by other p2w routines.
------------------------------------------------------------------------------
Description:
	Creates a p2w window (either from resources or from parameters),
	initializes it, and shows it if asked.
------------------------------------------------------------------------------
Parameters:
	windID			if 0, create window from scratch, if > 0, use as ID for GetNewWindow
	windBoundsPtr	Where to place the new window on the screen.
					If NULL, the resource bounds will be used.
	windTitle		The Pascal title string to use for the new window.
					If NULL, resource title will be used.
	windIsVisible	TRUE will show window after creation, FALSE will leave hidden.
	windFont		Text Font to use for new window.
	windFontSize	Text Font Size tose for new window.
	anError			Returns any error code, or zero (noErr) if all went OK.
------------------------------------------------------------------------------
When Used:
	Called when a new p2w window needs to be allocated.  It may or may not
	be shown at this time.  It will be opened in the back.  This call mainly 
	creates the window structures.
******************************************************************************
*/
p2w_WindowPtr_t p2w_NewWindow(	const	short		windID,
								const	Rect		*windBoundsPtr,
								const	Str255		windTitle,
								const	Boolean		windIsVisible,
								const	int			windFont,
								const	short		windFontSize,
										OSErr		*anError)
{
	WindowPtr		aWindow;
	Rect			destRect, viewRect, screenRect;
	p2w_WindowPtr_t	p2wWPtr = NULL;
	GrafPtr			savedPort;

	GetPort(&savedPort);	// I'll be back..

	/* how big is the screen */
	screenRect = qd.screenBits.bounds;

	/* allocate space for our extended p2window record */
	p2wWPtr = (p2w_WindowPtr_t)NewPtr(sizeof(p2w_WindowRecord_t));
	*anError = MemError();
	if (!*anError)
	{
		// initialize the p2w record

		p2wWPtr->p2wSignature		= kp2wWindowSignature; // bless this special window record
		p2wWPtr->p2wOpenedOK		= false; // until actually opened later
		p2wWPtr->p2wTEHandle		= NULL;
		p2wWPtr->p2wVScroller		= NULL;
		p2wWPtr->p2wHScroller		= NULL;
		p2wWPtr->p2wClickHandler	= NULL;
		p2wWPtr->p2wMaxDocWidth		= GET_RECT_WIDTH(screenRect) - kScrollbarWidth - 2*kTextMargin - 10;
		p2wWPtr->p2wAlwaysScrollToBottom	= true;

		/* now create the Window */
		if (windID > 0)
		{ // get from resource
			aWindow = GetNewWindow(windID, p2wWPtr, (WindowPtr)NULL);
			*anError = ResError();
		}
		else
		{ // Create from scratch, also moves/sizes appropriately
			aWindow = NewWindow(p2wWPtr, windBoundsPtr, windTitle, windIsVisible, documentProc,
								(WindowPtr)NULL, false/*hasGoAway*/, 0/*refcon*/);
			*anError = MemError();
		}
	}

	/* Move the window to user-specified position? */
	/* Do this only if getting from resources - already done (above) otherwise */
	if (windID > 0)
	{
		if ((!*anError) && (windBoundsPtr != NULL))
		{
			MoveWindow(aWindow, (*windBoundsPtr).left, (*windBoundsPtr).top, false);
			SizeWindow(aWindow, GET_RECT_WIDTH(*windBoundsPtr), GET_RECT_HEIGHT(*windBoundsPtr), false);
		}
	}

	if (!*anError)
	{
		// now its OK to call CloseWindow later..
		p2wWPtr->p2wOpenedOK = true;

		SetPort(aWindow);

		/* Set its title to what the user passed in, if non-null */
		/* Do this only if getting from resources - already done (above) otherwise */
		if (windID > 0)
			if (windTitle)							// not a NULL pointer?
				if (*windTitle)						// not an empty string?
					SetWTitle(aWindow, windTitle);	// then do it!

		/* set up the font in the port */
		p2w_SetTextFont(p2wWPtr, windFont, windFontSize);

		/* set up the EditText buffer & area */
		p2wi_GetTERect(p2wWPtr, &viewRect);
		destRect = viewRect;
		destRect.right = destRect.left + p2wWPtr->p2wMaxDocWidth;
		p2wWPtr->p2wTEHandle = TENew(&destRect, &viewRect);
		*anError = MemError();
	}

	if (!*anError)
	{
		p2wi_AdjustViewRect(p2wWPtr->p2wTEHandle);
		// turn on auto-scrolling
//		TEAutoView(true, p2wWPtr->p2wTEHandle); -- later, when we auto-connect to scrollbars
	}

	/* Create vertical scrollbar */
	if (!*anError)
	{
#if defined(USE_P2W_RESOURCES)
		p2wWPtr->p2wVScroller = GetNewControl(kp2w_VScrollID, aWindow);
		*anError = ResError();
#else
		p2wWPtr->p2wVScroller = NewControl(aWindow, &(*aWindow).portRect, "\pp2w_Vert", false, 0, 0, 1, scrollBarProc, 0);
		*anError = MemError();
#endif // USE_P2W_RESOURCES
		// Our window comes up in back, so turn OFF visibility of scrollbar initially
		if (!*anError)
			HideControl(p2wWPtr->p2wVScroller);
	}

	/* Create horizontal scrollbar */
	if (!*anError)
	{
#if defined(USE_P2W_RESOURCES)
		p2wWPtr->p2wHScroller = GetNewControl(kp2w_HScrollID, aWindow);
		*anError = ResError();
#else
		p2wWPtr->p2wHScroller = NewControl(aWindow, &(*aWindow).portRect, "\pp2w_Horz", false, 0, 0, 1, scrollBarProc, 0);
		*anError = MemError();
#endif // USE_P2W_RESOURCES
		// Our window comes up in back, so turn OFF visibility of scrollbar initially
		if (!*anError)
			HideControl(p2wWPtr->p2wHScroller);
	}

	// adjust & draw the controls, draw the window
	if (!*anError)
	{
		p2wi_AdjustScrollbars(p2wWPtr, true/*kResize*/);
		if (windIsVisible)
			ShowWindow(aWindow);
	}

	// Return to port after storm
	SetPort(savedPort);	// I'm back..

	// close & dispose if any errors happened!
	if (*anError)
	{
		p2w_DisposeWindow(p2wWPtr);
		// if errors happened, don't give user any pointers
		p2wWPtr = NULL;
	}

	// remember this for the StdIO routines later..
	local_p2wWindow = p2wWPtr;

	return (p2wWPtr);

} // p2w_NewWindow



/*
******************************************************************************
Name:
	p2w_DisposeWindow
------------------------------------------------------------------------------
Purpose:
	Deletes an existing p2w window structure and its related TE danglies.
------------------------------------------------------------------------------
Description:
	Disposes of the TE record, closes the window/grafport, and disposes
	of the window/grafport itself.
------------------------------------------------------------------------------
Parameters:
	the_p2wPtr		The p2w window to dispose.
------------------------------------------------------------------------------
When Used:
	Called when a p2w window is no longer needed.
******************************************************************************
*/
void p2w_DisposeWindow(p2w_WindowPtr_t the_p2wPtr)
{
	// Is the window pointer not null?
	if (the_p2wPtr)
		// is this really one of our p2w windows?
		// (We don't want to dispose extra stuff if it's really
		// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
		if (IS_P2W_WINDOW(the_p2wPtr))
		{
			// Dispose Window/GrafPort danglies if opened ok
			if (the_p2wPtr->p2wOpenedOK)
				CloseWindow((WindowPtr)the_p2wPtr);
			// Dispose of our TE Record
			if (the_p2wPtr->p2wTEHandle)
				TEDispose(the_p2wPtr->p2wTEHandle);
			// set fields back to nil, to help catch anyone touching them after dispose!
			the_p2wPtr->p2wOpenedOK		= false;
			the_p2wPtr->p2wTEHandle		= NULL;
			the_p2wPtr->p2wVScroller	= NULL;
			the_p2wPtr->p2wHScroller	= NULL;
			the_p2wPtr->p2wClickHandler	= NULL;
			// Dispose our version of window record itself now
			DisposePtr((Ptr)the_p2wPtr);
			local_p2wWindow = NULL;
		}
} // p2w_DisposeWindow


/*
******************************************************************************
Name:
	p2w_SetTextFont
------------------------------------------------------------------------------
Purpose:
	Changes the Font Type and Size in the p2w window.
------------------------------------------------------------------------------
Description:
------------------------------------------------------------------------------
Parameters:
	the_p2wPtr		The p2w window to update.
	newFontType		The Font type to change to (monaco, courier, times, etc.)
	newFontSize		The Font type to change to (9,10,12,etc.)
------------------------------------------------------------------------------
When Used:
	Called when text is to be added to the p2w window.
******************************************************************************
*/
void p2w_SetTextFont
					(		p2w_WindowPtr_t		the_p2wPtr,
							short				newFontType,
							short				newFontSize)
{
	// is this really one of our p2w windows?
	// (We don't want to fiddle with extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(the_p2wPtr))
	{
		// Tell our grafport that we have a new font
		SetPort((GrafPtr)the_p2wPtr);
		TextFont(newFontType);
		TextSize(newFontSize);

		// Tell our TextEdit record that we have a new font..
		if (the_p2wPtr->p2wTEHandle != NULL)
		{	// TE Record is valid
			(**the_p2wPtr->p2wTEHandle).txFont = newFontType;
			(**the_p2wPtr->p2wTEHandle).txSize = newFontSize;
			// let the TE adjust to this new font!
			TECalText(the_p2wPtr->p2wTEHandle);
			TEUpdate(&the_p2wPtr->p2wWindowRec.port.portRect,the_p2wPtr->p2wTEHandle);
		}
	}
} // p2w_SetTextFont


/*
******************************************************************************
Name:
	p2w_AddCString
------------------------------------------------------------------------------
Purpose:
	Adds the text in the C-style string passed to the p2w window.
------------------------------------------------------------------------------
Description:
------------------------------------------------------------------------------
Parameters:
	the_p2wPtr		The p2w window to add to.
	theCStrPtr		The string of text to add.
	p2w_AddCString	function returns zero if ok, or an error #.
------------------------------------------------------------------------------
When Used:
	Called when text is to be added to the p2w window.
******************************************************************************
*/
OSErr p2w_AddCString(p2w_WindowPtr_t the_p2wPtr, const char * theCStrPtr)
{
	OSErr	anError = noErr;
	long	theStrLen;

	if (the_p2wPtr && theCStrPtr)		// if window and string ptrs are valid
		// is this really one of our p2w windows?
		// (We don't want to fiddle with extra stuff if it's really
		// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
		if (IS_P2W_WINDOW(the_p2wPtr))
			if (the_p2wPtr->p2wTEHandle)
			{	// TE Record is valid
				theStrLen = strlen(theCStrPtr);
				if (theStrLen > 0)	// if string is not empty, add it
				{
					// If TE is too full, need to delete some at beginning before adding
					// (this looks weird, but it is the Mathematically Correct way to
					// compare numbers close to the edge of 32767-land.. note that
					// teLength+theStrLen could overflow, so we subtract instead.)
					if ((**(the_p2wPtr->p2wTEHandle)).teLength > (kMaxTELength - theStrLen))
					{
						// Delete some text at beginning of TE
						TESetSelect(0, theStrLen+kTEDeleteChunkSize, the_p2wPtr->p2wTEHandle);
						TEDelete(the_p2wPtr->p2wTEHandle);
					}
					// go to very end of TE (-1 is really +65535 in TE Worlds!)
					TESetSelect(-1, -1, the_p2wPtr->p2wTEHandle);
					// stick it in there
					TEInsert(theCStrPtr, theStrLen, the_p2wPtr->p2wTEHandle);
					// re-draw the scrollbars, since they may have changed
					p2wi_AdjustScrollbars(the_p2wPtr, false);
				}	// theStrLen > 0
			}	// TE Record is valid
	return(anError);
} // p2w_AddCString


/*
******************************************************************************
Name:
	p2w_DrawWindow
------------------------------------------------------------------------------
Purpose:
	Draw the contents of an application window.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_DrawWindow(const p2w_WindowPtr_t the_p2wPtr)
{
	// is this really one of our p2w windows?
	// (We don't want to dispose extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(the_p2wPtr))
	{
		// move in for the kill
		SetPort((WindowPtr)the_p2wPtr);
		// kill off old bits
		EraseRect(&((WindowPtr)the_p2wPtr)->portRect);
		// regain controls
		DrawGrowIcon((WindowPtr)the_p2wPtr);
		DrawControls((WindowPtr)the_p2wPtr);
		// show them our text
		TEUpdate(&((WindowPtr)the_p2wPtr)->portRect, the_p2wPtr->p2wTEHandle);
	}
} // p2w_DrawWindow



/*
******************************************************************************
Name:
	p2w_DoUpdate
------------------------------------------------------------------------------
Purpose:
	Update (redraw) the contents of an application window.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_DoUpdate(const p2w_WindowPtr_t thep2wWindow)
{
	// move in for the kill
	SetPort((WindowPtr)thep2wWindow);
	BeginUpdate((WindowPtr)thep2wWindow);				/* this sets up the visRgn */
	if (!EmptyRgn(((WindowPtr)thep2wWindow)->visRgn))	/* draw if updating needs to be done */
		p2w_DrawWindow(thep2wWindow);
	EndUpdate((WindowPtr)thep2wWindow);
} // p2w_DoUpdate


/*
******************************************************************************
Name:
	p2w_DoActivate
------------------------------------------------------------------------------
Purpose:
	This is called when a window is activated or deactivated.
	It calls TextEdit to handle the selection.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_DoActivate(const p2w_WindowPtr_t thep2wWindow, Boolean becomingActive)
{
	RgnHandle	tempRgn, clipRgn;

	// is this really one of our p2w windows?
	// (We don't want to dispose extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(thep2wWindow))
	{
		/* move in for the kill */
		SetPort((WindowPtr)thep2wWindow);

		/* force window to be redrawn on update */
		InvalRect(&((WindowPtr)thep2wWindow)->portRect);

		if (becomingActive)
		{
			/*
			Since we donÕt want TEActivate to draw a selection
			in an area where weÕre going to erase and redraw,
			weÕll clip out the update region before calling it.
			*/
			tempRgn = NewRgn();
			clipRgn = NewRgn();
			if (tempRgn && clipRgn)
			{
				p2wi_GetLocalUpdateRegion(thep2wWindow, tempRgn);	/* get localized update region */
				GetClip(clipRgn);
				DiffRgn(clipRgn, tempRgn, tempRgn);		/* subtract updateRgn from clipRgn */
				SetClip(tempRgn);
				TEActivate(thep2wWindow->p2wTEHandle);
				SetClip(clipRgn);						/* restore the full-blown clipRgn */
				DisposeRgn(tempRgn);
				DisposeRgn(clipRgn);
			}
	
			/* the controls must be redrawn on activation */
			ShowControl(thep2wWindow->p2wVScroller);
			ShowControl(thep2wWindow->p2wHScroller);
		}
		else
		{	/* De-Activating.. */
			TEDeactivate(thep2wWindow->p2wTEHandle);
	
			/* the controls must be hidden on deactivation */
			HideControl(thep2wWindow->p2wVScroller);
			HideControl(thep2wWindow->p2wHScroller);

			/* the growbox should be changed immediately here */
			DrawGrowIcon((WindowPtr)thep2wWindow);
	
		}
	}
} // p2w_DoActivate


/*
******************************************************************************
Name:
	p2w_DoGrow
------------------------------------------------------------------------------
Purpose:
	Called when a mouseDown occurs in the grow box of an active
	window. In order to eliminate any 'flicker', we want to
	invalidate only what is necessary. Since p2w_ResizeWindow
	invalidates the whole portRect, we save the old TE
	viewRect, intersect it with the new TE viewRect, and remove
	the result from the update region. However, we must make
	sure that any old update region that might have been around
	gets put back. 
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_DoGrow(const p2w_WindowPtr_t thep2wWindow, EventRecord *theEvent)
{
	long		growResult;
	Rect		tempRect;
//	RgnHandle	tempRgn;
	
	// is this really one of our p2w windows?
	// (We don't want to dispose extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(thep2wWindow))
	{
		// move in for the kill
		SetPort((WindowPtr)thep2wWindow);
	
		/* set up limiting values */
		GetMaxGrowRect((WindowPtr)thep2wWindow, &tempRect);	
		tempRect.left = kMinDocSize;
		tempRect.top = kMinDocSize;
	
		growResult = GrowWindow((WindowPtr)thep2wWindow, theEvent->where, &tempRect);
		/* see if it really changed size */
		if (growResult != 0)
		{
//			tempRect = (**thep2wWindow->p2wTEHandle).viewRect;	/* save old text box */
//			tempRgn = NewRgn();
//			p2wi_GetLocalUpdateRegion(thep2wWindow, tempRgn);		/* get localized update region */
			SizeWindow((WindowPtr)thep2wWindow, GET_LOWORD(growResult), GET_HIWORD(growResult), true);
			p2wi_ResizeWindow(thep2wWindow);
	
			/* calculate & validate the region that hasnÕt changed, so it wonÕt get redrawn */
//			SectRect(&tempRect, &(**thep2wWindow->p2wTEHandle).viewRect, &tempRect);
//			ValidRect(&tempRect);							/* take it out of update */
//			InvalRgn(tempRgn);								/* put back any prior update */
//			DisposeRgn(tempRgn);
		}
	}
} // p2w_DoGrow


/*
******************************************************************************
Name:
	p2w_DoZoom
------------------------------------------------------------------------------
Purpose:
	Called when a mouseClick occurs in the zoom box of an active
	window. Everything has to get re-drawn here, so we don't mind
	that p2w_ResizeWindow invalidates the whole portRect. 
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_DoZoom(const p2w_WindowPtr_t thep2wWindow, short thePart)
{
	// is this really one of our p2w windows?
	// (We don't want to dispose extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(thep2wWindow))
	{
		// move in for the kill
		SetPort((WindowPtr)thep2wWindow);
		EraseRect(&((WindowPtr)thep2wWindow)->portRect);
		ZoomWindow((WindowPtr)thep2wWindow, thePart, (WindowPtr)thep2wWindow==FrontWindow());
		p2wi_ResizeWindow(thep2wWindow);
	}
} // p2w_DoZoom


/*
******************************************************************************
Name:
	p2w_SelectAll
------------------------------------------------------------------------------
Purpose:
	This will select the entire contents of a window.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_SelectAll(const p2w_WindowPtr_t thep2wWindow)
{
	// is this really one of our p2w windows?
	// (We don't want to dispose extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(thep2wWindow))
	{
		TESetSelect(0, -1, thep2wWindow->p2wTEHandle);
	}
} // p2w_SelectAll

/*
******************************************************************************
Name:
	p2w_DoContentClick
------------------------------------------------------------------------------
Purpose:
	This is called when a mouseDown occurs in the content of a window.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
void p2w_DoContentClick(const p2w_WindowPtr_t thep2wWindow, EventRecord *theEvent)
{
	Point		mouse;
	ControlHandle control;
	short		part, value;
	Boolean		shiftDown;
	Rect		teRect;

	// is this really one of our p2w windows?
	// (We don't want to dispose extra stuff if it's really
	// a normal [sheep] WindowPtr passed in as a [wolf] p2w_WindowPtr_t!)
	if (IS_P2W_WINDOW(thep2wWindow))
	{
		SetPort((WindowPtr)thep2wWindow);
	
		/* get the click position */
		mouse = theEvent->where;
		GlobalToLocal(&mouse);
	
		/* see if we are in the viewRect. if so, we wonÕt check the controls */
		p2wi_GetTERect(thep2wWindow, &teRect);
		if (PtInRect(mouse, &teRect))
		{
			/* see if we need to extend the selection */
			shiftDown = (theEvent->modifiers & shiftKey) != 0;	/* extend if Shift is down */
			TEClick(mouse, shiftDown, thep2wWindow->p2wTEHandle);
		}
		else
		{
			part = FindControl(mouse, (WindowPtr)thep2wWindow, &control);
			switch (part)
			{
				case 0:		/* do nothing.. */
					break;
				case inThumb:
					value = GetCtlValue(control);
					part = TrackControl(control, mouse, nil);
					if (part != 0)
					{
						value -= GetCtlValue(control);
						/* value now has CHANGE in value; if value changed, scroll */
						if (value != 0)
							if (control == thep2wWindow->p2wVScroller)
								TEScroll(0, value * (*thep2wWindow->p2wTEHandle)->lineHeight, thep2wWindow->p2wTEHandle);
							else
								TEScroll(value, 0, thep2wWindow->p2wTEHandle);
					}
					break;
				default:	/* they clicked in an arrow, so track & scroll */
					if (control == thep2wWindow->p2wVScroller)
					{
#if defined(__powerc)
						value = TrackControl(control, mouse, (ControlActionUPP) &gVScrollRD);
#else
						value = TrackControl(control, mouse, (ProcPtr) p2wi_VScrollActionProc);
#endif
					}
					else
					if (control == thep2wWindow->p2wHScroller)
					{
#if defined(__powerc)
						value = TrackControl(control, mouse, (ControlActionUPP) &gHScrollRD);
#else
						value = TrackControl(control, mouse, (ProcPtr) p2wi_HScrollActionProc);
#endif
					}
					break;
			}
		}
	}
} // p2w_DoContentClick



/*---------------------------------------------------------------------*/
/*==== Private internal p2w routines ====*/
	
/*
******************************************************************************
Name:
	p2wi_GetTERect
------------------------------------------------------------------------------
Purpose:
	Return a rectangle that is inset from the portRect by the
	size of the scrollbars, plus a bit.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_GetTERect(const p2w_WindowPtr_t thep2wWindow, Rect *teRect)
{
	*teRect = ((WindowPtr)thep2wWindow)->portRect;
	InsetRect(teRect, kTextMargin, kTextMargin);	/* adjust for margin */
	teRect->bottom = teRect->bottom - 15;		/* and for the scrollbars */
	teRect->right = teRect->right - 15;
} // p2wi_GetTERect


/*
******************************************************************************
Name:
	p2wi_AdjustTE
------------------------------------------------------------------------------
Purpose:
	Scroll the TERec around to match up to the potentially updated scrollbar
	values. This is really useful when the window has been resized such that
	the scrollbars became inactive but the TERec was already scrolled.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_AdjustTE(const p2w_WindowPtr_t thep2wWindow)
{
	short		hVal,vVal;
	TEPtr		te;
	
	vVal = GetCtlValue(thep2wWindow->p2wVScroller);
	hVal = GetCtlValue(thep2wWindow->p2wHScroller);
	te = *thep2wWindow->p2wTEHandle;
	TEScroll((te->viewRect.left - te->destRect.left) - hVal,
			(te->viewRect.top - te->destRect.top) - (vVal * te->lineHeight),
			thep2wWindow->p2wTEHandle);
} // p2wi_AdjustTE


/*
******************************************************************************
Name:
	p2wi_AdjustViewRect
------------------------------------------------------------------------------
Purpose:
	Update our TE view rect so it's the greatest multiple of
	the lineHeight that still fits in the old viewRect.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_AdjustViewRect(TEHandle p2wTE)
{
	TEPtr		te;
	
	te = *p2wTE;
	te->viewRect.bottom = (((te->viewRect.bottom - te->viewRect.top) / te->lineHeight)
							* te->lineHeight) + te->viewRect.top;
} // p2wi_AdjustViewRect


/*
******************************************************************************
Name:
	p2wi_CommonAdjustScroller
------------------------------------------------------------------------------
Purpose:
	Calculate the new control maximum value and current value,
	for the horizontal or vertical scrollbar. The vertical max
	is calculated by comparing the number of lines to the
	vertical size of the viewRect. The horizontal max is
	calculated by comparing the maximum document width to the
	width of the viewRect. The current values are set by
	comparing the offset between the view and destination
	rects. If necessary and we canRedraw, have the control be
	re-drawn by calling ShowControl. 
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	Called by p2wi_AdjustScrollValues, twice.
******************************************************************************
*/
static void p2wi_CommonAdjustScroller(const Boolean isVert, const p2w_WindowPtr_t thep2wWindow, ControlHandle control, TEHandle theTEHandle, const Boolean canRedraw)
{
	short		value, lines, max;
	short		oldValue, oldMax;
	
	oldValue = GetCtlValue(control);
	oldMax = GetCtlMax(control);
	if (isVert)
	{
		lines = (**theTEHandle).nLines;
		/* since nLines isnÕt right if the last character is a return, check for that case */
		if ( *(*(**theTEHandle).hText + (**theTEHandle).teLength - 1) == '\n')
			lines += 1;
		max = lines
			-	(
				((**theTEHandle).viewRect.bottom - (**theTEHandle).viewRect.top)
				/ (**theTEHandle).lineHeight
				);
	}
	else
		max = thep2wWindow->p2wMaxDocWidth
			-	(
				(**theTEHandle).viewRect.right - (**theTEHandle).viewRect.left
				);
	
	if (max < 0)
		max = 0;
	SetCtlMax(control, max);
	
	if (isVert)
	{
		// always scroll to end?
		if (thep2wWindow->p2wAlwaysScrollToBottom)
			value = max;
		else
			value = ((**theTEHandle).viewRect.top - (**theTEHandle).destRect.top)
					/ (**theTEHandle).lineHeight;
	}
	else
		value = (**theTEHandle).viewRect.left - (**theTEHandle).destRect.left;
	
	if ( value < 0 )
		value = 0;
	else
		if (value >  max)
			value = max;
	
	SetCtlValue(control, value);

	/* now redraw the control if it needs to be and can be */
	if (canRedraw || (max != oldMax) || (value != oldValue))
	{
		ShowControl(control);
//		Draw1Control(control);
	}
} // p2wi_CommonAdjustScroller


/*
******************************************************************************
Name:
	p2wi_AdjustScrollValues
------------------------------------------------------------------------------
Purpose:
	Simply call the common adjust routine for both the vertical
	and horizontal scrollbars.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_AdjustScrollValues(const p2w_WindowPtr_t thep2wWindow, const Boolean doRedraw)
{
	p2wi_CommonAdjustScroller(true, thep2wWindow, thep2wWindow->p2wVScroller, thep2wWindow->p2wTEHandle, doRedraw);
	p2wi_CommonAdjustScroller(false, thep2wWindow, thep2wWindow->p2wHScroller, thep2wWindow->p2wTEHandle, doRedraw);
} // p2wi_AdjustScrollValues



/*
******************************************************************************
Name:
	p2wi_AdjustScrollSizes
------------------------------------------------------------------------------
Purpose:
	Re-calculate the position and size of the viewRect and the
	scrollbars. The constant kScrollTweek compensates for the
	"off-by-one" requirements of the scrollbars that need their
	borders coinciding with the growbox. 
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_AdjustScrollSizes(const p2w_WindowPtr_t thep2wWindow)
{
	Rect		teRect;

	p2wi_GetTERect(thep2wWindow, &teRect);	// start with TERect..
	(*thep2wWindow->p2wTEHandle)->viewRect = teRect;
	p2wi_AdjustViewRect(thep2wWindow->p2wTEHandle);	/* snap to nearest line */

	/* Vertical */
	MoveControl(thep2wWindow->p2wVScroller,
				((WindowPtr)thep2wWindow)->portRect.right - kScrollbarAdjust,
				-1);
	SizeControl(thep2wWindow->p2wVScroller,
				kScrollbarWidth,
				(((WindowPtr)thep2wWindow)->portRect.bottom
					- ((WindowPtr)thep2wWindow)->portRect.top)
					- (kScrollbarAdjust - kScrollTweek));

	/* Horizontal */
	MoveControl(thep2wWindow->p2wHScroller,
				-1,
				((WindowPtr)thep2wWindow)->portRect.bottom - kScrollbarAdjust);
	SizeControl(thep2wWindow->p2wHScroller,
				(((WindowPtr)thep2wWindow)->portRect.right
					- ((WindowPtr)thep2wWindow)->portRect.left)
					- (kScrollbarAdjust - kScrollTweek),
				kScrollbarWidth);
} // p2wi_AdjustScrollSizes


/*
******************************************************************************
Name:
	p2wi_AdjustScrollbars
------------------------------------------------------------------------------
Purpose:
	Recalculate the scrollbar pos/values, and redraw if need be.
------------------------------------------------------------------------------
Description:
	Turn off the controls by jamming a zero into their contrlVis fields
	(HideControl erases them and we don't want that). If the controls
	are to be resized as well, call the procedure to do that, then call
	the procedure to adjust the maximum and current values. Finally,
	re-enable the controls by jamming a $FF in their contrlVis fields.  
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_AdjustScrollbars(const p2w_WindowPtr_t thep2wWindow, const Boolean doResize)
{
	short	oldHvis, oldVvis;

	oldVvis = (**thep2wWindow->p2wVScroller).contrlVis;
	oldHvis = (**thep2wWindow->p2wHScroller).contrlVis;

	/* First, turn visibility of scrollbars off to eliminate unwanted redrawing */
	(*thep2wWindow->p2wVScroller)->contrlVis = kControlInvisible;	/* turn them off */
	(*thep2wWindow->p2wHScroller)->contrlVis = kControlInvisible;

	/* move & size as needed */
	if (doResize)
		p2wi_AdjustScrollSizes(thep2wWindow);

	/* adjust constrols' max and current values */
	p2wi_AdjustScrollValues(thep2wWindow, doResize);

	/* now scroll the TE record to match the scrollbars */
	p2wi_AdjustTE(thep2wWindow);

	/* Now, restore visibility in case we never had to ShowControl during adjustment */
	(*thep2wWindow->p2wVScroller)->contrlVis = oldVvis;	/* restore them */
	(*thep2wWindow->p2wHScroller)->contrlVis = oldHvis;
} // p2wi_AdjustScrollbars


/*
******************************************************************************
Name:
	p2wi_ResizeWindow
------------------------------------------------------------------------------
Purpose:
	Called when the window has been resized to fix up the controls and content.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_ResizeWindow(const p2w_WindowPtr_t thep2wWindow)
{
	p2wi_AdjustScrollbars(thep2wWindow, true);
	InvalRect(&((WindowPtr)thep2wWindow)->portRect);
} // p2wi_ResizeWindow



/*
******************************************************************************
Name:
	p2wi_GetLocalUpdateRegion
------------------------------------------------------------------------------
Purpose:
	Returns the update region in local coordinates
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static void p2wi_GetLocalUpdateRegion(const p2w_WindowPtr_t thep2wWindow, RgnHandle localRgn)
{
	CopyRgn(((WindowPeek)thep2wWindow)->updateRgn, localRgn);	/* save old update region */
	OffsetRgn(localRgn, ((WindowPtr)thep2wWindow)->portBits.bounds.left,
						((WindowPtr)thep2wWindow)->portBits.bounds.top);
} // p2wi_GetLocalUpdateRegion



/*
******************************************************************************
Name:
	p2wi_CommonScrollAction
------------------------------------------------------------------------------
Purpose:
	Common algorithm for pinning the value of a control. It
	returns the actual amount the value of the control changed.
	Note the pinning is done for the sake of returning the
	amount the control value changed.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	Called by p2wi_HScrollActionProc, p2wi_VScrollActionProc
******************************************************************************
*/
static void p2wi_CommonScrollAction(ControlHandle control, short *amount)
{
	short		value, max;
	
	/* get the current value.. */
	value = GetCtlValue(control);

	/* and the biggest value */
	max = GetCtlMax(control);

	*amount = value - *amount;
	if ( *amount < 0 )
		*amount = 0;
	else if ( *amount > max )
		*amount = max;
	SetCtlValue(control, *amount);

	/* figure out the delta change */
	*amount = value - *amount;
} // p2wi_CommonScrollAction


/*
******************************************************************************
Name:
	p2wi_VScrollActionProc
------------------------------------------------------------------------------
Purpose:
	Determines how much to change the value of the vertical scrollbar by,
	and how much to scroll the TE record.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	Callback routine, called by toolbox
******************************************************************************
*/
pascal void p2wi_VScrollActionProc(ControlHandle control, short part)
{
	short			amount;
	p2w_WindowPtr_t	p2w_Window;
	TEPtr			te;
	
	if (part != 0)
	{	/* if it was actually in the control */
		p2w_Window = (p2w_WindowPtr_t)(*control)->contrlOwner;
		te = *p2w_Window->p2wTEHandle;
		switch (part)
		{
			case inUpButton:
			case inDownButton:		/* one line */
				amount = 1;
				break;
			case inPageUp:			/* one page */
			case inPageDown:
				amount = (te->viewRect.bottom - te->viewRect.top) / te->lineHeight;
				break;
		}
		if ((part == inDownButton) || (part == inPageDown))
			amount = -amount;		/* reverse direction if going down */
		p2wi_CommonScrollAction(control, &amount);
		if (amount != 0)
			TEScroll(0, amount * te->lineHeight, p2w_Window->p2wTEHandle);
	}
} // p2wi_VScrollActionProc


/*
******************************************************************************
Name:
	p2wi_HScrollActionProc
------------------------------------------------------------------------------
Purpose:
	Determines how much to change the value of the horizontal scrollbar by,
	and how much to scroll the TE record.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	Callback routine, called by toolbox
******************************************************************************
*/
pascal void p2wi_HScrollActionProc(ControlHandle control, short part)
{
	short			amount;
	p2w_WindowPtr_t	p2w_Window;
	TEPtr			te;
	
	if (part != 0)
	{	/* if it was actually in the control */
		p2w_Window = (p2w_WindowPtr_t)(*control)->contrlOwner;
		te = *p2w_Window->p2wTEHandle;
		switch (part)
		{
			case inUpButton:
			case inDownButton:		/* some pixels */
				amount = kButtonScroll;
				break;
			case inPageUp:			/* a whole page */
			case inPageDown:
				amount = te->viewRect.right - te->viewRect.left;
				break;
		}
		if ( (part == inDownButton) || (part == inPageDown) )
			amount = -amount;		/* reverse direction.. */
		p2wi_CommonScrollAction(control, &amount);
		if ( amount != 0 )
			TEScroll(amount, 0, p2w_Window->p2wTEHandle);
	}
} // p2wi_HScrollActionProc


/*---------------------------------------------------------------------*/
/*==== Standard C library fns ====*/

/*
******************************************************************************
Name:
	p2wi_StdCOut_BottleNeck
------------------------------------------------------------------------------
Purpose:
	Internal Standard C output replacement common bottleneck routine..
	This is what they all call to actually display the string/character.
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static int p2wi_StdCOut_BottleNeck(FILE *stream)
{
	int		x;
	char	*OutBuf, *outp;

	// get ahold of output buffer
	OutBuf = *p2w_Stdio_OutBuf_Hdl;
	// make sure the string at least stops here! (maybe check strlen first for overflow errors?)
	*(OutBuf+kMaxStdIOBuffSize-1) = '\0';

	// Replace any line feeds (0x0a) with Macintosh carriage returns (0x0d)
	// This is mainly for Think C compatibility, since Think faithfully
	// literally adopted the K&R C idea (from Unix) that '\n' is really 0x0a,
	// and Apple's MPW C bends '\n' to 0x0d for better file compatibility.

/* Note: For now, do this in both MPW and Think C.  This will FORCE the mapping
of any LFs to CRs.  The main reason for doing this is to catch the odd times
that the source code uses either '\r', or worse, '\0x0a' to embed returns! */
/*		if ('\n' != 0x0d) -- commented out to force issue, per above comment */
	{
		outp = OutBuf;
		while (*outp)
		{
			if (*outp == 0x0a)
			  *outp = 0x0d;
			outp++;
		}
	}

	// If the output is for standard output (stderr/stdout) then capture it into our window,
	// else send it on to the standard C file output library routine.
	if ((stream == stderr) || (stream == stdout))
	{
		x = p2w_AddCString(local_p2wWindow, OutBuf);	// add it into window
	}
	else
	{
		// call a real std. C library routine.  This allows output to real
		// stdio C files to pass through unmolested.
		x = fwrite(OutBuf, strlen(OutBuf), 1, stream);
	}

	return x;

} // p2wi_StdCOut_BottleNeck


/*
******************************************************************************
Name:
	p2wi_vfprintf
------------------------------------------------------------------------------
Purpose:
	Internal common formatting handler for fprintf/printf
------------------------------------------------------------------------------
Description:
	
------------------------------------------------------------------------------
Parameters:
	
------------------------------------------------------------------------------
When Used:
	
******************************************************************************
*/
static int p2wi_vfprintf(FILE *stream, const char *format, va_list va_args)
{
	int		x;
	char	*outBuf;

	// get ahold of output buffer
	HLock(p2w_Stdio_OutBuf_Hdl);
	outBuf = *p2w_Stdio_OutBuf_Hdl;

#if defined(NEEDS_DEBUG)
{
	// stick in a TE-length value debug into the TE record
	int daLen = (**(local_p2wWindow->p2wTEHandle)).teLength;
	sprintf(outBuf, "[%5d] ",daLen);
	x = p2wi_StdCOut_BottleNeck(stream);
}
#endif // NEEDS_DEBUG

	// use C library routine to do printf formatting of the string into buffer
	x = vsprintf(outBuf, format, va_args); // format it into a string

	// do the window output
	x = p2wi_StdCOut_BottleNeck(stream);

	// cut buffer adrift again..
	HUnlock(p2w_Stdio_OutBuf_Hdl);

	return x;
} // p2wi_vfprintf



int p2w_fflush(FILE *stream)
{
#pragma unused (stream)
	int		x;

	/* major no-op, dude */
	x = 0;
	return x;
} // p2w_fflush




int p2w_fprintf(FILE *stream, const char *format, ...)
{
	va_list	va_args;
	int		x;

	// use our bottleneck routine to do printf formatting
	va_start(va_args, format);
	x = p2wi_vfprintf(stream, format, va_args); // format it into a string
	va_end(va_args);

	return x;
} // p2w_fprintf


int p2w_fputc(int theChar, FILE *stream)
{
	int		x;
	char	*outBuf;

	// get ahold of output buffer & make char into string
	HLock(p2w_Stdio_OutBuf_Hdl);
	outBuf = *p2w_Stdio_OutBuf_Hdl;

	outBuf[0] = theChar;
	outBuf[1] = '\0';

	x = p2wi_StdCOut_BottleNeck(stream);

	HUnlock(p2w_Stdio_OutBuf_Hdl);

	return x;

} // p2w_fputc


int p2w_fputs(const char *theString, FILE *stream)
{
	int		x;
	char	*outBuf;

	// get ahold of output buffer
	HLock(p2w_Stdio_OutBuf_Hdl);
	outBuf = *p2w_Stdio_OutBuf_Hdl;

	// Copy string into buffer & write it
	BlockMove(theString, outBuf, kMaxStdIOBuffSize);
	x = p2wi_StdCOut_BottleNeck(stream);

	// "puts()" puts a newline afterwards..
	if ((stream == stderr) || (stream == stdout))
		p2w_putc('\n', stream);

	// cut buffer adrift again..
	HUnlock(p2w_Stdio_OutBuf_Hdl);

	return x;
} // p2w_fputs


int p2w_printf(const char * format, ...)
{
	va_list	va_args;
	int		x;

	// use our bottleneck routine to do printf formatting
	va_start(va_args, format);
	x = p2wi_vfprintf(stdout, format, va_args); // format it into a string
	va_end(va_args);

	return x;
} // p2w_printf


int p2w_putc(int theChar, FILE *stream)
{
	return p2w_fputc(theChar, stream);
} // p2w_putc


int p2w_putchar(const char theChar)
{
	return p2w_fputc(theChar, stdout);
} // p2w_putchar


int p2w_puts(const char *theString)
{
	return p2w_fputs(theString, stdout);
} // p2w_puts

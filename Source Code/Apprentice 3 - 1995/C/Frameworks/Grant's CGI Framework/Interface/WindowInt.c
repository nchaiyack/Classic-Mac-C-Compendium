/*****
 *
 *	WindowInt.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright й1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "globals.h"

#include "AboutBox.h"
#include "DebugUtil.h"
#include "MemoryUtil.h"
#include "MenuFunc.h"

#include "WindowInt.h"


/***  LOCAL PROTOTYPES ***/

	Boolean		windowValid				( WindowPtr );
	Boolean		windowHasScrollbars		( WindowPtr );
	void		invalidateScrollbars	( WindowPtr );


/***  FUNCTIONS  ***/

/* MENU SELECTION HANDLING */

/*  */
void
WindowMenuClose ( short modifiers )
{
	WindowClose ( nil, modifiers, true );
} /* WindowMenuClose */


/* EVENT HANDLING */

/* I think that state should be true if it is activating, false if deactivating.
	IM-MTE: 2-53,54 */
void
WindowActivate ( WindowPtr theWindow, Boolean activate, const EventRecord *theEvent )
{
	window_type		windType;
	
	if ( activate )
	{
		/* show scroll bars if necessary */
		
		/* restore window's selections */
		
		adjustMenus ();
		
		/* If you have windows with the grow box in the bottom right corner,
			you should invalidate their growrect here */
	}
	else
	{
		/* hide scroll bars if necessary */
		
		/* hide, or reduce to outline, window's selections */
		
		windType = WindowType ( theWindow );
		
		switch ( windType )
		{
			case Window_dlgModal :
			case Window_dlgMoveableModal :
			case Window_about :
			case Window_DA :
			case Window_UNKNOWN :
			default :
				break;
		}
	}
} /* WindowActivate */


/* MOUSE CLICK HANDLING */

/* see the control manager chapter in IM-MTE for an example DoContentClick procedure
	was doInContent */
void 
WindowContentClick ( WindowPtr theWindow, const EventRecord *eventPtr)
{
	Point			whereClicked;
	
	whereClicked = eventPtr->where;
	
	GlobalToLocal ( &whereClicked );
		
	/* still nothing to do here yet */

} /* WindowContentClick */


/* was doZoomBox */
void
WindowZoomBox ( WindowPtr theWindow, short partCode )
{
	GrafPtr	savePort;
	
	GetPort ( &savePort );
	SetPort ( theWindow );

	EraseRect ( &theWindow->portRect );
	ZoomWindow ( theWindow, partCode, false );

	invalidateScrollbars ( theWindow );
	
	/* setup for redraw */
	InvalRect ( &theWindow->portRect );

	SetPort ( savePort );
} /* WindowZoomBox */


/* was doGrowWindow */
void 
WindowGrow ( WindowPtr theWindow, Point thePoint )
{
	long	newSize;
	Rect	limitRect;
	
	/* set maximum size of window to entire monitor(s) area */
	limitRect		= gGrayRgnRect;
	/* set minimum size of window */
	limitRect.left	= kMinWindSize;
	limitRect.top	= kMinWindSize;
	
	/* track user mouse and grow the window */
	newSize = GrowWindow ( theWindow, thePoint, &limitRect );
	
	if ( newSize != nil )
	{
		/* erase and invalidate scroll bar area */
		invalidateScrollbars ( theWindow );
		
		SizeWindow ( theWindow, LoWord(newSize), HiWord(newSize), true );

		/* relocate scroll bars, then erase and ivalidate scroll bar area */
		invalidateScrollbars ( theWindow );
	}
} /* WindowGrow */


/* was doCloseWindow */
Boolean
WindowClose ( WindowPtr theWindow, short modifiers, Boolean userInteract )
{
	WindowPtr	windowToClose;
	Boolean		theWindowClosed;
	
	my_assert ( (theWindow == nil) || (windowValid(theWindow)), "\pWindowClose: invalid window" );
	
	/* assume function unsuccessful until a window is acutally closed */
	theWindowClosed = false;
	
	if ( modifiers & optionKey )
	{
		/* if the optionKey is active, try to close all open windows */
		theWindowClosed = WindowCloseAll ( userInteract );
	}
	else
	{
		if ( theWindow == nil )
		{
			windowToClose = FrontWindow ();
		}
		else
		{
			windowToClose = theWindow;
		}
		
		/* do the appropriate type of close based on the kind of window */
		switch ( WindowType( windowToClose ) )
		{
			case Window_about :
				AboutBoxClose ();
				theWindowClosed = true;
				break;
			
			case Window_UNKNOWN :
				my_assert ( kForceAssert, "\pWindowClose: unknown window type" );
			
			case Window_dlgModal :
			case Window_dlgMoveableModal :
			case Window_DA :
			default :
				break;
		}
	}
	
	return theWindowClosed;
} /* WindowClose */


/* was closeAllWindows */
Boolean
WindowCloseAll ( Boolean userInteract )
{
	Boolean		theWindowsClosed;
	WindowPtr	theWindow;
	
	theWindowsClosed	= true;
	theWindow			= FrontWindow ();
	
	if ( theWindow != nil )
	{
		/* keep closing windows until they are all closed, or the user interrupts
			or cancels the closing */
		do
		{
			theWindowsClosed	= WindowClose ( theWindow, nil, userInteract );
			theWindow			= FrontWindow ();
			
		} while ( (theWindow != nil) && theWindowsClosed );
	}
	
	return theWindowsClosed;
} /* WindowCloseAll */


/* WINDOW KIND TESTING */

/* was kindOfWindow */
window_type
WindowType ( WindowPtr theWindow )
{
	int				windowKind;
	windowInfoHdl	windInfo;
	
	/* if the window is a null pointer return negative */
	if ( theWindow == nil )
	{
		return Window_none;
	}
	
	windInfo = (windowInfoHdl) GetWRefCon ( theWindow );
	
	if ( windInfo != nil )
	{
		return (*windInfo)->wType;
	}
	
	windowKind = ((WindowPeek)theWindow)->windowKind;
	
	if ( windowKind < nil )
	{
		/* desk accessory */
		return Window_DA;
	}
	else if ( windowKind == dialogKind )
	{
		return Window_dlgModal;
	}
	else
	{
		return Window_UNKNOWN;
	}
} /* WindowType */


/*  */
windowInfoHdl
WindowNewInfoHdl (
	window_type		theType,
	Boolean			hasScrollbars,
	Handle			theDataHdl )
{
	windowInfoHdl	theWindInfo;
	
	theWindInfo = (windowInfoHdl) MyNewHandle ( sizeof(windowInfo), nil );
	
	if ( theWindInfo != nil )
	{
		(*theWindInfo)->wType		= theType;
		(*theWindInfo)->scrollbars	= hasScrollbars;
		(*theWindInfo)->dataHdl		= theDataHdl;
	}
	
	return theWindInfo;
}

/* MISC UTILITIES */

/*  */
Boolean
windowValid ( WindowPtr theWindow )
{
	Boolean		isValid;
	
	if ( theWindow == nil )
	{
		isValid = false;
	}
	else
	{
		/* еее need more tests here */
		isValid = true;
	}
	
	return isValid;
} /* windowValid */


/*  */
Boolean
windowHasScrollbars ( WindowPtr theWindow )
{
	windowInfoHdl	windInfo;
	
	windInfo = (windowInfoHdl) GetWRefCon ( theWindow );
	
	return (*windInfo)->scrollbars;
} /* windowHasScrollbars */


/*  */
void
invalidateScrollbars ( WindowPtr theWindow )
{
	Rect	tempRect;
	
	if ( windowHasScrollbars(theWindow) )
	{
		SetPort ( theWindow );
		
		/* invalidate & erase vertical (right-hand side) scrollbar */
		tempRect		= theWindow->portRect;
		tempRect.left	= tempRect.right-15;
		InvalRect ( &tempRect );
		EraseRect ( &tempRect );
		
		/* invalidate & erase horizontal (bottom) scrollbar */
		tempRect		= theWindow->portRect;
		tempRect.top	= tempRect.bottom-15;
		InvalRect ( &tempRect );
		EraseRect ( &tempRect );
	}
} /* invalidateScrollbars */


/*****  EOF  *****/

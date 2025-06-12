/*______________________________________________________________________

	wstm.c - Window State Module
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This module keeps track of window states.
	
	All of the code is placed in its own segment named "wstm".
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "wstm.h"

#pragma segment wstm

/*______________________________________________________________________

	wstm_Init - Initialize Window State.
	
	Entry:	windState = pointer to WindState struct.
					
	Exit:		window state information initialized.
_____________________________________________________________________*/


void wstm_Init (WindState *windState)

{
	windState->moved = false;;
}

/*______________________________________________________________________

	wstm_Save - Save Window State.
	
	Entry:	theWindow = window pointer.
				windState = pointer to WindState struct.
					
	Exit:		window state information saved in WindState structure.
_____________________________________________________________________*/


void wstm_Save (WindowPtr theWindow, WindState *windState)

{
	if (windState->moved) {
		utl_SaveWindowPos(theWindow, &windState->userState, &windState->zoomed);
	};
}

/*______________________________________________________________________

	wstm_ComputeStd - Compute a Window's Standard State.
	
	Entry:	theWindow = pointer to window.
	
	Exit:		computed standard state stored in state data block.
	
	See TN 79.
_____________________________________________________________________*/


void wstm_ComputeStd (WindowPtr theWindow)

{
	WindowPeek		w;					/* window pointer */
	WStateData		**stateData;	/* handle to zoom state data */
	GDHandle			gd;				/* handle to gdevice containing window */
	Rect				zoomRect;		/* rect to zoom to */
	Rect				windRect;		/* window rect in global coords */
	Boolean			hasMB;			/* true if window contains menu bar */
	short				windWidth;		/* width of window */
	
	w = (WindowPeek)theWindow;
	if (!(w->dataHandle && w->spareFlag)) return;
	utl_GetWindGD(theWindow, &gd, &zoomRect, &windRect, &hasMB);
	zoomRect.top += titleBarHeight + zoomSlop + 
		(hasMB ? utl_GetMBarHeight() : 0);
	zoomRect.bottom -= 3;
	stateData = (WStateData**)w->dataHandle;
	windWidth = windRect.right - windRect.left;
	if (zoomRect.left <= windRect.left &&
		windRect.right <= zoomRect.right) {
		zoomRect.left = (**stateData).userState.left;
		zoomRect.right = zoomRect.left + windWidth;
	} else if (windRect.right > zoomRect.right) {
		zoomRect.right -= 3;
		zoomRect.left = zoomRect.right - windWidth;
	} else {
		zoomRect.left += 3;
		zoomRect.right = zoomRect.left + windWidth;
	};
	(**stateData).stdState = zoomRect;
}

/*______________________________________________________________________

	wstm_ComputeDef - Compute a Window's Default State.
	
	Entry:	theWindow = pointer to window.
	
	Exit:		userState = computed state rectangle (location and size).
_____________________________________________________________________*/


void wstm_ComputeDef (WindowPtr theWindow, Rect *userState)

{
	Point			pos;			/* window position */
	
	*userState = theWindow->portRect;
	utl_StaggerWindow(userState, staggerInitialOffset, staggerOffset, &pos);
	OffsetRect(userState, pos.h, pos.v);
}

/*______________________________________________________________________

	wstm_Restore - Restore Window State.
	
	Entry:	dlog = true if dialog window, false if regular window.
				windID = resource ID of window template.
				wStorage = pointer to window storage, or nil.
				windState = pointer to saved window state.
					
	Exit:		function result = pointer to window, positioned and sized.
_____________________________________________________________________*/


WindowPtr wstm_Restore (Boolean dlog, short windID, Ptr wStorage,
	WindState *windState)

{
	WindowPtr		theWindow;			/* pointer to window */
	Rect				userState;			/* user state rectangle */
	
	theWindow = dlog ?
		GetNewDialog(windID, wStorage, (WindowPtr)-1) :
		utl_GetNewWindow(windID, wStorage, (WindowPtr)-1);
	if (!windState->moved) {
		wstm_ComputeDef(theWindow, &userState);
		MoveWindow(theWindow, userState.left, userState.top, false);
		SizeWindow(theWindow, userState.right-userState.left,
			userState.bottom-userState.top, true);
	} else {
		utl_RestoreWindowPos(theWindow, &windState->userState, windState->zoomed,
			dragSlop, wstm_ComputeStd, wstm_ComputeDef);
	};
	return theWindow;
}

/*______________________________________________________________________

	wstm_Mark - Mark a Window Moved or Sized.
	
	Entry:	windState = pointer to saved window state.
_____________________________________________________________________*/


void wstm_Mark (WindState *windState)

{
	windState->moved = true;
};


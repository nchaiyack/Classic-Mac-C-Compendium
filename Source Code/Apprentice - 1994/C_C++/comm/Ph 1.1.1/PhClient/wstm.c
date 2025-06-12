/*______________________________________________________________________

	wstm.c - Window State Module
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "glob.h"
#include "wstm.h"
#include "oop.h"

#pragma segment wstm

/*______________________________________________________________________

	wstm_Save - Save Window State.
	
	Entry:	theWindow = window pointer.
				windState = pointer to WindState struct.
					
	Exit:		window state information saved in WindState structure.
_____________________________________________________________________*/

void wstm_Save (WindowPtr theWindow, WindState *windState)

{
	if (!windState->moved) windState->moved = oop_Moved(theWindow);
	if (windState->moved) {
		utl_SaveWindowPos(theWindow, &windState->userState, &windState->zoomed);
	}
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
	}
	(**stateData).stdState = zoomRect;
}

/*______________________________________________________________________

	ComputeDef - Compute a Window's Default State.
	
	Entry:	theWindow = pointer to window.
	
	Exit:		userState = computed state rectangle (location and size).
_____________________________________________________________________*/

static void ComputeDef (WindowPtr theWindow, Rect *userState)

{
	Point			pos;			/* window position */
	
	*userState = theWindow->portRect;
	if (((WindowPeek)theWindow)->spareFlag) {
		utl_StaggerWindow(userState, staggerInitial, staggerOffset, &pos);
		OffsetRect(userState, pos.h, pos.v);
	} else {
		userState->top -= titleBarHeight;
		utl_CenterDlogRect(userState, true);
		userState->top += titleBarHeight;
	}
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
		ComputeDef(theWindow, &userState);
		MoveWindow(theWindow, userState.left, userState.top, false);
		SizeWindow(theWindow, userState.right-userState.left,
			userState.bottom-userState.top, true);
	} else {
		if (!((WindowPeek)theWindow)->spareFlag) {
			windState->userState.right = windState->userState.left +
				theWindow->portRect.right;
			windState->userState.bottom = windState->userState.top +
				theWindow->portRect.bottom;
		}
		utl_RestoreWindowPos(theWindow, &windState->userState, windState->zoomed,
			dragSlop, wstm_ComputeStd, ComputeDef);
	}
	return theWindow;
}

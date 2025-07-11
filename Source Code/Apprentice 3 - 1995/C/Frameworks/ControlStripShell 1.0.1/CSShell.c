/* ---------------------------------------------------------------------------
CSShell.c

	�1994 Martin R. Wachter; All Rights Reserved.

	A sample Control Strip Module shell with a placeholder popup menu.
	
	NOTE: Requires the "ControlStrip.h" header available on 
	the Developer CD series.
	
Version History
1.0 9/94	by Marty Wachter mrw@welchgate.welch.jhu.edu

Based on the MacCalendar Control Strip Sample by Martin Minow, MACDTS.
--------------------------------------------------------------------------- */

#include "CSShell.h"

// ---------------------------------------------------------------------------
//		� main
// ---------------------------------------------------------------------------
//
// main entry point into our sdev
//
pascal long main (long message, long params, Rect *statusRect, GrafPtr statusPort)
{
	long result = 0L;
	Str255	helpString;
	
	switch (message)
	{
		case sdevInitModule: 		// check environs, allocate globals
			result = DoCSInit();
		break;
		
		case sdevCloseModule: 		// release my memory
			DoCSClose((MyGlobalHandle) params);
		break;
		
		case sdevFeatures: 			// let the strip track the mouse down
			result = (  (1<<sdevWantMouseClicks)	// We handle mouse down
					  | (1<<sdevDontAutoTrack)		// We track the mouse, too
					  | (1<<sdevHasCustomHelp)		// Custom help string
					);
			break;
		break;
		
		case sdevGetDisplayWidth:	// inform the strip how much strip space we need
			result = kIconWidth + width((*(*(MyGlobalHandle) params)->myArrowPict)->picFrame);
		break;
		
		case sdevPeriodicTickle:	// no idle time used
		break;
		
		case sdevDrawStatus: 		// draw my icon and arrow pict
			DoCSDraw((MyGlobalHandle) params, statusRect, statusPort);
		break;
		
		case sdevMouseClick: 		// the mouse was clicked & released in my button
			DoCSClick((MyGlobalHandle) params, statusRect);
		break;
		
		case sdevSaveSettings:		// no settings in this module
		break;
		
		case sdevShowBalloonHelp:	// we have a custom ballon help string
			SBGetDetachedIndString(helpString, (*(MyGlobalHandle) params)->myStrings, kHelpStringIndex);
			SBShowHelpString(statusRect, helpString);
		break;
	}
	return (result);
}

// ---------------------------------------------------------------------------
//		� DoCSClick
// ---------------------------------------------------------------------------
//
// handle a click in our status rect
//
void DoCSClick(MyGlobalHandle myGlobals, const Rect	*statusRect)
{
	OSErr				iErr;
	short				menuID;
	MenuHandle			menuH;
	
	if ((*myGlobals)->myMenuH){
		HLock((Handle)myGlobals);
	
		if ((*myGlobals)->myMenuH){
			menuID = SBTrackPopupMenu(statusRect, (*myGlobals)->myMenuH);
			
			switch (menuID){
				
				case 1:
					break;
				case 2:
					break;
				default:
				
					break;
			}				//end switch
		}
		HUnlock((Handle)myGlobals);
	}
}

// ---------------------------------------------------------------------------
//		� DoCSInit
// ---------------------------------------------------------------------------
//
// allocate memory for our globals and init our globals
//
long DoCSInit(void)
{
	MyGlobalHandle	myH;
	OSErr			iErr;
	Handle			iconSuite = 0L;
	Str255			tempString;
	
	//
	// get some memory
	//
	myH = (MyGlobalHandle) NewHandleClear(sizeof(MyGlobals));
	
	//
	// got it
	//
	if (myH){	
		HLock((Handle)myH);

	//
	// get our icon suite
	//
		iErr = SBGetDetachIconSuite(&iconSuite, 669, 0x0000FF00L);
		if (iErr) return iErr;
		(*myH)->iconSuite = iconSuite;
	//
	// get our string list
	//
		(*myH)->myStrings = GetResource('STR#', 256);
		iErr = ResError();
		if (iErr) return iErr;
		DetachResource((*myH)->myStrings);
		iErr = ResError();
		if (iErr) return iErr;
	//
	// get our popup menu
	//
		(*myH)->myMenuH = GetMenu(kPopupMenuID);
		if (!(*myH)->myMenuH) return ResError();
		DetachResource((Handle)(*myH)->myMenuH);
		iErr = ResError();
		if (iErr) return iErr;
	//
	// get our arrow picture
	//
		(*myH)->myArrowPict = GetPicture(kArrowPictID);
		if (!(*myH)->myArrowPict) return ResError();
		DetachResource((Handle)(*myH)->myArrowPict);
		iErr = ResError();
		if (iErr) return iErr;
		HUnlock((Handle)myH);
		return ((long) myH);
	}
	//
	// did not get any memory, don't install
	//
	return (-1L);
}

// ---------------------------------------------------------------------------
//		� DoCSClose
// ---------------------------------------------------------------------------
//
// free memory for our globals
//
void DoCSClose(MyGlobalHandle myGlobals)
{
	if (myGlobals){
		HLock((Handle)myGlobals);
		if ((*myGlobals)->iconSuite)
			DisposeIconSuite ((*myGlobals)->iconSuite, true);
		if ((*myGlobals)->myMenuH)
			DisposeMenu((*myGlobals)->myMenuH);
		if ((*myGlobals)->myStrings)
			DisposeHandle((*myGlobals)->myStrings);
		if ((*myGlobals)->myStrings)
			KillPicture((*myGlobals)->myArrowPict);
		DisposeHandle((Handle) myGlobals);
	}
}

// ---------------------------------------------------------------------------
//		� DoCSDraw
// ---------------------------------------------------------------------------
//
// draw our icon and arrow pict
//
void DoCSDraw(MyGlobalHandle myGlobals, Rect *statusRect, GrafPtr statusPort)
{
	OSErr		iErr;
	Rect		viewRect;
	short		alignment = 0x01 + 0x04; // center up & down , and left & right
	long		transform = 0; 			 // no transform
	short		arrowHeight = 0;
	
	if ((*myGlobals)->iconSuite){
	//
	// Draw our icon
	//
		viewRect = *statusRect;
		viewRect.right = viewRect.left + kIconWidth;
		(void)PlotIconSuite(&viewRect,atNone,ttNone,(*myGlobals)->iconSuite);
	}
	
	//
	// Draw our right-arrow pict to show that we have a popup menu
	//
	if ((*myGlobals)->myArrowPict){
		arrowHeight = height((*(*myGlobals)->myArrowPict)->picFrame);
		viewRect.left = viewRect.right;
		viewRect.right += width((*(*myGlobals)->myArrowPict)->picFrame);
		viewRect.top += ((height(viewRect) - arrowHeight) >> 1);
		viewRect.bottom = viewRect.top + arrowHeight;
		DrawPicture((*myGlobals)->myArrowPict, &viewRect);
	}
}
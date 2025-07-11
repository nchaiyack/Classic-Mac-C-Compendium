/*
Zoom.c 
10/17/90	dgp	translated from pascal DoWZoom.p in Inside Mac VI.
10/18/90 dgp	removed part of the code to make GetWindowDevice(), which I put
				in GetScreenDevice.c
8/24/91	dgp		Made compatible with THINK C 5.0.
12/27/91 dgp	Extracted code to create TitleBarHeight.c
3/26/92	dgp		Replaced use of SysEnvirons() by a call to QD8Exists().
*/
#include "VideoToolbox.h"
#if THINK_C
	#include <LoMem.h>
#else
	short MBarHeight : 0xBAA;
#endif

void Zoom(WindowPtr theWindow, int zoomDir)
{
	Rect r;
	GDHandle dominantGDevice;
	int headRoom;
	GrafPtr savePort;
	extern EventRecord theEvent;	/* global from main program */
	
	if(TrackBox(theWindow,theEvent.where,zoomDir)){
		GetPort(&savePort);
		SetPort(theWindow);
		EraseRect(&theWindow->portRect);		/*recommended for cosmetic reasons*/
		/* If there is the possibility of multiple gDevices, then we */
		/* must check them to make sure we are zooming onto the right */
		/* display device when zooming out. */
		if(zoomDir==inZoomOut && QD8Exists()){
			headRoom=TitleBarHeight(theWindow);
			/* We must create a zoom rectangle manually in this case. */
			/* Account for menu bar height as well, if on main device */
			dominantGDevice=GetWindowDevice(theWindow);
			if(dominantGDevice==GetMainDevice()) headRoom += MBarHeight;
			r=(*dominantGDevice)->gdRect;
			SetRect(&r,r.left+3,r.top+headRoom+3,r.right-3,r.bottom-3);
			/* Set up the WStateData record for this window. */
			(*((WStateData **)((WindowPeek)theWindow)->dataHandle))->stdState = r;
		}
		ZoomWindow(theWindow,zoomDir,TRUE);
		SetPort(savePort);
	}
}

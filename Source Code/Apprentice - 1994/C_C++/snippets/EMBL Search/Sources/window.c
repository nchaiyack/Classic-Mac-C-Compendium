/*
*********************************************************************
*	
*	Window.c
*	General routines for window handling
*		
*	Rainer Fuchs
*  EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*	
**********************************************************************
*	
*/ 

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "window.h"
#include "util.h"
#include "save.h"
#include "query.h"
#include "sequence.h"
#include "results.h"
#include "pstr.h"

/*
********************************* Globals *****************************
*/

extern FONTINFO 	fMonaco,fSystem;
extern MenuHandle	gMenu[MENUNR];
extern Prefs gPrefs;
extern Boolean gHasColorQD;

Rect	gDragRect, 		/* Max drag area */
		gSeqWinRect, 	/* size of sequence entry window */
		gResWinRect,  	/* size of result window */
		gQueryWinRect,	/* size of query window */
		gMinRect;		/* minimum and maximum size of res and seq window */

WDRec gWindows[MAXWIN];


/**************************************
*	Check for application window
*	Return value:	TRUE, if window belongs to application
*						FALSE, if not (or NULL was passed)
*/

Boolean IsAppWindow(WindowPtr wPtr)
{
	if ( wPtr == NULL )
		return(FALSE);
	else				/* application windows have windowKinds = userKind (8) */
		return (((WindowPeek) wPtr)->windowKind >= userKind ||
				  ((WindowPeek) wPtr)->windowKind == dialogKind);
}


/**************************************
*	Check for desk accessory window
*	Return value:	TRUE, if window belongs to accessory
*						FALSE, if not (or NULL was passed)
*/

Boolean IsDAWindow(WindowPtr wPtr)
{
	if ( wPtr == NULL )
		return(FALSE);
	else							/* DA windows have negative windowKinds */
		return (((WindowPeek) wPtr)->windowKind < 0);
}

/**************************************
*	Centers dialogs and alerts dependent on screen size
*	"what" is the resource type ('DLOG' or 'ALRT')
*	"id" is the resource ID
*	"offset" is the vertical offset from central position in percent (0 =center,
*	100 = top of screen)
*	Return value: none
*/

void CenterDA(ResType what,short id,short offset)
{
/*	Since the first field in both AlertTemplate and DialogTemplate is boundsRect,
	Alerts are simply treated as Dialogs...This is similar to WindowPtr vs.
	DialogPtr.
*/

	DialogTHndl	myHandle;
	short			height;
	short			width;
	short			pixels;
	short			mBarHeight=GetMBarHeight();			/* subtract menu bar height */

	/* get the resource and calculate width and height */
	
	myHandle=(DialogTHndl)GetResource(what,id);
	if(myHandle == NULL) return;
	
	height=(**myHandle).boundsRect.bottom-(**myHandle).boundsRect.top;
	width=(**myHandle).boundsRect.right-(**myHandle).boundsRect.left;
	
	/* center the window (taking into account menu bar height) */
	
	(**myHandle).boundsRect.top = mBarHeight +
		((screenBits.bounds.bottom - screenBits.bounds.top) - height)/2;
	/* Adjust for offset */
	pixels=(**myHandle).boundsRect.top * (offset/100.0);
	(**myHandle).boundsRect.top -= pixels;
	(**myHandle).boundsRect.bottom=(**myHandle).boundsRect.top+height;
	(**myHandle).boundsRect.left=
		((screenBits.bounds.right - screenBits.bounds.left) - width)/2;
	(**myHandle).boundsRect.right=(**myHandle).boundsRect.left+width;
}

/**************************************
*	Invalidates areas occupied by vertical and horizontal scroll bars
*/

void InvalBars(WindowPtr wPtr)
{
	Rect r;
	GrafPtr savePort;
	
	if( wPtr == NULL )
		return;
		
	savePort = ChangePort(wPtr);
	r = wPtr->portRect;
	r.top = r.bottom-SBarWidth;
	InvalRect(&r);
	
	r.top = wPtr->portRect.top;
	r.left = r.right-SBarWidth;
	InvalRect(&r);
	SetPort(savePort);
}

/**************************************
*	Initialises values of window record elements
*/

void ClearWindowRec(WDPtr wdp)
{
	if( wdp == NULL )
		return;
		
	((WindowPeek)wdp)->windowKind = 0;
	wdp->vScroll=wdp->hScroll = NULL;
	wdp->ctrl1=wdp->ctrl2=wdp->ctrl3=wdp->ctrl4=NULL;
	((WindowPeek)wdp)->refCon = 0L;
	wdp->vRefNum = 0;
	*(wdp->fName) = EOS;
	wdp->userHandle = NULL;
	wdp->dirty = FALSE;
	wdp->inUse = FALSE;
}

/**************************************
*	Windows initialisation
*/

void PrepWindows()
{
	short	mBarHeight;
	Rect	desktopRect;
	short	i;
	DialogTHndl	myHandle;

	mBarHeight=GetMBarHeight();  /*	to allow for varying sizes eg Radius
												screens */
	desktopRect=(**GetGrayRgn()).rgnBBox; /* to deal with multiple monitors */
	
	/* Now make window sizes only dependent on monitor size: */
	
	SetRect(&gMinRect, 4*fMonaco.finfo.widMax+SBarWidth+2*RES_MARGIN,
							4*fMonaco.height+SBarWidth,
							desktopRect.right,
							desktopRect.bottom);
														
	SetRect(&gDragRect,4,mBarHeight+4,desktopRect.right-4,desktopRect.bottom-4);
	
	SetRect(&gSeqWinRect,screenBits.bounds.left+4,
	                    	screenBits.bounds.top+mBarHeight+20,
							  	screenBits.bounds.left+4+MAXCOL*fMonaco.finfo.widMax +
							  		SBarWidth + 2*SEQ_MARGIN,
							  	screenBits.bounds.top+mBarHeight+20+SBarWidth+
							  		((screenBits.bounds.bottom-screenBits.bounds.top -
							  		mBarHeight-20-SBarWidth-60)/fMonaco.height) *
							  		fMonaco.height);
								/* -60 to let room for staggered windows */
								
	SetRect(&gResWinRect,screenBits.bounds.left+4,
	                    	screenBits.bounds.top+mBarHeight+20,
							  	screenBits.bounds.left+4+MAXCOL*fMonaco.finfo.widMax+
							  		SBarWidth+2*RES_MARGIN,
							  	screenBits.bounds.top+mBarHeight+20+SBarWidth +
							  		((screenBits.bounds.bottom-screenBits.bounds.top-
							  		mBarHeight-20-SBarWidth - 60)/fMonaco.height) *
							  		fMonaco.height);
								/* -60 to let room for staggered windows */

	myHandle=(DialogTHndl)GetResource('DLOG',QUERY_DLG);
	if(myHandle == NULL) return;
	
	SetRect(&gQueryWinRect,screenBits.bounds.left+4,
	                    	screenBits.bounds.top+mBarHeight+20,
							  	screenBits.bounds.left+4+
							  		(**myHandle).boundsRect.right -
							  		(**myHandle).boundsRect.left,
							  	screenBits.bounds.top+mBarHeight+20+
							  		(**myHandle).boundsRect.bottom -
							  		(**myHandle).boundsRect.top);
								
	for(i=0;i<MAXWIN;++i)
		ClearWindowRec(&gWindows[i]);
}


/**************************************
*	Simple staggering of windows by adding an offset to the window frame
*/

void AdjustPosition(short w, Rect *rect)
{
	short offset1,offset2,offset;
	
	offset1 = 10 * (w % 5);
	offset2 = 5 * (w / 5);
	offset = offset1 + offset2;
	
	rect->top += offset;
	rect->left += offset;
	rect->right += offset;
	rect->bottom += offset;
}


/**************************************
*	Identifies window description record corresponding to
*	a given WindowPtr
*	Return value:	Pointer to WindowDesc
*						NULL, if not found
*/

WDPtr FindMyWindow(WindowPtr wPtr)
{
	register short i;
	
	if( wPtr == NULL )
		return(NULL);
		 
	for(i=0;i<MAXWIN;++i) {
		if(wPtr == (WindowPtr)&gWindows[i])
			return(&gWindows[i]);
	}

	return(NULL);
}

/**************************************
*	find a free window description structure
*	Return value:	array offset of structure, if a free one was found
*						-1, if not
*/

short GetFreeWindow()
{
	register short w;
	
	for(w=0;w<MAXWIN;++w) {
		if(gWindows[w].inUse == FALSE)
			return(w);
	}
	
	return(-1);
}

/**************************************
*	Grow a window
*	Return value:	none
*/

void DoGrow(WindowPtr wPtr,Point where)
{
	WDPtr		wdp=FindMyWindow(wPtr);
	long		newBounds;
	short		newHeight,newWidth;
	Rect		tempRect;
	GrafPtr	savePort;
	
	if( wdp == NULL )
		return;

	savePort = ChangePort(wPtr);
	InvalBars(wPtr);
	if( (newBounds=GrowWindow(wPtr,where,&gMinRect)) ) {
		newWidth=LoWord(newBounds);
		newHeight=HiWord(newBounds);
		
		/* Adjust size */
	   SetRect(&tempRect,0,0,newWidth,newHeight);
	   AdjustWSize(((WindowPeek)wdp)->windowKind,&tempRect,fMonaco.height,
	   				fMonaco.finfo.widMax);
	
		/* Clear screen (looks nicer) and grow it */
		EraseRect(&wPtr->portRect);
		SizeWindow(wPtr,tempRect.right,tempRect.bottom,TRUE);
		InvalBars(wPtr);
		GrowScroll(wdp);
	
		/* redraw */
		InvalRect(&wPtr->portRect);
		SetVScroll(wdp);
		SetHScroll(wdp);
	}
	SetPort(savePort);
}

/**************************************
*	Grow scroll bars
*	Return value:	none
*/

void GrowScroll(WDPtr wdp)
{
	short	newTop,newBottom,newLeft,newRight;
	WindowPtr wPtr = (WindowPtr)wdp;
	short windowKind;
	
	if ( wdp == NULL )
		return;
		
	windowKind = ((WindowPeek)wdp)->windowKind;
	if(windowKind == queryW)	/* no scroll bars */
		return;
		
	HideControl(wdp->vScroll);
	HideControl(wdp->hScroll);

	switch(windowKind) {
		case resW:
			newTop=wPtr->portRect.top+3*fMonaco.height-1;
			break;
		case seqW:
			newTop=wPtr->portRect.top-1;
			break;
	}
	
	newLeft=wPtr->portRect.right-SBarWidth;
	newBottom=wPtr->portRect.bottom-SBarWidth;
	MoveControl(wdp->vScroll,newLeft,newTop);
	SizeControl(wdp->vScroll,SBarWidth+1,newBottom-newTop+1);
	
	newTop=wPtr->portRect.bottom-SBarWidth;
	newLeft=wPtr->portRect.left-1;
	newRight=wPtr->portRect.right-SBarWidth;
	MoveControl(wdp->hScroll,newLeft,newTop);
	SizeControl(wdp->hScroll,newRight-newLeft+1,SBarWidth+1);

	ShowControl(wdp->vScroll);
	ShowControl(wdp->hScroll);
}


/**************************************
*	Zoom a window. In an ideal program we would check to see on which monitor the main
*	part of the window is in order to zoom on this window.
*	Return value:	none
*/

void DoZoom(WindowPtr wPtr,short partCode)
{
	WDPtr		wdp=FindMyWindow(wPtr);
	short		windowKind;
	Rect		globalPortRect,theSect,zoomRect;
	GDHandle	nthDevice,dominantGDevice;
	long		greatestArea,sectArea;
	short		margin,bias;
	
	if ( wdp == NULL )
		return;
		
	windowKind = ((WindowPeek)wdp)->windowKind;
	if(windowKind == queryW)	/* no zoom box */
		return;
		
	SetPort(wPtr);
	EraseRect(&(wPtr->portRect));
	
	/* Adopted from DTS snippets:
		If there is the possibility of multiple gDevices, then we must check them to make
		sure we are zooming onto the right display device when zooming out. */
		
	if (partCode == inZoomOut && gHasColorQD) {
		/* window's portRect must be converted to global coordinates */
		globalPortRect = wPtr->portRect;
		LocalToGlobal(&TopLeft(globalPortRect));
		LocalToGlobal(&BottomRight(globalPortRect));

 		nthDevice = GetDeviceList();
		greatestArea = 0;
		/* This loop checks the window against all the gdRects in the
          	gDevice list and remembers which gdRect contains the largest
          	portion of the window being zoomed. */
		while(nthDevice != NULL) {
			SectRect(&globalPortRect,&(**nthDevice).gdRect,&theSect);
			sectArea = (long)(theSect.right - theSect.left) *
               			(long)(theSect.bottom - theSect.top);
			if (sectArea > greatestArea) {
				greatestArea = sectArea;
				dominantGDevice = nthDevice;
			}
			nthDevice = GetNextDevice(nthDevice);
		}

		/* We must create a zoom rectangle manually in this case.
 				account for menu bar height as well, if on main device */
		bias = 20;
		if(dominantGDevice == GetMainDevice())
			bias += GetMBarHeight();
			
		zoomRect = (**dominantGDevice).gdRect;
		zoomRect.left += 4;
		zoomRect.top += bias;
		
		if(windowKind == seqW) margin = SEQ_MARGIN;
		else margin = RES_MARGIN;
		zoomRect.right = zoomRect.left + MAXCOL*fMonaco.finfo.widMax +
								SBarWidth + 2*margin;
		zoomRect.bottom = zoomRect.top + SBarWidth +
								((zoomRect.bottom - SBarWidth - zoomRect.top)/fMonaco.height)
								* fMonaco.height;
			

		(**((WStateDataHandle)((WindowPeek)wPtr)->dataHandle)).stdState = zoomRect;
	}      

	ZoomWindow(wPtr,partCode,TRUE);

	GrowScroll(wdp);
	InvalRect(&(wPtr->portRect));
	switch( ((WindowPeek)wdp)->windowKind) {
		case resW:
			SetVScroll(wdp);
			SetHScroll(wdp);
			break;
		case seqW:
			SetVScroll(wdp);
			SetHScroll(wdp);
			break;
	}
}


/**************************************
*	Close a window. If shift key is pressed, we do not ask whether file should 
*	be saved.
*	Return value:	TRUE, if successful
*						FALSE, if error occurred or user cancelled
*/

Boolean CloseMyWindow(WDPtr wdp, Boolean shift)
{
	short			i;
	Str255		title;
	
	if( wdp == NULL )
		return(TRUE);
	
	GetWTitle((WindowPtr)wdp,title);
	InitCursor();
	if(wdp->dirty && !shift && gPrefs.confirmChg) {
		ParamText(title,"\p","\p","\p");
		CenterDA('ALRT',CLOSE_ALRT,33);
		switch (StopAlert(CLOSE_ALRT, NULL)) {
			case aaSave:
				if(!DoSave(wdp,TRUE))
					return(FALSE);
 				break;
			case aaDiscard:
 				break;
 			case aaCancel: return(FALSE);
 		}
 	}
 		
	switch( ((WindowPeek)wdp)->windowKind ) {
		case queryW:
			DisposeQuery(wdp);
			break;
		case seqW:
			DisposeSequence(wdp);
			break;
		case resW:
			DisposeResults(wdp);
			break;
	}
	
	DelWindowFromMenu(title);
	
	return(TRUE);
}


/**************************************
*	Close all windows. If shift key is pressed, we do not ask whether file should 
*	be saved.
*	Return value:	TRUE, if successful
*						FALSE, if error occurred or user cancelled
*/

Boolean CloseAllWindows(Boolean shift)
{
	WindowPtr	wPtr;
	WDPtr			wdp;
	short			n;
	
	while( (wPtr = FrontWindow()) != NULL ) {
		n=((WindowPeek)wPtr)->windowKind;
		if(n<0)
			CloseDeskAcc(n);
		else {
			wdp = FindMyWindow(wPtr);
			if(wdp)
				if(!CloseMyWindow(wdp,shift))
					return(FALSE);
		}
	}
	return(TRUE);
}

/**************************************
*	Adjust value of vertical scroll bar
*/

void SetVScroll(WDPtr wdp)
{
	Rect			viewRect;
	SeqRecHdl	seqRecHdl;
	ResultHdl	resHdl;
	short			n,visLines,max;
	
	if (wdp == NULL)
		return;
		
	switch(((WindowPeek)wdp)->windowKind) {
		case seqW:
			seqRecHdl = (SeqRecHdl)wdp->userHandle;
			GetViewRect((WindowPtr)wdp,&viewRect);	
			visLines = (viewRect.bottom - viewRect.top) / fMonaco.height;
			n = (**seqRecHdl).nlines - visLines;
			break;
		case resW:
			resHdl = (ResultHdl)wdp->userHandle;
			GetViewRect((WindowPtr)wdp,&viewRect);	
			visLines = (viewRect.bottom - viewRect.top) / fMonaco.height;
			n = (**resHdl).nhits - visLines;
			break;
		case queryW:	/* no scroll bar */
			return;
			break;
	}

	max = (n > 0) ? n : 0;
	SetCtlMax(wdp->vScroll,max);
}


/**************************************
*	Adjust value of horizontal scroll bar
*/

void SetHScroll(WDPtr wdp)
{
	Rect	viewRect;
	short	n,visCol,max,margin;
	
	if( wdp == NULL )
		return;
		
	switch(((WindowPeek)wdp)->windowKind) {
		case seqW:
			GetViewRect((WindowPtr)wdp,&viewRect);
			margin = SEQ_MARGIN;
			break;
		case resW:
			GetViewRect((WindowPtr)wdp,&viewRect);
			margin = RES_MARGIN;	
			break;
		case queryW:	/* no scroll bar */
			return;
			break;
	}

	visCol = (viewRect.right - viewRect.left - 2*margin) / fMonaco.finfo.widMax;
	n = MAXCOL - visCol;	/* We assume a max line length of MAXCOL */
	max = (n > 0) ? n : 0;
	SetCtlMax(wdp->hScroll,max);
}

/**************************************
*	Adjusts a rectangle (window size) to look nicer, taking
*	margins and scroll bars into account
*	Return value:	none
*	Side-effect:	adjusted rect in r
*/

void AdjustWSize(short wKind, Rect *r,short height, short width)
{	
	short margin;
	
	switch(wKind) {
		case seqW:
			margin = SEQ_MARGIN;
			break;
		case resW:
			margin = RES_MARGIN;
			break;
		case queryW:
			break;
	}

	r->bottom = r->top + height*((r->bottom - r->top - SBarWidth)/height)
					+ SBarWidth;

	r->right = r->left + width*((r->right - r->left - SBarWidth -
					2*margin)/width) + SBarWidth + 2*margin;
}

/**************************************
*	Draw grow icon in a window
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*/

void DoDrawGrowIcon(WindowPtr wPtr)
{
	WDPtr wdp=FindMyWindow(wPtr);
	Rect tempRect;
	RgnHandle tempRgn=NewRgn();
	short windowKind;
	
	if( wdp == NULL )
		return;
		
	windowKind = ((WindowPeek)wdp)->windowKind;
	if(windowKind == queryW)	/* no grow icon */
		return;
	
	tempRect=wPtr->portRect;
	
	switch(((WindowPeek)wdp)->windowKind) {
		case seqW:
			break;
		case resW:
			tempRect.top += 3*fMonaco.height;
			break;
	}
	
	GetClip(tempRgn);
	ClipRect(&tempRect);
	DrawGrowIcon(wPtr);
	SetClip(tempRgn);
	DisposeRgn(tempRgn);
}


/**************************************
*	Calculation of contents rectangle of a result window
*	Return value:	none
*	Side-effect:	rect in r
*/

void GetViewRect(WindowPtr wPtr, Rect *r)
{
	WDPtr wdp=FindMyWindow(wPtr);
	
	if( wdp == NULL )
		return;
		
	*r = wPtr->portRect;

	switch(((WindowPeek)wdp)->windowKind) {
		case seqW:
			break;
		case resW:
			r->top += 3*fMonaco.height;
			break;
		case queryW:
			break;
	}

	r->right -= SBarWidth;
	r->bottom -= SBarWidth;
}

/**************************************
*	Add a window title to Windows menu
*/

void AddWindowToMenu(StringPtr name)
{
	register short	entries=CountMItems(gMenu[WINDOWS]);
			
	/* we first insert a dummy name, then change it with SetItem.
		Otherwise we couldn't insert a name like "myfile;1" which contains
		a meta character */
		
	AppendMenu(gMenu[WINDOWS],"\pdummy");
	SetItem(gMenu[WINDOWS],entries+1,name);
	DrawMenuBar();
}

/**************************************
*	Delete a window title from Windows menu
*	Return value:	none
*/

void DelWindowFromMenu(StringPtr name)
{
	Str255 str;
	register short i;
	register short entries = CountMItems(gMenu[WINDOWS]);

	for(i=FIRSTWIN_I;i <= entries;++i) {
		GetItem(gMenu[WINDOWS],i,str);
		if(!pstrcmp(str,name)) {
			DelMenuItem(gMenu[WINDOWS],i);
			break;
		}
	}
}
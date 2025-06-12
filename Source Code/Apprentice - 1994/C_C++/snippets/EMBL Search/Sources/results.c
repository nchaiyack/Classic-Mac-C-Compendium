/*
*********************************************************************
*	
*	Results.c
*	Handling of results
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <stdio.h>
#include <string.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "results.h"
#include "util.h"
#include "window.h"
#include "hitstorage.h"
#include "pstr.h"
#include "click.h"
#include "export.h"
#include "events.h"

static void DrawResWin1(WDPtr wdp, short updown);
static pascal void ScrollResProc(ControlHandle theControl, short theCode);
static Boolean ReadResults(StringPtr fName, short vRefNum, QueryHdl *queryHdlP, HitmapHdl *hitmapHdlP);
static pascal Boolean resFileFilter(ParmBlkPtr myPBp);



/*
******************************** Global variables *****************
*/

extern WDRec		gWindows[MAXWIN];
extern Rect			gResWinRect;
extern FONTINFO	fMonaco,fSystem;
extern char			gError[256];
extern short		gScrollAmt,gScrollCode;
extern Boolean		gScrollDir;
extern DBInfo		gDBInfo[DB_NUM];
extern Boolean		gInBackground;

short gResNo;


/**************************************
*	Open a new result window
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean NewResultWindow(short w,WDPtr wdp, ResultHdl resHdl, StringPtr title)
{
	Rect			tempRect;
	WStateData	**stateHdl;
	WindowPtr	wPtr = (WindowPtr) wdp;
	Boolean		ret;
	
	if( resHdl == NULL || wdp == NULL )
		return(FALSE);
		
	/* fill short description buffer */
	StartWaitCursor();
	ret = FillDEBuffer(resHdl,0,FALSE);
	InitCursor();
	if ( !ret )
		return(FALSE);

	tempRect = gResWinRect;
	AdjustPosition(w,&tempRect);
	SetPort(NewWindow(wdp,&tempRect,"\p",FALSE,documentProc + 8,(void *)-1,TRUE,0L));
	
	/* anchor result record */
	wdp->userHandle = (Handle)resHdl;
			
	SetWTitle(wPtr,title);
	TextFont(fMonaco.num);
	TextSize(9);

	((WindowPeek)wPtr)->windowKind = resW;
	pstrcpy(wdp->fName,title);
	wdp->vRefNum = 0;
	wdp->dirty = TRUE;
	wdp->inited = FALSE;
	wdp->inUse = TRUE;
		
	/* adjust zoom standard state */
	stateHdl = (WStateData **)((WindowPeek)wPtr)->dataHandle;
	AdjustWSize(resW,&(**stateHdl).stdState,fMonaco.height,fMonaco.finfo.widMax);
	
	/* draw vertical scroll bar */
	tempRect = wPtr->portRect;
	tempRect.left = tempRect.right-SBarWidth;
	tempRect.right += 1;
	tempRect.top += 3*fMonaco.height - 1;
	tempRect.bottom -= (SBarWidth-1);
	wdp->vScroll = NewControl(wPtr,&tempRect,"\p",TRUE,0,0,10,scrollBarProc,0L);
	
	/* draw horizontal scroll bar */
	tempRect = wPtr->portRect;
	tempRect.top = tempRect.bottom-SBarWidth;
	tempRect.bottom += 1;
	tempRect.right -= (SBarWidth-1);
	tempRect.left -= 1;
	wdp->hScroll = NewControl(wPtr,&tempRect,"\p",TRUE,0,0,10,scrollBarProc,0L);
	
	/* Set scroll bars */
	SetVScroll(wdp);
	SetHScroll(wdp);
	
	/* Draw buttons */
	wdp->ctrl1 = GetNewControl(128,wPtr);
	wdp->ctrl2 = NULL;
	
	AddWindowToMenu(title);
	
	ShowWindow(wPtr);
	return(TRUE);
}


/**************************************
*	Close result window and dispose of result record
*/

void DisposeResults(WDPtr wdp)
{
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;
	
	if (wdp->inUse)
		CloseWindow((WindowPtr)wdp);
		
	if(wdp->userHandle) {
		DisposeResRec((ResultHdl)wdp->userHandle);
		DisposHandle(wdp->userHandle);
	}
			
	ClearWindowRec(wdp);
}


/**************************************
*	Completely redraws a result window
*/

void DrawResWinAll(WDPtr wdp,short dummy)
{
	GrafPtr			savePort;
	Rect				area,linebox,viewRect;
	short				i,visLines;
	ResultHdl		resHdl;
	HitlistHdl		hlHdl;
	CString80Hdl	bufHdl;
	WindowPtr		topWindow = FrontWindow();
	short 			vert,horiz;
	short				bufPos,hitPos,horizPos;
	SignedByte		oldState;
	RgnHandle		oldClipRgn,newClipRgn;
	Boolean			ret;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;
	
	/* Locate and lock down result data */
	resHdl = (ResultHdl)(wdp->userHandle);
	if(resHdl == NULL)
		return;
	bufHdl = (**resHdl).descBufHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
	hlHdl = (**resHdl).hlHdl;
	
	savePort = ChangePort((GrafPtr)wdp);
	
	DrawResHeader(wdp,resHdl);
	
	GetViewRect((WindowPtr)wdp,&area);
	/* Calculate text area without small margin (for redrawing) */
	viewRect = area;
	InsetRect(&viewRect,RES_MARGIN,0);
	
	/* How many lines to be shown */
	visLines = (area.bottom - area.top) / fMonaco.height;

	hitPos = GetCtlValue(wdp->vScroll);	/* first hit on screen */
	/* Check whether buffer contains enough lines */
	if(	(hitPos + visLines > (**resHdl).buftop + MAXBUFLINES) ||
			(hitPos < (**resHdl).buftop) ) {
		/* Refill buffer with new chunk of data */
		StartWaitCursor();
		ret = FillDEBuffer(resHdl,hitPos,TRUE);
		InitCursor();
		if( !ret )
			goto err;
	}

	bufPos = hitPos - (**resHdl).buftop;	/* where in buffer is first line? */

	/* empty drawing area */
	EraseRect(&area);
	
	/* store old clip region */
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);
	
	/* calculate horizontal and vertical drawing positions */
	horizPos = GetCtlValue(wdp->hScroll);
	horiz = area.left + RES_MARGIN - horizPos*fMonaco.finfo.widMax;
					/* this takes horizontal scrolling into account */
	vert = area.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;
	
	/* initialise area for "selected" box (hilighted hits) */
	linebox = area;
	linebox.bottom = linebox.top + fMonaco.height;

	/* Replace clip region by section of clip region and text viewRect area */
	newClipRgn = NewRgn();
	RectRgn(newClipRgn,&viewRect);
	SectRgn(oldClipRgn,newClipRgn,newClipRgn);

	/* now draw line by line and advance printing position */
	for(	i = 0;
			i < visLines && bufPos < MAXBUFLINES && hitPos < (**resHdl).nhits;
			++i, ++bufPos, ++hitPos, vert += fMonaco.height ) {

		SetClip(newClipRgn);
		MoveTo(horiz,vert);
		DrawString( (StringPtr)(*bufHdl)[bufPos] );
	
		/* Reset old clip region */
		SetClip(oldClipRgn);
		
		/* Hilite if selected */
		if( !gInBackground &&
				(WindowPtr)wdp == topWindow && GetSelectState(hlHdl,hitPos) )
			DoInvertRect(&linebox);
		OffsetRect(&linebox,0,fMonaco.height);	/* next line */
	}
	
	DisposeRgn(newClipRgn);		
	DisposeRgn(oldClipRgn);
	
err:
	SetPort(savePort);
	HSetState((Handle)bufHdl,oldState);
}


/**************************************
*	Redraws one line of a result window
*/

static void DrawResWin1(WDPtr wdp, short updown)
{
	GrafPtr			savePort;
	Rect				area,linebox,viewRect;
	short				visLines;
	ResultHdl		resHdl;
	HitlistHdl		hlHdl;
	CString80Hdl	bufHdl;
	WindowPtr		topWindow = FrontWindow();
	short				vert,horiz;
	short				hitPos,bufPos,horizPos;
	SignedByte 		oldState;
	RgnHandle		oldClipRgn,newClipRgn;
	Boolean			ret;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;
	
	/* Locate and lock down result data */
	resHdl = (ResultHdl)(wdp->userHandle);
	if(resHdl == NULL)
		return;
	bufHdl = (**resHdl).descBufHdl;
	oldState = LockHandleHigh((Handle)bufHdl);
	hlHdl = (**resHdl).hlHdl;
			
	savePort = ChangePort((GrafPtr)wdp);
	GetViewRect((WindowPtr)wdp,&area);	
	/* Calculate text area without small margin (for redrawing) */
	viewRect = area;
	InsetRect(&viewRect,RES_MARGIN,0);

	/* How many lines on screen ? */
	visLines = (area.bottom - area.top) / fMonaco.height;
	if (visLines < 1)
		return;

	/* first hit on page = first line on screen */
	hitPos = GetCtlValue(wdp->vScroll);
	/* Check whether buffer contains enough lines */
	if(	(hitPos + visLines > (**resHdl).buftop + MAXBUFLINES) ||
			(hitPos < (**resHdl).buftop) ) {
		/* Refill buffer with new chunk of data */
		StartWaitCursor();
		ret = FillDEBuffer(resHdl,hitPos,TRUE);
		InitCursor();
		if( !ret )
			goto err;
	}
	
	/* calculate horizontal and vertical drawing positions */
	horizPos = GetCtlValue(wdp->hScroll);
	horiz = area.left + RES_MARGIN - horizPos*fMonaco.finfo.widMax;
	vert = area.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;

	/* Draw first line or last line ? */
	if( updown > 0) { /* Down arrown/page => move upwards */
		hitPos += visLines-1;	/* we must redraw last line on screen */
		vert += fMonaco.height * (visLines-1);
	}

	/* Is last line blank? Then we can exit */
	if(hitPos > (**resHdl).nhits)
		goto err;

	bufPos = hitPos - (**resHdl).buftop; /* where in buffer */

	/* store old clip region */
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);
	
	/* Replace clip region by section of clip region and text view area */
	newClipRgn = NewRgn();
	RectRgn(newClipRgn,&viewRect);
	SectRgn(oldClipRgn,newClipRgn,newClipRgn);
	SetClip(newClipRgn);
	DisposeRgn(newClipRgn);	
		
	/* Draw that line */
	MoveTo(horiz,vert);
	DrawString( (StringPtr)(*bufHdl)[bufPos] );
		
	/* Reset old clip region */
	SetClip(oldClipRgn);
	DisposeRgn(oldClipRgn);
	
	/* Is it selected ? */
	if( (WindowPtr)wdp == topWindow && GetSelectState(hlHdl,hitPos) ) {
		linebox = area;
		linebox.bottom = vert + fMonaco.finfo.descent;
		linebox.top = linebox.bottom - fMonaco.height;
		DoInvertRect(&linebox);
	}

err:
	SetPort(savePort);
	HSetState((Handle)bufHdl,oldState);
}


/**************************************
*	(De)Selects all hits and redraws screen
*/

void SelectAllResults(WDPtr wdp,Boolean what)
{
	short			i;
	ResultHdl	resHdl;
	Rect			r;
	GrafPtr		savePort;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;
		
	resHdl = (ResultHdl)(wdp->userHandle);
	if(resHdl == NULL)
		return;

	savePort = ChangePort((GrafPtr)wdp);
	for( i = 0; i < (**resHdl).nhits; ++i)
		SelectDraw(wdp,i,what);
		
	InvalRect(&((WindowPtr)wdp)->portRect);
	GetViewRect((WindowPtr)wdp,&r);
	ValidRect(&r);
	SetPort(savePort);
}


/**************************************
*	(De)Hilites selected records
*/

void SelectDraw(WDPtr wdp, short i, short what)
{
	ResultHdl	resHdl;
	HitlistHdl	hlHdl;
	short			oldState;
	Rect			viewRect;
	short			screenTop,visLines;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;

	resHdl = (ResultHdl)(wdp->userHandle);
	if(resHdl == NULL)
		return;
	hlHdl = (**resHdl).hlHdl;

	oldState = GetSelectState(hlHdl,i);
	SetSelectState(hlHdl,i,what);
	
	/* if new state different from old state, we invert box around record */
	if(oldState != what) {
		GetViewRect((WindowPtr)wdp,&viewRect);
		screenTop = GetCtlValue(wdp->vScroll);	/* first hit on screen */
		visLines = (viewRect.bottom - viewRect.top) / fMonaco.height;
		if(i >= screenTop && i < screenTop + visLines) {
			viewRect.top += (i-screenTop) * fMonaco.height;
			viewRect.bottom = viewRect.top + fMonaco.height;
			DoInvertRect(&viewRect);
		}
	
		if(what ) ++(**resHdl).nsel;
		else --(**resHdl).nsel;
	}
}


/**************************************
*	Redrawing of screen after scrolling or sizing 
*/

void AdjustResText(WindowPtr wPtr,short oldvalue,short newvalue,
						scrollBarType which)
{
	void			(*drawFct)(WDPtr, short);
	Rect			r;
	RgnHandle	updateRgn, clip, eraseRgn;
	ResultHdl	resHdl;
	short			delta = oldvalue-newvalue;
	WDPtr			wdp = FindMyWindow(wPtr);
	GrafPtr		savePort;
	
	if(wPtr == NULL || ((WindowPeek)wPtr)->windowKind != resW)
		return;

	resHdl = (ResultHdl)wdp->userHandle;
	if(resHdl == NULL)
		return;
	
	savePort = ChangePort(wPtr);
	/* calculate drawing area */
	GetViewRect(wPtr,&r);
	
	/* We need a new region for ScrollRect */
	updateRgn  = NewRgn();
	
	if(which == vertBar) {
		/* Scroll screen vertically */
		ScrollRect(&r,0,delta * fMonaco.height,updateRgn);
		/* Redraw complete screen or only one line (top or bottom) ? */
		if(delta == 1 || delta == -1)
			drawFct = DrawResWin1;
		else
			drawFct = DrawResWinAll;
	}
	else {
		/* scroll screen horizontally */
		ScrollRect(&r,delta * fMonaco.finfo.widMax, 0,updateRgn);
		drawFct = DrawResWinAll;	/* redraw complete screen */
	}
	
	/* clip drawing area and do drawing */
	clip = wPtr->clipRgn;
	wPtr->clipRgn = updateRgn;
	(*drawFct)(wdp,-delta);
	wPtr->clipRgn = clip;
	DisposeRgn(updateRgn);
	SetPort(savePort);
}


/**************************************
*	Callback procedure called by TrackControl 
*/

static pascal void	ScrollResProc(ControlHandle theControl, short theCode)
{
	short			oldValue,newValue;
	WindowPtr	ctlWindow=(*theControl)->contrlOwner;
	short			max=GetCtlMax(theControl);
	
	if(theCode == gScrollCode) {
		oldValue=GetCtlValue(theControl);	
		newValue=oldValue+gScrollAmt;
		/* limit check */
		if(newValue < 0) newValue=0;
		if(newValue > max) newValue=max;
			
		SetCtlValue( theControl, newValue );
		AdjustResText(ctlWindow,oldValue,newValue,gScrollDir);
	}
}


/**************************************
*	Handling of key clicks in result window contents region
*/

void DoResClicks(WDPtr wdp, EventRecord *eventPtr)
{
	ControlHandle	theControl;
	short				cntlCode;
	short				oldvalue,newvalue;
	Rect				viewRect;
	short				pagesize;
	Point				where;
	GrafPtr			savePort;

	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;

	savePort = ChangePort((GrafPtr)wdp);
	where = eventPtr->where;
	GlobalToLocal(&where);	/* convert to local coordinates 	*/
	GetViewRect((WindowPtr)wdp,&viewRect);
	
	if (!(cntlCode = FindControl(where, (WindowPtr)wdp, &theControl))) {
		if(PtInRect(where,&viewRect))
			DoResContentClick(eventPtr,&viewRect,wdp);
	}
	else {
		if( theControl == wdp->ctrl1) {
			if( cntlCode == inButton )
				if (TrackControl(theControl,where,NULL))
					ExportRes(wdp);
			SetPort(savePort);
			return;
		}
		else if( theControl == wdp->vScroll ) {	/* click in scroll bar */
			gScrollDir = vertBar;
			pagesize = (viewRect.bottom - viewRect.top) / fMonaco.height;
		}
		else if (theControl == wdp->hScroll) { 
			gScrollDir = horizBar;
			pagesize = (viewRect.right - viewRect.left - 2*RES_MARGIN) /
							fMonaco.finfo.widMax;;
		}
		
		if(cntlCode == inThumb) {
			oldvalue=GetCtlValue(theControl);
			TrackControl(theControl, where, NULL);
			AdjustResText((WindowPtr)wdp,oldvalue,GetCtlValue(theControl),gScrollDir);
		}
		else {
			switch(cntlCode) {
				case inUpButton: gScrollAmt = -1;
					break;
				case inDownButton: gScrollAmt = 1;
					break;
				case inPageUp: gScrollAmt = -pagesize;
					break;
				case inPageDown: gScrollAmt = pagesize;
					break;
			}
			gScrollCode=cntlCode;
			TrackControl(theControl, where, (ProcPtr)ScrollResProc);
		} /* end else (cntlCode == inThumb) */
	} /* end else FindControl */
	SetPort(savePort);
}


/**************************************
*	Load results, check for database version and open window
*	( also called from CheckApp() )
*/

Boolean Load1Results(short w,StringPtr fName, short vRefNum)
{
	Str255		temp;
	WDPtr			wdp;
	QueryHdl		queryHdl;
	HitmapHdl	hitmapHdl;
	ResultHdl	resHdl;
	short			dbcode;
      	
   /* Read in resources from file */
   if(ReadResults(fName,vRefNum,&queryHdl,&hitmapHdl)) {
		wdp = &gWindows[w];
 
   	/* Check for database version  */
 		dbcode = (**queryHdl).dbcode;
		if( strcmp( (**queryHdl).DBRelNum,gDBInfo[dbcode].DBRelNum ) ) {
			DisposHandle((Handle)queryHdl);
			DisposHandle((Handle)hitmapHdl);
			ErrorMsg(ERR_INCVERSION);
			return(FALSE);
		}

		/* Initialise result record */
		if( !InitResultRec(&resHdl, dbcode, hitmapHdl, queryHdl) ) {
			DisposHandle((Handle)queryHdl);
			DisposHandle((Handle)hitmapHdl);
			ErrorMsg(ERR_MEMORY);
			return(FALSE);
		}
		if( !NewResultWindow(w,wdp,resHdl,fName) )
			DisposeResults(wdp);

		wdp->vRefNum = vRefNum;
		wdp->dirty = FALSE;
		wdp->inited = TRUE;
	}
}


/**************************************
*	Communication with user to load results from disk
*/

void LoadResults()
{
	Point 		where;
	SFTypeList	myTypes;
	SFReply 		reply;
	short			w;
	
	if( (w=GetFreeWindow() ) == -1 ) {
		ErrorMsg(ERR_MAXWIN);
		return;
	}

	/* Get file name */
	HandleActivates(FrontWindow(),0);
	CenterSFDlg(getDlgID,&where);
	myTypes[0] = kResFileType;
	SFGetFile(where,"\p",(ProcPtr)resFileFilter,1,myTypes,NULL,
		&reply); 	/* ask for file name */

   if (reply.good == TRUE)	{
   	StartWaitCursor();
   	Load1Results(w,reply.fName,reply.vRefNum);
	  	InitCursor();
   }
}


/**************************************
*	Read result file from disk
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

static Boolean ReadResults(StringPtr fName, short vRefNum,
									QueryHdl *queryHdlP, HitmapHdl *hitmapHdlP)
{
	short			oldVRefNum;
	Str255		volName;
	short			refNum;
	OSErr			err;
	Boolean		ret=FALSE;			/* We assume failure */
	short			curResFile;
	
	/* set default directory */
	GetVol(volName,&oldVRefNum);
	SetVol(NULL,vRefNum);
	
	/* store refnum of current resource file */
	curResFile = CurResFile();
	
	/* Open resource file. We ignore data fork of result file completely */
	if( (refNum=OpenResFile(fName)) == -1) {
		sprintf(gError,LoadErrorStr(ERR_OPENRES,FALSE),
					PtoCstr(fName),ResError());
		CtoPstr((char *)fName);
		ErrorMsg(0);
		goto end;
	}

	/* Make it active resource file and read necessary resources */
	UseResFile(refNum);	
	*queryHdlP = (QueryHdl)Get1Resource(kQueryRsrcType,kStdRsrc);
	if( (err=ResError()) != noErr ) {
		CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_READRES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		ErrorMsg(0);
		goto end;
	}
	
	*hitmapHdlP = (HitmapHdl)Get1Resource(kHitmapRsrcType,kStdRsrc);
	if( (err = ResError()) != noErr ){
		CloseResFile(refNum);
		sprintf(gError,LoadErrorStr(ERR_READRES,FALSE),
					PtoCstr(fName),err);
		CtoPstr((char *)fName);
		ErrorMsg(0);
		goto end;
	}
	
	/* Detach resources and close file */
	DetachResource((Handle)*queryHdlP);
	DetachResource((Handle)*hitmapHdlP);
	CloseResFile(refNum);
	ret=TRUE;

end:
	/* reset old values */
   UseResFile(curResFile);
	SetVol(NULL,oldVRefNum);
	return(ret);
}


/**************************************
*	Filter procedure for SFGetFile. We show only files created by our application
*	(we should also check file type !)
*	Return value:	TRUE, if file is to be hidden
*						FALSE, if file should be displayed in dialog
*/

static pascal Boolean resFileFilter(ParmBlkPtr myPBp) /* only our files */
{
	return( (*myPBp).fileParam.ioFlFndrInfo.fdCreator != kApplSignature );
}


/**************************************
*	Draw header lines in result window
*/

void DrawResHeader(WDPtr wdp, ResultHdl resHdl)
{
	Rect		myRect;
	GrafPtr	savePort;
	char		line[256];
	Str255	str;
	short		border,vert;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;

	savePort = ChangePort((GrafPtr)wdp);
	
	/* Erase top three lines */
	myRect=((WindowPtr)wdp)->portRect;
	myRect.bottom = myRect.top+3*fMonaco.height-1;
	myRect.right = myRect.left + 200;
	EraseRect(&myRect);
	myRect.right = ((WindowPtr)wdp)->portRect.right;
	
	/* use system font */
	TextFont(fSystem.num);
	TextSize(12);
	
	/* Print number of records and how many are selected */
	border = (myRect.bottom-myRect.top-fSystem.height)/2;
	vert = myRect.bottom-border-fSystem.finfo.descent-fSystem.finfo.leading-2;
	GetIndString(str,OTHERS,RESHEADER);
	sprintf(line,PtoCstr(str),(**resHdl).nhits,(**resHdl).nsel);
	MoveTo(RES_MARGIN,vert);
	DrawString(CtoPstr((char *)line));
	
	/* reset window font */
	TextFont(fMonaco.num);
	TextSize(9);
	
	/* Draw double line */
	MoveTo(0,myRect.bottom);
	Line(myRect.right,0);
	MoveTo(0,myRect.bottom-2);
	Line(myRect.right,0);
	
	/* no need to redraw it */
	ValidRect(&myRect);
	
	/* (De)Activate "Export selection" button */
	if( (**resHdl).nsel ) {
		if(wdp->ctrl1)
			HiliteControl(wdp->ctrl1,ACTIVE);
	}
	else {
		if(wdp->ctrl1)
			HiliteControl(wdp->ctrl1,INACTIVE);
	}
	
	SetPort(savePort);
}


/**************************************
*	Show hiliting or remove it (eg if window becomes active/inactive)
*/

void HideShowResSelections(WDPtr wdp, Boolean show)
{
	ResultHdl		resHdl;
	HitlistHdl		hlHdl;
	CString80Hdl	bufHdl;
	SignedByte 		oldState;
	Rect 				viewRect;
	short 			screenTop,visLines;
	short 			i;
	short 			top,horizPos,horiz,vert;
	RgnHandle		oldClipRgn,newClipRgn;
	GrafPtr			savePort;
	Boolean			ret;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != resW)
		return;

	resHdl = (ResultHdl)(wdp->userHandle);
	if(resHdl == NULL)
		return;
	bufHdl = (**resHdl).descBufHdl;
	oldState = LockHandleHigh((Handle)bufHdl);
	hlHdl = (**resHdl).hlHdl;
	
	savePort = ChangePort((GrafPtr)wdp);
	GetViewRect((WindowPtr)wdp,&viewRect);
	top = viewRect.top;

	/* store old clip region */
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);

	/* Replace clip region text view area */
	newClipRgn = NewRgn();
	RectRgn(newClipRgn,&viewRect);
	SetClip(newClipRgn);

	/* calculate horizontal and vertical drawing positions */
	horizPos = GetCtlValue(wdp->hScroll);
	horiz = viewRect.left + RES_MARGIN - horizPos*fMonaco.finfo.widMax;

	screenTop = GetCtlValue(wdp->vScroll);	/* first hit on screen */
	visLines = (viewRect.bottom - viewRect.top) / fMonaco.height;

	/* Check whether buffer contains enough lines */
	if(	(screenTop + visLines > (**resHdl).buftop + MAXBUFLINES) ||
			(screenTop < (**resHdl).buftop) ) {
		/* Refill buffer with new chunk of data */
		StartWaitCursor();
		ret = FillDEBuffer(resHdl,screenTop,TRUE);
		InitCursor();
		if( !ret )
			goto err;
	}

	for(i = screenTop; i < screenTop + visLines && i <(**resHdl).nhits; ++i) {
		if(GetSelectState(hlHdl,i)) {
			viewRect.top = top + (i-screenTop) * fMonaco.height;
			viewRect.bottom = viewRect.top + fMonaco.height;
			EraseRect(&viewRect);
			vert = viewRect.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;
			MoveTo(horiz,vert);
			DrawString( (StringPtr)(*bufHdl)[i - (**resHdl).buftop] );
			if(show)
				DoInvertRect(&viewRect);
			ValidRect(&viewRect);
		}
	}
	
err:
	/* Reset old clip region */
	SetClip(oldClipRgn);
	DisposeRgn(newClipRgn);		
	DisposeRgn(oldClipRgn);
	SetPort(savePort);
	HSetState((Handle)bufHdl,oldState);
}
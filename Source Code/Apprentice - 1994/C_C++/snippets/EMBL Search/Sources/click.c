/*
*********************************************************************
*	
*	Click.c
*	Mouseclicks in window content region
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

#include <string.h>
#include <stdio.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ****************************
*/

#include "click.h"
#include "util.h"
#include "results.h"
#include "hitstorage.h"
#include "sequence.h"

static void ResDragScroll(WindowPtr wPtr, ControlHandle theControl, short scrollAmt);
static short GetClickRow(WDPtr wdp, Rect *viewRectPtr, Point *where);
static Boolean IsResDoubleClick(EventRecord *eventPtr, Rect *viewRectPtr, WDPtr wdp);


/*
********************************* Globals *****************************
*/

extern FONTINFO	fMonaco;
extern ClickInfo	gLastClick;


/**************************************
*	Handle clicks in content region of sequence window
*/

void DoSeqContentClick(EventRecord *eventPtr, Rect *viewRectPtr, WDPtr wdp)
{
	if( wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW )
		return;
		
	/* we don't do anything, but we could allow text selection here */
}

/**************************************
*	Handle clicks in content region of result window
*/

void DoResContentClick(EventRecord *eventPtr,Rect *viewRectPtr,WDPtr wdp)
{
	register 	short	i;
	register 	short	bufPos,newBufPos,anchor;
	short			firstSelectRow,lastSelectRow,sel;
	ResultHdl	resHdl;
	HitlistHdl	hlHdl;
	short 		oldStatus,newStatus;
	Point			where;
	SignedByte	oldRState;
	short			screenTop,visLines;
	GrafPtr		savePort;

	if( wdp == NULL || ((WindowPeek)wdp)->windowKind != resW )
		return;
		
	savePort = ChangePort((GrafPtr)wdp);
	where=eventPtr->where;
	GlobalToLocal(&where);
	
	resHdl = (ResultHdl)(wdp->userHandle);
	hlHdl = (**resHdl).hlHdl;

	visLines = (viewRectPtr->bottom - viewRectPtr->top) / fMonaco.height;
	
	/* find current row */
	bufPos = GetClickRow(wdp,viewRectPtr,&where);
	if( bufPos >= (**resHdl).nhits ) {
	
		/* deselect all */
		oldRState = MyHLock((Handle)resHdl);
		for( i = 0; i < (**resHdl).nhits; i++)
			SelectDraw(wdp,i,FALSE);
		HSetState((Handle)resHdl,oldRState);
		
		/* update header */
		DrawResHeader(wdp, resHdl);
		SetPort(savePort);
		return;
	}

	/* Double click ? */
	if( IsResDoubleClick(eventPtr, viewRectPtr, wdp) ) {
		oldRState = MyHLock((Handle)resHdl);
		for(i=0;i < (**resHdl).nhits; i++) {
			if( GetSelectState(hlHdl,i) ) {
				if( !NewSequenceWindow(resHdl,i) )
					break;
			}
		}
		HSetState((Handle)resHdl,oldRState);
		SetPort(savePort);
		return;
	}

	/* Handle clicks differently depending on modifier keys pressed */
	
	/* no modifiers */
	if( !(eventPtr->modifiers & shiftKey) &&
		!(eventPtr->modifiers & cmdKey) ) { 
		
		/* deselect all */
		oldRState = MyHLock((Handle)resHdl);
		for( i = 0; i < (**resHdl).nhits; i++)
			SelectDraw(wdp,i,FALSE);
		HSetState((Handle)resHdl,oldRState);
			
		/* select and update header */
		SelectDraw(wdp, bufPos, TRUE);
		DrawResHeader(wdp, resHdl);
	
/*		while( StillDown() ) {*/
/*			GetMouse(&where);	*//* GetMouse returns local coordinates *//**/
/*			if (PtInRect(where,viewRectPtr) ) {*/
/*				newBufPos = GetClickRow(wdp,viewRectPtr,&where);*/
/*				if ( newBufPos != bufPos ) {	*//* if moved *//**/
/*					*//* deselect old row and select new row *//**/
/*					if( bufPos < (**resHdl).nhits && newBufPos < (**resHdl).nhits ) {*/
/*						SelectDraw(wdp,bufPos, FALSE);*/
/*						SelectDraw(wdp,newBufPos, TRUE);*/
/*						DrawResHeader(wdp, resHdl);*/
/*					}*/
/*					bufPos = newBufPos;*/
/*				}*/
/*			}*/
/*		}*/
	}
	
	/* command key */
	else if ( eventPtr->modifiers & cmdKey ) {
		/* Get status of selected cell */
		oldStatus = GetSelectState(hlHdl,bufPos);
		newStatus = !oldStatus;
		
		/* select and update header */
		SelectDraw(wdp, bufPos, newStatus);
		DrawResHeader(wdp, resHdl);
		
		while( StillDown() ) {
			screenTop = GetCtlValue(wdp->vScroll);	/* first hit on screen */
			GetMouse(&where);	/* GetMouse returns local coordinates */
			newBufPos = GetClickRow(wdp,viewRectPtr,&where);
			if(newBufPos < screenTop)
				newBufPos = screenTop;
			else if(newBufPos >= screenTop + visLines)
				newBufPos = screenTop + visLines -1;
				
			if(newBufPos >= (**resHdl).nhits)
				newBufPos = (**resHdl).nhits - 1;
			else if(newBufPos < 0)
				newBufPos = 0;
				
			/* select new row(s) */
			if(newBufPos < bufPos) {
				for(i = newBufPos; i<= bufPos; ++i) {
					SelectDraw(wdp,i, newStatus);
				}
			
				DrawResHeader(wdp, resHdl);
				bufPos = newBufPos;
			}
			if(newBufPos > bufPos) {
				for(i = bufPos; i<= newBufPos; ++i) {
					SelectDraw(wdp,i, newStatus);
				}
			
				DrawResHeader(wdp, resHdl);
				bufPos = newBufPos;
			}
			
			if(newBufPos < (**resHdl).nhits - 1 && where.v > viewRectPtr->bottom) {
				SelectDraw(wdp,bufPos = newBufPos + 1,newStatus);
				ResDragScroll((WindowPtr)wdp,(ControlHandle)wdp->vScroll,1);
				DrawResHeader(wdp, resHdl);
			}
			else if(newBufPos > 0 && where.v < viewRectPtr->top) {
				SelectDraw(wdp,bufPos = newBufPos - 1,newStatus);
				ResDragScroll((WindowPtr)wdp,(ControlHandle)wdp->vScroll,-1);
				DrawResHeader(wdp, resHdl);
			}
		}
	}
	
	/* shift key */
	else if ( eventPtr->modifiers & shiftKey ) {
		/* find first and last selected row */
		firstSelectRow = lastSelectRow = -1;
		oldRState = MyHLock((Handle)resHdl);
		for( i = sel = 0; i < (**resHdl).nhits && sel < (**resHdl).nsel; ++i)
			if( GetSelectState(hlHdl,i) ) {
				sel++;
				if (sel == 1)
					firstSelectRow = i;
				if (sel == (**resHdl).nsel)
					lastSelectRow = i;
			}
		HSetState((Handle)resHdl,oldRState);
		
		if( firstSelectRow == -1)
			firstSelectRow = bufPos;	
		if( lastSelectRow == -1) {
			if(bufPos > firstSelectRow)
				lastSelectRow = bufPos;
			else
				lastSelectRow = firstSelectRow;
		}
			
		if( bufPos >= firstSelectRow ) {
			for(i = firstSelectRow; i <= bufPos; ++i)
				SelectDraw(wdp,i,TRUE);
			for(i = bufPos+1; i<=lastSelectRow; ++i)
				SelectDraw(wdp,i,FALSE);
			anchor = firstSelectRow;
		}
		else {
			for(i = bufPos; i <= lastSelectRow; ++i)
				SelectDraw(wdp,i,TRUE);
			anchor = lastSelectRow;
		}
		DrawResHeader(wdp, resHdl);

	/* I wonder whether there is some bug in StillDown() ?!? If I hold the mouse button
		down and move up and down for a while, sometimes this procedure breaks out of the
		while (StillDown()) loop...
	*/
		while( StillDown() ) {
			screenTop = GetCtlValue(wdp->vScroll);	/* first hit on screen */
			GetMouse(&where);	/* GetMouse returns local coordinates */
			newBufPos = GetClickRow(wdp,viewRectPtr,&where);
			if(newBufPos < screenTop)
				newBufPos = screenTop;
			else if(newBufPos >= screenTop + visLines)
				newBufPos = screenTop + visLines -1;
				
			if(newBufPos >= (**resHdl).nhits)
				newBufPos = (**resHdl).nhits - 1;
			else if(newBufPos < 0)
				newBufPos = 0;
				
			if( newBufPos < bufPos ) {
				for(i = bufPos; i>= newBufPos ; --i) {
					if( i <= anchor || i == newBufPos )
						SelectDraw(wdp,i,TRUE);
					else
						SelectDraw(wdp,i, FALSE);
				}

				DrawResHeader(wdp, resHdl);
				bufPos = newBufPos;
			}
			
			if( newBufPos > bufPos ) {
				for(i = bufPos; i<= newBufPos; ++i) {
					if( i >= anchor || i == newBufPos )
						SelectDraw(wdp,i,TRUE);
					else
						SelectDraw(wdp,i, FALSE);
				}
				
				DrawResHeader(wdp, resHdl);
				bufPos = newBufPos;
			}
			
			if(newBufPos < (**resHdl).nhits - 1 && where.v > viewRectPtr->bottom) {
				SelectDraw(wdp,bufPos = newBufPos + 1,TRUE);
				ResDragScroll((WindowPtr)wdp,(ControlHandle)wdp->vScroll,1);
				DrawResHeader(wdp, resHdl);
			}
			else if(newBufPos > 0 && where.v < viewRectPtr->top) {
				SelectDraw(wdp,bufPos = newBufPos - 1,TRUE);
				ResDragScroll((WindowPtr)wdp,(ControlHandle)wdp->vScroll,-1);
				DrawResHeader(wdp, resHdl);
			}
		}
	}
	SetPort(savePort);
}

/**************************************
*	Update result window when user drag-scrolls
*/

static void ResDragScroll(WindowPtr wPtr, ControlHandle theControl, short scrollAmt)
{
	short			oldValue,newValue;
	short			max=GetCtlMax(theControl);
	
	oldValue=GetCtlValue(theControl);
	newValue=oldValue+scrollAmt;
	/* limit check */
	if(newValue < 0) newValue=0;
	if(newValue > max) newValue=max;
			
	SetCtlValue( theControl, newValue );
	AdjustResText(wPtr,oldValue,newValue,vertBar);
}


/**************************************
*	Find line in result buffer corresponding to screen position where click occurred
*	Return value:	line offset in result buffer
*/

static short GetClickRow(WDPtr wdp, Rect *viewRectPtr,Point *where)
{
	short screenRow,hitPos;
	
	hitPos = GetCtlValue(wdp->vScroll);	/* first hit on screen */
	screenRow = (where->v - viewRectPtr->top) / fMonaco.height;
	return(hitPos + screenRow);
}


static Boolean IsResDoubleClick(EventRecord *eventPtr, Rect *viewRectPtr, WDPtr wdp)
{
	Point where1,where2;
	GrafPtr savePort;
	
	if((WindowPtr)wdp != gLastClick.wPtr)
		return(FALSE);
		
	if(eventPtr->when-gLastClick.when > GetDblTime())
		return(FALSE);

	savePort = ChangePort((GrafPtr)wdp);
	where1 = eventPtr->where;
	GlobalToLocal(&where1);
	where2 = gLastClick.where;
	GlobalToLocal(&where2);
	SetPort(savePort);
	
	if(GetClickRow(wdp,viewRectPtr,&where1) ==
		GetClickRow(wdp,viewRectPtr,&where2))
		return(TRUE);
	
	return(FALSE);
}


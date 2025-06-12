/*
*********************************************************************
*	
*	Sequence.c
*	Handling of sequence window
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
#include <ctype.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "sequence.h"
#include "util.h"
#include "window.h"
#include "export.h"
#include "pstr.h"
#include "xref.h"
#include "click.h"

static Boolean OpenSeqWindow(CString80Hdl descBufHdl, short pos, short w, SeqRecHdl seqRecHdl);
static Boolean NewSequenceRec(SeqRecHdl *new);
static Boolean NewSeqLineBuf(CString80Hdl *new);
static Boolean InitSequenceRec(SeqRecHdl *new, short dbcode);
static void DisposeSeqRec(SeqRecHdl seqRecHdl);
static Boolean FillSequenceRec(ResultHdl resHdl, short bufPos, SeqRecHdl seqRecHdl);
static OSErr ReadLine(short refNum, long *count, Ptr buf, StringPtr fName);
static void DrawSeqWin1(WDPtr wdp, short updown);
static pascal void ScrollSeqProc(ControlHandle theControl, short theCode);



/*
******************************** Global variables *****************
*/

extern short		gSysVersion;
extern WDRec		gWindows[MAXWIN];
extern Rect			gSeqWinRect;
extern FONTINFO	fMonaco,fSystem;
extern char			gError[256];
extern short 		gScrollAmt,gScrollCode;
extern Boolean		gScrollDir;
extern DBInfo		gDBInfo[DB_NUM];
extern IndexFiles	gFileList;
extern Prefs 		gPrefs;
extern Boolean		gInBackground;

/**************************************
*	Create and open a new sequence window
*/

Boolean NewSequenceWindow(ResultHdl resHdl,short bufPos)
{
	short				w;
	SeqRecHdl		seqRecHdl;
	CString80Hdl	descBufHdl;

	if( resHdl == NULL )
		return(FALSE);
		
	if( (descBufHdl = (**resHdl).descBufHdl) == NULL )
		return(FALSE);
		
	if( (w=GetFreeWindow()) == -1 )
		return(ErrorMsg(ERR_MAXWIN));

	if( !InitSequenceRec(&seqRecHdl,(**resHdl).dbcode) )
		return(ErrorMsg(ERR_MEMORY));
	
	if( !FillSequenceRec(resHdl,bufPos,seqRecHdl) ) {
		DisposeSeqRec(seqRecHdl);
		DisposHandle((Handle)seqRecHdl);
		return(FALSE);
	}
	else {
		return(OpenSeqWindow(descBufHdl,bufPos-(**resHdl).buftop,w,seqRecHdl));
	}
}


/**************************************
*	Open a new sequence window
*/

static Boolean OpenSeqWindow(CString80Hdl descBufHdl,
						short pos,short w,SeqRecHdl seqRecHdl)
{
	Rect			tempRect;
	WStateData	**stateHdl;
	WDPtr			wdp;
	Str255		title;
	WindowPtr	wPtr;
	
	tempRect = gSeqWinRect;
	AdjustPosition(w,&tempRect);

	wdp = &gWindows[w];
	SetPort(NewWindow(wdp,&tempRect,"\p",FALSE,documentProc + 8,(void *)-1,TRUE,0L));

	wPtr = (WindowPtr)wdp;

	/* anchor sequence record */
	wdp->userHandle = (Handle)seqRecHdl;
			
	ExtractEname(descBufHdl,pos,title);
	SetWTitle(wPtr,title);
	TextFont(fMonaco.num);
	TextSize(9);

	((WindowPeek)wPtr)->windowKind = seqW;
	pstrcpy(wdp->fName,title);
	wdp->vRefNum = 0;
	wdp->dirty = TRUE;
	wdp->inited = FALSE;
	wdp->inUse = TRUE;
	
	/* adjust zoom standard state */
	stateHdl = (WStateData **)((WindowPeek)wPtr)->dataHandle;
	AdjustWSize(seqW,&(**stateHdl).stdState,fMonaco.height,fMonaco.finfo.widMax);
	
	/* draw vertical scroll bar */
	tempRect = wPtr->portRect;
	tempRect.left = tempRect.right-SBarWidth;
	tempRect.right += 1;
	tempRect.top -= 1;
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
	
	AddWindowToMenu(title);
	
	ShowWindow(wPtr);
	return(TRUE);
}



/**************************************
*	Allocates movable memory block for sequence record
*	Return values:
*		TRUE if successful, FALSE on failure
*		By side effect: Handle to this block
*/

static Boolean NewSequenceRec(SeqRecHdl *new)
{
	*new = (SeqRecHdl)NewHandleClear(sizeof(SequenceRec));
	return( *new != NULL );
}


/**************************************
*	Allocates movable memory block for sequence line buffer
*	Return values:
*		TRUE if successful, FALSE on failure
*		By side effect: Handle to this block
*/

static Boolean NewSeqLineBuf(CString80Hdl *new)
{
	*new = (CString80Hdl)NewHandleClear((Size)SEQBUFLINES * sizeof(CString80));
	return( *new != NULL );
}


/**************************************
*	Gets a new sequence record, incl. line buffer, and initialises it
*	Return values:
*		TRUE if successful, FALSE on failure
*		By side effect: Handle to new sequence record
*/

static Boolean InitSequenceRec(SeqRecHdl *new, short dbcode)
{
	SeqRecPtr seqPtr;
	short		i;
	
	if( new == NULL )
		return(FALSE);
		
	/* Allocate memory for sequence record */
	if ( !NewSequenceRec(new) )
		return( FALSE );
	
	/* Get a pointer to it */
	LockHandleHigh((Handle)*new);
	seqPtr = (SeqRecPtr)**new;
	
	/* Allocate memory for sequence line buffer */
	if ( !NewSeqLineBuf(&seqPtr->lineBufHdl) ) {
		DisposHandle((Handle)*new);
		*new = NULL;
		return(FALSE);
	}
	
	/* Init other fields */
	seqPtr->dbcode=dbcode;
	seqPtr->nlines = 0;
	seqPtr->buftop = 0;
	seqPtr->seqStart = 0L;
	for(i=0;i<MAXBUFPARTS;seqPtr->bufpart[i++]=0L)
		;
	seqPtr->xrefHdl = NULL;
	seqPtr->firstSel = seqPtr->lastSel = -1;
	
	HUnlock((Handle)*new);
	return(TRUE);
}


/**************************************
*	Dispose of contents of an existing sequence record
*	Return values:
*		none
*/

static void DisposeSeqRec(SeqRecHdl seqRecHdl)
{
	SeqRecPtr seqPtr;
	
	if(seqRecHdl == NULL)
		return;
	
	/* Get a pointer to the sequence record */
	LockHandleHigh((Handle)seqRecHdl);
	seqPtr = *seqRecHdl;
	
	/* Free memory linked to this record */
	if( seqPtr->lineBufHdl != NULL)
		DisposHandle((Handle)seqPtr->lineBufHdl);
	if( seqPtr->xrefHdl != NULL)
		DisposHandle((Handle)seqPtr->xrefHdl);
		
	HUnlock((Handle)seqRecHdl);
}



/**************************************
*	Close sequence window and dispose of sequence record
*/

void DisposeSequence(WDPtr wdp)
{
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
		
	if (wdp->inUse)
		CloseWindow((WindowPtr)wdp);
		
	if(wdp->userHandle) {
		DisposeSeqRec((SeqRecHdl)wdp->userHandle);
		DisposHandle(wdp->userHandle);
	}
		
	ClearWindowRec(wdp);
}


/**************************************
*	Fill sequence line buffer from disk so that "from" is in buffer.
*	To avoid continous loading when a user browses through the sequence,
*	the buffer is filled (if possible) such that "from" is in the middle of
*	the buffer.
*	Return values:
*		TRUE, if successful,
*		FALSE, if an error occurred
*/

Boolean FillLineBuffer(SeqRecHdl seqRecHdl,short first)
{
	CString80Ptr	buf;
	Str255			fName;
	short				vRefNum,input;
	SignedByte		oldState;
	short				i,part;
	u_long			pos;
	long				size,count;
	DescRec			rec;
	OSErr				err = noErr;
	char				line[80],*s,*t;
	
	if(seqRecHdl == NULL)
		return(FALSE);
		
	if( (**seqRecHdl).lineBufHdl == NULL )
		return(FALSE);
		
	/* calculate first hit to be loaded. The size of the buffer guarantees
		that we always load at least two screen fulls */
	if (first >= SEQBUFLINES/2)
		part = (first - SEQBUFLINES/4) / (SEQBUFLINES/2);
	else part = 0;
	
	/* Open sequence data file */
	pstrcpy(fName,(**seqRecHdl).fName);
	vRefNum = (**seqRecHdl).vRefNum;
	if(OpenMacFileReadOnly(fName,vRefNum,&input, TRUE) != noErr)
		return(FALSE);
	
	/* Lock block */
	oldState = LockHandleHigh((Handle)(**seqRecHdl).lineBufHdl);
	buf = *((**seqRecHdl).lineBufHdl);
	
	if( (err = SetFPos(input,fsFromStart,(**seqRecHdl).bufpart[part])) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(fName),err );
		ErrorMsg(0);
	}
	else {
		for(i=0; i<SEQBUFLINES; ++i) { 
			/* read line */
			count=80;
			if( (err=ReadLine(input,&count,line,fName)) != noErr )
				break;
			
			for(s=line,t=buf[i];*s != '\r';*t++ = *s++)
				;
			*t = EOS;
	
			CtoPstr((char *)buf[i]);
			if( !strncmp(line,"//",2) )
				break;
		}
	}

	FSClose(input);
	HSetState((Handle)(**seqRecHdl).lineBufHdl,oldState);
	(**seqRecHdl).buftop = part*(SEQBUFLINES/2);

	return(err == noErr);
}



/**************************************
*	Reads entry line by line and stores information in SeqRec and lineBuf
*	Return values:
*		TRUE if successful, FALSE on failure
*/

static Boolean FillSequenceRec(ResultHdl resHdl,short bufPos,SeqRecHdl seqRecHdl)
{
	HitlistHdl		hlHdl;
	short				dbcode;
	Str255			indexFName,divFName;
	short				input;
	u_long			rec;
	OSErr				err;
	long				size,count,offset;
	EnameRec			enameRec;
	CString80Ptr	buf;
	SignedByte		oldState;
	short				nlines,part;
	char				line[81];
	char				*s,*t;
	short				maxlines;
	
	if(resHdl == NULL || seqRecHdl == NULL)
		return(FALSE);
				
	if( (hlHdl = (**resHdl).hlHdl) == NULL )
		return(FALSE);
		
	dbcode = (**seqRecHdl).dbcode;
	
	/* Open entryname index  */
	pstrcpy(indexFName,gFileList.enameIdxFName);
	if( OpenMacFileReadOnly(indexFName,gDBInfo[dbcode].InxWDRefNum,&input, TRUE) != noErr )
		return(FALSE);
	
	/* goto appropriate position */
	size = sizeof(EnameRec);
	rec = (*hlHdl)[bufPos].ename_rec;
	
	if( (err=SetFPos(input,fsFromStart,rec*size+sizeof(Header))) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
					PtoCstr(indexFName),err );
		FSClose(input);
		return(ErrorMsg(0));
	}

	/* read record */
	count=size;
	if( ReadMacFile(input,&count,&enameRec,indexFName,TRUE) ) {
		FSClose(input);
		return(FALSE);
	}
	else
		FSClose(input);
	
	/* convert values */
	ConvertLong(&enameRec.annotation_offset);
	ConvertLong(&enameRec.sequence_offset);
	ConvertShort(&enameRec.div_code);
	
	/* Store data file name and vRefNum */	
	pstrcpy(divFName,
		(StringPtr)(*gDBInfo[dbcode].gDivNames + (enameRec.div_code)*(DIVNAMELEN+1)));
	pstrcpy((**seqRecHdl).fName,
		(StringPtr)(*gDBInfo[dbcode].gDivNames + (enameRec.div_code)*(DIVNAMELEN+1)));
	(**seqRecHdl).vRefNum = gDBInfo[dbcode].SeqWDRefNum;
	
	/* Open sequence data file */
	if( OpenMacFileReadOnly(divFName,gDBInfo[dbcode].SeqWDRefNum,&input,TRUE) != noErr )
		return(FALSE);
	
	/* Move to entry */
	offset=enameRec.annotation_offset;
		
	if( (err=SetFPos(input,fsFromStart,offset)) != noErr ) {
		FSClose(input);
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),PtoCstr(divFName),err );
		return(ErrorMsg(0));
	}

	oldState=LockHandleHigh((Handle)(**seqRecHdl).lineBufHdl);
	buf = *((**seqRecHdl).lineBufHdl);
	nlines=0;
	part = 0;
	
	err=noErr;
	maxlines = MAXBUFPARTS * SEQBUFLINES;
	do {
		RotateWaitCursor();
		
		/* store byte offset for every SEQBUFLINES/2th line */
		if( nlines % (SEQBUFLINES/2) == 0 ) {
			if( (err=GetFPos(input,&((**seqRecHdl).bufpart[part]))) != noErr ) {
				sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(divFName),err );
				ErrorMsg(0);
				break;
			}
			else
				++part;
		}
		
		/* read line */
		count=80;
		if( (err=ReadLine(input,&count,line,divFName)) != noErr )
			break;
		
		if(nlines < SEQBUFLINES) {
			for(s=line,t=buf[nlines];*s != '\r';*t++ = *s++)
				;
			*t = EOS;
			CtoPstr((char *)buf[nlines]);
		}
		
		if( linetype(line,"DR") ) {
			if( !ParseDRLine(seqRecHdl,line) ) {
				err = notEnoughMemoryErr;
				break;
			}
		}
			
		if( linetype(line,"SQ") ) {
			if( (err=GetFPos(input,&((**seqRecHdl).seqStart))) != noErr ) {
				sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
						PtoCstr(divFName),err );
				ErrorMsg(0);
				break;
			}
		}
		
		++nlines;
	} while( nlines < maxlines && !linetype(line,"//") );
	
	(**seqRecHdl).nlines = nlines;
	(**seqRecHdl).buftop = 0;
	
	HSetState((Handle)(**seqRecHdl).lineBufHdl,oldState);
	FSClose(input);
	return(err == noErr);
}

/**************************************
*	Reads database entry line by line
*	Return values:
*		OSErr
*/

static OSErr ReadLine(short refNum, long *count, Ptr buf, StringPtr fName)
{
	ParamBlockRec	pb;			
	OSErr			err;
	
	pb.ioParam.ioRefNum		= refNum;
	pb.ioParam.ioCompletion	= NULL;
	pb.ioParam.ioBuffer		= buf;
	pb.ioParam.ioReqCount	= *count;
	pb.ioParam.ioPosMode	=  ('\n' << 8) | 128 ;
	pb.ioParam.ioPosOffset	= 0;
	
	if( (err = PBRead((ParmBlkPtr) &pb, FALSE)) == noErr ) {
		*count = pb.ioParam.ioActCount;
		buf[*count] = EOS;
	}
	else {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),PtoCstr(fName),err);
		CtoPstr((char *)fName);
		ErrorMsg(0);
 	}
		
	return(err);
}


/**************************************
*	Completely redraws a sequence window
*/

void DrawSeqWinAll(WDPtr wdp,short dummy)
{
	GrafPtr			savePort;
	Rect				area,linebox,viewRect;
	short				i,visLines;
	SeqRecHdl		seqRecHdl;
	CString80Hdl	bufHdl;
	WindowPtr		topWindow = FrontWindow();
	short 			vert,horiz;
	short				bufPos,linePos,horizPos;
	SignedByte		oldState;
	RgnHandle		oldClipRgn,newClipRgn;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
			
	/* Locate and lock down sequence data */
	seqRecHdl = (SeqRecHdl)(wdp->userHandle);
	if(seqRecHdl == NULL)
		return;
	bufHdl = (**seqRecHdl).lineBufHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
	
	savePort = ChangePort((GrafPtr)wdp);
		
	/* Calculate text area without small margin (for redrawing) */
	GetViewRect((WindowPtr)wdp,&area);
	viewRect = area;
	InsetRect(&viewRect,SEQ_MARGIN,0);
	
	/* How many lines to be shown */
	visLines = (area.bottom - area.top) / fMonaco.height;

	linePos = GetCtlValue(wdp->vScroll);	/* first hit on screen */
	/* Check whether buffer contains enough lines */
	if(	(linePos + visLines > (**seqRecHdl).buftop +SEQBUFLINES) ||
			(linePos < (**seqRecHdl).buftop) ) {
		/* Refill buffer with new chunk of data */
		if( !FillLineBuffer(seqRecHdl,linePos) )
			goto err;
	}

	bufPos = linePos - (**seqRecHdl).buftop;	/* where in buffer is first line? */

	/* empty drawing area */
	EraseRect(&area);
	
	/* store old clip region */
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);
	
	/* calculate horizontal and vertical drawing positions */
	horizPos = GetCtlValue(wdp->hScroll);
	horiz = area.left + SEQ_MARGIN - horizPos*fMonaco.finfo.widMax;
					/* this takes horizontal scrolling into account */
	vert = area.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;
	
	/* initialise area for "selected" box (hilighted hits) */
	linebox = area;
	linebox.bottom = linebox.top + fMonaco.height;
	
	/* Replace clip region by section of clip region and text view area */
	newClipRgn = NewRgn();
	RectRgn(newClipRgn,&viewRect);
	SectRgn(oldClipRgn,newClipRgn,newClipRgn);

	/* now draw line by line and advance printing position */
	for(	i = 0;
			i < visLines && bufPos < SEQBUFLINES && linePos < (**seqRecHdl).nlines;
			++i, ++bufPos, ++linePos, vert += fMonaco.height ) {

		SetClip(newClipRgn);
		MoveTo(horiz,vert);
		DrawString( (StringPtr)(*bufHdl)[bufPos] );
	
		/* Reset old clip region */
		SetClip(oldClipRgn);
		
		/* Hilite if selected */
		if( !gInBackground &&
				(WindowPtr)wdp == topWindow &&
				linePos >= (**seqRecHdl).firstSel && linePos <= (**seqRecHdl).lastSel )
			DoInvertRect(&linebox);
		OffsetRect(&linebox,0,fMonaco.height);	/* next line */
	}
	
	DisposeRgn(newClipRgn);		
	DisposeRgn(oldClipRgn);
	
err:
	SetPort(savePort);
	HSetState((Handle)bufHdl,oldState);
}


static void DrawSeqWin1(WDPtr wdp, short updown)
{
	GrafPtr			savePort;
	Rect				area,linebox,viewRect;
	short				visLines;
	SeqRecHdl		seqRecHdl;
	CString80Hdl	bufHdl;
	WindowPtr		topWindow = FrontWindow();
	short				vert,horiz;
	short				linePos,bufPos,horizPos;
	SignedByte 		oldState;
	RgnHandle		oldClipRgn,newClipRgn;

	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
		
	/* Locate and lock down sequence data */
	seqRecHdl = (SeqRecHdl)(wdp->userHandle);
	if(seqRecHdl == NULL)
		return;
	bufHdl = (**seqRecHdl).lineBufHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
			
	savePort = ChangePort((GrafPtr)wdp);
	
	/* Calculate text area without small margin (for redrawing) */
	GetViewRect((WindowPtr)wdp,&area);
	viewRect = area;
	InsetRect(&viewRect,SEQ_MARGIN,0);

	/* How many lines on screen ? */
	visLines = (area.bottom - area.top) / fMonaco.height;
	if (visLines < 1)
		return;

	/* first hit on page = first line on screen */
	linePos = GetCtlValue(wdp->vScroll);
	/* Check whether buffer contains enough lines */
	if(	(linePos + visLines > (**seqRecHdl).buftop +SEQBUFLINES) ||
			(linePos < (**seqRecHdl).buftop) ) {
		/* Refill buffer with new chunk of data */
		if( !FillLineBuffer(seqRecHdl,linePos) )
			goto err;
	}
	
	/* calculate horizontal and vertical drawing positions */
	horizPos = GetCtlValue(wdp->hScroll);
	horiz = area.left + SEQ_MARGIN - horizPos*fMonaco.finfo.widMax;
	vert = area.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;

	/* Draw first line or last line ? */
	if( updown > 0) { /* Down arrown/page => move upwards */
		linePos += visLines-1;	/* we must redraw last line on screen */
		vert += fMonaco.height * (visLines-1);
	}

	/* Is last line blank? Then we can exit */
	if(linePos > (**seqRecHdl).nlines)
		goto err;

	bufPos = linePos - (**seqRecHdl).buftop; /* where in buffer */

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
	if( (WindowPtr)wdp == topWindow &&
		 linePos >= (**seqRecHdl).firstSel && linePos <= (**seqRecHdl).lastSel ) {
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
*	Redrawing of screen after scrolling or sizing 
*/

void AdjustSeqText(WindowPtr wPtr,short oldvalue,short newvalue,
						scrollBarType which)
{
	void			(*drawFct)(WDPtr, short);
	Rect			r;
	RgnHandle	updateRgn, clip, eraseRgn;
	SeqRecHdl	seqRecHdl;
	short			delta = oldvalue-newvalue;
	WDPtr			wdp = FindMyWindow(wPtr);
	
	if(wPtr == NULL || ((WindowPeek)wPtr)->windowKind != seqW)
		return;
		
	seqRecHdl = (SeqRecHdl)wdp->userHandle;
	if(seqRecHdl == NULL)
		return;
	
	/* calculate drawing area */
	GetViewRect(wPtr,&r);
	
	/* We need a new region for ScrollRect */
	updateRgn  = NewRgn();
	
	if(which == vertBar) {
		/* Scroll screen vertically */
		ScrollRect(&r,0,delta * fMonaco.height,updateRgn);
		/* Redraw complete screen or only one line (top or bottom) ? */
		if(delta == 1 || delta == -1)
			drawFct = DrawSeqWin1;
		else
			drawFct = DrawSeqWinAll;
	}
	else {
		/* scroll screen horizontally */
		ScrollRect(&r,delta * fMonaco.finfo.widMax, 0,updateRgn);
		drawFct = DrawSeqWinAll;	/* redraw complete screen */
	}
	
	/* clip drawing area and do drawing */
	clip = wPtr->clipRgn;
	wPtr->clipRgn = updateRgn;
	(*drawFct)(wdp,-delta);
	wPtr->clipRgn = clip;
	DisposeRgn(updateRgn);
}


void DoSeqClicks(WDPtr wdp, EventRecord *eventPtr)
{
	ControlHandle	theControl;
	short				cntlCode;
	short				oldvalue,newvalue;
	Rect				viewRect;
	short				pagesize;
	Point				where;
	GrafPtr			savePort;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
		
	savePort = ChangePort((GrafPtr)wdp);
	
	where = eventPtr->where;
	GlobalToLocal(&where);	/* convert to local coordinates 	*/
	GetViewRect((WindowPtr)wdp,&viewRect);
	
	if (!(cntlCode = FindControl(where, (WindowPtr)wdp, &theControl))) {
		if(PtInRect(where,&viewRect))
			DoSeqContentClick(eventPtr,&viewRect,wdp);
	}
	else {
	   if( theControl == wdp->vScroll ) {	/* click in scroll bar */
			gScrollDir = vertBar;
			pagesize = (viewRect.bottom - viewRect.top) / fMonaco.height;
		}
		else if (theControl == wdp->hScroll) { 
			gScrollDir = horizBar;
			pagesize = (viewRect.right - viewRect.left - 2*SEQ_MARGIN) /
							fMonaco.finfo.widMax;;
		}
		
		if(cntlCode == inThumb) {
			oldvalue=GetCtlValue(theControl);
			TrackControl(theControl, where, NULL);
			AdjustSeqText((WindowPtr)wdp,oldvalue,GetCtlValue(theControl),gScrollDir);
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
			TrackControl(theControl, where, (ProcPtr)ScrollSeqProc);
		} /* end else (cntlCode == inThumb) */
	} /* end else FindControl */
	SetPort(savePort);
}


/**************************************
*	Callback procedure called by TrackControl 
*/

static pascal void	ScrollSeqProc(ControlHandle theControl, short theCode)
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
		AdjustSeqText(ctlWindow,oldValue,newValue,gScrollDir);
	}
}


Boolean SaveSeqData(StringPtr fName,short vRefNum,SeqRecHdl seqRecHdl)
{
	char			*inpBuf,*outBuf;
	short			input,output;
	Str255		divFName;
	Boolean		ret=TRUE;
	DialogTHndl	myHandle;
	DialogPtr	myDialog;
	
	if( seqRecHdl == NULL )
		return(FALSE);
		
	/* Allocate file buffers */
	inpBuf=NewPtr((Size)FILEBUF);
	if(inpBuf)
		if( !(outBuf=NewPtr((Size)FILEBUF)) )
			DisposPtr((Ptr)inpBuf);
	if ( !inpBuf || !outBuf )
		return(ErrorMsg(ERR_MEMORY));

	if(gSysVersion >= 0x0700) {
		myHandle = (DialogTHndl)GetResource('DLOG',EXPORT_DLG);
		(**myHandle).procID = movableDBoxProc;
	}
	CenterDA('DLOG',EXPORT_DLG,50);
	myDialog = GetNewDialog(EXPORT_DLG,NULL,(WindowPtr) -1);
	DisableMenuBar();
	
	ParamText(fName,"\p","\p","\p");
	ShowWindow(myDialog);
	DrawDialog(myDialog);

	/* Open sequence data file */
	pstrcpy(divFName,(**seqRecHdl).fName);
	ret = (OpenMacFileReadOnly(divFName,(**seqRecHdl).vRefNum,&input, TRUE) == noErr);
	if(!ret) goto end_proc;
	
	if( !(ret=
		( CreateMacFile(fName,vRefNum,gPrefs.creatorSig,kSeqFileType,TRUE) == noErr )) ) {
		FSClose(input);
		goto end_proc;
	}
		
	if( !(ret= (OpenMacFile(fName,vRefNum, &output, TRUE) == noErr))) {
		FSClose(input);
		goto end_proc;
	}
	
	ret=Export2(input,(**seqRecHdl).bufpart[0],(**seqRecHdl).seqStart,(**seqRecHdl).dbcode,
					divFName,inpBuf,outBuf,output,fName,myDialog);	
	FSClose(input);	
	FSClose(output);
	FlushVol(NULL,vRefNum);

end_proc:
	DisposPtr((Ptr)inpBuf);
	DisposPtr((Ptr)outBuf);
	DisposDialog(myDialog);
	EnableMenuBar();
	return(ret);	
}

/**************************************
*	(De)Selects whole sequence and redraws screen
*/

void SelectAllSeq(WDPtr wdp,Boolean what)
{
	short			i;
	SeqRecHdl	seqHdl;
	Rect			r;
	GrafPtr		savePort;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
		
	seqHdl = (SeqRecHdl)(wdp->userHandle);
	if(seqHdl == NULL)
		return;

	savePort = ChangePort((GrafPtr)wdp);
	if(what) {
		(**seqHdl).firstSel = 0;
		(**seqHdl).lastSel = (**seqHdl).nlines - 1;
	}
	else {
		(**seqHdl).firstSel = (**seqHdl).lastSel = -1;
	}
	
	InvalRect(&((WindowPtr)wdp)->portRect);
	SetPort(savePort);
}


/**************************************
*	Show hiliting or remove it (eg if window becomes active/inactive)
*/

void HideShowSeqSelections(WDPtr wdp, Boolean show)
{
	SeqRecHdl		seqRecHdl;
	SignedByte 		oldState;
	CString80Hdl	bufHdl;
	Rect 				viewRect,linebox;
	short 			screenTop,visLines;
	short 			i;
	short 			top,horizPos,horiz,vert;
	RgnHandle		oldClipRgn,newClipRgn;
	GrafPtr			savePort;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != seqW)
		return;
		
	/* Locate and lock down sequence data */
	seqRecHdl = (SeqRecHdl)(wdp->userHandle);
	if(seqRecHdl == NULL)
		return;
	bufHdl = (**seqRecHdl).lineBufHdl;
	oldState=LockHandleHigh((Handle)bufHdl);
		
	savePort = ChangePort((GrafPtr)wdp);
		
	GetViewRect((WindowPtr)wdp,&viewRect);
	top = viewRect.top;
	
	/* store old clip region */
	oldClipRgn = NewRgn();
	GetClip(oldClipRgn);

	/* Replace clip region by section of clip region and text view area */
	newClipRgn = NewRgn();
	RectRgn(newClipRgn,&viewRect);
	SectRgn(oldClipRgn,newClipRgn,newClipRgn);

	/* calculate horizontal and vertical drawing positions */
	horizPos = GetCtlValue(wdp->hScroll);
	horiz = viewRect.left + SEQ_MARGIN - horizPos*fMonaco.finfo.widMax;
					/* this takes horizontal scrolling into account */
	vert = viewRect.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;
	
	/* initialise area for "selected" box (hilighted hits) */
	linebox = viewRect;
	linebox.bottom = linebox.top + fMonaco.height;

	screenTop = GetCtlValue(wdp->vScroll);	/* first hit on screen */
	visLines = (viewRect.bottom - viewRect.top) / fMonaco.height;

	/* Check whether buffer contains enough lines */
	if(	(screenTop + visLines > (**seqRecHdl).buftop +SEQBUFLINES) ||
			(screenTop < (**seqRecHdl).buftop) ) {
		/* Refill buffer with new chunk of data */
		if( !FillLineBuffer(seqRecHdl,screenTop) )
			goto err;
	}

	for(i = screenTop; i < screenTop + visLines && i <(**seqRecHdl).nlines; ++i) {
		if( i >= (**seqRecHdl).firstSel && i <= (**seqRecHdl).lastSel ) {
			viewRect.top = top + (i-screenTop) * fMonaco.height;
			viewRect.bottom = viewRect.top + fMonaco.height;
			EraseRect(&viewRect);
			vert = viewRect.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;
			MoveTo(horiz,vert);
			DrawString( (StringPtr)(*bufHdl)[i - (**seqRecHdl).buftop] );
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
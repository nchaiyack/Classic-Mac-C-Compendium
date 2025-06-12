/*
*********************************************************************
*	
*	Browse.c
*	Browsing through index
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
******************************* Prototypes ****************************
*/

#include "browse.h"
#include "pstr.h"
#include "util.h"
#include "window.h"
#include "search.h"
#include "bsearch.h"

static pascal void DrawIndex(WindowPtr myWindow, short itemNo);
static Boolean NewBrowseRec(BrowseRecPtr *new);
static Boolean InitBrowseRec(BrowseRecPtr *new, short dbcode, short dbfield, StringPtr fName, short fd);
static pascal Boolean browseDialogFilter(DialogPtr myDialog, EventRecord *theEvent, short *itemHit);
static void LocateWord(DialogPtr myDialog);
static pascal void ScrollResProc(ControlHandle theControl, short theCode);
static void AdjustBrowserVScroll(DialogPtr myDialog);
static pascal void DrawNRec(WindowPtr myWindow, short itemNo);


extern DBInfo		gDBInfo[DB_NUM];
extern IndexFiles	gFileList;
extern char			gError[256];
extern FONTINFO	fMonaco,fSystem;
extern Boolean		gInBackground;

extern short gScrollAmt,gScrollCode;

#define BROWSERLINES	20
#define BROWSERTEXTLEN	60
#define BROWSERHITLEN	7


/**************************************
*	Show an indx browser and handle user actions
*/

Boolean BrowseIndex(short dbcode, short field, StringPtr value)
{
	DialogPtr		myDialog;
	short				itemHit;
	BrowseRecPtr	browsePtr;
	short				vRefNum;
	Str255			targetFName;
	short				fd;
	Boolean			bDone;
	short				which;
	short				kind;
	ControlHandle	findH,scrollH;
	Rect				r;
	Str255			text;
	
	switch(field) {
		case ENAME_QRY:
			pstrcpy(targetFName,gFileList.enameIdxFName);
			break;
		case ACCNUM_QRY:
			pstrcpy(targetFName,gFileList.acnumTrgFName);
			break;
		case KEYWORD_QRY:
			pstrcpy(targetFName,gFileList.keywTrgFName);
			break;
		case FREETEXT_QRY:
			pstrcpy(targetFName,gFileList.textTrgFName);
			break;
		case AUTHOR_QRY:
			pstrcpy(targetFName,gFileList.authorTrgFName);
			break;
		case TAXON_QRY:
			pstrcpy(targetFName,gFileList.taxonTrgFName);
			break;
		default:
			return(FALSE);
	}
	
	/* Open index. We keep it open (in read-only mode) because we use a modal dialog and
		there should be no way of damaging the file */
	vRefNum = gDBInfo[dbcode].InxWDRefNum;
	if(OpenMacFileReadOnly(targetFName,vRefNum,&fd, TRUE) != noErr)
		return(FALSE);

	/* Allocate memory */
	if( !InitBrowseRec(&browsePtr,dbcode,field,targetFName,fd) ) {
		FSClose(fd);
		return(ErrorMsg(ERR_MEMORY));
	}
	
	/* Load first chunk into buffer */
	if( !FillBrowseBuffer(browsePtr,0L)) {
		FSClose(fd);
		DisposPtr((Ptr)browsePtr);
		return(ErrorMsg(ERR_MEMORY));
	}
	
	CenterDA('DLOG',BROWSER_DLG,66);
	myDialog=GetNewDialog(BROWSER_DLG,NULL,(WindowPtr)-1);
	((WindowPeek)myDialog)->refCon = (long)browsePtr;	/* link to buffer */

	/* install user items to draw frame and default button outline */
	InstallUserItem(myDialog,BROWSER_USRITEM1,-1,DrawIndex);
	InstallUserItem(myDialog,BROWSER_USRITEM2,BROWSER_FINDBTN,DrawOKBoxRect);
	InstallUserItem(myDialog,BROWSER_USRITEM3,-1,DrawNRec);
	/* inactivate Find button initially */
	GetDItem(myDialog,BROWSER_FINDBTN,&kind,(Handle *)&findH,&r);
	HiliteControl(findH,INACTIVE);								

	/* adjust scrollbar */
	GetDItem(myDialog,BROWSER_SCROLLBAR,&kind,(Handle *)&scrollH,&r);
	SetCtlMax(scrollH,browsePtr->nrec/browsePtr->scale);
	AdjustBrowserVScroll(myDialog);
	
	ShowWindow(myDialog);
	DrawDialog(myDialog);
	bDone = FALSE;
	while(!bDone) {
		ModalDialog((ProcPtr)browseDialogFilter,&itemHit);
		switch(itemHit) {
			case OK:
				/* refill buffer if necessary (user might haved scrolled on) */
				if(browsePtr->selected < browsePtr->buftop ||
					browsePtr->selected >= browsePtr->buftop + BROWSERLINES)
						FillBrowseBuffer(browsePtr,browsePtr->selected);
						
				/* copy selected text */
				which = browsePtr->selected - browsePtr->buftop;
				strcpy((char *)value,browsePtr->buf[which].text);
				CtoPstr((char *)value);
				bDone = TRUE;
				break;
			case Cancel:
				bDone = TRUE;
				break;
			case BROWSER_FINDBTN:
				/* lookup word */
				LocateWord(myDialog);
				break;
		}
		
		/* activate/deactivate Find button */
		GetDlgText(myDialog,BROWSER_EDITFLD,text);
		if(pstrlen(text) == 0)
			HiliteControl(findH,INACTIVE);
		else 
			HiliteControl(findH,ACTIVE);
	} while (itemHit != OK && itemHit != Cancel);
	
	FSClose(fd);
	
	DisposDialog(myDialog);
	DisposPtr((Ptr)browsePtr);
	
	return(itemHit == OK);
}

/**************************************
*	Draws the main browser field. Because the window is so small, we simply redraw it
*	completely whenever the user moves through the list instead of scrolling like we do
*	for other windows.
*/

static pascal void DrawIndex(WindowPtr myWindow,short itemNo)
{
	DrawBrowserAll(myWindow);
}

/**************************************
*	Allocates memory block for browse record
*	Return values:
*		TRUE if successful, FALSE on failure
*		By side effect: Handle to this block
*/

static Boolean NewBrowseRec(BrowseRecPtr *new)
{
	*new = (BrowseRecPtr)NewPtrClear(sizeof(BrowseRec));
	return( *new != NULL );
}


/**************************************
*	Gets a new browse record, incl. line buffer, and initialises it
*	Return values:
*		TRUE if successful, FALSE on failure
*		By side effect: Handle to new browse record
*/

static Boolean InitBrowseRec(BrowseRecPtr *new, short dbcode, short dbfield,
	StringPtr fName,short fd)
{
	BrowseRecPtr browsePtr;
	short		i;
	
	if( new == NULL )
		return(FALSE);
		
	/* Allocate memory for sequence record */
	if ( !NewBrowseRec(new) )
		return( FALSE );
	
	/* Get a pointer to it */
	browsePtr = *new;
	
	browsePtr->dbcode=dbcode;
	browsePtr->dbfield=dbfield;
	
	switch(dbfield) {
		case ENAME_QRY:
			browsePtr->nrec = gDBInfo[dbcode].ename_nrec;
			browsePtr->recsize = sizeof(EnameRec);
			break;
		case ACCNUM_QRY:
			browsePtr->nrec = gDBInfo[dbcode].actrg_nrec;
			browsePtr->recsize = gDBInfo[dbcode].actrg_recsize;
			break;
		case KEYWORD_QRY:
			browsePtr->nrec = gDBInfo[dbcode].kwtrg_nrec;
			browsePtr->recsize = gDBInfo[dbcode].kwtrg_recsize;
			break;
		case FREETEXT_QRY:
			browsePtr->nrec = gDBInfo[dbcode].texttrg_nrec;
			browsePtr->recsize = gDBInfo[dbcode].texttrg_recsize;
			break;
		case AUTHOR_QRY:
			browsePtr->nrec = gDBInfo[dbcode].authortrg_nrec;
			browsePtr->recsize = gDBInfo[dbcode].authortrg_recsize;
			break;
		case TAXON_QRY:
			browsePtr->nrec = gDBInfo[dbcode].taxontrg_nrec;
			browsePtr->recsize = gDBInfo[dbcode].taxontrg_recsize;
			break;
		default:
			return;
	}

	browsePtr->buftop = browsePtr->screentop = 0;
	pstrcpy(browsePtr->fName,fName);
	browsePtr->fd = fd;
	browsePtr->selected = 0;
	
	/* index files will contain more than SHRT_MAX records so we scale the scroll bar
		appropriately */
	browsePtr->scale = (short)( (browsePtr->nrec-1)/32000L) + 1;
	
	return(TRUE);
}



/**************************************
*	Fill browse line buffer from disk so that "from" is in buffer.
*	To avoid continous loading when a user browses through the index,
*	the buffer is filled (if possible) such that "from" is in the middle of
*	the buffer.
*	Return values:
*		TRUE, if successful,
*		FALSE, if an error occurred
*/

Boolean FillBrowseBuffer(BrowseRecPtr browsePtr,long from)
{
	long				first;
	SignedByte		oldState;
	long				size,count;
	Str255			indexName;
	CString80		text;
	Ptr				recPtr;
	long				nhits;
	long 				i,j,pos;
	short				fd;
	OSErr				err = noErr;
		
	if(browsePtr == NULL)
		return(FALSE);
		
	/* calculate first hit to be loaded. The size of the buffer guarantees
		that we always load at least two screen fulls */
	if (from >= (long)BROWSEBUFLINES/2)
		first = from - (long)BROWSEBUFLINES/2;
	else first = 0L;
	
	size = (long)browsePtr->recsize;	
	if((recPtr = NewPtrClear((Size)size)) == NULL)
		return(FALSE);
		
	pstrcpy(indexName,browsePtr->fName);
	
	fd = browsePtr->fd;
	if( (err = SetFPos(fd,fsFromStart,first*size+sizeof(Header))) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
					PtoCstr(indexName),err );
		ErrorMsg(0);
		DisposPtr((Ptr)recPtr);
		return(FALSE);
	}
		
	for(	i=first,j=0;
			i < first+BROWSEBUFLINES && i < browsePtr->nrec;
			++i,++j) {
		count = size;
		
		if( (err=ReadMacFile(fd,&count,recPtr,indexName,TRUE)) != noErr )
			break;
				
		if(browsePtr->dbfield == ENAME_QRY) {
			nhits = 1;
			strncpy(text,((EnameRec *)recPtr)->entry_name,ENTRYNAMELEN);
			text[ENTRYNAMELEN] = EOS;
		}
		else {
			nhits = ((TargetRec *)recPtr)->nhits;
			ConvertLong((u_long *)&nhits);
			strncpy(text,((TargetRec *)recPtr)->value,size-2*sizeof(u_long));
			text[size-2*sizeof(u_long)] = EOS;
		}

		rtrim(text);
		strcpy(browsePtr->buf[j].text,text);
		browsePtr->buf[j].nhits = nhits;
	}

	DisposPtr((Ptr)recPtr);
	
	browsePtr->buftop = first;

	return(err == noErr);
}

/**************************************
*	Draws the browser dialog
*/

#define BROWSE_MARGIN 4

void DrawBrowserAll(WindowPtr myWindow)
{
	short				kind;
	Handle			h;
	Rect				r,wideBox;
	short				horiz1,horiz2,horiz3,vert,width;
	Rect				linebox;
	long				i,j;
	short 			bufPos;
	BrowseRecPtr	browsePtr;
	PenState			oldPenState;
	WindowPtr		topWindow = FrontWindow();
	Str255			hitStr;
	char				text[256];
	GrafPtr			savePort;
	
	if( (browsePtr = (BrowseRecPtr)((WindowPeek)myWindow)->refCon) == NULL)
		return;
	
	/* Check whether buffer contains enough lines */
	if(	(browsePtr->screentop + BROWSERLINES > browsePtr->buftop + BROWSEBUFLINES) ||
			(browsePtr->screentop < browsePtr->buftop) ) {
		/* Refill buffer with new chunk of data */
		if( !FillBrowseBuffer(browsePtr,browsePtr->screentop) )
			return;
	}

	bufPos = browsePtr->screentop - browsePtr->buftop;	/* where in buffer is first line? */

	savePort = ChangePort(myWindow);
	GetPenState(&oldPenState);
	
	/* Frame main browser field */
	GetDItem(myWindow,BROWSER_USRITEM1,&kind,&h,&r);
	EraseRect(&r);
	InsetRect(&r,-1,-1);
	PenSize(1,1);
	FrameRect(&r);
	InsetRect(&r,1,1);
	
	/* frame browser field and scroll bar in System 7 style */
	wideBox = r;
	wideBox.right += SBarWidth;
	PenSize(2,2);
	InsetRect(&wideBox,-4,-4);
	FrameRect(&wideBox);
	InsetRect(&wideBox,4,4);
	PenSize(1,1);

	TextFont(fMonaco.num);
	TextSize(9);
	
	/* calculate horizontal and vertical drawing positions */
	horiz1 = r.left + BROWSE_MARGIN;
	horiz2 = horiz1 + BROWSERTEXTLEN*fMonaco.finfo.widMax + fMonaco.finfo.widMax/2;
	horiz3 = r.right - BROWSE_MARGIN;
	vert = r.top + fMonaco.finfo.ascent + fMonaco.finfo.leading;
	
	/* initialise area for "selected" box (hilighted hits) */
	linebox = r;
	linebox.bottom = linebox.top + fMonaco.height;

	/* now draw line by line and advance printing position */
	for(	i = 0,j = browsePtr->screentop;
			i < BROWSERLINES && bufPos < BROWSEBUFLINES && j < browsePtr->nrec;
			++i, ++bufPos, ++j, vert += fMonaco.height ) {

		MoveTo(horiz1,vert);
		strncpy(text,browsePtr->buf[bufPos].text,BROWSERTEXTLEN);
		text[BROWSERTEXTLEN] = EOS;
		DrawText(text,0,BROWSERTEXTLEN);
		NumToString(browsePtr->buf[bufPos].nhits,hitStr);
		width = StringWidth(hitStr);	/* right justification */
		MoveTo(horiz3-width+1,vert);
		DrawString(hitStr);
	
		/* Hilite if selected */
		if( !gInBackground &&
				myWindow == topWindow && j == browsePtr->selected ) {
			DoInvertRect(&linebox);
		}
		OffsetRect(&linebox,0,fMonaco.height);	/* next line */
	}
	
	PenSize(1,1);
	MoveTo(horiz2,r.top);
	LineTo(horiz2,r.bottom);
	
	SetPenState(&oldPenState);
	TextFont(fSystem.num);
	TextSize(12);
	
	SetPort(savePort);
}

/**************************************
*	Filter proc for browse modal dialog.
*	Notice that default button is *not* the OK button in this dialog.
*	Return value:	TRUE, if we handled the event
*						FALSE, if not
*/

static pascal Boolean browseDialogFilter(DialogPtr myDialog,EventRecord *theEvent,
										short *itemHit)
{
	char				charCode;
	short				keyCode;
	BrowseRecPtr	browsePtr;
	short				kind;
	Handle			h;
	ControlHandle	scrollH;
	Rect				r,textR,box;
	GrafPtr			savePort;
	Point				where;
	short				line,horiz;
	Boolean			ret = FALSE;
	long				oldSelected, oldScreentop;
	Str255			word;
	ControlHandle	theControl;
	short				cntlCode,oldValue;

	/* to detect double clicks we keep some information about the previous click */
	static short	prevLine = -1;
	static long		prevTime = 0;

#define HOME		0x73
#define END			0x77
#define PG_UP		0x74
#define PG_DN		0x79
	
	if( (browsePtr = (BrowseRecPtr)((WindowPeek)myDialog)->refCon) == NULL)
		return(FALSE);
		
	switch(theEvent->what) {
		case keyDown:
		case autoKey:
	      charCode=(char)(theEvent->message & charCodeMask);
	      keyCode = (theEvent->message & keyCodeMask) >> 8;
	      
	      if(CmdPeriod(theEvent) || keyCode == ESCAPE) {
				FakeClick(myDialog,Cancel);
	         *itemHit=Cancel;
	      	return(TRUE);
	   	}
	   	
	   	switch(keyCode) {
				case PG_UP:
				case PG_DN:
				case HOME:
				case END:	/* scroll without moving the selection */
					oldScreentop = browsePtr->screentop;
					
					if(keyCode == PG_DN)
						browsePtr->screentop += BROWSERLINES - 1; /* one line overlap */
					else if( keyCode == PG_UP )
						browsePtr->screentop -= BROWSERLINES - 1;
					else if( keyCode == HOME )
						browsePtr->screentop = 0;
					else browsePtr->screentop = browsePtr->nrec-BROWSERLINES;
					
					if( browsePtr->screentop < 0 )
						browsePtr->screentop = 0;
					if( browsePtr->screentop > browsePtr->nrec-BROWSERLINES )
						browsePtr->screentop = browsePtr->nrec-BROWSERLINES;
					
					/* redraw if necessary */
					if(oldScreentop != browsePtr->screentop) {
						savePort = ChangePort(myDialog);
	 					GetDItem(myDialog,BROWSER_USRITEM1,&kind,&h,&textR);
						InvalRect(&textR);
						SetPort(savePort);
					
						AdjustBrowserVScroll(myDialog);
					}
					
					*itemHit = 0; /* dummy */
					return(TRUE);
					break;
			}
	   	
	   	switch(charCode) {
		   	case 3:
		   	case 13:	/* Return or Enter ? */
					FakeClick(myDialog,BROWSER_FINDBTN);	
					*itemHit = BROWSER_FINDBTN;
		         return(TRUE);
					break;
				case 0x1E:	/* up arrow */
				case 0x1F:	/* down arrow */		/* move selection */
					oldSelected = browsePtr->selected;

					if(charCode == 0x1F ) 
						browsePtr->selected += 1;
					else browsePtr->selected -= 1;
					
					if( browsePtr->selected < 0 )
						browsePtr->selected = 0;
					if( browsePtr->selected >= browsePtr->nrec )
						browsePtr->selected = browsePtr->nrec - 1;
						
					/* redraw if necessary */
					if(oldSelected != browsePtr->selected) {
						savePort = ChangePort(myDialog);
	 					GetDItem(myDialog,BROWSER_USRITEM1,&kind,&h,&textR);

	 					/* scroll, if necessary */
						if(browsePtr->selected < browsePtr->screentop) {
							browsePtr->screentop = browsePtr->selected;
							InvalRect(&textR);
						}
						else if(browsePtr->selected >= browsePtr->screentop + BROWSERLINES) {
							browsePtr->screentop = browsePtr->selected - BROWSERLINES + 1;
							InvalRect(&textR);
						}
						/* if no scrolling necessary, unhilite old selection and hilite new */
						else {
							box = textR;
							box.top += (oldSelected-browsePtr->screentop) * fMonaco.height;
							box.bottom = box.top + fMonaco.height;
							DoInvertRect(&box);
							
							/* hilite new selection */
							box = textR;
							box.top += (browsePtr->selected-browsePtr->screentop)
											*fMonaco.height;
							box.bottom = box.top + fMonaco.height;
							DoInvertRect(&box);
		
							horiz = textR.left + BROWSE_MARGIN +
									(BROWSERTEXTLEN + 1)*fMonaco.finfo.widMax;
							MoveTo(horiz-fMonaco.finfo.widMax/2,textR.top);
							LineTo(horiz-fMonaco.finfo.widMax/2,textR.bottom);
						}
						
						SetPort(savePort);
						AdjustBrowserVScroll(myDialog);
					}

					*itemHit = 0; /* dummy */
					return(TRUE);
					break;
			}
				
	   	return(FALSE);
	   	break;
	   	
	   case mouseDown:
	   	ret = FALSE;
	   	line = -1;
	   	
	   	where=theEvent->where;
	   	savePort = ChangePort(myDialog);
	   	GlobalToLocal(&where);
			GetDItem(myDialog,BROWSER_USRITEM1,&kind,&h,&textR);

			/* check whether click occured in a control */
			if (!(cntlCode = FindControl(where, (WindowPtr)myDialog, &theControl))) {
				/* no, so check whether click occurred in browser text field */
				if(PtInRect(where,&textR)) {
					*itemHit = 0; /* dummy */
					
					/* find line where click occurred */
					line = (where.v-textR.top)/fMonaco.height;
					/* in case of double click we fake a click on the OK button */
					if(theEvent->when-prevTime <= GetDblTime() && line == prevLine) {
						browsePtr->selected = browsePtr->screentop+line;
						*itemHit = OK;
					}
					else {
						/* if click is not on the same line as currently selected */
						if(browsePtr->screentop+line != browsePtr->selected) {
							/* unhilite old selection if visible */
							if(browsePtr->selected >= browsePtr->screentop &&
								browsePtr->selected < browsePtr->screentop+BROWSERLINES) {
								box = textR;
								box.top += (browsePtr->selected-browsePtr->screentop)*fMonaco.height;
								box.bottom = box.top + fMonaco.height;
								DoInvertRect(&box);
							}
							
							/* hilite new selection */
							browsePtr->selected = browsePtr->screentop+line;
							box = textR;
							box.top += (browsePtr->selected-browsePtr->screentop)*fMonaco.height;
							box.bottom = box.top + fMonaco.height;
							DoInvertRect(&box);
		
							horiz = textR.left + BROWSE_MARGIN +
									(BROWSERTEXTLEN + 1)*fMonaco.finfo.widMax;
							MoveTo(horiz-fMonaco.finfo.widMax/2,textR.top);
							LineTo(horiz-fMonaco.finfo.widMax/2,textR.bottom);
						}
					}
					ret = TRUE;
				}		
			}
			else {		/* if user clicked into a control */
				/* click in scroll bar ? */
				GetDItem(myDialog,BROWSER_SCROLLBAR, &kind,(Handle *)&scrollH,
							&r);
				if( theControl == scrollH ) {
					oldScreentop = browsePtr->screentop;
					
					/* keep the old CtlValue for the following reason:
						If user moves the thumb we recover screentop by looking at CtlValue.
						Imagine user moved the thumb to the end of list. Then we set screentop
						to nrec-BROWSERLINES and set CtlValue to screentop/scale.
						Next time a user clicks on the thumb but doesn't move it, CtlValue
						is still the same but if we recalculated screentop = CtlValue * scale,
						we might get a different value for screentop due the the scaling
						effect (no floating point division is used for screentop/scale).
						Therefore, we only recalculate screentop if the user actually moved the
						thumb.
					*/ 
					oldValue = GetCtlValue(scrollH);
					
					if(cntlCode == inThumb) {
						TrackControl(theControl, where, NULL);
						if(oldValue != GetCtlValue(scrollH)) {
							/* only if user moved thumb: */
							browsePtr->screentop = (u_long)GetCtlValue(scrollH)*browsePtr->scale;
							if( browsePtr->screentop < 0 )
								browsePtr->screentop = 0;
							if( browsePtr->screentop > browsePtr->nrec-BROWSERLINES )
								browsePtr->screentop = browsePtr->nrec-BROWSERLINES;
						
							if(oldScreentop != browsePtr->screentop) {
	 							GetDItem(myDialog,BROWSER_USRITEM1,&kind,&h,&textR);
								InvalRect(&textR);

								AdjustBrowserVScroll(myDialog);
							}
						}
					}
					else {	/* click in page up/down or arrow up/down control part */
						switch(cntlCode) {
							case inUpButton: gScrollAmt = -1;
								break;
							case inDownButton: gScrollAmt = 1;
								break;
							case inPageUp: gScrollAmt = -(BROWSERLINES-1);
								break;
							case inPageDown: gScrollAmt = BROWSERLINES-1;
								break;
						}
						gScrollCode=cntlCode;
						
						/* track user action and redraw if necessary */
						TrackControl(theControl, where, (ProcPtr)ScrollResProc);
					} /* end else (cntlCode == inThumb) */
										
					*itemHit = 0; /* dummy */
					ret = TRUE;
				}
			} /* end else FindControl */
			
			/* save relevant information for next click */
			prevTime = theEvent->when;
			prevLine = line;
			SetPort(savePort);
			return(ret);
			break;
  }
   
   return(FALSE);
}

/**************************************
*	Look through the index for a specified word.
*/

static void LocateWord(DialogPtr myDialog)
{
	short				kind;
	Handle			h;
	Rect				r;
	short				size;
	long				pos,count;
	Ptr				recPtr;
	short				(*compare)(void *,void *);
	OSErr				err;
	Boolean			bDone = FALSE;
	Str255			findWord;
	GrafPtr			savePort;
	BrowseRecPtr	browsePtr;
	
	if( (browsePtr = (BrowseRecPtr)((WindowPeek)myDialog)->refCon) == NULL)
		return;

	size = (long)browsePtr->recsize;	
	if((recPtr = NewPtrClear((Size)size)) == NULL)
		return;
			
	/* compare routine depends on database field to be searched */	
	if(browsePtr->dbfield == ENAME_QRY)
		compare = ename_compare;
	else
		compare = general_compare;

	/* get word to look for */
	GetDlgText(myDialog,BROWSER_EDITFLD,findWord);
	if(pstrlen(findWord) == 0)	return;
	
	PtoCstr(findWord);
	str2upper((char *)findWord);
	
	/* do the binary search */
	StartWaitCursor();
	!CDIndex_BSearch(findWord,
				browsePtr->fName,
				browsePtr->fd,
				(u_long)browsePtr->recsize,
				(u_long)browsePtr->nrec,
				compare,
				recPtr, (u_long *)&pos);
				
	/* now look backwards for first occurrence. Yes, this is inefficient, I know... */			
	do {
		--pos;
		if( (err=SetFPos(browsePtr->fd,fsFromStart,
					pos*(long)browsePtr->recsize+sizeof(Header))) != noErr) {
			sprintf(gError,LoadErrorStr(ERR_READFILE,FALSE),
					PtoCstr(browsePtr->fName),err );
			CtoPstr((char *)browsePtr->fName);
			ErrorMsg(0);
			break;
		}

		count = (long)browsePtr->recsize;
		if( ReadMacFile(browsePtr->fd,&count,(void *)recPtr,browsePtr->fName,TRUE) ) {
			break;
		}
			
		if(compare(findWord,recPtr))
			bDone = TRUE;
	} while ( !bDone );
	InitCursor();

	DisposPtr((Ptr)recPtr);
	browsePtr->selected = browsePtr->screentop = pos + 1;
	
	/* adjust screen top */
	if( browsePtr->screentop < 0 )
		browsePtr->screentop = 0;
	if( browsePtr->screentop > browsePtr->nrec-BROWSERLINES )
		browsePtr->screentop = browsePtr->nrec-BROWSERLINES;
		
	savePort = ChangePort(myDialog);
 	GetDItem(myDialog,BROWSER_USRITEM1,&kind,&h,&r);
	InvalRect(&r);

	AdjustBrowserVScroll(myDialog);

	SetPort(savePort);
}

/**************************************
*	This procedure continuously updates the window as long as user holds mouse down
*	in scroll bar 
*/

static pascal void ScrollResProc(ControlHandle theControl, short theCode)
{
	WindowPtr		ctlWindow=(*theControl)->contrlOwner;
	BrowseRecPtr	browsePtr;
	long				oldScreentop;
	
	if(theCode == gScrollCode) {
		browsePtr = (BrowseRecPtr)((WindowPeek)ctlWindow)->refCon;
		oldScreentop = browsePtr->screentop;
		
		browsePtr->screentop += gScrollAmt;
		/* limit check */
		if( browsePtr->screentop < 0 )
			browsePtr->screentop = 0;
		if( browsePtr->screentop > browsePtr->nrec-BROWSERLINES )
			browsePtr->screentop = browsePtr->nrec-BROWSERLINES;
				
		/* only redraw if necessary. Avoids flickering at top and end of list */
		if(oldScreentop != browsePtr->screentop) {
			DrawBrowserAll(ctlWindow);
			AdjustBrowserVScroll(ctlWindow);
		}
	}
}


/**************************************
*	Adjust CtlValue according to current screentop value taking scaling into account
*/

static void AdjustBrowserVScroll(DialogPtr myDialog)
{
	short				kind;
	Handle			h;
	Rect				r;
	BrowseRecPtr	browsePtr;

	if( (browsePtr = (BrowseRecPtr)((WindowPeek)myDialog)->refCon) == NULL)
		return;
		
	GetDItem(myDialog,BROWSER_SCROLLBAR,&kind,&h,&r);	/* get handle to scroll bar */
	SetCtlValue((ControlHandle)h,browsePtr->screentop/browsePtr->scale);
}

/**************************************
*	Show number of items in index
*/

static pascal void DrawNRec(WindowPtr myWindow,short itemNo)
{
	short				kind;
	Handle			h;
	Rect				r;
	BrowseRecPtr	browsePtr;
	Str255			text,str;
	
	if( (browsePtr = (BrowseRecPtr)((WindowPeek)myWindow)->refCon) == NULL)
		return;
		
	GetDItem(myWindow,itemNo,&kind,&h,&r);	/* get drawing position */
	
	TextFont(fMonaco.num);
	TextSize(9);
	
	NumToString(browsePtr->nrec,text);
	GetIndString(str,OTHERS,ITEMSINLIST);
	pstrcat(text,str);
	
	MoveTo(r.left,r.bottom);
	DrawString(text);
	
	TextFont(fSystem.num);
	TextSize(12);
}

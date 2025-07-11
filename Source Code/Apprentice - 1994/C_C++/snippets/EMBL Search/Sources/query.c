/*
*********************************************************************
*	
*	Query.c
*	Handling of query screen
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
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

#include "query.h"
#include "util.h"
#include "menus.h"
#include "events.h"
#include "window.h"
#include "pstr.h"
#include "browse.h"
#include "search.h"
#include "hitstorage.h"
#include "results.h"

static Boolean NewQueryRec(QueryHdl *new);
static Boolean ReadQuery(StringPtr fName, short vRefNum, QueryHdl *queryHdlP);
static void HandleQueryEvt(DialogPtr myDialog, short itemHit);
static void SetBoolTxt(DialogPtr myDialog, short firstItem, short gBooleanOp);
static short DB2Button(QueryHdl queryHdl);
static short Button2DB(short theItem);
static short Bool2Button(QueryHdl queryHdl);
static short Button2Bool(short theItem);
static pascal void DrawQueryUsrItems(WindowPtr wPtr, short itemNo);
static void DrawTriangle(Rect *boxPtr);
static void HandlePopUpMenu(DialogPtr myDialog, short itemHit, QueryHdl queryHdl);
static void ExecuteQuery(WDPtr wdp);
static void CheckSqDBComp(DialogPtr myDialog, QueryHdl queryHdl);


/*
********************************* Globals *****************************
*/

extern DBInfo		gDBInfo[DB_NUM];
extern char			gError[256];
extern WDRec		gWindows[MAXWIN];
extern Rect			gQueryWinRect;
extern Boolean		gInBackground;
extern MenuHandle	gMenu[MENUNR];
extern short		gResNo;

short gQueryNo;


/**************************************
*	Handle events in query window. Returns TRUE, if event was handled, or FALSE
*	if event will have to be handled by some other routine.
*/

Boolean DoQueryEvt(EventRecord theEvent)
{
	DialogPtr myDialog;
	short itemHit;
	Boolean isHandled = FALSE;
	
	/* Do necessary pre-processing before we call DialogSelect */
	
  	switch (theEvent.what) {
		case keyDown:
		case autoKey:
			MaintainMenus();
			if (theEvent.modifiers & cmdKey) {
				DoEvent(theEvent);
				isHandled = TRUE;
			}
			else {
				switch(theEvent.message & charCodeMask) {
					case 0x09:	/* tab */
					case 0x1E:	/* up arrow */
					case 0x1F:	/* down arrow */
					case 0x1C:	/* left arrow */
					case 0x1D:	/* right arrow */
						break;
					case 0x03:	/* enter */
					case 0x0D:	/* return */
						ExecuteQuery((WDPtr)FrontWindow());
						isHandled = TRUE;
						break;
				}
			}
			break;
	} /* end switch */

	if(!isHandled) {
		if(isHandled = DialogSelect(&theEvent,&myDialog,&itemHit)) {
			((WDPtr)myDialog)->dirty = TRUE; /* not absolutely correct :-( */
			HandleQueryEvt(myDialog,itemHit);
		}
	}
	
	return(isHandled);
}


/**************************************
*	Open a new query window, do necessary initialisation
*/

void NewQuery()
{
	QueryHdl	queryHdl;
	Str255	title;
	char		temp[256];
	short		w;
	WDPtr		wdp;
	
	if( (w=GetFreeWindow()) == -1 ) {
		ErrorMsg(ERR_MAXWIN);
		return;
	}
	else
		wdp = &gWindows[w];
		
	if( !NewQueryBlock(DB_EMBL, &queryHdl, BOOLEAN_AND) ) {
		ErrorMsg(ERR_MEMORY);
		return;
	}

	/* Open window */
	GetIndString(title,OTHERS,QUERYTITLE);
	sprintf(temp,PtoCstr(title),++gQueryNo);
		
	if( !NewQueryWindow(w,wdp,queryHdl, CtoPstr((char *)temp)) ) {
		DisposHandle((Handle)queryHdl);
	} /* end if !NewResultsWindow() */
}		


/**************************************
*	Allocates a new query record and initialises it
*	Return values:
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side effect:	Handle to new query record
*/

Boolean NewQueryBlock(short dbcode, QueryHdl *new, short gBooleanOp)
{
	QueryPtr queryPtr;
	short i;
	
	/* Allocate memory for query record */
	if ( !NewQueryRec(new) )
		return( FALSE );
	
	/* Get a pointer to it */
	LockHandleHigh((Handle)*new);
	queryPtr = (QueryPtr)**new;

	/* Init fields */
	queryPtr->magic = TickCount();
	queryPtr->dbcode = dbcode;
	strcpy( queryPtr->DBRelNum,gDBInfo[dbcode].DBRelNum );
	queryPtr->gBooleanOp = gBooleanOp;
	for(i=0;i<MAXSUBQRY;++i) {
		*(queryPtr->subQuery[i].text) = EOS;
		queryPtr->subQuery[i].field = ENAME_QRY;
	}

	HUnlock((Handle)*new);
	return(TRUE);
}

/**************************************
*	Allocates relocatable memory block for query record
*	Return value:	TRUE, if successful
*						FALSE, if error occurred
*	Side-effect:	Handle to this block
*/

static Boolean NewQueryRec(QueryHdl *new)
{
	*new = (QueryHdl)NewHandleClear(sizeof(QueryRec));
	return( *new != NULL );
}


/**************************************
*	Open a new query window
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean NewQueryWindow(short w,WDPtr wdp, QueryHdl queryHdl, StringPtr title)
{
	register short i;
	Rect			tempRect;
	DialogPtr	dPtr;
	DialogTHndl	myHandle;
	SignedByte	oldState;

	if( queryHdl == NULL || wdp == NULL)
		return(FALSE);
		
	oldState = MyHLock((Handle)queryHdl);
		
	myHandle=(DialogTHndl)GetResource('DLOG',QUERY_DLG);
	if(myHandle == NULL) return(FALSE);
	(**myHandle).boundsRect = gQueryWinRect;
	AdjustPosition(w,&(**myHandle).boundsRect);
	
	dPtr = GetNewDialog(QUERY_DLG,wdp,(void *)-1);

	/* anchor query record */
	wdp->userHandle = (Handle)queryHdl;
				
	SetWTitle((WindowPtr)wdp,title);

	SelRadioButton(dPtr,QRY_EMBL_BTN,2,DB2Button(queryHdl));
	SelRadioButton(dPtr,QRY_AND_BTN,2,Bool2Button(queryHdl));
	SetBoolTxt(dPtr,QRY_BOOLSTR1+1,(**queryHdl).gBooleanOp);

	InstallUserItem(dPtr,QRY_USRITEM,OK,DrawOKBoxRect);
	for(i=0;i<MAXSUBQRY;++i)
		InstallUserItem(dPtr,QRY_POPUP1+i,-1,DrawQueryUsrItems);

	for(i = 0; i< MAXSUBQRY; ++i)
		SetDlgText(dPtr,QRY_SUBQRY1 + i,(**queryHdl).subQuery[i].text);

	pstrcpy(wdp->fName,title);
	wdp->vRefNum = 0;
	wdp->dirty = TRUE;
	wdp->inited = FALSE;
	wdp->inUse = TRUE;
	
	AddWindowToMenu(title);
	
	HSetState((Handle)queryHdl,oldState);
	
	ShowWindow(dPtr);
	SetPort(dPtr);
	return(TRUE);
}


/**************************************
*	Close query window and dispose of query record
*/

void DisposeQuery(WDPtr wdp)
{
	if (wdp == NULL || ((WindowPeek)wdp)->windowKind != queryW)
		return;
		
	if (wdp->inUse)
		CloseDialog((DialogPtr)wdp);
		
	if(wdp->userHandle) {
		DisposHandle((Handle)wdp->userHandle);
	}
			
	ClearWindowRec(wdp);
}

/**************************************
*	Updates query record before we duplicate or start a query.
*/

void UpdateQueryRec(WDPtr wdp)
{
	QueryHdl		queryHdl;
	SignedByte	oldState;
   short			itemType;
   Handle		item;
   Rect			box;
   Str255		text;
   register short i;
	
	if (wdp == NULL || ((WindowPeek)wdp)->windowKind != queryW)
		return;
		
	if((queryHdl = (QueryHdl)wdp->userHandle) == NULL)
		return;
		
	oldState = MyHLock((Handle)queryHdl);
	
	for(i = 0; i< MAXSUBQRY; ++i) {
   	GetDItem((DialogPtr)wdp, QRY_SUBQRY1 + i,&itemType,&item,&box);
   	GetIText(item,text);
   	pstrcpy((**queryHdl).subQuery[i].text,text);
	}
	
	(**queryHdl).magic = TickCount();
	
	HSetState((Handle)queryHdl,oldState);
}


/**************************************
*	Duplicate a query to a new window
*/

void DuplicateQuery(WDPtr wdp)
{
	SignedByte	oldState;
	Str255		title;
	char			temp[256];
	QueryHdl		queryHdl, newQueryHdl;
	short			w;
	WDPtr			queryWdp;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != dialogKind)
		return;
	
	if((queryHdl = (QueryHdl)wdp->userHandle) == NULL)
		return;

	if( (w=GetFreeWindow()) == -1 ) {
		ErrorMsg(ERR_MAXWIN);
		return;
	}
	else
		queryWdp = &gWindows[w];
			
	oldState = MyHLock((Handle)queryHdl);
	
	/* update query record */
	UpdateQueryRec(wdp);

	/* copy query record */
	newQueryHdl = queryHdl;
	if(HandToHand((Handle *)&newQueryHdl) != noErr) {
		ErrorMsg(ERR_MEMORY);
	}
	
	/* Open window */
	GetIndString(title,OTHERS,QUERYTITLE);
	sprintf(temp,PtoCstr(title),++gQueryNo);
		
	if( !NewQueryWindow(w,queryWdp,newQueryHdl, CtoPstr((char *)temp)) ) {
		DisposHandle((Handle)newQueryHdl);
	}

	HSetState((Handle)queryHdl,oldState);
}


/**************************************
*	Communication with user to load old query from disk
*/

void OpenQuery()
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
	myTypes[0] = kQryFileType;
	SFGetFile(where,"\p",NULL,1,myTypes,NULL,&reply); 	/* ask for file name */

   if (reply.good == TRUE)	{
   	StartWaitCursor();
   	LoadQuery(w,reply.fName,reply.vRefNum);
	  	InitCursor();
   }
}

/**************************************
*	Load queries, check for database version and open window
*	( also called from CheckApp() )
*/

Boolean LoadQuery(short w,StringPtr fName, short vRefNum)
{
	Str255		temp;
	WDPtr			wdp;
	QueryHdl		queryHdl;
	short			dbcode;
	short			field;
	register short i;
	  	
   /* Read in resources from file */
   if(ReadQuery(fName,vRefNum,&queryHdl)) {
		wdp = &gWindows[w];
 
   	/* Update database version  */
 		dbcode = (**queryHdl).dbcode;
 		strcpy((**queryHdl).DBRelNum,gDBInfo[dbcode].DBRelNum );

		/* here we have to check for non-existing values in queryField.field,
			eg if a query is used with an older CD-ROM */
			
		for(i = 0; i<MAXSUBQRY; ++i) {
			field = (**queryHdl).subQuery[i].field;
			
			switch(field) {
				case ENAME_QRY:	/* must always exist */
					break;
				case ACCNUM_QRY:
					if(gDBInfo[dbcode].actrg_recsize == 0) {
						DisposHandle((Handle)queryHdl);
						return(ErrorMsg(ERR_INCQUERY));
					}
					break;
				case KEYWORD_QRY:
					break;
					if(gDBInfo[dbcode].kwtrg_recsize == 0) {
						DisposHandle((Handle)queryHdl);
						return(ErrorMsg(ERR_INCQUERY));
					}
					break;
				case FREETEXT_QRY:
					if(gDBInfo[dbcode].texttrg_recsize == 0) {
						DisposHandle((Handle)queryHdl);
						return(ErrorMsg(ERR_INCQUERY));
					}
					break;
				case AUTHOR_QRY:
					if(gDBInfo[dbcode].authortrg_recsize == 0) {
						DisposHandle((Handle)queryHdl);
						return(ErrorMsg(ERR_INCQUERY));
					}
					break;
				case TAXON_QRY:
					if(gDBInfo[dbcode].taxontrg_recsize == 0) {
						DisposHandle((Handle)queryHdl);
						return(ErrorMsg(ERR_INCQUERY));
					}
					break;
				default:
					DisposHandle((Handle)queryHdl);
					return(ErrorMsg(ERR_INCQUERY));
					break;					
			}
		}

		if( !NewQueryWindow(w,wdp,queryHdl,fName) )
			DisposeQuery(wdp);

		wdp->vRefNum = vRefNum;
		wdp->dirty = FALSE;
		wdp->inited = TRUE;
		return(TRUE);
	}
	else return(FALSE);
}

/**************************************
*	Read query file from disk
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

static Boolean ReadQuery(StringPtr fName, short vRefNum,QueryHdl *queryHdlP)
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
	
	/* Open resource file */
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
	
	/* Detach resources and close file */
	DetachResource((Handle)*queryHdlP);
	CloseResFile(refNum);
	ret=TRUE;

end:
	/* reset old values */
   UseResFile(curResFile);
	SetVol(NULL,oldVRefNum);
	return(ret);
}


/**************************************
*	Handle mouse clicks in query dialogs
*/

static void HandleQueryEvt(DialogPtr myDialog, short itemHit)
{
	WDPtr			wdp = (WDPtr)myDialog;
	QueryHdl		queryHdl;
	SignedByte	oldState;
	Str255		oldText,newText;
	short			which,i;
   short			itemType;
   Handle		item;
   Rect			box;
	GrafPtr		savePort;
	
	if(wdp == NULL || ((WindowPeek)myDialog)->windowKind != dialogKind)
		return;
	
	if((queryHdl = (QueryHdl)wdp->userHandle) == NULL)
		return;
		
	oldState = MyHLock((Handle)queryHdl);
		
	savePort = ChangePort(myDialog);
	
	switch(itemHit) {
		case QRY_EMBL_BTN:
		case QRY_SWISS_BTN:
			SelRadioButton(myDialog,QRY_EMBL_BTN,2,itemHit);
			(**queryHdl).dbcode = Button2DB(itemHit);
			CheckSqDBComp(myDialog, queryHdl);
			break;
		case QRY_AND_BTN:
		case QRY_OR_BTN:
			SelRadioButton(myDialog,QRY_AND_BTN,2,itemHit);
			(**queryHdl).gBooleanOp = Button2Bool(itemHit);
			SetBoolTxt(myDialog,QRY_BOOLSTR1+1,(**queryHdl).gBooleanOp);
			break;
		case QRY_POPUP1:
		case QRY_POPUP1 + 1:
		case QRY_POPUP1 + 2:
		case QRY_POPUP1 + 3:
		case QRY_POPUP1 + 4:
			HandlePopUpMenu(myDialog,itemHit, queryHdl);
			break;
		case QRY_BROWSEBTN:			
			which = ((DialogPeek)myDialog)->editField + 1 - QRY_SUBQRY1;
			if(BrowseIndex((**queryHdl).dbcode,(**queryHdl).subQuery[which].field, newText)) {
				DlgDelete(myDialog);		/* in case the user had selected some text before
													accessing the index */
				GetDlgText(myDialog,QRY_SUBQRY1 + which,oldText);
				if( (u_short)*oldText + (u_short)*newText <= 255) {
					pstrcat(oldText,newText);
					SetDlgText(myDialog,QRY_SUBQRY1 + which,oldText);
					SelIText(myDialog,QRY_SUBQRY1 + which,32767,32767);
				}
			}
			break;			
		case OK:
			ExecuteQuery(wdp);
			break;
		case QRY_CLEARBTN:
			for(i=0;i<MAXSUBQRY;++i) {
				*((**queryHdl).subQuery[i].text) = EOS;
				SetDlgText(myDialog,QRY_SUBQRY1 + i,(**queryHdl).subQuery[i].text);
				if((**queryHdl).subQuery[i].field != ENAME_QRY) {
					(**queryHdl).subQuery[i].field = ENAME_QRY;
					GetDItem(myDialog,QRY_POPUP1+i,&itemType,&item,&box);
					InvalRect(&box);
				}
			}

			wdp->dirty = TRUE;
			break;
	}

	SetPort(savePort);
	HSetState((Handle)queryHdl,oldState);
}

/**************************************
*	Set the text which shows current boolean operation between subqueries
*/

static void SetBoolTxt(DialogPtr myDialog, short firstItem, short gBooleanOp)
{
	register short i;
	StringPtr text;
	
	switch(gBooleanOp) {
		case BOOLEAN_AND: text = "\pAND";
			break;
		case BOOLEAN_OR: text = "\pOR";
			break;
	}
	
	for(i=0;i<MAXSUBQRY-1;++i)
		SetDlgText(myDialog,firstItem+i,text);
}


/**************************************
*	Find dialog button which corresponds to a given dbcode
*/

static short DB2Button(QueryHdl queryHdl)
{
	short button;
	
	switch((**queryHdl).dbcode) {
		case DB_EMBL:
			button = QRY_EMBL_BTN;
			break;
		case DB_SWISS:
			button = QRY_SWISS_BTN;
			break;
	}
	return(button);
}

/**************************************
*	Find dbcode which corresponds to a given dialog button
*/

static short Button2DB(short theItem)
{
	short dbcode;
	
	switch(theItem) {
		case QRY_EMBL_BTN:
			dbcode = DB_EMBL;
			break;
		case QRY_SWISS_BTN:
			dbcode = DB_SWISS;
			break;
	}
	return(dbcode);
}

/**************************************
*	Find dialog button which corresponds to a given boolean operation
*/

static short Bool2Button(QueryHdl queryHdl)
{
	short button;
	
	switch((**queryHdl).gBooleanOp) {
		case BOOLEAN_AND:
			button = QRY_AND_BTN;
			break;
		case BOOLEAN_OR:
			button = QRY_OR_BTN;
			break;
	}
	return(button);
}

/**************************************
*	Find boolean operation which corresponds to a given dialog button
*/

static short Button2Bool(short theItem)
{
	short boolOp;
	
	switch(theItem) {
		case QRY_AND_BTN:
			boolOp = BOOLEAN_AND;
			break;
		case QRY_OR_BTN:
			boolOp = BOOLEAN_OR;
			break;
	}
	return(boolOp);
}

/**************************************
*	Draws a pop-up menu box
*/

static pascal void DrawQueryUsrItems(WindowPtr wPtr, short itemNo)
{
   short			itemType;
   Handle		item;
   Rect			box;
   Str255 		text;
   QueryHdl		queryHdl;
   SignedByte	oldState;
   short			subquery;
   
	if((queryHdl = (QueryHdl)((WDPtr)wPtr)->userHandle) == NULL)
		return;
	
	oldState = MyHLock((Handle)queryHdl);
	
	subquery = itemNo - QRY_POPUP1;

	/* draw popup-box frame*/
	GetDItem(wPtr,itemNo,&itemType,&item,&box);
	EraseRect(&box);
	InsetRect(&box,-1,-1);
	FrameRect(&box);
	MoveTo(box.left+15,box.bottom-5);
	GetItem (gMenu[POPUP], (**queryHdl).subQuery[subquery].field,text);
   DrawString(text);
	
	/*draw drop shadow */
	MoveTo(box.right,box.top+2);
	LineTo(box.right,box.bottom);
	LineTo(box.left+2,box.bottom);
	
	DrawTriangle(&box);
	
	HSetState((Handle)queryHdl,oldState);
}

/**************************************
*	Draws downwards triangle in pull-down menu
*/

static void DrawTriangle(Rect *boxPtr)
{
	PolyHandle	downArrow;

	downArrow = OpenPoly();								/* build polygon for down-arrow */
		MoveTo(boxPtr->right-3,boxPtr->top+6);		/* location for 16-pixel-high box */
 		Line(-12,0);
		Line(6,6);
		Line(6,-6);
	ClosePoly();
	FillPoly(downArrow,black);							/* and draw arrow */
	KillPoly(downArrow);
}


/**************************************
*	Handle mouse clicks in pop-up menus
*/

static void HandlePopUpMenu(DialogPtr myDialog, short itemHit, QueryHdl queryHdl)
{
	short		subquery;
   short		itemType;
   Handle	item;
   Rect		box,popRect,titleRect;
   short		current;
   short		whichItem;
   short		i;
   short		dbcode;
   GrafPtr	savePort;
	
	subquery = itemHit - QRY_POPUP1;
	current = (**queryHdl).subQuery[subquery].field;
	dbcode = (**queryHdl).dbcode;
	
	savePort = ChangePort(myDialog);
   GetDItem(myDialog,itemHit,&itemType,&item,&box);
   popRect = box;
	LocalToGlobal(&TopLeft(popRect));
	LocalToGlobal(&BottomRight(popRect));
	
	GetDItem(myDialog,QRY_IN1 + subquery,&itemType,&item,&titleRect);
	DoInvertRect(&titleRect);
	for(i=1;i<=CountMItems(gMenu[POPUP]);++i) {
		EnableItem(gMenu[POPUP],i);
		SetItemMark(gMenu[POPUP],i,' ');
	}
	SetItemMark(gMenu[POPUP],current,checkMark);

	/* Disable query functions not available */
	if(gDBInfo[dbcode].actrg_recsize == 0)
		DisableItem(gMenu[POPUP],ACCNUM_QRY);
	if(gDBInfo[dbcode].kwtrg_recsize == 0)
		DisableItem(gMenu[POPUP],KEYWORD_QRY);
	if(gDBInfo[dbcode].texttrg_recsize == 0)
		DisableItem(gMenu[POPUP],FREETEXT_QRY);
	if(gDBInfo[dbcode].authortrg_recsize == 0)
		DisableItem(gMenu[POPUP],AUTHOR_QRY);
	if(gDBInfo[dbcode].taxontrg_recsize == 0)
		DisableItem(gMenu[POPUP],TAXON_QRY);
		
	whichItem=PopUpMenuSelect(gMenu[POPUP],popRect.top,popRect.left,current);
	if(whichItem=LoWord(whichItem)) {
		(**queryHdl).subQuery[subquery].field = whichItem;
	}
	InvalRect(&box);
	DoInvertRect(&titleRect);
	SetPort(savePort);
}


/**************************************
*	Execute a query
*/

static void ExecuteQuery(WDPtr wdp)
{
	register short i;
	SignedByte	oldState;
	Str255		text,title;
	char			temp[256];
	short			field;
	short			boolOp;
	short			dbcode;
	HitmapHdl	masterHmHdl, newHmHdl;
	QueryHdl		queryHdl, newQueryHdl;
	ResultHdl	resHdl;
	short			w;
	WDPtr			resWdp;
	DialogPtr	myDialog;
	
	if(wdp == NULL || ((WindowPeek)wdp)->windowKind != dialogKind)
		return;
	
	if((queryHdl = (QueryHdl)wdp->userHandle) == NULL)
		return;

	if( (w=GetFreeWindow()) == -1 ) {
		ErrorMsg(ERR_MAXWIN);
		return;
	}
	else
		resWdp = &gWindows[w];
		
	CenterDA('DLOG',SEARCHWAIT_DLG,50);
	myDialog = GetNewDialog(SEARCHWAIT_DLG,NULL,(WindowPtr) -1);
	if(myDialog) {
		ShowWindow(myDialog);
		DrawDialog(myDialog);
	}
	
	StartWaitCursor();
	
	oldState = MyHLock((Handle)queryHdl);
	
	/* update query record */
	UpdateQueryRec(wdp);

	boolOp = (**queryHdl).gBooleanOp;
	dbcode = (**queryHdl).dbcode;

	masterHmHdl = NULL;
	
	/* now we handle each subquery */
	for(i=0;i<MAXSUBQRY;++i) {
		pstrcpy(text,(**queryHdl).subQuery[i].text);
		field = (**queryHdl).subQuery[i].field;
		
		/* skip empty fields */
		if(*text == EOS)
			continue;
		
		/* do search */
		newHmHdl = DoSearch(field,text,dbcode);

		/* check for success */
		if(newHmHdl == NULL) {
			/* AND requires action */
			if(boolOp == BOOLEAN_AND) {
				/* if we had found something in a previous subquery, forget it */
				if(masterHmHdl != NULL) {
					DisposHandle((Handle)masterHmHdl);
					masterHmHdl = NULL;
				}
				break;	/* stop query */
			}
			else	/* this is an OR, so simply continue */
				continue;
		}
			
		if(masterHmHdl == NULL)	/* this is first subquery, so we update master handle */
			masterHmHdl = newHmHdl;
		else {	/* for all further subqueries we AND/OR new hitmap with master hitmap */
			switch(boolOp) {
				case BOOLEAN_AND:
					AndHitmaps(masterHmHdl,newHmHdl);
					break;
				case BOOLEAN_OR:
					OrHitmaps(masterHmHdl,newHmHdl);
					break;
			}
			DisposHandle((Handle)newHmHdl);
		}
	}
	
	if(myDialog) DisposDialog(myDialog);
	
	if(masterHmHdl == NULL) {
		ErrorMsg(ERR_NOTFOUND);
	}
	else {
		/* copy query record */
		newQueryHdl = queryHdl;
		if(HandToHand((Handle *)&newQueryHdl) != noErr) {
			ErrorMsg(ERR_MEMORY);
			DisposHandle((Handle)masterHmHdl);
		}
		/* get a new result record and initialise it */
		else {
			if( !InitResultRec(&resHdl,dbcode,masterHmHdl,newQueryHdl) ) {
				DisposHandle((Handle)masterHmHdl);
				DisposHandle((Handle)newQueryHdl);
				ErrorMsg(ERR_MEMORY);
			}
			else {
			 	if( (**resHdl).nhits > MAXHITS ) {
					DisposeResRec(resHdl);
					ErrorMsg(ERR_MAXHITSEXC);
				}
				else if( (**resHdl).nhits == 0 ) {
					DisposeResRec(resHdl);
					ErrorMsg(ERR_NOTFOUND);
				}
				else {
					/* Open window */
					GetIndString(title,OTHERS,RESTITLE);
					sprintf(temp,PtoCstr(title),++gResNo);
	
					if( !NewResultWindow(w,resWdp,resHdl, CtoPstr((char *)temp)) ) {
						DisposeResults(resWdp);
					} /* end if !NewResultsWindow() */
				}
			}	/* end else */
		}
	}

	InitCursor();
	HSetState((Handle)queryHdl,oldState);
}

static void CheckSqDBComp(DialogPtr myDialog, QueryHdl queryHdl)
{
	register short i;
	short			field;
	short			dbcode;
   short			itemType;
   Handle		item;
   Rect			box;
   Boolean		bChanged;
   GrafPtr		savePort;

	if(queryHdl == NULL) return;
	dbcode = (**queryHdl).dbcode;

	savePort = ChangePort(myDialog);
	/* check each db field for existence */
	for(i = 0; i<MAXSUBQRY; ++i) {
		field = (**queryHdl).subQuery[i].field;
		bChanged = FALSE;
		
		switch(field) {
			case ENAME_QRY:	/* must always exist */
				break;
			case ACCNUM_QRY:
				if(gDBInfo[dbcode].actrg_recsize == 0) {
					(**queryHdl).subQuery[i].field = ENAME_QRY;
					bChanged = TRUE;
				}
				break;
			case KEYWORD_QRY:
				break;
				if(gDBInfo[dbcode].kwtrg_recsize == 0) {
					(**queryHdl).subQuery[i].field = ENAME_QRY;
					bChanged = TRUE;
				}
				break;
			case FREETEXT_QRY:
				if(gDBInfo[dbcode].texttrg_recsize == 0) {
					(**queryHdl).subQuery[i].field = ENAME_QRY;
					bChanged = TRUE;
				}
				break;
			case AUTHOR_QRY:
				if(gDBInfo[dbcode].authortrg_recsize == 0) {
					(**queryHdl).subQuery[i].field = ENAME_QRY;
					bChanged = TRUE;
				}
				break;
			case TAXON_QRY:
				if(gDBInfo[dbcode].taxontrg_recsize == 0) {
					(**queryHdl).subQuery[i].field = ENAME_QRY;
					bChanged = TRUE;
				}
				break;
			default:
				(**queryHdl).subQuery[i].field = ENAME_QRY;
				bChanged = TRUE;
				break;					
		}
		if(bChanged) {
			GetDItem(myDialog,QRY_POPUP1+i,&itemType,&item,&box);
			InvalRect(&box);
		}
	}
	SetPort(savePort);
}
/*
*********************************************************************
*	
*	Events.c
*	Event handling
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
#include <AppleEvents.h>
#include <Balloons.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "events.h"
#include "menus.h"
#include "appleevents.h"
#include "window.h"
#include "sequence.h"
#include "xref.h"
#include "sequence.h"
#include "results.h"
#include "util.h"
#include "click.h"
#include "info.h"
#include "show_help.h"
#include "query.h"
#include "save.h"
#include "copy.h"
#include "export.h"
#include "print.h"
#include "locate.h"
#include "pref.h"
#include "pstr.h"

static void HandleClick(WDPtr wdp, EventRecord theEvent);
static void HandleMouseDown(EventRecord theEvent);
static void HandleMenu(long sel, short modifiers);
static void HandleKeyDown(EventRecord myEvent);
static void HandleUpdates(EventRecord myEvent);
static void SelectAll(WDPtr wdp, Boolean what);



/*
******************************** Global variables ******************
*/

extern Boolean 	gQuitApplication;
extern CursHandle	gCursor[8];						/* cursors used							*/
extern MenuHandle	gMenu[MENUNR];					/* menu handles							*/
extern Rect 		gDragRect;
extern FONTINFO	fMonaco;							/* font information						*/
extern Prefs		gPrefs;
extern WDRec		gWindows[MAXWIN];
extern OSType		gCreatorSig[];
extern Boolean		gInBackground;
extern char			gError[256];

short			gScrollAmt,gScrollCode;
Boolean		gScrollDir;
ClickInfo	gLastClick;

void DoEvent(EventRecord theEvent)
{
	Point where;
	
  	switch (theEvent.what) {
  		case nullEvent:
  		/* here we can perform any periodic actions */
  			break;
		case mouseDown:					/* mouse click 							*/
			HandleMouseDown(theEvent);
			break;
		case mouseUp:
			/* store time of mouseUp event (in order to detect double-clicks */
			gLastClick.when = theEvent.when;
			break;
		case keyDown: 						/* key click 								*/
		case autoKey:
			MaintainMenus();
			if (theEvent.modifiers & cmdKey) 
				HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)),
								theEvent.modifiers);
			else
				HandleKeyDown(theEvent); /* e.g. Help key */
			break;
		case updateEvt:
			HandleUpdates(theEvent);
			break;
		case activateEvt:
			HandleActivates((WindowPtr)theEvent.message,
								theEvent.modifiers & activeFlag);
			break;
		case diskEvt:
			if(HiWord(theEvent.message) != noErr) {
				where.h=((screenBits.bounds.right-screenBits.bounds.left)-304)/2;
				where.v=((screenBits.bounds.bottom-screenBits.bounds.top)-184)/2+20;
				DIBadMount(where,theEvent.message);
			}
			break;
		case osEvt:
			if( ((theEvent.message >>24) & 0xFF) == 1) {
				gInBackground = ((theEvent.message & 1) == 0);
							/* resume/suspend */
				HandleActivates(FrontWindow(),!gInBackground);
				LoadScrap();
			}
			/* we don't handle mouse moved events (already done by
				call to MaintainCursor() in MainLoop()
			*/ 
			break;
		case kHighLevelEvent:
			DoHighLevelEvent(theEvent);
			break;
	} /* end switch */
} /* end if */


/**************************************
*	Activate events
*	Return value:	none
*/

void HandleActivates(WindowPtr wPtr,Boolean activateFlag)
{
	WDPtr				wdp;
	Str255			str,title;
	ListHandle		theList;
	GrafPtr			oldPort;
	short				wKind;
	
	if (wPtr == NULL)
		return;
		
	wKind = ((WindowPeek)wPtr)->windowKind;
	if(wKind == queryW) return;	/* done by dialog mgr */
	
	if(activateFlag) { 	/* Activate event */
		if(wdp=FindMyWindow(wPtr)) {
			SetPort(wPtr);
			switch(wKind) {
				case queryW:
					break;
				case seqW:
					HiliteControl(wdp->vScroll,ACTIVE);
					HiliteControl(wdp->hScroll,ACTIVE);
					DrawGrowIcon(wPtr);
					HideShowSeqSelections(wdp, TRUE);
					AddXRefsToMenu(wdp);
					break;
				case resW:
					HiliteControl(wdp->vScroll,ACTIVE);
					HiliteControl(wdp->hScroll,ACTIVE);
					DrawGrowIcon(wPtr);
					HideShowResSelections(wdp, TRUE);
					break;
				default:SysBeep(10);
			}
		}
	}
	else {															/* Deactivate event */
		if(wdp=FindMyWindow(wPtr)) {
			oldPort = ChangePort(wPtr);
			switch(((WindowPeek)wdp)->windowKind) {
				case queryW:	/* done by dialog mgr */
					break;
				case seqW:
					HiliteControl(wdp->vScroll,INACTIVE);
					HiliteControl(wdp->hScroll,INACTIVE);
					DrawGrowIcon(wPtr);
					HideShowSeqSelections(wdp,FALSE);
					break;
				case resW:
					HiliteControl(wdp->vScroll,INACTIVE);
					HiliteControl(wdp->hScroll,INACTIVE);
					DrawGrowIcon(wPtr);
					HideShowResSelections(wdp,FALSE);
					break;
				default:SysBeep(10);
			}
			SetPort(oldPort);
		}
	}
}


/**************************************
*	Handle mouse click events in window
*	Return value:	none
*/

static void HandleClick(WDPtr wdp, EventRecord theEvent)
{
	GrafPtr	savePort;
	
	if(wdp == NULL)
		return;
		
	savePort = ChangePort((GrafPtr)wdp);
	switch(((WindowPeek)wdp)->windowKind) {
		case queryW:	/* done by dialog mgr */
			break;
		case seqW:
			DoSeqClicks(wdp,&theEvent);
			break;	
		case resW:
			DoResClicks(wdp,&theEvent);
			break;
		default:SysBeep(10);
	}
	SetPort(savePort);
}

/**************************************
*	Mouse down events
*	Return value:	none
*/

static void HandleMouseDown(EventRecord theEvent)
{
	WindowPtr	wPtr;
	short			partCode;
	WDPtr			wdp;

	partCode=FindWindow( theEvent.where, &wPtr ); 
	switch (partCode) {
		case inSysWindow:
			SystemClick( &theEvent, wPtr );
			break;
		case inMenuBar:
			MaintainMenus();
			HandleMenu( MenuSelect(theEvent.where), theEvent.modifiers) ;
			break;
		case inDrag:
			if(FindMyWindow(wPtr))
				DragWindow(wPtr,theEvent.where,&gDragRect);
			break;
		case inGoAway:
			if(wdp=FindMyWindow(wPtr)) {
				if(TrackGoAway(wPtr,theEvent.where)) {
					if( (theEvent.modifiers & optionKey) != 0)
						CloseAllWindows((theEvent.modifiers & shiftKey) != 0);
					else
						CloseMyWindow(wdp, (theEvent.modifiers & shiftKey) != 0);
				}
			}
			break;
		case inGrow:
			if(FindMyWindow(wPtr))
				DoGrow(wPtr,theEvent.where);
			break;
		case inZoomIn:
		case inZoomOut:
			if(FindMyWindow(wPtr))
				if(TrackBox(wPtr,theEvent.where,partCode))
					DoZoom(wPtr, partCode);
			break;
		case inContent:
			if(wPtr != FrontWindow()) {
				SelectWindow(wPtr);
				SetPort(wPtr);
			}
			else
				if(wdp=FindMyWindow(wPtr))
					HandleClick(wdp,theEvent);
			break;
		case inDesk:
			break;
		default:SysBeep(10);
	} /* end switch */
	
	/* save some information about this click for later in order to identify
		double-clicks */
		
	gLastClick.where = theEvent.where;
	gLastClick.wPtr = wPtr;
}

/**************************************
*	Menu handling
*	Return value:	none
*/

static void HandleMenu(long sel, short modifiers)
{
	register short	theItem=LoWord(sel);
	GrafPtr			savePort;
	WindowPtr		topWindow=FrontWindow();
	Str255			name,title;
	WDPtr				wdp;
	short				i,k;
	Boolean			shift;
	short				dbcode;
	short 			from,to;
	MenuHandle		mh;
	
	shift = ((modifiers & shiftKey) != 0);
	
	switch (HiWord(sel)) {
		case APPLE_M:
			switch (theItem) {
				case ABOUT_I:						/* About� 									*/
					ShowAbout();
					break;
				case HELP_I:
					GetIndString(name,OTHERS,HELPSTR);
					Show_help( HELP_DLG, kHelpTextRsrc, kHelpPictRsrc, name,"\p");
					break;
				default:
					GetItem(gMenu[APPLE], theItem, name);
					GetPort(&savePort);				/* save old grafPort (just in case�)*/
					OpenDeskAcc(name);				/* open DA 									*/
					InitCursor();						/* DA may have changed cursor 		*/
					SetPort(savePort);				/*	DA may have changed grafPort 		*/
					break;
			}
			break;
			
		case FILE_M:
			switch(theItem) {
				case NEW_I:
					NewQuery();
					break;
				case OPEN_I:
					OpenQuery();
					break;
				case LOAD_I:
					LoadResults();
					break;
				case CLOSE_I:
					if(topWindow) {
						if ( (k=((WindowPeek)topWindow)->windowKind) < 0 )
							CloseDeskAcc(k);
						else {
							if(wdp=FindMyWindow(topWindow)) {
								if( (modifiers & optionKey) != 0)
									CloseAllWindows(shift);
								else
									CloseMyWindow(wdp,shift);
							}
						}
					}
					break;
				case SAVE_I:
					if(wdp = FindMyWindow(topWindow))
						DoSave(wdp,TRUE);
					break;
				case SAVEAS_I:
					if(wdp = FindMyWindow(topWindow))
						DoSave(wdp, FALSE);
					break;
				case EXPORTSEL_I:
					if(wdp=FindMyWindow(topWindow))
						ExportRes(wdp);
					break;
				case PAGESETUP_I:
					PrintDialog();
					break;
				case PRINT_I:
					if(wdp=FindMyWindow(topWindow))
						PrintIt(wdp);
					break;
				case QUIT_I:
				   if(CloseAllWindows(shift)) {
						gQuitApplication=TRUE;
						LoadScrap();
					}
					break;
				default:SysBeep(10);
			} /* end switch */
			break;

		case EDIT_M: 
			if(!SystemEdit(theItem-1))
				switch(theItem) {
					case UNDO_I:
						SysBeep(10);
						break;
					case CUT_I:
						if(wdp=FindMyWindow(topWindow))
							CutSelection(wdp);						
						break;
					case COPY_I:
						if(wdp=FindMyWindow(topWindow))
							CopySelection(wdp);
						break;
					case PASTE_I:
						if(wdp=FindMyWindow(topWindow))
							PasteSelection(wdp);
						break;
					case CLEAR_I:
						if(wdp=FindMyWindow(topWindow))
							ClearSelection(wdp);
						break;
					case SELALL_I:
						if(wdp=FindMyWindow(topWindow))
							SelectAll(wdp, TRUE);
						break;
					case SELNONE_I:
						if(wdp=FindMyWindow(topWindow))
							SelectAll(wdp, FALSE);
						break;
					case DUPLICATE_I:
						if(wdp=FindMyWindow(topWindow))
							DuplicateQuery(wdp);
						break;
					default:SysBeep(10);
				}
			break;
						
		case OSTUFF_M:
			switch(theItem) {
				case INFO_I:
					ShowDBInfo();
					break;
				default:SysBeep(10);
			}
			break;
			
		case PREFS_M:
			switch(theItem) {
				case LOCATE_I:
					Locate();
					break;
				case SAVEPREFS_I:
					WritePrefs();
					break;
				case GENERAL_I:
					GeneralOptions();
					break;
				default:
					SysBeep(10);
					break;
			}
			break;
	
		case FORMAT_M:
			CheckItem(gMenu[FORMAT],gPrefs.format,FALSE);
			CheckItem(gMenu[FORMAT],gPrefs.format=theItem,TRUE);
			break;
							
		case CREATOR_M:
			switch(theItem) {
				case WORD_I:
				case TTEXT_I:
				case WRITE_I:
					gPrefs.creatorSig = gCreatorSig[theItem];
					CheckItem(gMenu[CREATOR],gPrefs.creator,FALSE);
					CheckItem(gMenu[CREATOR],gPrefs.creator=theItem,TRUE);
					break;
				case OTHERS_I:
					if(PickNewCreator()) {
						UpdateOtherCreator();
						CheckItem(gMenu[CREATOR],gPrefs.creator,FALSE);
						CheckItem(gMenu[CREATOR],gPrefs.creator=theItem,TRUE);
					}
					break;
			}
			
			break;
			
		case WINDOWS_M:
			if(topWindow) {
				if(theItem == ROTATE_I) {
					SendBehind(topWindow,NULL);
				}
				else {
					GetItem(gMenu[WINDOWS],theItem,name);
					for(i=0;i<MAXWIN;++i) {
						if(gWindows[i].inUse == TRUE) {
							GetWTitle((WindowPtr)&gWindows[i],title);
							if(!pstrcmp(name,title)) {
								SelectWindow((WindowPtr)&gWindows[i]);
								SetPort((WindowPtr)&gWindows[i]);
								break;
							}
						}
					}
				}
			}
			break;
			
		case XREF_M:
			if(wdp=FindMyWindow(topWindow)) {
				dbcode = (**(SeqRecHdl)(wdp->userHandle)).dbcode;

				if(theItem == ALLXREFS_I) {
					from = 3;
					to = CountMItems(gMenu[XREF]);
				}
				else
					from = to = theItem;
					
				for( theItem=from; theItem <= to;++theItem ) {
					GetItem(gMenu[XREF],theItem,name);
					PtoCstr(name);
					if( !OpenXRefs((char *)name,(dbcode == DB_EMBL) ? DB_SWISS : DB_EMBL) )
						break;
				}
			}
			break;			
			
		case kHMHelpMenuID:
			HMGetHelpMenuHandle(&mh);
			if( theItem == CountMItems(mh) ) {
				GetIndString(name,OTHERS,HELPSTR);
				Show_help( HELP_DLG, kHelpTextRsrc, kHelpPictRsrc, name,"\p");
			}
			break;
			
		} /* end switch */
	HiliteMenu(0);
}

/**************************************
*	Key down events
*	Return value:	none
*/

static void HandleKeyDown(EventRecord myEvent)
{
#define HELPKEY 	0x72
#define HOME		0x73
#define END			0x77
#define PG_UP		0x74
#define PG_DN		0x79

	char			keycode=(myEvent.message & keyCodeMask) >> 8;
	WindowPtr	topWindow=FrontWindow();
	WDPtr			wdp;
	short			wKind;
	short			pagesize,oldValue,newValue,max;
	Rect			viewRect;
	Str255		saveStr;
		
	if(keycode == HELPKEY) {
		GetIndString(saveStr, OTHERS, HELPSTR);
		Show_help( HELP_DLG, kHelpTextRsrc, kHelpPictRsrc, saveStr, "\p");
	}
	
	if ( (wdp=FindMyWindow(topWindow)) == NULL) {
		SysBeep(10);
		return;
	}
	
	wKind=((WindowPeek)wdp)->windowKind;
	if(wKind == queryW)
		return;
		
	switch (keycode) {
		case HOME:
		case END:
			switch(wKind) {
				case queryW:
					break;
				case seqW:
					oldValue = GetCtlValue(wdp->vScroll);
					newValue = (keycode == HOME) ? 	GetCtlMin(wdp->vScroll) :
																GetCtlMax(wdp->vScroll);
					SetCtlValue(wdp->vScroll,newValue);
					AdjustSeqText((WindowPtr)wdp,oldValue,newValue,vertBar);
					break;
				case resW:
					oldValue = GetCtlValue(wdp->vScroll);
					newValue = (keycode == HOME) ? 	GetCtlMin(wdp->vScroll) :
																GetCtlMax(wdp->vScroll);
					SetCtlValue(wdp->vScroll,newValue);
					AdjustResText((WindowPtr)wdp,oldValue,newValue,vertBar);
					break;
				default: SysBeep(10);
			}
			break;
		case PG_DN:
		case PG_UP:
			switch(wKind) {
				case queryW:
					break;
				case seqW:
					GetViewRect((WindowPtr)wdp,&viewRect);	
					pagesize =
						(viewRect.bottom - viewRect.top) / fMonaco.height - 1;

					oldValue=GetCtlValue(wdp->vScroll);
					if(keycode == PG_DN)
						newValue = oldValue + pagesize;
					else newValue = oldValue - pagesize;
					if(newValue < 0) newValue=0;
					max = GetCtlMax(wdp->vScroll);
					if(newValue > max) newValue=max;
		
					SetCtlValue(wdp->vScroll, newValue );
					AdjustSeqText((WindowPtr)wdp,oldValue,newValue,vertBar);
					break;
				case resW:
					GetViewRect((WindowPtr)wdp,&viewRect);	
					pagesize =
						(viewRect.bottom - viewRect.top) / fMonaco.height - 1;

					oldValue=GetCtlValue(wdp->vScroll);
					if(keycode == PG_DN)
						newValue = oldValue + pagesize;
					else newValue = oldValue - pagesize;
					if(newValue < 0) newValue=0;
					max = GetCtlMax(wdp->vScroll);
					if(newValue > max) newValue=max;
		
					SetCtlValue(wdp->vScroll, newValue );
					AdjustResText((WindowPtr)wdp,oldValue,newValue,vertBar);
					break;
				default: SysBeep(10);
			}
			break;
		default: SysBeep(10);
	}
}


/**************************************
*	Update events
*	Return value:	none
*/

static void HandleUpdates(EventRecord myEvent)
{
	GrafPtr		savePort;
	WindowPtr	wPtr=(WindowPtr)myEvent.message;
	WDPtr			wdp;
	short			wKind;
	
	if(wPtr == NULL)
		return;
		
	wKind = ((WindowPeek)wPtr)->windowKind;
	if(wKind == queryW) return;	/* done by dialog mgr */
	
	BeginUpdate(wPtr);
	if( wdp=FindMyWindow(wPtr)) {
		savePort = ChangePort(wPtr);
		switch(wKind) {
			case queryW:
				break;
			case seqW:
				DrawSeqWinAll(wdp,0);
				DrawControls(wPtr);
				DoDrawGrowIcon(wPtr);
				break;
			case resW:
				DrawResWinAll(wdp,0);
				DrawControls(wPtr);
				DoDrawGrowIcon(wPtr);
				break;
			default:SysBeep(10);
		}
		SetPort(savePort);
	}
	EndUpdate(wPtr);
}


/**************************************
* (De)Selects complete content of a window
*	Return value:	none
*/

static void SelectAll(WDPtr wdp, Boolean what)
{
	if( wdp == NULL )
		return;
		
	switch( ((WindowPeek)wdp)->windowKind) {
		case queryW:
			break;
		case seqW:
			SelectAllSeq(wdp,what);
			break;
		case resW:
			SelectAllResults(wdp,what);
			break;
		default:SysBeep(10);
	}
}
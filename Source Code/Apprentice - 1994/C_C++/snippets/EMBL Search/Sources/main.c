/*
*********************************************************************
*	
*	main.c
*	Initialization, cleaning up, main loop and some general routines
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

#include <Balloons.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "main.h"
#include "util.h"
#include "window.h"
#include "menus.h"
#include "print.h"
#include "query.h"
#include "checkapp.h"
#include "events.h"
#include "environment.h"
#include "pref.h"
#include "cd.h"
#include "appleevents.h"

static void GetGlobalMouse(Point *mouse);
static void MaintainCursor(Point mouse, RgnHandle cursRgn);
static long GetSleep(void);
static void MainLoop(void);
static void InitDefaults(void);
static void PrepMenus(void);
static void PrepFonts(void);
static void PrepCursors(void);
static void InitThings(void);
static void CleaningWoman(void);


/*
********************************* Global variables *******************
*/

extern short		gResNo, gQueryNo;
extern Prefs		gPrefs;
extern ClickInfo	gLastClick;
extern Boolean		gHasAppleEvents;
extern Boolean    bErrorOccurred;
extern Boolean		gHasBalloonHelp;

CursHandle	gCursor[8];							/* cursors used							*/
MenuHandle	gMenu[MENUNR];						/* menu handles							*/
Boolean		gQuitApplication = FALSE;		/* flag for quitting the program		*/
VolInfo		gCurrentCD;							/* information about current CD		*/
DBInfo		gDBInfo[DB_NUM];					/* holds all information about databases */
IndexFiles	gFileList;							/* array of index file names */
Str255		gVolLabel;							/* default volume name */
FONTINFO		fMonaco,fSystem;					/* font information						*/
Boolean		gInBackground;						/* is set if we«re currently in
															background */

static CursHandle	editCursor;					/* actually not used */
															

/**************************************
*	Find current mouse position in global coordinates
*	Return value:	none
*	Side-effect:	mouse position in *mouse
*/

static void GetGlobalMouse(Point *mouse)
{
	EventRecord theEvent;
	
	OSEventAvail(0, &theEvent);		/* no need to actually *get* events */
	*mouse=theEvent.where;
	
	/* we could also call GetMouse plus LocalToGlobal, but that requires
		that a valid port is set */
}


/**************************************
*	Adjust cursor and recalculate cursor region for WNE.
*	(Not really needed, we could easily set cursor region to NULL, as long as
*  we don't have edit windows. But we include it to keep the structure of
*  this sample code more general to facilitate further extension.)
*	Return value:	none
*	Side-effect:	new region in cursRgn
*/

static void MaintainCursor(Point mouse, RgnHandle cursRgn)
{
	RgnHandle	arrowRgn;
	RgnHandle	editRgn;

	if (!gInBackground && !IsDAWindow(FrontWindow())) {
		editRgn=NewRgn();
		arrowRgn=NewRgn();	/* Now make it BIG */
		SetRectRgn(arrowRgn,-32768,-32768,32766,32766); /* see TN.SC002 */
		
		/* calculate editRgn */
		
		/* in this example we don«t have any */
				
		/* now subtract editRgn from arrowRgn */
		
		DiffRgn(arrowRgn,editRgn,arrowRgn);
		
		/* set cursor accordingly and set cursor region for next WNE call */

		if(PtInRgn(mouse,editRgn)) {
			SetCursor(*editCursor);
			CopyRgn(editRgn,cursRgn);
		}
		else {
			InitCursor();
			CopyRgn(arrowRgn,cursRgn);
		}
		
		DisposeRgn(arrowRgn);
		DisposeRgn(editRgn);
	}
}


/**************************************
*	Calculate value of 'sleep' for WaitNextEvent
*	Return value:	sleep value
*/

static long GetSleep()
{
	return( (gInBackground) ? 0x7FFFFFFF : GetCaretTime() );
}


/**************************************
*	This is the main loop, calling WaitNextEvent 
*/

static void MainLoop()
{
	EventRecord	theEvent;
	RgnHandle	cursRgn=NewRgn();
	Point			mouse;
	Boolean		isEvent,isHandled;
	
	gInBackground=FALSE;
	gLastClick.when=0L;					/* initialize gLastClick */

	InitCursor();
	GetGlobalMouse(&mouse);
	MaintainCursor(mouse,cursRgn);
	MaintainMenus();
	
	if(gPrefs.startQOpen && !IsAppWindow(FrontWindow()))
		NewQuery();

	while(!gQuitApplication){	
		/* if an error occurred while we were in the background show it now */
		if( bErrorOccurred )
			ErrorMsg(0);
		
		isEvent = WaitNextEvent(everyEvent,&theEvent,GetSleep(),cursRgn);
		MaintainCursor(theEvent.where,cursRgn);
		
		if(theEvent.what == osEvt) /* work around bug in DialogSelect */
			DoEvent(theEvent);
		else {
			/* special action needed if event is for a non-modal dialog window */
			isHandled = FALSE;
			if(IsDialogEvent(&theEvent))
				isHandled = DoQueryEvt(theEvent);
				
			if(isEvent && !isHandled)
				DoEvent(theEvent);
		}
	}	/* end while */
	DisposeRgn(cursRgn);
}


/**************************************
*	Sets global variables to default values
*/

static void InitDefaults()
{
	*(gCurrentCD.volName) = EOS;
	
	GetIndString(gDBInfo[DB_EMBL].SeqDName,EMBLDATA,DATA_DIR);
	GetIndString(gDBInfo[DB_EMBL].InxDName,EMBLDATA,INDEX_DIR);
	GetIndString(gDBInfo[DB_SWISS].SeqDName,SWISSDATA,DATA_DIR);
	GetIndString(gDBInfo[DB_SWISS].InxDName,SWISSDATA,INDEX_DIR);	
	
	GetIndString(gFileList.divFName,INDEX_NAMES,DIV_TABLE);
	GetIndString(gFileList.enameIdxFName,INDEX_NAMES,ENAME_IDX);
	GetIndString(gFileList.briefIdxFName,INDEX_NAMES,BRIEF_IDX);
	GetIndString(gFileList.acnumTrgFName,INDEX_NAMES,ACNUM_TRG);
	GetIndString(gFileList.acnumHitFName,INDEX_NAMES,ACNUM_HIT);
	GetIndString(gFileList.keywTrgFName,INDEX_NAMES,KEYWORD_TRG);
	GetIndString(gFileList.keywHitFName,INDEX_NAMES,KEYWORD_HIT);
	GetIndString(gFileList.textTrgFName,INDEX_NAMES,FREETEXT_TRG);
	GetIndString(gFileList.textHitFName,INDEX_NAMES,FREETEXT_HIT);
	GetIndString(gFileList.authorTrgFName,INDEX_NAMES,AUTHOR_TRG);
	GetIndString(gFileList.authorHitFName,INDEX_NAMES,AUTHOR_HIT);
	GetIndString(gFileList.taxonTrgFName,INDEX_NAMES,TAXON_TRG);
	GetIndString(gFileList.taxonHitFName,INDEX_NAMES,TAXON_HIT);
	
	GetIndString(gVolLabel,OTHERS,VOL_LABEL);
	
	gDBInfo[DB_EMBL].gDivNames = gDBInfo[DB_SWISS].gDivNames = NULL;
	gResNo = gQueryNo = 0;
}


/**************************************
*	Prepare our menus
*/

static void PrepMenus()
{
	register short i;
	Str255 helpMenu;
	MenuHandle mh;
	
	gMenu[APPLE]=GetMenu(APPLE_M);		
	AddResMenu(gMenu[APPLE],'DRVR');		/* insert DAs in Apple-menu 			*/
	
	gMenu[FILE]=GetMenu(FILE_M);
	gMenu[EDIT]=GetMenu(EDIT_M);
	gMenu[OSTUFF]=GetMenu(OSTUFF_M);
	gMenu[PREFS]=GetMenu(PREFS_M);
	gMenu[WINDOWS]=GetMenu(WINDOWS_M);
	gMenu[CREATOR]= GetMenu(CREATOR_M);
	gMenu[FORMAT]=GetMenu(FORMAT_M);
	gMenu[XREF] = GetMenu(XREF_M);
	gMenu[POPUP] = GetMenu(POPUP_M);
	
	for(i=0;i <= WINDOWS;InsertMenu(gMenu[i++],0))
		;
	InsertMenu(gMenu[CREATOR],-1);
	InsertMenu(gMenu[FORMAT],-1);
	InsertMenu(gMenu[XREF],-1);
	InsertMenu(gMenu[POPUP],-1);

	if(gPrefs.creator == OTHERS_I)
		UpdateOtherCreator();
	CheckItem(gMenu[CREATOR],gPrefs.creator,TRUE);
	
	CheckItem(gMenu[FORMAT],gPrefs.format,TRUE);

	/* if we run under System 7 we also insert help into Help menu */
	if(gHasBalloonHelp) {
		if(HMGetHelpMenuHandle(&mh) == noErr) {
			GetItem(gMenu[APPLE],HELP_I,helpMenu);
			AppendMenu(mh,helpMenu);
		}
	}

	DrawMenuBar();
}

/**************************************
*	Get font information
*/

static void PrepFonts()
{
	TextSize(9);
	GetFNum("\pMonaco",&fMonaco.num);
	TextFont(fMonaco.num);
	GetFontInfo(&fMonaco.finfo);
	fMonaco.height=fMonaco.finfo.ascent+
						fMonaco.finfo.descent+
	               fMonaco.finfo.leading;
	TextSize(12);
	TextFont(fSystem.num=0);
	GetFontInfo(&fSystem.finfo);
	fSystem.height=fSystem.finfo.ascent+
						fSystem.finfo.descent+
	               fSystem.finfo.leading;
}


/**************************************
*	Read in rotating cursor resources
*/

static void PrepCursors()
{
	register short i;
	
	editCursor = GetCursor(iBeamCursor);
   gCursor[0] = GetCursor(watchCursor);
   for(i=1;i<8;++i)
   	gCursor[i] = GetCursor(256+i);

   for(i=0;i<8;++i)
   	LockHandleHigh((Handle)gCursor[i]);
}


/**************************************
*	Initialisation
*/

static void InitThings()
{
	register short	i;
	EventRecord		event;
	
	MaxApplZone();
	InitGraf(&thePort);
	
	for(i=0;i<10;++i)
		MoreMasters();						/* extra pointer blocks at  bottom of  heap */
	
	InitFonts();							/* startup the font manager 		*/
   InitWindows();                   /* startup the window manager 	*/
   InitMenus();                     /* startup the menu manager 		*/
   TEInit();                        /* startup the text edit manager	*/
   InitDialogs(NULL);               /* startup the dialog manager 	*/

	/*
		This next bit of code is necessary to allow the default button of our
		alert be outlined.
	*/
	 
	for (i = 0; i < 3; ++i)
		EventAvail(everyEvent, &event);

   CheckEnvironment();					/* look for system version 6 	*/
	PrepCursors();							/* prepare cursors 				*/
	GetPrefs();
	InitDefaults();						/* set default values			*/
	PrepFonts();
	PrepWindows();
 	PrepMenus();							/* prepare menus					*/
 	PrepPrint();							/* prepare printing */
	InitCD();								/* look for CD-ROM drive		*/
	if(gHasAppleEvents)
 		InitAppleEvents();

}


/**************************************
*	Tidy up
*/

static void CleaningWoman()
{
	register short i;
	
	/* release any previously occupied memory */
	if(gDBInfo[DB_EMBL].gDivNames != NULL)
		DisposHandle(gDBInfo[DB_EMBL].gDivNames);
	if(gDBInfo[DB_SWISS].gDivNames != NULL)
		DisposHandle(gDBInfo[DB_SWISS].gDivNames);
	gDBInfo[DB_EMBL].gDivNames = gDBInfo[DB_SWISS].gDivNames = NULL;
	
   for(i=0;i < 8;++i)
   	HUnlock((Handle)gCursor[i]);
}


/**************************************
*	Main
*/

void main()
{
	InitThings();								/* prepare everything necessary			*/
	CheckAppFiles();
	MainLoop();									/* here we do the work						*/
	CleaningWoman();							/* tidy up										*/
}
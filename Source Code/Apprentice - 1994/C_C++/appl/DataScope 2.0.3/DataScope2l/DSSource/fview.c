# include <types.h> 				/* Nearly always required */
# include <quickdraw.h> 			/* To access the qd globals */
# include <toolutils.h> 			/* CursHandle and iBeamCursor */
# include <fonts.h> 				/* Only for InitFonts() trap */
# include <events.h>				/* GetNextEvent(), ... */
# include <windows.h>				/* GetNewWindow(), ... */
# include <controls.h>
# include <files.h>
# include <fcntl.h>
# include <packages.h>
# include <dialogs.h>				/* InitDialogs() and GetNewDialog() */
# include <menus.h> 				/* EnableItem(), DisableItem() */
# include <desk.h>					/* SystemTask(), SystemClick() */
# include <textedit.h>				/* TENew() */
# include <scrap.h> 				/* ZeroScrap() */
# include <StdIO.h>
# include <Math.h>
# include <OSEvents.h>
# include <Palette.h>
#include	<Resources.h>
#define switchEvt	 1		/* Switching event (suspend/resume )  for app4evt */
/*
 * Resource ID constants.
 */
# define appleID		128 			/* This is a resource ID */
# define fileID 		129 			/* ditto */
# define editID 		130 			/* ditto */
# define imageID        131
# define numID			132

# define appleMenu		0				/* MyMenus[] array indexes */
# define	MAbout_DataScope	1

# define fileMenu		1
# define    MOpen 		101
# define    MClose		102
# define    MSave 		103
# define    MSave_As 	104
# define 	MLoad_Palette    106
# define    MLoad_Text 	107
# define	MQuit 		109

# define editMenu		2
# define	MUndo 		201
# define	MCut		203
# define	MCopy	 	204
# define	MPaste		205
# define	MClear		206

# define imageMenu 				3
# define MGenerate_Image    	301
# define MImage_Size			302
# define MInterpolated_Image    304
# define MInterpolate_Size		305
# define MPolar_Image    		307
# define MPolar_Size			308

# define numMenu 				4
# define MAttributes			401
# define MSynchronize			402
# define MExtract_Selection     403
# define MSee_Notebook    		405
# define MCalculate_From_Notes  406

# define menuCount	 5
# define windowID			128
# define imgwinID	  		129
/*
 * For the About ... DLOG
 */
# define aboutMeDLOG		128
# define openDLOG  			131
# define attrDLOG  			129
# define nomemDLOG  		132

/*
*  For the controls
*/
#define Cnewuser 			129
#define Cdelete 			130
#define Cpasswd 			131
#define NCS 3

/*
 * HIWORD and LOWORD macros, for readability.
 */
# define HIWORD(aLong)		(((aLong) >> 16) & 0xFFFF)
# define LOWORD(aLong)		((aLong) & 0xFFFF)

/*
 * Global Data objects, used by routines external to main().
 */
MenuHandle		MyMenus[menuCount]; 	/* The menu handles */
Boolean 		DoneFlag;				/* Becomes TRUE when File/Quit chosen */

int openfloat(),loseit(),saveit(), loadtext(), 
	dopaste(), makeimage(), interpit(), polarit(),
	loadpal(), doatt(), dopatt(), extit(), setsynch(), dosize(),
	newnotes(), calcnotes();
		 	
#include "fview.h"

struct Mwin 
		*mw,				/* temporary list pointer */
		*fmw,				/* Data pointer which goes along with the front window */
		*Mlist;				/* Mwin list - master list of windows */
		
	CursHandle
		spinwatch[10],		/* for time-waits */
		ibeamHdl;
	Rect 
		tr,
		screenRect;
	RgnHandle 
		Urgn;				/* storage for update region */
	int 
		MFhere=0,			/* do we have multifinder? */
		synchro=0;			/* synchronization flag */
	char 
		*rgbsave;			/* space for default palette */
		
	Point 
		startwin = {50,20};
	
int (*callme)();			/* for menu re-direction */
struct Mwin *callmw;

int
main()
{
	extern void 	setupMenus();
	extern void 	doCommand();
	Rect		 	dragRect;
	Point			mousePt;
	EventRecord 	myEvent;
	WindowPtr		theActiveWindow, whichWindow;
	int				i,timer;
	short int		shorti,shortj;
	AppFile			theFile;
	/*
	 * Initialization
	 */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	
/*
*  check for multifinder presence.
*/
	if (GetTrapAddress( 0x60) != GetTrapAddress( 0x9f) ) {
		MFhere = 1;
		WaitNextEvent(everyEvent, &myEvent, 3, 0L);
		WaitNextEvent(everyEvent, &myEvent, 3, 0L);
	}

/*
*  Opening dialog display
*/
	timer = TickCount() + 2000;
	openingdialog(1);		/* show opening message */
	setupMenus();			/* Local procedure, below */
	Urgn = NewRgn();		/* for temporary update region */
	

	FlushEvents(everyEvent - diskEvt, 0);
/*
*  get a copy of the current color set
*/
	colorsave();
	setgreys();				/* initial menu setup */
	
	/*
	 * Calculate the drag rectangle in advance.
	 * This will be used when dragging a window frame around.
	 * It constrains the area to within 4 pixels from the screen edge
	 * and below the menu bar, which is 20 pixels high.
	 */
	screenRect = qd.screenBits.bounds;
	SetRect(&dragRect,  4, 20 + 4, screenRect.right-4, screenRect.bottom-4);
	ibeamHdl = GetCursor(iBeamCursor);		/* Grab this for use later */
	for (i=0; i<7; i++)
		spinwatch[i] = GetCursor(257+i);	/* grab these for later, too */
	
/*
*  allocate and setup rgbsave
*/
	rgbsave = (char *) NewPtr(800);
	
	for (i=0; i<240; i++) {
		rgbsave[i*3] = i;					/* set up grey starting values */
		rgbsave[i*3+1] = i;
		rgbsave[i*3+2] = i;
	}
/*
*  for the selected files, try to load them into windows.
*/
	Mlist = NULL;
	CountAppFiles( &shorti, &shortj);
	while (!shorti && shortj > 0) {
		GetAppFiles( shortj, &theFile);			/* get file name */
		p2cstr(theFile.fName);
		setvol(NULL, theFile.vRefNum);			/* set to this volume (dir) */
		getdf(theFile.fName);					/* inserts window(s) into Mlist */
		ClrAppFiles(shortj--);
		timer = TickCount() + 10;				/* shorten the time to wait */
	}
	setgreys();									/* grey or un-grey the menus */

/*
*  reset callme pointer
*/
	callme = NULL;
/*
*  start the network 
*/
	startnet();
	
	/*
	 * Ready to go.
	 * Start with a clean event slate, and cycle the main event loop
	 * until the File/Quit menu item sets DoneFlag.
	 *
	 */
/*
*  With the Opening dialog up,
*  Wait for many seconds or until user presses a key or the mouse.
*/	
	while (timer > TickCount()) {
		if (EventAvail(keyDownMask + mDownMask, &myEvent)) {
			if (myEvent.what == mouseDown 
				&& IsDialogEvent(&myEvent))		/* eat clicks in the dialog */
				GetNextEvent(mDownMask, &myEvent);
			break;								/* go directly into program */
		}
	}

	openingdialog(0);							/* take away the opening dialog */
	DoneFlag = false;
	for ( ;; ) {
		if (DoneFlag) {
			break;								/* from main event loop */
		}
		/*
		 * Main Event tasks:
		 */
		SystemTask();
		theActiveWindow = FrontWindow();		/* Used often, avoid repeated calls */
		
		if (!MFhere)
			i = GetNextEvent(everyEvent, &myEvent);
		else
			i = WaitNextEvent(everyEvent, &myEvent, 3, 0L);		/* MF only */

		if ( !i ) {
			 
/*
*  network events
*/
			donetevents();
			
			 if (callme) {						/* menu selection redirection */
			 	(*callme)(callmw);
				callme = NULL;
				fmw = findm(FrontWindow());		/* which window is front now? */
			}
			else {

/*
*  Cursor checking and background I-beam idling.
*/
				mw = Mlist;
				while (mw) {

					if (mw->wintype == FNOTES && mw->win == theActiveWindow) {
						SetPort(mw->win);
						GetMouse(&mousePt);
						SetCursor(ptinrect(&mousePt, &mw->nw->vis) ? *ibeamHdl : &qd.arrow);
						TEIdle(mw->nw->trec);
						mw = mw->next;
					}
					else
						mw = mw->next;
				}
			}
			
			setgreys();							/* check menu status, grey out? */

			continue;
		}
		
		SetCursor(&qd.arrow);
/*
*  main event case statement
*/
		switch (myEvent.what) {
			case mouseDown:
				switch (findwindow(&myEvent.where, &whichWindow)) {
					case inSysWindow:
						SystemClick(&myEvent, whichWindow);
						break;
					case inMenuBar:
						doCommand(menuselect(&myEvent.where));
						break;
					case inDrag:
						dragwindow(whichWindow, &myEvent.where, &dragRect);
						break;
					case inGoAway:
						GlobalToLocal(&myEvent.where);
						if (trackgoaway(whichWindow, &myEvent.where)) {
							if (mw = findm(whichWindow)) {
								switch (mw->wintype) {
								case FTEXT:
									loseit(mw);
									break;
								case FIMG:
									loseimage(mw);
									break;
								case FBI:
									loseinterp(mw);
									break;
								case FPOL:
									losepolar(mw);
									break;
								case FNOTES:
									losenotes(mw);
									break;
								}
							}
						}
						break;
					case inGrow:
						mw = findm(whichWindow);
						if (mw && mw->wintype == FTEXT) {
							growfloat(mw, &myEvent.where);
							break;
						}
					case inContent:
						if (whichWindow != theActiveWindow) {
							SelectWindow(whichWindow);
							break;
						}
						mw = findm(whichWindow);
						if (mw) switch (mw->wintype) {
							case FTEXT:
								mousefloat(mw, &myEvent.where, myEvent.modifiers);
								break;
							case FIMG:
							case FBI:
								mouseimg(mw, &myEvent.where, myEvent.modifiers);
								break;
							case FPOL:
								mousepol(mw, &myEvent.where, myEvent.modifiers);
								break;
							case FNOTES:
								SetPort(mw->win);
								GlobalToLocal(&myEvent.where);
								TEClick(myEvent.where, (myEvent.modifiers & shiftKey), mw->nw->trec);
								break;
							default:
								break;
						}
						
					default:
						break;
				}/*endsw findwindow*/
				break;

			case diskEvt:			/* check to see if disk needs to be initialized */
				mousePt.h = 100; mousePt.v = 120;
				if (noErr != (HIWORD(myEvent.message))) {
					dibadmount( &mousePt, myEvent.message );
				}
				break;
				
			case keyDown:
			case autoKey:
				if (myEvent.modifiers & cmdKey) {
					doCommand(MenuKey(myEvent.message & charCodeMask));
				}
				else if (fmw && fmw->wintype == FNOTES) {
					TEKey((char) (myEvent.message & charCodeMask), fmw->nw->trec);
					fmw->dat->needsave=1;			/* note change has been made */
				}
#ifdef DEBUGxx
				else if ((((char)(myEvent.message & charCodeMask)) == 'W')) {	/* special window */
					/* specialdebug(fmw); */
				}
#endif
				break;

			case activateEvt:
				mw = findm((WindowPtr) myEvent.message);
				if (myEvent.modifiers & activeFlag)
					fmw = mw;
				else
					fmw = NULL;
				if (mw) {
					SetCursor(&qd.arrow);
/*
*  set menus for TEXT or IMG window, disable/enable items
*/
					if (myEvent.modifiers & activeFlag) {
						Disit(MUndo);
						if (mw->wintype == FNOTES) {
							Disit(MSee_Notebook);
							Enabit(MCalculate_From_Notes);
						}
						else {
							Enabit(MSee_Notebook);
							Disit(MCalculate_From_Notes);
						}
					}
					else
						Enabit(MUndo);							/* for DAs */

					if (mw->wintype == FTEXT) {
						if (myEvent.modifiers & activeFlag) {
							HiliteControl(mw->cw->vbar, 0);
							HiliteControl(mw->cw->hbar, 0);
							DrawGrowIcon(mw->win);
						}
						else {
							HiliteControl(mw->cw->vbar, 255);
							HiliteControl(mw->cw->hbar, 255);
							DrawGrowIcon(mw->win);
						}
					}
					else if (mw->wintype == FNOTES) {
						if (myEvent.modifiers & activeFlag) {
							TEActivate(mw->nw->trec);
						} 
						else 
							TEDeactivate(mw->nw->trec);
					}
				}

				break;

			case updateEvt:
				mw = findm((WindowPtr) myEvent.message);	/* find right window */
				if (mw) {
					BeginUpdate(mw->win);
					switch (mw->wintype) {
					case FTEXT:						/* text window */
						controlfloat(mw);			/* scroll bars first, better feedback */
						drawfloat(mw,0,0);
						break;
					case FBI:
					case FPOL:
					case FIMG:						/* image window */
						copyimg(mw);
						break;
					case FNOTES:
						SetPort(mw->win);
						EraseRect(&mw->nw->vis);
						TEUpdate(&mw->nw->vis, mw->nw->trec);
						break;
					}
					EndUpdate(mw->win);
				}
					
				break;
				
			case app4Evt:
				switch(( myEvent.message >>24) &0xff) {		/* App4 is a multi-event event */
				case switchEvt:
					if (myEvent.message & 0x20)
						/*Convert clipboard here if necc. (it is not)*/;

					if (myEvent.message & 0x1) {		/* Resume Event */
						GrafPtr window;

						window = FrontWindow();	
						if ( (long)window != 0L) {
							myEvent.message = (long) window;
							myEvent.modifiers |= activeFlag;
							myEvent.what = activateEvt;
							myEvent.when = TickCount();
							SystemEvent( &myEvent);
						}
					}
					else {								/* Suspend Event */
						GrafPtr window;

						if ((window = FrontWindow())) {
							if ( (long)window != 0L) {
								myEvent.message = (long) window;
								myEvent.modifiers &= (~activeFlag);
								myEvent.what = activateEvt;
								myEvent.when = TickCount();
								SystemEvent( &myEvent);
							}
						}
					}
					break;			/* switch of myEvent.message >>24 */
				}
				break;				/* switch app4evt */

			default:
				break;

		}/*endsw myEvent.what*/

	}/*endfor Main Event loop*/

/*
*  Give windows back to the window manager.  Partially restores the color environment.
*/
	mw = Mlist;
	while (mw) {
		DisposeWindow(mw->win);
		mw = mw->next;
	}
	
	stopnet();   /* NET */
/*
*  restore color settings - system level.
*/
	colorrestore();

	return(0);				/* Return from main() to allow C runtime cleanup */
}

void
setupMenus()
{
	extern MenuHandle	MyMenus[];
	register MenuHandle *pMenu;

	MyMenus[appleMenu] = GetMenu(appleID);
	AddResMenu(MyMenus[appleMenu], (ResType) 'DRVR');
	/*
	 * Now the File and Edit menus.
	 */
	MyMenus[fileMenu] = GetMenu(fileID);
	MyMenus[editMenu] = GetMenu(editID);
	MyMenus[imageMenu] = GetMenu(imageID);
	MyMenus[numMenu] = GetMenu(numID);
	/*
	 * Now insert all of the application menus in the menu bar.
	 */
	for (pMenu = &MyMenus[0]; pMenu < &MyMenus[menuCount]; ++pMenu) {
		InsertMenu(*pMenu, 0);
	}

#ifdef DEBUG
	appendmenu( MyMenus[numMenu], "size" );
#endif

	DrawMenuBar();

	return;
}


/******************************************************************/
/* Disit and Enabit
*  Disable or Enable a menu item with the new numbering scheme.
*  Menu items are equal to 100 + the menu item number.
*/
Disit(n)
	int n;
{
	DisableItem(MyMenus[n / 100], n % 100);
}

Enabit(n)
	int n;
{
	EnableItem(MyMenus[n / 100], n % 100);
}

/******************************************************************/
/*  doCommand
 * Process mouse clicks in menu bar.
 *
 */
void
doCommand(mResult)
long mResult;
{
	extern MenuHandle	MyMenus[];
	extern Boolean		DoneFlag;
	int 				theMenu, theItem;
	Str255				daName;
	GrafPtr 			savePort;

	theMenu = HIWORD(mResult);					/* This is the resource ID */
	theItem = LOWORD(mResult);
	
	switch (theMenu) {							/* translation to menu array # */
		case appleID:
			if (theItem != MAbout_DataScope) {
				GetItem(MyMenus[appleMenu], theItem, daName);
				OpenDeskAcc(daName);
			}
			theMenu = 0;
			break;
		case fileID:
			theMenu = 100;
			break;
		case editID:
			/*
			 * If this is for a 'standard' edit item,
			 * run it through SystemEdit first.
			 * SystemEdit will return FALSE if it's not a system window.
			 */
			if (SystemEdit(theItem-1)) {
				HiliteMenu(0);					/* we are done, return */
				return;
			}
			theMenu = 200;
			break;
		case imageID:
			theMenu = 300;
			break;
		case numID:
			theMenu = 400;
			break;
	}
	theItem += theMenu;
/*
*  theItem is a calculated unique ID for the menu selection.
*  Menu*100 + item number.  Used to make the switch statement one level for
*  convenience.
*/

	callmw = NULL;

	switch (theItem) {
	case MAbout_DataScope:				/* About Dialog processing */
		showAboutMeDialog();
		break;

	case MOpen:
		callme = openfloat;				/* Open an HDF file */
		callmw = fmw;
		break;

	case MClose:
		callme = loseit;				/* Take away all windows associated with data */
		callmw = fmw;
		break;

	case MSave:							/* Save the current window's dataset */
	case MSave_As:
		if (fmw) {
			callme = saveit;
			callmw = fmw;
		}
		break;
		
	case MLoad_Text:					/* Load a Text file in */
		callme = loadtext;
		callmw = fmw;
		break;
		
	case MQuit:							/* Request exit */
		DoneFlag = true;
		/*
		*  One last chance to save the datasets which we have.
		*/
		mw = Mlist;
		while (mw) {
			if (mw->wintype == FTEXT) {
				if (checksave(mw)) {	/* user can save if he likes */
					DoneFlag = false;	/* cancel return, don't quit */
					break;
				}
			}
			mw = mw->next;
		}
		break;
		
	case MUndo:
		break;
		
	case MCut:
		if (fmw && fmw->wintype == FNOTES)
			fmw->dat->needsave = 1;
			/* fall through */
	case MCopy:
		if (fmw)
		switch  (fmw->wintype) {
		case FTEXT:
			copyfloat(fmw);
			break;
		case FNOTES:
			if (theItem == MCut) {
				TECut(fmw->nw->trec);
			} else {
				TECopy(fmw->nw->trec);
			}
			ZeroScrap();
			TEToScrap();
			break;
		default:
			scrapimg(fmw);
			break;
		}
		break;
		
	case MPaste:
		if (fmw) {
			if (fmw->wintype == FNOTES) {	/* paste text only to notebook window */
				fmw->dat->needsave = 1;
				TEFromScrap();
				TEPaste(fmw->nw->trec);
			}
			else {
				callme = dopaste;		/* paste image or data - will create new dataset */
				callmw = fmw;
			}
		}
		else {
			callme = dopaste;
			callmw = NULL;
		}
		
		break;

	case MClear:
		if (fmw && fmw->wintype == FNOTES) {
			fmw->dat->needsave = 1;
			TEFromScrap();
			TEDelete(fmw->nw->trec);
		}
		break;

	case MGenerate_Image:
		if (fmw) {
			callme = makeimage;
			callmw = fmw;
		}
		break;
		
	case MInterpolated_Image:
		if (fmw) {
			callme = interpit;
			callmw = fmw;
		}
		break;
		
	case MPolar_Image:
		if (fmw) {
			callme = polarit;
			callmw = fmw;
		}
		break;
		
	case MLoad_Palette:
			callme = loadpal;
			callmw = fmw;				/* if null, the window pointer won't be used */
		break;
		
	case MAttributes:
		if (fmw) {
			SetCursor(&qd.arrow);
			callme = doatt;
			callmw = fmw;
		}
		break;
		
	case MPolar_Size:
		if (fmw) {
			SetCursor(&qd.arrow);
			callme = dopatt;
			callmw = fmw;
		}
		break;
		
	case MImage_Size:
		if (fmw) {
			SetCursor(&qd.arrow);
			callme = dosize;
			callmw = fmw;
		}
		break;

	case MInterpolate_Size:
		if (fmw) {
			SetCursor(&qd.arrow);
			callme = dosize;
			callmw = fmw;
		}
		break;

	case MSynchronize:
		if (fmw) {
			callme = setsynch;
			callmw = fmw;
		}
		break;
		
	case MExtract_Selection:			/* extract selection region */
		if (fmw) {
			callme = extit;
			callmw = fmw;
		}
		break;	
	
	case MSee_Notebook:
		if (fmw) {
			callme = newnotes;
			callmw = fmw;
		}
		break;
		
	case MCalculate_From_Notes:
		if (fmw && fmw->wintype == FNOTES) {
			callme = calcnotes;
			callmw = fmw;
		}
		break;

	default:
		break;

	}/*endsw theMenu*/

#ifdef DEBUG
	{ int i;
	char s[256];
	sprintf(s, "%d", MaxMem( &i ) );
	setitem( MyMenus[numMenu], 7, s);
	}
#endif

	HiliteMenu(0);

	return;
}

/************************************************************************************/
/*  findm
*   Find which Mac window is associated with the given window pointer.  Returns the
*   Mwin pointer which contains that Mac window.
*/
struct Mwin 
*findm(thewin)
	WindowPtr thewin;
	{
	struct Mwin *m;
	
	m = Mlist;
	
	while (m) {
		if (m->win == thewin)
			return(m);
		m = m->next;
	}
	
	return(NULL);
}

/************************************************************************************/
/*  calcnotes
*   Calculate the value of the expression given and create a new text window for 
*   the answer.
*/
TEHandle lastte;

calcnotes(tw)
	struct Mwin *tw;
	{
	int ret;
	TEPtr tp;
	extern char *expstr;
	extern int explen;
	char *p;
	
	lastte = tw->nw->trec;

	p = (char *) NewPtr(40000);
	if (!p) {
		char s[100];
		sprintf(s,"\n** Out of memory **\n");
		tp = *tw->nw->trec;			/* TErec */
		TESetSelect(tp->selEnd, tp->selEnd, tw->nw->trec);
		TEInsert( s, strlen(s), tw->nw->trec);
		return;
	}
	DisposPtr(p);
/*
*  calculate selection size from the values in the TE record
*/
	tp = *tw->nw->trec;				/* TErec */
	explen = tp->selEnd - tp->selStart;

	HLock(tp->hText);				/* lock text handle */
	expstr = *tp->hText;			/* get pointer */
	expstr += tp->selStart;			/* move pointer to access selection region */

/*
*  If the selection region is nil, use the current line.
*/
	if (explen <= 0) {
		p = expstr = *tp->hText + tp->selStart;		/* string in text field */
		if (tp->selStart > 0)						/* not at start, go back one */
			--expstr;
		while (expstr > *tp->hText && *expstr != '\n')
			--expstr;
		while (*p != '\n' && p < *tp->hText + tp->teLength)
			p++;
		explen = p - expstr;
		TESetSelect(p - *tp->hText, p - *tp->hText, tw->nw->trec);
	}
	
	if (explen <= 0 || explen > 20000) {
		HUnlock(tp->hText);
		return;
	}

/*
*  at this point, the starting pointer and len are set for access by the parser.
*  Parser should not modify anything about the pointed-to data.
*/
	ret = execit();				/* call yacc-derived parser and executer */
	
	tp = *tw->nw->trec;			/* TErec */
	HUnlock(tp->hText);		
	
	SetCursor(&qd.arrow);
	
	if (ret) {					/* error return from execit */
		char s[100];
		if (ret < 0)
			sprintf(s,"\n**Parsing error.\n");
		else
			sprintf(s,"\n**Error in calculation.\n");

		tp = *tw->nw->trec;				/* TErec */
		TESetSelect(tp->selEnd, tp->selEnd, tw->nw->trec);
		TEInsert( s, strlen(s), tw->nw->trec);
	}
	

}

/************************************************************************************/
/* ans_const
*  If the calculation result is a constant, then we get a call back.
*/
ans_const(v)
	float v;
	{
	char s[100];
	
	sprintf(s,"\nResult: %f\n",v);
	TESetSelect((*lastte)->selEnd, (*lastte)->selEnd, lastte);
	TEInsert( s, strlen(s), lastte);
	
}

/************************************************************************************/
/*  newnotes
*  create a new notes window for the current window.  Can be any window with an 
*  associated dataset.
*/
newnotes(tw)
	struct Mwin *tw;
	{
	struct Mwin *tempw;
	struct fdatawin *td;
	struct fnotewin *tn;
	
/*
*  must have a data var
*/
	if (!tw->dat)
		return;
/*
*  if it already has a notes window, bring it to the front
*/
	if (tw->dat->notes)	{
		SelectWindow(tw->dat->notes->win);
		return;
	}
/*
*  link up new window into win list.
*/
	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
	if (!tempw)
		return;
		
	tempw->dat = tw->dat;			/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	tempw->nw = (struct fnotewin *)NewPtr(sizeof(struct fnotewin));
	
	tempw->next = tw->next;			/* install in list */
	tw->next = tempw;
	tw = tempw;						/* move to this element of list */
	
	tw->dat->notes = tw;			/* back pointer for text references */
	tw->wintype = FNOTES;			/* indicate the notes type flag */
	
	td = tw->dat;					/* help for later de-referencing */
	tn = tw->nw;					/* image window stuff */
	
	if (NULL == (tw->win = GetNewCWindow(imgwinID, NULL, (WindowPtr) -1)))
		return;

	setwtitle(tw->win,td->dvar);			/* a meaningful window title */
	
	MoveWindow(tw->win,startwin.h,startwin.v,false);
	newstartwin();

	ShowWindow(tw->win);
	SelectWindow(tw->win);
	SetPort(tw->win);

	tn->vis = tw->win->portRect;			/* rectangle definition in window */
	InsetRect(&tn->vis, 4, 0);
	tn->trec = TENew(&tn->vis, &tn->vis);	/* Not growable, so destRect == viewRect */
	
/*
*  Set up existing notebook contents into the textedit record.
*/
	if (tw->dat->content) {					/* we have some saved notes */
		(*tn->trec)->hText = (Handle) tw->dat->content;
		(*tn->trec)->teLength = tw->dat->contentlen;
		tw->dat->content = NULL;
		TECalText(tn->trec);
	}
	
	TESetSelect(0,0,tn->trec);				/* set selection to beginning */
	TEAutoView(true, tn->trec);				/* set autoscroll */

}

/************************************************************************************/
/*  emptyimage
*   Take a dataset and create a blank image window for that dataset.
*   Needs to know what type of image will be placed in here.
*/

emptyimage(tw,wtype)
	struct Mwin *tw;
	int wtype;						/* FIMG, FBI, FPOL */
	{
	struct Mwin *keepw,*tempw;
	int winwide,wintall,virtualwide,virtualtall;
	struct fdatawin *td;
	
	if (!tw->dat)
		return;
		
	keepw = tw;						/* need a copy for later insertion into list */

	td = tw->dat;					/* help for later de-referencing */

	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
	if (!tempw)
		return(-1);
		
	tempw->dat = tw->dat;			/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	tw = tempw;						/* move to this element of list */
	
	tw->wintype = wtype;			/* indicate the image type flag */

	switch (wtype) {
		case FIMG:
			if (checkmem(td->xdim*td->exx*td->ydim*td->exy+1000))
				return(-1);
			tw->iw = (struct fimgwin *)NewPtr(sizeof(struct fimgwin));	
			tw->dat->image = tw;					/* back pointer for text references */
			SetRect(&tw->iw->xr,-11,-11,-11,-11);	/* flag that box isn't set yet */
			tw->iw->exx = td->exx;
			tw->iw->exy = td->exy;
			tw->iw->xsize = winwide = td->xsize;
			tw->iw->ysize = wintall = td->ysize;
			virtualwide = td->xdim;
			virtualtall = td->ydim;
			break;
		case FBI:
			if (checkmem(td->xdim*td->exx*td->ydim*td->exy+1000))
				return(-1);
			tw->bw = (struct fbiwin *)NewPtr(sizeof(struct fbiwin));	
			tw->dat->interp = tw;					/* back pointer for text references */
			SetRect(&tw->bw->xr,-11,-11,-11,-11);	/* flag that box isn't set yet */
			tw->bw->exx = td->exx;
			tw->bw->exy = td->exy;
			winwide = virtualwide = tw->bw->xsize = td->xsize;
			wintall = virtualtall = tw->bw->ysize = td->ysize;
			break;
		case FPOL:
			if (checkmem(td->ydim*td->exx*td->ydim*td->exx+1000))
				return(-1);
			tw->pw = (struct fpolwin *)NewPtr(sizeof(struct fpolwin));	
			tw->dat->polar = tw;					/* back pointer for text references */
			SetRect(&tw->pw->xr,-11,-11,-11,-11);	/* flag that box isn't set yet */
			td->exy = td->exx;
			tw->pw->exx = td->exx;
			tw->pw->exy = td->exy;
			tw->pw->xsize = td->xsize;
			tw->pw->ysize = td->ysize;
			tw->pw->viewport = td->viewport;
			tw->pw->angleshift = td->angleshift;
			winwide = virtualwide = td->xsize = (td->viewport.right - td->viewport.left)*td->exx;
			wintall = virtualtall = td->ysize = (td->viewport.bottom - td->viewport.top)*td->exx;
			break;
		default:
			return(-1);
	}
	
	if ( 0 > blankimg(tw, &tw->vdev, winwide,wintall, virtualwide, virtualtall))
		return;						/* set up window stuff */

/*
*  After getting this far, we know that the window belongs in the system list.
*/
	tw->next = keepw->next;			/* install in list */
	keepw->next = tw;
	
	return(0);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	makeimage		Generate an image from the data in the current float
					window.  In this version, we force the xsize and
					ysize to be a multiple of the array dimensions.
					
				tw = pointer to the Mwin structure
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
makeimage(tw)
	struct Mwin *tw;
{
	struct Mwin				*keepw,*tempw;
	register int			i,j;
	int						wide,tall,down,oddone,spots,cmin,cmax,crange;
	register float			*f,invrange,tmin;
	register unsigned char	*p;
	unsigned char			*pstart;
	struct fdatawin			*td;
	struct fimgwin			*ti;
	void					ErrorAlert();
	extern struct Mwin 		*finddata ();
	extern struct Mwin 		*findimage ();

	if (!tw->dat)
		return;
		
	keepw = tw;						/* need a copy for later insertion into list */
	td = tw->dat;					/* help for later de-referencing */

	if (td->image)	{
		/* did the interpolation size change. */
		
		tempw = findimage (tw);
		if (td->xsize != tempw->iw->xsize || td->ysize != tempw->iw->ysize) {
			if (td->image == tw)
				keepw = tw = finddata (tw);
				
			loseimage (td->image);
			td = tw->dat;
		} else {
			tw = td->image;
			setwtitle(tw->win,td->dvar);	/* could be a new window title */
			goto genimage;				/* nasty hack - we already have mem allocated */
		}	
	}	

	if (td->xsize < 50) {			/* minimum sizes for window to create */
		td->xsize = 50;
		td->exx = 50/td->xdim;
	}
	if (td->ysize < 50) {
		td->ysize = 50;
		td->exy = 50/td->ydim;
	}

/*
*  Are we going to have enough memory to do this?
*/

	if (checkmem(td->xdim*td->ydim+2000))
	   {ErrorAlert(GetResource('STR ',1001));
		return;
	   }

	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
	if (!tempw)
	   {ErrorAlert(GetResource('STR ',1001));
		return;
	   }
		
	tempw->dat = tw->dat;			/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	tempw->iw = (struct fimgwin *)NewPtr(sizeof(struct fimgwin));
	
	tw = tempw;						/* move to this element of list */
	
	tw->dat->image = tw;			/* back pointer for text references */
	tw->wintype = FIMG;				/* indicate the image type flag */
	
	ti = tw->iw;					/* image window stuff */

	SetRect(&ti->xr,-11,-11,-11,-11);				/* flag that box isn't set yet */
	ti->exx = td->exx;
	ti->exy = td->exy;
	ti->xsize = td->xsize = td->exx*td->xdim;		/* must be a multiple of dimension */
	ti->ysize = td->ysize = td->exy*td->ydim;

	if ( 0 > blankimg(tw,&tw->vdev, ti->xsize, ti->ysize, td->xdim, td->ydim))
	   {ErrorAlert(GetResource('STR ',1016));
		return;
	   }
	
/*
*  After getting this far, we know that the window belongs in the system list.
*/
	tw->next = keepw->next;							/* install in list */
	keepw->next = tw;

genimage:
	tall = td->ydim;
	wide = td->xdim;
	
	f = td->vals;									/* floating pt data */
	p = pstart = tw->vdev.bp;						/* data space available */
	tmin = td->valmin;
	cmin = td->cmin;
	cmax = td->cmax;
	crange = td->cmax - td->cmin;
	invrange = 1.0/(td->valmax - tmin);
	if (invrange < 0) invrange = -invrange;			/* max must be > min */
	spots = 0;
	oddone = 0;
	if (wide & 1)
		oddone = 1;									/* has odd width */
	
	for (down=0; down < tall; down++) {
		
		if ((p - pstart) - spots*0x1000 > 0x1000) {	/* something for user to watch */
			SystemTask();
			if (calcidle())
				down = tall;
			/* copyimg(tw); */
			spots++;
		}

		for (i = 0; i < wide; i++) {

			j = cmin + crange*(*f++ - tmin)*invrange;	/* cut scale a little short */
			if (j >= cmax || j < cmin)
				*p++ = 0;
			else
				*p++ = (char)j;
		}
		
		if (oddone)
			*p++ = 0;								/* end of the line for odd width */
		
	}
	
	copyimg(tw);
	SetCursor(&qd.arrow);
	
}

/*******************************************************************************/
/*  blankimg
*   create a blank image window and a pixmap with palette.
*/
blankimg(tw,vdev,xbig,ybig,xsp,ysp)
	struct Mwin *tw;
	VDevicePtr vdev;
	int xbig,ybig,xsp,ysp;
	{
	unsigned char *p;
	PaletteHandle pal;
	struct fdatawin *td;
	Rect tr;
	
	td = tw->dat;								/* help for later de-referencing */
	
	SetRect(&tw->pref, 0,0, xsp,ysp);			/* preferred bounds rectangle */
	
	if (xsp & 1)
		xsp++;									/* odd width, add one */
/*
*  Try the big memory allocation first in case it fails.
*/
	if (NULL == (p = (char *) NewPtr( xsp*ysp ))) {		/* room for bitmap */
		return(-1);
	}
	
/*
*  The window to display it in.
*/
	if (NULL == (tw->win = GetNewCWindow(imgwinID, NULL, (WindowPtr) -1)))
		return(-1);

	SizeWindow(tw->win, xbig, ybig, false);
	setwtitle(tw->win,td->dvar);				/* a meaningful window title */
	
	MoveWindow(tw->win,startwin.h,startwin.v,false);
	newstartwin();
	ShowWindow(tw->win);
/*	SelectWindow(tw->win);
	
/*
*  create the palette for the window, will be all black before setpal
*/
	pal = NewPalette(255, NULL, pmTolerant, 0);
	SetPalette( tw->win, pal, false);
	ActivatePalette(tw->win);

	SetRect(&tr, 0,0, xsp,ysp);			/* bounds rectangle */
	vdev->bounds = tr;
	vdev->bp = p;						/* space for pixel storage */

	InitVDevice(vdev);					/* the off-screen drawing space */
	
	setpal(tw, rgbsave);				/* init with saved global color table */
	
	SetPort(tw->win);
	EraseRect(&tr);
	
}	


/*******************************************************************************/
/*  setpal
*  Take a window and palette - set the colors for it and the off-screen device.
*/
setpal(tw,rgb)
	struct Mwin *tw;
	unsigned char *rgb;
	{
	RGBColor curcol;
	register int i;
	register unsigned char *p;
	PaletteHandle pal;
	
	if (!tw || !tw->win ||
		(tw->wintype != FBI && tw->wintype != FIMG && tw->wintype != FPOL))
		return(-1);
/*
*  set up palette for window and off-screen device.
*  We assign a set of colors to indices inside the palette. 
*  By making the palette manager active, the colors needed for the window are always in
*  place when the window is frontmost.
*
*/
	pal = GetPalette(tw->win);					/* get existing palette */
	
	p = rgb;
	
	for (i=0; i<255; i++) {
		curcol.red = (*p++)<<8;
		curcol.green = (*p++)<<8;
		curcol.blue = (*p++)<<8;
		SetEntryColor( pal, i, &curcol);
	}
		
	SetPalette( tw->win, pal, true);			/* set new palette back */
	ActivatePalette(tw->win);

/*
*  apply the new colors to the hidden color table in the gdevice and cport.
*/
	ColorVDevice(&tw->vdev,pal);
	
	return(0);
	
}

/*******************************************************************************/
/*  newstartwin
*   Update the variable which contains the location for the next upper left corner
*   starting position.
*/
newstartwin()
	{
	startwin.h += 15;
	startwin.v += 20;
	if (startwin.v > 175)
		 startwin.v = 50;
	if (startwin.h > 300) {
		startwin.v = 50;
		startwin.h = 20;
	}
		 
}

/***************************************************************************************/
/*  setsynch
*  Set up the synch flag for the program.
*/
setsynch(tw)
	struct Mwin *tw;
	{
	MenuHandle men;
			
	if (synchro)			/* what is current setting of synch parm?, flip it */
		synchro = 0;
	else 
		synchro = 1;
	
	men = GetMenu( numID );
	CheckItem(men, MSynchronize % 100, synchro);		/* show menu checkmark */
	
}

/***************************************************************************************/
/*  rmwin
*   Take a window out of the list.
*/
rmwin(tw)
	struct Mwin *tw;
	{
	struct Mwin *oldw,*tmpw;
	
	if (Mlist == tw) {
		Mlist = tw->next;						/* at top of the list */
	}
	else {
		tmpw = oldw = Mlist;
		while (tmpw = tmpw->next) {				/* search list */
			if (tmpw == tw) {					/* got match */
				oldw->next = tmpw->next;		/* remove it */
				return;							/* now done */
			}
			oldw = tmpw;
		}
	}
	
}

/***************************************************************************************/
/*  synchfloat
*   Call setfloat for each of the floating point windows which have the synch flag
*   on.  Will invalidate the display rects for those windows and change their selection
*   region to match this one.
*/
synchfloat(tw,sr,metoo)
	struct Mwin *tw;
	Rect *sr;
	int metoo;
	{
		
/*
*  If synch is not on, we have a boring situation, check the metoo flag and update
*  the current float window (maybe) and the image window for that float window.
*/
	if (!synchro) {
	/*
	*  no synchro set, so we do the littlebox for the image window 
	*/
		if (tw->dat->image) 
			littlebox(tw->dat->image,sr,1);			/* draw that box */
		if (tw->dat->interp)
			littlebox(tw->dat->interp,sr,1);		/* draw that box */
		if (tw->dat->polar)
			littlesect(tw->dat->polar,sr,1);		/* draw in the polar window */
			
		if (metoo)
			setfloat(tw->dat->text,sr);				/* invalidate my window */
		return;
	}

/*
*  search list for matching windows and update them too.
*/
	mw = Mlist;
	
	while (mw) {
		if ( mw->dat->xdim == tw->dat->xdim
			&& mw->dat->ydim == tw->dat->ydim) {
			
			if (mw->wintype == FTEXT &&			/* unless suppressed with metoo, */
				(metoo || tw != mw))			/* set new text selection in text win */
				setfloat(mw,sr);
			else if (mw->wintype == FIMG ||
				mw->wintype == FBI) {			/* do littlebox in all synched image wins */
				littlebox(mw,sr,1);				/* draw that box */
			}
			else if (mw->wintype == FPOL)
				littlesect(mw,sr,1);			/* draw in the polar window */
		}
			
			
		mw = mw->next;
	}
}

/***************************************************************************************/
/*  extit
*  Extract the current selection region and make a new text window out of it.
*/
extit(tw)
	struct Mwin *tw;
	{
	struct ftextwin *tc;
	struct fdatawin *td,*extractdata();
	Rect tr;
	
	if (!tw->dat || !tw->dat->text)
		return;
		
	tc = tw->dat->text->cw;
	tr = tc->sel;
	if (tr.bottom - tr.top < 1 || tr.right - tr.left < 1 ||
		tr.top < 0 || tr.bottom < 0 ||
		tr.right < 0 || tr.left < 0)
		return;
	if (NULL == (td = extractdata(tw->dat,&tr)))	/* copies the data record */
		return;
	
	td->dvar[17] = '\0';						/* make sure it is short enough */
	strcat(td->dvar,"_x");						/* slightly modify variable name */
	
	ctextwin(td);								/* creates the new text win */

}

/***************************************************************************************/
/*  calcidle
*   Show progress for calculations and check for command-period being used to abort 
*   the calculation.  Return 0 for ok status and 1 for user-abort.
*/
int watchstate=0,nextidle=0;

calcidle()
	{
	EventRecord 	myevent;
/*
*  give up a little system time
*/
	SystemTask();
	
/*
*  If being called too soon after last time, just return.
*/
	if (TickCount() < nextidle)
		return(0);
	
	nextidle = TickCount() + 30; 			/* one-half second intervals */
/*
*  set spinning watch cursor 
*/
	SetCursor(*spinwatch[watchstate]);
	watchstate = ++watchstate % 7;
/*
*  check keyboard for command-period which indicates a user-cancel command.
*/

	if (GetNextEvent(keyDownMask, &myevent)) {				/* if we have a keydown event, */
		if (myevent.what == keyDown &&						/* and event is keyboard */
			(myevent.modifiers & cmdKey) &&					/* and command is held down */
			(myevent.message & charCodeMask) == '.')		/* and is a period */
			return(1);										/* stop the processing */
	}
	
	return(0);
}


/***************************************************************************************/
/*  saveit
*   Save the current text window's data and configuration so that the open command
*   will automatically get it back.
*/
saveit(tw)
	struct Mwin *tw;
	{
	SFReply reply;
	 Point wh;
	int reccr = 'NCSf',rectype = '_HDF';

	wh.h = wh.v = 50;
	sfputfile(&wh, "Name for saved HDF file", tw->dat->fname, nil, &reply);
	if (reply.good) {
		p2cstr(reply.fName);
		setvol(NULL, reply.vRefNum);			/* set to this volume (dir) */
		
/*
*  create the file with FScreate
*/
		create(reply.fName, reply.vRefNum, reccr, rectype);

		savedf(reply.fName, tw->dat);			/* write the data into it */
		return(0);
	}
	
	return(-1);	
}

/***************************************************************************/
/*  set greys for current mlist state.
*/
char greyon=1;

setgreys()
	{
	
	if (greyon && !Mlist) {			/* if we are turning them off */	
		Disit(MClose);
		Disit(MSave);
		Disit(MSave_As);
		Disit(MGenerate_Image);
		Disit(MInterpolated_Image);
		Disit(MPolar_Image);
		Disit(MAttributes);
		Disit(MImage_Size);
		Disit(MInterpolate_Size);
		Disit(MPolar_Size);
		Disit(MSynchronize);
		Disit(MExtract_Selection);
		Disit(MSee_Notebook);
		Disit(MCalculate_From_Notes);

		greyon = 0;
	}
	else if (!greyon && Mlist) {	/* if we are turning them on */
		Enabit(MClose);
		Enabit(MSave);
		Enabit(MSave_As);
		Enabit(MGenerate_Image);
		Enabit(MInterpolated_Image);
		Enabit(MPolar_Image);
		Enabit(MAttributes);
		Enabit(MImage_Size);
		Enabit(MInterpolate_Size);
		Enabit(MPolar_Size);
		Enabit(MSynchronize);
		Enabit(MExtract_Selection);
		Enabit(MSee_Notebook);
		
		greyon = 1;
	}

}

/***************************************************************************/
/*  colorsave and colorrestore
*  save and restore the color state of the current gdevice.
*/

struct {
	short size;
	short reqdata[260];
} sel;

CTabHandle gtab;

colorsave()
	{
	int i;
	
	sel.size = 255;
	for (i=0; i<256; i++)
		sel.reqdata[i] = i;
	
/*
*  extra space gives room for the Ctab struct
*/
	gtab = (CTabHandle)NewHandle(260*sizeof(ColorSpec)); 
	
	SaveEntries( NULL, gtab, (ReqListRec *) &sel);
	
}

colorrestore()
	{
	CGrafPtr cgp;
	
	WindowPtr cw;

	RestoreEntries(gtab, NULL, (ReqListRec *) &sel);

	cw = newcwindow(nil, &qd.screenBits.bounds, "Restoring Colors", true,
		2, nil, false, 0);

/*
*  We created a color window.
*  Now we change the color table seed value so that the color environment must reset
*  itself when the window is disposed.
*
*  We had to create the color window the size of the screen because it must invalidate the
*  background and all of the other windows under multifinder.  To date, they are too stupid
*  to remember to update themselves when the color environment changes back.
*
*  This means that there is, in effect a "color" update region.  That region where the
*  windows must re-draw themselves to get the right colors back.  When we are done, we do
*  so much damage that that region must become the whole screen.
*/
	cgp = (CGrafPtr) cw;
	(*((*(cgp->portPixMap))->pmTable))->ctSeed = GetCTSeed();
	
	DisposeWindow(cw);

}
#define	GENALRT			 500
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ErrorAlert		This routine will display an alert box and insert in
					it the passed string
					
				s = handle to the char string
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void ErrorAlert (s)
	char		**s;
{
	ParamText (*s, "", "", "");
	StopAlert ((short) GENALRT, NULL);

	return;
}

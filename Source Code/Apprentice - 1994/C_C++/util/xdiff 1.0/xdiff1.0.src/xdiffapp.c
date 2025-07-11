
/*
*  xdiff
*  Compares two text files and outputs a differences file which can be used to
*  execute duplicate edits on another system.
*
*
*  National Center for Supercomputing Applications
*  March 7, 1988
*
*/
/*
 * File xdiff.c
 *
 * Tim Krauskopf
 *
 * Portions Copyright Apple Computer, Inc. 1985, 1986
 * All rights reserved.
 *
 */

# include <types.h> 				/* Nearly always required */
# include <quickdraw.h> 			/* To access the qd globals */
# include <toolutils.h> 			/* CursHandle and iBeamCursor */
# include <fonts.h> 				/* Only for InitFonts() trap */
# include <events.h>				/* GetNextEvent(), ... */
# include <windows.h>				/* GetNewWindow(), ... */
# include <controls.h>
# include <lists.h>
# include <files.h>
# include <fcntl.h>
# include "OSEvents.h"
# include <packages.h>
# include <dialogs.h>				/* InitDialogs() and GetNewDialog() */
# include <menus.h> 				/* EnableItem(), DisableItem() */
# include <desk.h>					/* SystemTask(), SystemClick() */
# include <textedit.h>				/* TENew() */
# include <scrap.h> 				/* ZeroScrap() */
# include <StdIO.h>

/*
 * Resource ID constants.
 */
# define appleID		128 			/* This is a resource ID */
# define fileID 		129 			/* ditto */
# define editID 		130 			/* ditto */

# define appleMenu		0				/* MyMenus[] array indexes */
# define	aboutMeCommand	1

# define fileMenu		1
# define	lgoCommand  1
# define    changeCommand 2
# define	quitCommand 3

# define editMenu		2
# define	undoCommand 	1
# define	cutCommand		3
# define	copyCommand 	4
# define	pasteCommand	5
# define	clearCommand	6

# define menuCount	 3
/*
 * For the one and only text window
 */
# define windowID		128
# define drawinID  129
/*
 * For the About Sample... DLOG
 */
# define aboutMeDLOG	128
# define newuDLOG  129
# define newpDLOG  130
# define changefnDLOG 131
# define openDLOG 132

# define	okButton		1
/*
*  For the controls
*/
#define Cdiff 129
#define Ccont 130
#define Cedit 131
#define Clgo  132
#define Cquit 133
#define NCS 5

# define SETRECT(rectp, _top, _left,  _bottom, _right)	\
	(rectp)->left = (_left), (rectp)->top = (_top), 	\
	(rectp)->right = (_right), (rectp)->bottom = (_bottom)

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

	register
		WindowPtr	myWindow;			/* Referenced often */
		ControlHandle mycontrol[NCS];
 		 SFReply reply;
		 SFTypeList tlst;
		 Point wh;
		 
/*
*  data structure for timk
*/

	short int 
		myvol,f1vol,f2vol;
	
	Point lastPt = {0,0},    /* last point */
		csize = { 15,200 };
		
	Rect r1,r2,r3;
	Rect rview ,
		dbounds ;
	AppFile theFile;
	char s[256],currentfile[256]={""};
/*
*  data structures for diff program
*
*/

#include "xdiff.h"

long mycreator = 'xdif',mytype = 'TEXT';

diffile
	f1,f2;				/* storage for files to work on */
	
Rect
	rframe;				/* working area */

pascal Ptr malloc(int);

char space[500],
	fileout[256]={"Xdiff.up"};	/* diff output file name */
register char *p;				/* fast temporary work */
register int i;
int difflen=0;

Point
		tpt;					/* temporary point */
Rect
		tr;						/* temporary rect */
	
FILE *fp,*myopen();

int
main()
{
	extern void 	setupMenus();
	extern void 	doCommand();
	Rect			screenRect, dragRect;
	Point			mousePt;
	EventRecord 	myEvent;
	WindowRecord	wRecord;
	WindowPtr		theActiveWindow, whichWindow;
	int i,timer;
	short int shorti,shortj;
	/*
	 * Initialization traps
	 */
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	InitDialogs(nil);
	InitCursor();
	/*
	 * setupMenus is execute-once code, so we can unload it now.
	 */
	setupMenus();			/* Local procedure, below */
	
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	/*
	 * setupMenus is execute-once code, so we can unload it now.
	 */
	openingdialog(1);				/* show opening message */
	timer = TickCount() + 150;		/* set opening timer */	
/*
*  Wait for two 1/2 seconds or until user presses a key.
*/	
	while (timer > TickCount()) {
		if (GetNextEvent(keyDownMask, &myEvent)) {		/* if we have a keydown event, */
			if (myEvent.what == keyDown)
				break;									/* go directly into program */
		}
	}

	/*
	 * Calculate the drag rectangle in advance.
	 * This will be used when dragging a window frame around.
	 * It constrains the area to within 4 pixels from the screen edge
	 * and below the menu bar, which is 20 pixels high.
	 */
	screenRect = qd.screenBits.bounds;
	SETRECT(&dragRect, 20 + 4, 4, screenRect.bottom-4, screenRect.right-4);
	myWindow = GetNewWindow(windowID, (Ptr)&wRecord, (WindowPtr) -1);
	SetPort(myWindow);
/*
*  get controls
*/
	for (i=0; i<NCS; i++)
		mycontrol[i] = GetNewControl(i+129,myWindow);
	DrawControls(myWindow);
	
/*
*  Initialize file structures
*/
	f1.store = NULL;
	f2.store = NULL;
	f1.lstore = NULL;
	f2.lstore = NULL;
	f1.fn[0] = 0;
	f2.fn[0] = 0;
	getvol(s,&myvol);		/* save the directory we started from */
	
/*
*  if we have a selection, load it at start
*/
	CountAppFiles( &shorti, &shortj);
	if (!shorti && shortj > 0) {
		GetAppFiles( shortj, &theFile);			/* get file name */
		p2cstr(theFile.fName);
		strcpy(f1.fn,theFile.fName);			/* keep file name */
		f1vol = theFile.vRefNum;
		setvol(NULL, f1vol);
		loadfile(&f1);
		ClrAppFiles(shortj);
	}

	openingdialog(0);		/* remove opening message */

	/*
	 * Ready to go.
	 * Start with a clean event slate, and cycle the main event loop
	 * until the File/Quit menu item sets DoneFlag.
	 *
	 */
	DoneFlag = false;
	for ( ;; ) {
		if (DoneFlag) {
			break;		/* from main event loop */
		}
		/*
		 * Main Event tasks:
		 */
		SystemTask();
		theActiveWindow = FrontWindow();		/* Used often, avoid repeated calls */
		if (myWindow == theActiveWindow) {
			GetMouse(&mousePt);
		}
		if ( ! GetNextEvent(everyEvent, &myEvent)) {
			continue;
		}
		/*
		 * In the unlikely case that the active desk accessory does not
		 * handle mouseDown, keyDown, or other events, GetNextEvent() will
		 * give them to us!  So before we perform actions on some events,
		 * we check to see that the affected window in question is really
		 * our window.
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
						if (trackgoaway(whichWindow, &myEvent.where))
							DoneFlag = true;
						break;
					case inGrow:
						/* There is no grow box. (Fall through) */
					case inContent:
						if (whichWindow != theActiveWindow) {
							SelectWindow(whichWindow);
						}
						GlobalToLocal(&myEvent.where);
						if (whichWindow == myWindow) {
							ControlHandle tempc;
							findcontrol(&myEvent.where,myWindow,&tempc);
							if (tempc)
								if (trackcontrol(tempc, &myEvent.where, NULL)) {
								/* find which one */
								for (i=0; i<NCS; i++)
									if (tempc == mycontrol[i])
										switch (i+129) {
											case Cdiff:
												setvol(NULL,myvol);
												if (NULL != (fp = myopen(fileout,"w"))) {
													compfiles(&f1,&f2,fp);
													difflen = ftell(fp);
													fclose(fp);
													drawthem();
												}
												break;
											case Ccont:
												tpt.h = tpt.v = 50;
												tlst[0] = mytype;
												sfgetfile(&tpt, "Select control file", 
													nil, 1, tlst,nil, &reply);
												if (reply.good) {
													p2cstr(reply.fName);
													setvol(NULL, f1vol = reply.vRefNum);		/* set to this volume (dir) */
													strcpy(f1.fn,reply.fName);
													loadfile(&f1);
													drawthem();
												}
												break;
											case Cedit:
												tpt.h = tpt.v = 50;
												tlst[0] = mytype; 
												sfgetfile(&tpt, "Select edit file", 
													nil, 1, tlst,
														nil, &reply);
												if (reply.good) {
													p2cstr(reply.fName);
													setvol(NULL, f2vol = reply.vRefNum);		/* set to this volume (dir) */
													strcpy(f2.fn,reply.fName);
													loadfile(&f2);
													drawthem();
												}
												break;
											case Clgo:		/* load and diff */
												setvol(NULL, f1vol);
												if (0 > loadfile(&f1))
													break;
												setvol(NULL, f2vol);
												if (0 > loadfile(&f2))
													break;
												setvol(NULL, myvol);
												if (NULL != (fp = myopen(fileout,"w"))) {
													compfiles(&f1,&f2,fp);
													difflen = ftell(fp);
													fclose(fp);
													drawthem();
												}
												break;
											case Cquit:		/* load and diff */
												DoneFlag = true;
												break;
											default:
											break;
										}
										
								}
#ifdef saveme
				/* if something selected, highlight options, if not, dehighlight */
							csize.h = csize.v = 0;
							if (LGetSelect(true, &csize, mylist)) {
								HiliteControl(mycontrol[Cdelete-129], 0);
								HiliteControl(mycontrol[Cpasswd-129], 0);
							}
							else {
								HiliteControl(mycontrol[Cdelete-129], 255);
								HiliteControl(mycontrol[Cpasswd-129], 255);
							}
#endif
						}
						break;
					default:
						break;
				}/*endsw findwindow*/
				break;

			case keyDown:
			case autoKey:
				if (myWindow == theActiveWindow) {
					if (myEvent.modifiers & cmdKey) {
						doCommand(MenuKey(myEvent.message & charCodeMask));
					} else {
						if ('x' == (char) (myEvent.message & charCodeMask))
							DoneFlag = true;
					}
				}
				break;

			case activateEvt:
				if ((WindowPtr) myEvent.message == myWindow) {
				}
				break;

			case updateEvt:
				if ((WindowPtr) myEvent.message == myWindow) {
					BeginUpdate(myWindow);
					DrawControls(myWindow);
					drawthem();
					EndUpdate(myWindow);
				}
				break;

			default:
				break;

		}/*endsw myEvent.what*/

	}/*endfor Main Event loop*/
	/*
	 * No cleanup required, but if there was, it would happen here.
	 */
	return(0);		/* Return from main() to allow C runtime cleanup */
}

/*
 * Set up the Apple, File, and Edit menus.
 * If the MENU resources are missing, we die.
 */
void
setupMenus()
{
	extern MenuHandle	MyMenus[];
	register MenuHandle *pMenu;
	/*
	 * Set up the desk accessories menu.
	 * The "About Sample..." item, followed by a grey line,
	 * is presumed to be already in the resource.  We then
	 * append the desk accessory names from the 'DRVR' resources.
	 */
	MyMenus[appleMenu] = GetMenu(appleID);
	AddResMenu(MyMenus[appleMenu], (ResType) 'DRVR');
	/*
	 * Now the File and Edit menus.
	 */
	MyMenus[fileMenu] = GetMenu(fileID);
	MyMenus[editMenu] = GetMenu(editID);
	/*
	 * Now insert all of the application menus in the menu bar.
	 */
	for (pMenu = &MyMenus[0]; pMenu < &MyMenus[menuCount]; ++pMenu) {
		InsertMenu(*pMenu, 0);
	}

	DrawMenuBar();

	return;
}

/****************************************************************/
/*  putit
*   erase and drawstring
*/
putit(x,y,s)
	int x,y;
	char *s;
	{
	SETRECT(&tr, y-20, x, y+5, x+200);
	EraseRect(&tr);
	MoveTo(x,y);
	drawstring(s);
}

/*
 * Process mouse clicks in menu bar
 */
void
doCommand(mResult)
long mResult;
{
	extern MenuHandle	MyMenus[];
	extern Boolean		DoneFlag;
	extern TEHandle 	TextH;
	int 				theMenu, theItem;
	char				daName[256];
	GrafPtr 			savePort;

	theItem = LOWORD(mResult);
	theMenu = HIWORD(mResult);		/* This is the resource ID */

	switch (theMenu) {
		case appleID:
			if (theItem == aboutMeCommand) {
				showAboutMeDialog();
			} else {
				getitem(MyMenus[appleMenu], theItem, daName);
				GetPort(&savePort);
				(void) opendeskacc(daName);
				SetPort(savePort);
			}
			break;

		case fileID:
			switch (theItem) {
				case lgoCommand:
					setvol(NULL, f1vol);
					if (0 > loadfile(&f1))
						break;
					setvol(NULL, f2vol);
					if (0 > loadfile(&f2))
						break;
					setvol(NULL, myvol);
					if (NULL != (fp = myopen(fileout,"w"))) {
						compfiles(&f1,&f2,fp);
						difflen = ftell(fp);
						fclose(fp);
						drawthem();
					}
					break;
				case changeCommand:
					changefn();
					break;
				case quitCommand:
					DoneFlag = true;			/* Request exit */
					break;
				default:
					break;
			}
			break;
		case editID:
			/*
			 * If this is for a 'standard' edit item,
			 * run it through SystemEdit first.
			 * SystemEdit will return FALSE if it's not a system window.
			 */
			if ((theItem <= clearCommand) && SystemEdit(theItem-1)) {
				break;
			}
			/*
			 * Otherwise, it's my window.
			 */
			switch (theItem) {
				case undoCommand:
					break;
				case cutCommand:
				case copyCommand:
					ZeroScrap();
					break;
				case pasteCommand:
					break;
				case clearCommand:
					break;
				default:
					break;
			} /*endsw theItem*/
			break;

		default:
			break;

	}/*endsw theMenu*/

	HiliteMenu(0);

	return;
}



xbutton()
	{
	return(Button());
}

/*******************************************************/
/*  myopen
*  Open a Mac file with the correct creator and type
*/
FILE
*myopen(s,sp)
	char *s,*sp;
	{
	short int vref;
	Str255 vname;
/*
*  set correct creator and type
*/
	getvol( &vname, &vref);
	
	if (*sp == 'w')
		create(s, vref, mycreator, mytype);
	
	return(fopen(s,sp));
}

/*****************************************************/
/*  drawthem
*  Draw the screen layout
*/
drawthem()
	{
	int32 i;
	
	sprintf(space,"%ld bytes",difflen);
	putit(160,50,space);
	
	if (f1.store) {
		putit(10,100,f1.fn);
		i = f1.len;
	}
	else
		i = (int32)0;
		
	sprintf(space,"%ld bytes",i);
	putit(10,130,space);
	
	if (f2.store) {
		putit(210,100,f2.fn);
		i = f2.len;
	}
	else
		i = (int32)0;
		
	sprintf(space,"%ld bytes",i);
	putit(210,130,space);

}

/************************************************************************/
#include "XDversion.h"

/************************************************************************/
/*  openingdialog
*  Show and remove the openingdialog.
*/
DialogPtr	openDialog;
openingdialog(mode)
	int mode;
{
	pascal void VersionNumber();

	if (mode) {
		openDialog = GetNewDialog(openDLOG, nil, (WindowPtr) -1);
		UItemAssign( openDialog, 3, VersionNumber);
		DrawDialog(openDialog);
	}
	else
		DisposDialog(openDialog);

	return;
}

/**********************************************************************/
/*  standard about dialog processing
*/

showAboutMeDialog()
{
	DialogPtr	theDialog;
	short		itemHit;
	pascal void VersionNumber();

	theDialog = GetNewDialog(aboutMeDLOG, nil, (WindowPtr) -1);
	
	UItemAssign( theDialog, 6, VersionNumber);
	do {
		ModalDialog(nil, &itemHit);
	} while (itemHit < 1 || itemHit > 2);

	DisposDialog(theDialog);

	return;
}

/**********************************************************************/
/*  Changefn.  Change the filename which is to be used for output.
*/

changefn()
{
	SFReply reply;
	 Point wh;

	wh.h = wh.v = 50;
	sfputfile(&wh, "Name for xdiff output file", fileout, nil, &reply);

	if (reply.good) {
		p2cstr(reply.fName);
		myvol = reply.vRefNum;							/* set to this volume (dir) */
		strcpy(fileout,reply.fName);
	}
		
	return;
}


/***************************************************************************/
/*  VersionNumber - Gaige Paulsen
*   Prints a version number string into a dialog box.  Requires an item number
*   to be present to define where to center the string.
*/
pascal void VersionNumber( dlog, itm)
DialogPtr dlog;
short itm;
{
	Rect ibox;
	char **ihndl;
	short typ;
	int item=itm;

	GetDItem( dlog, item, &typ, &ihndl, &ibox);
	TextFont( 3);
	TextSize( 9);
	MoveTo( ibox.left+((ibox.right-ibox.left)/2)-(stringwidth( VERSION)/2), ibox.bottom-2);
	drawstring( VERSION );
}

UItemAssign( dlog, item, proc)
DialogPtr dlog;
int item;
int (*proc)();
{
	Rect ibox;
	Handle ihndl;
	short typ;

	GetDItem( dlog, item, &typ, &ihndl, &ibox);
	SetDItem( dlog, item,  typ,  (Handle) proc, &ibox);

}


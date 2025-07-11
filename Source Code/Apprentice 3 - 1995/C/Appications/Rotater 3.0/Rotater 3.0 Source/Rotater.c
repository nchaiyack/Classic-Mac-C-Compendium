/*************************************************************
*
*   Rotater 3.0 Source Code
*
*   This program reads a set of 3D points from a file and
*   then displays them in a window where they can be rotated
*   around using the track-ball method in real time.
*
*   Feel free to use chunks of this code.
*   I retain the copyright for the whole.
*
*   The project files are for CW5.
*
*   I just do this for fun so the code here may not represent
*   the best way to do things. It just works for me :-)
*
*
*   Craig Kloeden
*   craig@raru.adelaide.edu.au
*   27 May 1995
*
*************************************************************/

#include "InfWin.h"
#include <QDOffscreen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <EPPC.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <console.stubs.c>
#include <string.h>

#define	DEGTORAD  0.01745329252

void	main (void);
void	init (void);
void	HandleEvent (void);
void	HandleMouseDown (EventRecord *theEvent);
void	HandleMenu (long mSelect, long mods);
void	finishup (void);
void	mousepress (Point p);
void	giveicon (void);
void	getuserfile (void);
void	readfile (FSSpec theFSS);
void	startball (void);
void	goball (void);
void	installAEhandlers (void);
void	DoHighLevelEvent (EventRecord *theEvent);
pascal	OSErr	openAE (AppleEvent *theEvent, AppleEvent *reply, long refcon);
pascal	OSErr	quitAE (AppleEvent *theEvent, AppleEvent *reply, long refcon);
pascal	OSErr	printAE (AppleEvent *theEvent, AppleEvent *reply, long refcon);
pascal	OSErr	startAE (AppleEvent *theEvent, AppleEvent *reply, long refcon);
void	info (void);
void	died (char desc[255], int code, int ref);
void	keyrotate (char ch, Boolean rep);
void	rotate (void);
void	rotatestereo (void);
void	rotatemono (void);
void	adjustsize (void);
void	plotline (long x, long x1, long y, long y1, long bytecol);
void	plot_hiddenline (long x, long x1, long y, long y1, long bytecol, long ughx, long ugh1x);
void	doclose(void);
void	resetimage (void);
void	copyimage (void);
void	vectorcopy (void);
void	savepict (Boolean vector);
void	readprefs (void);
void	writeprefs (void);
void	rotatestereoRB (void);
void	plotlineRB (Byte col, Byte mix, Byte bkgd, long XX, long XX1, long YY, long YY1);
void	updatemenus (void);
void	cooldot (long XX, long YY, long col, double dep, long ugh);
void	centerimage (void);
void	maxwindowsize (void);
void	setdpi (void);
void	showangles (void);

#ifdef powerc
void	bzero(void *s, int n); // courtesy of tim@apple.com (Tim Olson)
#else
#define bzero(ptr,size) memset(ptr,0,size)
#endif

long MAXwidth;
long MAXheight;
Byte *plane;
struct	Aray { double a[3]; };
struct	Aray	*aaa;
struct	pars
	{
	long WINleft, WINtop, WINright, WINbottom, PERSPEC;
	long DEPTHcue, COLOR, WBACK, CHIR, STEREO, PARALLEL;
	long STangle, STsep;
	long dpi, KIND, GRAB, BIGDOTS;
	long LINE, HIDDEN;
	long INFO, INFOleft, INFOtop;
	long FUNKY;
	long MAXwidth, MAXheight;
	long ROTYPE, SHANGS;
	};
struct	pars PARS;

long	*cc;
long	MaxPoints = 0;
double	Xangle, Yangle, Zangle, zoom = 1;
long	N;
Boolean	done;
Rect	frameRect, frameRectDPI, dragRect, maxrect;
Rect	GRect, picrec, inforec;
CWindowPtr	thewindow;
WindowPtr	infowindow;
GWorldPtr	drawWorldP;
PixMapHandle	thePixMapH, windowPixMapH;
PixMapPtr	drawPixMapP, windowPixMapP;
MenuHandle	applemenu, filemenu, editmenu, viewmenu, stereomenu, actionmenu,
			infomenu, anglemenu, persmenu, dotmenu, linemenu;
GWorldPtr	saveWorld;
GDHandle	saveDevice;
OSErr	err;
RGBColor	black = {0,0,0}, white = {65535, 65535, 65535};
RGBColor	red = {65535, 0, 0}, green = {0, 65535, 0}, blue = {0, 0, 65535};
RGBColor	yellow = {65535, 65535, 0}, purple = {65535, 0, 65535};
RGBColor	cyan = {0, 65535, 65535};
RGBColor	acol[8], tcol;
RGBColor	vcol[8][12];
Byte	*base;
Byte	Bred [8][11][16][16], bcol[8][12];
Byte	STred, STblue, STgreen, STredblue, STredgreen, STwhite, STblack;
Byte	STWred, STWgreen, STWblue, STWredblue, STWredgreen;
long	rb;
long	Nspace;
long	Wwidth, Wheight, Wmiddle, W2middle;
long	ddd;
long	comW, comH;
long	comn = 0;
unsigned long	cn, ct;
char	errdes[50];
char	comments[2001];
double	dep;
PicHandle	redpic;
PicHandle	thepic;
CursHandle	curshan[8];
FSSpec	myFSS;
Boolean	hasicon = TRUE, gotdots;
Str255	infoTitle = "\pInfo";
double	aM, bM, cM, dM, eM, fM, gM, hM, iM;

//************************************************************************

void main (void)
{
	init ();
	
	done = false;
	
	while (!done)
		HandleEvent();
	
	finishup ();
}

//************************************************************************

void init (void)
{
	EventRecord tempEvent;
	long i, j, k, kk;
	long tt;
	Size siz;
	long gesres;
	GDHandle devhan;
	Boolean dep8;
	
	/* Initialize Managers */
	InitGraf (&qd.thePort);
	InitFonts ();
	FlushEvents(everyEvent - osMask - diskMask, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	
	EventAvail(everyEvent, &tempEvent);
	EventAvail(everyEvent, &tempEvent);
	EventAvail(everyEvent, &tempEvent);
	
	/* set up menus */
	applemenu = GetMenu(128);
	if (applemenu == NULL) died ("Can't find Apple Menu Resource", 0, 1);
	AddResMenu(applemenu, 'DRVR');
	filemenu = GetMenu(129);
	editmenu = GetMenu(130);
	viewmenu = GetMenu(131);
	stereomenu = GetMenu(132);
	actionmenu = GetMenu(133);
	infomenu = GetMenu(134);
	anglemenu = GetMenu(200);
	persmenu = GetMenu(201);
	dotmenu = GetMenu(202);
	linemenu = GetMenu(203);
	InsertMenu(applemenu, 0);
	InsertMenu(filemenu, 0);
	InsertMenu(editmenu, 0);
	InsertMenu(viewmenu, 0);
	InsertMenu(stereomenu, 0);
	InsertMenu(actionmenu, 0);
	InsertMenu(infomenu, 0);
	// sub menus
	InsertMenu(anglemenu, -1);
	InsertMenu(persmenu, -1);
	InsertMenu(dotmenu, -1);
	InsertMenu(linemenu, -1);
	
	DrawMenuBar();
	
	dragRect = qd.screenBits.bounds;
	
#if __MC68881__ // building for 881

	// Check for FPU
	Gestalt(gestaltFPUType, &gesres);
	if (gesres == gestaltNoFPU)
		died ("Cannot find FPU", gesres, 2);

#endif
	
	// Check for System 7
	Gestalt(gestaltSystemVersion, &gesres);
	if (gesres < (0x700 & 0x0000FFFF))
		died ("System 7 or greater is REQUIRED", gesres, 3);

	// Check for 32 bit quickdraw
	Gestalt(gestaltQuickdrawVersion, &gesres);
	if (gesres < gestalt32BitQD)
		died ("No 32 bit quickdraw found", gesres, 4);
	
	// check for any 8 bit deep screen
	devhan = GetDeviceList();
	dep8 = FALSE;
	while (devhan)
		{
		if ((**((**devhan).gdPMap)).pixelSize == 8) dep8 = TRUE;
		devhan = GetNextDevice(devhan);
		}
	if (!dep8) // no 8 bit screen around
		{
		Alert(132,NULL); // tell user 8 bits is good to use
		}
	
	/* no points yet */
	N = 0;
  	
	/* set up the main window */
	thewindow = (CWindowPtr) GetNewCWindow (400, nil, (WindowPtr)-1L);
	if (thewindow == NULL) died ("Can't get Window Resource", 0, 5);
	SetPort ((WindowPtr)thewindow);
	GetGWorld(&saveWorld, &saveDevice);
    windowPixMapH = GetGWorldPixMap(saveWorld);
	
	// set up floating info window
	SetRect (&inforec, 50, 50, 295, 130);
	infowindow = NewWindow(nil, &inforec, infoTitle, FALSE, kInfinityProc,
		(WindowPtr)-1L, TRUE, 128);
  	
	// read preference file
	readprefs();
	
	// check if option key is down
	GetOSEvent(everyEvent, &tempEvent);
  if ((tempEvent.modifiers & optionKey) != 0)
  	maxwindowsize(); // set the max window size
  
	MAXwidth = PARS.MAXwidth;
	MAXheight = PARS.MAXheight;
	
	// place windows on screen
	if (PARS.WINright-PARS.WINleft >= MAXwidth)
		PARS.WINright = PARS.WINleft + MAXwidth - 1;
	if (PARS.WINbottom-PARS.WINtop >= MAXheight)
		PARS.WINbottom = PARS.WINtop + MAXheight - 1;
	MoveWindow ((WindowPtr)infowindow, PARS.INFOleft, PARS.INFOtop, TRUE);
	MoveWindow ((WindowPtr)thewindow, PARS.WINleft, PARS.WINtop, TRUE);
	SizeWindow ((WindowPtr)thewindow, PARS.WINright-PARS.WINleft,
		PARS.WINbottom-PARS.WINtop, TRUE);
	adjustsize();
	
	// map of screen pixels for hidden line stuff
	plane = (Byte *) NewPtr(MAXwidth*MAXheight);
	if (!plane) died ("Could not assign storage", 0, 6);
	SetRect (&maxrect, 99, 99 , MAXwidth, MAXheight);
	SetRect (&GRect, 0, 0 , MAXwidth, MAXheight);
	
	// set up offscreen GWorld
	err = NewGWorld(&drawWorldP, 8, &GRect, nil, nil, keepLocal);
	if (err) died ("Could not Create GWorld", err, 7);
	thePixMapH = GetGWorldPixMap(drawWorldP);
	HLockHi((Handle)thePixMapH);
	LockPixels(thePixMapH);
	drawPixMapP = *thePixMapH;
	
	//	base = (Byte *) GetPixBaseAddr(thePixMapH);
	base = (Byte*) (*thePixMapH)->baseAddr; // for 7.0
	rb = (*thePixMapH)->rowBytes & 0x1fff; // high 3 bits are flags
	
	SetGWorld(drawWorldP, nil);
	TextFont (geneva);
	TextSize (9);

	// read in, draw and load ball pixels into array
	SetRect (&picrec, 0, 0, 88, 56);
	redpic = GetPicture (129);
	DrawPicture (redpic, &picrec);
	ReleaseResource ((Handle)redpic);
	for (kk=1;kk<=7;kk++)
		for (i=0;i<=10;i++)
			for (j=0;j<=7;j++)
				for (k=0;k<=7;k++)
					Bred[kk][i][j][k] = base[rb*(k+kk*8-8) + j + i*8];
	
	// set up the colors
	StdRect((PARS.WBACK) ? erase : paint,&frameRect);
	acol[1] = red;
	acol[2] = green;
	acol[3] = blue;
	acol[4] = yellow;
	acol[5] = purple;
	acol[6] = cyan;
	acol[7] = white;
 	for (i=1;i<8;i++)
 		{
 		vcol[i][10] = white;
 		vcol[i][11] = black;
 		bcol[i][10] = Color2Index(&white);
 		bcol[i][11] = Color2Index(&black);
 		bcol[i][9] = Color2Index(&acol[i]);
 		vcol[i][9] = acol[i];
 		for (j=8;j>=0;j--)
 			{
 			tt = acol[i].red;
 			tt = tt * ((65535-20000)*j/9 + 20000) / 65535;
 			tcol.red = tt;
 			tt = acol[i].green;
 			tt = tt * ((65535-20000)*j/9 + 20000) / 65535;
 			tcol.green = tt;
 			tt = acol[i].blue;
 			tt = tt * ((65535-20000)*j/9 + 20000) / 65535;
 			tcol.blue = tt;
 			vcol[i][j] = tcol;
 			bcol[i][j] = Color2Index(&tcol);
 			}
 		}
 	STblack = bcol[1][11];
 	STwhite = bcol[1][10];
 	STred = bcol[1][3];
 	STgreen = bcol[2][1];
 	STblue = bcol[3][9];
 	STredblue = bcol[5][9];
 	STredgreen = bcol[4][3];
 	STWred = Color2Index(&red);
 	STWgreen = Color2Index(&green);
 	STWblue = Color2Index(&blue);
 	STWredblue = Color2Index(&black);
 	STWredgreen = Color2Index(&black);
 	
	SetGWorld(saveWorld, saveDevice);
	
	/* get beachball cursors */
	for (i=0;i<8;i++)
		curshan[i] = GetCursor (256+i);
	
  	/* install apple event handlers */
  	installAEhandlers ();
  	
  	/* assign array memory */
  	siz = CompactMem (20000000);
  	/* save 200000 bytes for pictures and dialogs */
  	if (siz < 200000) died ("Not Enough Memory to Run", siz, 8);
  	Nspace = sizeof(struct Aray) + sizeof(long);
  	MaxPoints = (siz-200000) / Nspace;
	
	// the points array space
	aaa = (struct Aray *) NewPtr(MaxPoints*sizeof(struct Aray));
	if (!aaa) died ("Could not assign storage", 0, 9);
	
	// the colors array space
	cc = (long*) NewPtr(MaxPoints*sizeof(long));
	if (!cc) died ("Could not assign storage", 0, 10);
	
	// show info window if needed
	if (PARS.INFO)
		{
		ShowWindow (infowindow);
		info();
		}
}

//************************************************************************

void HandleEvent(void)
{
	EventRecord	theEvent;
	char ch;
	
	HiliteMenu(0);
	
	if (WaitNextEvent (everyEvent, &theEvent, 50, NULL))
	switch (theEvent.what)
		{
		case mouseDown:
			HandleMouseDown(&theEvent);
		break;
			
		case keyDown: 
		    if ((theEvent.modifiers & cmdKey) != 0)
					{
					HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)), theEvent.modifiers);
					if ((char) (theEvent.message & charCodeMask) == '=')
						keyrotate('+', FALSE);
					}
			else keyrotate(theEvent.message & charCodeMask, FALSE);
		break;
		
		case autoKey:
			ch = theEvent.message & charCodeMask;
	    if ((theEvent.modifiers & cmdKey) != 0)
	  		{
	  		// repeat these menu keys
	  		if (ch == '-' || ch == '+' || ch == '7' || ch == '9' ||
	  		    ch == '4' || ch == '6')
	  		    keyrotate(ch, TRUE);
	  		// treat '=' as a '+'
	  		if (ch == '=') keyrotate('+', TRUE);
				}
			else
				{
				if (ch != 'H' && ch != 'h' && ch != 'I' && ch != 'i')
					keyrotate(ch, TRUE);
				}
		break;
			
		case updateEvt:
			if ( (WindowPtr)(theEvent.message) == infowindow)
				{
				BeginUpdate((WindowPtr)infowindow);
				if (PARS.INFO) info();
				EndUpdate((WindowPtr)infowindow);
				}
			else
				{
				BeginUpdate((WindowPtr)thewindow);
				/* copy GWorld to screen window */
				SetGWorld(saveWorld, saveDevice);
				CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP,
					&frameRect, &frameRect, srcCopy, nil);
				EndUpdate((WindowPtr)thewindow);
				}
		  break;
		    
		case activateEvt:
			InvalRect(&thewindow->portRect);
			InvalRect(&infowindow->portRect);
		break;
			
		case osEvt:
			if (theEvent.message & 1) /* Resume Event */
				{
				InitCursor();
				HiliteWindow ((WindowPtr)thewindow, TRUE);
				}
			else /* Suspend Event */
				{
				InitCursor();
				HiliteWindow ((WindowPtr)thewindow, FALSE);
				}
		break;
		    
		case kHighLevelEvent:
			DoHighLevelEvent (&theEvent);
		break;
		}
	else /* null event */
		{
		}
}

//************************************************************************

void HandleMouseDown (EventRecord *theEvent)
{
	WindowPtr	awindow;
	int			windowCode = FindWindow (theEvent->where, &awindow);
	long		growsize;
	
	switch (windowCode)
		{
		case inSysWindow: 
			SystemClick (theEvent, awindow);
		break;
		
		case inMenuBar:
			HandleMenu(MenuSelect(theEvent->where), theEvent->modifiers);
		break;
		
		case inDrag:
			if (awindow == (WindowPtr)thewindow)
				{
				DragWindow((WindowPtr)thewindow, theEvent->where, &dragRect);
				if (PARS.INFO)
					{
					BringToFront (infowindow);
					HiliteWindow (infowindow, TRUE);
					}
				}
			else if (awindow == (WindowPtr)infowindow)
				{
				DragWindow((WindowPtr)infowindow, theEvent->where, &dragRect);
				info();
				}
			adjustsize();
			rotate();
		break;
		
		case inZoomIn:
		case inZoomOut:
			if (TrackBox ((WindowPtr)thewindow, theEvent->where, windowCode))
				{
				// Max out the window
				HideWindow ((WindowPtr)thewindow);
				UnlockPixels(windowPixMapH);
				ZoomWindow ((WindowPtr)thewindow, windowCode, FALSE);
				Wwidth = (thewindow->portRect).right - (thewindow->portRect).left;
				Wheight = (thewindow->portRect).bottom - (thewindow->portRect).top;
				if (Wwidth > MAXwidth || Wheight > MAXheight)
					SizeWindow ((WindowPtr)thewindow, MAXwidth-1, MAXheight-1, TRUE);
				if (theEvent->modifiers & optionKey) // check for option key
					{
					Wwidth = (thewindow->portRect).right - (thewindow->portRect).left;
					Wheight = (thewindow->portRect).bottom - (thewindow->portRect).top;
					if (PARS.STEREO && PARS.KIND<3)
						{
						if (Wwidth < Wheight*2) Wheight = Wwidth/2;
						else Wwidth = Wheight*2;
						}
					else
						{
						if(Wwidth<Wheight) Wheight=Wwidth;
						else Wwidth=Wheight;
						}
					SizeWindow ((WindowPtr)thewindow, Wwidth, Wheight, TRUE);
					}
				ShowWindow ((WindowPtr)thewindow);
				adjustsize();
				rotate();
				}
		break;
		
		case inGrow:
			DrawGrowIcon((WindowPtr)thewindow);
			growsize = GrowWindow ((WindowPtr)thewindow, theEvent->where, &maxrect);
			if (growsize)
				{
				// resize the window
				UnlockPixels(windowPixMapH);
				if (LoWord(growsize) <= MAXwidth && HiWord(growsize) <= MAXheight)
					SizeWindow ((WindowPtr)thewindow, LoWord(growsize),
						HiWord(growsize), TRUE);
				adjustsize();
				}
			rotate();
		break;
		
		case inContent:
			if (awindow == (WindowPtr)thewindow)
				{
				GlobalToLocal(&theEvent->where);
				mousepress (theEvent->where);
				}
			else if (awindow == (WindowPtr)infowindow)
				{
				SelectWindow((WindowPtr)infowindow);
				}
		break;
	  	
		case inGoAway:
	  	if (awindow == (WindowPtr)thewindow && 
	  		TrackGoAway((WindowPtr)thewindow, theEvent->where))
		  		doclose();
		if ((awindow == infowindow) && TrackGoAway(infowindow, theEvent->where))
			{
			HideWindow (infowindow);
			PARS.INFO = FALSE;
			updatemenus();
			}
		break;
		}
}

//************************************************************************

void HandleMenu (long mSelect, long mods)
{
int			menuID = HiWord(mSelect);
int			menuItem = LoWord(mSelect);
Str255		name;
GrafPtr		savePort;
long		speedi, speed1, speed2, wild, slug;
Boolean		upity;
char		a[50], b[50], c[50];

switch (menuID)
	{
	case 128: // apple menu
		if (menuItem == 1) // About
			{
			Alert(128,NULL);
			break;
			}
		else // system apple menu item
			{
			GetPort(&savePort);
			GetItem(applemenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
			}
	break;
	
	case 129: // file menu
		switch (menuItem)
			{
			case 1: // open
				getuserfile();
				break;
			case 2: // close
				doclose();
				break;
			case 3: // Give Icon
				giveicon();
				break;
			case 4: // save bitmap pict
				savepict(FALSE); // bitmap copy
				break;
			case 5: // save vector pict
				if (mods & optionKey) setdpi();
				else savepict(TRUE); // vector copy
				break;
			case 7: // quit
				done = true;
				break;
			}
	break;
	
	case 130: // edit menu
		switch (menuItem)
			{
			case 2: // copy
				copyimage();
				break;
			case 5: // save preferences
				writeprefs();
				break;
			}
	break;
	
	case 131: /* view menu */
		switch (menuItem)
			{
			case 4: /* depth cueing */
				if (PARS.DEPTHcue) PARS.DEPTHcue = FALSE; else PARS.DEPTHcue = TRUE;
				updatemenus();
				rotate ();
				break;
			case 5: /* hide lines */
				if (PARS.HIDDEN) PARS.HIDDEN = FALSE; else PARS.HIDDEN = TRUE;
				updatemenus();
				rotate ();
				break;
			case 6: /* color */
				if (PARS.COLOR) PARS.COLOR = FALSE; else PARS.COLOR = TRUE;
				updatemenus();
				rotate ();
				break;
			case 7: /* black background */
				if (PARS.WBACK) PARS.WBACK = FALSE; else PARS.WBACK = TRUE;
				updatemenus();
				rotate ();
				break;
			case 8: /* funky */
				if (PARS.FUNKY) PARS.FUNKY = FALSE; else PARS.FUNKY = TRUE;
				updatemenus();
				rotate ();
				break;
			case 10: /* Right handed */
				PARS.CHIR = TRUE;
				updatemenus();
				rotate ();
				break;
			case 11: /* Left handed */
				PARS.CHIR = FALSE;
				updatemenus();
				rotate ();
				break;
			case 13: /* Mono */
				PARS.STEREO = FALSE;
				updatemenus();
				rotate ();
				break;
			case 14: /* Stereo */
				PARS.STEREO = TRUE;
				updatemenus();
				rotate ();
				break;
			}
	break;

	case 132: // stereo menu
		switch (menuItem)
			{
			case 1: /* stereo crosseyed */
				PARS.KIND = 1;
				PARS.PARALLEL = FALSE;
				updatemenus();
				rotate ();
				break;
			case 2: /* stereo parallel */
				PARS.KIND = 2;
				PARS.PARALLEL = TRUE;
				updatemenus();
				rotate ();
				break;
			case 3: /* red blue */
				PARS.KIND = 3;
				updatemenus();
				rotate ();
				break;
			case 4: /* red green */
				PARS.KIND = 4;
				updatemenus();
				rotate ();
				break;
			case 7: /* increase angle */
				PARS.STangle++;
				if (PARS.STangle > 20) { SysBeep(1); PARS.STangle = 20; }
				updatemenus();
				rotate ();
				break;
			case 8: /* decrease angle */
				PARS.STangle--;
				if (PARS.STangle < 0) { SysBeep(1); PARS.STangle = 0; }
				updatemenus();
				rotate ();
				break;
			case 10: /* increase seperation */
				PARS.STsep++;
				if (PARS.STsep > 40) { SysBeep(1); PARS.STsep = 40; }
				updatemenus();
				rotate ();
				break;
			case 11: /* decrease seperation */
				PARS.STsep--;
				if (PARS.STsep < -40) { SysBeep(1); PARS.STsep = -40; }
				updatemenus();
				rotate ();
				break;
			}
	break;

	case 133: // action menu
		switch (menuItem)
			{
			case 1: // zoom in
				zoom *= 1.01;
				rotate ();
			break;
			case 2: // zoom out
				zoom /= 1.01;
				rotate ();
			break;
			case 4: // switch to Trackball rotation
				PARS.ROTYPE = 1;
				updatemenus();
				rotate();
				break;
			case 5: // switch to Z-X rotation
				PARS.ROTYPE = 2;
				updatemenus();
				rotate();
				break;
			case 7: // reset angles
				aM = 1; bM = 0; cM = 0;
				dM = 0; eM = 1; fM = 0;
				gM = 0; hM = 0; iM = 1;
				Xangle = 0;
				Yangle = 0;
				Zangle = 0;
				rotate();
				break;
			case 8: // grab and spin
				if (PARS.GRAB) PARS.GRAB = FALSE; else PARS.GRAB = TRUE;
				updatemenus();
				break;
			case 9: /* rotate Z-axis */
				while (! Button())
					{
					//SystemTask();
					Zangle += 2;
					rotate ();
					}
				FlushEvents(keyDownMask + keyUpMask + autoKeyMask, 0);
				break;
			case 10: // wobble
				wild = 4;
				upity = TRUE;
				slug = TickCount() + 4;
				while (! Button())
					{
					while (TickCount() < slug+4) {}
					slug = TickCount();
					if (upity && wild == 4) upity = FALSE;
					else if (!upity && wild == -4) upity = TRUE;
					else if (upity) wild++;
					else wild--;
					Zangle += wild;
					rotate ();
					}
				// ignore any key presses done while wobbling
				FlushEvents(keyDownMask + keyUpMask + autoKeyMask, 0);
				break;
			case 12: // center image
				centerimage();
				rotate();
				break;
			case 14: // full screen
				UnlockPixels(windowPixMapH);
				MoveWindow ((WindowPtr)thewindow, 0, 20, TRUE);
				SizeWindow ((WindowPtr)thewindow, MAXwidth-1, MAXheight-1, TRUE);
				BringToFront (infowindow); //!!
				HiliteWindow (infowindow, TRUE); //!!
				adjustsize();
				rotate();
				break;
			case 15: /* small window*/
				UnlockPixels(windowPixMapH);
				MoveWindow ((WindowPtr)thewindow, 10, 50, TRUE);
				if (MAXwidth < 199 || MAXheight < 199)
					SizeWindow ((WindowPtr)thewindow, MAXwidth-1, MAXheight-1, TRUE);
				else
					SizeWindow ((WindowPtr)thewindow, 199, 199 , TRUE);
				BringToFront (infowindow); //!!
				HiliteWindow (infowindow, TRUE); //!!
				adjustsize();
				rotate();
				break;
			case 16: /* master reset */
				resetimage();
				break;
			}
	break;
	
	case 134: // info menu
		switch (menuItem)
			{
			case 1: /* Show Info */
				if (PARS.INFO)
					{
					PARS.INFO = FALSE;
					HideWindow (infowindow);
					}
				else
					{
					PARS.INFO = TRUE;
					ShowWindow (infowindow);
					BringToFront (infowindow);
					HiliteWindow (infowindow, TRUE);
					info();
					}
				updatemenus();
				break;
				
			case 2: /* Show Angles */
				if (PARS.SHANGS) PARS.SHANGS = FALSE;
				else PARS.SHANGS = TRUE;
				updatemenus();
				rotate();
				break;
				
			case 4: // run speed test
				speed1 = TickCount();
				speedi=0;
				while (speedi < 180 && !Button())
					{
					Zangle += 2;
					rotate ();
					speedi++;
					}
				speed2 = TickCount() - speed1;
				if (speed2 == 0) speed2 = 1;
				NumToString (speedi, (StringPtr)a);
				NumToString (speed2, (StringPtr)b);
				NumToString (60*speedi/speed2, (StringPtr)c);
				ParamText ((StringPtr)a, (StringPtr)b, (StringPtr)c, NULL);
				Alert(138,NULL);
				FlushEvents(everyEvent - osMask - diskMask, 0);
				break;
			}
	break;
	
	case 200: // angle menu
		PARS.STangle = menuItem-1;
		updatemenus();
		rotate();
	break;
	
	case 201: // perspective menu
		PARS.PERSPEC = menuItem-1;
		updatemenus();
		rotate();
	break;
	
	case 202: // dot size menu
		PARS.BIGDOTS = menuItem;
		updatemenus();
		rotate();
	break;
	
	case 203: // line width menu
		PARS.LINE = menuItem;
		updatemenus();
		rotate();
	break;
	}
}

//************************************************************************

void finishup (void) /* get outa here */
{
	ExitToShell();
}

//************************************************************************

void mousepress (Point p)
{
	Point p1;
	int x, y, x1, y1, x2, y2, x3, y3;
	double xx, yy;
	unsigned long tim, tim1, tim2;
	
	HideCursor();
	
	x = 0;
	y = 0;
	x1 = 0; x2 = 0; x3 = 0;
	y1 = 0; y2 = 0; y3 = 0;
	tim = 0;
	tim1 = 0;
	
	while (StillDown()) /* loop while button held down */
		{
		GetMouse (&p1);
		x = p.h - p1.h;
		y = p.v - p1.v;
		p.h = p1.h;
		p.v = p1.v;
		
		if (x || y) /* if mouse moved since last time around */
			{
			if (PARS.ROTYPE == 1)
				{
				Zangle = -x;
				Xangle = -y;
				}
			else
				{
				Zangle -= x;
				Xangle -= y;
				}
			tim1 = TickCount();
			rotate ();
			x3 = x2;
			y3 = y2;
			x2 = x1;
			y2 = y1;
			x1 = x;
			y1 = y;
			}
		}
	
	ShowCursor();
	
	tim = TickCount();
	tim = tim - tim1;
	
	// continue rotating the image if moved and
	// last time interval is less than 0.25 seconds (15 ticks)
	if (((PARS.ROTYPE == 1 && PARS.GRAB && (x1 || y1)) ||
	     (PARS.ROTYPE == 2 && PARS.GRAB && x1)) && (tim < 15))
		{
		// average last 3 rotations
		xx = (x1 + x2 + x3) / 3.0;
		yy = (y1 + y2 + y3) / 3.0;
		if (xx > 10) xx = 10;
		if (xx < -10) xx = -10;
		if (yy > 10) yy = 10;
		if (yy < -10) yy = -10;
		
		#ifdef powerc
		if (PARS.ROTYPE == 2) xx = ceil(xx);
		#else
		if (PARS.ROTYPE == 2) xx = ceild(xx);
		#endif
		
		while (!Button())
			{
			// use last time interval for pacing rotations
			tim1 = TickCount();
			if (PARS.ROTYPE == 1)
				{
				Zangle = -xx;
				Xangle = -yy;
				}
			else
				{
				Zangle -= xx;
				}
			rotate ();
			tim2 = TickCount();
			while ((tim2 - tim1) < tim) { tim2 = TickCount(); }
			}
		}
	FlushEvents(keyDownMask + keyUpMask + autoKeyMask, 0);
}

//************************************************************************

void giveicon (void)
{
	FInfo fin;
	
	err = FSpGetFInfo (&myFSS, &fin);
	if (err) died ("Could not Get File Info", err, 201);
	fin.fdCreator = 'KrOt'; // set creator to Rotaters type
	if (FSpSetFInfo (&myFSS, &fin))
		{
		/* couldnt change icon */
		ParamText ((StringPtr)myFSS.name, NULL, NULL, NULL);
		Alert(134,NULL);
		}
	hasicon = true;
	updatemenus();
}

//************************************************************************

void getuserfile (void)
{
	SFTypeList types;
	StandardFileReply mySFR;
	
	/* get a file record */
	types[0] = 'TEXT';
	StandardGetFile (NULL, 1, types, &mySFR);
	
	if (mySFR.sfGood)
		readfile (mySFR.sfFile);
}

//************************************************************************

void readfile (FSSpec theFSS)
{
	FILE *points;
	long    i, j, linelen, linenum;
	int		comT;
	long size = MAXwidth*MAXheight-1;
	double max, maybe;
	Byte *cp;
	FInfo fin;
	GrafPtr oldPort;
	Boolean filedone, recom;
	char aline[1001];
	char a[50];

	myFSS = theFSS;
	
	StdRect((PARS.WBACK) ? erase : paint,&frameRect);
	
	GetPort (&oldPort);
	SetPort (infowindow);
	EraseRect (&infowindow->portRect);
	SetPort (oldPort);
	
	// can only read TEXT files
	err = FSpGetFInfo (&myFSS, &fin);
	if (err) died ("Could not Get File Info", err, 301);
 	if (fin.fdType != 'TEXT') died ("Cannot Read this File Type", err, 302);
	
	SetWTitle ((WindowPtr)thewindow, theFSS.name);
	
	startball(); // start the spinning ball
	
	// open the file
	PtoCstr((StringPtr) theFSS.name);
	err = HSetVol (NULL, theFSS.vRefNum, theFSS.parID);
	if (err) died ("Could not Set Volume", err, 303);
	points = fopen ((char *)theFSS.name, "rb");
	if (points == NULL) died ("Can't Open File", 0, 304);
	
	// read in first part of the file
	linenum = 0;
	filedone = FALSE;
	recom = TRUE;
	cp=plane;
	i=fread(cp,1,size,points); // read file (i = number of characters read)
	cp[i]=0;
	if (i == 0) filedone = TRUE; // empty file
	
	N = 0;
	comn = 0;
	comW = 0;
	comH = 0;
	
	while (!filedone)
		{
		goball();
		// get a line
		linenum++;
		linelen = 0;
		
		scanning: while (*cp && *cp != 13)
			{
			if (linelen < 1000)
				{
				aline[linelen] = *cp;
				linelen++;
				}
			cp++;
			}
		
		if (*cp == 0 && !feof(points))
			{
			cp=plane;
			i=fread(cp,1,size,points); // read more of the file (i = number of characters read)
			cp[i]=0;
			goto scanning;
			}
		
		if (*cp == 0 && feof(points)) filedone = TRUE;
		
		if (*cp == 13) cp++;
		
		// got a line in "aline"
		aline[linelen] = 0;
		
		if (aline[0] == '#') // comment line
			{
			if (recom)
				{
				if (comn < 2000) comH++;
				comT = 0;
				for (j=1;j<linelen;j++)
					if (comn < 2000)
						{
						comments[comn] = aline[j];
						comn++;
						comT++;
						}
				if (comT > comW) comW = comT;
				if (*cp == '#' && comn < 2000)
					{
					comments[comn] = 13;
					comn++;
					}
				}
			}
		else if (aline[0]) // data line
			{
			recom = FALSE;
			if (N == MaxPoints) // too many commands
				{
				InitCursor();
				NumToString (MaxPoints, (StringPtr)errdes);
				ParamText ((StringPtr)errdes, NULL, NULL, NULL);
				Alert(130,NULL);
				N = 0;
				filedone = TRUE;
				}
			else // read command in
				{
				err = (sscanf(aline, "%lf %lf %lf %d", &aaa[N].a[0],
					&aaa[N].a[1], &aaa[N].a[2], &cc[N]));
				if (err == 4) // read line ok
					{
					if (cc[N] > 7) cc[N] = 7;
					if (cc[N] < -7) cc[N] = -7;
					N++;
					}
				else // error in line number "linenum"
					{
					InitCursor();
					NumToString (linenum, (StringPtr)a);
					ParamText ((StringPtr)a, NULL, NULL, NULL);
					Alert(131,NULL);
					N = 0;
					filedone = TRUE;
					}
				}
			}
		else recom = FALSE; // blank line - stop recording comments
		}
	
	// finished reading file
	fclose (points);
	
	/* get maximum distance of points from origin */
	max = 0;
	gotdots = FALSE;
	for (i=0; i<N; i++)
		{
		maybe = sqrt(aaa[i].a[0] * aaa[i].a[0] + aaa[i].a[1] * aaa[i].a[1] +
			aaa[i].a[2] * aaa[i].a[2]);
		if (maybe > max) max = maybe;
		if (cc[i] < 0) gotdots = TRUE;
		}
	
	/* scale the points */
	if (max != 0) // cant scale points at origin
		for (i=0; i<N; i++)
			for (j=0; j<3; j++)
				aaa[i].a[j] = (aaa[i].a[j] / max) * 0.99999;
	
	/* reset rotation angles */
	Xangle = 0;
	Yangle = 0;
	Zangle = 0;
	zoom = 1;
	
	/* check for icon */
	hasicon = true;
	if (N)
		{
		err = FSpGetFInfo (&myFSS, &fin);
		if (err) died ("Could not Get File Info", err, 305);
		if (fin.fdCreator != 'KrOt') hasicon = FALSE;
		updatemenus();
		}
	else doclose();
	
	/* draw the picture to GWorld and screen */
	if (N != 0)
		{
		ShowWindow ((WindowPtr)thewindow);
		aM = 1; bM = 0; cM = 0;
		dM = 0; eM = 1; fM = 0;
		gM = 0; hM = 0; iM = 1;
		}
	
	rotate ();
	InitCursor();
	
	if (PARS.INFO) info();
}

//************************************************************************

void startball (void)
{
	cn = 0;
	ct = TickCount();
	SetCursor (*(curshan[cn]));
}

//************************************************************************

void goball (void)
{
	if (TickCount() > (ct+1))
		{
		if (++cn >= 8) cn = 0;
		SetCursor (*(curshan[cn]));
		ct = TickCount();
		}
}

//************************************************************************

void installAEhandlers (void)
{
	/* install the required apple event handlers */
	AEEventHandlerUPP OPENae, QUITae, STARTae, PRINTae;
	
	// must use these for PPC proc pointers
	// also works for 68K if using Universal Headers
	OPENae = NewAEEventHandlerProc ((ProcPtr) &openAE);
	QUITae = NewAEEventHandlerProc ((ProcPtr) &quitAE);
	STARTae = NewAEEventHandlerProc ((ProcPtr) &startAE);
	PRINTae = NewAEEventHandlerProc ((ProcPtr) &printAE);
	
	err = AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,
		OPENae, 0, FALSE);
	if (err) died ("Error Installing OpenDoc Apple Event", err, 401);
	
	err = AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
		QUITae, 0, FALSE);
	if (err) died ("Error Installing QuitApp Apple Event", err, 402);
	
	err = AEInstallEventHandler (kCoreEventClass, kAEOpenApplication,
		STARTae, 0, FALSE);
	if (err) died ("Error Installing OpenApp Apple Event", err, 403);
	
	err = AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments,
		PRINTae, 0, FALSE);
	if (err) died ("Error Installing PrintDoc Apple Event", err, 404);
}

//************************************************************************

void DoHighLevelEvent (EventRecord *theEvent)
{
	/* process high level events */
	err = AEProcessAppleEvent (theEvent);
	if (err) died ("Error Processing Apple Event", err, 405);
}

//************************************************************************

pascal	OSErr	openAE (AppleEvent *theEvent, AppleEvent *reply, long refcon)
{
	AEDescList	docList;
	long		n;
	AEKeyword	keywd;
	DescType	rtype;
	Size		acsize;
	FSSpec		myFSS;
	
	/* open document apple event */
	err = AEGetParamDesc (theEvent, keyDirectObject, typeAEList, &docList);
	if (err) died ("Error Processing Apple Event", err, 406);
	
	/* count number of document to be opened */
	err = AECountItems (&docList, &n);
	if (err) died ("Error Processing Apple Event", err, 407);
	if (n < 1) died ("No File for OpenDoc Apple Event", 0, 408);
	
	/* get FSS record of the first document only */
	err = AEGetNthPtr (&docList, 1, typeFSS, &keywd, &rtype,
		&myFSS, sizeof(myFSS), &acsize);
	if (err) died ("Error Processing Apple Event", err, 409);
	
	/* get rid of storage */
	err = AEDisposeDesc (&docList);
	if (err) died ("Error Processing Apple Event", err, 410);
	
	/* open the passed file */
	readfile (myFSS);
	
	return noErr;
}

//************************************************************************

pascal OSErr quitAE(AppleEvent *theEvent, AppleEvent *reply, long refcon)
{
	// quit apple event - set the done flag
	// CANNOT just ExitToShell() from here!!!
	done = true;
	return noErr;
}

//************************************************************************

pascal	OSErr	printAE (AppleEvent *theEvent, AppleEvent *reply, long refcon)
{
	/* print apple event not handled here */
	return errAEEventNotHandled;
}

//************************************************************************

pascal	OSErr	startAE (AppleEvent *theEvent, AppleEvent *reply, long refcon)
{
	/* startup apple event - ask for an input file */
	getuserfile();
	return noErr;
}

//************************************************************************

void info (void)
{
	GrafPtr		oldPort;
	Str255		tempStr;
	long	Theight, Twidth, i, h;
	
	Theight = 80 + comH*12;
	if (comn) Theight = Theight + 20;
	Twidth = comW*5 + 7;
	if (Twidth < 245) Twidth = 245;
	
	SizeWindow(infowindow, Twidth, Theight, TRUE);
	
	GetPort (&oldPort);
	SetPort (infowindow);
	EraseRect (&infowindow->portRect);
	
	// Main Headers
	TextFont (geneva);
	TextFace (bold);
	TextSize (9);
	MoveTo (4, 13);
	DrawString ("\pFile Name:");	
	MoveTo (4, 30);
	DrawString ("\pMemory Information");	
	MoveTo(4, 95);
	DrawString("\pFile Comments");	
	
	// File Name
	TextFace(0);
	GetWTitle((WindowPtr)thewindow, tempStr);
	MoveTo(65, 13);
	if (N > 0) DrawString (tempStr);
	else DrawString ("\p<none>");
		
	// Memory info
	MoveTo (7, 45);
	DrawString ("\pNumber of drawing commands in file:");	
	NumToString (N, tempStr);
	MoveTo (200, 45);
	DrawString (tempStr);
	MoveTo (7, 60);
	DrawString ("\pNumber of drawing commands allowed:");
	NumToString (MaxPoints, tempStr);
	MoveTo (200, 60);
	DrawString (tempStr);
	MoveTo (7, 75);
	DrawString ("\pNumber of bytes/additional command:");
	NumToString (Nspace, tempStr);
	MoveTo (200, 75);
	DrawString (tempStr);
	
	h = 108;
	MoveTo (7, h);
	for (i=0;i<comn;i++)
		{
		if (comments[i] == 13)
			{
			h = h + 12;
			MoveTo(7, h);
			}
		else
			{
			DrawChar (comments[i]);
			}
		}
	
	SetPort (oldPort);
}

//************************************************************************

void died (char desc[255], int code, int ref)
{
	/* nasty error has occurred - report it and quit */
	char a[50], b[50];
	
	CtoPstr (desc);
	NumToString (code, (StringPtr)a);
	NumToString (ref, (StringPtr)b);
	ParamText ((StringPtr)desc, (StringPtr)a, (StringPtr)b, NULL);
	Alert(129,NULL);
	ExitToShell();
}

//************************************************************************

void keyrotate (char ch, Boolean rep) // rep = repeat key
{
	KeyMap theKeys, theKeys1;
	Boolean validkey, stopit, slowly;
	long counting = 0;
	unsigned long tim, tim1;
	
	if (ch == 'I' || ch == 'i')
		{
		if (PARS.INFO)
			{
			PARS.INFO = FALSE;
			HideWindow (infowindow);
			}
		else
			{
			PARS.INFO = TRUE;
			ShowWindow (infowindow);
			BringToFront (infowindow);
			HiliteWindow (infowindow, TRUE);
			info();
			}
		updatemenus();
		}
	
	if (N)
		{
		stopit = FALSE;
		slowly = FALSE;
		validkey = TRUE;
		GetKeys (theKeys);
		GetKeys (theKeys1);
		
		do
			{
			tim = TickCount();
			switch (ch)
				{
				case 'm':
					centerimage();
				break;
				
				case 'M':
					centerimage();
				break;
				
				case 'x':
					Xangle++;
				break;
				
				case 'y':
					Yangle++;
				break;
				
				case 'z':
					Zangle++;
				break;
				
				case 'X':
					Xangle--;
				break;
				
				case 'Y':
					Yangle--;
				break;
				
				case 'Z':
					Zangle--;
				break;
				
				case 31: // down arrow
					Xangle++;
				break;
				
				case 30: // up arrow
					Xangle--;
				break;
				
				case 29: // right arrow
					Zangle++;
				break;
				
				case 28: // left arrow
					Zangle--;
				break;
				
				case '=':
				case '+':
					{
					counting++;
					zoom *= 1 + counting/100.0;
					slowly = TRUE;
					}
				break;
				
				case '-':
					{
					counting++;
					zoom /= 1 + counting/100.0;
					slowly = TRUE;
					}
				break;
				
				case '7': // decrease angle
					if (PARS.STEREO)
						{
						PARS.STangle--;
						if (PARS.STangle < 0)
							{
							SysBeep(1);
							PARS.STangle = 0;
							}
						updatemenus();
						slowly = TRUE;
						}
				break;
				
				case '9': // increase angle
					if (PARS.STEREO)
						{
						PARS.STangle++;
						if (PARS.STangle > 20)
							{
							SysBeep(1);
							PARS.STangle = 20;
							}
						updatemenus();
						slowly = TRUE;
						}
				break;
				
				case '4': // decrease sep
					if (PARS.STEREO && PARS.KIND < 3)
						{
						PARS.STsep--;
						if (PARS.STsep < -40)
							{
							SysBeep(1);
							PARS.STsep = -40;
							}
						slowly = TRUE;
						}
				break;
				
				case '6': // increase sep
					if (PARS.STEREO && PARS.KIND < 3)
						{
						PARS.STsep++;
						if (PARS.STsep > 40)
							{
							SysBeep(1);
							PARS.STsep = 40;
							}
						slowly = TRUE;
						}
				break;
				
				case 'H':
				case 'h':
					if (PARS.SHANGS) PARS.SHANGS = FALSE;
					else PARS.SHANGS = TRUE;
					updatemenus();
					rotate();
				break;
				
				default:
					validkey = FALSE;
				}
			if (validkey) rotate ();
			GetKeys (theKeys);
			if (theKeys[0] != theKeys1[0] || theKeys[1] != theKeys1[1] ||
				theKeys[2] != theKeys1[2] || theKeys[3] != theKeys1[3] || !rep)
				stopit = TRUE;
			if (!stopit && slowly)
				do { tim1 = TickCount(); } while ((tim1-tim) < 4);
			}
		while (!stopit);
		}
}

//************************************************************************

void rotate (void) // rotate using Xangle, Yangle and Zangle and draw the result
{
	if (!PARS.FUNKY) // erase buffer
		{
		Byte *cbase; long i;
		for (i=0,cbase=base;i<Wheight;i++,cbase+=rb)
			if(PARS.WBACK) bzero(cbase, Wwidth);
			else memset(cbase,255, Wwidth);
		}
	
	if (PARS.STEREO)
		{
		if (PARS.KIND < 3) rotatestereo();
		else rotatestereoRB();
		}
	else rotatemono();
	
	if (PARS.SHANGS)
		{
		while (Xangle >= 360) Xangle -= 360;
		while (Yangle >= 360) Yangle -= 360;
		while (Zangle >= 360) Zangle -= 360;
		while (Xangle <= -360) Xangle += 360;
		while (Yangle <= -360) Yangle += 360;
		while (Zangle <= -360) Zangle += 360;
		showangles();
		}
	
	if (PARS.ROTYPE == 1)
		{
		Xangle = 0;
		Yangle = 0;
		Zangle = 0;
		}
	
	// show the result
	CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP,
		&frameRect, &frameRect, srcCopy, nil);
}

//************************************************************************

void rotatestereo ()
{
	double  X, Y, Z, X3, Y3, Z3, X3b, Y3b, Z3b;
	double  CZ, SZ, CY, SY, CX, SX, CZb, SZb;
	double  AM, BM, CM, DM, EM, FM, GM, HM, IM;
	double	AN, BN, CN, DN, EN, FN, GN, HN, IN;
	double  P, B, H, Bb;
	long    i, j, XX, YY, XX1=0, YY1=0, XXb, YYb, XX1b=0, YY1b=0;
	double  tempWidth, tempWidthb, tempHeight, tempZoom, tempPerspec;
	double  Ldep, Ldepb, dep1=0, dep2=0;
	long    LEc, Lddd, Ldddb, Lugh, Lugh1, Lughb, Lugh1b;
	long    currcolor, LWheight=Wheight, LWwidth=Wwidth, Lrb=rb,bigdots=PARS.BIGDOTS;
	Byte    bytecolor, bytecolorb;
	struct Aray *paaa;
	
	if ((PARS.KIND < 3 || !PARS.STEREO) && (PARS.HIDDEN || (PARS.BIGDOTS == 11 && gotdots)))
		bzero(plane, LWwidth*LWheight); //bzero courtesy of tim@apple.com (Tim Olson)

	P  = Xangle * DEGTORAD;
	H  = Yangle * DEGTORAD;
	B  = Zangle * DEGTORAD;
	Bb = PARS.STangle * DEGTORAD * 0.5;
	if (PARS.PARALLEL) Bb = -Bb;

	CX = cos(P);
	SX = sin(P);
	CY = cos(H);
	SY = sin(H);
	CZ = cos(B);
	SZ = sin(B);
	CZb= cos(Bb);
	SZb= sin(Bb);
	
	/* rotation matrix for x[y[z[rot]]] */
	AM =  CY*CZ;
	BM =  CY*SZ;
	CM =  -SY;
	DM =  SX*SY*CZ-CX*SZ;
	EM =  SX*SY*SZ+CX*CZ;
	FM =  SX*CY;
	GM =  CX*SY*CZ+SX*SZ;
	HM =  CX*SY*SZ-SX*CZ;
	IM =  CX*CY;

	// rotate global matrix by new matrix
	AN = AM*aM + BM*dM + CM*gM;
	BN = AM*bM + BM*eM + CM*hM;
	CN = AM*cM + BM*fM + CM*iM;
	DN = DM*aM + EM*dM + FM*gM;
	EN = DM*bM + EM*eM + FM*hM;
	FN = DM*cM + EM*fM + FM*iM;
	GN = GM*aM + HM*dM + IM*gM;
	HN = GM*bM + HM*eM + IM*hM;
	IN = GM*cM + HM*fM + IM*iM;
	
	if (PARS.ROTYPE == 1)
		{
		// save new matrix
		aM = AN;
		bM = BN;
		cM = CN;
		dM = DN;
		eM = EN;
		fM = FN;
		gM = GN;
		hM = HN;
		iM = IN;
		}
		
	if (PARS.COLOR) //setup so Lddd=dep*dep1+dep2
		{
		if (PARS.DEPTHcue)
			dep1=10.0;
		else
			dep2=9.0;
		}
	else
		{
		if (PARS.WBACK)
			dep2=11.0;
		else
			dep2=10.0;
		}
	
	tempZoom = (double)(W2middle)*zoom*.5;
	tempWidth = (double)(LWwidth)*(0.25 - (double)PARS.STsep*0.00625);
	tempWidthb = (double)(LWwidth)*(0.75 + (double)PARS.STsep*0.00625);
	tempHeight = (double)(LWheight)*.5;
	tempPerspec = ((double)PARS.PERSPEC*.07);
	
	for (LEc = 0; LEc < N; LEc++)
		{
		//rotate a point
		paaa=aaa+LEc;
		X = paaa->a[0];
		if (PARS.CHIR) Y =-paaa->a[1]; else Y = paaa->a[1];
		Z = paaa->a[2];
		
		X3 = AN * X + BN * Y + CN * Z;
		Y3 = DN * X + EN * Y + FN * Z;
		Z3 = GN * X + HN * Y + IN * Z;
		
		X3b = CZb * X3 + SZb * Y3;
		Y3b = -SZb * X3 + CZb * Y3;
		Z3b = Z3;
		
		/* calculate depth of point */
		Ldep = Y3*0.5+0.5;
		Ldepb   = Y3b*0.5+0.5;
		Lugh    = Ldep * 255.0;
		Lughb   = Ldepb* 255.0;
		currcolor=cc[LEc];
		Lddd    = dep1*Ldep+dep2;
		Ldddb   = dep1*Ldepb+dep2;
		
		// take perspective into account
		Y3 = 1.0 - tempPerspec*(1.0 - Ldep);
		Y3b = 1.0 - tempPerspec*(1.0 - Ldepb);
		X3 *= Y3;     X3b *= Y3b;
		Z3 *= Y3;     Z3b *= Y3b;
		
		// 3D -> 2D
		XX  = tempWidth  + X3*tempZoom;
		YY  = tempHeight - Z3*tempZoom;
		XXb = tempWidthb + X3b*tempZoom;
		YYb = tempHeight - Z3b*tempZoom;
		
		if (currcolor < 0) // a point
			{
			currcolor = -currcolor;
			bytecolor = bcol [currcolor] [Lddd];
			bytecolorb = bcol [currcolor] [Ldddb];
			switch (bigdots)
				{
				case 11: //ball
					{
					cooldot (XX, YY, currcolor, Ldep, Lugh);
					cooldot (XXb, YYb, currcolor, Ldepb, Lughb);
					}
					break;
				
				case 1: //dot size =1
					{
					if (XX < LWwidth && XX >= 0 && YY < LWheight && YY >= 0)
						{
						if (PARS.HIDDEN)
							{
							if (Lugh >= *(plane + LWwidth*YY + XX))
								{
								*(plane + LWwidth*YY + XX) = Lugh;
								base[Lrb*YY + XX] = bytecolor;
								}
							}
						else base[Lrb*YY + XX] = bytecolor;
						}
					
					if (XXb < LWwidth && XXb >= 0 && YYb < LWheight && YYb >= 0)
						{
						if (PARS.HIDDEN)
							{
							if (Lughb >= *(plane + LWwidth*YYb + XXb))
								{
								*(plane + LWwidth*YYb + XXb) = Lughb;
								base[Lrb*YYb + XXb] = bytecolorb;
								}
							}
						else base[Lrb*YYb + XXb] = bytecolorb;
						}
					}
					break;
				
				default: //PARS.BIGDOTS >1
					{
					XX -= (bigdots >> 1);
					YY -= (bigdots >> 1);
					XXb-= (bigdots >> 1);
					YYb-= (bigdots >> 1);
					if (PARS.HIDDEN)
						{
						for (j=0;j<bigdots;j++,YY++,YYb++)
							for (i=0;i<bigdots;i++)
								{
								if (YY < LWheight && YY >= 0 && XX+i < LWwidth && XX+i >= 0)
									if (Lugh >= *(plane + LWwidth*YY + XX+i))
										{
										*(plane + LWwidth*YY + XX+i) = Lugh;
										base[Lrb*YY + XX+i] = bytecolor;
										}
								if (YYb < LWheight && YYb >= 0 && XXb+i < LWwidth && XXb+i >= 0)
									if (Lughb >= *(plane + LWwidth*YYb + XXb+i))
										{
										*(plane + LWwidth*YYb + XXb+i) = Lughb;
										base[Lrb*YYb + XXb+i] = bytecolorb;
										}
								}
						}
					else // not hidden
						for (j=0;j<bigdots;j++,YY++,YYb++)
							for (i=0;i<bigdots;i++)
								{
								if(YY < LWheight && YY >= 0 && XX+i < LWwidth && XX+i >= 0)
									base[Lrb*YY + XX+i] = bytecolor;
								if(YYb < LWheight && YYb >= 0 && XXb+i < LWwidth && XXb+i >= 0)
									base[Lrb*YYb + XXb+i] = bytecolorb;
								}
					} //end default
				} //end switch
			} //end a point
		else
			{
			if (currcolor) // a line
				{
				if (PARS.HIDDEN)
					{
					plot_hiddenline (XX, XX1, YY, YY1, bcol[currcolor][Lddd], Lugh, Lugh1);
					plot_hiddenline (XXb, XX1b, YYb, YY1b, bcol[currcolor][Ldddb], Lughb, Lugh1b);
					}
				else
					{
					plotline (XX, XX1, YY, YY1, bcol[currcolor][Lddd]);
					plotline (XXb, XX1b, YYb, YY1b, bcol[currcolor][Ldddb]);
					}
				}
			XX1 = XX;  XX1b = XXb; // move
			YY1 = YY;  YY1b = YYb;
			Lugh1= Lugh; Lugh1b= Lughb;
			}
		}
}

//************************************************************************

void rotatemono (void)
{
	double 	X, Y, Z, X3, Y3, Z3;
	double	CZ, SZ, CY, SY, CX, SX;
	double	AM, BM, CM, DM, EM, FM, GM, HM, IM;
	double	AN, BN, CN, DN, EN, FN, GN, HN, IN;
	double	P, B, H;
	long i, j;
	double  tempWidth, tempHeight, tempZoom, tempPerspec;
	double  Ldep, dep1=0, dep2=0;
	long    LEc, Lugh, Lugh1, Lddd, currcolor, LWheight=Wheight, LWwidth=Wwidth, Lrb=rb;
	long		XX, XX1, YY, YY1;
	long    bigdots=PARS.BIGDOTS;
	Byte		bytecolor;
	struct Aray *paaa;
	Byte *cbase, *cplane;
	
	//set up locals
	tempZoom	=(double)(Wmiddle)*zoom*.5;
	tempWidth	=(double)(LWwidth)*.5;
	tempHeight	=(double)(LWheight)*.5;
	tempPerspec	=((double)PARS.PERSPEC*.07);
	
	if (PARS.COLOR) //setup so Lddd=dep*dep1+dep2
		{
		if (PARS.DEPTHcue)
			dep1=10.0;
		else
			dep2=9.0;
		}
	else
		{
		if (PARS.WBACK)
			dep2=11.0;
		else
			dep2=10.0;
		}

	if ((PARS.KIND < 3 || !PARS.STEREO) && (PARS.HIDDEN || (PARS.BIGDOTS == 11 && gotdots)))
		bzero(plane, LWwidth*LWheight);
		
	P  = DEGTORAD * Xangle;
	H  = DEGTORAD * Yangle;
	B  = DEGTORAD * Zangle;
	
	CX = cos(P);
	SX = sin(P);
	CY = cos(H);
	SY = sin(H);
	CZ = cos(B);
	SZ = sin(B);
	
	/* rotation matrix for x[y[z[rot]]] */
	AM =  CY*CZ;
	BM =  CY*SZ;
	CM =  -SY;
	DM =  SX*SY*CZ-CX*SZ;
	EM =  SX*SY*SZ+CX*CZ;
	FM =  SX*CY;
	GM =  CX*SY*CZ+SX*SZ;
	HM =  CX*SY*SZ-SX*CZ;
	IM =  CX*CY;
	
		// rotate global matrix by new matrix
		AN = AM*aM + BM*dM + CM*gM;
		BN = AM*bM + BM*eM + CM*hM;
		CN = AM*cM + BM*fM + CM*iM;
		DN = DM*aM + EM*dM + FM*gM;
		EN = DM*bM + EM*eM + FM*hM;
		FN = DM*cM + EM*fM + FM*iM;
		GN = GM*aM + HM*dM + IM*gM;
		HN = GM*bM + HM*eM + IM*hM;
		IN = GM*cM + HM*fM + IM*iM;
		
	if (PARS.ROTYPE == 1)
		{
		// save new matrix
		aM = AN;
		bM = BN;
		cM = CN;
		dM = DN;
		eM = EN;
		fM = FN;
		gM = GN;
		hM = HN;
		iM = IN;
		}
	
	for (LEc = 0; LEc < N; LEc++)
		{
		//rotate a point
		paaa=aaa+LEc;
		X = paaa->a[0];
		if (PARS.CHIR) Y =-paaa->a[1]; else Y = paaa->a[1];
		Z = paaa->a[2];
		
		X3 = AN * X + BN * Y + CN * Z;
		Y3 = DN * X + EN * Y + FN * Z;
		Z3 = GN * X + HN * Y + IN * Z;
		
		/* calculate depth of point */
		Ldep = Y3*0.5+0.5;
		Lugh = Ldep * 255.0;
		currcolor=cc[LEc];
		Lddd=dep1*Ldep+dep2;
		
		// take perspective into account
		Y3=1.0 - tempPerspec*(1.0 - Ldep);
		X3 *= Y3;
		Z3 *= Y3;
		
		// 3D -> 2D
		XX = tempWidth  + X3*tempZoom;
		YY = tempHeight - Z3*tempZoom;
		
		if (currcolor < 0) // a point
			{
			currcolor=-currcolor;
			bytecolor = bcol[currcolor][Lddd];
			switch (bigdots)
				{
				case 11: //ball
					cooldot (XX, YY, currcolor, Ldep, Lugh);
					break;
				
				case 1: //dot size =1
					{
					if (XX < LWwidth && XX >= 0 && YY < LWheight && YY >= 0)
						{
						if (PARS.HIDDEN)
							{
							if (Lugh >= *(plane + LWwidth*YY + XX))
								{
								*(plane + LWwidth*YY + XX) = Lugh;
								base[Lrb*YY + XX] = bytecolor;
								}
							}
						else base[Lrb*YY + XX] = bytecolor;
						}
					}
					break;
				
				default: //PARS.BIGDOTS >1
					{
					XX -= (bigdots >> 1);
					YY -= (bigdots >> 1);
					if (PARS.HIDDEN)
						{
						for (j=0;j<bigdots;j++,YY++) if(YY < LWheight && YY >= 0)
							for (cplane=plane+LWwidth*YY+XX,i=0;i<bigdots;i++)
									if (XX+i < LWwidth && XX+i >= 0 && Lugh >= cplane[i])
										{
										cplane[i] = Lugh;
										base[Lrb*YY + XX+i] = bytecolor;
										}
						}
					else

						for (j=0;j<bigdots;j++,YY++) if(YY < LWheight && YY >= 0)
							for (cbase=base+Lrb*YY+XX,i=0;i<bigdots;i++) if (XX+i < LWwidth && XX+i >= 0)
								cbase[i] = bytecolor;
					} //end bigdots
				} //end switch
			} //end a point
		else
			{
			if (currcolor) // a line
				{
				if (PARS.HIDDEN)
					plot_hiddenline (XX, XX1, YY, YY1, bcol[currcolor][Lddd], Lugh, Lugh1);
				else
					plotline (XX, XX1, YY, YY1, bcol[currcolor][Lddd]);
				}
			XX1 = XX; // move
			YY1 = YY;
			Lugh1 = Lugh;
			}
		}
}

//************************************************************************

void adjustsize (void)
{
	Point p;
	GrafPtr oldPort;
	
	HLockHi((Handle)windowPixMapH);
	LockPixels(windowPixMapH);
	windowPixMapP = *thewindow->portPixMap;
	
	/* get window width and height */
	p.h = (thewindow->portRect).left;
	p.v = (thewindow->portRect).top;
	LocalToGlobal (&p);
	PARS.WINleft = p.h;
	PARS.WINtop = p.v;
	p.h = (thewindow->portRect).right;
	p.v = (thewindow->portRect).bottom;
	LocalToGlobal (&p);
	PARS.WINright = p.h;
	PARS.WINbottom = p.v;
	
	Wwidth = (thewindow->portRect).right - (thewindow->portRect).left;
	Wheight = (thewindow->portRect).bottom - (thewindow->portRect).top;
	
	// get floating window position
	GetPort (&oldPort);
	SetPort (infowindow);
	p.h = (infowindow->portRect).left;
	p.v = (infowindow->portRect).top;
	LocalToGlobal (&p);
	PARS.INFOleft = p.h;
	PARS.INFOtop = p.v;
	SetPort (oldPort);
	
	/* size of square image */
	if (Wheight < Wwidth) Wmiddle = Wheight;
	else Wmiddle = Wwidth;
	
	/* size of stereo image */
	if (Wheight < Wwidth/2) W2middle = Wheight;
	else W2middle = Wwidth/2;
	
	/* set active rectangle dimensions */
	SetRect (&frameRect, 0, 0, Wwidth+1, Wheight+1);
	ClipRect(&frameRect);
}

//************************************************************************

void plotline (long x, long x1, long y, long y1, long bytecol) // Custom Line drawing routine for extra speed
{
	long i, k;
	long LWwidth=Wwidth,LWheight=Wheight,Lrb=rb,line=PARS.LINE,halfline=(line>>1);
	int r, dx, dy;
	Byte *cbase;
	
	// make x <= x1
	if (x > x1) { i = x1; x1 = x; x = i; i = y1; y1 = y; y = i; }
	
	dx = (x1-x)*2; dy = (y1-y)*2; // << 1
	
	if (dy >= 0) //positive line
		{
		if (dx > dy) // x longer
			{
			if(x1>=LWwidth-1) x1=LWwidth-1;
			r = dy - (dx>>1); dx -= dy;
			for (;x<=x1;x++)
				{
				if (x >= 0)
					{
					i = y - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWheight) k=LWheight;
					for(cbase=base+Lrb*i + x;i<k;i++,cbase+=Lrb) *cbase = bytecol;
					}
				if (r >= 0) { y++; r -= dx; }
				else r += dy;
				}
			}
		else // y longer
			{
			if(y1>=LWheight-1) y1=LWheight-1;
			r = dx - (dy>>1); dy -= dx;
			for (;y<=y1;y++)
				{
				if (y >= 0)
					{
					i = x - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWwidth) k=LWwidth;
					for(cbase=base+Lrb*y;i<k;i++) cbase[i] = bytecol;
					}
				if (r >= 0) { x++; r -= dy; }
				else r += dx;
				}
			}
		}
	else //negative line
		{
		dy=-dy;
		if (dx > dy) // x longer
			{
			if(x1>=LWwidth-1) x1=LWwidth-1;
			r = dy - (dx>>1); dx -= dy;
			for (;x<=x1;x++)
				{
				if (x >= 0)
					{
					i = y - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWheight) k=LWheight;
					for(cbase=base+Lrb*i + x;i<k;i++,cbase+=Lrb) *cbase = bytecol;
					}
				if (r >= 0) { y--; r -= dx; }
				else r += dy;
				}
			}
		else // y longer
			{
			if(y>=LWheight-1) y=LWheight-1;
			r = dx - (dy>>1); dy -= dx;
			for (;y1<=y;y1++)
				{
				if (y1 >= 0)
					{
					i = x1 - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWwidth) k=LWwidth;
					for(cbase=base+Lrb*y1;i<k;i++) cbase[i] = bytecol;
					}
				if (r >= 0) { x1--; r -= dy; }
				else r += dx;
				}
			}
		}
}

//************************************************************************

void plot_hiddenline (long x, long x1, long y, long y1, long bytecol, long ughx, long ugh1x) // Custom Line drawing routine for extra speed
{
	long xi, yi, ui, i, k;
	long LWwidth=Wwidth,LWheight=Wheight,Lrb=rb;
	long ughy,ddx,ddy,line=PARS.LINE,halfline=(line>>1);
	int r, dx, dy;
	Byte *cplane;
	
	// make x <= x1
	if (x > x1)
	{
	 xi = x1; x1 = x; x = xi;
	 yi = y1; y1 = y; y = yi;
	 ui = ughx; ughx = ugh1x; ugh1x = ughx-ugh1x;
	}
	else ugh1x -= ughx;
	
	ddx=x1-x; ddy=y1-y;
	dx = ddx*2; dy = ddy*2; // << 1
	
	if (dy >= 0) //positive line
		{
		if (dx > dy) // x longer
			{
			if(x1>=LWwidth-1) x1=LWwidth-1;
			r = dy - ddx; dx -= dy; if (!ddx) ughy = ughx;
			for (xi=0;x<=x1;x++,xi+=ugh1x)
				{
				if (x >= 0)
					{
					i = y - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWheight) k=LWheight;
					if (ddx) ughy = xi/ddx + ughx;
					for(cplane=plane+x+LWwidth*i;i<k;i++,cplane+=LWwidth) if(ughy>=*cplane)
					{ *cplane = ughy; *(base + x+Lrb*i) = bytecol; }
					}
				if (r >= 0) { y++; r -= dx; }
				else r += dy;
				}
			}
		else // y longer
			{
			if(y1>=LWheight-1) y1=LWheight-1;
			r = dx - ddy; dy -= dx; if (!ddy) ughy = ughx;
			for (yi=0;y<=y1;y++,yi+=ugh1x)
				{
				if (y >= 0)
					{
					i = x - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWwidth) k=LWwidth;
					if (ddy) ughy = yi/ddy + ughx;
					for(cplane=plane+LWwidth*y;i<k;i++) if(ughy>=cplane[i])
					{ cplane[i] = ughy; *(base+Lrb*y + i) = bytecol; }
					}
				if (r >= 0) { x++; r -= dy; }
				else r += dx;
				}
			}
		}
	else //negative line
		{
		dy=-dy;
		if (dx > dy) // x longer
			{
			if(x1>=LWwidth-1) x1=LWwidth-1;
			r = dy - ddx; dx -= dy; if (!ddx) ughy = ughx;
			for (xi=0;x<=x1;x++,xi+=ugh1x)
				{
				if (x >= 0)
					{
					i = y - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWheight) k=LWheight;
					if (ddx) ughy = xi/ddx + ughx;
					for(cplane=plane+x+LWwidth*i;i<k;i++,cplane+=LWwidth) if(ughy>=*cplane)
					{ *cplane = ughy; *(base + x+Lrb*i) = bytecol; }
					}
				if (r >= 0) { y--; r -= dx; }
				else r += dy;
				}
			}
		else // y longer
			{
			//ddy = -ddy;
			if(y>=LWheight-1) y=LWheight-1;
			r = dx + ddy; dy -= dx; if (!ddy) ughy = ughx;
			for (yi=ddy*ugh1x;y1<=y;y1++,yi+=ugh1x)
				{
				if (y1 >= 0)
					{
					i = x1 - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWwidth) k=LWwidth;
					if (ddy) ughy = yi/ddy + ughx;
					for(cplane=plane+LWwidth*y1;i<k;i++) if(ughy>=cplane[i])
					{ cplane[i] = ughy; *(base + i + Lrb*y1) = bytecol; }
					}
				if (r >= 0) { x1--; r -= dy; }
				else r += dx;
				}
			}
		}
}

//************************************************************************

void doclose(void) // close window
{
	N = 0;
	comn = 0;
	comW = 0;
	comH = 0;
	HideWindow ((WindowPtr)thewindow);
	if (PARS.INFO) info();
	updatemenus();
}

//************************************************************************

void resetimage (void)
{
	zoom = 1;
	
	PARS.STEREO = FALSE;
	PARS.PERSPEC = 3;
	PARS.STangle = 6;
	PARS.STsep = 0;
	PARS.DEPTHcue = TRUE;
	PARS.COLOR = TRUE;
	PARS.WBACK = FALSE;
	PARS.BIGDOTS = 1; //small dots
	PARS.CHIR = TRUE;
	PARS.PARALLEL = FALSE;
	PARS.dpi = 300;
	PARS.KIND = 1;
	PARS.GRAB = TRUE; // grab and spin on
	PARS.LINE = 1; // line width of 1
	PARS.HIDDEN = TRUE; // hide lines
	PARS.INFO = TRUE; // show info window
	PARS.FUNKY = FALSE; // no wierd traces
	PARS.ROTYPE = 1; // trackball rotation
	PARS.SHANGS = FALSE; // dont show angles
	
	UnlockPixels(windowPixMapH);
	MoveWindow ((WindowPtr)infowindow, 220, 50, TRUE);
	MoveWindow ((WindowPtr)thewindow, 10, 50, TRUE);
	if (MAXheight < 199 || MAXwidth < 199)
		SizeWindow ((WindowPtr)thewindow, MAXwidth-1, MAXheight-1, TRUE);
	else
		SizeWindow ((WindowPtr)thewindow, 199, 199, TRUE);
	if (PARS.INFO)
		{
		ShowWindow (infowindow);
		BringToFront (infowindow); //!!
		HiliteWindow (infowindow, TRUE); //!!
		}
	else HideWindow (infowindow);
	adjustsize();
	updatemenus();
	rotate();
}

//************************************************************************

void copyimage (void) // copy image to clipboard
{
	OpenCPicParams picheader;
	
	picheader.srcRect = frameRect;
	picheader.hRes = 72 << 16;
	picheader.vRes = 72 << 16;
	picheader.version = -2;
	picheader.reserved1 = 0;
	picheader.reserved2 = 0;
	
	thepic = OpenCPicture (&picheader);
	
	CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP,
		&frameRect, &frameRect, srcCopy, nil);
	
	ClosePicture();
	
	err = ZeroScrap();
	if (err) died ("Error Clearing Clipboard", err, 501);
	
	HLock ((Handle)(thepic));
	err = PutScrap (GetHandleSize((Handle)thepic), 'PICT', (Ptr)*thepic);
	HUnlock ((Handle)(thepic));
	KillPicture (thepic);

	if (err) Alert(136,NULL);
}

//************************************************************************

void vectorcopy (void) // Quickdraw stuff only - no custom fast things
{
	long mem, Ec;
	double 	X, Y, Z, X3, Y3, Z3;
	double	CZ, SZ, CY, SY, CX, SX;
	double	AM, BM, CM, DM, EM, FM, GM, HM, IM;
	double	AN, BN, CN, DN, EN, FN, GN, HN, IN;
	double	P, B, H;
	double	dep;
	long XX, YY, XX1=0, YY1=0;
	
	startball();
	
	if (!PARS.WBACK) PaintRect(&frameRectDPI);
	
	P  = Xangle * DEGTORAD;
	H  = Yangle * DEGTORAD;
	B  = Zangle * DEGTORAD;

	CX = cos(P);
	SX = sin(P);
	CY = cos(H);
	SY = sin(H);
	CZ = cos(B);
	SZ = sin(B);
	
	/* rotation matrix for x[y[z[rot]]] */
	AM =  CY*CZ;
	BM =  CY*SZ;
	CM =  -SY;
	DM =  SX*SY*CZ-CX*SZ;
	EM =  SX*SY*SZ+CX*CZ;
	FM =  SX*CY;
	GM =  CX*SY*CZ+SX*SZ;
	HM =  CX*SY*SZ-SX*CZ;
	IM =  CX*CY;

	// rotate global matrix by new matrix
	AN = AM*aM + BM*dM + CM*gM;
	BN = AM*bM + BM*eM + CM*hM;
	CN = AM*cM + BM*fM + CM*iM;
	DN = DM*aM + EM*dM + FM*gM;
	EN = DM*bM + EM*eM + FM*hM;
	FN = DM*cM + EM*fM + FM*iM;
	GN = GM*aM + HM*dM + IM*gM;
	HN = GM*bM + HM*eM + IM*hM;
	IN = GM*cM + HM*fM + IM*iM;
	
	if (!PARS.STEREO)
		{
		for (Ec = 0; Ec < N; Ec++)
			{
			goball();
			X = aaa[Ec].a[0];
			Y = aaa[Ec].a[1];
			Z = aaa[Ec].a[2];
			
			if (PARS.CHIR) Y = -Y;
			
			/* rotate the point */
			X3 = AN * X + BN * Y + CN * Z;
			Y3 = DN * X + EN * Y + FN * Z;
			Z3 = GN * X + HN * Y + IN * Z;
			
			/* calculate depth of point */
			dep = (Y3 + 1)/2;
			
			if (PARS.PERSPEC) /* perspective is on*/
				{
				X3 = X3 * (1.0 - 7.0*PARS.PERSPEC/100.0 + 7.0*PARS.PERSPEC*dep/100.0);
				Z3 = Z3 * (1.0 - 7.0*PARS.PERSPEC/100.0 + 7.0*PARS.PERSPEC*dep/100.0);
				}
			
			XX = Wwidth*PARS.dpi/144 + Wmiddle*X3*zoom*PARS.dpi/144;
			YY = Wheight*PARS.dpi/144 - Wmiddle*Z3*zoom*PARS.dpi/144;
	
			if (cc[Ec] < 0) // a point
				{
				if (PARS.COLOR)
					if (PARS.DEPTHcue) ddd = dep*10;
					else ddd = 9;
				else
					{
					if (PARS.WBACK) ddd = 11;
					else ddd = 10;
					}
				//SetCPixel(XX, YY, &vcol[-cc[Ec]][ddd]);
				MoveTo (XX,YY);
				RGBForeColor(&vcol[-cc[Ec]][ddd]);
				LineTo (XX, YY);				
				}
			else if (cc[Ec] == 0) // move
				{
				MoveTo (XX, YY);
				}
			else // a line
				{
				if (PARS.COLOR)
					if (PARS.DEPTHcue) ddd = dep*10;
					else ddd = 9;
				else
					{
					if (PARS.WBACK) ddd = 11;
					else ddd = 10;
					}
				RGBForeColor(&vcol[cc[Ec]][ddd]);
				LineTo (XX, YY);
				}
			mem = FreeMem ();
			if (mem < 20000)
				{
				ClosePicture();
				KillPicture (thepic);
				died ("Ran out of memory", Ec, 601);
				}
			}
		}
	else //stereo
		{
		for (Ec = 0; Ec < N; Ec++)
			{
			goball();
			X = aaa[Ec].a[0];
			Y = aaa[Ec].a[1];
			Z = aaa[Ec].a[2];
			
			if (PARS.CHIR) Y = -Y;
			
			/* rotate the point */
			X3 = AN * X + BN * Y + CN * Z;
			Y3 = DN * X + EN * Y + FN * Z;
			Z3 = GN * X + HN * Y + IN * Z;
			
			/* calculate depth of point */
			dep = (Y3 + 1)/2;
			
			if (PARS.PERSPEC) /* perspective is on*/
				{
				X3 = X3 * (1.0 - 7.0*PARS.PERSPEC/100.0 + 7.0*PARS.PERSPEC*dep/100.0);
				Z3 = Z3 * (1.0 - 7.0*PARS.PERSPEC/100.0 + 7.0*PARS.PERSPEC*dep/100.0);
				}
			
			XX = Wwidth*PARS.dpi/288 + W2middle*X3*zoom*PARS.dpi/144 -
				PARS.STsep*Wwidth*PARS.dpi/11520;
			YY = Wheight*PARS.dpi/144 - W2middle*Z3*zoom*PARS.dpi/144;
	
			if (cc[Ec] < 0) // a point
				{
				if (PARS.COLOR)
					if (PARS.DEPTHcue) ddd = dep*10;
					else ddd = 9;
				else
					{
					if (PARS.WBACK) ddd = 11;
					else ddd = 10;
					}
				SetCPixel( XX, YY, &vcol[-cc[Ec]][ddd]);
				}
			else if (cc[Ec] == 0) // move
				{
				MoveTo (XX, YY);
				}
			else // a line
				{
				if (PARS.COLOR)
					if (PARS.DEPTHcue) ddd = dep*10;
					else ddd = 9;
				else
					{
					if (PARS.WBACK) ddd = 11;
					else ddd = 10;
					}
				RGBForeColor(&vcol[cc[Ec]][ddd]);
				LineTo (XX, YY);
				}
			mem = FreeMem ();
			if (mem < 20000)
				{
				ClosePicture();
				KillPicture (thepic);
				died ("Ran out of memory", Ec, 602);
				}
			}
		
		P  = DEGTORAD * Xangle;
		H  = DEGTORAD * Yangle;
		B  = DEGTORAD * Zangle;

		if (PARS.PARALLEL)
			B = (Zangle - PARS.STangle * 0.5) * DEGTORAD;
		else
			B = (Zangle + PARS.STangle * 0.5) * DEGTORAD;
		
		CX = cos(P);
		SX = sin(P);
		CY = cos(H);
		SY = sin(H);
		CZ = cos(B);
		SZ = sin(B);
		
		/* rotation matrix for x[y[z[rot]]] */
		AM =  CY*CZ;
		BM =  CY*SZ;
		CM =  -SY;
		DM =  SX*SY*CZ-CX*SZ;
		EM =  SX*SY*SZ+CX*CZ;
		FM =  SX*CY;
		GM =  CX*SY*CZ+SX*SZ;
		HM =  CX*SY*SZ-SX*CZ;
		IM =  CX*CY;
		
		// rotate global matrix by new matrix
		AN = AM*aM + BM*dM + CM*gM;
		BN = AM*bM + BM*eM + CM*hM;
		CN = AM*cM + BM*fM + CM*iM;
		DN = DM*aM + EM*dM + FM*gM;
		EN = DM*bM + EM*eM + FM*hM;
		FN = DM*cM + EM*fM + FM*iM;
		GN = GM*aM + HM*dM + IM*gM;
		HN = GM*bM + HM*eM + IM*hM;
		IN = GM*cM + HM*fM + IM*iM;
		
		for (Ec = 0; Ec < N; Ec++)
			{
			goball();
			X = aaa[Ec].a[0];
			Y = aaa[Ec].a[1];
			Z = aaa[Ec].a[2];
			
			if (PARS.CHIR) Y = -Y;
			
			/* rotate the point */
			X3 = AN * X + BN * Y + CN * Z;
			Y3 = DN * X + EN * Y + FN * Z;
			Z3 = GN * X + HN * Y + IN * Z;
			
			/* calculate depth of point */
			dep = (Y3 + 1)/2;
			
			if (PARS.PERSPEC) /* perspective is on*/
				{
				X3 = X3 * (1.0 - 7.0*PARS.PERSPEC/100.0 + 7.0*PARS.PERSPEC*dep/100.0);
				Z3 = Z3 * (1.0 - 7.0*PARS.PERSPEC/100.0 + 7.0*PARS.PERSPEC*dep/100.0);
				}
			
			XX = Wwidth*PARS.dpi/72 - Wwidth*PARS.dpi/288 + W2middle*X3*zoom*PARS.dpi/144 +
				PARS.STsep*Wwidth*PARS.dpi/11520;
			YY = Wheight*PARS.dpi/144 - W2middle*Z3*zoom*PARS.dpi/144;
			
			if (cc[Ec] < 0) // a point
				{
				if (PARS.COLOR)
					if (PARS.DEPTHcue) ddd = dep*10;
					else ddd = 9;
				else
					{
					if (PARS.WBACK) ddd = 11;
					else ddd = 10;
					}
				SetCPixel( XX, YY, &vcol[-cc[Ec]][ddd]);
				}
			else if (cc[Ec] == 0) // move
				{
				MoveTo (XX, YY);
				}
			else // a line
				{
				if (PARS.COLOR)
					if (PARS.DEPTHcue) ddd = dep*10;
					else ddd = 9;
				else
					{
					if (PARS.WBACK) ddd = 11;
					else ddd = 10;
					}
				RGBForeColor(&vcol[cc[Ec]][ddd]);
				LineTo (XX, YY);
				}
			mem = FreeMem ();
			if (mem < 20000)
				{
				ClosePicture();
				KillPicture (thepic);
				died ("Ran out of memory", Ec, 603);
				}
			}
		}
	RGBForeColor(&black);
	InitCursor();
}

//************************************************************************

void savepict (Boolean vector)
{
	static Point SFPwhere = {150, 150};
	static StandardFileReply reply;
	long longCount = 1;
	short globalRef;
	long i;
	ResType theType = 'PICT';
	OpenCPicParams picheader;
	char name[256], zero = 0;
	
	for (i=1;i<=myFSS.name[0];i++)
		name[i] = myFSS.name[i];
	if (vector)
		{
		i = myFSS.name[0];
		name[++i] = '.';
		name[++i] = 'v';
		name[++i] = 'e';
		name[++i] = 'c';		
		name[++i] = 't';
		name[++i] = 'o';
		name[++i] = 'r';
		name[0] = i;
		}
	else
		{
		i = myFSS.name[0];
		name[++i] = '.';
		name[++i] = 'b';
		name[++i] = 'i';
		name[++i] = 't';		
		name[++i] = 'm';
		name[++i] = 'a';
		name[++i] = 'p';
		name[0] = i;
		}
	
	StandardPutFile ("\pSave Pict File as:", (StringPtr)name, &reply);
	
	if (reply.sfGood)
		{
		rotate();
		
		if (vector)
			{
			SetRect (&frameRectDPI, 0, 0, (Wwidth+1)*PARS.dpi/72, (Wheight+1)*PARS.dpi/72);
			picheader.srcRect = frameRectDPI;
			picheader.hRes = PARS.dpi << 16;
			picheader.vRes = PARS.dpi << 16;
			picheader.version = -2;
			picheader.reserved1 = 0;
			picheader.reserved2 = 0;
			}
		else
			{
			picheader.srcRect = frameRect;
			picheader.hRes = 72 << 16;
			picheader.vRes = 72 << 16;
			picheader.version = -2;
			picheader.reserved1 = 0;
			picheader.reserved2 = 0;
			}
		
		thepic = OpenCPicture (&picheader);
		
		if (vector)
			{
			ClipRect(&frameRectDPI);
			vectorcopy();
			}
		else
		CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP,
			&frameRect, &frameRect, srcCopy, nil);
		
		ClosePicture();
		
		ClipRect(&frameRect);
		
		DrawPicture (thepic, &frameRect);
		
		err = FSpCreate (&reply.sfFile, 'ttxt', 'PICT', reply.sfScript);
		
		if (err && err != -48) died ("Could not Create File", err, 701);
		if (err == -48) // file already exists
			{
			err = FSpDelete (&reply.sfFile);
			if (err && err != -45) died ("Could not Update File", err, 702);
			if (err) Alert(137,NULL);
			else
				{
				err = FSpCreate (&reply.sfFile, 'ttxt', 'PICT', reply.sfScript);
				if (err) died ("Could not Overwrite File", err, 703);
				}
			}
		
		if (!err)
			{
			// open the file
			err = FSpOpenDF (&reply.sfFile, fsRdWrPerm, &globalRef);
			if (err) died ("Could not Open File", err, 704);
			
			longCount = 1;
			for (i=0; i<512; i++) // write 512 zeros
				FSWrite (globalRef, &longCount, &zero);
			
			HLock ((Handle)(thepic));
			longCount = GetHandleSize ((Handle)thepic);
			FSWrite (globalRef, &longCount, (Ptr)(*thepic));
			FSClose (globalRef);
			HUnlock ((Handle)(thepic));
			}
		
		KillPicture (thepic);
		}
}

//************************************************************************

void readprefs (void)
{
	short refnum;
	long dirid, siz;
	FSSpec spec;
	
	// set up reasonable defaults
	PARS.WINleft = 10;
	PARS.WINtop = 50;
	PARS.WINright = 210;
	PARS.WINbottom = 250;
	
	PARS.INFOleft = 220;
	PARS.INFOtop = 50;
	
	PARS.PERSPEC = 3; // perspective is on
	PARS.DEPTHcue = TRUE; // depth cueing is on
	PARS.COLOR = TRUE; // colored image
	PARS.WBACK = FALSE; // black background
	PARS.BIGDOTS = 1; //small dots
	PARS.CHIR = TRUE; // right hand system
	PARS.STEREO = FALSE; // no stereo
	PARS.PARALLEL = FALSE; // crosseyed stereo
	PARS.STangle = 6; // stereo angle = 3 degrees
	PARS.STsep = 0; // normal stereo seperation
	PARS.dpi = 300; // 300 dpi pict
	PARS.KIND = 1; // crosseyed stereo
	PARS.GRAB = TRUE; // grab and spin
	PARS.LINE = 1; // line width of 1
	PARS.HIDDEN = TRUE; // hide lines
	PARS.INFO = TRUE; // show info window
	PARS.FUNKY = FALSE; // no wierd traces
	
	PARS.MAXwidth = 640;
	PARS.MAXheight = 460;
	
	PARS.ROTYPE = 1; // trackball rotation
	PARS.SHANGS = FALSE; // dont show angles
	
	err = FindFolder (kOnSystemDisk, kPreferencesFolderType,
		kCreateFolder, &refnum, &dirid);
	if (err) died ("Could not find preferences folder", err, 801);
	
	err = FSMakeFSSpec (refnum, dirid, "\pRotater Prefs", &spec);
	if (err != 0 && err != -43) died ("Could not make FSSpec", err, 802);
	
	if (!err) // prefs file exists
		{
		// read in prefs
		err = FSpOpenDF (&spec, fsRdWrPerm, &refnum);
		if (err) died ("Could not Open prefs file", err, 803);
		
		siz = sizeof(PARS);
		err = FSRead (refnum, &siz, &PARS);
		// IGNORE errors so we can read old shorter Prefs files
		//if (err) died ("Old Prefs file found - Please delete manually", err, 804);
		//if (siz != sizeof(PARS)) died ("Old Prefs file found - Please delete manually", err, 805);
		
		FSClose (refnum);
		}
	
	if (!PARS.BIGDOTS) PARS.BIGDOTS = 1; // fix for old prefs files
	
	updatemenus();
}

//************************************************************************

void writeprefs (void)
{
	short refnum;
	long dirid, siz;
	FSSpec spec;
	
	err = FindFolder (kOnSystemDisk, kPreferencesFolderType,
		kCreateFolder, &refnum, &dirid);
	if (err) died ("Could not find preferences folder", err, 806);
	
	err = FSMakeFSSpec (refnum, dirid, "\pRotater Prefs", &spec);
	if (err != 0 && err != -43) died ("Could not make FSSpec", err, 807);
	
	if (err != -43) // file exists
		{
		err = FSpDelete (&spec); // delete it
		if (err) died ("Could not Delete old prefs file", err, 808);
		}
	
	// create new file
	err = FSpCreate (&spec, 'KrOt', 'PrEf', smSystemScript);
	if (err) died ("Could not Create prefs file", err, 809);
	
	err = FSpOpenDF (&spec, fsRdWrPerm, &refnum);
	if (err) died ("Could not Open prefs file", err, 810);
	
	siz = sizeof(PARS);
	err = FSWrite (refnum, &siz, &PARS);
	if (err) died ("Could not write to file", err, 811);
	if (siz != sizeof(PARS)) died ("Error writing Pref Info", err, 812);
	
	FSClose (refnum);
}

//************************************************************************

void rotatestereoRB (void)
{
	double 	X, Y, Z, X3, Y3, Z3, X3b, Y3b, Z3b;
	double	CZ, SZ, CY, SY, CX, SX, CZb, SZb;
	double	AM, BM, CM, DM, EM, FM, GM, HM, IM;
	double	AN, BN, CN, DN, EN, FN, GN, HN, IN;
	double	Ldep, Ldepb;
	double	P, B, H, Bb;
	double  tempWidth, tempHeight, tempZoom, tempPerspec;
	long	i, j, XX, YY, XX1=0, YY1=0, XXb, YYb, XX1b=0, YY1b=0;
	long    LEc, currcolor, LWheight=Wheight, LWwidth=Wwidth;
	long    bigdots=PARS.BIGDOTS;
	Byte    STr, STb_or_g, STredb_or_g, STbackground;
	Byte *cbase, *cbaseb;
	struct Aray *paaa;
	
	if(PARS.WBACK)
	{
		STbackground=STwhite; STb_or_g=STWred;
		if (PARS.KIND == 3)// red blue
			{	STr=STWblue; STredb_or_g=STWredblue; }
		else
			{	STr=STWgreen; STredb_or_g=STWredgreen; }
	}
	else
	{
		STbackground=STblack; STr=STred;
		if (PARS.KIND == 3)// red blue
			{	STb_or_g=STblue; STredb_or_g=STredblue; }
		else
			{	STb_or_g=STgreen; STredb_or_g=STredgreen; }
	}
	
	P  = DEGTORAD * Xangle;
	H  = DEGTORAD * Yangle;
	B  = DEGTORAD * Zangle;
	Bb =-DEGTORAD * PARS.STangle *.5; // left eye red
	
	CX = cos(P);
	SX = sin(P);
	CY = cos(H);
	SY = sin(H);
	CZ = cos(B);
	SZ = sin(B);
	CZb= cos(Bb);
	SZb= sin(Bb);
	
	/* rotation matrix for x[y[z[rot]]] */
	AM =  CY*CZ;
	BM =  CY*SZ;
	CM =  -SY;
	DM =  SX*SY*CZ-CX*SZ;
	EM =  SX*SY*SZ+CX*CZ;
	FM =  SX*CY;
	GM =  CX*SY*CZ+SX*SZ;
	HM =  CX*SY*SZ-SX*CZ;
	IM =  CX*CY;
	
	// rotate global matrix by new matrix
	AN = AM*aM + BM*dM + CM*gM;
	BN = AM*bM + BM*eM + CM*hM;
	CN = AM*cM + BM*fM + CM*iM;
	DN = DM*aM + EM*dM + FM*gM;
	EN = DM*bM + EM*eM + FM*hM;
	FN = DM*cM + EM*fM + FM*iM;
	GN = GM*aM + HM*dM + IM*gM;
	HN = GM*bM + HM*eM + IM*hM;
	IN = GM*cM + HM*fM + IM*iM;
	
	if (PARS.ROTYPE == 1)
		{
		// save new matrix
		aM = AN;
		bM = BN;
		cM = CN;
		dM = DN;
		eM = EN;
		fM = FN;
		gM = GN;
		hM = HN;
		iM = IN;
		}
	
	tempZoom	=(double)(Wmiddle)*zoom*.5;
	tempWidth	=(double)(LWwidth)*.5;
	tempHeight	=(double)(LWheight)*.5;
	tempPerspec	=((double)PARS.PERSPEC*.07);
	
	for (LEc = 0; LEc < N; LEc++)
		{
		currcolor=cc[LEc];
		//rotate a point
		paaa=aaa+LEc;
		X = paaa->a[0];
		if (PARS.CHIR) Y =-paaa->a[1]; else Y = paaa->a[1];
		Z = paaa->a[2];
		
		X3 = AN * X + BN * Y + CN * Z;
		Y3 = DN * X + EN * Y + FN * Z;
		Z3 = GN * X + HN * Y + IN * Z;
		
		X3b = CZb * X3 + SZb * Y3;
		Y3b = -SZb * X3 + CZb * Y3;
		Z3b = Z3;
		
		// calculate depth of point
		Ldep = Y3 * 0.5 + 0.5;
		Ldepb= Y3b* 0.5 + 0.5;
		
		// take perspective into account
		Y3 = 1.0 - tempPerspec*(1.0 - Ldep);
		Y3b= 1.0 - tempPerspec*(1.0 - Ldepb);
		X3 *= Y3;       X3b*= Y3b;
		Z3 *= Y3;       Z3b*= Y3b;
		
		// 3D -> 2D
		XX = tempWidth  + X3*tempZoom;
		YY = tempHeight - Z3*tempZoom;
		XXb= tempWidth  + X3b*tempZoom;
		YYb= tempHeight - Z3b*tempZoom;
		
		if (currcolor < 0) // a point
			{
			XX -= (bigdots>>1);
			YY -= (bigdots>>1);
			XXb-= (bigdots>>1);
			YYb-= (bigdots>>1);
			for (j=0;j<bigdots;j++,YY++,YYb++)
				for (cbase=base+rb*YY+XX, cbaseb=base+rb*YYb+XXb, i=0;i<bigdots;i++)
					{
					if (YY < LWheight && YY >= 0 && XX+i < LWwidth && XX+i >= 0)
						{
						if (cbase[i] == STbackground) cbase[i] = STr;
						else if (cbase[i] == STb_or_g) cbase[i] = STredb_or_g;
						}
					if (YYb < LWheight && YYb >= 0 && XXb+i <LWwidth && XXb+i >= 0)
						{
						if (cbaseb[i] == STbackground) cbaseb[i] = STb_or_g;
						else if (cbaseb[i] == STr) cbaseb[i] = STredb_or_g;
						}
					}
			}
		else
			{
			if (currcolor) // a line
				{
				plotlineRB (STr, STredb_or_g, STbackground, XX, XX1, YY, YY1);
				plotlineRB (STb_or_g, STredb_or_g, STbackground, XXb, XX1b, YYb, YY1b); // red blue
				}
			XX1 = XX; XX1b = XXb; // move
			YY1 = YY; YY1b = YYb;
			}
		}
}

//************************************************************************

void plotlineRB (Byte col, Byte mix, Byte bkgd, long x, long x1, long y, long y1) // handles overlapping colors
{
	long i, k;
	long LWwidth=Wwidth,LWheight=Wheight,Lrb=rb,line=PARS.LINE,halfline=(line>>1);
	int r, dx, dy;
	Byte *cbase;
	
	// make x <= x1
	if (x > x1) { i = x1; x1 = x; x = i; i = y1; y1 = y; y = i; }
	
	dx = (x1-x)*2; dy = (y1-y)*2; // << 1
	
	if (dy >= 0) //positive line
		{
		if (dx > dy) // x longer
			{
			if(x1>=LWwidth-1) x1=LWwidth-1;
			r = dy - (dx>>1); dx -= dy;
			for (;x<=x1;x++)
				{
				if (x >= 0)
					{
					i = y - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWheight) k=LWheight;
					for(cbase=base+Lrb*i + x;i<k;i++,cbase+=Lrb)
					{
					 if (*cbase == bkgd) *cbase = col;
					 else if (*cbase != col && *cbase != mix) *cbase = mix;
					}
					}
				if (r >= 0) { y++; r -= dx; }
				else r += dy;
				}
			}
		else // y longer
			{
			if(y1>=LWheight-1) y1=LWheight-1;
			r = dx - (dy>>1); dy -= dx;
			for (;y<=y1;y++)
				{
				if (y >= 0)
					{
					i = x - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWwidth) k=LWwidth;
					for(cbase=base+Lrb*y;i<k;i++)
					{
					 if (cbase[i] == bkgd) cbase[i] = col;
					 else if (cbase[i] != col && cbase[i] != mix) cbase[i] = mix;
					}
					}
				if (r >= 0) { x++; r -= dy; }
				else r += dx;
				}
			}
		}
	else //negative line
		{
		dy=-dy;
		if (dx > dy) // x longer
			{
			if(x1>=LWwidth-1) x1=LWwidth-1;
			r = dy - (dx>>1); dx -= dy;
			for (;x<=x1;x++)
				{
				if (x >= 0)
					{
					i = y - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWheight) k=LWheight;
					for(cbase=base+Lrb*i + x;i<k;i++,cbase+=Lrb)
					{
					 if (*cbase == bkgd) *cbase = col;
					 else if (*cbase != col && *cbase != mix) *cbase = mix;
					}
					}
				if (r >= 0) { y--; r -= dx; }
				else r += dy;
				}
			}
		else // y longer
			{
			if(y>=LWheight-1) y=LWheight-1;
			r = dx - (dy>>1); dy -= dx;
			for (;y1<=y;y1++)
				{
				if (y1 >= 0)
					{
					i = x1 - halfline; k = i + line;
					if(i<0) i=0;
					if(k>LWwidth) k=LWwidth;
					for(cbase=base+Lrb*y1;i<k;i++)
					{
					 if (cbase[i] == bkgd) cbase[i] = col;
					 else if (cbase[i] != col && cbase[i] != mix) cbase[i] = mix;
					}
					}
				if (r >= 0) { x1--; r -= dy; }
				else r += dx;
				}
			}
		}
}

//************************************************************************

void updatemenus (void)
{
	long i;
	
	HiliteMenu(0);
	
	// perspective submenu
	for (i=1;i<=11;i++) CheckItem (persmenu, i, FALSE);
	CheckItem (persmenu, PARS.PERSPEC+1, TRUE);
	
	// view ticks
	if (PARS.DEPTHcue) CheckItem (viewmenu, 4, TRUE); else CheckItem (viewmenu, 4, FALSE);
	if (PARS.HIDDEN) CheckItem (viewmenu, 5, TRUE); else CheckItem (viewmenu, 5, FALSE);
	if (PARS.COLOR) CheckItem (viewmenu, 6, TRUE); else CheckItem (viewmenu, 6, FALSE);
	if (PARS.WBACK) CheckItem (viewmenu, 7, FALSE); else CheckItem (viewmenu, 7, TRUE);
	if (PARS.FUNKY) CheckItem (viewmenu, 8, TRUE); else CheckItem (viewmenu, 8, FALSE);
	if (PARS.GRAB) CheckItem (actionmenu, 8, TRUE); else CheckItem (actionmenu, 8, FALSE);
	if (PARS.INFO) CheckItem (infomenu, 1, TRUE); else CheckItem (infomenu, 1, FALSE);
	if (PARS.SHANGS) CheckItem (infomenu, 2, TRUE); else CheckItem (infomenu, 2, FALSE);
	
	if (PARS.ROTYPE == 1)
		{
		SetItemMark(actionmenu, 4, '�');
		CheckItem (actionmenu, 5, FALSE);
		}
	else
		{
		SetItemMark(actionmenu, 5, '�');
		CheckItem (actionmenu, 4, FALSE);
		}
	
	if (PARS.CHIR)
		{
		SetItemMark(viewmenu, 10, '�');
		CheckItem (viewmenu, 11, FALSE);
		}
	else
		{
		CheckItem (viewmenu, 10, FALSE);
		SetItemMark(viewmenu, 11, '�');
		}
	
	if (PARS.STEREO)
		{
		CheckItem (viewmenu, 13, FALSE);
		SetItemMark(viewmenu, 14, '�');
		}
	else
		{
		SetItemMark(viewmenu, 13, '�');
		CheckItem (viewmenu, 14, FALSE);
		}
	
	for (i=1;i<=4;i++) CheckItem (stereomenu, i, FALSE);
	SetItemMark (stereomenu, PARS.KIND, '�');
	
	// dot size submenu
	for (i=1;i<=11;i++) CheckItem (dotmenu, i, FALSE);
	CheckItem (dotmenu, PARS.BIGDOTS, TRUE);
	
	// line width submenu
		for (i=1;i<=10;i++) CheckItem (linemenu, i, FALSE);
		CheckItem (linemenu, PARS.LINE, TRUE);
	
	// angle submenu
	for (i=1;i<=21;i++) CheckItem (anglemenu, i, FALSE);
	CheckItem (anglemenu, PARS.STangle+1, TRUE);
	
	EnableItem (infomenu, 1);
	EnableItem (viewmenu, 7);
	
	if (PARS.STEREO && PARS.KIND > 2)
		{
		DisableItem (stereomenu, 10);
		DisableItem (stereomenu, 11);
		DisableItem (viewmenu, 4);
		DisableItem (viewmenu, 5);
		DisableItem (viewmenu, 6);
		}
	else
		{
		EnableItem (stereomenu, 10);
		EnableItem (stereomenu, 11);
		EnableItem (viewmenu, 4);
		EnableItem (viewmenu, 5);
		EnableItem (viewmenu, 6);
		}
	
	if (N) // file is open
		{
		EnableItem(filemenu, 2); // Close
		EnableItem(filemenu, 3); // give icon
		EnableItem(filemenu, 4); // save bitmap pict
		EnableItem(filemenu, 5); // save vector pict
		EnableItem(editmenu, 2); // copy
		EnableItem(viewmenu, 0); // view menu
		EnableItem(actionmenu, 0); // action menu
		EnableItem(infomenu, 2); // show angles test
		EnableItem(infomenu, 4); // speed test
		if (PARS.STEREO) EnableItem(stereomenu, 0); else DisableItem(stereomenu, 0);
		if (hasicon) DisableItem(filemenu, 3); else EnableItem(filemenu, 3);
		if (PARS.STEREO && PARS.KIND > 2) DisableItem (filemenu, 5);
			else EnableItem (filemenu, 5);
		}
	else // no file open
		{
		DisableItem(filemenu, 2);
		DisableItem(filemenu, 3);
		DisableItem(filemenu, 4);
		DisableItem(filemenu, 5);
		DisableItem(editmenu, 2);
		DisableItem(viewmenu, 0);
		DisableItem(stereomenu, 0);
		DisableItem(actionmenu, 0);
		DisableItem(infomenu, 2); // show angles
		DisableItem(infomenu, 4); // speed test
		}
		
	DrawMenuBar();
}

//************************************************************************

void cooldot (long XX, long YY, long col, double dep, long ugh) // splat ball pixels to the Gworld
{
	static long tabi[8]={5,6,7,7,7,7,6,5};
	long i, j, k, ddd;
	long Lrb=rb, LWwidth=Wwidth, LWheight=Wheight;
	Byte *cbase, *cplane;

	if (PARS.DEPTHcue) ddd = (int)(10.0-dep*11.0);
	else ddd = 0;
	XX = XX - 3;
	YY = YY - 3;
	cbase=base+Lrb*YY + XX;
	cplane=plane+ LWwidth*YY + XX;
	
	if (XX+7 < LWwidth && XX >= 0 && YY+7 < LWheight && YY >= 0 )
		{
		for(j=0;j<=7;j++,YY++,cbase+=Lrb,cplane+=LWwidth)
			for (k=tabi[j],i=7-k;i<=k;i++)
				if (ugh >= cplane[i])
					{
					cbase[i] = Bred[col][ddd][i][j];
					cplane[i] = ugh;
					}
		}
	else
		for(j=0;j<=7;j++,YY++,cbase+=Lrb,cplane+=LWwidth) if(YY < LWheight && YY >= 0)
			for (k=tabi[j],i=7-k;i<=k;i++)
				if (XX+i < LWwidth && XX+i >= 0 && ugh >= cplane[i])
					{
					cbase[i] = Bred[col][ddd][i][j];
					cplane[i] = ugh;
					}
}

//************************************************************************

void centerimage (void)
{
	double maxX, maxY, maxZ, minX, minY, minZ, max, maybe;
	long i, j;
	struct Aray *paaa;
	
	maxX = -1; maxY = -1; maxZ = -1;
	minX = 1; minY = 1; minZ = 1;
	
	for (i = 0; i < N; i++)
		{
		paaa=aaa+i;
		if (paaa->a[0] > maxX) maxX = paaa->a[0];
		if (paaa->a[1] > maxY) maxY = paaa->a[1];
		if (paaa->a[2] > maxZ) maxZ = paaa->a[2];
		if (paaa->a[0] < minX) minX = paaa->a[0];
		if (paaa->a[1] < minY) minY = paaa->a[1];
		if (paaa->a[2] < minZ) minZ = paaa->a[2];
		}
	
	/* get maximum distance of points from origin */
	max = 0;
	for (i=0; i<N; i++)
		{
		paaa=aaa+i;
		paaa->a[0] -= (maxX + minX)/2;
		paaa->a[1] -= (maxY + minY)/2;
		paaa->a[2] -= (maxZ + minZ)/2;		
		maybe = paaa->a[0] * paaa->a[0] + paaa->a[1] * paaa->a[1] + paaa->a[2] * paaa->a[2];
		if (maybe > max) max = maybe;
		}
	max=sqrt(max);
		
	/* scale the points */
	if (max != 0) // cant scale points at origin
	for (i=0; i<N; i++)
		for (paaa=aaa+i, j=0; j<3; j++)
			paaa->a[j] = (paaa->a[j] / max) * 0.99999;
}

//************************************************************************

void maxwindowsize (void)
{
	switch (Alert(139,NULL))
		{
		case 1: // cancel
			break;
		case 2: // 200x200
			PARS.MAXwidth = 200;
			PARS.MAXheight = 200;
			writeprefs();
			break;
		case 3: // classic
			PARS.MAXwidth = 512;
			PARS.MAXheight = 332;
			writeprefs();
			break;
		case 4: // 12"
			PARS.MAXwidth = 512;
			PARS.MAXheight = 364;
			writeprefs();
			break;
		case 5: // 640x400
			PARS.MAXwidth = 640;
			PARS.MAXheight = 380;
			writeprefs();
			break;
		case 6: // 13"
			PARS.MAXwidth = 640;
			PARS.MAXheight = 460;
			writeprefs();
			break;
		case 7: // full 640x480
			PARS.MAXwidth = 640;
			PARS.MAXheight = 480;
			writeprefs();
			break;
		case 8: // portrait
			PARS.MAXwidth = 640;
			PARS.MAXheight = 850;
			writeprefs();
			break;
		case 9: // 16"
			PARS.MAXwidth = 832;
			PARS.MAXheight = 604;
			writeprefs();
			break;
		case 10: // 19"
			PARS.MAXwidth = 1024;
			PARS.MAXheight = 748;
			writeprefs();
			break;
		case 11: // 21"
			PARS.MAXwidth = 1182;
			PARS.MAXheight = 850;
			writeprefs();
			break;
		}
}

//************************************************************************

void setdpi (void)
{
	long i;
	
	i = Alert(200,NULL);
	
	switch (i)
		{
		case 1: // cancel
			break;
		case 2:
			PARS.dpi = 72;
			break;
		case 3:
			PARS.dpi = 75;
			break;
		case 4:
			PARS.dpi = 150;
			break;
		case 5:
			PARS.dpi = 288;
			break;
		case 6:
			PARS.dpi = 300;
			break;
		case 7:
			PARS.dpi = 360;
			break;
		case 8:
			PARS.dpi = 576;
			break;
		case 9:
			PARS.dpi = 600;
			break;
		case 10:
			PARS.dpi = 800;
			break;
		case 11:
			PARS.dpi = 1200;
			break;
		case 12:
			PARS.dpi = 1500;
			break;
		case 13:
			PARS.dpi = 2400;
			break;
		}
	if (i != 1) savepict(TRUE);
}

//************************************************************************

void showangles (void)
{
	Str255 tempStr;
	
	SetGWorld(drawWorldP, nil);
	
	if (!PARS.WBACK) RGBForeColor (&white);
	else RGBForeColor (&black);

	MoveTo (3, 10);
	DrawString ("\pX angle = ");	
	NumToString (Xangle, tempStr);
	DrawString (tempStr);
	
	MoveTo (3, 20);
	DrawString ("\pY angle = ");	
	NumToString (Yangle, tempStr);
	DrawString (tempStr);
	
	MoveTo (4, 30);
	DrawString ("\pZ angle = ");	
	NumToString (Zangle, tempStr);
	DrawString (tempStr);

	SetGWorld(saveWorld, saveDevice);
}

//************************************************************************








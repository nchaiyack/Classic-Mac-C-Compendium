/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*
*    Menu Handling and initialization code.
*
*	
*	Revisions:
*	7/92 Telnet 2.6 initial version: reorganized defines, put all stray globals in a struct, and
*				put all cursors in a nice array.  Moved some routines to other places -	Scott Bulmahn			
*
*/

#ifdef MPW
#pragma segment 4
#endif

#include	<stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TelnetHeader.h"
#include	"wind.h"
#include "menuseg.proto.h"
#include "mainseg.proto.h"
#include "Sets.proto.h"				/* JMB: For Saved Sets functions */
#include	"configure.proto.h"
#include "parse.proto.h"
#include "InternalEvents.h"
#include "mydnr.proto.h"
#include "rsmac.proto.h"
#include "network.proto.h"
#include "maclook.proto.h"
#include "vrrgmac.proto.h"
#include "rgmp.proto.h"
#include "tekrgmac.proto.h"
#include "vgtek.proto.h"
#include "netevent.proto.h"
#include "Connections.proto.h"

#include "vsdata.h"
#include "vsinterf.proto.h"

#include "event.proto.h"
#include "macros.proto.h"
#include "DlogUtils.proto.h"			/* For VersionNumber, OutlineItem, and DialogProc Protos */
#include "telneterrors.h"
#include "debug.h"

#include <Printing.h>
#include "printing.proto.h"
#include "menuseg.proto.h"
#include "translate.proto.h"
#include "parse.proto.h"				// For SendNAWSinfo proto

MenuHandle
	myMenus[NMENUS];		/* Menu Handles .... */

char *tempspot;				/* temporary storage ~255 bytes malloc-ed */


/*
 * External variable declarations (those which we borrow )
 *
 */
extern short scrn;
extern Cursor *theCursors[];
extern WindRec
	*screens,			/* The screen array from Maclook */
	*ftplog;					/* The FTP log screen from Maclook */
extern SysEnvRec theWorld;						/* System Environment record */
extern	short	nNational;

void CloseCaptureFile(short w)
{
	VSclosecapture(w);								/* BYU 2.4.18 */
	CheckItem(myMenus[Emul], EMcapture,FALSE);		/* BYU 2.4.18 */
}

/*	portsOpen()	- Count the number of ports open. Returns 3 different answers
 *				  0= no ports at all, 1= at least active, -1= ports/none active. */
short	portsOpen(void)
{
	short pnum;

	pnum=TelInfo->numwindows-1;
	if (pnum<0)  return(0);
	while (pnum>=0) 
//		if (!screens[pnum--].active && !screens[pnum+1].corpse) return(1);
		// corpse status now part of active flag
		if (screens[pnum--].active == CNXN_ACTIVE) return(1);
	return(-1);
}

void	AdjustMenus(void)
{
	short		i;
	WindowPtr	wind;
	
	if ((wind = FrontWindow()) != NULL && (((WindowPeek)wind)->windowKind >= userKind))
		EnableItem( myMenus[Fil],FLclose);
	else
		DisableItem( myMenus[Fil],FLclose);
		
	if ((i=portsOpen()) <1) {
		DisableItem( myMenus[Fil],FLsave);
		DisableItem( myMenus[Fil],FLprint);
		DisableItem( myMenus[Edit],EDcut);
		DisableItem( myMenus[Edit],EDundo);
		DisableItem( myMenus[Edit],EDclear);
		DisableItem( myMenus[Edit],EDcopy);
		DisableItem( myMenus[Edit],EDcopyt);


		DisableItem( myMenus[Emul],EMbs);
		DisableItem( myMenus[Emul],EMdel);
		DisableItem( myMenus[Emul],EMecho);
		DisableItem( myMenus[Emul],EMwrap);
		DisableItem( myMenus[Emul],EMscroll);
		DisableItem( myMenus[Emul],EMreset);
		DisableItem ( myMenus[Emul],EMjump);
		DisableItem ( myMenus[Emul],EMpage);
		DisableItem ( myMenus[Emul],EMclear);	/* BYU 2.4.14 */
		DisableItem ( myMenus[Emul],EMscreensize);
		DisableItem ( myMenus[Emul],EMsetup);
		DisableItem ( myMenus[Emul],EMfont);
		DisableItem ( myMenus[Emul],EMsize);
		DisableItem ( myMenus[Emul],EMcolor);
		DisableItem ( myMenus[Emul],EMcapture);	/* BYU 2.4.18 */

		DisableItem( myMenus[Net ],NEftp);
		DisableItem( myMenus[Net ],NEip);
		DisableItem( myMenus[Net ],NEayt);
		DisableItem( myMenus[Net ],NEao);
		DisableItem( myMenus[Net ],NEinter);
		DisableItem( myMenus[Net ],NEec);
		DisableItem( myMenus[Net ],NEel);
		DisableItem( myMenus[Net ],NEscroll);

		if (TelInfo->ScrlLock) {
			TelInfo->ScrlLock=0;
			CheckItem(myMenus[Net ], NEscroll,FALSE);
			}
		}
	else {
		EnableItem ( myMenus[Fil],FLsave);

/*		EnableItem ( myMenus[Fil],FLprint); */

		EnableItem ( myMenus[Emul],EMbs);
		EnableItem ( myMenus[Emul],EMdel);
		EnableItem ( myMenus[Emul],EMecho);
		EnableItem ( myMenus[Emul],EMwrap);
		if (nNational > 0) {
			EnableItem ( myMenus[National], 0);
			}
		EnableItem ( myMenus[Emul],EMscroll);
		EnableItem ( myMenus[Emul],EMreset);
		EnableItem ( myMenus[Emul],EMjump);
		EnableItem ( myMenus[Emul],EMpage);
		EnableItem ( myMenus[Emul],EMclear);	/* BYU 2.4.14 */
		EnableItem ( myMenus[Emul],EMscreensize);
		EnableItem ( myMenus[Emul],EMsetup);
		EnableItem ( myMenus[Emul],EMfont);
		EnableItem ( myMenus[Emul],EMsize);
		EnableItem ( myMenus[Emul],EMcapture);	/* BYU 2.4.18 */
	
		if (theWorld.hasColorQD) 
			EnableItem ( myMenus[Emul],EMcolor);

		EnableItem ( myMenus[Net ],NEftp);
		EnableItem ( myMenus[Net ],NEip);
		EnableItem ( myMenus[Net ],NEayt);
		EnableItem ( myMenus[Net ],NEao);
		EnableItem ( myMenus[Net ],NEinter);
		EnableItem ( myMenus[Net ],NEec);
		EnableItem ( myMenus[Net ],NEel);
		EnableItem ( myMenus[Net ],NEscroll);
	}

}

/*	switchMenus( which) - Switch from our current menus to the key menus (1)
 *						  or the normal menus (0). */
void switchMenus(short which)
{
	short i;

	DeleteMenu( fileMenu);					/* Take them from the menu bar */
	DeleteMenu( editMenu);
	DeleteMenu( termMenu);
	DeleteMenu(  netMenu);
	DeleteMenu(NfileMenu);					/* Take them from the menu bar */
	DeleteMenu(NeditMenu);
	DeleteMenu(NtermMenu);
	DeleteMenu(NnetMenu);

	if (which) {
		myMenus[Fil ] = GetMenu(NfileMenu);
		myMenus[Edit] = GetMenu(NeditMenu);
		myMenus[Emul] = GetMenu(NtermMenu);
		myMenus[Net ] = GetMenu(NnetMenu );
		DelMenuItem(myMenus[Conn],COnext);
		InsMenuItem(myMenus[Conn],(StringPtr)"\017Next Session/N\0",0);
		}
	else {
		myMenus[Fil ] = GetMenu(fileMenu);
		myMenus[Edit] = GetMenu(editMenu);
		myMenus[Emul] = GetMenu(termMenu);
		myMenus[Net ] = GetMenu( netMenu);
		DelMenuItem(myMenus[Conn],COnext);
		InsMenuItem(myMenus[Conn],(StringPtr)"\015Next Session\0",0);
		}

	for(i=1; i<Conn; i++)
		InsertMenu( myMenus[i], connMenu);	/* Put them in the menu bar */

	AdjustMenus();			/* Hilite the right stuff */

	if (TelInfo->MacBinary)
		CheckItem(myMenus[Fil],FLbin,TRUE);		/* Check MacBinary ... */
	else
		CheckItem(myMenus[Fil],FLbin,FALSE);

	if (TelInfo->ftplogon)
		CheckItem(myMenus[Fil],FLlog,TRUE);		/* and the log .... */
	else
		CheckItem(myMenus[Fil],FLlog,FALSE);

	if (TelInfo->ScrlLock) 								/* and the Suspend network ... */
		CheckItem(myMenus[Net], NEscroll,TRUE);
	else 
		CheckItem(myMenus[Net], NEscroll,FALSE);

	if (TelInfo->numwindows>0)							/* and set the BSDEL flag */
		CheckItem(myMenus[Emul], EMbs+screens[scrn].bsdel,FALSE);

	DrawMenuBar();								/* Draw what we have done */

	SetupOpSpecSubmenu(myMenus[OpSpec]);

	if (!theWorld.hasColorQD)
		DisableItem( myMenus[Emul], EMcolor);
}

/*	setupmenu - Set up (load) all menus and prepare menu bar.
 *				set default check marks. */
void setupmenu(short def)
{
	myMenus[0] = GetMenu(appleMenu);				/* Get all of our friendly menus */
	myMenus[Conn] = GetMenu(connMenu);
	myMenus[Font] = GetMenu(fontMenu);
	myMenus[Sizem] = GetMenu(sizeMenu);
	myMenus[OpSpec] = GetMenu(opspecMenu);
	myMenus[PrefsSub] = GetMenu(prefsMenu);
	myMenus[National] = GetMenu(transMenu);

	AddResMenu(myMenus[0], 'DRVR');					/* Add in the DA's */

	InsertMenu(myMenus[0], 0);						/* Insert the Menus into the bar */
	InsertMenu(myMenus[Conn], 0);
	InsertMenu(myMenus[Font], -1);
	InsertMenu(myMenus[Sizem], -1);
	InsertMenu(myMenus[OpSpec], -1);
	InsertMenu(myMenus[National], -1);
	InsertMenu(myMenus[PrefsSub], -1);

	AddResMenu(myMenus[Font], 'FONT');				/* Put the fonts in the font menu */

	SetupOpSpecSubmenu(myMenus[OpSpec]);
	switchMenus(def);
}

void CheckOpSpecSubmenu(void)
{
	SetupOpSpecSubmenu(myMenus[OpSpec]);
}

void SetupOpSpecSubmenu(MenuHandle theMenu)
{
	short			scratchshort;
		
	scratchshort = CountMItems(theMenu);
	for (; scratchshort>0; scratchshort--) DelMenuItem(theMenu, scratchshort);

	UseResFile(TelInfo->SettingsFile);
	AddResMenu(theMenu, SESSIONPREFS_RESTYPE);
}

/*	updateMenuChecks() - update the check marks for file transfer
 *						 (MacBinary) */

void updateMenuChecks( void)
{
	if (TelInfo->MacBinary)
		CheckItem(myMenus[Fil],FLbin,TRUE);		/* Check MacBinary ... */
	else
		CheckItem(myMenus[Fil],FLbin,FALSE);
}

/*	DisplayMacBinary()	- Sets the macbinary check mark according to the MacBinary flag */
void DisplayMacBinary( void)
{
	if (TelInfo->MacBinary)
		CheckItem(myMenus[Fil],FLbin,TRUE);		/* Check MacBinary ... */
	else
		CheckItem(myMenus[Fil],FLbin,FALSE);
}

/*CheckFonts()		- Place checkmarks and outlines on the appropriate
 *						  menu items for the fonts */
void CheckFonts(void)
{
	short 	i, fsiz, fnum;
	long	itemFontSize;
	Str255 	temp, itemString;

	RSgetfont( screens[scrn].vs, &fnum, &fsiz);
	GetFontName(fnum, temp);
	
	for(i=1; i<= CountMItems( myMenus[Font]); i++)
		{
		GetItem( myMenus[Font], i, itemString);
		if (EqualString(temp, itemString, FALSE, FALSE))
			CheckItem( myMenus[Font], i, TRUE);			/* Check the current font */
		else
			CheckItem( myMenus[Font], i, FALSE);
		}
	
	for(i=1; i<=CountMItems( myMenus[Sizem]); i++) {
		GetItem( myMenus[Sizem], i, itemString);			/* JMB 2.6 -- Much safer to do it */
		StringToNum(itemString, &itemFontSize);			/*				this way! */

		if (fsiz == (short)itemFontSize)
			CheckItem( myMenus[Sizem], i, TRUE);			/* Check Our Current Size */
		else
			CheckItem( myMenus[Sizem], i, FALSE);

		if (RealFont( fnum, (short)itemFontSize))		/* Outline All Available REAL Sizes */
			SetItemStyle( myMenus[Sizem], i, outline);
		else
			SetItemStyle( myMenus[Sizem], i, 0);
		}
}

/*	applAbout - display the about dialog for the application.*/
void applAbout( void)
{
	DialogPtr About;
	short itemhit;

	About=GetNewMyDialog( AboutDLOG, (Ptr) 0L,(WindowPtr) -1L, (void *)ThirdCenterDialog);
	if (About) {
		UItemAssign( About, 2, VersionNumberUPP);
		ModalDialog(NULL, &itemhit);
		DisposDialog(About);
		}
}

short ReallyClose( short scrn)
{
	DialogPtr	dtemp;
	short		item;
	Str255		scratchPstring;
	
	SetCursor(theCursors[normcurs]);

	GetWTitle(screens[scrn].wind, scratchPstring);
	ParamText(scratchPstring, NULL, NULL, NULL);
	
	dtemp = GetNewMyDialog( CloseDLOG, NULL, kInFront, (void *)ThirdCenterDialog);

	item = DLOGCancel +1;
	while (item> DLOGCancel)
		ModalDialog(DLOGwOK_CancelUPP, &item);

	DisposDialog( dtemp);

	updateCursor(1);
	
	if (item == DLOGCancel) return(0);

	return(1);
}

PicHandle RGtoPICT(short i)
{
	short j;
	PicHandle tpic;
	Rect trect;

	SetRect(&trect,0,0,384,384);
	j=VGnewwin(3,VGgetVS(i));			/* NCSA 2.5: get the right VS */
	RGMPsize( &trect );
	VGzcpy( i, j);				/* Love dat zm factr */
	tpic=OpenPicture(&trect);
	ClipRect(&trect);
	
	VGredraw(i,j);
	ClosePicture();
	VGclose(j);

	return(tpic);
}

/* 
 * copyGraph	- Copy the current graphics screen.
 *			dnum - the number of the drawing to copy .
 */
void copyGraph( short dnum)
{
	long tlong;					/* Temporary Variable */
	PicHandle tpic;				/* Mental picture of the thing */

	tpic=RGtoPICT(dnum);			/* Get the picture */
	tlong=ZeroScrap();				/* Nobody else can live here */
	HLock((Handle) tpic);					/* Lock it for Puting */
	tlong=PutScrap(GetHandleSize((Handle) tpic),'PICT', (Ptr) *tpic);	/* Store as a PICT */
	HUnlock((Handle) tpic);					/* Unlock so we can toss it */
	KillPicture(tpic);				/* Kill the picture..... */
}				

/* 
 * copyText	- Copy the current selection on the virtual screen
 *			vs - the number of the virtual screen to copy from
 */

void copyText( short vs)
{
	char **charh;			/* where to store the characters */
	long tlong;				/* Necessary temporary variable */

	tlong=ZeroScrap();		/* This Scrap aint big enough for the both of us */

	charh=RSGetTextSel(vs,0);		/* Get the text selection */

	if (charh == (char **)-1L)
		OutOfMemory(400);
	else if (charh != (char **)0L) {	/* BYU LSC - Can't do anything without characters */
		HLock(charh);				/* Lock for putting */
		tlong=PutScrap(GetHandleSize(charh),'TEXT',*charh);
									/* Put it as a TEXT resource */
		HUnlock(charh);				/* Unlock for disposal */
		DisposHandle(charh);		/* Kill the chars */
		}
}

/* 
 * copyTable   - Copy the current selection on the virtual screen
 *			vs - the number of the virtual screen to copy from
 */

void copyTable( short vs)
{
	char **charh;			/* where to store the characters */
	long tlong;				/* Necessary temporary variable */

	tlong=ZeroScrap();		/* This Scrap aint big enough for the both of us */

	charh=RSGetTextSel(vs, gApplicationPrefs->CopyTableThresh);		/* Get the text selection */

	if (charh>(char **)0L) {					/* BYU LSC - Can't do anything without characters */
		HLock(charh);				/* Lock for putting */
		tlong=PutScrap(GetHandleSize(charh),'TEXT',*charh);
									/* Put it as a TEXT resource */
		HUnlock(charh);				/* Unlock for disposal */
		DisposHandle(charh);		/* Kill the chars */
		}
	else putln("No characters to copy darn it!");
}

/*
 *	paste - Paste the resource from the scrap into the current WIND, if
 *			and only if it is really text
 */

void paste( void)
{
	long
		off,				/* offset */
		length;				/* the lenght of what is on the Scrap */

	if (screens[scrn].clientflags & PASTE_IN_PROGRESS) {  // One paste at a time, please
		SysBeep(4);
		return;
		}
		
	/* Flush the buffer if we have echo */
	if ((screens[scrn].ftpstate == 0) &&	/* BYU */
		screens[scrn].echo && 				/* BYU */
		screens[scrn].kblen>0) {			/* BYU */
		netwrite( screens[scrn].port, screens[scrn].kbbuf,
					screens[scrn].kblen);	/* if full send buffer */
		screens[scrn].kblen=0;
		}

	if (GetScrap(0L, 'TEXT', &off)<=0L)		/* If there are no TEXT res's */
			return;						/* then we can't paste it */

	screens[scrn].outhand=NewHandle(0L);	/* create a handle to put chars in */

	length= GetScrap( screens[scrn].outhand, 'TEXT',&off);
											/* Store the scrap into the handle */
	screens[scrn].outlen = length;			/* Set the length */
	HLock(screens[scrn].outhand);			/* Lock the Handle down for safety */
	screens[scrn].outptr=*screens[scrn].outhand;	/* Set the pointer */

	screens[scrn].clientflags |= PASTE_IN_PROGRESS;
	screens[scrn].incount = 0;
	screens[scrn].outcount = 0;
	
	trbuf_mac_nat((unsigned char *)screens[scrn].outptr,screens[scrn].outlen, screens[scrn].national);	/* LU: translate to national chars */

	pasteText( scrn);	/* BYU LSC - routine to paste to net, w/echo if neccessary */
}

void displayStatus(short n)
{
	DialogPtr		dptr;
	short			item;
	Str255			scratchPstring;
	
	SetCursor(theCursors[normcurs]);
	
	switch(screens[n].active) {
		case CNXN_ISCORPSE:
			GetWTitle(screens[n].wind, scratchPstring);
			ParamText( scratchPstring, "\pawaiting dismissal", NULL, NULL);
			break;
		case CNXN_OPENING:
			ParamText( screens[n].machine,"\pbeing opened", NULL, NULL);
			break;
		default:
			ParamText( screens[n].machine,"\pbeing looked up", NULL, NULL);
		}

	dptr = GetNewMyDialog( StatusDLOG, NULL, kInFront, (void *)ThirdCenterDialog);

	item = DLOGCancel+1;
	while (item > DLOGCancel)
		ModalDialog(DLOGwOK_CancelUPP, &item);

	updateCursor(1);
	
	if (item == DLOGCancel) {
		netclose(screens[n].port);
		destroyport( n);
		}	

	DisposDialog(dptr);
}

/*
 *	changeport - handle the menu updates for changing from one port to another
 */
void changeport(short oldprt, short newprt)
{

	sprintf(tempspot,"oldscrn: %d, newscrn: %d",oldprt,newprt); putln(tempspot);
	
	if (screens[oldprt].active == CNXN_ACTIVE) 
		CheckItem(myMenus[Conn], oldprt + FIRST_CNXN_ITEM, FALSE);		/* Adjust Conn menu */
	CheckItem(myMenus[Conn], newprt + FIRST_CNXN_ITEM, TRUE);

	CheckItem(myMenus[Emul], EMbs,FALSE);						/* Adjust BS */
	CheckItem(myMenus[Emul], EMdel,FALSE);
	CheckItem(myMenus[Emul], EMbs+screens[newprt].bsdel,TRUE);	/* and DEL */

	if (screens[newprt].tektype < 0) {	// TEK is inhibited
		DisableItem(myMenus[Emul],EMclear);
		DisableItem(myMenus[Emul],EMpage);
		}
	else {
		EnableItem(myMenus[Emul],EMclear);
		EnableItem(myMenus[Emul],EMpage);
	
		if (screens[newprt].tekclear)				/* BYU 2.4.8 */
			CheckItem(myMenus[Emul],EMclear,TRUE);	/* BYU 2.4.8 */
		else 										/* BYU 2.4.8 */
			CheckItem(myMenus[Emul],EMclear,FALSE);	/* BYU 2.4.8 */
		}
		
	if (screens[newprt].ESscroll)
		CheckItem(myMenus[Emul],EMscroll,TRUE);
	else 
		CheckItem(myMenus[Emul],EMscroll,FALSE);

	if (screens[newprt].echo) 					/* LOCAL ECHO */
		CheckItem(myMenus[Emul],EMecho,TRUE);
	else 										/* REMOTE ECHO */
		CheckItem(myMenus[Emul],EMecho,FALSE);
		
	if (screens[newprt].wrap) 					/* wrap on */
		CheckItem(myMenus[Emul],EMwrap,TRUE);
	else 										/* wrap off */
		CheckItem(myMenus[Emul],EMwrap,FALSE);

	if (VSiscapturing(screens[newprt].vs))				/* BYU 2.4.18 */
		CheckItem(myMenus[Emul], EMcapture,TRUE);		/* BYU 2.4.18 */
	else												/* BYU 2.4.18 */
		CheckItem(myMenus[Emul], EMcapture,FALSE);		/* BYU 2.4.18 */

	if (screens[newprt].arrowmap)						/* JMB */
		CheckItem(myMenus[Emul],EMarrowmap, TRUE);		/* JMB */
	else												/* JMB */
		CheckItem(myMenus[Emul],EMarrowmap, FALSE);		/* JMB */
		
	if (screens[newprt].pgupdwn)						/* JMB */
		CheckItem(myMenus[Emul],EMpgupdwn, TRUE);		/* JMB */
	else												/* JMB */
		CheckItem(myMenus[Emul],EMpgupdwn, FALSE);		/* JMB */
	
	scrn=newprt;

	CheckFonts();

	CheckNational(screens[newprt].national);
}

// Returns TRUE if the user cancelled the quit
Boolean	HandleQuit(void)
{
	short	i;
	
	if (TelInfo->numwindows>0) {
		if (AskUserAlert(REALLY_QUIT_QUESTION, FALSE)) {
			for (i = TelInfo->numwindows - 1; i >= 0; i--) {
				netclose(screens[i].port);
				destroyport(i);
				}
			}
		else
			return (TRUE);
	  }
	  
	quit();
	return (FALSE);
}

/*
 *	HandleMenuCommand - preform a command denoted by the arguments.
 *		mResult - the result of the menu event
 *		modifiers- modifiers from the menu event
 */

void HandleMenuCommand( long mResult, short modifiers)
{
	register short i;
	short theItem, theMenu;

	theMenu = mResult >> 16;
	theItem = mResult & 0xffff;
	switch(theMenu) {
	case appleMenu:
		if (theItem==1)				/* About Dialog */
			applAbout();
		else
		  {
			Str255
				name;
			GetItem(myMenus[0], theItem, name);		/* Desk accessory */
			OpenDeskAcc(name);
		  }
		break;

	case fileMenu:
	case NfileMenu:
		switch(theItem) {			
			
		case FLopen:
			PresentOpenConnectionDialog();					/* Open a connection */				
			break;

		case FLclose:
//			if ( TelInfo->numwindows<1 ) break;				/* Close a connection */
			CloseAWindow(FrontWindow());
#if 0
			if ( screens[scrn].active == CNXN_ISCORPSE) {
				netclose( screens[scrn].port);
				destroyport( scrn);
				}
			else {
				if ( !ReallyClose( scrn) ) break;
				netclose(screens[scrn].port);
				removeport( scrn);
				}
			if (TelInfo->numwindows <1) 
				{
				DisableItem( myMenus[Conn],0);
				DisableItem(myMenus[Conn],COnext);
				DrawMenuBar();
				}
#endif
			break;

		case FLload:								/* Load a set */
			LoadSet();
			break;
		case FLsave:								/* Save a set */
			if (TelInfo->numwindows<1) break;
			SaveSet();
			break;
		case FLbin:									/* Toggle MacBinary on/off */
			TelInfo->MacBinary = !TelInfo->MacBinary;
			if (TelInfo->MacBinary) {
				CheckItem(myMenus[Fil], FLbin,TRUE);
				}
			else {
				CheckItem(myMenus[Fil], FLbin,FALSE);
				}
			break;

		case FLlog:										/* Toggle FTP window on/off*/
			TelInfo->ftplogon=!TelInfo->ftplogon;
			if(TelInfo->ftplogon) {
				CheckItem(myMenus[Fil],FLlog,TRUE);
				RSshow(ftplog->vs);
				SelectWindow(ftplog->wind);
				}
			else {
				CheckItem(myMenus[Fil],FLlog,FALSE);
				RShide(ftplog->vs);
				}
			break;

		case FLprint:								/* Print Selection (or gr) */
			PrintSelection();
			break;

		case FLpset:									/* Set up for printer */
			PrintPageSetup();
			break;

		case FLquit:
			(void) HandleQuit();
			break;

		}
		break;

	case editMenu:
	case NeditMenu:
		if (!SystemEdit(theItem-1)) {				/* Is this mine? */
			switch(theItem) {
			case EDcopy:							/* Copy */
				i = MacRGfindwind(FrontWindow());	/* is ICR window? */
				if (i >= 0)
					MacRGcopy(FrontWindow());		/* copy the ICR window */
				else {
					i=RGgetdnum(FrontWindow());
					if (i>-1)						/* Copy Graphics */
						copyGraph( i);
					else							/* Copy Text */
						if ( (i=RSfindvwind(FrontWindow())) >-1)
							copyText( i);
				}
				break;

			case EDcopyt:							/* Copy Table */
				/* 
				*  tech note #180 trick to get MultiFinder to pay attention 
				*/
				if (!SystemEdit(EDcopy-1)) {		/* tell it we did a copy */
					i=RGgetdnum(FrontWindow());
					if (i>-1)			/* Copy Graphics */
						copyGraph( i);
					else				/* Copy Text */
						if ( (i=RSfindvwind(FrontWindow())) >-1)
							copyTable( i);
				}
				break;

			case EDpaste:							/* Paste */
				if (TelInfo->numwindows<1)
						break;
					else paste();			/* Paste if there is a wind to do to*/
				break;
			case EDmacros:							/* Set them Macros */
				Macros();
				break;
			default:
				break;
			}
		}
		break;

	case connMenu:
	case NconnMenu:
		if (theItem == COnext) {
			short	scratchshort;
			if (TelInfo->numwindows >1) {
				scratchshort = WindowPtr2ScreenIndex(FrontWindow()) + 1;
				// Skip over inactive connections
				while(	(screens[scratchshort].active != CNXN_ACTIVE) &&
						(screens[scratchshort].active != CNXN_ISCORPSE) &&
						(scratchshort <= TelInfo->numwindows+1))						
					scratchshort++;
					
				if ((scratchshort < 0) || (scratchshort >= TelInfo->numwindows))
					scratchshort = 0;
				SelectWindow(screens[scratchshort].wind);
				}
			break;
			}
		
		if (theItem == COtitle)	{
			ChangeWindowName(FrontWindow());
			break;
			}
			
		if (theItem >= FIRST_CNXN_ITEM) {
			if ((theItem - FIRST_CNXN_ITEM-1)>(TelInfo->numwindows+1)) break;  /* rotten danish */
			if (screens[theItem - FIRST_CNXN_ITEM].active != CNXN_ACTIVE) {
				displayStatus(theItem - FIRST_CNXN_ITEM);	/* Tell them about it..... */
				break;
				}
			else {
				HiliteWindow(screens[scrn].wind, FALSE);
				changeport(scrn,(theItem - FIRST_CNXN_ITEM));
				if (!(modifiers &  optionKey)) SelectWindow(screens[scrn].wind);
				else HiliteWindow(screens[scrn].wind, TRUE);
				}
			}
		break;

	case netMenu:
	case NnetMenu:
		switch(theItem) {
		case NEftp:										/* Send FTP command */
		case NEip:										/* Send IP Number */
			if (TelInfo->numwindows<1) break;
			{	char tmpout[30];						/* Basically the same except for */
				unsigned char tmp[4];					/* The ftp -n phrase in NEftp */

				if (screens[scrn].echo && (screens[scrn].kblen>0)) {
					netwrite( screens[scrn].port, screens[scrn].kbbuf,
								screens[scrn].kblen);/* if not empty send buffer */
					screens[scrn].kblen=0;
					}
				netgetip(tmp);
				if (theItem == NEftp) {
					if ((gFTPServerPrefs->ServerState == 1) && !(modifiers & shiftKey))
						sprintf(tmpout,"ftp -n %d.%d.%d.%d\015\012",tmp[0],tmp[1],tmp[2],tmp[3]);
					else
						sprintf(tmpout,"ftp %d.%d.%d.%d\015\012",tmp[0],tmp[1],tmp[2],tmp[3]);
					}
				else
					sprintf(tmpout,"%d.%d.%d.%d",tmp[0],tmp[1],tmp[2],tmp[3]);
				netwrite(screens[scrn].port,tmpout,strlen(tmpout));
				if (screens[scrn].echo)
					VSwrite(screens[scrn].vs,tmpout, strlen(tmpout));
			}
			break;

		case NEayt:								/* Send "Are You There?"*/
			if (TelInfo->numwindows<1) break;
			netpush(screens[scrn].port);
			netwrite(screens[scrn].port, "\377\366",2);
			break;

		case NEao:								/* Send "Abort Output"*/
			if (TelInfo->numwindows<1) break;
			netpush(screens[scrn].port);
			netwrite(screens[scrn].port, "\377\365",2);
			screens[ scrn].timing = 1;						/* set emulate to TMwait */
			netwrite(screens[scrn].port, "\377\375\006",3);		/* send TM */
			break;

		case NEinter:								/* Send "Interrupt Process"*/
			if (TelInfo->numwindows<1) break;
			netpush(screens[scrn].port);
			netwrite(screens[scrn].port, "\377\364",2);
			screens[ scrn].timing = 1;						/* set emulate to TMwait */
			netwrite(screens[scrn].port, "\377\375\006",3);		/* send TM */
			break;

		case NEec:								/* Send "Erase Character"*/
			if (TelInfo->numwindows<1) break;
			netpush(screens[scrn].port);
			netwrite(screens[scrn].port, "\377\367",2);
			break;

		case NEel:								/* Send "Erase Line"*/
			if (TelInfo->numwindows<1) break;
			netpush(screens[scrn].port);
			netwrite(screens[scrn].port, "\377\370",2);
			break;
			
		case NEscroll:							/* Suspend Network */
			TelInfo->ScrlLock=!TelInfo->ScrlLock;
			if (TelInfo->ScrlLock) 
				CheckItem(myMenus[Net], NEscroll,TRUE);
			else 
				CheckItem(myMenus[Net], NEscroll,FALSE);
			break;

		case NEnet:								/* Show Network Numbers */
			showNetNumbers();
			break;

		default:
			break;
		}
		break;

	case termMenu:
	case NtermMenu:
		switch(theItem) {

		case EMbs:								/* Backspace for backspace  */
			if (TelInfo->numwindows<1) break;
			CheckItem(myMenus[Emul], EMbs+screens[scrn].bsdel,FALSE);
			screens[scrn].bsdel=0;
			CheckItem(myMenus[Emul], EMbs+screens[scrn].bsdel,TRUE);
			break;
		
		case EMdel:								/* Delete for backspace */
			if (TelInfo->numwindows<1) break;
			CheckItem(myMenus[Emul], EMbs+screens[scrn].bsdel,FALSE);
			screens[scrn].bsdel=1;
			CheckItem(myMenus[Emul], EMbs+screens[scrn].bsdel,TRUE);
			break;

		case EMecho:								/* Toggle Local Echo (if poss.) */
			if (TelInfo->numwindows < 1) break;
			if (screens[scrn].echo && (screens[scrn].kblen>0)) {
				netwrite( screens[scrn].port, screens[scrn].kbbuf,
							screens[scrn].kblen);	/* if not empty send buffer */
				screens[scrn].kblen=0;
				}
			screens[scrn].echo= !screens[scrn].echo;	/* toggle */
			if (screens[scrn].echo) {					/* LOCAL ECHO */
				netwrite(screens[scrn].port,"\377\376\001",3);
				CheckItem(myMenus[Emul],EMecho,TRUE);
				}
			else {										/* REMOTE ECHO */
				netwrite(screens[scrn].port,"\377\375\001",3);
				CheckItem(myMenus[Emul],EMecho,FALSE);
				}
			break;
			
		case EMwrap:								/* wrap mode */
			if (TelInfo->numwindows < 1) break;
			if (!screens[scrn].wrap) {				/* is off, turn on */
				screens[scrn].wrap = 1;
				CheckItem( myMenus[Emul],EMwrap, TRUE);
				VSwrite(screens[scrn].vs, "\033[?7h",5);	/* kick emulator */
				}
			else {
				screens[scrn].wrap = 0;
				CheckItem( myMenus[Emul],EMwrap, FALSE);
				VSwrite(screens[scrn].vs, "\033[?7l",5);
				}
			break;
		
		case EMarrowmap:										/* JMB */
			if (TelInfo->numwindows < 1) break;					/* JMB */
			screens[scrn].arrowmap = !screens[scrn].arrowmap;	/* JMB */
			if (screens[scrn].arrowmap)							/* JMB */
				CheckItem( myMenus[Emul], EMarrowmap, TRUE);	/* JMB */
			else												/* JMB */
				CheckItem( myMenus[Emul], EMarrowmap, FALSE);	/* JMB */
			break;												/* JMB */
			
		case EMpgupdwn:
			if (TelInfo->numwindows < 1) break;					/* JMB */
			screens[scrn].pgupdwn = !screens[scrn].pgupdwn;		/* JMB */
			if (screens[scrn].pgupdwn)							/* JMB */
				CheckItem( myMenus[Emul], EMpgupdwn, TRUE);		/* JMB */
			else												/* JMB */
				CheckItem( myMenus[Emul], EMpgupdwn, FALSE);	/* JMB */
			break;												/* JMB */
			
		case EMscroll:										/* Scrollback on CLS */
			if (TelInfo->numwindows<1) break;
			screens[scrn].ESscroll = !screens[scrn].ESscroll;
			VSscrolcontrol( screens[scrn].vs, -1, screens[scrn].ESscroll);
			if (screens[scrn].ESscroll)
				CheckItem(myMenus[Emul],EMscroll, TRUE);
			else
				CheckItem(myMenus[Emul],EMscroll, FALSE);
			break;
			
		case EMpage:									/* TEK page command */
			if (TelInfo->numwindows<1) break;
			parse( &screens[scrn],  (unsigned char *) "\033\014",2);	/* BYU LSC */
			break;

		case EMclear:								/* BYU 2.4.8 - Clear on TEK page */
			if (TelInfo->numwindows<1) break;
			screens[scrn].tekclear = !screens[scrn].tekclear;
			if (screens[scrn].tekclear)
				CheckItem(myMenus[Emul],EMclear, TRUE);
			else
				CheckItem(myMenus[Emul],EMclear, FALSE);
			break;

		case EMscreensize:
			if (TelInfo->numwindows<1) break;		/* NCSA: SB */
			SetScreenDimensions((short)scrn);		/* NCSA: SB */
			break;

		case EMreset:									/* Reset Screen */
			if (TelInfo->numwindows<1) break;
			VSreset(screens[scrn].vs);					/* Reset it */
			screens[scrn].timing=0;
			screens[scrn].wrap = 0;						/* turns wrap off */
			CheckItem( myMenus[Emul],EMwrap, FALSE);
			break;
		case EMjump:									/* Jump Scroll */
			if (TelInfo->numwindows<1) break;
			FlushNetwork(scrn);							/* Flush it */
			break;
			
		case EMsetup:							/* need dialog to enter new key values */
			setupkeys();
			break;

		case EMcolor:										/* Set color */
			if (TelInfo->numwindows<1) break;
			if (theWorld.hasColorQD)
				RScprompt(screens[scrn].vs);	/* Color it */
			break;

		case EMcapture:											/* BYU 2.4.18 - Capture session to file */
			if (VSiscapturing(screens[scrn].vs)) {				/* BYU 2.4.18 */
				CloseCaptureFile(screens[scrn].vs);				/* BYU 2.4.18 */
			} else {											/* BYU 2.4.18 */
				if(VSopencapture(scrn, screens[scrn].vs))		/* BYU 2.4.18 */
					CheckItem(myMenus[Emul], EMcapture,TRUE);	/* BYU 2.4.18 */
			}													/* BYU 2.4.18 */

			break;												/* BYU 2.4.18 */

		default:
			break;
		}
		break;
	case fontMenu:
		if (TelInfo->numwindows>0) {
			short 	itemFontNum;
			Str255 	temp;
	
			GetItem( myMenus[Font], theItem, temp);
			GetFNum( temp, &itemFontNum);

			RSchangefont( screens[scrn].vs, itemFontNum, 0, 1);
			CheckFonts();
			}
		break;
	case sizeMenu:
		if (TelInfo->numwindows>0) {
			long	itemFontSize;
			Str255 	temp;
			
			GetItem( myMenus[Sizem], theItem, temp);	/* JMB 2.6 -- Much safer to do it */
			StringToNum(temp, &itemFontSize);		/*				this way! */

			RSchangefont( screens[scrn].vs, -1, itemFontSize, 1);
			CheckFonts();
			}
		break;
	case opspecMenu:											// JMB
		OpenPortSpecial(myMenus[OpSpec], theItem);				// JMB
		break;													// JMB
	case prefsMenu:
		switch(theItem) {
			case prfGlobal:
				Cenviron();
				break;
			case prfFTP:
				Cftp();
				break;
			case prfSess:
				EditConfigType(SESSIONPREFS_RESTYPE, &EditSession);
				CheckOpSpecSubmenu();
				break;
			case prfTerm:
				EditConfigType(TERMINALPREFS_RESTYPE, &EditTerminal);
				break;
			case prfFTPUser:
				EditConfigType(FTPUSER, &EditFTPUser);
			}
		break;
	case transMenu:
		if (TelInfo->numwindows>0) {
			CheckNational(theItem-1);		// Set up the menu
			transBuffer(screens[scrn].national, theItem-1);	// Translate the scrollback buffer
			// and redraw the screen
			VSredraw(screens[scrn].vs,0,0,VSmaxwidth(screens[scrn].vs),VSgetlines(screens[scrn].vs)-1);	/* LU */
			screens[scrn].national = theItem-1;
			}
		break;
	default:
		break;
	
	}
	HiliteMenu(0);
  } /* HandleMenuCommand */

//	Take the user's new translation choice and make sure the proper tables exist to do 
//		the translations.  If there is a problem, return the default translation as the chouce.
void	CheckNational(short choice)
{
	short i;
			
	for(i=1; i<=(nNational+1);i++)
		if ((choice+1) == i)					/* Check the Current NatLang */
			CheckItem( myMenus[National], i, TRUE);
		else
			CheckItem( myMenus[National], i, FALSE);

}

/*
 *	extractmenu - remove a connection from the menu.
 */

void extractmenu(short screen)
{
	DelMenuItem(myMenus[Conn], screen + FIRST_CNXN_ITEM);
	AdjustMenus();
}

/*
 *	addinmenu - add a connection's name to the menu in position pnum. (name is
 *		an str255 pointed at by temps).
 */

void addinmenu( short screen, Str255 temps, char mark)
{
	InsMenuItem(myMenus[Conn], "\pDoh", (screen-1) + FIRST_CNXN_ITEM);
	SetItem(myMenus[Conn], screen + FIRST_CNXN_ITEM, temps);		// Avoid metas
	SetItemMark( myMenus[Conn], screen + FIRST_CNXN_ITEM, mark);
	AdjustMenus();
}

/*	Set the item mark for <scrn> to opening connection */
void SetMenuMarkToOpeningForAGivenScreen( short scrn)
{
	unsigned char c=0xa5;

	SetItemMark( myMenus[Conn], scrn + FIRST_CNXN_ITEM, c );
}

/*	Set the item mark for <scrn> to opened connection */
void SetMenuMarkToLiveForAGivenScreen( short scrn)
{
	SetItemMark( myMenus[Conn], scrn + FIRST_CNXN_ITEM, noMark);
	AdjustMenus();
}

void DoTheMenuChecks(void)
{
	short	active;
	short	windownum;

	if (TelInfo->numwindows>0)
		{
		EnableItem( myMenus[Conn],0);
		DrawMenuBar();
		}

	else 
		{
		DisableItem(myMenus[Conn],0);
		DrawMenuBar();
		}

	active =0;
	
	for (windownum=0;windownum<TelInfo->numwindows;windownum++)
		if (screens[windownum].active == CNXN_ACTIVE) active++;
		
	if (active<2)	
		DisableItem(myMenus[Conn],COnext);
	else EnableItem(myMenus[Conn],COnext);

	if (!active)
		{
/*		DisableItem(myMenus[Edit],EDcopy);
		DisableItem(myMenus[Edit],EDcopyt); */
		DisableItem(myMenus[Edit],EDpaste);
		DisableItem(myMenus[Emul],0);
		DrawMenuBar();
		}
	else 
		{
/*		EnableItem(myMenus[Edit],EDcopy);
		EnableItem(myMenus[Edit],EDcopyt); */
		EnableItem(myMenus[Edit],EDpaste);
		EnableItem( myMenus[Emul],0);
		DrawMenuBar();
		}
}



/*--------------------------------------------------------------------------*/
/* SetupMenusForSelection													*/
/* If there is a selection on screen, then let the user copy and print.		*/
/* If not, then, oh well....just disable the menus and forget about it		*/
/* ...and to think that this good stuff USED to be in rsmac.c. 				*/
/* This is called from RSselect after the user clicks in the window, and 	*/
/* was moved here for modularity	- SMB									*/
/*--------------------------------------------------------------------------*/
void SetMenusForSelection (short selected)					/* NCSA: SB */	
{															/* NCSA: SB */
	if (!selected)											/* NCSA: SB */
		{													/* NCSA: SB */
		DisableItem(myMenus[Fil],FLprint);					/* NCSA: SB */
		DisableItem(myMenus[Edit],EDcopy);					/* NCSA: SB */
		DisableItem(myMenus[Edit],EDcopyt);					/* NCSA: SB */
		}													/* NCSA: SB */
	else 													/* NCSA: SB */
		{													/* NCSA: SB */
		EnableItem(myMenus[Fil],FLprint);					/* NCSA: SB */
		EnableItem(myMenus[Edit],EDcopy);					/* NCSA: SB */
		EnableItem(myMenus[Edit],EDcopyt);					/* NCSA: SB */
		}													/* NCSA: SB */
}															/* NCSA: SB */


/*----------------------------------------------------------------------*/
/* NCSA: SB - SetColumnWidth											*/
/*	Allow the user to FINALLY pick how many columns he wants on the 	*/
/*	screen.  Set up a dialog box to pick the # of columns, and then		*/
/*	size-up the Telnet screen accordingly.  NOTE: The user still needs	*/
/* 	to do a "resize", unless he is using NAWS							*/
/*----------------------------------------------------------------------*/
void SetScreenDimensions(short scrn)
{
	DialogPtr	dtemp;
	Str255		ColumnsSTR, LinesSTR;
	long		columns, lines;
	short		ditem, notgood;
	
	dtemp=GetNewMyDialog( SizeDLOG, NULL, kInFront, (void *)ThirdCenterDialog); 
	
	SetCursor(theCursors[normcurs]);

	notgood = 1;
	lines = VSgetlines(screens[scrn].vs);
	columns = VSgetcols(screens[scrn].vs) + 1;
	
	while (notgood) {
		notgood = 0;								/* Default to good */
		NumToString(columns, ColumnsSTR);
		NumToString(lines, LinesSTR);
		SetTEText(dtemp, ColumnsNumber, ColumnsSTR);
		SetTEText(dtemp, LinesNumber, LinesSTR);
		SelIText( dtemp, ColumnsNumber, 0, 32767);
	
		ditem = 3;
		while(ditem>2)
			ModalDialog(DLOGwOK_CancelUPP, &ditem);
	
		if (ditem == DLOGCancel) {
			DisposeDialog( dtemp);
			return;
			}
		
		GetTEText(dtemp, ColumnsNumber, ColumnsSTR);
		StringToNum(ColumnsSTR, &columns);
		GetTEText(dtemp, LinesNumber, LinesSTR);
		StringToNum(LinesSTR, &lines);
		
		if (columns < 10) {
			columns = 10;
			notgood = 1;
			}
		else if (columns > 132) {
			columns = 132;
			notgood = 1;
			}

		if (lines < 10) {
			lines = 10;
			notgood = 1;
			}
		else if (lines > 200) {
			lines = 200;
			notgood = 1;
			}
		
		if (notgood) SysBeep(4);
	}
	
	DisposeDialog( dtemp);
	
	VSsetlines( screens[scrn].vs, lines);
	RScalcwsize( screens[scrn].vs, columns);
/*	RSsize( screens[scrn].wind, NULL, 0); */

	if (screens[scrn].naws)
		SendNAWSinfo(&screens[scrn], (short)columns, (short)lines);
		
	updateCursor(1);
}

void	ChangeWindowName(WindowPtr	theWindow)
{
	DialogPtr	dptr;
	short		itemHit;
	Str255		theName;

	if( theWindow != NULL) {
		InitCursor();
		dptr = GetNewMySmallDialog(WinTitlDLOG, NULL, kInFront, (void *)ThirdCenterDialog );

		GetWTitle(theWindow, theName); 
		SetTEText( dptr, kWinNameTE, theName);
		SelIText( dptr, kWinNameTE, 0, 250 );

		itemHit = 0;
		while(itemHit != DLOGOk && itemHit != DLOGCancel)
			ModalDialog(DLOGwOK_CancelUPP, &itemHit);
		
		if(itemHit == DLOGOk) {
			GetTEText(dptr, kWinNameTE, theName);
			set_new_window_name(theName, theWindow);
			}
			
		DisposDialog(dptr);
		}
}

void	set_new_window_name(Str255 theName, WindowPtr theWindow)
{
    short	i;
    
	if(theName[0]) {
		i = WindowPtr2ScreenIndex(theWindow);
		if (i >= 0) {
			i += FIRST_CNXN_ITEM;
			SetWTitle(theWindow, theName);
			SetItem(myMenus[Conn], i, theName);
			}
		}
}

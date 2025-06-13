/*
*	init.c
*	Top level of init code & general initialization routines
*	Called only by main()
*
*****************************************************************
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
*  Revisions:
*  7/92		Telnet 2.6:	Initial version.  Jim Browne
*/

#ifdef MPW
#pragma segment INIT
#endif

#include "TelnetHeader.h"

#include <string.h>
#include <Folders.h>
#include <Script.h>
#include <stdio.h>
#include <GestaltEqu.h>
#include <AppleEvents.h>

#include "init.proto.h"
#include "debug.h"
#include "mainseg.proto.h"		/* For forcequit() proto */
#include "iNetwork.proto.h"	/* For initnet proto */
#include "macros.proto.h"		/* For setmacro proto */
#include "wind.h"		/* For WindRec definition */
#include "rsmac.proto.h"		/* For RSgetwindow & RSnewwindow protos */
#include "DlogUtils.proto.h"	/* For VersionNumber, UItemAssign, GetNewMyDialog protos */
#include "Sets.proto.h"		// For readconfig proto
#include "prefs.proto.h"
#include "vr.h"			/* For VRinit proto */
#include "vsdata.h"
#include "vsinterf.proto.h"	/* For VSinit proto */
#include "vgtek.proto.h"		/* For VGinit proto */
#include "vrrgmac.proto.h"	/* For MacRGinit proto */
#include "telneterrors.h"		/* for DoError proto */
#include "tekrgmac.proto.h"	/* For RGMalloc proto */
#include "menuseg.proto.h"
#include "translate.proto.h"
#include "dnr.proto.h"	// For OpenResolver proto
#include "desproto.h"
#include "ae.proto.h"			// For AppleEvent handlers
#include "MacBinary.h"
#include "binsubs.proto.h"		// For init_mb_files proto


/*--------------------------------------------------------------------------------*/
#define CurrentVersion 1				/* Last known SysEnvirons version */
#define HFSPtr ((long *)1014)

extern	char *tempspot;				/* for temp storage only */

extern	unsigned char
	*macro[10];		/* the wonderful macro package .... */

extern	Cursor *theCursors[];

extern	WindRec
	*screens,		/* Window Records (VS) for :	Current Sessions */
	*ftplog;			/*								FTP Log			 */
extern	MenuHandle	myMenus[];

OSErr		io;
char		*mungbuf;
SysEnvRec	theWorld;						/* System Environment record */
short		TempItemsVRefNum;
long		TempItemsDirID;
Boolean		gKeyboardHasControlKey, gAEavail;

Boolean encryptOK;				// true if des encryption routines available
Boolean authOK;					// true if authentication driver available

/*	checkCONF - check if the user clicked on a configuration file */
void checkCONF( void)
{
	short	i,mess,count;
	AppFile	theFile;
	FSSpec	FileSpec;
	long	junk = 0;

	if (gAEavail) return;	// If AppleEvents are available, we'll get ODOCs

#ifndef __powerpc__
	CountAppFiles(&mess,&count);
	if (mess==1) return;
	if (count<1) return;
	for(i=1;i<=count;i++) {
		GetAppFiles(i, &theFile);
		if (theFile.fType=='CONF') {
			ClrAppFiles(i);
			BlockMove(&theFile.fName, FileSpec.name, (*theFile.fName)+1); // pstring copy theFile.fName -> FileSpec.name
			GetWDInfo(theFile.vRefNum, &FileSpec.vRefNum, &FileSpec.parID, &junk);
			readconfig(FileSpec);	/* BYU LSC */
			}
		}
#endif
	return;
}

void initmacros( void)
{
	short i;

	for (i=0; i<10 ; i++) {
		macro[i] = (unsigned char *) NewPtrClear(256);		/* BYU LSC */
		if (macro[i]== NULL) 
			{			/* Cant allocate space for the Macros */
			DoError(102 | MEMORY_ERRORCLASS, LEVEL3,NULL);	
			forcequit();
			}
		setmacro( i, (unsigned char *) "");		/* BYU LSC */
		}
}

void initEvents( void)
{
	EventRecord myEvent;

	FlushEvents(everyEvent - diskEvt,0);	/* Don't Let 'em get off without seeing it. */

	WaitNextEvent(everyEvent, &myEvent, gApplicationPrefs->TimeSlice, 0L);	/* WNE always avail */
	WaitNextEvent(everyEvent, &myEvent, gApplicationPrefs->TimeSlice, 0L);
}

/* initftplog - initialize the FTP log window */
void initftplog( void)
{
	Rect prect;

	SetRect(&prect, 300,256,512,384);

	ftplog = (WindRec *) NewPtrClear(sizeof(WindRec));
	
	ftplog->vs=RSnewwindow( &prect, 50, 80, 24,
					"\pFTP Log", 1, DefFONT, DefSIZE, gFTPServerPrefs->ShowFTPlog,
						1,0);	/* NCSA 2.5 */

	ftplog->wind = RSgetwindow( ftplog->vs);
	((WindowPeek)ftplog->wind)->windowKind = WIN_LOG;
	
	VSwrite(ftplog->vs,"\033[24;0H",7);		/* Move to bottom of screen */
	ftplog->active=0; 						/* Meaningless */
	ftplog->port=0;							/* As is this */
	ftplog->termstate=VTEKTYPE;
}

  /* copies a specified cursor from a resource into a nonrelocatable block
	and returns a pointer to it in *result. */
void loadcursor( short CursorID, CursPtr *result)
{
	CursHandle tempcurs;
	OSErr Err;

	*result = nil;
	do /* once */
	  {
		tempcurs = GetCursor(CursorID);
		Err = ResError();
		if (Err != noErr)
			break;
		*result = (CursPtr) NewPtr(sizeof(Cursor));		/* BYU LSC */
		Err = MemError();
		if (Err != noErr)
			break;
		LoadResource((Handle) tempcurs); /* in case it was purged */
		Err = ResError();
		if (Err != noErr)
			break;
		BlockMove((Ptr) *tempcurs, (Ptr) *result, sizeof(Cursor));
	  }
	while (false);
	if (Err != noErr)
	  {
		if (*result != nil)
			DisposPtr((Ptr) *result);
		*result = &qd.arrow;
	  }
} /* loadcursor */

/* initmcurs - Set up the mouse Cursors
 *		NOTE: debug tells if the bug or the pointer is to be used as dflt.*/
void initmcurs ( short debug) 
{
	loadcursor(rWatchCursor, &theCursors[watchcurs]);
	loadcursor(rCrossCursor, &theCursors[graphcurs]);
	loadcursor(rPOSCURS, &theCursors[poscurs]);
	loadcursor(rBeamCursor, &theCursors[textcurs]);
	loadcursor(rGINCURS, &theCursors[gincurs]);
	loadcursor(rXferCURS, &theCursors[xfercurs]);
	if (debug)
		loadcursor(rDbugCURS, &theCursors[normcurs]);
	else
		theCursors[normcurs] = &qd.arrow;
	SetCursor(theCursors[normcurs]);
}

/* 	If a file with the filetype "DBUG" was sent to us, set the debug mode. */
short checkdebug( void)
{
	short i,mess,count;
	AppFile theFile;
	long junk = 0;

	if (gAEavail) return(FALSE);	// 	Don't use CountAppFiles if AppleEvents
									//	are available.
#ifndef __powerpc__	
	CountAppFiles(&mess,&count);
	if (mess==1) return(FALSE);
	if (count<1) return(FALSE);
	for(i=1;i<=count;i++) {
		GetAppFiles(i, &theFile);
		if (theFile.fType=='DBUG') {
			ClrAppFiles(i);
			return(TRUE);
			}
		}
	return(FALSE);
#endif
}

void InquireEnvironment( void)
{
	OSErr	err;
	Boolean HFSflag;
	long	response;
	
	long *HFSp=(long *)1014L;
	
	HFSflag= (*HFSp) > 0L;
	if (!HFSflag) FatalAlert(NEED_HFS_ERR, 0, 0);

	err = SysEnvirons(CurrentVersion, &theWorld);
	
	if (err == envVersTooBig) FatalAlert(SYS_ENVIRON_ERR, 0, 0);
	
	if (theWorld.systemVersion < 0x0600) FatalAlert(SYSTEM_VERS_ERR, 0, 0);
	
	if (theWorld.machineType < 0) FatalAlert(ROM_VERS_ERR, 0, 0);
	
	// If there is a problem w/Gestalt, assume our keyboard has a Control key.
	// Otherwise, we assume we have a control key unless a Mac or MacPlus keyboard is
	// present.
		
	if ((err = Gestalt(gestaltKeyboardType, &response)) != noErr)
		gKeyboardHasControlKey = TRUE;
	else
		gKeyboardHasControlKey = !(	(response == gestaltMacKbd) ||
									(response == gestaltMacAndPad) ||
									(response == gestaltMacPlusKbd) );
									
	/* System 6.0 and up ALWAYS has WNE trap! */
}

void initmac( void)
{
	EventRecord myEvent;
	short 	i;
	long	gestaltvalue;
	OSErr	err;

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);				/* No resume proc */
	InitCursor();
	
	TelInfo = (TelInfoRec *)NewPtrClear(sizeof(TelInfoRec));
	mungbuf = (char *)NewPtrClear(1024);
	tempspot = (char *)NewPtrClear(256);
	gApplicationPrefs = (ApplicationPrefs *)NewPtrClear(sizeof(ApplicationPrefs));
	gFTPServerPrefs = (FTPServerPrefs *)NewPtrClear(sizeof(FTPServerPrefs));
	screens = (WindRec *)NewPtrClear(MaxSess*sizeof(WindRec));

	InquireEnvironment();

	for (i=1;i<12;i++)
		EventAvail( 0xffff, &myEvent);
	
	err = Gestalt(gestaltAppleEventsAttr, &gestaltvalue);		// See if AppleEvents are available
	gAEavail = (!err && ((gestaltvalue >> gestaltAppleEventsPresent) & 0x0001));
	
	if (gAEavail) {
		if (err = AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,
											MyHandleOAppUPP,0,FALSE))
			FatalAlert(AE_PROBLEM_ERR, 0, 0);
		if (err = AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,
											MyHandleODocUPP,0,FALSE))
			FatalAlert(AE_PROBLEM_ERR, 0, 0);
		if (err = AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,
											MyHandlePDocUPP,0,FALSE))
			FatalAlert(AE_PROBLEM_ERR, 0, 0);
		if (err = AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,
											MyHandleQuitUPP,0,FALSE))
			FatalAlert(AE_PROBLEM_ERR, 0, 0);
		}

	TelInfo->screenRect = qd.screenBits.bounds;			/* well, they have to be set */
												/* somewhere, where else ? */
	SetRect(&TelInfo->dragRect, 4, 24, TelInfo->screenRect.right-4,
						TelInfo->screenRect.bottom-4);
}

void DoTheGlobalInits(void)
{
	long junk = 0;
	ParamBlockRec pb;
	
	TelInfo->ScrlLock=0;
	TelInfo->ftplogon=0;
	TelInfo->done=0;
	TelInfo->MacBinary=0;
	TelInfo->numwindows=0;
	TelInfo->CONFstate=0;
	TelInfo->CONFactive=0;
	TelInfo->SettingsFile=-1;
	TelInfo->ApplicationFile = CurResFile();
	TelInfo->ginon   = 0;
	TelInfo->xferon  = 0;
	TelInfo->graphs = 0;
	TelInfo->debug = 1;
	TelInfo->myfronttype = NO_WINDOW;
	TelInfo->myfrontvs = 0;
	TelInfo->suspended = FALSE;
	TelInfo->myfrontwindow = 0L;	
	TelInfo->myfrontRgn = 0L;	
	TelInfo->lastCursor = 0L;
	TelInfo->DefaultDirVRefNum = -1;
	TelInfo->FTPClientVRefNum = -1;
	TelInfo->DefaultDirDirID = 2;
	TelInfo->FTPClientDirID = 2;
	
	pb.ioParam.ioNamePtr = 0L;
	PBGetVol(&pb, FALSE);			/* see Tech Note 140 for implications of this */

	GetWDInfo(pb.ioParam.ioVRefNum, &(TelInfo->ApFolder.vRefNum), &(TelInfo->ApFolder.parID), &junk);
	TelInfo->ApFolder.name[0] = 0;

	FindFolder( kOnSystemDisk, kSystemFolderType, kCreateFolder,
					&(TelInfo->SysFolder.vRefNum), &(TelInfo->SysFolder.parID));
	TelInfo->SysFolder.name[0]=0;

	FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
					&(TelInfo->PrefFolder.vRefNum), &(TelInfo->PrefFolder.parID));
	TelInfo->PrefFolder.name[0]=0;
	
	FindFolder( kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &TempItemsVRefNum, &TempItemsDirID);
}

void init (void)
{
	long		s;
	DialogPtr 	dtemp;		/* Used for dialog display */

	FlushEvents(everyEvent,0);
	initmac();				/* initialize Macintosh stuff */
	
	DoTheGlobalInits();

	OpenPreferencesFile();
	LoadPreferences();
	UnloadSeg(&LoadPreferences);
	
	setupmenu(gApplicationPrefs->CommandKeys);

	dtemp = GetNewMyDialog(FirstDLOG, NULL, kInFront, (void *)ThirdCenterDialog);	/* opening dialog */
	UItemAssign( dtemp, 2, VersionNumberUPP);
	DrawDialog(dtemp);										/* while we init everything */

	trInit(myMenus[National]);	/* LU: Initialize translation routines; must be done */
								/* LU: prior to checkdebug(), since even the debug 	 */
								/* LU: output goes through the translation routines. */
	UnloadSeg(&trInit);

	TelInfo->debug=checkdebug();		/* must return TRUE or FALSE */
	
	initmcurs(TelInfo->debug);		/* init the mouse cursor */
	SetCursor(theCursors[watchcurs]);

	if (RGMalloc()) {		
		DoError(103 | MEMORY_ERRORCLASS, LEVEL3, NULL);
		forcequit();
	}						
	UnloadSeg(&RGMalloc);
	
	VGalloc();						/* BYU - save global space */
	UnloadSeg(&VGalloc);
	
	if (VSinit(MaxSess+2)) {		/* initialize the Virtual Screen */
		DoError(104 | MEMORY_ERRORCLASS, LEVEL3, NULL);
		forcequit();
	}
	UnloadSeg(&VSinit);
	
	InitDebug();
	initftplog();					/* allocate/initialize ftp log screen */

	VGinit();						/* initialize Virtual Graphics after VS so that */
									/* init messages can come on console screen */
	UnloadSeg(&VGinit);
	
	VRinit();
	UnloadSeg(&VRinit);
	
	MacRGinit();
	UnloadSeg(&MacRGinit);
	
	init_mb_files();				/* Initialize file id's to closed state */
	UnloadSeg(init_mb_files);

	initnet();						/* initialize network stuff */
	switchMenus(gApplicationPrefs->CommandKeys);

	TelInfo->done = FALSE;			// We're not quitting yet.

	SetCursor(theCursors[normcurs]);

	initmacros();
//	initEvents();

	io = OpenResolver(NULL);

	if (io) {								
		DialogPtr theErrDialog;
		short itemhit = 0;

		theErrDialog = GetNewDialog(RESOLVERERROR, NULL, kInFront);
		DrawDialog(theErrDialog);
		while (!itemhit)
			ModalDialog(DLOGwOKUPP,&itemhit);
		DisposDialog(theErrDialog);
		}
	
	updateMenuChecks();
	DoTheMenuChecks();

	checkCONF();	 				/* Did user click on a set? */

	DisposDialog(dtemp);			/* Remove the splashbox...  */

	encryptOK = authOK = true;
	if (s = init_cornell_des()) {
		encryptOK = false;
		if (s == -2)				/* if no kdriver */
			authOK = false;
	}

}
/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************/

#ifdef MPW
#pragma segment 4
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "TelnetHeader.h"

#include "telneterrors.h"
#include "DlogUtils.proto.h"

#include "wind.h"
#include "event.proto.h"

#include "rsmac.proto.h"
#include "vsdata.h"
#include "vskeys.h"
#include "vsinterf.proto.h"
#include "vgtek.proto.h"
#include "tekrgmac.proto.h"
#include "vr.h"
#include "vrrgmac.proto.h" 
#include "network.proto.h"
#include "mydnr.proto.h"
#include "InternalEvents.h"
#include "menuseg.proto.h"
#include "maclook.proto.h"
#include "parse.proto.h"
#include "parse.h"
#include "configure.proto.h"

#include "prefs.proto.h"
#include "popup.h"
#include "popup.proto.h"

#include "Connections.proto.h"
#include "encrypt.proto.h"
#include "wdefpatch.proto.h"

/*	These are all of the variables we need... */

extern	Cursor	*theCursors[NUMCURS];		/* all the cursors in a nice bundle */
extern	WindRec	*screens;
extern	short	scrn;
extern	short	nNational;				// Number of user-installed translation tables
extern	MenuHandle	myMenus[];
extern	Boolean	authOK;
extern	Boolean	encryptOK;

static	pascal short POCdlogfilter( DialogPtr dptr, EventRecord *evt, short *item);
PROTO_UPP(POCdlogfilter, ModalFilter);

void OpenPortSpecial(MenuHandle menuh, short item)
{
	ConnInitParams		**theParams;
	Boolean				success;
	Str255				scratchPstring;
	
	GetItem(menuh, item, scratchPstring);
	
	theParams = NameToConnInitParams(scratchPstring);
	if (theParams == NULL) {
		OutOfMemory(1020);
		return;
		}
		
	success = CreateConnectionFromParams(theParams);
}

SIMPLE_UPP(POCdlogfilter, ModalFilter);
pascal short POCdlogfilter( DialogPtr dptr, EventRecord *evt, short *item)
{
	short key;

	if (evt->what == keyDown) {
		key = evt->message & charCodeMask;
		if ( ((key == 'F') || (key == 'f')) && (evt->modifiers & cmdKey) ) {
			*item = NCftpcheckbox;
			return(-1);
			}
		if ( ((key == 'A') || (key == 'a')) && (evt->modifiers & cmdKey) ) {
			*item = NCauthenticate;
			return(-1);
			}
		if ( ((key == 'E') || (key == 'e')) && (evt->modifiers & cmdKey) ) {
			*item = NCencrypt;
			return(-1);
			}
		}
	
	return(DLOGwOK_Cancel(dptr, evt, item));
}

void	PresentOpenConnectionDialog(void)
{
	ConnInitParams	**InitParams;
	DialogPtr		dptr;
	short			ditem, scratchshort;
	static long		numWind = 1;
	Boolean			success;
	long			scratchlong;
	Str255			scratchPstring;
	Handle			ItemHandle;
	SessionPrefs	**defaultSessHdl;
	MenuHandle		SessPopupHdl;
	Rect			scratchRect;
	Point			SessPopupLoc;
	
	SetCursor(theCursors[normcurs]);
	
	dptr = GetNewMyDialog(NewCnxnDLOG, NULL, kInFront, (void *)ThirdCenterDialog);
	if (dptr == NULL) {
		OutOfMemory(1000);
		return;
		}
		
	ditem = 3;
	
	SessPopupHdl = NewMenu(668, "\pSession:");
	if (SessPopupHdl == NULL) {
		OutOfMemory(1000);
		DisposeDialog(dptr);
		return;
		}
		
	UseResFile(TelInfo->SettingsFile);
	AddResMenu(SessPopupHdl, SESSIONPREFS_RESTYPE);
	EnableItem(SessPopupHdl, 0);		// Make sure the entire menu is enabled
	
	GetDItem(dptr, NCsesspopup, &scratchshort, &ItemHandle, &scratchRect);
	SessPopupLoc.h = scratchRect.left;
	SessPopupLoc.v = scratchRect.top;
	SetPort(dptr);
	LocalToGlobal(&SessPopupLoc);
	
	// Get default auth/encrypt settings from default session
	defaultSessHdl = GetDefaultSession();
	HLock((Handle)defaultSessHdl);

	BlockMove("\p<Default>", scratchPstring, 15);
	GetHostNameFromSession(scratchPstring);
	SetTEText(dptr, NChostname, scratchPstring);
	SelIText(dptr, NChostname, 0, 32767);

	SetCntrl(dptr, NCauthenticate, (**defaultSessHdl).authenticate && authOK);
	SetCntrl(dptr, NCencrypt, (**defaultSessHdl).encrypt && encryptOK);
	DisposeHandle((Handle)defaultSessHdl);
		
	setSessStates(dptr);

	while (ditem > NCcancel) {
		ModalDialog(POCdlogfilterUPP, &ditem);
		switch(ditem) {
			case	NCftpcheckbox:
			case	NCauthenticate:
			case	NCencrypt:
				GetDItem(dptr, ditem, &scratchshort, &ItemHandle, &scratchRect);
				if ((**(ControlHandle)ItemHandle).contrlHilite == 0) {	// if control not disabled
					FlipCheckBox(dptr, ditem);
					setSessStates(dptr);
				}
				break;

			case	NCsesspopup:
				InsertMenu(SessPopupHdl, hierMenu);
				CalcMenuSize(SessPopupHdl);
				scratchlong = PopUpMenuSelect(SessPopupHdl, SessPopupLoc.v,
												SessPopupLoc.h, 0);
				DeleteMenu(668);
				if (scratchlong) {
					scratchshort = scratchlong & 0xFFFF; //	Apple sez ignore the high word
					GetItem(SessPopupHdl, scratchshort, scratchPstring);
					SetTEText(dptr, NChostname, scratchPstring);
					SelIText(dptr, NChostname, 0, 32767);
					}
				break;
								
			default:
				break;
			} // switch
		} // while
	
	DisposeMenu(SessPopupHdl);
	
	if (ditem == NCcancel) {
		DisposeDialog(dptr);
		return;
		}
	
	GetTEText(dptr, NChostname, scratchPstring);
	if (!Length(scratchPstring)) {
		DisposeDialog(dptr);
		return;
		}
		
	InitParams = NameToConnInitParams(scratchPstring);
	if (InitParams == NULL) {
		OutOfMemory(1000);
		DisposeDialog(dptr);
		return;
		}

	HLock((Handle)InitParams);
	HLock((Handle)(**InitParams).session);
	GetTEText(dptr, NCwindowname, scratchPstring);

	// Copy over the user specified window name.  If blank, CreateConnectionFromParams 
	// will copy the hostname to the windowname and append a number.
	if (Length(scratchPstring)) 
		BlockMove(scratchPstring, (**InitParams).WindowName,
					(Length(scratchPstring) > 63) ? 64 : (Length(scratchPstring) + 1));

	if (GetCntlVal(dptr, NCftpcheckbox)) {
		(**InitParams).ftpstate = 1;
		(**(**InitParams).session).halfdup = 1;		/* BYU */
		}
 	if (GetCntlVal(dptr, NCauthenticate))
  		(**(**InitParams).session).authenticate = 1;
 	else
 		(**(**InitParams).session).authenticate = 0;
 	if (GetCntlVal(dptr, NCencrypt))
  		(**(**InitParams).session).encrypt = 1;
 	else
 		(**(**InitParams).session).encrypt = 0;
		
	HUnlock((Handle)(**InitParams).session);
	HUnlock((Handle)InitParams);
		
	DisposeDialog(dptr);
	
	success = CreateConnectionFromParams(InitParams);
}

// Set states of session checkboxes
void setSessStates (DialogPtr dptr)
{
	if (GetCntlVal(dptr, NCftpcheckbox)  || !authOK)
		Hilite(dptr, NCauthenticate, 255);
	else
		Hilite(dptr, NCauthenticate, 0);
		
	if (GetCntlVal(dptr, NCauthenticate)) {
		Hilite(dptr, NCftpcheckbox, 255);
		Hilite(dptr, NCencrypt, (encryptOK)? 0 : 255);
	} else {
		Hilite(dptr, NCftpcheckbox, 0);
		Hilite(dptr, NCencrypt, 255);
		SetCntrl(dptr, NCencrypt, false);
	}
}
		
Boolean CreateConnectionFromParams( ConnInitParams **Params)
{
	short			scratchshort, fontnumber, offset;
	static short	numWind = 1, stagNum = 1;
	SessionPrefs	*SessPtr;
	TerminalPrefs	*TermPtr;
	short			cur;
	Str32			numPstring;
	Str255			scratchPstring;
	Boolean			scratchBoolean;
	
	SetCursor(theCursors[watchcurs]);					/* We may be here a bit */

	// Check if we have the max number of sessions open
	if (TelInfo->numwindows == MaxSess) return(FALSE);
	
	cur = TelInfo->numwindows;			/* Adjust # of windows and get this window's number */
	TelInfo->numwindows++;
	screens[cur].active = CNXN_NOTINUSE;	// Make sure it is marked as dead (in case we
										// abort screen creation after initiating DNR.
										// That way CompleteConnectionOpening will know
										// we didn't make it.
	HLockHi((Handle)Params);
	HLockHi((Handle)(**Params).terminal);
	HLockHi((Handle)(**Params).session);
	SessPtr = *((**Params).session);
	TermPtr = *((**Params).terminal);

	if (Length((**Params).WindowName) == 0) {
		BlockMove((**(**Params).session).hostname, (**Params).WindowName, 
					Length((**(**Params).session).hostname)+1);
		NumToString(numWind++, numPstring);
		pstrcat((**Params).WindowName, "\p ");
		pstrcat((**Params).WindowName, numPstring);	// tack the number onto the end.
		}
		
	// Get the IP for the host while we set up the connection
	if (DoTheDNR(SessPtr->hostname, cur) != noErr) {
		OutOfMemory(1010);
		DisposeHandle((Handle)(**Params).terminal);
		DisposeHandle((Handle)(**Params).session);
		DisposeHandle((Handle)Params);
		TelInfo->numwindows--;
		updateCursor(1);
		return(FALSE);
		}
		
	DoTheMenuChecks();
	
  	screens[cur].authenticate = SessPtr->authenticate && authOK;
  	screens[cur].encrypt = SessPtr->encrypt && encryptOK;
 
 	if (screens[cur].authenticate || screens[cur].encrypt) {
 		if (screens[cur].edata = (CDATA *)NewPtrClear(sizeof(CDATA))) {
 			encrypt_init(screens[cur].edata, "Telnet", 0);
 			screens[cur].edata->wp = &screens[cur];
 		} else {
 			screens[cur].encrypt = false;
 			screens[cur].authenticate = false;
 			screens[cur].edata = NULL;
 		}
 	} else
 		screens[cur].edata = NULL;
 	
 	for (scratchshort = 0; scratchshort < sizeof(screens[cur].myopts); scratchshort++) {
		screens[cur].myopts[scratchshort] = 0;
		screens[cur].hisopts[scratchshort] = 0;		
	}	
	screens[cur].cannon[0] = '\0';

	screens[cur].vtemulation = TermPtr->vtemulation;
	screens[cur].forcesave = SessPtr->forcesave;
	screens[cur].lineAllow = SessPtr->linemode;
	screens[cur].eightbit = TermPtr->eightbit;	// Is this necessary?
	screens[cur].portNum = SessPtr->port;
	screens[cur].emacsmeta = TermPtr->emacsmetakey;
	screens[cur].Xterm = TermPtr->Xtermsequences;
	
	screens[cur].port = -1;				// netxopen will take care of this

	screens[cur].lmode = 0;
	for (scratchshort=0; scratchshort<= SLC_ARRAY_SIZE; scratchshort++)
		screens[cur].slc[scratchshort] = -1;
	screens[cur].slc[SLC_IP] = 3;
	screens[cur].slc[SLC_EC] = 127;
	screens[cur].slc[SLC_EL] = 21;
	screens[cur].slc[SLC_EOF] = 4;
	screens[cur].slc[SLC_ABORT] = 3;
	screens[cur].slc[SLC_SUSP] = 26; 

	// If the caller has not specified a window location, come up with our own.
	// This function can possibly stagger windows right off the screen.
	if((**Params).WindowLocation.top == 0) {
		offset = ((gApplicationPrefs->StaggerWindows == TRUE) ? 10 : 2) * (stagNum++);
		(**Params).WindowLocation.top = GetMBarHeight() + 10 + offset;
		(**Params).WindowLocation.left  = offset;
		(**Params).WindowLocation.bottom= 30000 + offset;
		(**Params).WindowLocation.right = 30000 + offset;
		}
		
	GetFNum(TermPtr->DisplayFont, &fontnumber);
	
	screens[cur].vs = RSnewwindow( &((**Params).WindowLocation), TermPtr->numbkscroll, TermPtr->vtwidth,
									TermPtr->vtheight, (**Params).WindowName, TermPtr->vtwrap,
									fontnumber, TermPtr->fontsize, 0,
									1,
									SessPtr->forcesave);
									
	if (screens[cur].vs <0 ) { 	/* we have a problem opening up the virtual screen */
		OutOfMemory(1011);
		DisposeHandle((Handle)(**Params).terminal);
		DisposeHandle((Handle)(**Params).session);
		DisposeHandle((Handle)Params);
		TelInfo->numwindows--;
		DoTheMenuChecks();
		updateCursor(1);
		return(FALSE);
		}

	screens[cur].wind = RSgetwindow( screens[cur].vs);
	((WindowPeek)screens[cur].wind)->windowKind = WIN_CNXN;
	
	/*
	 * Attach our extra part to display encryption status
	 */
	PatchWindowWDEF(screens[cur].wind, &screens[cur]);

	screens[cur].arrowmap = TermPtr->emacsarrows;  		/* MAT -- save our arrow setting */
	screens[cur].maxscroll= TermPtr->numbkscroll;
	screens[cur].rows = TermPtr->vtheight;				/* BYU 2.4.16 */
	screens[cur].bsdel = SessPtr->bksp;
	screens[cur].crmap = SessPtr->crmap;
	screens[cur].tekclear = SessPtr->tekclear;
	screens[cur].ESscroll= TermPtr->clearsave;
	screens[cur].tektype = SessPtr->tektype;
	screens[cur].wrap = TermPtr->vtwrap;
	screens[cur].pgupdwn = TermPtr->MATmappings;		/* JMB: map pgup/pgdwn/home/end? */
	screens[cur].width= TermPtr->vtwidth;
	screens[cur].TELstop = SessPtr->skey;
	screens[cur].TELgo = SessPtr->qkey;
	screens[cur].TELip = SessPtr->ckey;
	BlockMove((Ptr)SessPtr->hostname, screens[cur].machine, Length(SessPtr->hostname)+1);
	BlockMove(TermPtr->AnswerBackMessage, screens[cur].answerback, 32);
	screens[cur].termstate = VTEKTYPE;
	screens[cur].naws = 0;								/* NCSA - set default NAWS to zero */
	screens[cur].xfer=0;
	screens[cur].telstate=0;
	screens[cur].timing=0;
	screens[cur].curgraph=-1;				/* No graphics screen */
	screens[cur].ftpport = -1;				/* BYU - No additional FTP port opened yet */
	screens[cur].clientflags = 0;			/* BYU */
	screens[cur].kblen = 0;				/* nothing in the buffer */
	screens[cur].enabled = 1;			/* Gotta be enabled to start with */
	screens[cur].Ittype = 0;
	screens[cur].Isga = 0;				/* I suppress go ahead = no */
	screens[cur].Usga = 0;				/* U suppress go ahead = no */

	screens[cur].ftpstate = (**Params).ftpstate;	/* BYU */
	if ((**Params).ftpstate != 0) {		/* BYU */
		screens[cur].termstate=DUMBTYPE;	/* BYU */
		screens[cur].echo=1;				/* BYU - Echo for ftp */
		screens[cur].halfdup = 1;			/* BYU */
	} else {							/* BYU */
		screens[cur].termstate=VTEKTYPE;	/* BYU */
		screens[cur].echo = 1;
		screens[cur].halfdup = SessPtr->halfdup;	/* BYU */
	}
	
	screens[cur].national = 0;			// Default to no translation.
	// Now see if the desired translation is available, if not use default translation.
	for(scratchshort = 1; scratchshort <= nNational+1; scratchshort++) {
		GetItem(myMenus[National], scratchshort, scratchPstring);
		if (EqualString(SessPtr->TranslationTable, scratchPstring, TRUE, FALSE))
			screens[cur].national = scratchshort-1;
		}
				
	
	// Set up paste related variables
	screens[cur].incount = 0;
	screens[cur].outcount = 0;
	screens[cur].outptr = NULL;
	screens[cur].outhand = NULL;
	screens[cur].outlen = 0;
	screens[cur].pastemethod = SessPtr->pastemethod;
	screens[cur].pastesize = SessPtr->pasteblocksize;
	
	scratchBoolean = NewRSsetcolor( screens[cur].vs, 0, TermPtr->nfcolor);
	scratchBoolean = NewRSsetcolor( screens[cur].vs, 1, TermPtr->nbcolor);
	scratchBoolean = NewRSsetcolor( screens[cur].vs, 2, TermPtr->bfcolor);
	scratchBoolean = NewRSsetcolor( screens[cur].vs, 3, TermPtr->bbcolor);

	addinmenu(cur, (**Params).WindowName, diamondMark);
	screens[cur].active = CNXN_DNRWAIT;			// Signal we are waiting for DNR.

	screens[cur].myInitParams = (Handle)Params;
	HUnlock((Handle)(**Params).terminal);
	HUnlock((Handle)(**Params).session);
	// Params handle must stay locked because interrupt level DNR completion routine needs to deref it

	VSscrolcontrol( screens[cur].vs, -1, screens[cur].ESscroll);

	updateCursor(1);							/* Done stalling the user */
	return(TRUE);
}

void	CompleteConnectionOpening(short dat, ip_addr the_IP, OSErr DNRerror, char *cname)
{
	ConnInitParams	**Params;
	
	if (screens[dat].active != CNXN_DNRWAIT) return;			// Something is wrong.
	
	Params = (ConnInitParams **)screens[dat].myInitParams;
	
	if (DNRerror == noErr) {
		HLockHi((Handle)(**Params).session);
		if (screens[dat].ftpstate != 0) 				/* BYU - ftp client */
			screens[dat].port  = netxopen(the_IP,HFTP,40);	/* BYU 2.4.15 - open to host name */
		else 											/* BYU */
			screens[dat].port  = netxopen(the_IP,(**(**Params).session).port,40);/* BYU 2.4.15 - open to host name */
		
		// We need the cannonical hostname for Kerberos. Make best guess if
		// DNR did not return a cname.
		if (cname)
			strncpy(screens[dat].cannon, cname, sizeof(screens[dat].cannon));
		else
			strncpy(screens[dat].cannon, (char *)(**(**Params).session).hostname, sizeof(screens[dat].cannon));
		screens[dat].cannon[sizeof(screens[dat].cannon)-1] = '\0';
		
		DisposeHandle((Handle)(**Params).session);
		DisposeHandle((Handle)(**Params).terminal);
		DisposeHandle((Handle)Params);
		
		if (screens[dat].port <0) {					/* Handle netxopen fail */
			destroyport(dat);
			}
		screens[dat].active = CNXN_OPENING;
		SetMenuMarkToOpeningForAGivenScreen(dat);	/* Change status mark */
		}
	else
		{	// We should report the real DNR error here!
		Str255		errorString, numberString, numberString2, scratchPstring;
		DialogPtr	theDialog;
		short		message, ditem = 3;
		
		HLockHi((Handle)(**Params).session);
		BlockMove((**(**Params).session).hostname, scratchPstring, Length((**(**Params).session).hostname)+1);

		if (DNRerror >= -23048 && DNRerror <= -23041) message = DNRerror + 23050;
		else message = 1;
		
		GetIndString(errorString,DNR_MESSAGES_ID, message);
		NumToString((long)0, numberString);
		NumToString((long)DNRerror, numberString2);
		ParamText(scratchPstring, errorString, numberString, numberString2);
		
		theDialog = GetNewMyDialog(DNRErrorDLOG, NULL, kInFront, (void *)ThirdCenterDialog);
		ShowWindow(theDialog);
	
		while (ditem > 1)	ModalDialog(DLOGwOKUPP, &ditem);
		DisposeDialog(theDialog);

		DisposeHandle((Handle)(**Params).session);
		DisposeHandle((Handle)(**Params).terminal);
		DisposeHandle((Handle)Params);
		destroyport(dat);
		}
}

void destroyport(short wind)
{
	Handle	h;
	short	i,
			callNoWindow=0;

	SetCursor(theCursors[watchcurs]);		/* We may be here a while */

	if (screens[wind].active == CNXN_ISCORPSE) {
		if (screens[wind].curgraph>-1)
			detachGraphics( screens[wind].curgraph);	/* Detach the Tek screen */
		if (screens[wind].outlen>0) {
			screens[wind].outlen=0;						/* Kill the remaining send*/
			HUnlock( screens[wind].outhand);			/*  buffer */
			HPurge ( screens[wind].outhand);
			}
		}

	if (FrontWindow() == screens[wind].wind)
		callNoWindow=1;

 	if (screens[wind].edata != NULL)
 		DisposePtr((Ptr)screens[wind].edata);
 
	/*
	 * Get handle to the WDEF patch block, kill the window, and then
	 * release the handle.
	 */
	h = GetPatchStuffHandle(screens[wind].wind, &screens[wind]);
	RSkillwindow( screens[wind].vs);
	if (h)
		DisposeHandle(h);

	screens[wind].active = CNXN_NOTINUSE;
	for (i=wind;i<TelInfo->numwindows-1;i++) {
		screens[i]=screens[i+1];		/* Bump all of the pointers */
		RePatchWindowWDEF(screens[i].wind, &screens[i]);	/* hack hack hack */
 		if (screens[i].edata)
 			screens[i].edata->wp = &screens[i];
		}
	if (scrn>wind) scrn--;				/* Adjust for deleting a lower #ered screen */

	TelInfo->numwindows--;						/* There are now fewer windows */
	extractmenu( wind);					/* remove from the menu bar */

	DoTheMenuChecks();

/* BYU 2.4.11 - the call to "NoWindow()" changes "myfrontwindow", 
                which is used by "updateCursor()", so we reversed 
                the order of the following two lines. */
	if (callNoWindow) NoWindow();		/* BYU 2.4.11 - Update cursor stuff if front window */
	updateCursor(1);					/* BYU 2.4.11 - Done stalling the user */

} /* destroyport */

void removeport(short n)
{
	Str255		scratchPstring;
	
	SetCursor(theCursors[watchcurs]);				/* We may be here a while */

	if (screens[n].curgraph>-1)
		detachGraphics( screens[n].curgraph);		/* Detach the Tek screen */
		
	if (screens[n].outlen>0) {
				screens[n].outlen=0;				/* Kill the remaining send*/
				HUnlock( screens[n].outhand);		/*  buffer */
				HPurge ( screens[n].outhand);
				}

	if (VSiscapturing(screens[n].vs))				/* NCSA: close up the capture */
		CloseCaptureFile(screens[n].vs);			/* NCSA */

	if (VSisprinting(screens[n].vs))
		ClosePrintingFile(screens[n].vs);
		
	if (!gApplicationPrefs->WindowsDontGoAway)
		destroyport(n);
	else {
		Str255	temp;
		
		GetWTitle(screens[n].wind, scratchPstring);
#ifdef THINK_C
		sprintf((char *)temp, "(%#s)", scratchPstring);
#else
		PtoCstr(scratchPstring);
		sprintf((char *)temp, "(%s)", scratchPstring);
#endif
		CtoPstr((char *)temp);
		SetWTitle(screens[n].wind, temp);

		screens[n].port = 32700;
		screens[n].active = CNXN_ISCORPSE;
		}

	updateCursor(1);							/* Done stalling the user */
} /* removeport */




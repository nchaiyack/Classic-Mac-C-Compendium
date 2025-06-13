/*
*    event.c
*****************************************************************
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
*****************************************************************
*
*    Main Event loop code for NCSA Telnet for the Macintosh
*
*	Called by:
*		maclook.c
*
*	Revisions:
*	7/92	Telnet2.6:	added 2 support for 2 global structs, put cursors into
*						an array, cleaned up defines			Scott Bulmahn						
*
*/

#ifdef MPW
#pragma segment 4
#endif

/*
 *	Files for inclusion.
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <EPPC.h>
#include <AppleEvents.h>

#include "TelnetHeader.h"
#include "VSkeys.h"
#include "wind.h"

#include "network.proto.h"
#include "mydnr.proto.h"
#include "bkgr.proto.h"
#include "maclook.proto.h"
#include "InternalEvents.h"
#include "vsdata.h"
#include "vsinterf.proto.h"
#include "menuseg.proto.h"
#include "vrrgmac.proto.h"
#include "tekrgmac.proto.h"
#include "rsmac.proto.h"
#include "event.proto.h"
#include "macros.proto.h"				/* For setmacro proto */
#include "netevent.proto.h"
#include "translate.proto.h"
#include "parse.proto.h"
#include "ftpbin.proto.h"
#include "debug.h"
#include "Connections.proto.h"

extern short		scrn;
extern MenuHandle	myMenus[NMENUS];
extern Boolean		gKeyboardHasControlKey;
extern SysEnvRec	theWorld;		/* BYU 2.4.12 - System Environment record */
extern WindRec		*screens,
					*ftplog;
extern Cursor		*theCursors[];

/*
 *	Declarations for constant variables
 */

unsigned char kpxlate[2][62] =
  /* table for translating virtual scan codes to internal codes for
	special keys. */
  {
	  {		/* virtual key code */
		VSKP,	/* $41 */
		VSRT,	/* $42 (Mac+) */
		VSF4,	/* $43 (ADB) */
		0,		/* $44 */
		VSKC,	/* $45 (ADB std) */
		VSLT,	/* $46 (Mac+) */
		VSF1,	/* $47 */
		VSDN,	/* $48 (Mac+) */
		0,		/* $49 */
		0,		/* $4A */
		VSF3,	/* $4B (ADB) */
		VSKE,	/* $4C */
		VSUP,	/* $4D (Mac+) */
		VSKM,	/* $4E */
		0,		/* $4F */
		0,		/* $50 */
		VSF2,	/* $51 (ADB) */
		VSK0,	/* $52 */
		VSK1,	/* $53 */
		VSK2,	/* $54 */
		VSK3,	/* $55 */
		VSK4,	/* $56 */
		VSK5,	/* $57 */
		VSK6,	/* $58 */
		VSK7,	/* $59 */
		0,		/* $5A */
		VSK8,	/* $5B */
		VSK9,	/* $5C */
		0,		/* $5D */
		0,		/* $5E */
		0,		/* $5F */
		VSF10,	/* $60 */	/* BYU 2.4.12 */
		VSF11,	/* $61 */	/* BYU 2.4.12 */
		VSF12,	/* $62 */	/* BYU 2.4.12 */
		VSF8,	/* $63 */	/* BYU 2.4.12 - was VSF3 */
		VSF13,	/* $64 */	/* BYU 2.4.12 */
		VSF14,	/* $65 */	/* BYU 2.4.12 */
		0,		/* $66 */
		VSF16,	/* $67 */	/* BYU 2.4.12 */
		0,		/* $68 */
		VSF18,	/* $69 */	/* BYU 2.4.12 */
		0,		/* $6A */
		VSF19,	/* $6B */	/* BYU 2.4.12 */
		0,		/* $6C */
		VSF15,	/* $6D */	/* BYU 2.4.12 */
		0,		/* $6E */
		VSF17,	/* $6F */	/* BYU 2.4.12 */
		0,		/* $70 */
		VSF20,	/* $71 */	/* BYU 2.4.12 */
		VSHELP,	/* $72 */	/* BYU 2.4.12 */
		VSHOME,	/* $73 */	/* BYU 2.4.12 */
		VSPGUP,	/* $74 */	/* BYU 2.4.12 */
		VSDEL,	/* $75 (ADB ext) */		/* BYU 2.4.12 - was 0x7f */
		VSF9,	/* $76 */	/* BYU 2.4.12 - was VSF4 */
		VSEND,	/* $77 */	/* BYU 2.4.12 */
		VSF7,	/* $78 */	/* BYU 2.4.12 - was VSF2 */
		VSPGDN,	/* $79 */	/* BYU 2.4.12 */
		VSF6,	/* $7A */	/* BYU 2.4.12 - was VSF1 */
		VSLT,	/* $7B */
		VSRT,	/* $7C */
		VSDN,	/* $7D */
		VSUP	/* $7E */
	  },
	  {
		VSKP,	/* $41 */
		VSF4,	/* $42 (Mac+) */
		VSF4,		/* $43 (ADB) */
		0,		/* $44 */
		VSKC,		/* $45 (ADB) */
		VSKC,	/* $46 (Mac+) */
		VSF1,	/* $47 */
		VSF2,	/* $48 */
		0,		/* $49 */
		0,		/* $4A */
		VSF3,		/* $4B */
		VSKE,	/* $4C */
		VSF3,	/* $4D */
		VSKM,		/* $4E */
		0,		/* $4F */
		0,		/* $50 */
		VSF2,		/* $51 */
		VSK0,	/* $52 */
		VSK1,	/* $53 */
		VSK2,	/* $54 */
		VSK3,	/* $55 */
		VSK4,	/* $56 */
		VSK5,	/* $57 */
		VSK6,	/* $58 */
		VSK7,	/* $59 */
		0,		/* $5A */
		VSK8,	/* $5B */
		VSK9,	/* $5C */
		0,		/* $5D */
		0,		/* $5E */
		0,		/* $5F */
		VSF10,	/* $60 */	/* BYU 2.4.12 */
		VSF11,	/* $61 */	/* BYU 2.4.12 */
		VSF12,	/* $62 */	/* BYU 2.4.12 */
		VSF8,	/* $63 */	/* BYU 2.4.12 - was VSF3 */
		VSF13,	/* $64 */	/* BYU 2.4.12 */
		VSF14,	/* $65 */	/* BYU 2.4.12 */
		0,		/* $66 */
		VSF16,	/* $67 */	/* BYU 2.4.12 */
		0,		/* $68 */
		VSF18,	/* $69 */	/* BYU 2.4.12 */
		0,		/* $6A */
		VSF19,	/* $6B */	/* BYU 2.4.12 */
		0,		/* $6C */
		VSF15,	/* $6D */	/* BYU 2.4.12 */
		0,		/* $6E */
		VSF17,	/* $6F */	/* BYU 2.4.12 */
		0,		/* $70 */
		VSF20,	/* $71 */	/* BYU 2.4.12 */
		VSHELP,	/* $72 */	/* BYU 2.4.12 */
		VSHOME,	/* $73 */	/* BYU 2.4.12 */
		VSPGUP,	/* $74 */	/* BYU 2.4.12 */
		VSDEL,	/* $75 (ADB ext) */		/* BYU 2.4.12 - was 0x7f */
		VSF9,	/* $76 */	/* BYU 2.4.12 - was VSF4 */
		VSEND,	/* $77 */	/* BYU 2.4.12 */
		VSF7,	/* $78 */	/* BYU 2.4.12 - was VSF2 */
		VSPGDN,	/* $79 */	/* BYU 2.4.12 */
		VSF6,	/* $7A */	/* BYU 2.4.12 - was VSF1 */
		VSLT,	/* $7B */
		VSRT,	/* $7C */
		VSDN,	/* $7D */
		VSUP	/* $7E */
	  }
  };

/* ASCII character set defines */

#define DELchar		0x7f	/* BYU LSC - (DEL is defined in MacLook.h) the delete character */
#define	KILLCHAR	0x15	/* the character to kill the local line with */

#include "event.proto.h"

short updateCursor(short force)
{
	static Point	lastPoint;
	static short	optwasdown;
	short			optDown;
	Point			myPoint;
	unsigned char	allthekeys[16];			/* Someplace to put the keymap */

	if (TelInfo->myfrontwindow) {					/* BYU 2.4.11 */
		SetPort((GrafPtr) TelInfo->myfrontwindow);	/* BYU 2.4.11 */
	} else {								/* BYU 2.4.11 */
		SetCursor(theCursors[normcurs]);				/* BYU 2.4.11 */
		return(0);							/* BYU 2.4.11 */
	}

	GetMouse(&myPoint);

	GetKeys((long *) allthekeys);
	optDown = allthekeys[7] &4;

	if ( (!force) && EqualPt(myPoint,lastPoint) && (optDown ==optwasdown))
		return(0);

	if (force)
		TelInfo->lastCursor=0L;
	if (TelInfo->ginon) {
		if (TelInfo->lastCursor!= theCursors[gincurs]) {
			SetCursor(theCursors[gincurs]);
			TelInfo->lastCursor = theCursors[gincurs];
			}
		return(1);
		}

	if (TelInfo->xferon && !optDown) {
		if  (TelInfo->lastCursor!= theCursors[xfercurs]) {
			SetCursor( theCursors[xfercurs]);
			TelInfo->lastCursor = theCursors[xfercurs];
			}
		return(1);
		}

	switch (TelInfo->myfronttype) {
		case DEC_WINDOW:
			if (RSmouseintext( TelInfo->myfrontvs, myPoint)) {
				if (optDown) {				/* Option key is down */
					if (TelInfo->lastCursor != theCursors[poscurs]) {
						TelInfo->lastCursor  = theCursors[poscurs];
						SetCursor(theCursors[poscurs]);
						}
				} else {
					if (TelInfo->lastCursor != theCursors[textcurs]) {
						TelInfo->lastCursor  = theCursors[textcurs];
						SetCursor(theCursors[textcurs]);
						}
					}
			} else {
				if (TelInfo->lastCursor != theCursors[normcurs]) {
					TelInfo->lastCursor  = theCursors[normcurs];
					SetCursor(theCursors[normcurs]);
					}
			}
			break;
		case TEK_WINDOW:
			LocalToGlobal(&myPoint);
			if (PtInRgn(myPoint, TelInfo->myfrontwindow->contRgn)) {		/* BYU LSC */
				if (TelInfo->lastCursor != theCursors[graphcurs]) {
					TelInfo->lastCursor  = theCursors[graphcurs];
					SetCursor(theCursors[graphcurs]);
					}
			} else {
				if (TelInfo->lastCursor != theCursors[normcurs]) {
					TelInfo->lastCursor  = theCursors[normcurs];
					SetCursor(theCursors[normcurs]);
					}
			}
			break;
		case NO_WINDOW:
		default:
			if (force) {
				SetCursor( theCursors[normcurs]);
				TelInfo->lastCursor= theCursors[normcurs];
				}
		}
	lastPoint=myPoint;
	optwasdown=optDown;
	return(0);
}

void NoWindow( void)
{
		TelInfo->myfrontwindow=0L;
		TelInfo->myfronttype=NO_WINDOW;
		TelInfo->myfrontRgn=0L;
		updateCursor(1);
}

/* 	The following code was graciously donated by Marc Tamsky.  When are YOU going to donate
	YOUR code, eh?  We know you're reading this.  -- JMB */

Boolean CheckPageKeys(short code)											/* NCSA: SB */
{																			/* NCSA: SB */
	GrafPtr currFW;     // current front window holder              		// MAT--
	short     ourW;       // virtual screen number holder             		// MAT--kinda
	short     x1, y2, x2, y1; // coordinates from window              		// MAT--pulled from scrollproc
																			/* NCSA: SB */
	currFW = (GrafPtr)FrontWindow();                            			// MAT--
	ourW = RSfindvwind(currFW);                                 			// MAT--
																			/* NCSA: SB */	
	switch (code)															/* NCSA: SB */
		{																	/* NCSA: SB */
		case VSPGUP:														/* NCSA: SB */
			VSgetrgn(ourW, &x1, &y1, &x2, &y2);                   			// MAT--
			VSscrolback(ourW, y2 - y1); /* scroll a whole windowful */  	// MAT--
			return TRUE;                                                   	// MAT--
			break;															/* NCSA: SB */															
																			/* NCSA: SB */
		case VSPGDN:                                                   		// MAT--121 is a PAGE DOWN.		                                                             	 
																			// MAT--in rsmac.c
			VSgetrgn(ourW, &x1, &y1, &x2, &y2);                     		// MAT--
			VSscrolforward(ourW, y2 - y1); /* scroll a whole windowful */   // MAT--
			return TRUE;                                                    // MAT--
			break; 															/* NCSA: SB */
			                                                              	// MAT--
		case VSHOME:														/* NCSA: SB */
			VSscroltop(ourW);												/* JMB 2.6 -- Created VSscroltop just for this purpose */
			return TRUE;													/* NCSA: SB */
			break;                                                     		// MAT--
		                                                               		// MAT--
		case VSEND:															/* NCSA: SB */
			VSgetrgn(ourW, &x1, &y1, &x2, &y2);                     		// MAT--
			VSscrolforward(ourW, 32765); /* scroll a whole BUNCH! */    	// MAT-- kludge time again.  anyone suggest
			return TRUE;													/* NCSA: SB */
			break;                                                     		// MAT--  a better way to hack this part?
		}                                                               	// MAT--
																			/* NCSA: SB */
	return FALSE;			/* NCSA: SB - we didnt handle event, let us know about it */
}																			/* NCSA: SB */

/*----------------------------------------------------------------------------------*/
/*  SendOneChar --																	*/
/*	This code was moved out of HandleKeyDown also, and made into one chunk of code.		*/
/*	Basically I got tired of looking at this code all over HandleKeyDown, 					*/
/*	so I put it here -- SMB.  Take one char, and write it to the network.  If it	*/
/*	needs to be echoed, then send it off to parse... 		-SMB					*/	
	
void SendOneChar(unsigned char sendch)						/* NCSA: SB */
{															/* NCSA: SB */
	if (screens[scrn].echo && screens[scrn].halfdup) 		/* NCSA: SB - support local echo */
		parse( &screens[scrn], &sendch, 1);					/* NCSA: SB */
															/* NCSA: SB */
	netwrite(screens[scrn].port,&sendch,1);					/* NCSA: SB */
	netpush(screens[scrn].port);							/* NCSA: SB */
}															/* NCSA: SB */

/*  HandleKeyDown --
		By now, we have already gotten a keypress signal from the event handler, so we
		just need to interpret it.  Get the	raw code and ascii value, and then decide
		what to do with it.	*/	

void HandleKeyDown(EventRecord theEvent)
{
	unsigned char ascii, code;
	unsigned char sendch;
	long	menuEquiv;
	short	enterkey = 0;
	Boolean	commanddown, optiondown, controldown;
		
	ObscureCursor();

	// We don't muck with the KCHR anymore, let the system do that!
	
	ascii = theEvent.message & charCodeMask;
	code = ((theEvent.message & keyCodeMask) >> 8);
	commanddown = ((theEvent.modifiers & cmdKey) != 0);
	optiondown = ((theEvent.modifiers & optionKey) != 0);
	controldown = ((theEvent.modifiers & controlKey) != 0);

	if (DebugKeys(commanddown, ascii, scrn))	// Handle debug info keys
			return;
			
	//	Check for EMACS meta key hack first.
	/* NCSA: SB - here is where we are going to check if it is an option-escape mapping */
	/* NCSA: SB - If so, handle the keypress and return									*/
	if (screens[scrn].emacsmeta && !screens[scrn].ftpstate &&
		commanddown && controldown && (ascii >= 1 && ascii < ESC))	/* NCSA: SB */
		{																		/* NCSA: SB */				
		if (screens[scrn].lmode)												/* NCSA: SB */
			{																	/* NCSA: SB */
			netwrite( screens[scrn].port, screens[scrn].kbbuf					/* NCSA: SB */
					, screens[scrn].kblen);										/* NCSA: SN - flush buffer */
			screens[scrn].kblen=0;												/* NCSA: SB */
			}																	/* NCSA: SB */
		ascii |= 0x60;															/* NCSA: SB */
		SendOneChar(ESC);								/* NCSA: SB - send the ESC char first */
		SendOneChar(ascii);								/* NCSA: SB - now send the char */
		return;											/* NCSA: SB - we handled the char, so return */
		}
	
	//	Handle command keys first (if CommandKeys preference is set to TRUE)
	if (commanddown && gApplicationPrefs->CommandKeys) {
		menuEquiv = MenuKey(ascii);
		if ((menuEquiv & 0xFFFF0000) != 0) {
			HandleMenuCommand(menuEquiv,theEvent.modifiers);
			return;
			}
		}

	//	If the keyboard has no control key and command keys are off, remap the command
	//	key to control
	if (commanddown && !gApplicationPrefs->CommandKeys && !gKeyboardHasControlKey) {
		commanddown = 0;	
		controldown = 1;
		}
	
	//	Control-Space results in 0 ASCII	
	if (controldown && (ascii == 32))
				ascii=0;			/* ASCII nul value for command-spacebar */

	/* Remap PgUp,PgDown,Home,End if the user wants it that way */
	if (screens[scrn].pgupdwn && !screens[scrn].ftpstate && (code >= KPlowest)) 
		if (CheckPageKeys(code)) return;		

	/* The rest of HandleKeyDown doesn't make any sense if the window is a corpse.
		So we can just return out in that case */
	if (screens[scrn].active == CNXN_ISCORPSE) return;

	if (commanddown)  {		/* handle key macros */
		if (ascii >='0' && ascii <='9' )  {
			sendmacro( ascii-'0');
			return;
			}
		}

	//	If we are mapping "`" to ESC, do it unless the command key is down.
	if (ascii == '`' && gApplicationPrefs->RemapTilde && (!commanddown))
		ascii = ESC;

	/*	If there are no active windows, or if user has hit a command key that doesn't
		correspond to a menu or a tilde remap override, ignore the key event */
	if (TelInfo->numwindows<1 || (	(commanddown) && 
									gApplicationPrefs->CommandKeys &&
									ascii != '`' ))
		return;

	if (!screens[scrn].ftpstate && (code >= KPlowest)) {		/* BYU - Handle Keypad */
		short shifted;
		shifted = ((theEvent.modifiers & shiftKey) != 0);
		if (theWorld.keyBoardType == envStandADBKbd)			/* BYU 2.4.12 */
			if (code == 0x45)	code = 0x4e;
			else if (code == 0x4e)	code = 0x45;
			
		ascii = kpxlate[shifted][code - KPlowest];
		// Should we check here for ascii being zero?
		VSkbsend(screens[scrn].vs, (unsigned char) ascii, screens[scrn].echo);
		return;
	  }


	/* BSD-like mapping.... if we don't want this, set chars to zero and it wont work */
	if (ascii == screens[scrn].TELstop)  {
		screens[scrn].enabled = 0;
		return;
		}

	if (ascii == screens[scrn].TELgo) {
		screens[scrn].enabled = 1;
		return;
		}
		
	if (ascii == screens[scrn].TELip)  {
		netpush(screens[scrn].port);
		netwrite(screens[scrn].port, "\377\364",2);
		screens[ scrn].timing = 1;							/* set emulate to TMwait */
		netwrite(screens[scrn].port, "\377\375\006",3);		/* send TM */
		if (screens[scrn].lmode) 
			screens[scrn].kblen=0;
		return;
		}

	//	Handle whatever mapping is needed.
	mac_nat(&ascii, screens[scrn].national); /* LU/PM: Convert char to US */
	
	//	If bsdel is set and the "delete" key was hit, make the ascii a Delete character
	if ((screens[scrn].bsdel) && (code == BScode)) ascii = DELchar;

	//	If the option key or command key is down, flip BS and DEL
	if (theEvent.modifiers & (optionKey | cmdKey) && (code == BScode))
			if (screens[scrn].bsdel) ascii = BS;
				else ascii = DEL;

	//	If we have a Mac Plus keyboard and not using command keys, command = control key
	if (commanddown && !gApplicationPrefs->CommandKeys && !gKeyboardHasControlKey)
		ascii &= 0x1f;
				
	if (screens[scrn].ftpstate)							/* BYU - ftpclient */
		{												/* BYU */
		if (ascii>31 && ascii != 127 && ascii < 255 && code<KPlowest)	/* BYU - add these chars to buffer */
			{											/* BYU */
			if (screens[scrn].kblen < (MAXKB -1)) 		/* BYU - Add to buffer if not full */
				{										/* BYU */
				screens[scrn].kbbuf[ screens[scrn].kblen++ ] = ascii;	/* BYU */
				sendch=ascii;							/* BYU */
				parse( &screens[ scrn], &sendch, 1);	/* BYU */
				}										/* BYU */
			else 										/* BYU */
				SysBeep(4);								/* BYU - buffer full */
			}											/* BYU */
		else											/* BYU - not printable char */
			{											/* BYU */
			if ( code == BScode ) 						/* BYU */
				{										/* BYU */
				if (screens[scrn].kblen>0) 				/* BYU */
					{									/* BYU */
					screens[scrn].kblen--;					/* BYU */
					parse( &screens[scrn],(unsigned char *) "\010 \010",3);	/* BYU */
					}										/* BYU */
				}										/* BYU */
			else if (ascii == CR)						/* BYU */
				{										/* BYU */
				parse( &screens[scrn],(unsigned char *) "\015\012",2);		/* BYU */
				screens[scrn].kbbuf[ screens[scrn].kblen++ ] = 0;	/* BYU */
				ftppi(screens[scrn].kbbuf);							/* BYU - ftp client */
				screens[scrn].kblen=0;					/* BYU */
				}										/* BYU */
			else if (ascii == KILLCHAR) 				/* BYU */
				{										/* BYU */
				while (screens[scrn].kblen >0) 			/* BYU */
					{										/* BYU */
					parse( &screens[scrn],(unsigned char *) "\010 \010",3);	/* BYU */
					screens[scrn].kblen--;					/* BYU */
					}									/* BYU */
				}
			}
		return;											/* BYU */
		}

	else if (screens[scrn].lmode)	/* Some form of linemode is active */
		{
		if (screens[scrn].lmode & 2) {		// TRAPSIG mode active
			char vtemp[5];
	
			
			switch (ascii) {
				case 26:
						netwrite( screens[scrn].port, screens[scrn].kbbuf, screens[scrn].kblen);	/* if full send buffer */
						screens[scrn].kblen=0;
						sprintf(vtemp,"%c%c",255,237);	// IAC SUSP
						netwrite(screens[scrn].port,vtemp,2);
					break;
				case 3: 
						netwrite( screens[scrn].port, screens[scrn].kbbuf, screens[scrn].kblen);	/* if full send buffer */
						screens[scrn].kblen=0;
						sprintf(vtemp,"%c%c",255,244);	// IAC ABORT
						netwrite(screens[scrn].port,vtemp,2);
					break;
				}
			}
					
		if (screens[scrn].echo && ! screens[scrn].halfdup)	/* Handle local ECHOs */

			if (ascii>31 && ascii <127 && code<KPlowest)	/* add these chars to buffer */
				{
				if (screens[scrn].kblen < (MAXKB -1)) 	/* Add to buffer if not full */
					screens[scrn].kbbuf[ screens[scrn].kblen++ ] = ascii;
				else 
					{				
					netwrite( screens[scrn].port, screens[scrn].kbbuf, screens[scrn].kblen);	/* if full send buffer */
					screens[scrn].kbbuf[0]=ascii;
					screens[scrn].kblen=1;
					}

				sendch=ascii;
				parse( &screens[ scrn], &sendch, 1);
				if (!screens[scrn].halfdup)	
					return;								/* OK, were set...*/
	
				}
			 else					/* not printable char */
				{
				if ( code == BScode ) 
					{
					if (screens[scrn].kblen>0) 
						{
						screens[scrn].kblen--;
						parse( &screens[scrn],(unsigned char *) "\010 \010",3);	/* BYU LSC */
						}
					return;
					}
				else if (ascii == CR) 
					{
					char tt[50];
					
					netwrite( screens[scrn].port, screens[scrn].kbbuf, screens[scrn].kblen);
					sprintf(tt,"LINEMODE: length %i ",(int)screens[scrn].kblen);
					putln(tt);
					screens[scrn].kblen=0;
					}
	
				else if (ascii == KILLCHAR) 
					{
					while (screens[scrn].kblen >0) 
						{
						parse( &screens[scrn],(unsigned char *) "\010 \010",3);	/* BYU LSC */
						screens[scrn].kblen--;
						}
					return;
					}

				else if (code <KPlowest) 
					{
					netwrite( screens[scrn].port, screens[scrn].kbbuf, screens[scrn].kblen);	/* if full send buffer */
					screens[scrn].kblen=0;
					
					if (ascii != CR) {
						sendch='@'+ascii;
						parse( &screens[scrn],(unsigned char *) "^",1);
						parse( &screens[scrn], &sendch, 1);
						}
					}
				}
			}
			
	else		/* BYU = was: if (!screens[scrn].lmflag ) */
		{
		if (screens[scrn].echo && ! screens[scrn].halfdup)		/* Handle local ECHOs */
			if (ascii>31 && ascii <127 && code<KPlowest)	
				{
				if (screens[scrn].kblen < (MAXKB -1)) 	/* Add to buffer if not full */
					screens[scrn].kbbuf[ screens[scrn].kblen++ ] = ascii;
				else 
					{					
					netwrite( screens[scrn].port, screens[scrn].kbbuf,
					screens[scrn].kblen);	/* if full send buffer */	
					screens[scrn].kbbuf[ 0 ]=ascii;
					screens[scrn].kblen=1;
					}

				sendch=ascii;
				parse( &screens[ scrn], &sendch, 1);
				if (!screens[scrn].halfdup)	
					return;								/* OK, were set...*/
				}
			else 
				{
				if ( code == BScode ) 
					{
					if (screens[scrn].kblen>0) 
						{
						screens[scrn].kblen--;
						parse( &screens[scrn],(unsigned char *) "\010 \010",3);	/* BYU LSC */
						}
					return;
					}
				else if (ascii == KILLCHAR) 
					{
					while (screens[scrn].kblen >0) 
						{
						parse( &screens[scrn],(unsigned char *) "\010 \010",3);	/* BYU LSC */
						screens[scrn].kblen--;
						}
					return;
					}
		
				else if (code <KPlowest) 
					{
					netwrite( screens[scrn].port, screens[scrn].kbbuf,
					screens[scrn].kblen);	/* if full send buffer */
					screens[scrn].kblen=0;
					if (ascii !=CR) 
						{
						sendch='@'+ascii;
						parse( &screens[scrn],(unsigned char *) "^",1);	/* BYU LSC */
						parse( &screens[scrn], &sendch, 1);
						}
					}
				}
			}
			

	if (ascii == '\015') 						/* BYU 2.4.18 - changed \n to \015 */
		{		/* Map CR->CRLF */
		netpush( screens[scrn].port);
		
		//	If crmap is on, send CR-NULL instead of CR-LF.  If linemode is on, ignore crmap option.
		if (!screens[scrn].lmode && screens[scrn].crmap) { 
			netwrite(screens[scrn].port,"\015",1);
			ascii = 0;
			}
		else if (screens[scrn].lmode)
			{
			ascii = '\012';
			}
		else
			{
			netwrite(screens[scrn].port,"\015",1);
			ascii = '\012';
			}
			
		if (screens[scrn].echo) parse( &screens[scrn],(unsigned char *) "\012\015",2);	/* BYU LSC */
		}


	// Don't use SendOneChar here because we have to double ASCII 255 to escape its effect
	// as the IAC character.
	if (screens[scrn].echo && screens[scrn].halfdup) 
		{
		sendch=ascii;
		parse( &screens[scrn], &sendch, 1);
		}

	if (ascii != 255) {
		sendch= ascii;
		netwrite(screens[scrn].port,&sendch,1);
		}
	else
		netwrite(screens[scrn].port, "\377\377", 2);
		
	netpush(screens[scrn].port);					
}

void	HandleMouseDown(EventRecord myEvent)
{
	GrafPtr	whichWindow;
	short	code, myRGMnum;
	
	code = FindWindow(myEvent.where, &whichWindow);
	
	switch (code) {
		case inMenuBar:
			HandleMenuCommand(MenuSelect(myEvent.where),myEvent.modifiers);	/* BYU LSC */
			break;
		case inSysWindow:
			SystemClick(&myEvent, whichWindow);
			break;
		case inGoAway:
			if (TrackGoAway( whichWindow, myEvent.where))
				CloseAWindow((WindowPtr)whichWindow);
			break;

		case inDrag:
			if ((whichWindow != FrontWindow()) &&
					(!(myEvent.modifiers & cmdKey)) &&
					(!(myEvent.modifiers & optionKey))) {
				SelectWindow(whichWindow);
				}
			DragWindow(whichWindow, myEvent.where, &TelInfo->dragRect);
			break;

		case inZoomIn:
		case inZoomOut:
			if (TrackBox( whichWindow, myEvent.where, code))
				RSzoom( whichWindow, code, myEvent.modifiers & shiftKey);
			break;
	
	/* NCSA: SB - Telnet now allows you to grow the TEK window, finally.		*/
	/* NCSA: SB - So check to see if the click was in a TEK window				*/
	
		case inGrow:
			RSsize( whichWindow, (long *) &myEvent.where, myEvent.modifiers);
			myRGMnum = RGfindbywind(whichWindow);			/* NCSA: SB - is it a TEK window click? */
			if (myRGMnum  != -1)			/* NCSA: SB - Anyone want to play some BOLO? */
				{											/* NCSA: SB */
				RGMgrowme((short)myRGMnum, whichWindow,(long *) &myEvent.where,myEvent.modifiers);	/* NCSA: TG */
				}											/* NCSA: SB */
		break;
		
		case inContent:
			if (whichWindow != FrontWindow()) {
				SelectWindow(whichWindow);
				}
			else
				if (RSclick(whichWindow, myEvent) <0) {
					SetPort(whichWindow);
					GlobalToLocal(&myEvent.where);
					RGmousedown(whichWindow, &myEvent.where );
					}
			break;
		
		default:
			break;
	}
}

void	DoEvents( void)
{
	Boolean		gotOne;			/* Did we get an event */
	short		vs, i;
	EventRecord	myEvent;
	
	gotOne = WaitNextEvent(everyEvent, &myEvent, gApplicationPrefs->TimeSlice, 0L);

	if (gotOne) {

/* BYU 2.4.11 - Turn the cursor off when the human makes the slightest move. */
		if (gApplicationPrefs->BlinkCursor) {								/* BYU 2.4.11 */
			if ( (vs=RSfindvwind(FrontWindow())) >= 0)	/* BYU 2.4.11 */
				if (vs == screens[scrn].vs)				/* BYU 2.4.11 */
					if (!(myEvent.modifiers & cmdKey) &&	/* BYU 2.4.18 */
						((myEvent.what == keyDown) || (myEvent.what == autoKey))
						)
						RScursblinkon(vs);				/* BYU 2.4.18 */
					else								/* BYU 2.4.18 */
						RScursblinkoff(vs);				/* BYU 2.4.18 */
		}												/* BYU 2.4.11 */

		switch(myEvent.what) {
		case mouseDown:
			HandleMouseDown(myEvent);
			break;

		case updateEvt:
			switch(((WindowPeek)myEvent.message)->windowKind) {
				case WIN_CONSOLE:
				case WIN_LOG:
				case WIN_CNXN:
					if (RSupdate((GrafPtr) myEvent.message))
						putln("Ack, problem in update!");
					break;
				
				case WIN_ICRG:
					if (MacRGupdate((WindowPtr) myEvent.message))
						putln("Ack, problem in update!");
					break;
					
				case WIN_TEK:
					if (RGupdate((GrafPtr) myEvent.message) ==0) 
						TekDisable(RGgetVG((GrafPtr) myEvent.message));
					else
						putln("Ack, problem in update!");
					break;
				
				default:
					putln("Bad windowkind!");
					break;
				}			
			break;

		case keyDown:
		case autoKey:
			HandleKeyDown(myEvent);		/* All key events are processed through here */
			break;

		case diskEvt:			/* check to see if disk needs to be initialized */
			myEvent.where.h = 100;
			myEvent.where.v = 120;
			if (noErr != (( myEvent.message >> 16 ) & 0xffff )) {	/* check hi word */
				DILoad();
				DIBadMount( myEvent.where, myEvent.message);	/* BYU LSC */
				DIUnload();
			}
			break;

		case activateEvt:
			if ((myEvent.modifiers & activeFlag)==1) {
				AdjustMenus();
				DrawMenuBar();
				i=WindowPtr2ScreenIndex((GrafPtr) myEvent.message);	/* We need to know who */
				if (i>=0) {
					if ((screens[i].curgraph>-1) && (!(myEvent.modifiers & optionKey)))
						detachGraphics(screens[i].curgraph);
					changeport(scrn,i);
					scrn=i;
					}
				if ((i=RSfindvwind((GrafPtr) myEvent.message))>=0) {
					if (RSTextSelected(i)) {					/* BYU 2.4.11 */
						EnableItem(myMenus[Fil],FLprint);		/* BYU 2.4.11 */
						EnableItem(myMenus[Edit],EDcopy);		/* BYU 2.4.11 */
						EnableItem(myMenus[Edit],EDcopyt);		/* BYU 2.4.11 */
					} else {									/* BYU 2.4.11 */
						DisableItem(myMenus[Fil],FLprint);		/* BYU 2.4.11 */
						DisableItem(myMenus[Edit],EDcopy);		/* BYU 2.4.11 */
						DisableItem(myMenus[Edit],EDcopyt);		/* BYU 2.4.11 */
					}											/* BYU 2.4.11 */
					RSactivate(i);
					TelInfo->myfrontwindow=(WindowPeek) myEvent.message;
					TelInfo->myfronttype=DEC_WINDOW;
					TelInfo->myfrontvs = i;
					TelInfo->myfrontRgn =0L;
					updateCursor(1);
				} else {					
					TelInfo->myfrontwindow=(WindowPeek) myEvent.message;
					TelInfo->myfronttype=TEK_WINDOW;
					TelInfo->myfrontRgn =0L;
					updateCursor(1);
					if ( (i = RGgetdnum((GrafPtr) myEvent.message)) >-1) {
						if (( i = RGgetVS( i)) >-1) {
							EnableItem(myMenus[Fil],FLprint);	/* BYU 2.4.11 - enable printing */
							EnableItem(myMenus[Edit],EDcopy);	/* BYU 2.4.11 - enable copying */
							DisableItem(myMenus[Edit],EDcopyt);	/* BYU 2.4.11 */
							i = findbyVS( i);
							changeport(scrn,i);
							scrn=i;
							}
						}
				}
			} else {
				short i;

//				putln("disable event");
				AdjustMenus();
				DrawMenuBar();
				if ((i=RSfindvwind((GrafPtr) myEvent.message))>=0)
					RSdeactivate(i);
				NoWindow();
				}
			break;
		case app4Evt:
			switch(( myEvent.message >>24) &0xff) {		/* App4 is a multi-event event */
				case switchEvt:
					if (myEvent.message & 0x20)
						/*Convert clipboard here if necc. (it is not)*/;

					if (myEvent.message & 0x1) {		/* Resume Event */
						GrafPtr window;

						TelInfo->suspended = FALSE;				/* We are no longer in suspension */
						DisableItem( myMenus[Edit],EDcut);
						DisableItem( myMenus[Edit],EDundo);
						DisableItem( myMenus[Edit],EDclear);

						window = FrontWindow();			/* Who's on first */
						if ( (vs=RSfindvwind(window)) >= 0) {
							RSactivate(vs);
							TelInfo->myfrontwindow = (WindowPeek) window;
							TelInfo->myfronttype=DEC_WINDOW;
							TelInfo->myfrontvs = vs;
							TelInfo->myfrontRgn =0L;
							updateCursor(1);
						} else if ( (long)window != 0L) {
							myEvent.message = (long) window;
							myEvent.modifiers |= activeFlag;
							myEvent.what = activateEvt;
							myEvent.when = TickCount();
							SystemEvent( &myEvent);
							}
						}
					else {								/* Suspend Event */
						GrafPtr window;

						TelInfo->suspended=TRUE;					/* We be in waitin' */
						EnableItem( myMenus[Edit],EDcut);
						EnableItem( myMenus[Edit],EDundo);
						EnableItem( myMenus[Edit],EDclear);

						window = FrontWindow();			/* Who's on first */
						if ((window = FrontWindow())) {
							if ( (vs=RSfindvwind(window)) >= 0)
								RSdeactivate(vs);
							else if ( (long)window != 0L) {
								myEvent.message = (long) window;
								myEvent.modifiers &= (~activeFlag);
								myEvent.what = activateEvt;
								myEvent.when = TickCount();
								SystemEvent( &myEvent);
								}
							}
						NoWindow();
						}
					break;			/* switch of myEvent.message >>24 */
				}
			break;
		case kHighLevelEvent:
			(void) AEProcessAppleEvent(&myEvent);
			break;			
		}

	} else if (gApplicationPrefs->BlinkCursor && !TelInfo->suspended) {	/* BYU 2.4.11 */
		if ( (vs=RSfindvwind(FrontWindow())) >= 0)		/* BYU 2.4.11 */
			if (vs == screens[scrn].vs)					/* BYU 2.4.11 */
				RScursblink(vs);						/* BYU 2.4.11 */
	}													/* BYU 2.4.11 */
	updateCursor(0);
}

void	CloseAWindow(WindowPtr	theWindow)
{
	short	i;
	
	switch(((WindowPeek)theWindow)->windowKind) {
		case WIN_CONSOLE:
//			Debugger();				// Can't close the console window
			break;
					
		case WIN_LOG:
			if (theWindow == ftplog->wind) {
				CheckItem(myMenus[Fil],FLlog,FALSE);
				TelInfo->ftplogon = FALSE;
				RShide(ftplog->vs);
			}
			break;
					
		case WIN_CNXN:
			if ((i = WindowPtr2ScreenIndex(theWindow)) >= 0) {
				if ( screens[i].active == CNXN_ISCORPSE)
					destroyport(i);
				else {
					if ( !ReallyClose( i) ) break;
					netclose(screens[i].port);
					removeport(i);
					}
				}
			break;
		
		case WIN_ICRG:
			MacRGdestroy(MacRGfindwind(theWindow));
			break;
			
		case WIN_TEK:
			destroyGraphics(RGgetVG(theWindow));
			break;
		
		default:
			DebugStr("\pBad windowkind!");
			break;
		}			

		NoWindow();
}




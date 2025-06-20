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
****************************************************************/

#ifdef MPW
#pragma segment 7
#endif

#include <string.h>

#include "TelnetHeader.h"
#include "vsdata.h"
#include "vsinterf.proto.h"					/* For VSwrite proto */
#include "wind.h"						/* For WindRec structure */
#include "debug.h"
#include "parse.proto.h"		// For DemangleLinemodeShort
#include "rsmac.proto.h"		// For RSshow proto

WindRec	*console;		/*	Window Record (VS) for :	console Window   */
extern WindRec	*screens;

//#define DEBUG_FACILITIES

#ifdef SUPPORT_AUDIT
#include "Audit.h"
AuditPtr	gAuditPtr = NULL;
#endif

void InitDebug(void)
{
#ifdef DEBUG_FACILITIES
#ifdef SUPPORT_AUDIT
	gAuditPtr = GetAuditPtr('NCSA');
#endif
	Rect pRect;

	console = (WindRec *) NewPtrClear(sizeof(WindRec));
	
	SetRect(&pRect, 50, 150, 700, 350);		// Need to make this a resource!
	
	console->vs=RSnewwindow( &pRect, 350, 80, 24,
					"\p<console>", 1, DefFONT, DefSIZE, TelInfo->debug,0,0);	/* NCSA 2.5 */

	console->wind = RSgetwindow( console->vs);
	((WindowPeek)console->wind)->windowKind = WIN_CONSOLE;

	VSwrite(console->vs,"\033[24;0H",1);		
	console->active=0;
	console->port=0;
	console->termstate=VTEKTYPE;
	console->national = 0;			/* LU: no translation */
#else
	console = NULL;
#endif
}

void putln( char *cp)								
{
#ifdef DEBUG_FACILITIES
	short temp;
	Str255	temppstr;
	
	strcpy((char *)temppstr, cp);
	CtoPstr((char *)temppstr);
	
#ifdef SUPPORT_AUDIT
	AuditString(gAuditPtr, 'PTLN', temppstr);
#endif
	
	if (!TelInfo->debug)
		return;

	temp=strlen(cp);
	if (temp>80) return;
	VSwrite(console->vs,cp,temp);
	VSwrite(console->vs,"\015\012",2);
#endif
}

// Called by HandleKeyDown.  Allows me to insert debug info keys all in one place
//	that can be easily #defined out for release versions.  Returns TRUE if
//	HandleKeyDown should do an immediate return after calling us.
Boolean	DebugKeys(Boolean cmddwn, unsigned char ascii, short s)
{
#ifdef DEBUG_FACILITIES
	if (cmddwn && (ascii == ';')) {	// 2.6b16.1
		char hackhackhack[80];
		
		strcpy(hackhackhack, "Linemode: ");
		DemangleLineModeShort(hackhackhack, screens[s].lmode);
		putln(hackhackhack);
		return(FALSE);
		}
#endif
	return (FALSE);
}

void	ShowDebugWindow(void)
{
#ifdef DEBUG_FACILITIES
	if (console != NULL)
		RSshow(console->vs);
#endif
}
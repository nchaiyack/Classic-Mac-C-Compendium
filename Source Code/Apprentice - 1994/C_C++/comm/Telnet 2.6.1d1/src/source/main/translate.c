/*
*	translate.c
*	written by Roland MŒnsson, Lund University Computing Center, Sweden
*	roland_m@ldc.lu.se
*	July 1992
*
*	Modified by Pascal Maes
*	UCL/ELEC
*	Place du Levant, 3
*	B-1348 Louvain-la-Neuve
*****************************************************************
*	Part of:													*
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1993,											*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*	Modified 7/93 by Jim Browne for NCSA.
*/

#ifdef MPW
#pragma segment 4
#endif

#include <stdio.h>

#include "TelnetHeader.h"
#include "debug.h"
#include "wind.h"
#include "vsdata.h"
#include "telneterrors.h"
#include "translate.proto.h"
#include "vsinterf.proto.h"

//#define	DEBUG_TRANSLATION
/*************** external variables ***************/


extern 	WindRec *screens;		/* The screen array from maclook.c */
extern	short scrn;				/* The current screen from maclook.c */

/*************** global variables ***************/

BytePtr	DefaultTable,
		FTPinTable,
		FTPoutTable;
		
Handle	transTablesHdl;
short	nNational;

#if 0
Boolean get_trsl (short id, Byte **table)
{
	Handle h;
	long size;

	h = GetResource (TRSL,id);

	if ((h==NULL) || (ResError()!=noErr)) 
		{
		DoError(106 | RESOURCE_ERRORCLASS, LEVEL2, NULL);
		return (FALSE);
		}
		
	size = GetHandleSize(h);
	if (size != 256) 
		{
		DoError(107 | RESOURCE_ERRORCLASS, LEVEL2, NULL);
		return (FALSE);
		}

	HLockHi(h);
	*table = (Byte *) *h;
	return (TRUE);
}
#endif

short	transBuffer(short oldtable, short newtable)		/* translate entire buffer */
{
	VSscrn	*vsscreen;
	VSline	*vslin,*p;
	short		lineNo,maxLineNo;
	short		width;				/* allocated witdth of window (80/132) */
	char		tmp[80];				/* only for debugging */
	
	vsscreen = VSwhereis (screens[scrn].vs);
	vslin = vsscreen->buftop;

#ifdef DEBUG_TRANSLATION
	putln ("in transBuffer, well and alive");
	sprintf (tmp,"VSgetlines(screens[scrn].vs):%d", VSgetlines(screens[scrn].vs)); putln (tmp);
	sprintf (tmp,"VSmaxwidth(screens[scrn].vs):%d", VSmaxwidth(screens[scrn].vs)); putln (tmp);
	sprintf (tmp,"vsscreen->lines:%d\n",vsscreen->lines); putln (tmp);
	sprintf (tmp,"vsscreen->maxlines:%d\n",vsscreen->maxlines); putln (tmp);
	sprintf (tmp,"vsscreen->numlines:%d\n",vsscreen->numlines); putln (tmp);
	sprintf (tmp,"vsscreen->allwidth:%d\n",vsscreen->allwidth); putln (tmp);
	sprintf (tmp,"vsscreen->maxwidth:%d\n",vsscreen->maxwidth); putln (tmp);
	sprintf (tmp,"vsscreen:%08x\n",vsscreen);  putln (tmp);
	sprintf (tmp,"vslin:%08x\n",vslin);  putln (tmp);
	sprintf (tmp,"next:%08x\n",vslin->next);  putln (tmp);
	sprintf (tmp,"prev:%08x\n",vslin->prev);  putln (tmp);
	sprintf (tmp,"text:%08x\n\n",vslin->text);  putln (tmp);
#endif

	width = VSmaxwidth(screens[scrn].vs)+1;	/* VSmaxwidth returns 79 or 131 */
	p = vslin;
	maxLineNo = vsscreen->numlines+VSgetlines(screens[scrn].vs); /* VSgetlines returns 24 or whatever */
	for (lineNo=1; lineNo<=maxLineNo; lineNo++) {
		/*sprintf(tmp,"lineNo:%d, p:%08x, starts with:%c%c%c%c",lineNo,p,*(p->text),*(p->text+1),*(p->text+2),*(p->text+3)); putln(tmp);*/
		if (p==NULL) { putln ("p is NULL"); return (-1); }
		if (p->text==NULL) { putln ("p->text is NULL"); return (-1); }
		
		// First convert the line back to Mac US format, and then to the new format.
		trbuf_nat_mac((unsigned char *)p->text,width, oldtable);
		trbuf_mac_nat((unsigned char *)p->text,width, newtable);
		
		p = p->next;
	}
	sprintf (tmp, "transBuffer:did convert %d lines", lineNo-1); putln (tmp);
}

BytePtr		GetTranslationResource(short id)
{
	Handle	h;
	
	h = GetResource(MY_TRSL, id);
	
	if ((h == NULL) || (ResError() != noErr)) {
		// Do nasty mean error here. BUGG
		}
	
	DetachResource(h);
	HLockHi(h);
	return((BytePtr) *h);
}

//	table #'s 1...n correspond to tables in our master array, table #0 is the default table
BytePtr		ReturnTablePtr(short table, Boolean out)
{
	if (table > nNational || table < 1) return(DefaultTable + ((out == TRUE) * 256));
	return((BytePtr)(*transTablesHdl + ((table - 1) * 512) + ((out == TRUE) * 256)));
}

//	The Default table (i.e. no translation) and the two FTP tables are stored in the 
//	 Application's resource fork as resources of type TRSL.  The
//	 tables added and removed by the user are stored in the prefs file as resources of
//	 type taBL.  This routine loads the default table and the two FTP tables into memory.
//	 Failures can only be caused by an incorrect application resource fork.
void	Setup_Default_Tables(void)
{
	DefaultTable = GetTranslationResource(TRSL_DEFAULT_TABLE);
	FTPinTable = GetTranslationResource(TRSL_FTP_TABLE);
	FTPoutTable = FTPinTable + 256;
}

/* 
*	Be very careful with calling putln from this procedure, since
*	putln uses the translation tables. If the tables are not setup
*	garbage output will appear. This is not harmful, but very
*	annoying.
*/

void trInit (MenuHandle	whichMenu)
{
	short	i, nRestaBL;		/* mp: # of resources of type taBL */
	Handle	h;
	short	rsrcID;
	ResType	rsrcType;
	Str255	rsrcName;
	
		
 	nNational = 0;
	Setup_Default_Tables();
	transTablesHdl = NewHandle(0);
	
	if (nRestaBL = CountResources(USER_TRSL)) {
		for (i = 1; i <= nRestaBL; i++)
		{
			h = GetIndResource(USER_TRSL, i);
			if (ResError() == noErr && (GetHandleSize(h) == 512)) {
				GetResInfo(h, &rsrcID, &rsrcType, rsrcName);
				nNational++;

				// Insert the table's name in the Translation menu
				AppendMenu(whichMenu, "\pDoh");
				SetItem(whichMenu, nNational+1, rsrcName);	// No metas!
				
				// Now append the table's data to the master array of table data
				HUnlock(transTablesHdl);
				SetHandleSize(transTablesHdl, (nNational * 512));
				HLockHi(transTablesHdl);
				HLock(h);
				BlockMove(*h, (*transTablesHdl) + ((nNational - 1) * 512), 512);
				
				// Release the resource
				ReleaseResource(h);
				}
		}

	}
	
	CalcMenuSize(whichMenu);
}


/*	Converts a char from 8-bit National to 8-bit Macintosh */
unsigned char	nat_mac(unsigned char *ascii, short table)
{
	short	b;
	BytePtr	table_data = ReturnTablePtr(table, FALSE);
	
	b = (short) *ascii;
	*ascii = table_data[b];
	return (*ascii);
}

unsigned char	ftp_iso_mac(unsigned char *ascii)
{
	short	b;
	
	b = (short) *ascii;
	*ascii = FTPinTable[b];
	return (*ascii);
}

void	trbuf_nat_mac(unsigned char *buf, short len, short table)
{
	short			i;
	unsigned char	ascii;
	unsigned char	*p;

	for (i=0,p=buf; i<len; i++,p++)
	{
		ascii = *p;
		*p = nat_mac(&ascii, table);
	}
			
}

void	trbuf_ftp_mac(unsigned char *buf, short len)
{
	short			i;
	unsigned char	ascii;
	unsigned char	*p;

	for (i=0,p=buf; i<len; i++,p++)
	{
		ascii = *p;
		*p = ftp_iso_mac(&ascii);
	}
			
}


/*	Converts a char from 8-bit Macintosh to 8-bit National */
unsigned char	mac_nat(unsigned char *ascii, short table)
{
	short	b;
	BytePtr	table_data = ReturnTablePtr(table, TRUE);
	
	b = (short) *ascii;
	*ascii = table_data[b];
	return (*ascii);
}

unsigned char	ftp_mac_iso(unsigned char *ascii)
{
	short	b;
	
	b = (short) *ascii;
	*ascii = FTPoutTable[b];
	return (*ascii);
}

void	trbuf_mac_nat(unsigned char *buf, short len, short table)
{
	short			i;
	unsigned char	ascii;
	unsigned char	*p;
	
	for (i=0,p=buf; i<len; i++,p++)
	{
		ascii = *p;
		*p = mac_nat(&ascii, table);
	}
}

void	trbuf_mac_ftp(unsigned char *buf, short len)
{
	short			i;
	unsigned char	ascii;
	unsigned char	*p;
	
	for (i=0,p=buf; i<len; i++,p++)
	{
		ascii = *p;
		*p = ftp_mac_iso(&ascii);
	}
}
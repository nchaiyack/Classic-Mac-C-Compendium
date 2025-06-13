/*
 *
 *      Virtual Screen Kernel Emulation Routines
 *                      (vsem.c)
 *  
 *   National Center for Supercomputing Applications
 *      by Gaige B. Paulsen
 *
 *    This file contains the private emulation calls for the NCSA
 *  Virtual Screen Kernel.
 *
 *-----------------------------------------------------------------------------------
 *	This also now contains Roland Mansson's code for printer redirection
 *	Thanks for all of the work, Roland!
 *-----------------------------------------------------------------------------------
 *
 *      Version Date    Notes
 *      ------- ------  ---------------------------------------------------
 *      0.01    861102  Initial coding -GBP
 *      0.10    861111  Added/Modified VT emulator -GBP
 *      0.50    861113  First compiled edition -GBP
 *		2.1		871130	NCSA Telnet 2.1 -GBP
 *		2.2 	880715	NCSA Telnet 2.2 -GBP
 *		2.6		12/92	Telnet 2.6: Fixed some VS stuff, and added LU changes
 */
 
#ifdef MPW
#pragma segment VS
#endif

#include <Printing.h>
#include <string.h>
#include <stdio.h>

#include "TelnetHeader.h"					/* LU - need normcurs definition */
#include "wind.h"
#include "vsdata.h"
#include "vskeys.h"
#include "rsmac.proto.h"
#include "vsintern.proto.h"
#include "vsinterf.proto.h"
#include "translate.proto.h"
#include "debug.h"
#include "maclook.proto.h"
#include "printing.proto.h"
#include "event.proto.h"
#include "menuseg.proto.h"

#include "vsem.proto.h"

extern THPrint PrRecHandle;		/* LU - our print record handle from menu.c */
extern SysEnvRec theWorld;		/* LU - System Environment record from environ.c */
extern Cursor *theCursors[];
extern long		TempItemsDirID;
extern short	TempItemsVRefNum;
extern WindRec *screens;

void	VSunload(void) {}

/* LU - this is the start of the main LU changes for doing printer redirection 	*/
/* 	Once again I would like to thank Roland Mansson, of the Lund University 
	Computing Center (Sweden) for all of his work on Telnet.   Not only for the 
	national char support, but also for the printer redirection support, a new 
	version of PrintPages, and everything else.  Thanks a lot, and keep up the
	good work!!!		-SMB													*/
/* LU - we have gotten the escape sequence to turn on printer redirection, so do so */ 
void VSprON(void)
{
	char	tmp[100];				/* only for debugging */
	OSErr	sts;
	
	putln ("printer redirection ON");
	VSIw->prredirect = 1;
	VSIw->prbuf = 0x00000000;
	sprintf (VSIw->fname,"NCSA Telnet tempfile #%d",VSIwn);
	c2pstr(VSIw->fname);									/* BYU 2.4.18 */

	if (sts = HCreate(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname, '????', 'TEXT')) {
		if (sts != dupFNErr) {
			SysBeep(1);
			VSIw->prredirect = 0;
			sprintf(tmp,"Create: ERROR %d",sts);
			putln(tmp);
			return;
			}
		}
	if (sts = HOpen(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname, fsRdWrPerm, &(VSIw->refNum))) {
		SysBeep(1);
		VSIw->prredirect = 0;
		sprintf(tmp,"FSOpen: ERROR %d",sts);
		putln(tmp);
		HDelete(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname);
		return;
		}
	if (SetEOF(VSIw->refNum, 0L)) {
		SysBeep(1);
		VSIw->prredirect = 0;
		putln("VSPRON:SETEOF ERROR");
		HDelete(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname);
		return;
		}
}


/* LU - we just got the escape sequence to turn OFF redirection.  Take what we have
		and dump it to the printer */
		
void VSprOFF(void)
{
	Str255		Title;
	TPrStatus	prStatus;	/* Status record */
	TPPrPort	prPort;		/* the Printer port */
	OSErr		sts;
	GrafPtr		savePort;
	char		tmp[100];	/* only for debugging */
	short		temp;		/* NCSA: SB - the screen # */
	THPrint		PrRecHandle;
	
	putln ("printer redirection OFF");
	if (VSIw->prredirect==0)			/* no redirection started! */
		return;
	VSIw->prredirect = 0;
	GetPort (&savePort);				/* save old port */

	PrOpen();
	
	PrRecHandle = PrintSetupRecord();
	
	GetWTitle ((GrafPtr) RSgetwindow(VSIwn),Title);
	SetCursor(theCursors[normcurs]);
	
	if (PrJobDialog(PrRecHandle)) {			/* Cancel the print if FALSE */
		if (sts=PrError()) { sprintf(tmp,"PrJobDialog: ERROR %d",sts); putln(tmp); }
		prPort=PrOpenDoc(PrRecHandle,0L,0L);
		if (sts=PrError()) {
			SysBeep(1);
			sprintf(tmp,"PrOpenDoc: ERROR %d",sts); putln(tmp);
		} else {
			temp = findbyVS(VSIwn);					/* NCSA: SB */
			if (temp < 0)	{
				PrClose();					/* NCSA: SB */
				DisposeHandle((Handle)PrRecHandle);
				return;							/* NCSA: SB */
				}
			printPages (prPort, PrRecHandle, Title, VSmaxwidth(VSIwn), NULL, VSIw->refNum, 0L,temp);
			PrCloseDoc(prPort);
			if (sts=PrError()) { sprintf(tmp,"PrCloseDoc: ERROR %d",sts); putln(tmp); }
			if (((*PrRecHandle)->prJob.bJDocLoop == bSpoolLoop) && (PrError()==0)) {
				PrPicFile(PrRecHandle,0L,0L,0L,&prStatus); /* Spool if necessaryÉ */
				if (sts=PrError()) { sprintf(tmp,"PrPicFile: ERROR %d",sts); putln(tmp); }
			}
		}
	}
	
	PrClose();
	DisposeHandle((Handle)PrRecHandle);
	
	SetPort (savePort);				/* restore old port */
	if (sts=FSClose (VSIw->refNum)) {
		SysBeep(1);
		sprintf(tmp,"FSClose: ERROR %d",sts); putln(tmp);
	}
	VSIw->refNum = -1;

	if (sts=HDelete(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname)) {
		SysBeep(1);
		sprintf(tmp,"HDelete: ERROR %d",sts); putln(tmp);
	}
	updateCursor(1);
}


#define ENDOFPRT 	'\033[4i'		/* <ESC>[4i   (0x1b5b3469) */

void VSpr(unsigned char **pc, short *pctr)
{
	long count;				/* number of chars to print to file */
	char *start; 			/* original start of buffer */
	OSErr sts;
	char tmp[100];			/* only for debugging */
	short rdy;				/* true if <ESC>[4i */

	count=0;				
	start=(char *)*pc;	
	rdy=0;

	while ((*pctr>0) && (!rdy)) {
		VSIw->prbuf=(VSIw->prbuf<<8) + **pc;
		if (VSIw->prbuf==ENDOFPRT) {			/* i.e. no more redirection */
			rdy=1;
			count--;					/* will be incremented again below */
		}
		count++;
		(*pc)++;
		(*pctr)--;
	}

	trbuf_nat_mac ((unsigned char *)start,count, screens[findbyVS(VSIwn)].national);
	if (sts=FSWrite(VSIw->refNum,&count,start)) {
		SysBeep(1);
		sprintf(tmp,"FSWrite: ERROR %d",sts); putln(tmp);
	}
	if (rdy || sts)
		VSprOFF();
}



/* LU - that is the end of the new routines needed for printer redirection 	*/
/* LU - now we just patch up VSem() to use this code, and were done! 		*/







void VSem
  (
	unsigned char *c, /* pointer to character string */
	short ctr /* length of character string */
  )
  /* basic routine for placing characters on a virtual screen, and
	interpreting control characters and escape sequences. Simple
	interpretation of controls & escapes is done here, while the
	harder stuff is done by calling VSIxx routines in vsintern.c. */
{
    register short sx;
    register short escflg; /* state of escape sequence interpretation */
    short insert, attrib, extra, offend;
    char *acurrent, *current, *start;

    escflg = VSIw->escflg;

    while (ctr > 0)
	  {
		if (VSIw->prredirect)	/* PR - printer redirection? */
		VSpr(&c,&ctr);			/* PR -if yes, call VSpr */
								/* PR - when we return from VSpr there may (ctr!=0) É */
								/* PR - É or may not (ctr==0) be chars left in *c to print */
		while ((escflg == 0) && (ctr > 0) && (*c < 32))
		  {
			switch (*c)
			  {
				case 0x1b: /* esc */
					escflg++;
					break;
				case 0x0e: /* shift out */
					if (VSIw->G1)
						VSIw->attrib = VSgraph(VSIw->attrib);
					else
						VSIw->attrib = VSnotgraph(VSIw->attrib);
					VSIw->charset = 1;
					break;
				case 0x0f: /* shift in */
					if (VSIw->G0)
						VSIw->attrib = VSgraph(VSIw->attrib);
					else
						VSIw->attrib = VSnotgraph(VSIw->attrib);
					VSIw->charset = 0;
					break;
				case 0x07: /* bell */
					RSbell(VSIwn);
					break;
				case 0x08: /* backspace */
					VSIw->x--;
					if (VSIw->x < 0)
					  /* hit left margin */
						VSIw->x = 0;
					break;
				case 0x0c: /* ff */
					VSIindex();
					break;
				case 0x09: /* ht */		/* Later change for versatile tabbing */
					VSItab();
					VScapture(c,1);				/* BYU 2.4.18 */
					break;
				case 0x0a: /* lf */
					VSIindex();
					break;
				case 0x0d: /* cr */
					VSIw->x = 0;
					VScapture(c,1);				/* BYU 2.4.18 */
					break;
				case 0x0b: /* vt */
					VSIindex();
					break;
#ifdef CISB
				case 0x10: /* dle */
					bp_DLE(c, ctr);
					ctr = 0;
					break;
				case 0x05: /* enq */
					bp_ENQ();
					break;
#endif CISB
			  } /* switch */
			c++;
			ctr--;
		  } /* while */
		if ((escflg == 0) && (ctr > 0) && (*c & 0x80) && (*c < 0xA0)
			&& (screens[findbyVS(VSIwn)].vtemulation == 1))	/* BYU 2.4.12 - VT220 starts here */
		  {												/* BYU 2.4.12 */
			switch (*c)									/* BYU 2.4.12 */
			  {											/* BYU 2.4.12 */
				case 0x84: /* ind */			/* BYU 2.4.12 - same as ESC D */
					VSIindex();					/* BYU 2.4.12 */
					goto ShortCut;				/* BYU 2.4.12 */
				case 0x85: /* nel */			/* BYU 2.4.12 - same as ESC E */
					VSIw->x = 0;				/* BYU 2.4.12 */
					VSIindex();					/* BYU 2.4.12 */
					goto ShortCut;				/* BYU 2.4.12 */
				case 0x88: /* hts */			/* BYU 2.4.12 - same as ESC H */
					VSIw->tabs[VSIw->x] = 'x';	/* BYU 2.4.12 */
					goto ShortCut;				/* BYU 2.4.12 */
				case 0x8d: /* ri */				/* BYU 2.4.12 - same as ESC M */
					VSIrindex();				/* BYU 2.4.12 */
					goto ShortCut;				/* BYU 2.4.12 */
				case 0x9b: /* csi */			/* BYU 2.4.12 - same as ESC [ */
					VSIapclear();				/* BYU 2.4.12 */
					escflg = 2;					/* BYU 2.4.12 */
					break;						/* BYU 2.4.12 */
				case 0x86: /* ssa */			/* BYU 2.4.12 - same as ESC F */
				case 0x87: /* esa */			/* BYU 2.4.12 - same as ESC G */
				case 0x8e: /* ss2 */			/* BYU 2.4.12 - same as ESC N */
				case 0x8f: /* ss3 */			/* BYU 2.4.12 - same as ESC O */
				case 0x90: /* dcs */			/* BYU 2.4.12 - same as ESC P */
				case 0x93: /* sts */			/* BYU 2.4.12 - same as ESC S */
				case 0x96: /* spa */			/* BYU 2.4.12 - same as ESC V */
				case 0x97: /* epa */			/* BYU 2.4.12 - same as ESC W */
				case 0x9d: /* osc */			/* BYU 2.4.12 - same as ESC ] */
				case 0x9e: /* pm */				/* BYU 2.4.12 - same as ESC ^ */
				case 0x9f: /* apc */			/* BYU 2.4.12 - same as ESC _ */
					goto ShortCut;				/* BYU 2.4.12 */
			  } /* switch */					/* BYU 2.4.12 */
/*
 * Not sure why this code was here. It causes Telnet to ignore printable chars
 * >= 0x80. RW. 5/27/93
 */
#ifdef notdef
			c++;								/* BYU 2.4.12 */
			ctr--;								/* BYU 2.4.12 */
#endif
		  } /* if */							/* BYU 2.4.12 */
		while ((ctr > 0) && (escflg == 0) && (*c >= 32))
		  {
		  /* display printing characters */
			start = &VSIw->linest[VSIw->y]->text[VSIw->x]; /* start of area needing redrawing */
			current = start; /* where to put next char */
			acurrent = &VSIw->attrst[VSIw->y]->text[VSIw->x]; /* where to put corresponding attribute byte */
			attrib = VSIw->attrib; /* current writing attribute */
			insert = VSIw->IRM; /* insert mode (boolean) */
			offend = 0; /* wrapped to next line (boolean) */
			extra = 0; /* overwriting last character of line (boolean) */
			sx = VSIw->x; /* starting column of area needing redrawing */
			if (VSIw->x > VSIw->maxwidth)
			  {
				if (VSIw->DECAWM)
				  {
				  /* wrap to next line */
					VSIw->x = 0;
					VSIindex();
				  }
				else
				  /* stay at right margin */
					VSIw->x = VSIw->maxwidth;
				current = start = &VSIw->linest[VSIw->y]->text[VSIw->x];
				acurrent = &VSIw->attrst[VSIw->y]->text[VSIw->x];
				sx = VSIw->x;
			  } /* if */
			while ((ctr > 0) && (*c >= 32) && (offend == 0))
			  {
			  trbuf_nat_mac(c,1, screens[findbyVS(VSIwn)].national);			/* LU/MP: translate to national chars */
			  /* write characters within a single line */
				if (insert)
				  /* make room for the char */
					VSIinschar(1);
			  /* poke the character and its attribute into the
				screen buffer at the current cursor position */
				*current = *c;
				*acurrent = attrib;
				c++;
				ctr--;
				if (VSIw->x < VSIw->maxwidth)
				  {
				  /* advance the cursor position */
					acurrent++;
					current++;
					VSIw->x++;
				  }
				else
				  {
				  /* hit right margin */
					if (VSIw->DECAWM)
					  {
					  /* autowrap to start of next line */
						VSIw->x++;
						offend = 1; /* terminate inner loop */
					  }
					else
					  {
					  /* stay at right margin */
						VSIw->x = VSIw->maxwidth;
						extra = 1; /* cursor position doesn't advance */
					  } /* if */
				  } /* if */
			  } /* while */
		  /* update the screen to show what I've done */
		  	extra += VSIw->x - sx + offend;									/* BYU 2.4.18 */
			if (insert)
				VSIinsstring(extra, start);									/* BYU 2.4.18 */
									/* actually just decides which RS to use */
			else
				VSIdraw(VSIwn, sx, VSIw->y, VSIw->attrib, extra, start);	/* BYU 2.4.18 */
			VScapture((unsigned char *) start, extra);						/* BYU 2.4.18 */
		  } /* while */

		while((ctr > 0) && (escflg == 1))
		  { /* basic escape sequence processing */
			switch (*c)
			  {
				case 0x08:
					VSIw->x--;
					if (VSIw->x < 0)
						VSIw->x = 0;
					break;
				case '[': /* csi */
					VSIapclear();
					escflg++;
					break;
				case '7':
					VSIsave();
					goto ShortCut;				/* BYU 2.4.12 */
				case '8':
					VSIrestore();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'c':
					VSIreset();
					break;
				case 'D':
					VSIindex();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'E':
					VSIw->x = 0;
					VSIindex();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'M':
					VSIrindex();
					goto ShortCut;				/* BYU 2.4.12 */
				case '>':
					VSIw->DECPAM = 0;
					goto ShortCut;				/* BYU 2.4.12 */
				case '=':
					VSIw->DECPAM = 1;
					goto ShortCut;				/* BYU 2.4.12 */
				case 'Z':
					VTsendident();
					goto ShortCut;				/* BYU 2.4.12 */
				case ' ':						/* BYU 2.4.12 */
				case '*':						/* BYU 2.4.12 */
				case '#':
					escflg = 3;
					break;
				case '(':
					escflg = 4;
					break;
				case ')':
					escflg = 5;
					break;
				case 'H':
					VSIw->tabs[VSIw->x] = 'x';
					goto ShortCut;				/* BYU 2.4.12 */
#ifdef CISB
				case 'I':
					bp_ESC_I();
					break;
#endif CISB
				case ']':								// WNR - XTerm
					if (screens[findbyVS(VSIwn)].Xterm)	// WNR - XTerm
						escflg = 6;						// WNR - XTerm
					break;								// WNR - XTerm
					
				default:
					goto ShortCut;				/* BYU 2.4.12 */
			  } /* switch */
			c++;
			ctr--;
		  } /* while */
		while ((escflg == 2) && (ctr > 0))
		  { /* "control sequence" processing */
			switch (*c)
			  {
				case 0x08:
					VSIw->x--;
					if (VSIw->x < 0)
						VSIw->x = 0;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				  /* parse numeric parameter */
					if (VSIw->parms[VSIw->parmptr] < 0)
						VSIw->parms[VSIw->parmptr] = 0;
#if 0 /* MPW code generator bug */
					VSIw->parms[VSIw->parmptr] *= 10;
#else
					VSIw->parms[VSIw->parmptr] = VSIw->parms[VSIw->parmptr] * 10;
#endif
					VSIw->parms[VSIw->parmptr] += *c - '0';
					break;
				case '?':
				  /* DEC-private control sequence */
					VSIw->parms[VSIw->parmptr++] = -2;
					break;
				case ';':
				  /* parameter separator */
					VSIw->parmptr++;
					break;
				case 'A': /* cursor up */
#if 1														/* BYU */
					if (VSIw->parms[0]<1) VSIw->y--;		/* BYU */
					else VSIw->y-=VSIw->parms[0];			/* BYU */
					if ( VSIw->y < 0 ) VSIw->y=0;			/* BYU */
					if (VSIw->y < VSIw->top)				/* NCSA: SB */
						VSIw->y = VSIw->top;				/* NCSA: SB */
						
#else														/* BYU */
					if (VSIw->y < VSIw->top)
					  {
					  /* outside scrolling region */
						if (VSIw->parms[0] < 1)
							VSIw->y--;
						else
							VSIw->y -= VSIw->parms[0];
					  }
					else
					  {
					  /* don't leave scrolling region */
						if (VSIw->parms[0] < 1)
							VSIw->y--;
						else
							VSIw->y -= VSIw->parms[0];
						if (VSIw->y < VSIw->top)
							VSIw->y = VSIw->top;
					  }
#endif														/* BYU */
					VSIrange();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'B': /* cursor down */
#if 1														/* BYU */
					if (VSIw->parms[0]<1) VSIw->y++;		/* BYU */
					else VSIw->y+=VSIw->parms[0];			/* BYU */
					if (VSIw->y > VSIw->bottom)				/* NCSA: SB */
						VSIw->y = VSIw->bottom;				/* NCSA: SB */
						
#else														/* BYU */
					if (VSIw->y > VSIw->bottom)
					  {
					  /* outside scrolling region */
						if (VSIw->parms[0] < 1)
							VSIw->y++;
						else
							VSIw->y += VSIw->parms[0];
					  }
					else
					  {
					  /* don't leave scrolling region */
						if (VSIw->parms[0] < 1)
							VSIw->y++;
						else
							VSIw->y += VSIw->parms[0];
						if (VSIw->y > VSIw->bottom)
							VSIw->y = VSIw->bottom;
					  }
#endif														/* BYU */
					VSIrange();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'C': /* cursor right */
					if (VSIw->parms[0] < 1)
						VSIw->x++;
					else
						VSIw->x += VSIw->parms[0];
					VSIrange();
					if (VSIw->x > VSIw->maxwidth)
						VSIw->x = VSIw->maxwidth;
					goto ShortCut;				/* BYU 2.4.12 */
				case 'D': /* cursor left */
					if (VSIw->parms[0] < 1)
						VSIw->x--;
					else
						VSIw->x -= VSIw->parms[0];
					VSIrange();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'f':
				case 'H':
				  /* absolute cursor positioning */
					VSIw->x = VSIw->parms[1] - 1;
					if (VSIw->DECORG)
					  /* origin mode -- position relative to top of scrolling region */
						VSIw->y = VSIw->parms[0] - 1 + VSIw->top;
					else
						VSIw->y = VSIw->parms[0] - 1;
					/*	Don't use actual VSIrange 'cause it will wrap us to first column if 
						we are past screen edge.  This causes "resize" to break */
					if (VSIw->x < 0)						/* JMB 2.6 */
						VSIw->x = 0;						/* JMB 2.6 */
					if (VSIw->x > (VSIw->maxwidth))			/* JMB 2.6 */
						VSIw->x = VSIw->maxwidth;			/* JMB 2.6 */
					if (VSIw->y < 0)						/* JMB 2.6 */
						VSIw->y = 0;						/* JMB 2.6 */
					if (VSIw->y > VSIw->lines)				/* JMB 2.6 */
						VSIw->y = VSIw->lines;				/* JMB 2.6 */
					goto ShortCut;				/* BYU 2.4.12 */
		        case 'i':											/* PR: media copy */
					if (VSIw->parms[VSIw->parmptr]==5) {			/* PR */
						/*c++; ctr--; */							/* PR */			
						VSprON();	/* PR - set status and open temp file etc */
									/* PR - chars will be redirected at top of loop É */
									/* PR - É in this procedure */
					}				/* PR */
		            escflg = 0;		/* PR */
		            break;			/* PR */
				case 'K':
				  /* erase to beginning/end/whole of line */
					switch (VSIw->parms[0])
					  {
						case -1:
						case  0:
							VSIeeol();
							break;
						case  1:
							VSIebol();
							break;
						case  2:
							VSIel(-1);
							break;
						default:
							goto ShortCut;		/* BYU 2.4.12 */
					  } /* switch */
					goto ShortCut;				/* BYU 2.4.12 */
				case 'J':
				  /* erase to beginning/end/whole of screen */
					switch (VSIw->parms[0])
					  {
						case -1:
						case  0:
							VSIeeos();
							break;
						case  1:
							VSIebos();
							break;
						case  2:
							VSIes();
							break;
						default:
							goto ShortCut;		/* BYU 2.4.12 */
					  } /* switch */
					goto ShortCut;				/* BYU 2.4.12 */
				case 'm':
				  /* set/clear attributes */
				  {
					short temp = 0;
					while (temp <= VSIw->parmptr)
					  {
						if (VSIw->parms[temp] < 1)
							VSIw->attrib &= 128; /* turn them all off */
						else if (VSIw->parms[temp] > 20)						/* BYU 2.4.13 */
						  /* turn off the appropriate bit */					/* BYU 2.4.13 */
							VSIw->attrib &= ~(1 << (VSIw->parms[temp] - 21));	/* BYU 2.4.13 */
						else
						  /* turn on the appropriate bit */
							VSIw->attrib |= 1 << (VSIw->parms[temp] - 1);
						temp++;
					  } /* while */
				  }
				  goto ShortCut;				/* BYU 2.4.12 */
				case 'q':
				  /* flash dem LEDs. What LEDs? */
					goto ShortCut;				/* BYU 2.4.12 */
				case 'c':
					VTsendident();
					goto ShortCut;				/* BYU 2.4.12 */
				case 'n':
					switch (VSIw->parms[0])
					  {
						case 5:
							VTsendstat();
							break;
						case 6:
							VTsendpos();
							break;
					  } /* switch */
					goto ShortCut;				/* BYU 2.4.12 */
				case 'L':
					if (VSIw->parms[0] < 1)
						VSIw->parms[0] = 1;
					VSIinslines(VSIw->parms[0], -1);
					goto ShortCut;				/* BYU 2.4.12 */
				case 'M':
					if (VSIw->parms[0] < 1)
						VSIw->parms[0] = 1;
					VSIdellines(VSIw->parms[0], -1);
					goto ShortCut;				/* BYU 2.4.12 */
				case 'P':
					if (VSIw->parms[0] < 1)
						VSIw->parms[0] = 1;
					VSIdelchars(VSIw->parms[0]);
					goto ShortCut;				/* BYU 2.4.12 */
				case 'r':
				  /* set scrolling region */
					if (VSIw->parms[0] < 0)
						VSIw->top = 0;
					else
						VSIw->top = VSIw->parms[0] - 1;
					if (VSIw->parms[1] < 0)
						VSIw->bottom = VSIw->lines;
					else
						VSIw->bottom = VSIw->parms[1] - 1;
					if (VSIw->top < 0)
						VSIw->top = 0;
					if (VSIw->top > VSIw->lines - 1)
						VSIw->top = VSIw->lines - 1;
					if (VSIw->bottom < 1)
						VSIw->bottom = VSIw->lines;
					if (VSIw->bottom > VSIw->lines)
						VSIw->bottom = VSIw->lines;

					if (VSIw->top >= VSIw->bottom)			/* NCSA: SB */
						{									/* NCSA: SB */
						if (VSIw->bottom >=1)				/* NCSA: SB */
							VSIw->top = VSIw->bottom -1;	/* NCSA: SB */
						else VSIw->bottom = VSIw->top +1;	/* NCSA: SB */
						}									/* NCSA: SB */
 				
 					VSIw->x = 0;
					VSIw->y = 0;
					if (VSIw->DECORG)
						VSIw->y = VSIw->top;	/* origin mode relative */
					goto ShortCut;				/* BYU 2.4.12 */
				case 'h':
				  /* set options */
					VSIsetoption(1);
					goto ShortCut;				/* BYU 2.4.12 */
				case 'l':
				  /* reset options */
					VSIsetoption(0);
					goto ShortCut;				/* BYU 2.4.12 */
				case 'g':
					if (VSIw->parms[0] == 3)
					  /* clear all tabs */
						VSItabclear();
					else if (VSIw->parms[0] <= 0)
					  /* clear tab at current position */
						VSIw->tabs[VSIw->x] = ' ';
					goto ShortCut;				/* BYU 2.4.12 */
				case '!':						/* BYU 2.4.12 - More private DEC stuff? */
				case '\'':						/* BYU 2.4.12 - More private DEC stuff? */
				case '\"':						/* BYU 2.4.12 - More private DEC stuff? */
					escflg++;					/* BYU 2.4.12 */
					break;						/* BYU 2.4.12 */
				default:			/* Dang blasted strays... */
					goto ShortCut;				/* BYU 2.4.12 */
			  } /* switch */
			c++;
			ctr--;
		  } /* while */

		while ((escflg == 3) && (ctr > 0))
		  {	/* "#" handling */
		  /* no support for double-width and double-height characters yet */
			switch (*c)
			  {
				case 0x08:
					VSIw->x--;
					if (VSIw->x < 0)
						VSIw->x = 0;
					break;
				case '8': /* alignment display */
					VTalign();
					goto ShortCut;				/* BYU 2.4.12 */
				default:
					goto ShortCut;				/* BYU 2.4.12 */
			  } /* switch */
			c++;
			ctr--;
		  } /* while */

		while ((escflg == 4) && (ctr > 0))
		  {	/* "(" handling (selection of G0 character set) */
			switch (*c)
			  {
				case 0x08:
					VSIw->x--;
					if (VSIw->x < 0)
						VSIw->x = 0;
					break;
				case 'A': /* UK */
				case 'B': /* US */
				case '1': /* "soft" */
					VSIw->G0 = 0;
					if (!VSIw->charset)
						VSIw->attrib = VSnotgraph(VSIw->attrib);
					goto ShortCut;				/* BYU 2.4.12 */
				case '0': /* DEC special graphics */
				case '2': /* "soft" */
					VSIw->G0 = 1;
					if (!VSIw->charset)
						VSIw->attrib = VSgraph(VSIw->attrib);
					goto ShortCut;				/* BYU 2.4.12 */
				default:
					goto ShortCut;				/* BYU 2.4.12 */
			  } /* switch */
			c++;
			ctr--;
		  } /* while */
	
		while ((escflg == 5) && (ctr > 0))
		  {	/* ")" handling (selection of G1 character set) */
			switch (*c)
			  {
				case 0x08:
					VSIw->x--;
					if (VSIw->x < 0)
						VSIw->x = 0;
					break;
				case 'A': /* UK */
				case 'B': /* US */
				case '1': /* "soft" */
					VSIw->G1 = 0;
					if (VSIw->charset)
						VSIw->attrib = VSnotgraph(VSIw->attrib);
					goto ShortCut;				/* BYU 2.4.12 */
				case '0': /* DEC special graphics */
				case '2': /* "soft" */
					VSIw->G1 = 1;
					if (VSIw->charset)
						VSIw->attrib = VSgraph(VSIw->attrib);
					goto ShortCut;				/* BYU 2.4.12 */
				default:
					goto ShortCut;				/* BYU 2.4.12 */
			  } /* switch */
			c++;
			ctr--;
		  } /* while */

        // Handle XTerm rename functions, code contributed by Bill Rausch
        // Modified by JMB to handle ESC]2; case as well.
		if( (escflg >= 6) && (ctr > 0) ) {
			static char *tmp;
			static Str255 newname;
          
		if( (escflg == 6) && ((*c == '0') || (*c == '2'))) {
			escflg++;
            c++;
            ctr--;
			}
		if( (escflg == 7 ) && (ctr > 0) && (*c == ';') ) {
			ctr--;
			c++;
			escflg++;
			newname[0] = 0;
			tmp = (char *)&newname[1];
			}
		while( (escflg == 8) && (ctr > 0) && (*c != 07) ) {
			*tmp++ = *c++;
			ctr--;
			(*newname)++;
			}
		if( (escflg == 8) && (*c == 07) && (ctr > 0) ) {
			set_new_window_name( newname, RSgetwindow(VSIwn) );
			goto ShortCut;
			}
		} /* if */

		if ((escflg > 2) && (ctr > 0))
		  {
ShortCut:						/* BYU 2.4.12 - well, sacrificing style for speed */
			escflg = 0;
			c++;
			ctr--;
		  } /* if */
	  } /* while (ctr > 0) */
    VSIw->escflg = escflg;
  } /* VSem */


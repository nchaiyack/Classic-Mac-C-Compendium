/*
 *
 *	  Virtual Screen Kernel Interface
 *			  (vsinterf.c)
 *
 *	  by Gaige B. Paulsen
 *
 *	This file contains the control and interface calls for the NCSA
 *  Virtual Screen Kernal.
 *
 *	  VSinit(maxwidth)					- Initialize the VSK
 *	  VSnewscreen(maxlines,scrnsave)	- Initialize a new screen.
 *	  VSdetach(w)						- Detach screen w
 *	  VSredraw(w,x1,y1,x2,y2)			- redraw region for window w
 *	  VSwrite(w,ptr,len)				- write text @ptr, length len
 *	  VSclear(w)						- clear w's real screen
 *	  VSkbsend(w,k,echo)				- send keycode k's rep. out window w (w/echo if req.)
 *	  VSclearall(w)						- clear w's real and saved screen
 *	  VSreset(w)						- reset w's emulator (as per TERM)
 *	  VSgetline(w,y)					- get a ptr to w's line y
 *	  VSsetrgn(w,x1,y1,x2,y2)			- set local display region
 *	  VSscrolback(w,n)					- scrolls window w back n lines
 *	  VSscrolforward(w,n)				- scrolls window w forward n lines
 *	  VSscrolleft(w,n)			 		- scrolls window w left  n columns
 *	  VSscrolright(w,n)			 		- scrolls window w right n columns
 *	  VSscrolcontrol(w,scrlon,offtop)	- sets scroll vars for w
 *	  VSgetrgn(w,&x1,&y1,&x2,&y2)		- returns set region
 *	  VSsnapshot(w)				  		- takes a snapshot of w
 *	  VSgetlines(w)						- Returns current # of lines
 *	  VSsetlines(w, lines)				- Sets the current # of lines to lines
 *	
 *		Version Date	Notes
 *		------- ------  ---------------------------------------------------
 *		0.01	861102  Initial coding -GBP
 *		0.10	861113  Added some actual program to this file -GBP
 *		0.15	861114  Initiated Kludge Operation-GBP
 *		0.50	8611VSPBOTTOM  Parameters added to VSnewscreen -GBP
 *		0.90	870203	Added the kbsend routine -GBP
 *		2.1		871130	NCSA Telnet 2.1 -GBP
 *		2.2 	880715	NCSA Telnet 2.2 -GBP
 *
 */

#ifdef MPW
#pragma segment VS
#endif

#include "TelnetHeader.h"

#define VSMASTER

#include <ToolUtils.h>	/* BYU 2.4.18 - for GetIndString() */
#include <stdio.h>

#include <String.h>

#include "rsmac.proto.h"
#include "vsem.proto.h"
#include "vsdata.h"
#include "vskeys.h"
#include "vsinit.h"
#include "vsintern.proto.h"
#include "Wind.h"
#include "DlogUtils.proto.h"
#include "maclook.proto.h"
#include "errors.proto.h"
#include "debug.h"

#define DEBUGMAC

#include "vsinterf.proto.h"

extern TelInfoRec *TelInfo;
extern WindRec	*screens;
extern long		TempItemsDirID;
extern short	TempItemsVRefNum;

short
  /* Internal variables for use in managing windows */
	VSmax = 0, /* max nr screens allowed */
	VSinuse = 0; /* nr screens actually in existence */
VSscrndata *VSscreens;

short VSinit
  (
	short max /* max nr screens to allow */
  )
  /* initializes virtual screen and window handling. */
  {
	short i;

	RSinitall(max);
	VSmax = max;
	VSIwn = 0;
	if ((VSscreens = (VSscrndata *) NewPtr(max * sizeof(VSscrndata))) == 0L)
		return(-2);
	for (i = 0; i < max; i++)
	  {
		VSscreens[i].loc = 0L;
		VSscreens[i].stat = 0;
	  } /* for */
	return(0);
  } /* VSinit */


short VSiscapturing(short w) {								/* BYU 2.4.18 */
	return(VSscreens[w].captureRN);					/* BYU 2.4.18 */
}														/* BYU 2.4.18 */

Boolean VSopencapture(short scrn_num, short w)
{
	#pragma unused (scrn_num)
	static short	captNumber = 1;
	short			VRefNum;
	long			DirID;
	Str255			filename;
	Str32			numstring;
	Point			where = {100,100};
	SFReply			sfr;
	long			junk = 0;
	OSErr			err;
	
	NumToString(captNumber++, numstring);
	GetIndString(filename, MISC_STRINGS, CAPTFILENAME);
	filename[++(filename[0])] = ' ';
	pstrcat(filename, numstring);
	
	SFPutFile(where, "\pSave captured text as:", filename, NULL, &sfr);
	if (sfr.good) {
		(void) GetWDInfo(sfr.vRefNum, &VRefNum, &DirID, &junk);

		err = HCreate(VRefNum, DirID, sfr.fName, 
					gApplicationPrefs->CaptureFileCreator, 'TEXT');
		if (err == dupFNErr) {
			HDelete(VRefNum, DirID, sfr.fName);
			err = HCreate(VRefNum, DirID, sfr.fName,
							gApplicationPrefs->CaptureFileCreator, 'TEXT');
			}
			
		if (err != noErr)
			OperationFailedAlert(CANT_CREATE_FILE, 500, err);
		else {		
			err = HOpen(VRefNum, DirID, sfr.fName, fsRdWrPerm,
				&VSscreens[w].captureRN);
			if (err != noErr) OperationFailedAlert(CANT_OPEN_FILE, 501, err);
			else {
				SetEOF(VSscreens[w].captureRN, (long) 0);
				return(TRUE);
				}
			}
		}
		
	return(FALSE);
}

void VSclosecapture(short w) {									/* BYU 2.4.18 */
	FSClose(VSscreens[w].captureRN);					/* BYU 2.4.18 */
	VSscreens[w].captureRN = 0;							/* BYU 2.4.18 */
}														/* BYU 2.4.18 */

void VScapture(unsigned char *ptr, short len) {		/* BYU 2.4.18 */
	long ln = len;								/* BYU 2.4.18 */
	if (VSscreens[VSIwn].captureRN) {			/* BYU 2.4.18 */
		unsigned char captbuf[512];				/* BYU 2.4.18 */
		unsigned char *ptr2,*ptr3;				/* BYU 2.4.18 */
		ptr2 = ptr;								/* BYU 2.4.18 */
		ptr3 = &captbuf[0];						/* BYU 2.4.18 */
		for (len = 0; len < ln; len++) {		/* BYU 2.4.18 */
			if (*ptr2 >= 32 || 					/* BYU 2.4.18 */
				*ptr2 == 13 ||					/* BYU 2.4.18 */
				*ptr2 ==  9)					/* BYU 2.4.18 */
				*(ptr3++) = *(ptr2++);			/* BYU 2.4.18 */
			else {								/* BYU 2.4.18 */
				ptr2++;							/* BYU 2.4.18 */
				ln--;							/* BYU 2.4.18 */
			}									/* BYU 2.4.18 */
		}										/* BYU 2.4.18 */
		if (ln > 0) {													/* BYU 2.4.18 */
			if (FSWrite(VSscreens[VSIwn].captureRN, &ln, captbuf)) {	/* BYU 2.4.18 */
				FSClose(VSscreens[VSIwn].captureRN);					/* BYU 2.4.18 */
				VSscreens[VSIwn].captureRN = 0;							/* BYU 2.4.18 */
			}											/* BYU 2.4.18 */
		}											/* BYU 2.4.18 */
	}												/* BYU 2.4.18 */
}													/* BYU 2.4.18 */

short	VSisprinting(short w)
{
	return((VSscreens[w].loc)->prredirect);
}

void	ClosePrintingFile(short w)
{
	OSErr sts;
	char tmp[80];

	putln("Attempting to remove print file");
	
	if (sts=FSClose ((VSscreens[w].loc)->refNum)) {
		SysBeep(1);
		sprintf(tmp,"FSClose: ERROR %d",sts); putln(tmp);
		}
	if (sts=HDelete(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname)) {
		SysBeep(1);
		sprintf(tmp,"HDelete: ERROR %d",sts); putln(tmp);
		}
}

short VSvalids
  (
	short w
  )
  /* validates a virtual screen number and sets it as the
	current screen for subsequent operations if success.
	Returns 0 iff success. */
  {
	if (VSinuse == 0)
		return(-5); /* -5=no ports in use */
	if (VSIwn == w)
		return(0);	/* Currently set to that window */
	if ((w > VSmax) || (w < 0))
		return(-6); /* blown out the top of the stuff */
	VSIwn = w;
	if (VSscreens[w].stat != 1)
		return(-3);/* not currently active */
	VSIw = VSscreens[w].loc;
	if (VSIw == 0L)
		return(-3); /* no space allocated */
	return(0);
  } /* VSvalids */

VSscrn *VSwhereis(short i) /* screen number */
  /* returns a pointer to the structure for the specified screen. */
  {
	VSvalids(i);
	return(VSIw);
  } /* VSwhereis */

void VSIclrbuf
  (
	void
  )
  /* clears out the text and attribute buffers for the current screen.
	All text characters are set to blanks, and all attribute bytes
	are set to zero. Doesn't update the display. */
  {
	register short j, i;
	register char *ta, *tx;

	for (i = 0; i <= VSIw->lines; i++)
	  {
		ta = &VSIw->attrst[i]->text[0];
		tx = &VSIw->linest[i]->text[0];
		for (j = 0; j <= VSIw->allwidth; j++)
		  {
			*ta++ = 0;
			*tx++ = ' ';
		  } /* for */
	  } /* for */
  } /* VSIclrbuf */

short VSnewscreen
  (
	short maxlines, /* max lines to save in scrollback buffer */
	short screensave, /* whether to have a scrollback buffer */
	short maxwid, /* number of columns on screen */
	short IDC, /* NCSA 2.5:whether to be smart and scroll things on character
		insertion/deletion. Currently always passed as 1 (in the
		only call, from RSnewwindow in rsmac.c). Just as well,
		because the two places (both in vsintern.c) where this does
		matter don't do correct redrawing otherwise. */
	short forcesave	/* NCSA 2.5: force lines to be saved */
  )
  /* creates a new virtual screen, and returns its number. */
  {
	VSlinePtr tt;

	if (maxlines < VSDEFLINES)
		maxlines = VSDEFLINES;

	if (VSinuse >= VSmax)
	  /* too many screens in existence */
		return(-1);
	VSIwn = 0;
	while ((VSIwn < VSmax) && (VSscreens[VSIwn].stat == 1))
		VSIwn++;
	if (VSIwn >= VSmax)
	  /* shouldn't occur? */
		return(-1);

	if (VSscreens[VSIwn].stat == 2)
	  {
	  /* recycling an old screen structure */
		VSIw = VSscreens[VSIwn].loc;
		if (VSIw == 0L)
			return(-7);
	  }
	else
	  /* create a new screen structure */
		if ((VSscreens[VSIwn].loc = VSIw = (VSscrn *) NewPtr(sizeof(VSscrn))) == 0L)
		  {
			VSscreens[VSIwn].loc = 0L;
			return(-2);
		  } /* if */
	if (VSscreens[VSIwn].stat != 2)
	  {
		VSIw->maxlines = maxlines;
		VSIw->numlines = 0;
	  } /* if */
	VSscreens[VSIwn].captureRN = 0;		/* BYU 2.4.18 - capture file's RefNum */
	VSIw->id = 'VSCR';
	VSIw->maxwidth = maxwid - 1;
	VSIw->allwidth = 131;				/* NCSA: SB - always allocate max lines */
	VSIw->savelines = screensave;
	VSIw->forcesave = forcesave;		/* NCSA 2.5 */
	VSIw->attrib = 0;
	VSIw->Pattrib = -1; /* initially no saved attribute */
	VSIw->x = 0;
	VSIw->y = 0;
	VSIw->charset = 0;
	VSIw->G0 = 0;
	VSIw->G1 = 1;
	VSIw->VSIDC = IDC;
	VSIw->DECAWM = 0;
	VSIw->DECCKM = 0;
	VSIw->DECPAM = 0;
	VSIw->DECORG = 0;
	VSIw->IRM = 0;
	VSIw->escflg = 0;
	VSIw->top = 0;
	VSIw->bottom = 23;
	VSIw->parmptr = 0;
	VSIw->Rtop = 0;
	VSIw->Rleft = 0;
	VSIw->Rright = 79;
	VSIw->Rbottom = 23;
	VSIw->ESscroll = 1;
  /* shouldn't the following three statements be conditional on
	*not* recycling an existing screen structure? */
	VSIw->lines = 23;
	VSIw->linest = VSInewlinearray(VSIw->lines + 1);
	VSIw->attrst = VSInewlinearray(VSIw->lines + 1);
	VSIw->prredirect = 0;						/* LU */
	VSIw->prbuf = 0;							/* LU */
	VSIw->refNum = -1;							/* LU */
	VSinuse++;

	if (VSscreens[VSIwn].stat == 2)
	  {
		VSscreens[VSIwn].stat = 1;
		VSIclrbuf();
		VSItabinit();
		return(VSIwn);
	  } /* if */
	VSscreens[VSIwn].stat = 1;

	VSIw->tabs = (char *) NewPtr(132);		/* NCSA: SB - allow 132 column mode */
	
/*
*  Fill initial scrollback buffer and screen storage space.
*
*  Memory allocation rules:
*  line->mem == 0 if not a memory allocation, line->mem == 1 if it is the first
*     VSline in a block (indeterminate size, may be size == 1)
*  
*  attributes array is ALWAYS allocated as one block.  Internally represented and
*  manipulated as a linked list of lines, but only one of the lines will have 
*  line->mem == 1.  This list is always supposed to be circular (it is never
*  extended, as attributes are never scrolled back).
*
*  scrollback and screen line buffer space is allocated in large blocks.  Each
*  block will have line->mem == 1 if the pointer to that VSline is "free"able.
*  This list will either be circular (which means it has reached its full size),
*  or it will have a NULL next field at the end.  During scrolling, the end may
*  be augmented until VSIw->numlines > VSIw->maxlines or we run out of memory.
*  Typically allocate memory 100 lines at a time in two blocks, one is the VSline
*  list, the other is the mem for the character storage.
*
*/
	if (screensave)
		tt = VSInewlines(VSIw->lines + 1 + VSDEFLINES); /* screen lines plus some initial preallocated scrollback space */
	else
		tt = VSInewlines(VSIw->lines + 1); /* screen lines, no scrollback */
	if (!tt)
		return(-2);
	VSIw->buftop = tt;
	VSIw->scrntop = VSIw->buftop;	
	
	tt = VSInewlines(VSIw->lines + 1);		/* new space for attributes (these are never scrolled back) */
	if (!tt)
		return(-2);
	VSIw->attrst[0] = tt;
	VSIlistndx(VSIw->scrntop, VSIw->attrst[0]);	/* Set up screen arrays */
	VSIw->attrst[0]->prev = VSIw->attrst[VSIw->lines]; /* make attribute list circular, since it is never extended */
	VSIw->attrst[VSIw->lines]->next = VSIw->attrst[0];
	if (!screensave)
	  {
	  /* make text line list circular to indicate no extensions */
		VSIw->linest[0]->prev = VSIw->linest[VSIw->lines];
		VSIw->linest[VSIw->lines]->next = VSIw->linest[0];
	  } /* if */
	VSIclrbuf();
	VSItabinit();
	VSIw->vistop = VSIw->scrntop; /* initial view = screen */
#ifdef	DEBUGPC
	VSckconsist(1, 1);
#endif	DEBUGPC
	return(VSIwn);
  } /* VSnewscreen */

short VSdestroy(short w) /* screen number */
  /* gets rid of a virtual screen. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	VSIfreelines();
	VSIfreelinelist(VSIw->attrst[0]);
	DisposPtr((Ptr) VSIw->attrst);
	DisposPtr((Ptr) VSIw->linest);
	DisposPtr(VSIw->tabs);
	DisposPtr((Ptr) VSIw);
	VSscreens[w].stat = 0;
	VSIwn = -1;
	VSinuse--;			/* SCA '87 */
	return(0);
  } /* VSdestroy */

#ifdef DEBUGPC
int VSckconsist(int out, int verbose)
{
	VSlinePtr topa, topt, a, t;
	short line, i;

	for (i=0; i<VSmax; i++) {				/* For all possible screens... */
		switch( VSscreens[i].stat) {
		case 0:
			if (out && verbose) printf("Screen %d inactive\015",i);	/* BYU 2.4.18 - changed \n to \015 */
			break;
		case 1:
			if (out) printf("Screen %d active\015",i);	/* BYU 2.4.18 - changed \n to \015 */
			VSvalids(i);

			topa=VSIw->attrst[ 0]->prev; topt=VSIw->linest[ 0]->prev;
			a = VSIw->attrst[0]->next;  t= VSIw->linest[0]->next;
			
			if (topa && out) printf("  Attrib thinks its circular\015");	/* BYU 2.4.18 - changed \n to \015 */
			if (topa != VSIw->attrst[VSIw->lines])
								printf("***********BUT IT'S NOT*************\015");	/* BYU 2.4.18 - changed \n to \015 */
			for (line=1; line<=VSIw->lines; line++) {
				if (a != VSIw->attrst[ line])  {
					if (out) printf("    Attrib line %d is wrong !\015", line);	/* BYU 2.4.18 - changed \n to \015 */
					else return(-1);
					}
				a = a->next;
				if ( !a) {
					if (out) printf("    Attrib line %d is NULL\015", line);	/* BYU 2.4.18 - changed \n to \015 */
					else if (!out && line!=VSIw->lines) return(-2);
					}
				}

			if (topt && out) printf("  Linest thinks its circular\015");	/* BYU 2.4.18 - changed \n to \015 */
			if (VSIw->linest[VSIw->lines]->next) printf(" More than VSIw->lines lines.... \015");	/* BYU 2.4.18 - changed \n to \015 */
			for (line=1; line<=VSIw->lines; line++) {
				if (t != VSIw->linest[ line])  {
					if (out) printf("    Linest line %d is wrong !\015", line);	/* BYU 2.4.18 - changed \n to \015 */
					else  return (-3);
					}
				t = t->next;
				if ( !t) {
					if (out) printf("    Linest line %d is NULL\015", line);	/* BYU 2.4.18 - changed \n to \015 */
					else if (line!=VSIw->lines) return(-4);
					}
				}
			if (VSIw->numlines >0) {
				if (out)
					printf("    Thinks that there is scrollback of %d lines ", VSIw->numlines);
				t= VSIw->linest[VSIw->lines]->next;
				line=0;
				while ( t!=0L && t!=VSIw->buftop) {
					t=t->next;
					line++;
					}
				if (out) printf("    [ Actual is %d ]\015", line);	/* BYU 2.4.18 - changed \n to \015 */
				if (out && t==0L) printf("    Lines end in a null\015");	/* BYU 2.4.18 - changed \n to \015 */
				if (out && t==VSIw->buftop) printf("    Lines end in a wraparound\015");	/* BYU 2.4.18 - changed \n to \015 */
				}
			else if (out) printf("    There is no scrollback");
			break;
		case 2:
			if (out && verbose) printf("Screen %d detached\015",i);	/* BYU 2.4.18 - changed \n to \015 */
			break;
		default:
			if (out) printf("Screen %d invalid stat\015",i);	/* BYU 2.4.18 - changed \n to \015 */
			break;
		}
	}
	return(0);
}
#endif DEBUGPC


#ifdef USEDETACH
void VSdetach(short w)
{
	if (VSscreens[w].captureRN)									/* BYU 2.4.18 */
		FSClose(VSscreens[w].captureRN);						/* BYU 2.4.18 */
	if (VSscreens[w].stat!=1) return(-1);
	VSscreens[w].stat=2;
	VSIwn = -1;
	VSinuse--;			/* SCA '87 */
}
#else

void VSdetach
  (
	short w
  )
  /* this version doesn't try to keep the screen storage for reuse. */
  {
	VSdestroy(w);
  } /* VSdetach */

#endif 

short VSredraw
  (
	short w, /* window to redraw */
	short x1, /* bounds of area to redraw, relative to visible region */
	short y1,
	short x2,
	short y2
  )
  /* redisplays the specified portion of a virtual screen. */
  {
	char *pt, *pa;
	short cc, cm;
	char lc, la;
	register VSlinePtr yp;
	register short y;
	short ox, tx1, tx2, ty1, ty2, tn = -1, offset;

	if (VSvalids(w) != 0)
		return(-3);
	VSIcuroff(w); /* temporarily hide cursor */

	x1 += VSIw->Rleft; x2 += VSIw->Rleft;	/* Make local coords global again */
	y1 += VSIw->Rtop;  y2 += VSIw->Rtop;

	if (x2 < 0)
		x2 = 0;
	if (x1 < 0)
		x1 = 0;
	if (x2 > VSIw->maxwidth)
		x2 = VSIw->maxwidth;
	if (x1 > VSIw->maxwidth)
		x1 = VSIw->maxwidth;
	if (y2 < -VSIw->numlines)
		y2 = -VSIw->numlines;	/* limit to amount of scrollback */
	if (y1 < -VSIw->numlines)
		y1 = -VSIw->numlines;
	if (y2 > VSIw->lines)
		y2 = VSIw->lines;
	if (y1 > VSIw->lines)
		y1 = VSIw->lines;

	tx1 = x1; tx2 = x2; ty1 = y1; ty2 = y2; /* Set up to clip redraw area to visible area */
	if (!VSIclip(&tx1, &ty1, &tx2, &ty2, &tn, &offset))
		RSerase(w, tx1, ty1, tx2, ty2);		/* Erase the offending area */

  /* draw visible part of scrollback buffer */
	if (y1 < 0)
	  {
		yp = VSIw->vistop;
		y = y1 - VSIw->Rtop;
		while (y-- > 0)
			yp = yp->next;	/* Get pointer to top line we need */
	  } /* if */
	y = y1;
	while ((y < 0) && (y <= y2))
	  {
		ox = tx1 = x1; tx2 = x2; ty1 = ty2 = y; tn = -1;
		if (!VSIclip(&tx1, &ty1, &tx2, &ty2, &tn, &offset))
			RSdraw(w, tx1, ty1, 0, tn, &yp->text[ox + offset]);
		yp = yp->next;
		y++;
	  } /* while */
  /* draw visible part of on-screen buffer, taking account of attributes */
	while (y <= y2)
	  {
		pt = &VSIw->linest[y]->text[x2]; /* text pointer */
		pa = &VSIw->attrst[y]->text[x2]; /* attribute pointer */
		cm = x2; /* start of a run of characters */
		cc = 1; /* length of the run */
		lc = *pt; /* the last character examined */
		la = *pa; /* the attributes of the last character examined */
		while (cm >= x1)
		  {
			if ((lc == ' ') && (la == 0))
			  {
			  /* skip blank areas of the screen (runs of spaces with no attributes) */
				while((cm > x1) && (*(pt - 1) == ' ') && (*(pa - 1) == 0))
				  {
					pa--;
					pt--;
					cm--;
					cc++;
				  } /* while */
				pa--;
				pt--;
				cm--;
				cc = 1;
				lc = *pt;
				la = *pa;
				continue;
			  } /* if */
		  /* find a run of characters with the same attributes */
			while((cm > x1) && (la == *(pa - 1)))
			  {
				pa--;
				pt--;
				cm--;
				cc++;
			  } /* while */
			if (cm >= x1)
			  /* draw the run */
				VSIdraw(w, cm, y, la, cc, pt);
			pa--;
			pt--;
			cm--;
			cc = 1;
			lc = *pt;
			la = *pa;
		  } /* while */
		y++;
	  } /* while */
	VSIcurson(w, VSIw->x, VSIw->y, 0); /* restore cursor at original position */

	tx1 = ty1 = 0;
	tn = 132;
	if (!VSIclip(&tx1, &ty1, &tx2, &ty2, &tn, &offset))
		RSdrawsep(w, ty1, 1);					/* Draw Separator */

	return(0);
  } /* VSredraw */

short VSwrite
  (
	short w, /* screen to draw into */
	char *ptr, /* pointer to text string */
	short len  /* length of text string */
  )
  /* sends a stream of characters to the specified window. */
  {
//  	_profile = 1;
	if (VSvalids(w) != 0)
		return(-3);
	VSIcuroff(w); /* hide cursor momentarily */
	VSem((unsigned char *) ptr, len);	/* BYU LSC - interpret the character stream */
	VSIcurson(w, VSIw->x, VSIw->y, 1); /* restore cursor, force it to be visible. */
//	_profile = 0;
	return(0);
  } /* VSwrite */

short VSclear(short w)
  /* clears the screen and homes the cursor. Not used. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	VSIes(); /* clear screen */
	VSIw->x = VSIw->y = 0; /* home cursor */
	VSIcurson(w, VSIw->x, VSIw->y, 1); /* Force Move */
	return(0);
  } /* VSclear */




															// MAT--We need to define how big the Queue is
															// MAT--for sending the cursor characters.
															// MAT--Since MacTCP doesn't like 30+ individual
#define MATSbufSize	254										// MAT--SendChar requests.
void VSpossendEM											// MAT--we can change this to support EMACS
  (															// MAT--style movement commands.
	short w, /* affected screen */												// MAT--
	short x, /* column to move to */												// MAT--
	short y, /* line to move to */												// MAT--
	short echo /* local echo flag */												// MAT--
  )																				// MAT--
  /* sends a stream of VT100 cursor-movement sequences to move the				// MAT--
	cursor on the specified screen to the specified position. */				// MAT--
{																				// MAT--
	#pragma unused (echo)
	static char																	// MAT--
			tt[MATSbufSize] = "";												// MAT--
		char curschar;															// MAT--
		short localCount;															// MAT--
																				// MAT--
//	if (VSvalids(w) != 0)			already checked for us in VSpossend			// MAT--
//		return;																	// MAT--
																				// MAT--
//	if (VSIw->DECPAM && VSIw->DECCKM)  		what is this for???					// MAT--
//		vskptr = VSkbax;														// MAT--
//	else																		// MAT--
//		vskptr = VSkban;														// MAT--
																				// MAT--
	if (x < 0 || y < 0 || x > VSIw->maxwidth || y > VSIw->lines)				// MAT--
		return;																	// MAT--
  /* convert x and y to relative distance to move */							// MAT--
	x -= VSIw->x;																// MAT--
	y -= VSIw->y;																// MAT--
																				// MAT--
	curschar = '\002'; /* EMACS cursor left */									// MAT--
	localCount=0;
	while (x < 0 && localCount < MATSbufSize)									// MAT--
	  {																			// MAT--
		tt[localCount] = curschar;												// MAT--
		x++; localCount++;														// MAT--
	  } /* while */																// MAT--
	if(localCount)	RSsendstring(w, tt, localCount);							// MAT--

	curschar = '\016'; /* EMACS cursor down */									// MAT--
	localCount=0;
	while (y > 0 && localCount < MATSbufSize)									// MAT-- why we check to see if localCount is < MATSbufSize
	  {																			// MAT-- I dont know. But If they had a window > 254 columns
		tt[localCount] = curschar;												// MAT-- maybe it's a good idea.
		y--; localCount++;														// MAT-- but it never hurts to be safe.
	  } /* while */																// MAT--
	if(localCount)	RSsendstring(w, tt, localCount);							// MAT--

	curschar = '\020'; /* EMACS cursor up */									// MAT--
	localCount=0;
	while (y < 0 && localCount < MATSbufSize)									// MAT--
	  {																			// MAT--
		tt[localCount] = curschar;												// MAT--
		y++; localCount++;														// MAT--
	  } /* while */																// MAT--
	if(localCount)	RSsendstring(w, tt, localCount);							// MAT--

	curschar = '\006'; /* EMACS cursor right */									// MAT--
	localCount=0;
	while (x > 0 && localCount < MATSbufSize)									// MAT--
	  {																			// MAT--
		tt[localCount] = curschar;												// MAT--
		x--; localCount++;														// MAT--
	  } /* while */																// MAT--
	if(localCount)	RSsendstring(w, tt, localCount);							// MAT--
																				// MAT--
	if (0)	//(note: supposed to look for "echo" here)							// MAT--
	  {																			// MAT--
		VSIcuroff(w);															// MAT--
		VSIw->x = x;															// MAT--
		VSIw->y = y;															// MAT--
		VSIcurson(w, VSIw->x, VSIw->y, 1); /* Force Move */						// MAT--
	  } /* if */																// MAT--
  } /* VSpossendEM */					// changed comment						// MAT--
 	
void VSpossend
  (
	short w, /* affected screen */
	short x, /* column to move to */
	short y, /* line to move to */
	short echo /* local echo flag */
  )
  /* sends a stream of VT100 cursor-movement sequences to move the
	cursor on the specified screen to the specified position. */
{
	static char
		VSkbax[] = "\033O ",		/* prefix for auxiliary code */
		VSkban[] = "\033[ ";		/* prefix for arrows normal */
	char *vskptr;

	if (VSvalids(w) != 0)
		return;
	
	
/* NCSA: SB - This would bomb before.  You need to get the screens # from the 
				translation routine before you access the record! */  	
	if (screens[findbyVS(w)].arrowmap) {    /* NCSA: SB - get the CORRECT screens # */
		VSpossendEM(w,x,y,echo);            // MAT--  call our cursor movement routine
		return;                             // MAT--  then exit
	}

	if (VSIw->DECPAM && VSIw->DECCKM)
		vskptr = VSkbax;
	else
		vskptr = VSkban;
	if (x < 0 || y < 0 || x > VSIw->maxwidth || y > VSIw->lines)
		return;
  /* convert x and y to relative distance to move */
	x -= VSIw->x;
	y -= VSIw->y;

	vskptr[2] = 'D'; /* cursor left */
	while (x < 0)
	  {
		x++;
		RSsendstring(w, vskptr, 3);
	  } /* while */

	vskptr[2] = 'B'; /* cursor down */
	while (y > 0)
	  {
		y--;
		RSsendstring(w, vskptr, 3);
	  } /* while */
	vskptr[2] = 'A'; /* cursor up */
	while (y < 0)
	  {
		y++;
		RSsendstring(w, vskptr, 3);
	  } /* while */
	vskptr[2] = 'C'; /* cursor right */
	while (x > 0)
	  {
		x--;
		RSsendstring(w, vskptr, 3);
	  } /* while */

	if (echo)
	  {
		VSIcuroff(w);
		VSIw->x = x;
		VSIw->y = y;
		VSIcurson(w, VSIw->x, VSIw->y, 1); /* Force Move */
	  } /* if */
  } /* VSpossend */

char VSkbsend
  (
	short w, /* active window */
	unsigned char k, /* special key code if > 128, else ascii code */
	short echo /* local echo flag */
  )
  /* sends the appropriate sequence for the specified key, taking due
	account of terminal mode settings. */
  {
	static char
		VSkbkn[] = "\033O ",		/* prefix for keypad normal */
		VSkbax[] = "\033O ",		/* prefix for auxiliary code*/
		VSkban[] = "\033[ ",		/* prefix for arrows normal */
		VSkbfn[] = "\033O ",		/* prefix for function keys */		/* BYU 2.4.12 */
		VSk220[] = "\033[  ~";		/* prefix for vt220 keys */			/* BYU 2.4.12 */
	char *vskptr;
	short vskplen;


	if (VSvalids(w) != 0)
		return(-3);

	if ( screens[findbyVS(w)].arrowmap && (k <= VSLT) && (k >= VSUP) )	// MAT--
										// MAT-- important...we need to check this first before
		{								// MAT-- the next if(É) statement gets its hands on the string.
			switch (k) {				// MAT--  do the mapping from arrowkeys -> EMACS ctrl keys.
				case VSLT:				// MAT-- 
					k = 0x02;			// MAT-- ^B			Question: Is there a way to find out if the option
					break;				// MAT-- 					  key was held down with this character?
				case VSRT:				// MAT-- 			I didn't want to declare myEvent an extern
					k = 0x06;			// MAT-- ^F			(I didn't know if that was a no-no)
					break;				// MAT-- 			If I can.....let me know, I want to make
				case VSUP:				// MAT-- 			option-arrowkey's do useful things too
					k = 0x10;			// MAT-- ^P
					break;				// MAT-- 			checking the keymap would be a kludge here.
				case VSDN:				// MAT-- 
					k = 0x0e;			// MAT-- ^N
					break;				// MAT-- 
			}	/* switch k */			// MAT--
			RSsendstring(w,(char *)&k,1);		// MAT-- send the character
			return(0);					// MAT--
		}								// MAT--


	if (k < VSF10)							/* BYU 2.4.12 */
	  /* 7-bit ascii code -- send as is */
		RSsendstring(w,(char *) &k, 1);		/* BYU LSC */
	
	/* Keypad (Not Application Mode): 0-9 , - . Enter */	
	if ((k > VSLT) && (k < VSF1) && (!VSIw->DECPAM)) {
		RSsendstring(w, &VSIkpxlate[0][k - VSUP], 1);
		if (echo)
			VSwrite(w, &VSIkpxlate[0][k - VSUP], 1);
		if (k == VSKE)
			RSsendstring(w, "\012", 1);
		return(0);
	  } /* if */
	
	  
	if (VSIw->DECPAM && VSIw->DECCKM) {
	  /* aux kp mode */
	  	vskptr = VSkbax;
		vskplen = 3;
		vskptr[2] = VSIkpxlate[1][k - VSUP];	/* BYU 2.4.12 */
	  }
	else if (k < VSUP) {						/* BYU 2.4.12 */
		vskptr = VSk220;						/* BYU 2.4.12 */
		vskplen = VSIkplen[k - VSF10];			/* BYU 2.4.12 */
		vskptr[2] = VSIkpxlate2[k - VSF10];		/* BYU 2.4.12 */
		vskptr[3] = VSIkpxlate3[k - VSF10];		/* BYU 2.4.12 */
	} else {									/* BYU 2.4.12 */
		vskplen = 3;							/* BYU 2.4.12 */
		if (k < VSK0) {							/* BYU 2.4.13 - arrow keys */
			vskptr = VSkban;					/* BYU 2.4.12 */
			if (VSIw->DECCKM)					/* BYU 2.4.13 */
				vskptr[1] = 79;					/* BYU 2.4.13 */
			else								/* BYU 2.4.13 */
				vskptr[1] = 91;					/* BYU 2.4.13 */
		}										/* BYU 2.4.13 */
		else if (k < VSF1) 						/* BYU 2.4.12 */
			vskptr = VSkbkn;					/* BYU 2.4.12 */
		else 									/* BYU 2.4.12 */
			vskptr = VSkbfn;					/* BYU 2.4.12 */
												/* BYU 2.4.12 */
		vskptr[2] = VSIkpxlate[1][k - VSUP];	/* BYU 2.4.12 */
	}											/* BYU 2.4.12 */

	RSsendstring(w, vskptr, vskplen);
	if (echo)
		VSwrite(w, vskptr, vskplen);
	return(0);
  } /* VSkbsend */

short VSclearall(short w)
  /* doesn't seem to do anything interesting. Not used. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	return(0);
  } /* VSclearall */

short VSreset
  (
	short w
  )
  /* resets virtual terminal settings to default state, clears screen
	and homes cursor. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	VSIcuroff(w);			/* NCSA: SB -- get rid of extraneous cursor BS */
	VSIreset(); /* causes cursor to disappear */
	VSIcurson(w, VSIw->x, VSIw->y, 1); /* redisplay cursor at home position */
	return(0);
  } /* VSreset */

char *VSgetline(short w, short y)
  /* returns a pointer to the text for the specified on-screen line.
	Doesn't work for negative values of y (i e lines in the
	scrollback area). Not used. */
  {
	if (VSvalids(w) != 0)
		return((char *) -3);
	return(VSIw->linest[y]->text);
  } /* VSgetline */

void VSscrolright
  (
	short w,
	short n /* number of columns to scroll */
  )
  /* moves the view of the virtual screen within its window the
	specified number of columns to the right. */
  {
	short sn, lmmax;

	if (VSvalids(w) != 0)
		return;

  /* limit scroll amount against number of invisible columns */
	lmmax = VSIw->maxwidth - (VSIw->Rright - VSIw->Rleft);
	if (VSIw->Rleft + n > lmmax)
		n = lmmax - VSIw->Rleft; /* can't scroll any further right than this */
	if (n == 0)
		return;									/* Do nothing if appropriate */

	VSIcuroff(w); /* temporarily hide cursor */
	VSIw->Rleft += n; /* update visible region */
	VSIw->Rright += n;
	sn = VSIw->Rbottom - VSIw->Rtop;
	RSmargininfo(w, lmmax, VSIw->Rleft);	/* update horizontal scroll bar */
	RSdelcols(w, n); /* scroll the window contents */
	VSIcurson(w, VSIw->x, VSIw->y, 0); /* restore cursor at original position */
  /* redraw newly-revealed portion of screen */
	VSredraw(w, (VSIw->Rright - VSIw->Rleft) - n, 0, (VSIw->Rright - VSIw->Rleft), sn);
  } /* VSscrolright */

void VSscrolleft
  (
	short w,
	short n /* number of columns to scroll */
  )
  /* moves the view of the virtual screen within its window the
	specified number of columns to the left. */
  {
	short sn, lmmax;

	if (VSvalids(w) != 0)
		return;

	lmmax = VSIw->maxwidth - (VSIw->Rright - VSIw->Rleft); /* number of invisible columns */

	if (n > VSIw->Rleft)
		n = VSIw->Rleft; /* can't scroll any further left than this */
	if (n == 0)
		return;									/* Do nothing if appropriate */

	VSIcuroff(w); /* temporarily hide cursor */
	VSIw->Rleft -= n; /* update visible region */
	VSIw->Rright -= n;
	sn = VSIw->Rbottom - VSIw->Rtop;
	RSmargininfo(w, lmmax, VSIw->Rleft); /* update horizontal scroll bar */
	RSinscols(w, n); /* scroll the window contents */
	VSIcurson(w, VSIw->x, VSIw->y, 0); /* restore cursor at original position */
	VSredraw(w, 0, 0, n, sn); /* redraw newly-revealed portion of screen */
  } /* VSscrolleft */

short VSscroltop( short w /* which window */)					/* JMB 2.6 */
{															/* JMB 2.6 */
	if (VSvalids(w) != 0)									/* JMB 2.6 */
		return(-3);											/* JMB 2.6 */
															/* JMB 2.6 */
	return(VSscrolback(w, VSIw->Rtop + VSIw->numlines)); /* can't scroll back any further than this */
}															/* JMB 2.6 */

short VSscrolback
  (
	short w, /* which window */
	short in /* number of lines to scroll */
  )
  /* moves the view of the virtual screen within its window the
	specified number of lines upwards. */
  {
	short sn, n;

	n = in;

	if (VSvalids(w) != 0)
		return(-3);

	if (VSIw->numlines < (n - VSIw->Rtop))
		n = VSIw->Rtop + VSIw->numlines; /* can't scroll back any further than this */
	if (n <= 0)
		return(0);			/* Dont be scrollin' no lines.... */

	VSIcuroff(w); /* temporarily hide cursor */

	VSIw->Rtop = VSIw->Rtop - n; /* adjust the visible region */
	VSIw->Rbottom = VSIw->Rbottom - n;

  /* find the line list element for the new topmost visible line */
	sn = n;
	while (sn-- > 0)
	  {
#ifdef DEBUGMAC
		if (VSIw->vistop->prev == 0L)
			DebugStr("\pVSscrolback -- something wrong with linked list structure");
#endif DEBUGMAC
		VSIw->vistop = VSIw->vistop->prev;
	  } /* while */

	sn = VSIw->Rbottom - VSIw->Rtop;
  /* update vertical scroll bar */
	RSbufinfo(w, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom);

	if (n <= VSIw->lines)
	  {
		RSinslines(w, 0, sn, n, 0);	/* scroll, preserving current selection */
		VSIcurson(w, VSIw->x, VSIw->y, 0); /* restore cursor at original position */
		VSredraw(w, 0, 0, VSIw->maxwidth, n - 1); /* redraw newly-revealed portion */
	  }
	else
	  /* currently-visible contents scrolled completely off the screen--
		just redraw everything */
		VSredraw(w, 0, 0, VSIw->maxwidth, sn);

	return(0);
  } /* VSscrolback */

short VSscrolforward
  (
	short w, /* which window */
	short n /* number of lines to scroll */
  )
  /* moves the view of the virtual screen within its window the
	specified number of lines downwards. */
  {
	short sn;

	if (VSvalids(w) != 0)
		return(-3);

	if (n > VSIw->lines - VSIw->Rbottom)
		n = VSIw->lines - VSIw->Rbottom; /* can't scroll any further forward than this */
	if (n <= 0)
		return(0);			/* Dont be scrollin' no lines.... */

	VSIcuroff(w); /* temporarily hide cursor */

	VSIw->Rtop = n + VSIw->Rtop; /* adjust the visible region */
	VSIw->Rbottom = n + VSIw->Rbottom;

  /* find the line list element for the new topmost visible line */
	sn = n;
	while (sn-- > 0)
	  {
#ifdef DEBUGMAC
		if (VSIw->vistop->next == nil)
			DebugStr("\pVSscrolforward -- something wrong with linked list structure");
#endif DEBUGMAC
		VSIw->vistop = VSIw->vistop->next;
	  } /* while */

	sn = VSIw->Rbottom - VSIw->Rtop;
  /* update vertical scroll bar */
	RSbufinfo(w, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom);

	if (n <= VSIw->lines)
	  {
		RSdellines(w, 0, sn, n, 0);	/* scroll, preserving current selection */
		VSIcurson(w, VSIw->x, VSIw->y, 0); /* restore cursor at original position */
		VSredraw(w, 0, (sn + 1) - n, VSIw->maxwidth, sn); /* redraw newly-revealed portion */
	  } /* if */
	else
	  /* currently-visible contents scrolled completely off the screen--
		just redraw everything */
		VSredraw(w, 0, 0, VSIw->maxwidth, sn);

	return(0);
  } /* VSscrolforward */

short VSsetrgn
  (
	short w,
	short x1, /* leftmost column */
	short y1, /* uppermost line */
	short x2, /* rightmost column */
	short y2 /* bottommost line */
  )
  /* sets the visible region for the specified virtual screen
	in its window, scrolling its contents as appropriate. Assumes
	that either the vertical bounds or the height of the region has
	changed, but not both, and similarly that the horizontal bounds
	or the width has changed, but not both. */
  {
	short n, offset;

	if (VSvalids(w) != 0)
		return(-3);

	VSIw->Rbottom = VSIw->Rtop + (y2 - y1); /* make change in height of visible region first */

	if (x2 > VSIw->maxwidth)
	  {
	  /* trying to make columns visible which aren't there--
		adjust the left and right boundaries to avoid this */
		n = x2 - VSIw->maxwidth; /* how far to adjust to the left */
		if (n > x1)
			n = x1; /* but I'd rather have unused columns on the right than on the left */
		x1 -= n;							/* Adjust left	*/
		x2 -= n;							/* Adjust right */
	  } /* if */

	if (VSIw->Rleft != x1)
	  {
	  /* left margin changed -- scroll horizontally */
	  /* (assume width of region hasn't also changed) */
		n = x1 - VSIw->Rleft;
		if (n > 0)
			VSscrolright(w, n);
		else
			VSscrolleft(w, -n);
	  }
	else
	  /* just update horizontal scroll bar limits for new width of visible region */
		RSmargininfo(w, VSIw->maxwidth - (x2 - x1), x1);

	VSIw->Rleft = x1;
	VSIw->Rright = x2;

	if (VSIw->Rbottom > VSIw->lines)
	  /* don't scroll off the bottom of the screen */
		n = VSIw->Rbottom - VSIw->lines;
	else
	  /* scroll to new topmost line as specified */
		n = VSIw->Rtop - y1;

	if (n != 0)
	  /* scroll vertically (assume height of region hasn't also changed) */
		if (n > 0)
			VSscrolback(w, n);
		else
			VSscrolforward(w, -n);
	else
	  {
	  /* just redraw separator (if it's become visible) */
		x1 = y1 = 1;
		n = 132;
		if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset))
			RSdrawsep(w, n, 1);
	  /* and update vertical scroll bar limits for new height of visible region */
		RSbufinfo(w, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom);
	  } /* if */
	return(0);
  } /* VSsetrgn */

short VSscrolcontrol
  (
	short w,
	short scrolon, /* whether to save lines off top */
	short offtop /* whether to save lines on clearing entire screen */
  )
  /* changes scrollback flag settings for a virtual screen. */
  {
	if (VSvalids(w) != 0)
		return(-3);

	if (scrolon > -1)
		VSIw->savelines = scrolon;
	if (offtop > -1)
		VSIw->ESscroll = offtop;

	return(0);
  } /* VSscrolcontrol */

short VSgetrgn
  (
	short w,
	short *x1,
	short *y1,
	short *x2,
	short *y2
  )
  /* returns the current visible region for a virtual screen. */
  {
	if (VSvalids(w) != 0)
		return(-3);

	*x1 = VSIw->Rleft;
	*y1 = VSIw->Rtop;
	*x2 = VSIw->Rright;
	*y2 = VSIw->Rbottom;
   
	return(0);
  } /* VSgetrgn */

short VSsnapshot(short w)
  /* does nothing interesting. Not used. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	return(0);
  } /* VSsnapshot */

short VSmaxwidth
  (
	short w
  )
  /* returns one less than the number of columns on a virtual screen. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	return(VSIw->maxwidth);
  } /* VSmaxwidth */

VSlinePtr VSIGetLineStart(short w, short y1)
  /* returns a pointer to the specified text line (number may be
	negative for a line in the scrollback buffer). */
  {
	VSlinePtr ptr;
	short n;

	if (VSvalids(w) != 0)
		return((VSlinePtr) -3);

	if (y1 >= 0)
		return(VSIw->linest[y1]);

	n = y1 - VSIw->Rtop;				/* Number of lines from VISTOP to scroll forward */
	ptr = VSIw->vistop;
	while (n > 0)
	  {
		n--;
		ptr = ptr->next;
	  } /* while */
	while (n < 0)
	  {
		n++;
		ptr = ptr->prev;
	  } /* while */
	return(ptr);
  } /* VSIGetLineStart */

char *VSIstrcopy(char *src, short len, char *dest, short table)
  /* copies characters from *src (length len) to *dest, dropping
	trailing blanks. If table is nonzero, then this number (or more) of
	consecutive embedded blanks will be replaced with a tab. Returns a pointer
	to the position one past the last character copied to the *dest buffer. */
  {
	char *p, *tempp;
	short tblck;

	p = src + len - 1;
  /* skip trailing blanks */
	while ((*p == ' ') && (p >= src))
		p--;
	if (p < src)
		return(dest);
	if (!table)
	  /* straight character copy */
		while (src <= p)
			*dest++ = *src++;
	else
	  /* tab-replacement copy */
		while (src <= p)
		  {
			while ((src <= p) && (*src != ' '))
				*dest++ = *src++;
			if (src < p)
			  {
				tempp = dest; /* remember start of run of spaces */
				tblck = 0; /* length of run */
				while ((src <= p) && (*src == ' '))
				  {
					*dest++ = *src++;
					tblck++;
				  } /* while */
				if (tblck >= table)
				  {
					*tempp++ = '\011'; /* replace first space with a tab */
					dest = tempp; /* drop remaining spaces */
				  } /* if */
			  } /* if */
		  } /* while */
	return(dest);
  } /* VSIstrcopy */

long VSgettext(short w, short x1, short y1, short x2, short y2, char *charp, long max, char *EOLS, short table)
  /* copies a portion of text from the specified virtual screen into
	the *charp buffer. table, if nonzero, is the minimum length of
	runs of spaces to be replaced with single tabs. Returns the
	length of the copied text. max is supposed to be the maximum
	length to copy, but this is currently ignored!
	EOLS is the end-of-line sequence to insert at line boundaries.
	This is currently assumed to be exactly one character long. */
  {
#pragma unused(max) /* !! */
	short EOLlen;
	short lx,ly,					/* Upper bounds of selection */
		ux,uy;					/* Lower bounds of selection */
	short maxwid;
	char *origcp;
	VSlinePtr t;

	if (VSvalids(w) != 0)
		return(-3);
	EOLlen = strlen(EOLS);
	maxwid = VSIw->maxwidth;
	origcp = charp;

	if (y1 < -VSIw->numlines)
	  {
		y1 = -VSIw->numlines;
		x1 = -1;
	  } /* if */
	if (y1 == y2)
	  {
	  /* copying no more than a single line */
		t = VSIGetLineStart(w, y1);
		if (x1 < x2)	/* Order the lower and upper bounds */
		  {
			ux = x1;
			uy = y1;
			lx = x2;
			ly = y2;
		  }
		else
		  {
			ux = x2;
			uy = y2;
			lx = x1;
			ly = y1;
		  } /* if */

		if ((long)(lx-ux) < max)
			charp=VSIstrcopy(&t->text[ux+1], lx-ux, charp, table);
		else
			charp=VSIstrcopy(&t->text[ux+1], (short)(max - (long)(charp-origcp)), charp, table);
		if (lx == maxwid)
			*charp++ = *EOLS; /* assumes it's only one character! */
	  }
	else
	  {
	  /* copying more than one line */
		if (y1 < y2)	/* Order the lower and upper bounds */
		  {
			ux = x1;
			uy = y1;
			lx = x2;
			ly = y2;
		  }
		else
		  {
			ux = x2;
			uy = y2;
			lx = x1;
			ly = y1;
		  } /* if */
		t = VSIGetLineStart(w, uy);
		if (((long) (maxwid-ux) < max))
			charp=VSIstrcopy(&t->text[ux+1],maxwid-ux,charp,table);
		else
			charp=VSIstrcopy(&t->text[ux+1],(short) (max-(long)(charp-origcp)),charp,table);
		*charp++ = *EOLS; /* assumes it's only one character! */
		uy++;
		t = t->next;
		while (uy < ly && uy < VSIw->lines)
		  {
			if ((long)(maxwid+1) < max)
				charp=VSIstrcopy(t->text,maxwid+1,charp, table);
			else
				 charp=VSIstrcopy(t->text,(short)(max - (long) (charp-origcp)),charp, table);
			*charp++=*EOLS;
			t=t->next; 
			uy++;
		  } /* while */
		if (ly > VSIw->lines)
			lx = maxwid;

		if ((long) (lx+1) < max)
			charp=VSIstrcopy(t->text,lx+1,charp,table);
		else
			charp=VSIstrcopy(t->text,(short)(max - (long)(charp-origcp)),charp,table);

		if (lx >= maxwid)
			*charp++ = *EOLS; /* assumes it's only one character! */
	  } /* if */
	return(charp - origcp);
  } /* VSgettext */

short VSgetlines
  (
	short w
  )
  /* returns the number of lines in a virtual screen. */
  {
	if (VSvalids(w) != 0)
		return(-3);
	return(VSIw->lines + 1);
  } /* VSgetlines */

short VSsetlines
  (
	short w, /* window number */
	short lines /* new number of lines */
  )
  /* sets the number of lines in a virtual screen, reallocating text
	and attribute arrays accordingly. Returns the new number of lines
	on success. */
  {
	VSlineArray attrst, linest;				/* For storage of old ones */
	VSlinePtr line;							/* pointer to a line */
	short i, j, oldlines;
	char *temp, *tempa;
	
	if (VSvalids(w) != 0)
		return(-3);
	
	lines -= 1;								/* Correct for internal use */
	oldlines = VSIw->lines;
	if (lines == oldlines)	
	  /* no change */
		return(0);
	
	VSIw->x = 0;
	VSIw->y = 0;
	VSIcurson(w, VSIw->x, VSIw->y, 1); 	/* keeps cursor from pointing outside of window */

  /* VSIw->scrntop = VSIw->linest[0]; */
	VSIw->vistop = VSIw->scrntop;			/* Force view to the top of the screen */
  /* VSIlistndx(VSIw->scrntop, VSIw->attrst[0]); */
	attrst = VSIw->attrst; /* save old screen arrays */
	linest = VSIw->linest;
  /* allocate new screen buffers for text and attributes */
	VSIw->linest = VSInewlinearray(lines + 1);
	if (!VSIw->linest)
	  {
	  /* aarrgghh -- out of memory */
		VSIw->linest = linest;
		return -2;
	  } /* if */
	VSIw->attrst = VSInewlinearray(lines + 1);
	if (!VSIw->attrst)
	  {
	  /* aarrgghh -- out of memory */
		DisposPtr((Ptr) VSIw->linest);
		VSIw->linest = linest;
		VSIw->attrst = attrst;
		return -2;
	  } /* if */
	VSIw->lines = lines; /* set new number of screen lines */

	VSIw->linest[0] = VSInewlines(lines + 1); /* allocate new text and attribute lines */
	VSIw->attrst[0] = VSInewlines(lines + 1);
	if (VSIw->linest[0] && VSIw->attrst[0])
	  {	/* mem is there */
		VSIlistndx(VSIw->linest[0], VSIw->attrst[0]); /* build the new screen arrays */
		if (VSIw->savelines)
		  {
		  /* save previous screen contents in scrollback buffer */
			line = linest[oldlines]->next;				/* save continuation */
			linest[oldlines]->next = VSIw->linest[0];
			VSIw->linest[lines]->next = line;			/* restore continuation */
			VSIw->linest[0]->prev = linest[oldlines];	/* backpointer */
			if (line)									/* if there was a follower */
				line->prev = VSIw->linest[lines];		/* new prev for it */
			VSIw->numlines += oldlines;					/* we made more scrollback */
		  }
		else
		  /* get rid of previous screen contents */
			VSIfreelinelist(linest[0]);
	  }
	else
	  {										/* need more mem - emergency */
		if (VSIw->linest[0])
			VSIfreelinelist(VSIw->linest[0]);
		if (VSIw->attrst[0])
			VSIfreelinelist(VSIw->attrst[0]);
		VSIfreelines();							/* release them all */
		VSIw->linest[0] = VSInewlines(lines + 1); /* allocate new screen arrays */
		VSIw->attrst[0] = VSInewlines(lines + 1);
		if (!VSIw->linest[0] || !VSIw->attrst[0])
		  /* aarrgghh -- still out of memory -- give up */
		  /* shouldn't I free the block if I got only one? */
			return(-2);
		VSIw->buftop = VSIw->linest[0];
		VSIw->numlines = 0; /* nothing in scrollback */
	  } /* if */

	VSIw->scrntop = VSIw->linest[0];			/* new top of screen */
	VSIw->vistop = VSIw->scrntop;				/* Force a scroll to the top of the screen */
	VSIlistndx(VSIw->scrntop, VSIw->attrst[0]); /* rebuild screen arrays */	
	VSIw->attrst[0]->prev = VSIw->attrst[lines];	/* Make attribute list circular */
	VSIw->attrst[lines]->next = VSIw->attrst[0];
	if (!VSIw->savelines)
	  {
	  /* make text line list circular to indicate no extensions */
		VSIw->linest[lines]->next = VSIw->linest[0];
		VSIw->linest[0]->prev = VSIw->linest[lines];
	  } /* if */

  /* initialize the new screen lines to blank text and no attributes */
	for (i = 0; i <= lines; i++)
	  {
		tempa = VSIw->attrst[i]->text;
		temp = VSIw->linest[i]->text;
		for (j = 0; j <= VSIw->allwidth; j++)
		  {
			*temp++ = ' ';
			*tempa++ = 0;
		  } /* for */
	  } /* for */
  /* reset scrolling region */
	VSIw->top = 0;
	VSIw->bottom = lines;
 /* reset visible region */
	VSIw->Rtop = 0;
	VSIw->Rbottom = lines;

  /* dispose of old screen arrays and attribute lines */
	VSIfreelinelist(attrst[0]);
	DisposPtr((Ptr) attrst);
	DisposPtr((Ptr) linest);
	
	VSredraw(w, 0, 0, VSIw->maxwidth, lines); /* draw new blank screen */
	RSbufinfo(w, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom); /* adjust vertical scroll bar */

	return(VSIw->lines);

} /* VSsetlines */


/*--------------------------------------------------------------------------*/
/* NCSA: SB - VSsetcols 													*/
/* 	This allows external procedures to set the column width.  Used by 		*/
/*	routines in main, to allow ARBITRARY column widths						*/
/*--------------------------------------------------------------------------*/
short VSsetcols(short w, short myWidth)									/* NCSA: SB */
{																	/* NCSA: SB */
	if (VSvalids(w) != 0)											/* NCSA: SB */
		return(-1);													/* NCSA: SB */
	VSIw->maxwidth = myWidth;										/* NCSA: SB */
} 																	/* NCSA: SB */


/*--------------------------------------------------------------------------*/
/* NCSA: SB - VSgetcols 													*/
/* 	This returns the column width.  Used by SetScreenDimensions, when		*/
/*	the procedure needs to know the initial column width					*/
/*--------------------------------------------------------------------------*/
short VSgetcols(short w)												/* NCSA: SB */
{																	/* NCSA: SB */
	if (VSvalids(w) != 0)											/* NCSA: SB */
		return(-1);													/* NCSA: SB */
	return VSIw->maxwidth;											/* NCSA: SB */
} 																	/* NCSA: SB */

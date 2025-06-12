#define NCSA_TELNET
/*
 *
 *      Virtual Screen Kernel Macintosh Real Screen Interface
 *                          (rsmac.c)
 *
 *   National Center for Supercomputing Applications
 *      by Gaige B. Paulsen
 *
 *    This file contains the macintosh real screen calls for the NCSA
 *  Virtual Screen Kernel.
 *
 *      RSbell(w)                   - Ring window w's bell
 *      RScursoff(w)                - Turn the cursor off in w
 *      RScurson(w,x,y)             - Turn the cursor on in w at x,y
 *      RSdraw(w,x,y,a,len,ptr)     - Draw @x,y in w string@ptr for length len
 *      RSdelchars(w,x,y,n)         - Delete n chars in w from x,y
 *      RSdellines(w,t,b,n)         - Delete n lines in region t->b in w
 *      RSerase(w,x1,y1,x2,y2)      - Erase from x1,y1 to x2,y2 in w
 *      RSinitall()                 - Initialize the world if necessary
 *      RSinslines(w,t,b,n)         - Insert n lines in region t->b in w
 *      RSinsstring(w,x,y,a,len,ptr)- Insert len chars @x,y in w attrib a
 *      RSsendstring(w,ptr,len)     - Send string @ptr length len from window w
 *		RSbufinfo( w, total,current)- Tells you the total/current lines in buffer
 *		RSdrawsep( w, y1,draw)		- Tells you to draw the line at y1 (0 is erase)
 *		RSmargininfo( w, total, current)	- Tells you total/current columns in VS
 *
 *
 *  Macintosh only Routines:
 *	NI	RSregnconv( *)				- Convert region to rect coords
 *  NI  RSsetwind(w)                - Set the port and vars to window w
 *  NI  RSsetattr(a)                - Set font/text style to a
 *	NI	RSsetConst(w)
 *	ML	RSattach(w,wind)			- Attach the RS (w) to window wind
 *	ML	RSdetach(w)				- Ready window for go-away
 *	ML	RSselect(w,pt,shift)		- Handle selection RS (w) point (pt) and (shift) if held down
 *	ML	RSzoom(window,code,shifted)	- Zoom Box handling
 *	ML	RSsize( window, where)		- Resize handling
 *	IN	RSgetwindow(w)				- Get the WindowPtr for RS (w)
 *	IN	RSfindvwind(wind)			- Find the (RS/VS) # of wind
 *	IN	RSfindscroll( control, n)	- Find which VS the control is in and which control it is
 *  	RSupdate(wind)				- Handle updates on WIND, return 0 if not an RS
 *		RSactivate(w)				- Handle activate events 
 *		RSdeactivate(w)				- Handle deactivate events 
 *		RSGetTextSel(w,table)		- Returns handle to text selection of window w, table->tabs
 *		RSchangefont(w,fnum,fsiz,r)	- Set (w) to font fnum, size fsiz, resize window if (r)
 *		RSgetfont(w, &fnum, &fsiz)	- Get the font size and number into fnum and fsiz
 *		RSnewwindow( wDims, sb, wid, lines
 *			name,wrap,fnum,fsiz,
 *			showit, goaway)			- Returns VS # of newly created text window -
 *									  wDims (dimension),sb(scrollback),wid(width 80/132),
 *                                    lines (# of lines, 24 <> 66),
 *									  name(window), wrap(0/1),fnum,fsiz, showit(vis),goaway(0,1)
 *		RSkillwindow( w)			- Destroys, deallocates, kills window (w)
 *		RSclick(window, eventRecord)- Handle clicks in window (returns false if not RS window)
 *		RShide(w)					- Hide RS (w)
 *		RSshow(w)					- Show RS (w)
 *		RScprompt(w, FilterProc)	- Prompt for colors...FilterProc is for Modal Dialog
 *		RSsetcolor(w,n,r,g,b)		- Set one of the 4 colors of RS (w) to R,G,B
 *		RSgetcolor(w,n,r,g,b)		- Get one of the 4 colors of RS (w) into R,G,B
 *		RSmouseintext(w,myPoint)	- Returns true if Mouse is in text part of current RS window
 *		RSskip(w,on)				- Activate/deactivate drawing in an RS
 *		
 *		IN - Informational
 *		ML - Mid Level
 *		NI - Necessary Internal
 *		   - Suggested calls
 *
 *      Version Date    Notes
 *      ------- ------  ---------------------------------------------------
 *      0.01    861102  Initial coding -GBP
 *      0.25    861106  Added code from screen.c -GBP
 *      0.50    861113  First compiled edition -GBP
 *		2.1		871130	NCSA Telnet 2.1 -GBP
 *		2.2 	880715	NCSA Telnet 2.2 -GBP
 *		2.6		7/92	put globals into struct, cursors into array, and cleaned up 
 *						some of the font typedefs			Scott Bulmahn
 *		2.6b4	12/92	Cleaned up the code, and added double clicking -- Scott Bulmahn
 *
 */

#ifdef MPW
#pragma segment RS
#endif

#define __ALLNU__
#include <OSUtils.h>
#include <Palettes.h>
#include <Picker.h>		/* BYU LSC */
#include <Fonts.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "TelnetHeader.h"
#include "DlogUtils.proto.h"
#include "configure.proto.h"	// For colorboxmodalproc and colorboxproc
#include "maclook.proto.h"
#include "network.proto.h"
#include "menuseg.proto.h"
#include "vskeys.h"
#include "vsdata.h"
#include "vsinterf.proto.h"
#include "vsintern.proto.h"
#include "telneterrors.h"
#include "wind.h"
#include "rsdefs.h"
#include "parse.proto.h"		// For SendNAWSinfo proto
#include "wdefpatch.proto.h"	/* 931112, ragge, NADA, KTH */

#define NFDEF {0,0,0}
#define NBDEF {65535,65535,65535}
#define BFDEF {0,61183,11060}
#define BBDEF {61183,2079,4938}
#define UFDEF {1,0,0}
#define UBDEF {0,0,0}

#define	MYSETRECT(a,b,c,d,e)	a.left = b; \
								a.top = c;\
								a.right = d;\
								a.bottom = e;\
								
#define INFINITY	20000		// Will screens ever be this large?

#ifdef NCSA_TELNET
extern WindRec *screens;
#endif NCSA_TELNET
extern SysEnvRec	theWorld;

#include "rsmac.proto.h"

void RScheckmaxwind(Rect *,short , 
			short , short *, short *);

void RScalcwsize(short w, short width);

short MaxRS;

RSdata *RSlocal, *RScurrent;
Rect	noConst,
		RScur;				/* cursor rectangle */

RgnHandle RSuRgn;            /* update region */

short RSw=-1,         /* last window used */
    RSa=0;          /* last attrib used */
short RScolor=1;		/* true if try to use color stuff */

short RScolors[]=
	{ 33,			/* black */
	  30,			/* white */
	  205,			/* red */
	  341,			/* green */
	  409,			/* blue */
	  273,			/* cyan */
	  137,			/* magenta */
	  69			/* yellow */
	  };

void	RSunload(void) {}

void RSinitall
  (
	short max /* max nr windows to allow */
  )
  /* initializes handling of terminal windows. */
  {
    short i;
	extern SysEnvRec theWorld;

	MaxRS = max;
	RSlocal = (RSdata *) NewPtrClear(MaxRS * sizeof(RSdata));
    for (i = 0; i < MaxRS; RSlocal[i++].window = 0L)
	  {
		RScurrent = RSlocal + i;
		RScurrent->id = 'RSDA';
		RScurrent->color[0] = 0;			/* Foreground */
		RScurrent->color[1] = 1;			/* Background */
		RScurrent->color[2] = 2;			/* Blink Fore */
		RScurrent->color[3] = 3;			/* Blink Back */
		RScurrent->cursor.top = 0;
		RScurrent->cursor.bottom = 0;
		RScurrent->cursor.left = 0;
		RScurrent->cursor.right = 0;
	  }
    RSuRgn = NewRgn();
    RScur.left = 0;
    RScur.top = 0;
    RScur.bottom = 0;
    RScur.right = 0;
	RScolor = theWorld.hasColorQD;
  } /* RSinitall */

short RSTextSelected(short w) {		/* BYU 2.4.11 */
  return(RSlocal[w].selected);	/* BYU 2.4.11 */
}								/* BYU 2.4.11 */

void RSsetConst
  (
	short w
  )
  /* sets "noConst" global to a zero-based rectangle equal in size
	to the specified terminal window. */
  {
  noConst.left = 0;
  noConst.top = 0;
  noConst.right = RSlocal[w].width;
  noConst.bottom = RSlocal[w].height;
  } /* RSsetConst */

/****************************************************************************/
/*  Given a window record number, do a SetPort() to the window associated with
*   that window record.
*/
short RSsetwind
  (
	short w
  )
  {
	if ((w < 0) || (w > MaxRS))
		return(-3);
    if (RSw != w)								/* if last window used is different */
	  {
        if (RSlocal[w].window == 0L)
			return(-4);
		RScurrent = RSlocal + w;
        RSw = w;
		RSa = -1; /* attributes will need setting */
		SetPort(RScurrent->window);
		return(1);
	  }
    SetPort(RScurrent->window);
	return(0);
  } /* RSsetwind */

void RSbell
  (
	short w
  )
  /* gives an audible signal associated with the specified window. */
  {
    RSsetwind(w);
	if (FrontWindow() != RScurrent->window)
	  {
	  /* beep and temporarily invert the window contents, so
		the user sees which window is beeping */
	    InvertRect(&RScurrent->window->portRect);
	    SysBeep(8);
	    InvertRect(&RScurrent->window->portRect);
	  }
	else
	  /* window is frontmost--just beep */
		SysBeep(8);
  } /* RSbell */

void	RScursblink( short w)		/* Blinks the cursor */
{
	GrafPtr	oldwindow;
	long	now = TickCount();
	
	if (now > TelInfo->blinktime) {
		if (VSvalids(w) != 0)			/* BYU 2.4.12 */
			return;						/* BYU 2.4.12 */
		if (!VSIcursorvisible())		/* BYU 2.4.12 */
			return;						/* BYU 2.4.12 - cursor isn't visible */
	
		GetPort(&oldwindow);			/* BYU 2.4.11 */
		TelInfo->blinktime = now + 40;			/* BYU 2.4.11 */
		RSlocal[w].cursorstate ^= 1; 	/* BYU 2.4.11 */
		SetPort(RSlocal[w].window);		/* BYU 2.4.11 */
		InvertRect(&RSlocal[w].cursor);	/* BYU 2.4.11 */
		SetPort(oldwindow);				/* BYU 2.4.11 */
	}
} /* RScursblink */

void RScursblinkon						/* BYU 2.4.18 */
  (										/* BYU 2.4.18 */
    short w								/* BYU 2.4.18 */
  )										/* BYU 2.4.18 */
  /* Blinks the cursor */				/* BYU 2.4.18 */
  {										/* BYU 2.4.18 */
  	if (!RSlocal[w].cursorstate) {		/* BYU 2.4.18 */
		GrafPtr oldwindow;				/* BYU 2.4.18 */
		GetPort(&oldwindow);			/* BYU 2.4.18 */
		RSlocal[w].cursorstate = 1; 	/* BYU 2.4.18 */
		SetPort(RSlocal[w].window);		/* BYU 2.4.18 */
		InvertRect(&RSlocal[w].cursor);	/* BYU 2.4.18 */
		SetPort(oldwindow);				/* BYU 2.4.18 */
	}									/* BYU 2.4.18 */
  } /* RScursblink */					/* BYU 2.4.18 */

void RScursblinkoff						/* BYU 2.4.11 */
  (										/* BYU 2.4.11 */
    short w								/* BYU 2.4.11 */
  )										/* BYU 2.4.11 */
  /* Blinks the cursor */				/* BYU 2.4.11 */
  {										/* BYU 2.4.11 */
  	if (RSlocal[w].cursorstate) {		/* BYU 2.4.11 */
		GrafPtr oldwindow;				/* BYU 2.4.11 */
		GetPort(&oldwindow);			/* BYU 2.4.11 */
		RSlocal[w].cursorstate = 0; 	/* BYU 2.4.11 */
		SetPort(RSlocal[w].window);		/* BYU 2.4.11 */
		InvertRect(&RSlocal[w].cursor);	/* BYU 2.4.11 */
		SetPort(oldwindow);				/* BYU 2.4.11 */
	}									/* BYU 2.4.11 */
  } /* RScursblink */					/* BYU 2.4.11 */

void RScursoff
  (
	short w
  )
  /* hides the text cursor for the specified window. Assumes it
	is currently being shown. */
  {
	if (RSlocal[w].skip || !RSlocal[w].cursorstate)		/* BYU 2.4.11 */
		return;
    RSsetwind(w);
    RScurrent->cursorstate = 0;							/* BYU 2.4.11 */
	InvertRect(&RScurrent->cursor);
  } /* RScursoff */

void RScurson
  (
	short w,
	short x,
	short y
  )
  /* displays the text cursor for the specified window, at the
	specified position. Assumes it isn't currently being shown. */
  {
	if (RSlocal[w].skip || RSlocal[w].cursorstate)		/* BYU 2.4.11 */
		return;
    RSsetwind(w);

    RScurrent->cursor.left = x * RScurrent->fwidth;			/* BYU 2.4.11 */
    RScurrent->cursor.top  = y * RScurrent->fheight;		/* BYU 2.4.11 */

	switch (gApplicationPrefs->CursorType) {											/* BYU 2.4.11 */
		case UNDERSCORECURSOR:										/* BYU 2.4.11 */
    		RScurrent->cursor.top  += RScurrent->fheight;			/* BYU 2.4.11 */
    		RScurrent->cursor.right  = RScurrent->cursor.left + RScurrent->fwidth;	/* BYU 2.4.11 */
    		RScurrent->cursor.bottom = RScurrent->cursor.top + 1;	/* BYU 2.4.11 */
			break;
		case VERTICALCURSOR:										/* BYU 2.4.11 */
    		RScurrent->cursor.left += 2;							/* BYU 2.4.11 */
    		RScurrent->cursor.right  = RScurrent->cursor.left + 1;	/* BYU 2.4.11 */
    		RScurrent->cursor.bottom = RScurrent->cursor.top + RScurrent->fheight;	/* BYU 2.4.11 */
			break;
		case BLOCKCURSOR:											/* BYU 2.4.11 */
		default:													/* BYU 2.4.11 */
    		RScurrent->cursor.right  = RScurrent->cursor.left + RScurrent->fwidth;	/* BYU 2.4.11 */
    		RScurrent->cursor.bottom = RScurrent->cursor.top + RScurrent->fheight;	/* BYU 2.4.11 */
			break;
    }

	if (!gApplicationPrefs->BlinkCursor) {									/* BYU 2.4.11 */
    	RScurrent->cursorstate = 1;						/* BYU 2.4.11 */
    	InvertRect(&RScurrent->cursor);					/* BYU 2.4.11 */
	}													/* BYU 2.4.11 */
  } /* RScurson */

void RSTextFont(short myfnum, short myfsiz, short myface) 				/* BYU */
{										/* BYU */
	if ((myfnum == monaco) && 			/* BYU - If Monaco, size 9, and bold, then */
		(myfsiz == 9) &&				/* BYU */
		(myface & bold))	{			/* BYU */
		TextFont(75);					/* BYU - use NCSA's Monaco. */
	} else {							/* BYU */
		TextFont(myfnum);				/* BYU */
	}									/* BYU */
}										/* BYU */

void RSsetattr
  (
	short a
  )
  /* sets the text attributes for drawing into the current window. */
  {
 	GrafPtr	thePort;			/* NCSA: SB */
	
	GetPort(&thePort);			/* NCSA: SB */

   if (VSisgrph(a))
        TextFont(74); /* use "NCSA VT" font for special graphics */
	else
#if 1													/* BYU */
		RSTextFont(RScurrent->fnum,RScurrent->fsiz,a); 	/* BYU - use user-selected text font */
#else
		TextFont(RScurrent->fnum); /* use user-selected text font */
#endif

	TextSize(RScurrent->fsiz);

/* BYU - bold system fonts don't work (they overwrite the scroll bars), 
         but NCSA's 9 point Monaco bold works okay. */

    TextFace((a & outline) >> 1); 	/* BYU - do outline as underline setting */

#ifdef GOTTHEGOUGH
	if (a & bold)									/* NCSA: SB */
		TextFace(thePort->txFace + bold);			/* NCSA: SB */
#endif

  /* set up reverse video setting */
	if (RScolor)
	  {
		if (VSisrev(a))
			TextMode(notSrcCopy);
		else
			TextMode(srcCopy);
	  }
	else
	  {
		if (VSisrev(a))
		  {
        	BackPat(PATTERN(qd.black));	/* Reverses current attributes regard */
        	PenPat(PATTERN(qd.white));	/* less of the color, etc.... */
          }
		else
		  {
        	BackPat(PATTERN(qd.white));
        	PenPat(PATTERN(qd.black));
          } /* if */
	  } /* if */
  /* use colors to stand in for blink setting */
	if (VSisblnk(a))
	  {
		if (RScolor)
		  {
			PmForeColor(2);
			PmBackColor(3);
		  }
		else
		  {
			ForeColor((long) RScolors[RScurrent->color[2]]);	/* Blink foreground */
			BackColor((long) RScolors[RScurrent->color[3]]);	/* Blink Background */
		  } /* if */
	  }
	else
	  {
		if (RScolor)
		  {
			PmForeColor(0);
			PmBackColor(1);
		  }
		else
		  {
			ForeColor((long) RScolors[RScurrent->color[0]]);	/* normal foreground */
			BackColor((long) RScolors[RScurrent->color[1]]);	/* normal Background */
		  } /* if */
	  } /* if */
    RSa = a;
  } /* RSsetattr */

#ifdef	NO_UNIVERSAL
#define LMGetHiliteMode() (* (unsigned char *) 0x0938)
#define LMSetHiliteMode(HiliteModeValue) ((* (unsigned char *) 0x0938) = (HiliteModeValue))
#endif

void DoHiliteMode(void)		/* BYU LSC */
  /* enables use of highlighting in place of simple color inversion
	for next QuickDraw operation. */
  {
  	
  	LMSetHiliteMode(LMGetHiliteMode() & 0x7F);
//	char *p = (char *) 0x938; /* pointer to HiliteMode low-memory global */
//	*p = *p & 0x7f; /* clear the HiliteBit */
  } /* HiliteMode */

void RSinvText
  (
	short w, 
	Point curr,
	Point last,
	RectPtr constrain /* don't highlight anything outside this rectangle */
  )
  /* highlights the text from curr to last inclusive. */
  {
	Rect temp, temp2;
	Point lb, ub;

	RSsetwind(w);
  /* normalize coordinates with respect to visible area of virtual screen */
	curr.v -= RScurrent->topline;
	curr.h -= RScurrent->leftmarg;
	last.v -= RScurrent->topline;
	last.h -= RScurrent->leftmarg;

	if (curr.v == last.v)
	  {
	  /* highlighted text all on one line */
		if (curr.h < last.h) /* get bounds the right way round */
		  {
			ub = curr;
			lb = last;
		  }
		else
		  {
			ub = last;
			lb = curr;
		  } /* if */
		MYSETRECT /* set up rectangle bounding area to be highlighted */
		  (
			temp,
			(ub.h + 1) * RScurrent->fwidth,
			ub.v * RScurrent->fheight,
			(lb.h + 1) * RScurrent->fwidth,
			(lb.v + 1) * RScurrent->fheight
		  );
		SectRect(&temp, constrain, &temp2); /* clip to constraint rectangle */
		DoHiliteMode();						/* BYU LSC */
		InvertRect(&temp2);
	  }
	else
	  {
	  /* highlighting across more than one line */
		if (curr.v < last.v)
			ub = curr;
		else
			ub = last;
		if (curr.v > last.v)
			lb = curr;
		else
			lb = last;
		MYSETRECT /* bounds of first (possibly partial) line to be highlighted */
		  (
			temp,
			(ub.h + 1) * RScurrent->fwidth,
			ub.v * RScurrent->fheight,
			RScurrent->width,
			(ub.v + 1) * RScurrent->fheight
		  );
		SectRect(&temp, constrain, &temp2); /* clip to constraint rectangle */
		DoHiliteMode();						/* BYU LSC */
		InvertRect(&temp2);
		MYSETRECT /* bounds of last (possibly partial) line to be highlighted */
		  (
			temp,
			0,
			lb.v * RScurrent->fheight,
			(lb.h + 1) * RScurrent->fwidth,
			(lb.v + 1) * RScurrent->fheight
		  );
		SectRect(&temp, constrain, &temp2); /* clip to constraint rectangle */
		DoHiliteMode();						/* BYU LSC */
		InvertRect(&temp2);

		if (lb.v - ub.v > 1) /* highlight extends across more than two lines */
		  {
		  /* highlight complete in-between lines */
			SetRect
			  (
				&temp,
				0,
				(ub.v + 1) * RScurrent->fheight,
				RScurrent->width,
				lb.v * RScurrent->fheight
			  );
			SectRect(&temp, constrain, &temp2); /* clip to constraint rectangle */
			DoHiliteMode();						/* BYU LSC */
			InvertRect(&temp2);

		  } /* if */
	  } /* if */
  } /* RSinvText */

void RSdraw
  (
	short w, /* window number */
	short x, /* starting column */
	short y, /* line on which to draw */
	short a, /* text attributes */
	short len, /* length of text to draw */
	char *ptr /* pointer to text */
  )
  /* draws a piece of text (assumed to fit on a single line) in a window,
	using the specified attributes. If any part of the text falls
	within the current selection, it will be highlighted. */
  {
    Rect rect;
	static char temp[128];
	short ys;
	static short theLen;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
    RSsetattr(0);
    MYSETRECT /* set up rectangle bounding text being drawn */
	  (
		rect,
		x * RScurrent->fwidth,
		y * RScurrent->fheight,
		(x + len) * RScurrent->fwidth,
		(y + 1) * RScurrent->fheight
	  );
    if (RSa != a)
		RSsetattr(a);
	if (x <= 0)			/* BYU 2.4.12 - Without this, 1 pixel column of reverse */
	  rect.left = -3;	/* BYU 2.4.12 - video text does not clear at left margin */
#ifdef OLDM
	if (!RScolor)
#endif OLDM
	    EraseRect(&rect);
	if (x <= 0)			/* BYU 2.4.12 - Okay, just putting it back the way it was */
	  rect.left = 0;	/* BYU 2.4.12 */
	ys = y * RScurrent->fheight + RScurrent->fascent;
	    MoveTo(x * RScurrent->fwidth, ys);
	strncpy(temp,ptr,len);
	temp[len]=0;
	theLen = StringWidth((StringPtr)temp);
	
#ifdef NEWTXT
	    DrawText(ptr, 0, len);
		
#else
	if (RScurrent->monospaced)
	    DrawText(ptr, 0, len);
	else
		while (len--)
		  {
		  /* draw the characters one at a time to defeat the proportional spacing */
			DrawChar(*ptr++);
			MoveTo((++x) * RScurrent->fwidth, ys);  /*good version */
/*			MoveTo((++x) * CharWidth(*(ptr-1)), ys); */
		  } /* while */
#endif
	if (RScurrent->selected)
		RSinvText(w, *(Point *) &RScurrent->anchor,
			*(Point *) &RScurrent->last, &rect);
  } /* RSdraw */

void RSdelcols
  (
	short w,
	short n /* number of columns to scroll */
  )
  /* scrolls the entire visible display of a virtual screen the
	specified number of columns to the left, blanking out
	the newly-revealed area. */
  {
    Rect rect;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
    MYSETRECT /* bounds of entire text area, for scrolling */
	  (
		rect,
		0,
		0,
		RScurrent->width,
		RScurrent->height
	  );
    ScrollRect(&rect, -n * RScurrent->fwidth, 0, RSuRgn);
    InvalRgn(RSuRgn);
    ValidRect(&rect); /* any necessary redrawing in newly-revealed area will be done by caller */
    MYSETRECT /* bounds of newly-revealed area */
	  (
		rect,
		RScurrent->width - (n * RScurrent->fwidth),
		0,
		RScurrent->width,
		RScurrent->height
	  );
	if (RScurrent->selected)
	  /* highlight any newly-revealed part of the current selection */
		RSinvText(w, *(Point *) &RScurrent->anchor,
			*(Point *) &RScurrent->last, &rect);
  } /* RSdelcols */

void RSdelchars
  (
	short w, /* affected window */
	short x, /* column to delete from */
	short y, /* line on which to do deletion */
	short n /* number of characters to delete */
  )
  /* deletes the specified number of characters from the specified
	position to the right, moving the remainder of the line to the
	left. */
  {
    Rect rect;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
    RSsetattr(0); /* avoid funny pen modes */
    MYSETRECT /* bounds of area from starting column to end of line */
	  (
		rect,
		x * RScurrent->fwidth,
		y * RScurrent->fheight,
		RScurrent->width,
		(y + 1) * RScurrent->fheight
	  );
	if ((x + n) * RScurrent->fwidth > RScurrent->width)
	  /* deleting to end of line */
		EraseRect(&rect);
	else
	  {
	  /* scroll remainder of line to the left */
    	ScrollRect(&rect, - RScurrent->fwidth * n, 0, RSuRgn);
    	InvalRgn(RSuRgn);
   		ValidRect(&rect); /* leave newly-revealed area blank */
		if (RScurrent->selected)
		  {
		  /* highlight any part of selection which lies in newly-blanked area */
			HLock((Handle) RSuRgn);
			RSinvText(w, *(Point *) &RScurrent->anchor, *(Point *) &RScurrent->last, &((*RSuRgn)->rgnBBox));
			HUnlock((Handle) RSuRgn);
		  } /* if */
	  } /* if */
  } /* RSdelchars */

void RSdellines
  (
	short w, /* affected window */
	short t, /* top line of affected region */
	short b, /* bottom line of affected region */
	short n, /* number of lines to delete */
	short scrolled
	  /*
		-ve => cancel current selection, if any;
		+ve => selection has moved up one line;
		0 => don't touch selection
	  */
  )
  /* deletes lines at the top of the specified region of a window,
	inserting new blank lines at the bottom, and scrolling up the
	stuff in between. */
  {
    Rect	rect;
	short	RSfheightTimesn, RSfheightTimesbplus1;

	if (RSlocal[w].skip)
		return;

    RSsetwind(w);
	RSsetConst(w);
    RSsetattr(0); /* avoid funny pen modes */

	if (scrolled)
	  {
		if (RScurrent->selected && scrolled < 0)
		  {
		  /* unhighlight and cancel current selection */
			RSinvText(w, *(Point *) &RScurrent->anchor, *(Point *) &RScurrent->last, &noConst);
			RScurrent->selected = 0;
		  }
		else
		  {
			RScurrent->last.v -= 1;		/* Subtract one from each of the */
			RScurrent->anchor.v -= 1;	/* vertical Selection components */
		  } /* if */
	  } /* if */

    rect.left = -1;							/* BYU 2.4.12 - necessary */
    rect.right = RScurrent->width;
    rect.top = t * RScurrent->fheight;
    RSfheightTimesbplus1 = (b + 1) * RScurrent->fheight;
	rect.bottom = RSfheightTimesbplus1;

  /* adjust the update region to track the scrolled window contents */
  	RSfheightTimesn = RScurrent->fheight * n;
	OffsetRgn(((WindowPeek) RScurrent->window)->updateRgn,
		0, -RSfheightTimesn);
    ScrollRect(&rect, 0, -RSfheightTimesn , RSuRgn);
    RSsetattr(VSIw->attrib); /* restore mode for text drawing */
    InvalRgn(RSuRgn);

  /* validate the area containing the newly-inserted blank lines. */
  /* any necessary redrawing in newly-revealed area will be done by caller */
	MYSETRECT
	  (
		rect,
		0,
		(b - n + 1) * RScurrent->fheight - 1,
		RScurrent->width,
		RSfheightTimesbplus1 + 1
	  );

    ValidRect(&rect);
  } /* RSdellines */

void RSerase
  (
	short w, /* affected window */
	short x1, /* left column */
	short y1, /* top line */
	short x2, /* right column */
	short y2 /* bottom line */
  )
  /* erases a rectangular portion of the screen display, preserving
	the selection highlight. */
  {
    Rect rect;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
    RSsetattr(0); /* avoid funny pen modes */
    SetRect
	  (
		&rect,
		x1 * RScurrent->fwidth,
		y1 * RScurrent->fheight,
		(x2 + 1) * RScurrent->fwidth - 1,
		(y2 + 1) * RScurrent->fheight + 1
	  );
	if (rect.left <= 0)						/* little buffer strip on left */
		rect.left = CHO;
	if (rect.right >= RScurrent->width - 1)
		rect.right = RScurrent->rwidth - 2;	/* clear to edge of window, including edge strip */
	if (rect.bottom >= RScurrent->height - 2)
		rect.bottom = RScurrent->rheight + 1;	/* clear to bottom edge also */
    EraseRect(&rect);
	if (RScurrent->selected)
	  /* highlight any part of the selection within the cleared area */
		RSinvText(w, *(Point *) &RScurrent->anchor, *(Point *) &RScurrent->last, &rect);
  } /* RSerase */

void RSinslines
  (
	short w, /* affected window */
	short t, /* where to insert blank lines */
	short b, /* bottom of area to scroll */
	short n, /* number of lines to insert */
	short scrolled /* -ve <=> cancel current selection, if any */
  )
  /* inserts blank lines at the top of the given area of the display,
	scrolling the rest of it down. */
  {
    Rect rect;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
	RSsetConst(w);
    RSsetattr(0); /* avoid funny pen modes */
	if (RScurrent->selected && (scrolled < 0))
	  {
	  /* unhighlight and cancel selection */
		RSinvText(w, *(Point *) &RScurrent->anchor,
			*(Point *) &RScurrent->last, &noConst);
		RScurrent->selected = 0;
	  } /* if */
	rect.left = -1;						/* BYU 2.4.12 - necessary */
    rect.right = RScurrent->width;
    rect.top = t * RScurrent->fheight;
    rect.bottom = (b + 1) * RScurrent->fheight;
  /* adjust the update region to track the scrolled window contents */
	OffsetRgn(((WindowPeek) RScurrent->window)->updateRgn,
		0, RScurrent->fheight * n);
    ScrollRect(&rect, 0, RScurrent->fheight * n, RSuRgn);
    InvalRgn(RSuRgn);
  /* newly-inserted area is already blank -- validate it to avoid redrawing. */
  /* any necessary redrawing will be done by caller */
	SetRect(&rect, 0, t * RScurrent->fheight - 1,
		RScurrent->width, (t + n) * RScurrent->fheight + 1);
    ValidRect(&rect);
  } /* RSinslines */

void RSinscols
  (
	short w,
	short n /* number of columns to insert */
  )
  /* inserts blank columns at the left side of the text display in
	the specified window, scrolling its current contents to the right.
	Maintains the selection highlight, but doesn't move the selection.
	Doesn't even unhighlight text which moves out of the selection area. */
  {
    Rect rect;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
    SetRect /* bounds of entire text area */
	  (
		&rect,
		0,
		0,
		RScurrent->width,
		RScurrent->height
	  );
    ScrollRect(&rect, n * RScurrent->fwidth, 0, RSuRgn);
    InvalRgn(RSuRgn);
    ValidRect(&rect); /* any necessary redrawing in newly-revealed area will be done by caller */
    SetRect /* bounds of newly-inserted blank area */
	  (
		&rect,
		0,
		0,
		(n + 1) * RScurrent->fwidth - 1,
		RScurrent->height
	  );
	if (RScurrent->selected)
	  /* highlight any part of the selection in the newly-blanked area */
		RSinvText(w, *(Point *) &RScurrent->anchor, *(Point *) &RScurrent->last, &rect);
  } /* RSinscols */

void RSinsstring
  (
	short w, /* affected window */
	short x, /* starting column at which to insert */
	short y, /* line on which to insert */
	short a, /* attributes for inserted text */
	short len, /* length of inserted text */
	char *ptr /* pointer to inserted text */
  )
  /* inserts a string of characters at the specified position, scrolling
	the rest of the line to the right. Highlights any part of the newly-
	inserted text lying within the current selection. */
  {
    Rect rect;

	if (RSlocal[w].skip)
		return;
    RSsetwind(w);
    SetRect /* bounds of part of line from specified position to end of line */
	  (
		&rect,
		x * RScurrent->fwidth,
		y * RScurrent->fheight,
		RScurrent->width,
		(y + 1) * RScurrent->fheight
	  );
    ScrollRect(&rect, len * RScurrent->fwidth, 0, RSuRgn); /* scroll remainder of line to the right */
    if (RSa != a)
		RSsetattr(a);
    InvalRgn(RSuRgn);
    ValidRect(&rect); /* any necessary redrawing in newly-revealed area will be done by caller */
    SetRect /* bounds area to contain inserted string */
	  (
		&rect,
		x * RScurrent->fwidth,
		y * RScurrent->fheight,
		(x + len) * RScurrent->fwidth,
		(y + 1) * RScurrent->fheight
	  );
    EraseRect(&rect); /* erase area to appropriate background */
    MoveTo
	  (
		x * RScurrent->fwidth,
		y * RScurrent->fheight + RScurrent->fascent
	  );
    DrawText(ptr, 0, len);
	if (RScurrent->selected)
	  /* highlight any part of selection covering the newly-inserted text */
		RSinvText(w, *(Point *) &RScurrent->anchor,
			*(Point *) &RScurrent->last, &rect);
  } /* RSinsstring */

void RSsendstring
  (
	short w, /* which terminal window */
	char *ptr, /* pointer to data */
	short len /* length of data */
  )
  /* sends some data to the host along the connection associated
	with the specified window. */
  {
#ifdef NCSA_TELNET
	short temp;

	temp = findbyVS(w);
	if (temp < 0)
		return;
/*	if (!screens[scrn].lmflag)	*/
		netwrite(screens[temp].port, ptr, len);
		netpush(screens[temp].port);				/* BYU 2.4.18 - for Diab systems? */
/*	else 
		{
		sprintf(screens[scrn].kbbuf,"%s",ptr);
		screens[scrn].kblen += len;
		if (screens[scrn].echo) parse(&screens[scrn],ptr,len);
		}*/
		
#else
	while (len--)
		putu(*ptr++);
#endif
  } /* RSsendstring */

void RSmargininfo
  (
	short w,
	short total, /* number of invisible character positions (screen width less visible width) */
	short current /* leftmost visible character position */
  )
  /* updates the horizontal scroll bar and associated variables
	to reflect the current view of the virtual screen within the
	specified window. */
  {
	RSlocal[w].leftmarg = current;			/* Adjust local vars */
	if (RSlocal[w].lcurrent != current)
		SetCtlValue(RSlocal[w].left, (RSlocal[w].lcurrent = current));
	if (RSlocal[w].lmax != total)
		SetCtlMax(RSlocal[w].left, (RSlocal[w].lmax = total));
  } /* RSmargininfo */

void RSbufinfo
  (
	short w, /* affected window */
	short total, /* number of lines of scrollback */
	short current, /* current topmost visible line */
	short bottom /* current bottommost visible line */
  )
  /* readjusts the vertical scroll bar and associated variables
	to reflect the current view of the virtual screen within the
	specified window. */
  {
	RSdata *RSthis;
	short newmax;

	RSthis = RSlocal + w;
	RSthis->topline = current;			/* Adjust local vars */
	if (RSthis->min != -total)
	  {
		SetCtlMin(RSthis->scroll, (RSthis->min = -total));
		SetCtlValue(RSthis->scroll, (RSthis->current = current));
	  } /* if */

	if (RSthis->current != current)
		SetCtlValue(RSthis->scroll, (RSthis->current = current));
	newmax = (VSgetlines(w) - 1) - (bottom - current);
	if (RSthis->max != newmax)
	  {
		SetCtlMax(RSthis->scroll, (RSthis->max = newmax));
		SetCtlValue(RSthis->scroll, (RSthis->current = current));
	  } /* if */

  } /* RSbufinfo */

GrafPtr RSgetwindow
  (
	short w
  )
  /* returns a pointer to the Mac window structure for the
	specified terminal window. */
  {
    return(RSlocal[w].window);
  } /* RSgetwindow */

void RSattach
  (
	short w,
	GrafPtr wind
  )
  /* attaches the specified window to a virtual screen, for
	use for displaying that screen. */
  {
    RSlocal[w].window = wind;
    RSlocal[w].selected = 0;
	RScurrent->cursorstate = 0;	/* BYU 2.4.11 - cursor off */
    SetPort(wind);
#if 1								/* BYU */
    RSTextFont(FONT,FSIZE,0);			/* BYU */
#else
    TextFont(FONT);
#endif
    TextSize(FSIZE);
	if (!RScolor)
		TextMode(srcXor);			/* Xor mode*/
	else
		TextMode(srcCopy);
	RSw = -1;	/* make sure we don't write in wrong place */
  } /* RSattach */

void RSdetach
  (
	short w
  )
  /* detaches the window for the specified virtual screen. */
  {
    RSlocal[w].window = 0L;
	RSw = -1;	/* make sure we don't write in wrong place */
  } /* RSdetach */

short RSfindvwind
  (
	GrafPtr wind
  )
  /* returns the number of the virtual screen associated with
	the specified window, or -4 if not found. */
  {
    short
		i = 0;
    while ((RSlocal[i].window != wind) && (i < MaxRS))
		i++;
    if ((RSlocal[i].window == 0L) || (i >= MaxRS))
		return(-4);
	else
		return(i);
  } /* RSfindvwind */

short RSfindscroll				/* Find screen index by control*/
  (
	ControlHandle control,
	short *n
  )
  /* finds the window to which the given scroll bar belongs.
	Returns the window number in *n if found, and a function
	result of 1 for a vertical scroll bar, 2 for a horizontal
	one, or -1 if the window wasn't found. */
  {
  /* look for a vertical scroll bar */
	*n = 0;
	while ((*n < MaxRS) && (control != RSlocal[*n].scroll))
		(*n)++;
	if (*n < MaxRS)
		return (1); /* found it */
  /* look for a horizontal scroll bar */
	*n = 0;
	while ((*n < MaxRS) && (control != RSlocal[*n].left))
		(*n)++;
	if (*n < MaxRS)
		return (2); /* found it */
	return(-1); /* not found */
  } /* RSfindscroll */

void RSregnconv
  (
	RgnHandle regn,
	short *x1, /* left (output) */
	short *y1, /* top (output) */
	short *x2, /* right (output) */
	short *y2, /* bottom (output) */
	short fh, /* font character height */
	short fw /* font character width */
  )
  /* converts the bounding box of the specified QuickDraw region
	into units of character positions (using the specified character
	height and width) and returns the results in *x1, *y1, *x2 and *y2. */
  {
    HLock((Handle) regn);
    *y1 = ((*regn)->rgnBBox.top) / fh;
    *y2 = (((*regn)->rgnBBox.bottom) + fh - 1) / fh;
    *x1 = ((*regn)->rgnBBox.left) / fw;
    *x2 = (((*regn)->rgnBBox.right) + fw - 1) / fw;
    HUnlock((Handle) regn);
	if (*x1 < 0)
		*x1 = 0;
	if (*y1 < 0)
		*y1 = 0;
	if (*x2 < 0)
		*x2 = 0;
	if (*y2 < 0)
		*y2 = 0;
#ifdef WHONEEDSIT
    if (*x1> VSIw->maxwidth)
		*x1 = VSIw->maxwidth;
    if (*x2 > VSIw->maxwidth)
		*x2 = VSIw->maxwidth;
    if (*y1 > VSIw->lines)
		*y1 = VSIw->lines;
    if (*y2 > VSIw->lines)
		*y2 = VSIw->lines;
#endif WHONEEDSIT
  } /* RSregnconv */

short RSupdate
  (
	GrafPtr wind
  )
  /* does updating for the specified window, if it's one of mine.
	Returns zero iff it is. */
  {
    short w, x1, x2, y1, y2;

    w = RSfindvwind(wind);
    if (RSsetwind(w) < 0)
		return(-1); /* not one of mine */
    BeginUpdate(wind);
	RSregnconv /* find bounds of text area needing updating */
	  (
		wind->visRgn,
		&x1, &y1, &x2, &y2,
		RScurrent->fheight, RScurrent->fwidth
	  );
	VSredraw(w, x1, y1, x2, y2); /* draw that text */
  /* We must reset, less we risk looking UGLY as sin... */
	BackPat(PATTERN(qd.white));
	PenPat(PATTERN(qd.black));
	if (RScolor)
	  {
		PmForeColor(0);
		PmBackColor(1);
	  }
	else
	  {
		ForeColor((long) RScolors[ RScurrent->color[0]]);		/* normal foreground */
		BackColor((long) RScolors[ RScurrent->color[1]]);		/* normal Background */
	  } /* if */
    RSa = -1;
	PenMode(patOr);
	DrawGrowIcon(wind);
	PenMode(patCopy);
	DrawControls(wind);
    EndUpdate(wind);
	return(0);
  } /* RSupdate */

#ifdef FASTERCOMMENTED

RSupdprint(WindowPeek wind)
  /* debugging routine: displays bounds (in character units) of
	update region of specified window. */
  {
	short x, y, z, w;
	char tbuf[50];

	RSregnconv(wind->updateRgn, &x, &y, &z, &w);
//	sprintf(tbuf, "RSregnconv: %d,%d,%d,%d", x, y, z, w);
//	putln(tbuf);
  } /* RSupdprint */

#endif

Point getlocalmouse(GrafPtr wind)
  /* returns the current mouse position in coordinates local
	to the specified window. Leaves the current grafPort set
	to that window. */
  {
	Point temp;

	SetPort(wind);
	GetMouse(&temp);
	return(temp);
  } /* getlocalmouse */

#define	Fwidthhalf	FWidth/2

Point normalize(Point in, short w)
  /* converts in from a pixel position in local coordinates to
	a character cell position within the virtual screen corresponding
	to the specified window. Constrains the position to lie within
	the currently-visible region of the screen, autoscrolling the
	screen if necessary. */
  {

	if (in.v <0)
	  {
		in.v = 0;
		VSscrolback(w, 1);
	  } /* if */
	if (in.v > RSlocal[w].height)
	  {
		in.v = RSlocal[w].height;
		VSscrolforward(w, 1);
	  } /* if */
	in.v = in.v / FHeight;

	if (in.h < 0)
	  {
		in.h = -1;
		VSscrolleft(w, 1);
	  } /* if */
	if (in.h > RSlocal[w].width)
	  {
		in.h = RSlocal[w].width;
		VSscrolright(w, 1);
	  } /* if */
  /* in.h = (in.h + Fwidthhalf) / FWidth - 1; */
  /* the MPW C 3.0 compiler has a bug in its register allocation */
  /* which keeps the above line from working. So, replace it with this: */
	in.h = in.h + Fwidthhalf;
	in.h = in.h / FWidth - 1;
  /* note the bug has been fixed in the 3.1 compiler. */
  /* convert to virtual screen coordinates */
	in.v += RSlocal[w].topline;
	in.h += RSlocal[w].leftmarg;
	return(in);
  } /* normalize */









/*--------------------------------------------------------------------------*/
/* HandleDoubleClick														*/
/* This is the routine that does the real dirty work.  Since it is not a	*/
/* true TextEdit window, we have to kinda "fake" the double clicking.  By	*/
/* this time, we already know that a double click has taken place, so check	*/
/* the chars to the left and right of our location, and select all chars 	*/
/* that are appropriate	-- SMB												*/
/*--------------------------------------------------------------------------*/
void HandleDoubleClick(short w)													
{																				
	Point	leftLoc, rightLoc, curr, oldcurr;													
	long	mySize;															
	char	theChar[5];															
	short	mode = -1, newmode, foundEnd=0;															
																				
	RSsetConst(w);																/* get window dims */							
	leftLoc = RSlocal[w].anchor;									/* these two should be the same */							
	rightLoc = RSlocal[w].last;									
																				
	while(!foundEnd)															/* scan to the right first */														
		{																		
		mySize = VSgettext(w,rightLoc.h, rightLoc.v, rightLoc.h+1, rightLoc.v,	
			(char *)theChar,(long)1,"\015",0);									
		if(mySize ==0 || isspace(*theChar))									/* stop if not a letter */			
			foundEnd =1;														
		else rightLoc.h++;														
		}																		
																				
	foundEnd =0;																
	while(!foundEnd)															/* ...and then scan to the left */															
		{																		
		mySize = VSgettext(w,leftLoc.h-1, leftLoc.v, leftLoc.h, leftLoc.v,		
			(char *)theChar,(long)1,"\015",0);									
		if(mySize ==0 || isspace(*theChar))										/* STOP! */		
			foundEnd =1;														
		else leftLoc.h--;														
		}																		
																				
	if (leftLoc.h != rightLoc.h)												/* we selected something */							
		{																		
		RSlocal[w].anchor = leftLoc;									/* new left bound */
		RSlocal[w].last = rightLoc;									/* and a matching new right bound */
		RSlocal[w].selected =1;													/* give me credit for the selection I just made */
		RSinvText(w, RSlocal[w].anchor,								/* time to show it off */
			RSlocal[w].last, &noConst);

		curr.h = 0; curr.v = 0;

		while (StillDown()) {
		  /* wait for mouse position to change */
			do {
				oldcurr = curr;
				curr = normalize(getlocalmouse(RSlocal[w].window), w);
				} while (EqualPt(curr, oldcurr) && StillDown());
	
			
			if ((curr.v < leftLoc.v) || ((curr.v == leftLoc.v) && (curr.h < leftLoc.h))) {
				newmode = 1;	// up
				}
			else if ((curr.v > leftLoc.v) || ((curr.v == leftLoc.v) && (curr.h > rightLoc.h))) {
				newmode = 2;	// down
				}
			else 
				newmode = -1;	// inside dbl-clicked word
				
			/* toggle highlight state of text between current and last mouse positions */
			if (mode == -1) {
				if (newmode == 2) {
					RSlocal[w].anchor = leftLoc;
					RSinvText(w, curr, rightLoc, &noConst);
					RSlocal[w].last = curr;
					}
				if (newmode == 1) {
					RSlocal[w].anchor = rightLoc;
					RSinvText(w, curr, leftLoc, &noConst);
					RSlocal[w].last = curr;
					}
				}

			if (mode == 1) {
				if (newmode == 2) {
					RSlocal[w].anchor = leftLoc;
					RSinvText(w, oldcurr, leftLoc, &noConst);
					RSinvText(w, rightLoc, curr, &noConst);
					RSlocal[w].last = curr;
					}
				if (newmode == -1) {
					RSlocal[w].anchor = leftLoc;
					RSinvText(w, oldcurr, leftLoc, &noConst);
					RSlocal[w].last = rightLoc;
					}
				if (newmode == mode) {
					RSinvText(w, oldcurr, curr, &noConst);
					RSlocal[w].last = curr;
					}
				}
			
			if (mode == 2) {
				if (newmode == 1) {
					RSlocal[w].anchor = rightLoc;
					RSinvText(w, oldcurr, rightLoc, &noConst);
					RSinvText(w, leftLoc, curr, &noConst);
					RSlocal[w].last = curr;
					}
				if (newmode == -1) {
					RSlocal[w].anchor = leftLoc;
					RSinvText(w, oldcurr, rightLoc, &noConst);
					RSlocal[w].last = rightLoc;
					}
				if (newmode == mode) {
					RSinvText(w, oldcurr, curr, &noConst);
					RSlocal[w].last = curr;
					}
				}
				
			mode = newmode;
			} /* while */
		}	
}
	

void	RSsortAnchors(short w)
{
	Point	temp;
	
	if (RSlocal[w].anchor.v > RSlocal[w].last.v) {
		temp = RSlocal[w].anchor;
		RSlocal[w].anchor = RSlocal[w].last;
		RSlocal[w].last = temp;
		}
		
	if ((RSlocal[w].anchor.v == RSlocal[w].last.v) && (RSlocal[w].anchor.h > RSlocal[w].last.h)) {
		temp = RSlocal[w].anchor;
		RSlocal[w].anchor = RSlocal[w].last;
		RSlocal[w].last = temp;
		}
}	

/*------------------------------------------------------------------------------*/
/* RSselect																		*/
/* Handle the mouse down in the session window.  All we know so far is that it	*/
/* is somewhere in the content window, and it is NOT an option - click.			*/
/* Double clicking now works -- SMB												*/
// And I fixed it so it works correctly.  Grrrr... - JMB
//	WARNING: Make sure RSlocal[w].selected is 1 when doing selections.  If it is
//		zero, the autoscrolling routines will seriously hose the selection drawing.
//		Heed this advice, it took me two hours to find the cause of this bug! - JMB

  /* called on a mouse-down in the text display area of the
	active window. Creates or extends the highlighted selection
	within that window, autoscrolling as appropriate if the user
	drags outside the currently visible part of the display. */
void RSselect( short w, Point pt, EventRecord theEvent)
{
	static	long 	lastClick = 0;
	static 	Point 	lastClickLoc = {0,0};
	GrafPtr tempwndo;
	Point	curr, temp;
	long	clickTime;
	short	shift = (theEvent.modifiers & shiftKey);
	
	RSsetConst(w);
	tempwndo = RSlocal[w].window;
	
	curr = normalize(pt, w);
	clickTime = TickCount();
	
	if  ( ( EqualPt(RSlocal[w].anchor, curr) || EqualPt(RSlocal[w].anchor, RSlocal[w].last) )
			&&  ((clickTime - lastClick) <= GetDblTime())
			&& EqualPt(curr, lastClickLoc)) {
		/* NCSA: SB - check to see if this is a special click */
		/* NCSA: SB - It has to be in the right time interval, and in the same spot */
		curr = RSlocal[w].anchor = RSlocal[w].last = normalize(pt, w);
		HandleDoubleClick(w);
		RSlocal[w].selected = 1;
		lastClick = clickTime;
		lastClickLoc = curr;
		}
	else {
		lastClick = clickTime;
		lastClickLoc = curr;
		if (RSlocal[w].selected) {
			if (!shift) {
			  /* unhighlight current selection */
				RSinvText(w, RSlocal[ w].anchor, RSlocal[w].last, &noConst);
			  /* start new selection */
				curr = RSlocal[w].last = RSlocal[w].anchor = normalize(pt, w);
			}
			else {
				RSsortAnchors(w);
				if ((curr.v < RSlocal[w].anchor.v) || ((curr.v == RSlocal[w].anchor.v) && (curr.h < RSlocal[w].anchor.h))) {
					temp = RSlocal[w].anchor;
					RSlocal[w].anchor = RSlocal[w].last;
					RSlocal[w].last = temp;
					}
				}
		  }
		else
		  {
		  /* start new selection */
			curr = RSlocal[w].anchor = RSlocal[w].last = normalize(pt, w);
			RSlocal[w].selected = 1;
			}
			
		while (StillDown())
		  {
		  /* wait for mouse position to change */
			do {
				curr = normalize(getlocalmouse(tempwndo), w);
				} while (EqualPt(curr, RSlocal[w].last) && StillDown());
	
		  /* toggle highlight state of text between current and last mouse positions */
			RSinvText(w, curr, RSlocal[w].last, &noConst);
			RSlocal[w].last = curr;
		  } /* while */
		}

	
	if (EqualPt(RSlocal[w].anchor, RSlocal[w].last)) RSlocal[w].selected = 0;
		else RSlocal[w].selected = 1;
	SetMenusForSelection((short)RSlocal[w].selected);
  } /* RSselect */

void RSactivate
  (
	short w
  )
  /* handles an activate event for the specified window. */
  {
	RSsetConst(w);
  /* display the grow icon */
	DrawGrowIcon(RSlocal[w].window);
  /* and activate the scroll bars */
	if (RSlocal[w].scroll != 0L)
		HiliteControl(RSlocal[w].scroll, 0);
	if (RSlocal[w].left != 0L)
		HiliteControl(RSlocal[w].left, 0);
  } /* RSactivate */

void RSdeactivate
  (
	short w
  )
  /* handles a deactivate event for the specified window. */
  {
	RSsetConst(w);
  /* update the appearance of the grow icon */
	DrawGrowIcon(RSlocal[w].window); 
  /* and deactivate the scroll bars */
	if (RSlocal[w].scroll != 0L)
		HiliteControl(RSlocal[w].scroll, 255);
	if (RSlocal[w].left != 0L)
		HiliteControl(RSlocal[w].left, 255);
  } /* RSdeactivate */

char **RSGetTextSel
  (
	short w, /* window to look at */
	short table /* nonzero for "table" mode, i e
		replace this many (or more) spaces with a single tab. */
  )
  /* returns the contents of the current selection as a handle,
	or nil if there is no selection. */
  {
	char **charh, *charp;
	short maxwid;
	long realsiz;
	Point Anchor,Last;

	if (!RSlocal[w].selected)
		return(0L);	/* No Selection */
	maxwid = VSmaxwidth(w);
	Anchor = RSlocal[w].anchor;
	Last = RSlocal[w].last;
	
	realsiz = Anchor.v - Last.v;
	if (realsiz < 0)
		realsiz = - realsiz;
	realsiz ++;								/* lines 2,3 selected can be 2 lines */
	realsiz *= (maxwid + 2);
//	sprintf(tempc, "Size of block=%d", realsiz);
//	putln(tempc);
	charh = NewHandle(realsiz);
	if (charh == 0L)
		return((char **) -1L);				/* Boo Boo return */
	HLock(charh);
	charp = *charh;
	realsiz = VSgettext(w, Anchor.h, Anchor.v, Last.h, Last.v,
		charp, realsiz, "\015", table);
//	putln("unlocking");
	HUnlock(charh);
	SetHandleSize(charh, realsiz);
//	sprintf(tempc, "Size of clip=%d", realsiz);
//	putln(tempc);
	return(charh);
  }  /* RSGetTextSel */

void	RSsetsize( short w, short v, short h)
/*	saves the new size settings for a window, and repositions
	the scroll bars accordingly. */
{
	RSlocal[w].height = ((v - 16 + CVO) / FHeight) * FHeight;
	RSlocal[w].width = ((h - 16 + CHO) / FWidth) * FWidth;
	RSlocal[w].rheight = v - 16;
	RSlocal[w].rwidth = h - 16;

/*
*  Get rid of the scroll bars which were in the old size.
*  Hiding them causes the region to be updated later.
*/
	if (RSlocal[w].scroll != NULL )
		HideControl(RSlocal[w].scroll);
	if (RSlocal[w].left != NULL ) 
		HideControl(RSlocal[w].left);

	DrawGrowIcon(RSlocal[w].window);			/* Draw in the necessary bugger */

/*	move the scroll bars to their new positions and sizes, and redisplay them */	

	if (RSlocal[w].scroll != NULL ) {
		SizeControl(RSlocal[w].scroll, 16, (v - 13));
		MoveControl(RSlocal[w].scroll, (h - 15) + CHO, -1 + CVO);
		ShowControl(RSlocal[w].scroll);
		}
	if (RSlocal[w].left != NULL ) {
		SizeControl(RSlocal[w].left, (h - 13), 16);
		MoveControl(RSlocal[w].left, -1 + CHO,  (v - 15) + CVO);
		ShowControl(RSlocal[w].left);
		}
		
	SetRect(&RSlocal[w].textrect, 0, 0, RSlocal[w].rwidth, RSlocal[w].rheight);
	
} /* RSsetsize */

void RSdrawsep
  (
	short w,
	short y1,
	short draw /* 1 to draw the line, 0 to erase the existing line */
  )
  /* draws a dotted line separating the screen contents from the
	scrollback area. This is currently conditionally compiled to
	be a no-op. */
  {
#ifdef SEPLINE
    RSsetwind(w);
	PenPat(gray);
	if (draw)
		PenMode(srcCopy);
	else
		PenMode(srcXor);
	MoveTo(0, y1 * RScurrent->fheight);
	Line(RSlocal[w].width-1, 0);
	PenMode(srcOr);
	RSsetattr(0);
#else
#pragma unused(w, y1, draw)
#endif SEPLINE
  } /* RSdrawsep */

void RSfontmetrics
  (
	void
  )
  /* calculates various metrics for drawing text with selected font
	and size in current grafport into *RScurrent. */
  {
	FontInfo finforec;
	GrafPtr myGP;
 
	GetPort(&myGP); 
#ifdef GOTTHEDOUGH
	TextFace(bold);			/* NCSA: SB */
#endif
	GetFontInfo(&finforec);
	RScurrent->fascent = finforec.ascent;
	RScurrent->fheight = finforec.ascent + finforec.descent + finforec.leading /* +1 */;
	RScurrent->monospaced = (CharWidth('W') == CharWidth('i'));   
/*	RScurrent->monospaced = (CharWidth('W') == CharWidth('i') == CharWidth(' '));  */

/*	if (RScurrent->monospaced)	*/						/* NCSA: SB */
		RScurrent->fwidth = CharWidth('W'); 
/*	else 	
		RScurrent->fwidth = finforec.widMax; */			/* NCSA: SB */
#ifdef GOTTHEDOUGH
	TextFace(myGP->txFace - bold);		/* NCSA: SB */
#endif
}

void RSchangefont
  (
	short w,
	short fnum, /* new font ID or -1 for no change */
	long fsiz, /* new font size or 0 for no change */
	short resizwind /* should I resize window to keep same number of visible lines & cols */
  )
  {
	Rect pRect;
	short x1, x2, y1, y2, width, lines;
	short srw,srh;
	WStateData *wstate;
	WindowPeek wpeek;
	short resizeWidth, resizeHeight;		/* NCSA: SB */

    RSsetwind(w);
	srw = RScurrent->rwidth;
	srh = RScurrent->rheight;

	if (fnum != -1)
	  {
#if 1								/* BYU */
		RSTextFont(fnum,fsiz,0);	/* BYU */
#else
		TextFont(fnum);
#endif
		RScurrent->fnum = fnum;
	  } /* if */
	if (fsiz)
	  {
		TextSize(fsiz);
		RScurrent->fsiz = fsiz;
	  } /* if */
	RSfontmetrics();

	width = VSmaxwidth(w) + 1;
	lines = VSgetlines(w);
	if (!resizwind)
	  {
	  /* round window size to new character cell dimensions */
		RScurrent->height =
			((RScurrent->rheight) / RScurrent->fheight) * RScurrent->fheight;
		RScurrent->width =
			((RScurrent->rwidth + CHO) / RScurrent->fwidth ) * RScurrent->fwidth
				- CHO;
	  }
	else
	  {
	  /* resize window to preserve its dimensions in character cell units */
		VSgetrgn(w, &x1, &y1, &x2, &y2);
		RScurrent->rwidth =
			RScurrent->width = (x2 - x1 + 1) * RScurrent->fwidth - CHO;
		RScurrent->rheight =
			RScurrent->height= (y2 - y1 + 1) * RScurrent->fheight;
	  } /* if */

	if (RScurrent->rwidth > RMAXWINDOWWIDTH - 16 - CHO)
	 	 RScurrent->rwidth = RMAXWINDOWWIDTH - 16 - CHO;
	if (RScurrent->rheight > RMAXWINDOWHEIGHT - 16)
	 	 RScurrent->rheight = RMAXWINDOWHEIGHT - 16;
	
	RScheckmaxwind(&RScurrent->window->portRect,RScurrent->rwidth +16,	/* NCSA: SB */
		RScurrent->rheight + 16, &resizeWidth, &resizeHeight);			/* NCSA: SB */


	SizeWindow
	  (
		RScurrent->window,
		RScurrent->rwidth + 16, RScurrent->rheight+16,
		FALSE
	  ); /*  TRUE if done right */
	RSsetsize(w, RScurrent->rheight + 16, RScurrent->rwidth + 16);

	wpeek = (WindowPeek) RScurrent->window;

	HLock(wpeek->dataHandle);
	wstate = (WStateData *) *wpeek->dataHandle;

	BlockMove(&pRect, &wstate->stdState, 8);
	pRect.right = pRect.left + RMAXWINDOWWIDTH;
	if (pRect.right > TelInfo->screenRect.right)
		pRect.right = TelInfo->screenRect.right;
	pRect.bottom = pRect.top + RMAXWINDOWHEIGHT;
	BlockMove(&wstate->stdState, &pRect, 8);

	VSgetrgn(w, &x1, &y1, &x2, &y2);
	VSsetrgn(w, x1, y1,
		(short) (x1 + (RScurrent->rwidth ) / RScurrent->fwidth - 1),
		(short) (y1 + (RScurrent->rheight) / RScurrent->fheight - 1));
	VSgetrgn(w, &x1, &y1, &x2, &y2);		/* Get new region */
	
	DrawGrowIcon(RScurrent->window);
	VSredraw(w, 0, 0, x2 - x1 + 1, y2 - y1 + 1); /* redraw newly-revealed area, if any */
	ValidRect(&RScurrent->window->portRect); /* no need to do it again */
	DrawControls(RScurrent->window);
  } /* RSchangefont */

short RSgetfont
  (
	short w, /* which window */
	short *pfnum, /* where to return font ID */
	short *pfsiz /* where to return font size */
  )
  /* returns the current font ID and size setting for the specified window. */
  {
/*
*  Bug fix.  RSsetwind's error conditions are all < 0.  There is a success condition == 1.
*  TK 12/17/88
*/
    if (0 > RSsetwind(w))
		return -1;
	*pfnum = RScurrent->fnum;
	*pfsiz = RScurrent->fsiz;
	return(0);
  } /* RSgetfont */

short RSnewwindow
  (
	RectPtr wDims, /* window bounds in pixels */
	short scrollback, /* number of lines to save off top */
	short width, /* number of characters per text line (80 or 132) */
	short lines, /* number of text lines */
	StringPtr name, /* window name */
	short wrapon, /* autowrap on by default */
	short fnum, /* ID of font to use initially */
	short fsiz, /* size of font to use initially */
	short showit, /* window initially visible or not */
	short goaway, /* NCSA 2.5 */
	short forcesave		/* NCSA 2.5: force screen save */
  )
  /* creates a virtual screen and a window to display it in. */
  {
	GrafPort gp; /* temp port for getting text parameters */
	short w;

	Rect		pRect;
	short		wheight, wwidth;
	WStateData	*wstate;
	WindowPeek	wpeek;
	CTabHandle	RSctab;

  /* create the virtual screen */
	w = VSnewscreen(scrollback, (scrollback != 0), /* NCSA 2.5 */
		width,1, forcesave);	/* NCSA 2.5 */
	if (w < 0) {		/* problems opening the virtual screen -- tell us about it */
		return(-1);
	  	}
	  
	RScurrent = RSlocal + w;

	RScurrent->fnum = fnum;
	RScurrent->fsiz = fsiz;

	OpenPort(&gp);
#if 1							/* BYU */
	RSTextFont(fnum,fsiz,0);	/* BYU */
#else
	TextFont(fnum);
#endif
	TextSize(fsiz);
	RSfontmetrics();
	ClosePort(&gp);

	if ((wDims->right - wDims->left) > RMAXWINDOWWIDTH)
		wDims->right = wDims->left + RMAXWINDOWWIDTH;
	if ((wDims->bottom - wDims->top) > RMAXWINDOWHEIGHT)
		wDims->bottom = wDims->top + RMAXWINDOWHEIGHT;
	wwidth = wDims->right - wDims->left;
	wheight = wDims->bottom - wDims->top;

  /* create the window */
	if (!RScolor) {
		RScurrent->window = NewWindow(0L, wDims, name, showit?TRUE:FALSE, 8,	/* BYU LSC */
			kInFront, goaway ? TRUE:FALSE, (long)w);
		RScurrent->pal = NULL;
		if (RScurrent->window == NULL) {
			VSdetach(w);
			return(-2);
			}
		}
	else
	  {
		RScurrent->window = NewCWindow(0L, wDims, name, showit?TRUE:FALSE, (short)8,	/* BYU LSC */
			kInFront, goaway ? TRUE:FALSE, (long)w);
					/* the screen is not there until we can see it..... */
		if (RScurrent->window == NULL) {
			VSdetach(w);
			return(-2);
			}

		RSctab = (CTabHandle) NewHandleClear((long) (sizeof(ColorTable) + 4 * sizeof(CSpecArray)));
		if (RSctab == NULL) {
			DisposeWindow(RScurrent->window);
			VSdetach(w);
			return(-2);
			}
		HLock((Handle) RSctab);

		(*RSctab)->ctSize = 3;		// Number of entries minus 1
		(*RSctab)->ctFlags = 0;
		RScurrent->RGBs[0].red  =(*RSctab)->ctTable[0].rgb.red		=     0;
		RScurrent->RGBs[0].green=(*RSctab)->ctTable[0].rgb.green	=     0;
		RScurrent->RGBs[0].blue =(*RSctab)->ctTable[0].rgb.blue		=     0;
		RScurrent->RGBs[1].red  =(*RSctab)->ctTable[1].rgb.red		= 65535;
		RScurrent->RGBs[1].green=(*RSctab)->ctTable[1].rgb.green	= 65535;
		RScurrent->RGBs[1].blue =(*RSctab)->ctTable[1].rgb.blue		= 65535;
		RScurrent->RGBs[2].red  =(*RSctab)->ctTable[2].rgb.red		=     0;
		RScurrent->RGBs[2].green=(*RSctab)->ctTable[2].rgb.green	= 61183;
		RScurrent->RGBs[2].blue =(*RSctab)->ctTable[2].rgb.blue		= 11060;
		RScurrent->RGBs[3].red  =(*RSctab)->ctTable[3].rgb.red		= 61183;
		RScurrent->RGBs[3].green=(*RSctab)->ctTable[3].rgb.green	=  2079;
		RScurrent->RGBs[3].blue =(*RSctab)->ctTable[3].rgb.blue		=  4938;
		(*RSctab)->ctTable[0].value = 0;
		(*RSctab)->ctTable[1].value = 0;
		(*RSctab)->ctTable[2].value = 0;
		(*RSctab)->ctTable[3].value = 0;
		HUnlock((Handle) RSctab);

		RScurrent->pal = NewPalette(4, RSctab, pmCourteous, 0);
		DisposeHandle((Handle) RSctab);
		if (RScurrent->pal == NULL) {
			DisposeWindow(RScurrent->window);
			VSdetach(w);
			return(-2);
			}

		SetPalette(RScurrent->window, RScurrent->pal, TRUE);
	  } /* if */

	SetPort(RScurrent->window);
	SetOrigin(CHO, CVO);			/* Cheap way to correct left margin problem */

	wpeek = (WindowPeek) RScurrent->window;

	HLock(wpeek->dataHandle);
	wstate = (WStateData *) *wpeek->dataHandle;

	BlockMove(&wstate->userState, wDims, 8);
	pRect.top = wDims->top;
	pRect.left = wDims->left;
	pRect.right = pRect.left + RMAXWINDOWWIDTH;
	if (pRect.right > TelInfo->screenRect.right)
		pRect.right = TelInfo->screenRect.right;

	pRect.bottom = pRect.top + RMAXWINDOWHEIGHT;
	BlockMove(&wstate->stdState, &pRect, 8);

  /* create scroll bars for window */
	pRect.top = -1 + CVO;
	pRect.bottom = wheight - 14 + CVO;
	pRect.left = wwidth - 15 + CHO;
	pRect.right = wwidth + CHO;
	RScurrent->scroll = NewControl(RScurrent->window, &pRect, "\p", FALSE,	/* BYU LSC */
		0, 0, 0, 16, 1L);

	if (RScurrent->scroll == 0L) return(-3);

	pRect.top = wheight - 15 + CVO;
	pRect.bottom = wheight + CVO;
	pRect.left = -1 + CHO;
	pRect.right = wwidth - 14 + CHO;
	RScurrent->left = NewControl(RScurrent->window, &pRect, "\p", FALSE,		/* BYU LSC */
		0, 0, 0, 16, 1L);

	if (RScurrent->left == 0L) return(-3);

	RScurrent->skip = 0; /* not skipping output initially */
	RScurrent->max = 0; /* scroll bar settings will be properly initialized by subsequent call to VSsetrgn */
	RScurrent->min = 0;
	RScurrent->current = 0;
	RScurrent->lmax = 0;
	RScurrent->lmin = 0;
	RScurrent->lcurrent = 0;
	RScurrent->selected = 0;	/* no selection initially */
	RScurrent->cursorstate = 0;	/* BYU 2.4.11 - cursor off initially */
	RScurrent->flipped = 0;		/* Initially, the color entries are not flipped */

	RSsetsize(w, wheight, wwidth);
	VSsetlines(w, lines);
	VSsetrgn(w, 0, 0, ((wwidth - 16 + CHO) / FWidth -1),
		((wheight - 16 + CVO) / FHeight - 1));

#if 1												/* BYU */
	RSTextFont(RScurrent->fnum,RScurrent->fsiz,0);	/* BYU LSC */
#else
	TextFont(RScurrent->fnum);
#endif
	TextSize(RScurrent->fsiz);				/* 9 point*/
	if (!RScolor)
		TextMode(srcXor);			/* Xor mode*/
	else
		TextMode(srcCopy);

	if (wrapon)
	  /* turn on autowrap */
		VSwrite(w, "\033[?7h",5);

	return(w);
  } /* RSnewwindow */

void RSkillwindow
  (
	short w
  )
  /* closes a terminal window. */
  {
 	RSdata *temp = RSlocal + w;
 
 	if (temp->pal) {
 		DisposePalette(temp->pal);		
 		temp->pal = NULL;
		}
 
	VSdetach(w);		/* Detach the virtual screen */
	KillControls(RSlocal[w].window);  /* Get rid of those little slidy things */
	DisposeWindow(RSlocal[w].window);	/* Get rid of the actual window */
	RSdetach(w);		/* Detach from the table */
  }

void RSzoom
  (
	GrafPtr window, /* window to zoom */
	short code, /* inZoomIn or inZoomOut */
	short shifted /* bring to front or not */
  )
  /* called after a click in the zoom box, to zoom a terminal window. */
  {
	WStateData	**WSDhdl;
	short		w;
	short		h, v, x1, x2, y1, y2;
	short		width, lines;			// For setting Standard State before zooming
	short		top, left;				// Ditto
	
	SetPort(window);
	w = RSfindvwind(window); /* which window is it, anyway */

	width = VSmaxwidth(w) + 1;
	lines = VSgetlines(w);
	WSDhdl = (WStateData **)((WindowPeek)window)->dataHandle;
	top = (**WSDhdl).userState.top;
	left = (**WSDhdl).userState.left;
	HLock((Handle)WSDhdl);
	SetRect(&((*WSDhdl)->stdState), left, top, RMAXWINDOWWIDTH + left,
				RMAXWINDOWHEIGHT + top);
	HUnlock((Handle)WSDhdl);
	
	/* EraseRect(&window->portRect); */
	ZoomWindow(window, code, shifted);
    EraseRect(&window->portRect);			/* BYU 2.4.15 */

  /* get new window size */
	h = window->portRect.right - window->portRect.left;
	v = window->portRect.bottom - window->portRect.top;

	RSsetsize(w, v, h); /* save new size settings and update scroll bars */
  /* update the visible region of the virtual screen */
	VSgetrgn(w, &x1, &y1, &x2, &y2);
	VSsetrgn(w, x1, y1, (x1 + (h - 16 + CHO) / FWidth -1),
		(y1 + (v - 16 + CVO) / FHeight - 1));
	VSgetrgn(w, &x1, &y1, &x2, &y2);		/* Get new region */
  /* refresh the part which has been revealed, if any */
	VSredraw(w, 0, 0, x2 - x1 + 1, y2 - y1 + 1); 
  /* window contents are now completely valid */
	ValidRect(&window->portRect);
  } /* RSzoom */

/*
*  This routine is called when the user presses the grow icon, or when the size of
*  the window needs to be adjusted (where==NULL, modifiers==0).
*  It limits the size of the window to a legal range.
*/

void RSsize (GrafPtr window, long *where, long modifiers)
{
	Rect	SizRect;
	long	size;
	short	w, width, lines;
	short	tw, h, v, x1, x2, y1, y2, th;
	Boolean	changeVSSize = false;
	short	screenIndex = 0;
	Boolean	screenIndexValid = false;

	if ((w = RSfindvwind(window)) < 0)
		return;
	
	if (modifiers & cmdKey) return;
	
	screenIndexValid = (screenIndex = findbyVS(w)) != -1;

	changeVSSize = (modifiers & optionKey) == optionKey;

#define DONT_DEFAULT_CHANGE_VS_IF_NAWS				// JMB
	// 931112, ragge, NADA, KTH 
	// I think this is the way it should work, if there is naws available it
	// should be used by default, and option toggles behaviour.
	// Maybe it should be user configurable?
#ifndef DONT_DEFAULT_CHANGE_VS_IF_NAWS
	if(screenIndexValid && screens[screenIndex].naws) {
		changeVSSize = (modifiers & optionKey) != optionKey;
	}
#endif

	SetPort(window);

	width = VSmaxwidth(w) + 1;
	lines = VSgetlines(w);


	if (changeVSSize) {
		th = INFINITY;
		tw = INFINITY-1;
		}
	else {
		tw = RMAXWINDOWWIDTH;
		th = RMAXWINDOWHEIGHT + 1;
		}

	SetRect(&SizRect, 48, 48, tw + 1, th);
	
	if (where)											/* grow icon actions */
		{							
		if (changeVSSize) { /* 931112, ragge, NADA, KTH */
			setupForGrow(window, 1 - CHO, 1 - CVO, FWidth, FHeight);
		}
		size = GrowWindow(window, *(Point *) where, &SizRect);	/* BYU LSC */
		if (changeVSSize) { /* 931112, ragge, NADA, KTH */
			cleanupForGrow(window);
		}

		if (size != 0L)
		  {
			SizeWindow(window, size & 0xffff, (size >> 16) & 0xffff, FALSE);
			h = window->portRect.right - window->portRect.left;
			v = window->portRect.bottom - window->portRect.top;
		  }
		else return;							/* user skipped growing */
	  }
	else
	  {									/* just resize the window */
		h = window->portRect.right - window->portRect.left;	/* same width */
		v = (FHeight) * (VSgetlines(w));					/* new height */
		SizeWindow(window, h, v, FALSE);					/* change it */
		} 	

	RSsetsize(w, v, h); /* save new size settings and update scroll bars */

	
  /* update the visible region of the virtual screen */

	VSgetrgn(w, &x1, &y1, &x2, &y2);
	VSsetrgn(w, x1, y1, (short)((x1 + (h - 16 + CHO) / FWidth - 1)),
		(short)((y1 + (v - 16) / FHeight - 1)));
	VSgetrgn(w, &x1, &y1, &x2, &y2);		/* Get new region */

	if (changeVSSize) {
		
		VSsetlines(w,y2 -y1 +1);
		RScalcwsize(w,x2 - x1 +1);
		if (screenIndexValid && screens[screenIndex].naws)
			SendNAWSinfo(&screens[screenIndex], (x2-x1+1), (y2-y1+1));
		return;
		}

	VSredraw(w, 0, 0, x2 - x1 + 1, y2 - y1 + 1);		/* refresh the part which has been revealed, if any */
	ValidRect(&window->portRect);						/* window contents are now completely valid */
  } /* RSsize */



SIMPLE_UPP(ScrollProc,ControlAction);
pascal void ScrollProc(ControlHandle control, short part)
  /* scroll-tracking routine which does continuous scrolling of visible
	 region. */
  {
	short w, kind, x1, y2, x2, y1;

	kind = RSfindscroll(control, &w);
	VSgetrgn(w, &x1, &y1, &x2, &y2);

	if (kind == 2)
	  { /* horizontal scroll bar */
		switch (part)
		  {
			case inUpButton:							/* Up is left */
				VSscrolleft(w, 1);
				break;
			case inDownButton:							/* Down is right */
				VSscrolright(w, 1);
				break;
			case inPageUp:
				VSscrolleft(w, x2 - x1); /* scroll a whole windowful */
				break;
			case inPageDown:
				VSscrolright(w, x2 - x1); /* scroll a whole windowful */
				break;
			default:
				break;
		  } /* switch */
	  }
	else if (kind == 1)
	  { /* vertical scroll bar */
		switch (part)
		  {
			case inUpButton:
				VSscrolback(w, 1);
				break;
			case inDownButton:
				VSscrolforward(w, 1);
				break;
			case inPageUp:
				VSscrolback(w, y2 - y1); /* scroll a whole windowful */
				break;
			case inPageDown:
				VSscrolforward(w, y2 - y1); /* scroll a whole windowful */
				break;
			default:
				break;
		  } /* switch */
	  } /* if */
  } /* ScrollProc */

/* handles a click in a terminal window. */
short RSclick( GrafPtr window, EventRecord theEvent)
{
	ControlHandle ctrlh;
	short w, part, part2, x1, x2, y1, y2;
	Point	where = theEvent.where;
	short	shifted = (theEvent.modifiers & shiftKey);
	short	optioned = (theEvent.modifiers & optionKey);

	w = RSfindvwind(window);
	if (w < 0)
		return -1; /* what the heck is going on here?? */
	SetPort(window);
	GlobalToLocal((Point *) &where);
	part = FindControl(where, window, &ctrlh);		/* BYU LSC */
	if (part != 0)
		switch (part)
		  {
			case inThumb:
				part2 = TrackControl(ctrlh, where, 0L);		/* BYU LSC */
				if (part2 == inThumb)
				  {
					part = GetCtlValue(ctrlh);
					if (ctrlh == RSlocal[w].scroll)
					  {
					  /* scroll visible region vertically */
						VSgetrgn(w, &x1, &y1, &x2, &y2);
						VSsetrgn(w, x1, part, x2, part + (y2 - y1));
					  }
					else
					  { /* ctrlh must be .left */
					  /* scroll visible region horizontally */
						VSgetrgn(w, &x1, &y1, &x2, &y2);
						VSsetrgn(w, part, y1, part + (x2 - x1), y2);
					  } /* if */
				  } /* if */
				break;
			case inUpButton:
			case inDownButton:
			case inPageUp:
			case inPageDown:
				part2 = TrackControl(ctrlh, where, ScrollProcUPP);	/* BYU LSC */
	/*			InvalRect(&(**RSlocal->scroll).contrlRect); */  /* cheap fix */
				break;
			default:
				break;
		  } /* switch */
	else
	  {
		if (optioned) 
		  {
		  /* send host the appropriate sequences to move the cursor
			to the specified position */
			Point x;
			x = normalize(where, w);
			VSpossend(w, x.h, x.v, screens[scrn].echo); /* MAT--we can check here if we want to use normal */
														/* MAT--or EMACS movement. */
		  }
		else if (ClickInContent(where,w))			/* NCSA: SB - prevent BUS error */
			RSselect(w, where, theEvent);			
	  } /* if */
	return
		0;
  } /* RSclick */

void	RShide( short w)		/* hides a terminal window. */
{
	if (RSsetwind(w) < 0)
		return;
	
	HideWindow(RScurrent->window);
}

void	RSshow( short w)		/* reveals a hidden terminal window. */
{
	if (RSsetwind(w) < 0)
		return;
	
	ShowWindow(RScurrent->window);
}


#ifdef NCSA_TELNET

extern short	NumberOfColorBoxes;
extern short	BoxColorItems[8];
extern RGBColor	BoxColorData[8];

void	RScprompt(short w)
  /* puts up the dialog that lets the user examine and change the color
	settings for the specified window. */
{
	short		scratchshort, ditem;
	Point		ColorBoxPoint;
	DialogPtr	dptr;
	Boolean		UserLikesNewColor;
	RGBColor	scratchRGBcolor;
	
	dptr = GetNewMySmallDialog(ColorDLOG, NULL, kInFront, (void *)ThirdCenterDialog);

	for (scratchshort = 0, NumberOfColorBoxes = 4; scratchshort < NumberOfColorBoxes; scratchshort++) {
		BoxColorItems[scratchshort] = ColorNF + scratchshort;
		BlockMove(&(RSlocal[w].RGBs[scratchshort]),
			&BoxColorData[scratchshort], sizeof(RGBColor));
		UItemAssign( dptr, ColorNF + scratchshort, ColorBoxItemProcUPP);
		}
		
	ColorBoxPoint.h = 0;			// Have the color picker center the box on the main
	ColorBoxPoint.v = 0;			// screen
	
	ditem = 3;	
	while (ditem > 2) {
		ModalDialog(ColorBoxModalProcUPP, &ditem);
		switch (ditem) {
			case	ColorNF:	
			case	ColorNB:	
			case	ColorBF:	
			case	ColorBB:	
				if (theWorld.hasColorQD) {
					UserLikesNewColor = GetColor(ColorBoxPoint, "\pPlease Select New Color",
						 &BoxColorData[ditem-ColorNF], &scratchRGBcolor);
					if (UserLikesNewColor)
						BoxColorData[ditem-ColorNF] = scratchRGBcolor;
					}
				break;
				
			default:
				break;
			
			} // switch
		} // while

	if (ditem == DLOGCancel) {
		DisposeDialog(dptr);
		return;
		}
		
	for (scratchshort = 0; scratchshort < NumberOfColorBoxes; scratchshort++) {
			BlockMove(&BoxColorData[scratchshort], 
				&(RSlocal[w].RGBs[scratchshort]), sizeof(RGBColor));
		}
	
	/* force redrawing of entire window contents */
	SetPort(RSlocal[w].window);
	InvalRect(&RSlocal[w].window->portRect);

	for (scratchshort = 0; scratchshort < 6; scratchshort++)
		SetEntryColor(RSlocal[w].pal, scratchshort, &RSlocal[w].RGBs[scratchshort]);

	DisposeDialog(dptr);
} /* RScprompt */

short RSsetcolor
  (
	short w, /* window number */
	short n, /* color entry number */
	unsigned short r, /* components of new color */
	unsigned short g,
	unsigned short b
  )
  /* sets a new value for the specified color entry of a terminal window. */
  {
    if ( !theWorld.hasColorQD || (RSsetwind(w) < 0))		/* BYU */
		return(-1);
	RScurrent->RGBs[n].red = r;
	RScurrent->RGBs[n].green = g;
	RScurrent->RGBs[n].blue = b;
	if (RScolor)
	  {
	  /* only take account of the color settings on a color-capable machine */
		SetEntryColor(RScurrent->pal, n, &RScurrent->RGBs[n]);
		SetPort(RScurrent->window);
		InvalRect(&RScurrent->window->portRect);
	  } /* if */
	return(0);
  } /* RSsetcolor */

Boolean NewRSsetcolor
	(
	short w, /* window number */
	short n, /* color entry number */
	RGBColor	Color
	)
  /* sets a new value for the specified color entry of a terminal window. */
  {
    if ( !theWorld.hasColorQD || (RSsetwind(w) < 0) || (n > 5) || (n < 0))
		return(-1);
	RScurrent->RGBs[n] = Color;

	if (RScolor) {
	  /* only take account of the color settings on a color-capable machine */
		SetEntryColor(RScurrent->pal, n, &RScurrent->RGBs[n]);
		SetPort(RScurrent->window);
		InvalRect(&RScurrent->window->portRect);
	  } /* if */
	return(0);
  } /* RSsetcolor */

void RSgetcolor
  (
	short w, /* window number */
	short n, /* color entry number */
	unsigned short *r, /* where to return components of color */
	unsigned short *g,
	unsigned short *b
  )
  /* gets the current value for the specified color entry of a terminal window. */
  {
	*r = RSlocal[w].RGBs[n].red;
	*g = RSlocal[w].RGBs[n].green;
	*b = RSlocal[w].RGBs[n].blue;
  } /* RSgetcolor */

short RSmouseintext				/* Point is in global coords */
  (
	short w,
	Point myPoint
  )
  /* is myPoint within the text-display area of the specified window. */
  {
	return
		PtInRect(myPoint, &RSlocal[w].textrect); 	/* BYU LSC */
  } /* RSmouseintext */

void RSskip
  (
	short w,
	Boolean on
  )
  /* sets the "skip" flag for the specified window (whether ignore
	screen updates until further notice). */
  {
	RSlocal[w].skip = on;
  } /* RSskip */

/**********************************************************************************/
/*  Make sure at least this much memory is available before allocating more memory
*   for me.  Returns true or false whether that much is available.
*/
short RSokmem
  (
	short amount
  )
  {
	char *p;
	
	if (NULL == (p = NewPtr(amount)))
		return(0);
	DisposPtr(p);
	return(1);
  } /* RSokmem */

/*------------------------------------------------------------------------------*/
/* NCSA: SB - RScalcwsize 														*/
/* 		This routine is used to switch between 80 and 132 column mode. All that	*/	
/* 		is passed in is the RS window, and the new width.  This calculates the	*/	
/* 		new window width, resizes the window, and updates everything.  - SMB	*/
/*------------------------------------------------------------------------------*/
void RScalcwsize(short w, short width)
{
	short x1,x2,y1,y2;
	short lines;
	
	RSsetwind(w);
	VSsetcols(w,(short)(width-1));
	VSgetrgn(w, &x1, &y1, &x2, &y2);
	x2= width-1;
	
	lines = VSgetlines(w);				/* NCSA: SB - trust me, you need this... */
	RScurrent->rwidth =
		RScurrent->width = (x2 - x1 + 1) * RScurrent->fwidth - CHO;
	RScurrent->rheight =
		RScurrent->height= (y2 - y1 + 1) * RScurrent->fheight; 


	if (RScurrent->rwidth > RMAXWINDOWWIDTH - 16 - CHO)
	 	 RScurrent->rwidth = RMAXWINDOWWIDTH - 16 - CHO;
	if (RScurrent->rheight > RMAXWINDOWHEIGHT - 16)
	 	 RScurrent->rheight = RMAXWINDOWHEIGHT - 16;
	
	SizeWindow
	  (
		RScurrent->window,
		RScurrent->rwidth + 16, RScurrent->rheight+16,
		FALSE
	  ); 
	RSsetsize(w, RScurrent->rheight + 16, RScurrent->rwidth + 16);
	VSgetrgn(w, &x1, &y1, &x2, &y2);
	VSsetrgn(w, x1, y1,
		(short) (x1 + (RScurrent->rwidth ) / RScurrent->fwidth - 1),
		(short) (y1 + (RScurrent->rheight) / RScurrent->fheight - 1));
	VSgetrgn(w, &x1, &y1, &x2, &y2);		/* Get new region */
	
	DrawGrowIcon(RScurrent->window);
	VSredraw(w, 0, 0, x2 - x1 + 1, y2 - y1 + 1); /* redraw newly-revealed area, if any */
	ValidRect(&RScurrent->window->portRect); /* no need to do it again */
	DrawControls(RScurrent->window);
}


/*--------------------------------------------------------------------------*/
/* NCSA: SB - RSbackground													*/
/*	This procedure allows Telnet to switch from dark background to light	*/
/*	background.  Save the current state into the RSdata struct, so that		*/
/* 	we know our background state next time we want to do anything.			*/
/*	Make sure the screen contents (and palette) is updated NOW.				*/
/*--------------------------------------------------------------------------*/
void RSbackground(short w, short value)
{
	RGBColor temp;
	short x;
	
	RSsetwind(w);
	if ((value && !RSlocal[w].flipped) || (!value && RSlocal[w].flipped))
		{	
		RSlocal[w].flipped = !RSlocal[w].flipped;

		temp = RSlocal[w].RGBs[0];
		RSlocal[w].RGBs[0] = RSlocal[w].RGBs[1];
		RSlocal[w].RGBs[1] = temp;
		
		SetPort(RSlocal[w].window);
		InvalRect(&RSlocal[w].window->portRect);
	
		for (x = 0; x < 4; x++)
			SetEntryColor(RSlocal[w].pal, x, &RSlocal[w].RGBs[x]);
		}
}

void RScheckmaxwind(Rect *origRect,short origW, 
			short origH, short *endW, short *endH)
{
	*endW = origW;
	*endH = origH;
	
	if (origW > (TelInfo->screenRect.right - origRect->left -15 ))
		*endW = TelInfo->screenRect.right - origRect->left -15;

	if (origH > (TelInfo->screenRect.bottom - origRect->top -15 ))
		*endH = TelInfo->screenRect.bottom - origRect->top -15;
}		


/*--------------------------------------------------------------------------*/
/* NCSA: SB - ClickInContent												*/
/*	This procedure is a quick check to see if the mouse click is in the		*/
/*	content region of the window.  Normalize the point to be a VS location	*/
/* 	and then see if that is larger than what it should be...				*/
/*	Used by RSClick to see if the click is in the scroll bars, or content..	*/
/*--------------------------------------------------------------------------*/
short ClickInContent(Point where,short w)				/* NCSA: SB */
{														/* NCSA: SB */
	Point x;											/* NCSA: SB */
	x = normalize(where, w);							/* NCSA: SB */
	if (x.v >= VSgetlines(w)) return 0;					/* NCSA: SB */
	else return 1;										/* NCSA: SB */
}														/* NCSA: SB */

#endif NCSA_TELNET

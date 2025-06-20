/*
 *
 *	  Virtual Screen Kernel Internal Routines
 *					  (vsintern.c)
 *  National Center for Supercomputing Applications
 *
 *	  by Gaige B. Paulsen
 *
 *	This file contains the private internal calls for the NCSA
 *  Virtual Screen Kernel.
 *
 *		Version Date	Notes
 *		------- ------  ---------------------------------------------------
 *		0.01	861102  Initial coding -GBP
 *		0.50	861113  First compiled edition -GBP
 *		0.70	861114  Internal operation confirmed -GBP
 *		2.1		871130	NCSA Telnet 2.1 -GBP
 *		2.2 	880715	NCSA Telnet 2.2 -GBP
 */

#ifdef MPW
#pragma segment VS
#endif

#include "TelnetHeader.h"

#include <String.h>
#include <Quickdraw.h>
#include <Controls.h>
#include <Memory.h>
#include <OSUtils.h>				/* LU */
#include <stdio.h>
#include "vsdata.h"
#include "vskeys.h"
#include "vsinterf.proto.h"
#include "rsmac.proto.h"
#include "maclook.proto.h"
#include "wind.h"

#define ScrollbackQuantum 100

#define VSIclrattrib 0

#include "vsintern.proto.h"

extern SysEnvRec	theWorld;		/* BYU 2.4.12 - System Environment record */
extern short 	TempItemsVRefNum;
extern long		TempItemsDirID;
extern WindRec *screens;

short VSIclip
  (
	 short *x1, /* starting column */
	 short *y1, /* line on which to draw (assumed to lie within visible region) */
	 short *x2, /* ending column (inclusive) (output if *n >= 0) */
	 short *y2, /* ending line (inclusive) (output if *n >= 0) */
	 short *n, /* length of text to draw (input and output) */
	 short *offset /* length of initial part of text to skip (output) */
  )
  /* clips a text string to the visible region, given the starting
	line and column in screen coordinates at which it is to be drawn.
	If the length of the string is given, will also compute the ending
	line and column. On return, these coordinates will be normalized
	to the current visible region. Returns a nonzero function result
	iff the string is completely invisible. */
  {
	if (*n >= 0)
	  {
	  /* compute ending line and column (inclusive) */
		*x2 = *x1 + *n - 1;
		*y2 = *y1;
	  }
  /* else take these as given */

	if ((*x1 > VSIw->Rright) || (*y2 < VSIw->Rtop))
		return (-1); /* nothing to draw */

	if (*x2 > VSIw->Rright)
		*x2 = VSIw->Rright;
	if (*y2 > VSIw->Rbottom)
		*y2 = VSIw->Rbottom;
  /* normalize x1, x2, y1, y2 to be relative to current visible region */
	*x1 -= VSIw->Rleft;
	*x2 -= VSIw->Rleft;
	*y1 -= VSIw->Rtop;
	*y2 -= VSIw->Rtop;
  /* clip part of text string lying outside region, if any */
	*offset = - *x1;
	if (*offset < 0)
		*offset = 0; /* text string starts within region--nothing to clip */
  /* don't draw anything outside region */
	if (*x1 < 0)
		*x1 = 0;
	if (*y1 < 0)
		*y1 = 0;

	*n = *x2 - *x1  + 1 ; /* length of string to draw (assuming it's all on one line) */
	if ((*n <= 0) || (*y2 - *y1 < 0))
		return (-1); /* nothing to draw */
	return (0);
  } /* VSIclip */

short VSIcdellines(short w, short top, short bottom, short n, short scrolled)
		  /*
			-ve => cancel current selection, if any;
			+ve => selection has moved up one line;
			0 => don't touch selection
		  */
  /* updates the display to indicate deletion of the specified
	number of lines from the top of the specified region.
	Returns 0 iff any part of the change is visible. */
  {
	short
		x1 = 0,
		x2 = VSIw->maxwidth,
		tn = -1,
		offset;

	if (VSIclip(&x1, &top, &x2, &bottom, &tn, &offset))
		return(-1); /* affected region is invisible */
	tn = bottom - top;
	if (tn < n)
		n = tn; /* don't bother scrolling more lines than scrolling region holds */
	RSdellines(w, top, bottom, n, scrolled);
	return(0);				/* I delete the whole thing! */
  } /* VSIcdellines */

short VSIcinslines(short w, short top, short bottom, short n, short scrolled) /* -ve <=> cancel current selection, if any */
  /* updates the display to indicate insertion of the specified
	number of blank lines at the top of the specified region.
	Returns 0 iff any part of the change is visible. */
  {
	short
		x1 = 0,
		x2 = VSIw->maxwidth,
		tn = -1,
		offset;

	if (VSIclip(&x1, &top, &x2, &bottom, &tn, &offset))
		return -1; /* affected region is invisible */
	tn = bottom - top;
	if (tn < n)
		n = tn; /* don't bother scrolling more lines than scrolling region holds */
	RSinslines(w, top, bottom, n, scrolled);
	return 0;
  } /* VSIcinslines */

void VSIcurson
  (
	short w,
	short x,
	short y,
	short ForceMove
  )
  /* displays the text cursor at the specified position. If
	ForceMove is true, I am to do any appropriate scrolling of
	the display to ensure the cursor is within the visible region.
	Assumes cursor isn't currently being shown. */
  {
	short
		x2,
		y2,
		n = 1,
		offset;

	if (!VSIclip(&x, &y, &x2, &y2, &n, &offset))
	  /* cursor already lies within visible region */
		RScurson(w, x, y); /* just make it visible */
	else if (ForceMove)
	  {
	  /* scroll to make cursor visible */
		x2 = VSIw->Rbottom - VSIw->Rtop;
		if (x2 >= VSIw->lines)
		  /* visible region is big enough to show entire screen--
			make sure I don't scroll off the bottom of the screen.
			This call will also do any appropriate scrolling and
			redisplaying of the cursor. */
			VSsetrgn(VSIwn, VSIw->Rleft, VSIw->lines - x2,
				VSIw->Rright, VSIw->lines);
		else
		  {
		  /* x & y have been normalized relative to left & top
			of current visible region. Just call the appropriate scroll
			routine, which will also redisplay the cursor. */
			if (y > 0)
				VSscrolforward(VSIwn, y);
			else
				VSscrolback(VSIwn, -y);
		  } /* if */
	  } /* if */
  } /* VSIcurson */

void VSIcuroff
  (
	short w
  )
  /* hides the cursor for the specified screen. Assumes it
	is currently being shown (or that it's on an invisible
	part of the screen). */
  {
	short
		x = VSIw->x,
		y = VSIw->y,
		x2,
		y2,
		n = 1,
		offset;

	if (!VSIclip(&x, &y, &x2, &y2, &n, &offset))
	  /* cursor is on visible part of screen */
		RScursoff(w);
  } /* VSIcuroff */

short VSIcursorvisible		/* BYU 2.4.12 */
  (							/* BYU 2.4.12 */
	void					/* BYU 2.4.12 */
  )							/* BYU 2.4.12 */
  {							/* BYU 2.4.12 */
	short						/* BYU 2.4.12 */
		x = VSIw->x,		/* BYU 2.4.12 */
		y = VSIw->y,		/* BYU 2.4.12 */
		x2,					/* BYU 2.4.12 */
		y2,					/* BYU 2.4.12 */
		n = 1,				/* BYU 2.4.12 */
		offset;				/* BYU 2.4.12 */
															/* BYU 2.4.12 */
	if (!VSIclip(&x, &y, &x2, &y2, &n, &offset))			/* BYU 2.4.12 */
		return 1;											/* BYU 2.4.12 */
	else					/* BYU 2.4.12 */
		return 0;			/* BYU 2.4.12 */
}							/* BYU 2.4.12 */

VSlineArray VSInewlinearray
  (
	short nrlines
  )
  /* allocates an array to hold the specified number of pointers
	to line elements. */
  {
	return
		(VSlineArray) NewPtrClear(sizeof(VSlinePtr) * nrlines);
  } /* VSInewlinearray */

VSlinePtr VSInewlines
  (
	short nlines
  )
  /* allocates a doubly-linked list of the specified number of
	line elements, and returns a pointer to the head of the list,
	or nil if ran out of memory. The maximum number of characters
	each line can hold is controlled by VSIw->allwidth. */
  {
	VSlinePtr t2;
	char *t;
	register short i;
	
	if (!RSokmem(30000))				/* don't use up end of mem for scrollback */
		return((VSlinePtr) 0L);
  /* allocate one block for the line list elements, and another
	block for the line contents. These blocks will be divided up
	and appropriate flags set so I will be able to call DisposPtr
	the right number of times, with the right arguments. */
	if ((t = NewPtr(nlines * (VSIw->allwidth + 1))) != 0L)
	  {
		if ((t2 = (VSlinePtr) NewPtr(nlines * sizeof(VSline))) != 0L)
			t2->text = t;
		else
		  {
		  /* clean up gracefully before giving up */
			DisposPtr(t);
			return(0L);
		  } /* if */
	  }
	else
	  /* sorree no memoree */
		return((VSlinePtr) 0L);

/*
*  indicate to the free routine that the first record is the one to free.
*/
	t2->mem = 1;						/* call DisposPtr on this one */
	t2->next = t2 + 1;					/* point to next one */
/*
*  Take our allocation for multiple lines and fill in the structures to 
*  point to the right text fields and connect the doubly-linked chain.
*
*/
	for (i = 1; i < nlines; i++)
	  {
		t += (VSIw->allwidth + 1);		/* inc to next text space for a line */
		t2[i].mem = 0;					/* don't DisposPtr any of these */
		t2[i].text = t;
		t2[i].prev = t2 + i - 1;		/* point back one */
		t2[i].next = t2 + i + 1;		/* point forward one */
	  } /* for */
	
	t2[0].prev = 0L;					/* first one has no prev yet */
	t2[nlines - 1].next = 0L;			/* last one has no next yet */

	return(t2);
  } /* VSInewlines */

void VSIlistndx
  (
	register VSlinePtr ts,
	register VSlinePtr as
  )
  /* sets up the screen arrays for the current screen to point
	at the given lists of attribute and text lines. */
  {
	register short i;
	for (i = 0; i <= VSIw->lines; i++)
	  {
		VSIw->attrst[i] = as;
		VSIw->linest[i] = ts;
		ts = ts->next;
		as = as->next;
	  } /* for */
  } /* VSIlistndx */

void VSIscroff
  (
	void
  )
  /* called to save current screen contents in scrollback buffer,
	if it is ordained that I should do so. This is called by VSIes
	(below) just before the entire screen is cleared. */
  {
	VSlinePtr tmp;
	register short i, j;

	if
	  (
			(!VSIw->savelines) /* not saving lines */
		||
			(VSIw->top != 0) || (VSIw->bottom != VSIw->lines)
			  /* scrolling region isn't exactly the entire screen */
	  )
		return; /* do nothing */

	tmp = VSIw->linest[VSIw->lines]; /* need VSIw->lines + 1 more lines */
	for (i = 0; i <= VSIw->lines; i++)
	  {
	  /* count off the lines in the part of the scrollback buffer
		below the screen (if any), to see if there's enough to hold
		a screenful. If the scrollback list isn't circular, then
		this part contains lines that have been allocated, but not
		yet used. If the list is circular (meaning it has reached
		its full size), then this is the part that is next in line
		for reuse. */
		if (!tmp->next) 
		  { /* not enough */
			j = VSIw->maxlines - VSIw->numlines - i; /* potential unallocated scrollback */
			if (j > ScrollbackQuantum)
				j = ScrollbackQuantum; /* but don't bother allocating more than this */
			if (j <= 0)
			  {
			  /* already reached user-specified scrollback limit-- */
			  /* make the list circular to indicate no more extension. */
				tmp->next = VSIw->buftop;
				VSIw->buftop->prev = tmp;		/* connect it up */
			  }
			else
			  {
			  /* extend the scrollback buffer to make room for
				another screenful */
				if (j < VSIw->lines - i + 1)
					j = VSIw->lines - i + 1; /* need at least this many */
				if ((tmp->next = VSInewlines(j)) != 0L) 
					tmp->next->prev = tmp;		/* got some space--link it up */
				else
				  {
				  /* out of memory--no more extensions */
					tmp->next = VSIw->buftop;
					VSIw->buftop->prev = tmp;	
				  } /* if */				
			  } /* if */
			break;								/* only allocate once is enough */
		  } /* if */
		tmp = tmp->next; /* keep counting */
	  } /* for */
		
/*
*  at this point, we know we have enough memory for the whole scroll.
*  It might be wraparound (reuse of some line elements), might not.
*/
		
	for (i = 0; i <= VSIw->lines; i++)
	  {
	  /* push another screen line into the scrollback area */
		if (VSIw->linest[VSIw->lines]->next == VSIw->buftop)
			VSIw->buftop = VSIw->buftop->next;	/* reusing old space */
		else
			VSIw->numlines++;				/* using some new space */
		VSIw->scrntop = VSIw->scrntop->next; /* move another line into the scrollback buffer */
		VSIlistndx(VSIw->scrntop, VSIw->attrst[1]); /* and update screen arrays */
	  /* note that it's up to the caller to clear out the new screen text
		and attribute lines */
	  } /* for */

	VSIw->vistop = VSIw->scrntop;
	RSbufinfo(VSIwn, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom); /* update vertical scroll bar */
  } /* VSIscroff */

void VSIelo
  (
	short s /* line to erase, -ve => line containing cursor */
  )
  /* blanks out the specified line in the screen buffer.
	Doesn't do anything to the display. */
  {
	char *tt, *ta;
	short i;

	if (s < 0)
		s = VSIw->y;

	ta = &VSIw->attrst[s]->text[0];
	tt = &VSIw->linest[s]->text[0];
	for (i = 0; i <= VSIw->allwidth; i++)
	  {
		*ta++ = VSIclrattrib;
		*tt++ = ' ';
	  } /* for */
  } /* VSIelo */

void VSIes
  (
	void
  )
  /* clears the screen, first saving its contents in the
	scrollback buffer if appropriate. Also updates the display. */
  {
	short
		i;
	short
		x1 = 0,
		y1 = 0,
		x2 = VSIw->maxwidth,
		y2 = VSIw->lines,
		n = -1,
		offset;

  /* save screen contents in scrollback buffer, if appropriate */
	if (VSIw->ESscroll)
		VSIscroff();
  /* clear out screen buffer */
	for (i = 0; i <= VSIw->lines; i++)
		VSIelo(i);
  /* update display to show what I've done */
	if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
		RSerase(VSIwn, x1, y1, x2, y2);
	VSIw->vistop = VSIw->scrntop;
  } /* VSIes */

void VSItabclear
  (
	void
  )
  /* clears all current tab settings. */
  {
	short
		x = 0;

	while (x <= VSIw->allwidth)
	  {
		VSIw->tabs[x] = ' ';
		x++;
	  } /* while */
  } /* VSItabclear */

void VSItabinit
  (
	void
  )
  /* sets tab settings to default (every 8 columns). */
  {
	short
		x = 0;

	VSItabclear();
	while (x <= VSIw->allwidth)
	  {
		VSIw->tabs[x] = 'x';
		x += 8;
	  }
	VSIw->tabs[VSIw->allwidth] = 'x';
  } /* VSItabinit */

void VSIreset
  (
	void
  )
  /* restores terminal mode settings to defaults and clears screen. */
  {
	VSIw->top = 0;
	VSIw->bottom = VSIw->lines;
	VSIw->parmptr = 0;
	VSIw->escflg = 0;
	VSIw->DECAWM = 0;
	VSIw->DECCKM = 0;
	VSIw->DECPAM = 0;
	VSIw->DECORG = 0;		/* NCSA: SB -- is this needed? */
	VSIw->Pattrib = -1;		/* NCSA: SB -- is this needed? */
	VSIw->IRM = 0;
	VSIw->attrib = 0;
	VSIw->x = 0;
	VSIw->y = 0;
	VSIw->charset = 0;
	VSIw->prbuf=0;										/* LU */
	if (VSIw->prredirect) {								/* LU - kill redirection */
		VSIw->prredirect=0;								/* LU */
		FSClose (VSIw->refNum);							/* LU */
		VSIw->refNum=-1;								/* LU */
		HDelete(TempItemsVRefNum, TempItemsDirID, (StringPtr)VSIw->fname);		/* LU */
	}													/* LU */
	VSIes();
	VSItabinit();
	} /* VSIreset */

void VSIlistmove(VSlinePtr TD, VSlinePtr BD, VSlinePtr TI, VSlinePtr BI)
  /* moves the lines from TD to BD inclusive from their
	present position to between TI and BI. Either of the
	latter may be nil. */
  {
  /* unlink the lines from TD to BD */
	if (TD->prev != 0L)
		TD->prev->next = BD->next;	/* Maintain circularity */
	if (BD->next != 0L)
		BD->next->prev = TD->prev;
  /* relink them between TI and BI */
	TD->prev = TI;								/* Place the node in its new home */
	BD->next = BI;
	if (TI != 0L)
		TI->next = TD;					/* Ditto prev->prev */
	if (BI != 0L)
		BI->prev = BD;
  } /* VSIlistmove */

void VSIdellines
  (
	short n, /* nr lines to delete */
	short s /* starting line to delete, -ve => line containing cursor */
  )
  /* deletes lines from the screen, scrolling up the remainder and
	inserting new blank lines at the bottom of the scrolling region. */
  {
	short i, j;
	char *ta, *tt;
	VSlinePtr as, ts, TD, BD, TI, BI, itt, ita;

	if (s < 0)
		s = VSIw->y;
	if (s + n - 1 > VSIw->bottom)
		n = VSIw->bottom - s + 1; /* don't bother deleting more than scrolling region will hold */

  /* find new tops of screen arrays */
	if (s == 0 && n <= VSIw->lines)
	  {
	  /* element for line after last one being deleted */
		ts = VSIw->linest[n];
		as = VSIw->attrst[n];
	  }
	else
	  {
	  /* top line unaffected, or entire screen is being wiped */
		ts = VSIw->linest[0];
		as = VSIw->attrst[0];
	  } /* if */

	TD = VSIw->linest[s]; /* topmost line to delete */
	BD = VSIw->linest[s + n - 1]; /* bottommost line to delete */
	TI = VSIw->linest[VSIw->bottom]; /* insert replacement blank lines after this line */
	BI = TI->next; /* insert them before this line (might be nil) */
	itt = TD; /* start of text lines to be blanked out */
  /* the space taken by the deleted lines will be reused for
	the inserted blank lines */
	if (TD != BI && TI != BD)
	  /* insertion and deletion areas not adjacent -- move the lines to
		their new position */
		VSIlistmove(TD, BD, TI, BI);

	TD = VSIw->attrst[s]; /* topmost line to delete */
	BD = VSIw->attrst[s + n - 1]; /* bottommost line to delete */
	TI = VSIw->attrst[VSIw->bottom]; /* insert new lines after this one */
	BI = TI->next; /* insert them before this line */
  /* perform same rearrangement on attribute lines as on text lines */
	if (TD != BI && TI != BD)
	  /* insertion and deletion areas not adjacent -- move the lines to
		their new position */
		VSIlistmove(TD, BD, TI, BI);

  /* blank out the newly-created replacement lines */
	ita = TD; /* start of attribute lines to be blanked out */
	for (i = 0; i < n; i++)
	  {
		ta = ita->text;
		tt = itt->text;
		for (j = 0; j <= VSIw->allwidth; j++)
		  {
			*tt++ = ' ';
			*ta++ = VSIclrattrib;
		  } /* for */
		ita = ita->next;
		itt = itt->next;
	  } /* for */

	VSIw->scrntop = ts; /* new topmost line (if it's changed) */
  /* re-sync screen arrays */
	VSIlistndx(ts, as);
	if (VSIw->Rtop >= 0)
	  /* make sure vistop still points to same line position
		on screen that it did before */
		VSIw->vistop = VSIw->linest[VSIw->Rtop];
  /* and actually display the change on-screen */
	VSIcdellines(VSIwn, s, VSIw->bottom, n, -1); /* Cancel current selection */
  } /* VSIdellines */

void VSIinslines
  (
	short n, /* how many to insert */
	short s /* where to insert them, -ve => line containing cursor */
  )
  /* inserts the specified number of blank lines, scrolling the
	remaining ones down, and dropping off any that fall off the
	end of the scrolling region. */
  {
	short i, j;
	char *ta, *tt;
	VSlinePtr as, ts, TD, BD, TI, BI, itt, ita;

	if (s < 0)
		s = VSIw->y;
	if (s + n - 1 > VSIw->bottom)
	  /* don't bother inserting more than scrolling region can hold */
		n = VSIw->bottom - s + 1;

  /* find new tops of screen arrays */
	if (s == 0 && n <= VSIw->lines)
	  {
	  /* element for first blank line being inserted */
		ts = VSIw->linest[VSIw->bottom - n + 1];
		as = VSIw->attrst[VSIw->bottom - n + 1];
	  }
	else
	  {
	  /* top line unaffected, or entire screen is being wiped */
		ts = VSIw->linest[0];
		as = VSIw->attrst[0];
	  } /* if */

	BI = VSIw->linest[s]; /* insert blank lines before this one */
	TI = BI->prev; /* insert them after this one */
	TD = VSIw->linest[VSIw->bottom - n + 1]; /* topmost line to delete */
	BD = VSIw->linest[VSIw->bottom]; /* bottommost line to delete */
	itt = TD; /* start of text lines to be blanked out */
  /* the space occupied by the deleted lines will be reused for
	the new blank lines */
	if (TD != BI && TI != BD)
	  /* new and deleted lines not contiguous -- move the space
		to its new position */
		VSIlistmove(TD, BD, TI, BI);

	BI = VSIw->attrst[s]; /* insert new lines before this one */
	TI = BI->prev; /* insert them after this one */
	TD = VSIw->attrst[VSIw->bottom - n + 1]; /* topmost line to delete */
	BD = VSIw->attrst[VSIw->bottom]; /* bottommost line to delete */
  /* do the same rearrangement on the attribute lines */
	if (TD != BI && TI != BD)
	  /* new and deleted lines not contiguous -- move the space
		to its new position */
		VSIlistmove(TD, BD, TI, BI);

  /* blank out the newly-inserted lines */
	ita = TD; /* start of attribute lines to be blanked out */
	for (i = 0; i < n; i++)
	  {
		tt = itt->text;
		ta = ita->text;
		for (j = 0; j <= VSIw->allwidth; j++)
		  {
			*tt++ = ' ';
			*ta++ = VSIclrattrib;
		  }
		itt = itt->next;
		ita = ita->next;
	  } /* for */

	VSIw->scrntop = ts;
	VSIlistndx(ts, as); /* re-sync screen arrays */
	if (VSIw->Rtop >= 0)
	  /* make sure vistop still points to same line position
		on screen that it did before */
		VSIw->vistop = VSIw->linest[VSIw->Rtop];
  /* update display to match reality */
	VSIcinslines(VSIwn, s, VSIw->bottom, n, -1);  /* Destroy selection area if this is called tooo */
  } /* VSIinslines */

void VSIscroll
  (
	void
  )
  /* scrolls scrolling region up one line. */
  {
	register char *temp, *tempa;
	VSlinePtr tmp;
	register short i;
//	short tx1, tx2, ty1, ty2, tn, offset;
	short theBottom;				/* NCSA 2.5: the correct screen bottom */

	if (VSIw->y > VSIw->lines)		/* BYU - replaces BYU modification below */
		return;						/* BYU */

//	tx1 = ty1 = 0;
//	tn = 132;
//	if (!VSIclip(&tx1, &ty1, &tx2, &ty2, &tn, &offset))
	  /* top line of screen is visible */
//		RSdrawsep(VSIwn, ty1, 1);					/* Draw Separator */

#if 0														/* BYU */
/* BYU mod - Don't scroll if the cursor is outside of the scrolling region */
	if ((VSIw->y < VSIw->top) || (VSIw->y > VSIw->bottom))	/* BYU */
		return;												/* BYU */
#endif														/* BYU */

	if ((!VSIw->savelines) /* no scrollback */    ||    (VSIw->top != 0) 	/* NCSA 2.5 */
			|| ((VSIw->bottom != VSIw->lines) && !VSIw->forcesave))	/* NCSA 2.5 */
			  /* region being scrolled is not entire screen */		

	  /* no saving of lines */
		VSIdellines(1, VSIw->top);
	else
	  {
	  /* scrolling region is entire screen, and lines are being saved off top */
		if (VSIw->linest[VSIw->lines]->next == 0L)
		  {
		  /* all currently-allocated scrollback lines have been used, but
			scrollback buffer isn't at its full size -- allocate some more
			space */
			i = VSIw->maxlines - VSIw->numlines; /* number of lines that can be allocated */
			if (i > ScrollbackQuantum)
				i = ScrollbackQuantum; /* don't bother allocating more than this at once */
			if ((i > 0) && (tmp = VSInewlines(i)) != 0L)
			  {
			  /* link newly-allocated lines into the list */
				VSIw->linest[VSIw->lines]->next = tmp;
				tmp->prev = VSIw->linest[VSIw->lines];
				VSIw->numlines++; /* use one of the newly-allocated scrollback lines */
				RSbufinfo(VSIwn, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom); /* update vertical scroll bar accordingly */
			  }
			else
			  {
			  /* not enough memory to extend scrollback buffer--reuse
				oldest line and give up on future extensions */
				VSIw->linest[VSIw->lines]->next = VSIw->buftop;		/* Make it circular */
				VSIw->buftop->prev = VSIw->linest[VSIw->lines];
				VSIw->buftop = VSIw->buftop->next;	/* step one forward */
			  } /* if */
		  }	
		else
		  {
		  /* either there's allocated, but not yet used, space at
			VSIw->linest[VSIw->lines]->next, or the text line list
			is circular. Either way, don't do any new scrollback
			allocation. */
			if (VSIw->linest[VSIw->lines]->next == VSIw->buftop)
			  /* scrollback buffer is at full size--reuse oldest line */
				VSIw->buftop = VSIw->buftop->next;
			else
			  {
			  /* haven't used up all the space I allocated last time */
				VSIw->numlines++;					/* count another line */
				RSbufinfo(VSIwn, VSIw->numlines, VSIw->Rtop, VSIw->Rbottom); /* update vertical scroll bar accordingly */
			  } /* if */
		  } /* if */

		VSIw->scrntop = VSIw->scrntop->next; /* scroll the screen buffer */
		VSIlistndx(VSIw->scrntop, VSIw->attrst[1]); /* update screen arrays */
	  /* reflect the change in the display by scrolling up the visible
		part of the on-screen area, if any */


		if (VSIw->forcesave) theBottom = VSIw->bottom;	/* NCSA 2.5: get the correct scroll rgn */
		else theBottom = VSIw->Rbottom;				/* NCSA 2.5: just use whole screen */
		
		if (VSIcdellines(VSIwn, VSIw->Rtop, theBottom, 1, 1))	/* NCSA 2.5 */
		  {
		  /* no part of on-screen area is visible */
			if (VSIw->Rtop > -VSIw->numlines)
			  /* update bounds of visible region to be consistent
				with portion of scrollback buffer still being displayed */
			  {
				VSIw->Rtop--;
				VSIw->Rbottom--;
			  }
			else
			  {
			  /* displaying right from top of scrollback buffer. Topmost
				line being shown has in fact vanished. Update the display
				to show this fact. */
				VSIw->vistop = VSIw->vistop->next;
				RSdellines(VSIwn, 0, VSIw->Rbottom - VSIw->Rtop, 1, 1);
			  } /* if */
		  }
		else
			VSIw->vistop = VSIw->vistop->next; /* consistent with changed display */
	  /* blank out newly-revealed bottom line */
		tempa = VSIw->attrst[VSIw->lines]->text;
		temp = VSIw->linest[VSIw->lines]->text;
		for (i = 0; i <= VSIw->allwidth; i++)
		  {
			*temp++ = ' ';
			*tempa++ = 0;
		  } /* for */
	  } /* if */
//	tx1 = ty1 = 0;
//	tn = 132;
//	if (!VSIclip(&tx1, &ty1, &tx2, &ty2, &tn, &offset))
//		RSdrawsep(VSIwn, ty1, 1);					/* Draw Separator */
  } /* VSIscroll */

void VSIindex
  (
	void
  )
  /* moves cursor down one line, unless it's at the bottom of
	the scrolling region, in which case scrolls up one. */
  {
	if (VSIw->y == VSIw->bottom)	/* BYU - changed "==" to ">=" and back again */
		VSIscroll();
	else if (VSIw->y < VSIw->lines) 	/* BYU  - added "if ... " */
		VSIw->y++;
  } /* VSIindex */

void VSIwrapnow(short *xp, short *yp)
  /* checks current cursor position for VSIw to see if
	it's within bounds, wrapping to next line if not.
	Returns correct cursor position in either case in *xp
	and *yp. */
  {
	if (VSIw->x > VSIw->maxwidth) 
	  {
		VSIw->x = 0;
		VSIindex();
	  } /* if */
	*xp = VSIw->x;
	*yp = VSIw->y;
  } /* VSIwrapnow */

void VSIeeol
  (
	void
  )
  /* erases characters to the end of the current line. */
  {
	char
		*tt,
		*ta;
	short
		x1 = VSIw->x,
		y1 = VSIw->y,
		x2 = VSIw->maxwidth,
		y2 = VSIw->y,
		n = -1,
		offset;
	short
		i;

	VSIwrapnow(&x1, &y1);
	y2 = y1;
  /* clear out screen line */
	ta = &VSIw->attrst[y1]->text[x1];
	tt = &VSIw->linest[y1]->text[x1];
	for (i = VSIw->allwidth - x1 + 1; i > 0; i--)
	  {
		*ta++ = VSIclrattrib;
		*tt++ = ' ';
	  }
  /* update display */
	if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
		RSerase(VSIwn, x1, y1, x2, y2);
  } /* VSIeeol */

void VSIdelchars
  (
	short x /* how many characters to delete */
  )
  /* deletes characters at the current cursor position onwards,
	moving the remainder of the line to the left. */
  {
	short
		i;
	short
		x1 = VSIw->x,
		y1 = VSIw->y,
		x2 = VSIw->maxwidth,
		y2 = VSIw->y,
		n = -1,
		offset;
	char
		*tempa,
		*temp;

	VSIwrapnow(&x1, &y1);
	y2 = y1;

	if (x > VSIw->maxwidth)
		x = VSIw->maxwidth;
	tempa = VSIw->attrst[y1]->text;
	temp = VSIw->linest[y1]->text;
	for (i = x1; i <= VSIw->maxwidth - x; i++)
	  {
	  /* move remainder of line to the left */
		temp[i] = temp[x + i];
		tempa[i] = tempa[x + i];
	  }
	for (i = VSIw->maxwidth - x + 1; i <= VSIw->allwidth; i++)
	  {
	  /* insert blank characters after end of line */
		temp[i] = ' ';
		tempa[i] = VSIclrattrib;
	  }
  /* update display */
	if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset))
	  {
		if (VSIw->VSIDC)
			RSdelchars(VSIwn, x1, y1, x);
		else
		  /* redraw from cursor position to end of line.
			Trouble is, this isn't going to preserve the
			right attributes. */
			RSdraw
			  (
				VSIwn, x1, y1, VSIw->attrib, n,
				&VSIw->linest[y1]->text[x1]
			  );
	  } /* if */
  } /* VSIdelchars */

void VSIfreelinelist
  (
	VSlinePtr listhead
  )
  /* frees up the list of line elements pointed to by listhead. */
  {
	register VSlinePtr
		ThisElt, NextElt, ToFree;

	ThisElt = listhead;
	ToFree = nil;
	while (true)
	  {
		if (ThisElt == nil)
			break;
		NextElt = ThisElt->next;
		if (ThisElt->mem)
		  {
			ThisElt->next = ToFree;
			ToFree = ThisElt;
		  } /* if */
		ThisElt = NextElt;
		if (ThisElt == listhead)
			break;
	  } /* while */
	while (ToFree)
	  {
		NextElt = ToFree->next;
		DisposPtr(ToFree->text);
		DisposPtr((Ptr) ToFree);
		ToFree = NextElt;
	  } /* while */
  } /* VSIfreelinelist */

void VSIfreelines
  (
	void
  )
  /* frees up all the memory allocated for screen and scrollback
	text lines for the current screen. */
  {
	VSIfreelinelist(VSIw->buftop);
  } /* VSIfreelines */

void VSIrindex
  (
	void
  )
  /* moves cursor up one line, unless it's at the top of
	the scrolling region, in which case scrolls down one. */
  {
	if (VSIw->y == VSIw->top)
		VSIinslines(1, VSIw->top);
	else
		VSIw->y--;
  } /* VSIrindex */

void VSIebol
  (
	void
  )
  /* erases characters from beginning of line to cursor. */
  {
	char
		*tt,
		*ta;
	short
		x1 = 0,
		y1 = VSIw->y,
		x2 = VSIw->x,
		y2 = VSIw->y,
		n = -1,
		offset;
	short
		i;

	VSIwrapnow(&x2, &y1);
	y2 = y1;
  /* clear from beginning of line to cursor */
	ta = &VSIw->attrst[y1]->text[0];
	tt = &VSIw->linest[y1]->text[0];
	for (i = 0; i <= x2; i++)
	  {
		*ta++ = VSIclrattrib;
		*tt++ = ' ';
	  }
  /* update display */
	if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
		RSerase(VSIwn, x1, y1, x2, y2);
  } /* VSIebol */

void VSIel
  (
	short s /* line to clear, -ve => line containing cursor */
  )
  /* erases the specified line. */
  {
	char
		*tt,
		*ta;
	short
		x1 = 0,
		y1 = s,
		x2 = VSIw->maxwidth,
		y2 = s,
		n = -1,
		offset;
	short
		i;

	if (s < 0)
	  {
		VSIwrapnow(&x1, &y1);
		s = y2 = y1;
		x1 = 0;
	  } /* if */
  /* clear out line */
	ta = &VSIw->attrst[s]->text[0];
	tt = &VSIw->linest[s]->text[0];
	for(i = 0; i <= VSIw->allwidth; i++)
	  {
		*ta++ = VSIclrattrib;
		*tt++ = ' ';
	  }
  /* update display */
	if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
		RSerase(VSIwn, x1, y1, x2, y2);
  } /* VSIel */

void VSIeeos
  (
	void
  )
  /* erases characters from cursor to end of screen. */
  {
	short
		i;
	short
		x1 = 0,
		y1 = VSIw->y + 1,
		x2 = VSIw->maxwidth,
		y2 = VSIw->lines,
		n = -1,
		offset;

	VSIwrapnow(&x1, &y1);
	y1++;
	x1 = 0;

	i = y1;
 
 	if (VSIw->forcesave)		/* NCSA 2.5 */
		VSIscroff();			/* NCSA 2.5 */

 
  /* erase complete lines from screen */
	if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
		RSerase(VSIwn, x1, y1, x2, y2);
  /* blank out current line from cursor to end */
	VSIeeol(); /* this also erases the partial line on-screen */
  /* blank out remaining lines to end of screen */
	while (i <= VSIw->lines)
	  {
		VSIelo(i);
		i++;
	  } /* while */
	if (VSIw->y < VSIw->lines && (VSIw->x <= VSIw->maxwidth))
	  /* erase the partial line (what--again??) */
		if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
			RSerase(VSIwn, x1, y1, x2, y2);
  } /* VSIeeos */

void VSIebos
  (
	void
  )
  /* erases characters from beginning of screen to cursor. */
  {
	short
		i;
	short
		x1,
		y1,
		x2 = VSIw->maxwidth,
		y2,
		n = -1,
		offset;

	VSIwrapnow(&x1, &y1);
	y2 = y1 - 1;
	x1 = 0;
	y1 = 0;
  /* blank out current line from beginning to cursor */
	VSIebol(); /* this also erases the partial line on-screen */
	i = 0;
  /* blank out remaining lines from beginning of screen to previous line */
	while (i < (y2 + 1))
	  {
		VSIelo(i);
		i++;
	  } /* while */
	if (y2 >= 0)
	  /* erase the partial line (what--again??) */
		if (!VSIclip(&x1, &y1, &x2, &y2, &n, &offset)) 
			RSerase(VSIwn, x1, y1, x2, y2);
  } /* VSIebos */

void VSIrange
  (
	void
  )
  /* constrains cursor position to valid range (somewhere on the screen). */
  {
	short
		wrap = 0;
	if (VSIw->DECAWM)
		wrap = 1;
	if (VSIw->x < 0)
		VSIw->x = 0;
	if (VSIw->x > (VSIw->maxwidth + wrap))
		VSIw->x = VSIw->maxwidth + wrap;
	if (VSIw->y < 0)
		VSIw->y = 0;
	if (VSIw->y > VSIw->lines)
		VSIw->y = VSIw->lines;
  } /* VSIrange */

void VTsendpos
  (
	void
  )
  /* sends an escape sequence representing the current cursor position. */
  {
	char
		tempbuf[19];
	short
		x = VSIw->x,
		y = VSIw->y;

	if (x > VSIw->maxwidth)
	  {
	  /* autowrap pending */
		x = 0;
		y++;
	  }
	if (y > VSIw->lines)
	  /* scroll pending (because of the autowrap) */
		y = VSIw->lines;

	sprintf(tempbuf, "\033[%d;%dR", y + 1, x + 1);
	RSsendstring(VSIwn, tempbuf, strlen(tempbuf));
  } /* VTsendpos */

void VTsendstat
  (
	void
  )
  /* sends the terminal status string. */
  {
	RSsendstring(VSIwn, "\033[0n", 4);
  } /* VTsendstat */

void VTsendident
  (
	void
  )
  /* sends an appropriate terminal identification sequence. */
  {
#ifdef VT100RESP
	if (VSIw->allwidth > 80)
		RSsendstring(VSIwn, "\033[?4;6c", 7);
	else
		RSsendstring(VSIwn, "\033[?1;6c", 7);
#endif VT100RESP
	if (screens[findbyVS(VSIwn)].vtemulation)
		RSsendstring(VSIwn, "\033[?62;1;6c", 10);			/* BYU 2.4.12 - VT200-series*/
	else													/* BYU 2.4.12 */
		RSsendstring(VSIwn, "\033[?6c", 5);					/* BYU 2.4.12 - VT102 */
  } /* VTsendident */

void VTalign
  (
	void
  )
  /* fills screen with uppercase "E"s, for checking screen alignment. */
  /* Yeah, right. */
  {
	char *tt;
	short i, j;

	VSIes();		/* erase the screen */
	for (j = 0; j < VSIw->lines; j++)
	  {
		tt = &VSIw->linest[j]->text[0];
		for (i = 0; i <= VSIw->maxwidth; i++)
			*tt++ = 'E';
	  } /* for */
  /* update the display */
	VSredraw(VSIwn, 0, 0,
		(VSIw->Rright - VSIw->Rleft), (VSIw->Rbottom - VSIw->Rtop));
  } /* VTalign */

void VSIapclear
  (
	void
  )
  /* initializes all the parameters for the current control
	sequence, and the current param index, to zero. */
  {
	short
		parmptr = maxparms;
	while (--parmptr >= 0)
		VSIw->parms[parmptr] = -1;
	VSIw->parmptr = 0;
  } /* VSIapclear */

void VSIsetoption
  (
	short toggle /* 1 => set, 0 => reset */
  )
  /* sets/resets various options, as specified by the parms in
	the current control sequence. Note that this implementation
	will not set/reset more than one option at a time! */
  {
	short
		WindWidth = VSIw->Rright - VSIw->Rleft;

	switch (VSIw->parms[0])
	  {
		case -2: /* DEC-private control sequence */
			switch (VSIw->parms[1])
			  {
				case 1: /* cursor-key mode */
					VSIw->DECCKM = toggle;
					break;
				case 3: /* 80/132 columns */
					VSIw->x = VSIw->y = 0; /* home cursor */
					VSIes(); /* and clear screen */
					if (toggle)	/* 132 column mode */
						{												/* NCSA: SB */
						VSIw->maxwidth = 131;							/* NCSA: SB */
						RScalcwsize(VSIwn,132);							/* NCSA: SB */
						}												/* NCSA: SB */
					else												/* NCSA: SB */
						{												/* NCSA: SB */
						VSIw->maxwidth = 79;							/* NCSA: SB */
						RScalcwsize(VSIwn,80);							/* NCSA: SB */
						}												/* NCSA: SB */
				  /* update scroll bars */									
					RSmargininfo(VSIwn, VSIw->maxwidth, VSIw->Rleft);	/* NCSA: SB */	
					break;
					
/* NCSA: SB -  this next one will allow us to flip the foreground and		*/
/*		background colors.													*/
				case 5:											/* NCSA: SB - screen mode */
					RSbackground(VSIwn,toggle);
					break;
				
				case 6: /* origin mode */
					VSIw->DECORG = toggle;
					break;
				case 7: /* autowrap mode */
					VSIw->DECAWM = toggle;
					break;
				default:
					break;
			  } /* switch */
			break;
		case  4: /* insert/replace character writing mode */
			VSIw->IRM = toggle;
			break;
		default:
			break;
	  } /* switch */
  } /* VSIsetoption */

short VSItab
  (
	void
  )
  /* advances VSIw->x to the next tab position. */
  {
	if (VSIw->x >= VSIw->maxwidth)
	  {
	  /* already at right margin */
		VSIw->x = VSIw->maxwidth;
		return(0);
	  } /* if */
	VSIw->x++; /* advance at least one position */
	while ((VSIw->tabs[VSIw->x] != 'x') && (VSIw->x < VSIw->maxwidth))
		VSIw->x++;
	return(0);
  } /* VSItab */

void VSIinschar
  (
	short x /* number of blanks to insert */
  )
  /* inserts the specified number of blank characters at the
	current cursor position, moving the rest of the line along,
	losing any characters that fall off the right margin.
	Does not update the display. */
  {
	short i, j;
	char *tempa, *temp;

	VSIwrapnow(&i, &j);

	tempa = VSIw->attrst[VSIw->y]->text;
	temp = VSIw->linest[VSIw->y]->text;
	for (i = VSIw->maxwidth - x; i >= VSIw->x; i--)
	  {
	  /* move along remaining characters on line */
		temp[x + i] =temp[i];
		tempa[x + i] = tempa[i];
	  } /* for */
	for (i = VSIw->x; i < VSIw->x + x; i++)
	  {
	  /* insert appropriate number of blanks */
		temp[i] = ' ';
		tempa[i] = VSIclrattrib;
	  } /* for */
  } /* VSIinschar */

void VSIinsstring
  (
	short len,
	char *start
  )
  /* updates the screen to show insertion of a string of characters
	at the current cursor position. The text has already been
	inserted into the screen buffer. Also, the cursor position has
	already been updated, so the part needing redrawing begins at column
	(VSIw->x - len). */
  {
	if (VSIw->VSIDC)
		RSinsstring(VSIwn, VSIw->x - len, VSIw->y,
			VSIw->attrib, len, start);
	else
	  /* redraw from beginning of text insertion to end of line.
		Trouble is, this isn't going to preserve the right attributes. */
		RSdraw(VSIwn, VSIw->x - len, VSIw->y, VSIw->attrib,
			VSIw->maxwidth - VSIw->x + len + 1, start);
  } /* VSIinsstring */

void VSIsave
  (
	void
  )
  /* saves the current cursor position and attribute settings. */
  {
	VSIw->Px = VSIw->x;
	VSIw->Py = VSIw->y;
	VSIw->Pattrib = VSIw->attrib;
  } /* VSIsave */

void VSIrestore
  (
	void
  )
  /* restores the last-saved cursor position and attribute settings. */
  {
	if (VSIw->Pattrib < 0)
	  /* no previous save */
		return;
		
	VSIw->x = VSIw->Px;
	VSIw->y = VSIw->Py;
	VSIrange();
	VSIw->attrib = VSinattr(VSIw->Pattrib); /* hmm, this will clear the graphics character set selection */
  } /* VSIrestore */

void VSIdraw
  (
	short VSIwn, /* window number */
	short x, /* starting column */
	short y, /* line on which to draw */
	short a, /* text attributes */
	short len, /* length of text to draw */
	char *c /* pointer to text */
  )
  /* displays a piece of text (assumed to fit on a single line) on a
	screen, using the specified attributes, and clipping to the
	current visible region. Highlights any part of the text lying
	within the current selection. */
  {
	short x2, y2, offset;

	if (!VSIclip(&x, &y, &x2, &y2, &len, &offset))
		RSdraw(VSIwn, x, y, a, len, (char *) (c + offset));	/* BYU LSC */
  } /* VSIdraw */

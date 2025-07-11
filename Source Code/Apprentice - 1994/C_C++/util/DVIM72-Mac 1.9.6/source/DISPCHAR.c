/* -*-C-*- dispchar.h */
/*-->dispchar*/
/**********************************************************************/
/****************************** dispchar ******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "mac-specific.h"
#include "Scale_rect.h"

void
dispchar(c)		// called by setchar
BYTE c;		/* character number in current font */

/***********************************************************************

   This procedure has the delicate  job of OR'ing the current  character
   raster description into the bitmap,  where the character box  extends
   horizontally from  (xcorner  ..  xcorner+wp-1)  and  vertically  from
   (ycorner  ..   ycorner+hp-1).   The  lower  left  corner  coordinates
   (xcorner, ycorner) are  related to  the current point  (xcp, ycp)  as
   follows:

	<------wp------>
    ^	................
    |	................
    |	................
    |	................
    |	................
    |	................
    |	................
    hp  ................
    |	................
    |	................
    |	................
    |	.....o..........       <-- (xcp,ycp) at "o"
    |	................ ^
    |	................ |
    |	................ |--- (hp - yoffp - 1)
    |	................ |
    v	+............... v     <-- (xcorner,ycorner) at "+"
	<--->
	  |
	  |
	 xoffp

   The current PXL file format stores character rasters in 32-bit words,
   with the rightmost portion of the  last word in each line beyond  the
   edge of  the character  being all  0 bits.	For efficiency,  the  OR
   operation is done a word  at a time, and  in general, each such  word
   contributes to two words in the bitmap line.

	     line		    line		   line
   |.........word.........|.........word.........|.........word.........|

  32-bit chunks--> |.....chrast.....|.....chrast.....|.....chrast.....|

		   |<---->|<------->|
		       |       |
     bits_to_current---^       ^--- bits_to_next

   Thus, each  32-bit  chunk  will  be  right-shifted  (filling  vacated
   positions at the left with 0 bits) leaving "bits_to_current" bits  at
   the low end and then OR'd into the current word of the bitmap line.

   Since the C language  right-shift operator may  or may not  propagate
   the sign bit  (which is usually  at the left),  a compile-time  flag,
   ARITHRSHIFT, is necessary to include an extra AND operation to remove
   them  when	the  right-shift   is  implemented   by  an   arithmetic
   (sign-propagating), rather than a logical, shift.

   The 32-bit  chunk will  then  be left-shifted  (with 0  bits  filling
   vacated positions on  the right) leaving  "bits_to_next" bits at  the
   high end and OR'd into the next word of the bitmap line.

   When the host word  size exceeds 32 bits  (e.g. DEC-10 or -20  36-bit
   word), the  first step  may in  fact require  a left  shift, and  the
   second step is then not needed.   This is detected in the code  below
   by "bits_to_next" being negative.

***********************************************************************/
{
    register struct char_entry *tcharptr;
    COORDINATE x,xcorner,ycorner;
    UNSIGN16 ilimit;
    register INT16 bits_to_next;
    register UNSIGN16 i;
    UNSIGN32 word32;
    register UNSIGN32 *p;
    register UNSIGN32 *raster_word;
    register COORDINATE j;
#if OS_THINKC
#if PIXMAP
	PixMap	char_map;
#else
	BitMap	char_map;
#endif
	Rect	dest, print_dest, preview_dest;
#endif

    if ((c < FIRSTPXLCHAR) || (LASTPXLCHAR < c)) /* check character range */
	return;

    tcharptr = &(fontptr->ch[c]);

    if (tcharptr->rasters == (UNSIGN32*)NULL)/* if rasters still on file */
		loadchar(c);			/* go get them */
	if (g_abort_dvi)
		return;

    if (tcharptr->rasters == (UNSIGN32*)NULL)
	return;	/* character image must be empty */

    tcharptr->refcount++;		/* update reference count */
    raster_word = tcharptr->rasters;	/* pointer to first raster word */

    xcorner = xcp - tcharptr->xoffp;
    ycorner = ycp - (tcharptr->hp - tcharptr->yoffp - 1);

    if (DBGOPT(DBG_CHAR_DUMP))
    {
        (void)printf(
	    "\ndispchar(): (xcp,ycp) = (%d,%d) (xcorner,ycorner) = (%d,%d)",
	    xcp,ycp,xcorner,ycorner);
	(void)printf("\n            (wp,hp) = (%d,%d)  (xoffp,yoffp) = (%d,%d)\n",
	    tcharptr->wp,tcharptr->hp,tcharptr->xoffp,tcharptr->yoffp);
		ilimit = (UNSIGN16)((tcharptr->wp + 31) >> 5);
		for (j = tcharptr->hp; j > 0; --j)
		{
	 	   for (i = 0; i < ilimit; ++i)
	  	      (void)printf(" %08lx\n",*raster_word++);
		}
		raster_word = tcharptr->rasters;
    }
#if OS_THINKC /* New */
	char_map.baseAddr = (Ptr) tcharptr->rasters;
	char_map.rowBytes = ((tcharptr->wp + 31) >> 5) << 2;
	SetRect( &char_map.bounds, 0, 0, tcharptr->wp, tcharptr->hp );

#if PIXMAP
	char_map.rowBytes |= 0x8000;
	char_map.pmVersion = 0;
	char_map.packType = 0;
	char_map.packSize = 0;
	char_map.hRes = char_map.vRes = ((Fixed)g_dpi) << 16;
	char_map.pixelType = 0;
	char_map.pixelSize = 1;
	char_map.cmpCount = 1;
	char_map.cmpSize = 1;
	char_map.planeBytes = 0L;
	char_map.pmTable = GetCTable(1);
	char_map.pmReserved = 0L;
#endif
	
	SetRect( &dest, xcorner,
		YSIZE - (ycorner + tcharptr->hp),
		xcorner + tcharptr->wp,
		YSIZE - ycorner );
	if (g_draw_offscreen)
	{
		CopyBits( (BitMap *)&char_map, &g_offscreen_GrafPort.portBits,
			&char_map.bounds, &dest, srcOr, NIL );
	}
	else
	{
		Scale_rect( &dest, &print_dest, &preview_dest );
		if (g_preview)
		{
			SetPort( g_page_window );
			/* We use srcCopy instead of srcOr here for speed. */
			CopyBits( (BitMap *)&char_map, &g_page_window->portBits,
				&char_map.bounds, &preview_dest, srcCopy, NIL );
		}
		SetPort( (GrafPtr) g_print_port_p );
		CopyBits( (BitMap *)&char_map, &g_print_port_p->gPort.portBits,
			&char_map.bounds, &print_dest, srcOr, nil );
	}
#else
#if OS_THINKC
    for (j = tcharptr->hp; j > 0; --j)	/* loop over hp rasters from */
    {					/* top to bottom */
	x = xcorner;			/* select horizontal position */
	p = BITMAP(ycorner+1 - j,x/HOST_WORD_SIZE); /* and find word on line */
#else
    for (j = tcharptr->hp; j > 0; --j)	/* loop over hp rasters from */
    {					/* top to bottom */
	x = xcorner;			/* select horizontal position */
	p = BITMAP(ycorner+j-1,x/HOST_WORD_SIZE); /* and find word on line */
#endif
	ilimit = (UNSIGN16)((tcharptr->wp + 31) >> 5);
	for (i = 0; i < ilimit; ++i)
	{		    /* loop over current line */
	    word32 = *raster_word++; /* get 32-bit portion of raster */
	    bits_to_next = (INT16)((x % HOST_WORD_SIZE) - HOST_WORD_SIZE + 32);

#if    (HOST_WORD_SIZE > 32)
	    if (bits_to_next < 0)   /* then must left shift character raster */
		*p |= (word32 << (-bits_to_next));  /* and OR into line */
	    else
#endif

	    {
		*p |=

		      (word32 >> bits_to_next);     /* OR into line */

		if (bits_to_next > 0)
		    *++p |= (word32 << (HOST_WORD_SIZE - bits_to_next));
			    /* OR in any spill into next word */
		else if (bits_to_next == 0)
		    ++p;    /* ended at word boundary, so start new one */
	    }
	    x += 32;	    /* and update horizontal position */
	}
    }
#endif /* OS_THINKC New */
}


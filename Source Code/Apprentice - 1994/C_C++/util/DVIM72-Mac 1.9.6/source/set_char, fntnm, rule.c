/**********************************************************************/
/****************************** setchar *******************************/
/**********************************************************************/
#include <ctype.h>
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "m72.h"
#include "egblvars.h"
#include "mac-specific.h"

void
setchar(c, update_h)
register BYTE c;
register BOOLEAN update_h;
{
    register struct char_entry *tcharptr;  /* temporary char_entry pointer */

    if (DBGOPT(DBG_SET_TEXT))
    {
	(void)printf("setchar('");
	if (isprint(c))
	    (void)printf("%c",c);
	else
	    (void)printf("\\%03o",(int)c);
	(void)printf("'<%d>) (hh,vv) = (%ld,%ld) font name <%s>",
	    (int)c, (long)hh, (long)vv, fontptr->n);
	printf("\n");
    }

    tcharptr = &(fontptr->ch[c]);
    if (((hh - tcharptr->xoffp + tcharptr->pxlw) <= XSIZE)
	&& (hh >= 0)
	&& (vv <= YSIZE)
	&& (vv >= 0))
    {			    /* character fits entirely on page */
		moveto( hh, (COORDINATE)(YSIZE-vv));
		dispchar(c);
		if (g_abort_dvi)
			return;
    }
    else if (DBGOPT(DBG_OFF_PAGE) && !quiet)
    {				/* character is off page -- discard it */
	(void)printf(
	    "setchar(): Char %c [10#%3d 8#%03o 16#%02x] off page.",
	    isprint(c) ? c : '?',c,c,c);
	printf("\n");
    }

    if (update_h)
    {
	hxxxx += (INT32)tcharptr->tfmw;
	hh += (COORDINATE)tcharptr->pxlw;
	hh = (COORDINATE)(fixpos(hh-lmargin,hxxxx,conv) + lmargin);
    }
}

/**********************************************************************/
/****************************** setfntnm ******************************/
/**********************************************************************/
void
setfntnm(k)
register INT32 k;

/***********************************************************************
This routine is used to specify the  font to be used in printing  future
characters.
***********************************************************************/

{
    register struct font_entry *p;

    p = hfontptr;
    while ((p != (struct font_entry *)NULL) && (p->k != k))
		p = p->next;
    if (p == (struct font_entry *)NULL)
    {
		(void)sprintf(message,"setfntnm():  font %ld undefined", k);
		Kill_dvi(message);
    }
    else
		fontptr = p;

#if    (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2)
    font_switched = TRUE;
#endif

}

/**********************************************************************/
/****************************** setrule *******************************/
/**********************************************************************/
void
setrule(height, width, update_h)
register UNSIGN32 height, width;
register BOOLEAN update_h;

{   /* draw a rule with bottom left corner at (h,v) */

    if ((height > 0) && (width > 0))		/* non-empty rule */

#if    BBNBITGRAPH
	fillrect(hh+xscreen, YSIZE-vv+yscreen,
	    rulepxl(width,conv)  ,  rulepxl(height,conv));
#else
	fillrect(hh, YSIZE-vv,
	    rulepxl(width,conv), rulepxl(height,conv));
#endif

    if (update_h)
    {
	hxxxx += (INT32)width;
	hh += rulepxl(width, conv);
	hh = fixpos(hh-lmargin,hxxxx,conv) + lmargin;
    }
}

/**********************************************************************/
/****************************** getpgtab ******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "mac-specific.h"

void
getpgtab(lastpageptr)	// called by readpost
long lastpageptr;

{
    register long p;
    register INT16 i,k;

    (void) FSEEK (dvifp,lastpageptr,0);
    p = lastpageptr;

    for (k = 0; (p != (-1)) && (k < MAXPAGE); ++k)
    {
        page_ptr[MAXPAGE-1-k] = p;
        (void) FSEEK (dvifp,(long) p, 0);

        if ((BYTE)nosignex(dvifp,(BYTE)1) != BOP)
        {
        	Kill_dvi( "getpgtab():  Invalid BOP "
        		"(beginning-of-page) back chain" );
        	return;
        }
 
        for (i = 0; i <= 9; ++i)
            (void) nosignex(dvifp,(BYTE)4);   /* discard count0..count9 */
        p = (long)signex(dvifp,(BYTE)4);
    }
    page_count = k;
    if (k >= MAXPAGE)
        (void)warning("getpgtab():  Page table full...rebuild driver with \
larger MAXPAGE");
    else	/* move pointer table to front of array */
        for (k = 0; k < page_count; ++k)
	    page_ptr[k] = page_ptr[MAXPAGE-page_count+k];
}

/**********************************************************************/
/****************************** getbytes ******************************/
/**********************************************************************/

void
getbytes(fp, cp, n)	/* get n bytes from file fp */
register FILE *fp;	/* file pointer	 */
register char *cp;	/* character pointer */
register BYTE n;	/* number of bytes */

{
    while (n--)
	*cp++ = (char)getc(fp);
}

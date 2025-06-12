/* -*-C-*- skgfspec.h */
/*-->skgfspec*/
/**********************************************************************/
/****************************** skgfspec ******************************/
/************************#include "dvihead.h"**************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "m72.h"
#include "egblvars.h"

void
skgfspec()	/* Skip GF font file specials */
{
    BYTE the_byte;

    the_byte = (BYTE)nosignex(fontfp,(BYTE)1);
    while ((the_byte >= (BYTE)GFXXX1) && (the_byte != GFPOST))
    {
	switch(the_byte)
	{
	case GFXXX1:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)1),1);
	    break;

	case GFXXX2:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)2),1);
	    break;

	case GFXXX3:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)3),1);
	    break;

	case GFXXX4:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)4),1);
	    break;

	case GFYYY:
	    (void)nosignex(fontfp,(BYTE)4);
	    break;

	case GFNOOP:
	    break;

	default:
	    (void)sprintf(message,"skgfspec():  Bad GF font file [%s]",
		fontptr->name);
	    (void)fatal(message);
	}
	the_byte = (BYTE)nosignex(fontfp,(BYTE)1);
    }
    (void)UNGETC((char)the_byte,fontfp);	/* put back lookahead byte */
}

/**********************************************************************/
/****************************** skipfont ******************************/
/**********************************************************************/
void
skipfont(k)
INT32 k;				/* UNUSED */

{
    BYTE a, l;
    char n[MAXSTR];

    (void) nosignex(dvifp,(BYTE)4);
    (void) nosignex(dvifp,(BYTE)4);
    (void) nosignex(dvifp,(BYTE)4);
    a = (BYTE) nosignex(dvifp,(BYTE)1);
    l = (BYTE) nosignex(dvifp,(BYTE)1);
    getbytes(dvifp, n, (BYTE)(a+l));
}


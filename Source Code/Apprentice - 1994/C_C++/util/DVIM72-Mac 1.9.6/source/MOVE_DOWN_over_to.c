/* -*-C-*- movedown.h */
/*-->movedown*/
/**********************************************************************/
/****************************** movedown ******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"

void
movedown(a)
register INT32 a;

{
    /*	From DVITYPE Version 2.6:
	"Vertical motion is done similarly [to horizontal motion handled
	in moveover()],  but with  the threshold  between ``small''  and
	``large'' increased by  a factor of  five. The idea  is to  make
	fractions  like  ``1/2''  round  consistently,  but  to   absorb
	accumulated rounding errors in the baseline-skip moves."

	The one precaution we need to  take here is that fontptr can  be
	NULL, which we treat like  a large movement.  This NULL  pointer
	was used without error  on many different  machines for 2  years
	before it was caught on the VAX VMS implementation, which  makes
	memory page 0 inaccessible.
    */
    vxxxx += a;
    if ((fontptr == (struct font_entry *)NULL) ||
        (ABS(a) >= 5*fontptr->font_space))
	vv = PIXROUND(vxxxx, conv) + tmargin;
    else
    {
	vv += PIXROUND(a, conv);
	vv = fixpos(vv-tmargin,vxxxx,conv) + tmargin;
    }
}

/**********************************************************************/
/****************************** moveover ******************************/
/**********************************************************************/
void
moveover(b)
register INT32 b;

{
    /*	From DVITYPE Version 2.6:
	"Rounding to the nearest pixel is  best done in the manner shown
	here, so as to  be inoffensive to the  eye: When the  horizontal
	motion is small, like a kern, |hh| changes by rounding the kern;
	but when the motion is large, |hh| changes by rounding the  true
	position |h| so that  accumulated rounding errors disappear.  We
	allow a  larger space  in  the negative  direction than  in  the
	positive one, because TeX  makes comparatively large  backspaces
	when it positions accents."

	The one precaution we need to  take here is that fontptr can  be
	NULL, which we treat like  a large movement.  This NULL  pointer
	was used without error  on many different  machines for 2  years
	before it was caught on the VAX VMS implementation, which  makes
	memory page 0 inaccessible.
    */
    hxxxx += b;
    if ((fontptr == (struct font_entry *)NULL) ||
	ABS(b) >= fontptr->font_space)
	hh = PIXROUND(hxxxx, conv) + lmargin;
    else
    {
	hh += PIXROUND(b, conv);
	hh = fixpos(hh-lmargin,hxxxx,conv) + lmargin;
    }
}

/**********************************************************************/
/******************************* moveto *******************************/
/**********************************************************************/

void
moveto(x,y)
COORDINATE x,y;

{
    xcp = x;
    ycp = y;
}

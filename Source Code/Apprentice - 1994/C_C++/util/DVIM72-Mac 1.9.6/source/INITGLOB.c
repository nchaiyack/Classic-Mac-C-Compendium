/* -*-C-*- initglob.h */
/*-->initglob*/
/**********************************************************************/
/****************************** initglob ******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "math.h"
#include "mac-specific.h"


void
initglob()			/* initialize global variables */
{
    register INT16 i;		/* loop index */
    register char* tcp;		/* temporary character pointer */
    float halfmag;			/* temp used to initialize mag_table */

/***********************************************************************
    Set up masks such that rightones[k]  has 1-bits at the right end  of
    the word from k ..	 HOST_WORD_SIZE-1, where bits are numbered  from
    left (high-order) to right (lower-order), 0 .. HOST_WORD_SIZE-1.

    img_mask[k] has  a 1-bit  in  position k,  bits numbered  as  above.
    power[k] is  1  <<  k,  and gpower[k]  is  2**k-1  (i.e.  1-bits  in
    low-order k positions).  These 3 require only 32 entries each  since
    they deal with 32-bit words from the PK and GF font files.

    These are set  at run-time, rather  than compile time  to avoid  (a)
    problems with C  preprocessors which sometimes  do not handle  large
    constants correctly, and (b) host byte-order dependence.
***********************************************************************/

    rightones[HOST_WORD_SIZE-1] = 1;
    for (i = HOST_WORD_SIZE-2; (i >= 0); --i)
	rightones[i] = (rightones[i+1] << 1) | 1;

    img_mask[31] = 1;
    for (i = 30; i >= 0; --i)
	img_mask[i] = img_mask[i+1] << 1;

    power[0] = 1;
    for (i = 1; i <= 31; ++i)
	power[i] = power[i-1] << 1;

    gpower[0] = 0;		/* NB: we have gpower[0..32], not [0..31] */
    for (i = 1; i <= 32; ++i)
	gpower[i] = power[i-1] | gpower[i-1];

    debug_code = 0;

	basemag = g_dpi * 5;
    runmag = basemag;

    npage = 0;
    copies = 1;				/* number of copies of each page */
    topmargin = 1.0;			/* DVI driver standard default */
    leftmargin = 1.0;			/* DVI driver standard default */

    subfile[0] = '\0';

#if    VIRTUAL_FONTS
    virt_font = FALSE;
#endif

    /*
    Copy default file fields into global  variables, then replace by any
    runtime environment  variable definitions.  We need not  do this for
    TOPS-20   and  VAX  VMS,  since   SUBPATH and  FONTPATH are  already
    initialized to logical  names  which   the  operating  system   will
    translate at file open time.
    */

    (void)strcpy(helpcmd,DVIHELP);
    (void)strcpy(subpath,SUBPATH);
    (void)strcpy(subname,SUBNAME);
    (void)strcpy(subext,SUBEXT);
    (void)strcpy(fontpath,FONTPATH);
    (void)strcpy(fontlist,FONTLIST);

    if ((tcp = GETENV("DVIHELP")) != (char *)NULL)
	(void)strcpy(helpcmd,tcp);

#if    (OS_ATARI | OS_PCDOS | OS_UNIX | OS_THINKC)
    if ((tcp = GETENV(TEXFONTS)) != (char *)NULL)
    {
		(void)strcpy(subpath,tcp);
		(void)strcpy(fontpath,tcp);
    }
#endif

    if ((tcp = GETENV("FONTLIST")) != (char *)NULL)
	(void)strcpy(fontlist,tcp);

/* Initialize mag_table */
	mag_table[16] = 1.0;
	halfmag = (float)sqrt(1.2);
	for (i = 17; i < 33; i++)
		mag_table[i] = halfmag * mag_table[i-1];
	for (i = 15; i >= 0; i--)
		mag_table[i] = mag_table[i+1] / halfmag;
}

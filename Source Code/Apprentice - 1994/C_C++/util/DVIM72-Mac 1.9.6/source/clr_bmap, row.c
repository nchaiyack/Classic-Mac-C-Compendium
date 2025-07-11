#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"

/**********************************************************************/
/****************************** clrrow ********************************/
/**********************************************************************/
/* called by chargf and charpk */

void
clrrow()	/* clear the current character image row, img_row[] */
{
    register UNSIGN32 *p;
    register UNSIGN16 nwords;

    nwords = img_words;
    for (p = &img_row[nwords-1]; nwords; (--p,--nwords))
        *p = (UNSIGN32)0L;
}

/* lzw.c -- compress files in LZW format.
 * This is a dummy version avoiding patent problems.
 */

/*
 * Modified:1993 by SPDsoft for MacGzip
 *
 */


#ifdef RCSID
static char rcsid[] = "$Id: lzw.c,v 0.9 1993/06/10 13:27:31 jloup Exp $";
#endif

#include "tailor.h"
#include "gzip.h"
#include "lzw.h"

#include "MacErrors.h"

static int msg_done = 0;

/* Compress in to out with lzw method. */
int lzw(in, out)
    int in, out;
{
    if (msg_done) return ERROR;
    msg_done = 1;
    sprintf(strerr,"output in compress .Z format not supported");
    Calert(strerr);
    if (in != out) { /* avoid warnings on unused variables */
        exit_code = ERROR;
    }
    return ERROR;
}

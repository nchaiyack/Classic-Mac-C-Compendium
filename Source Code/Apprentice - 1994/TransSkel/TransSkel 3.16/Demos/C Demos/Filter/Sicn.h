/*
 * Sicn.h	- stuff for messing with SICN resources.
 *
 * A SicnHandle is a handle to a SICN resource, which consists of a set
 * of SICN items (i.e., it's really an array).  A Sitm is a single SICN
 * item.
 */

# define	sitmSize	16    /* SICN items are 16 bits wide x 16 words down */


typedef short	Sitm[sitmSize];
typedef Sitm	Sicn[1];	/* actually variable-length */
typedef Sicn	*SicnPtr, **SicnHandle;

void PlotSitm (Rect *r, Sitm *s);
void PlotSicn (Rect *r, SicnHandle s, short i);

/* Return the name of the boot volume (not the current directory).
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).
*/

#include "macdefs.h"

char *
getbootvol()
{
	short vrefnum;
	static char name[32];
	
	(void) GetVol(name, &vrefnum);
		/* Shouldn't fail; return ":" if it does */
	strcat(name, ":");
	return name;
}

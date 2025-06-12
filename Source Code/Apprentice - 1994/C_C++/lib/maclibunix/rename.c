/* Rename emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

#include "macdefs.h"

int
rename(old, new)
	char *old, *new;
{
	if (Rename(old, 0, new) == noErr)
		return 0;
	errno= EPERM;
	return -1;
}

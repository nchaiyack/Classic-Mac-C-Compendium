/* Function to convert a C string to a Pascal string.
   The conversion is not in-line, but returns a pointer to a static buffer.
   This is needed when calling some toolbox routines.
   MPW does the conversion in the glue.
*/

#include "macwin.h"

#ifndef CLEVERGLUE

char *
PSTRING(src)
	register char *src;
{
	static char buf[256];
	register char *dst;
	
	dst = &buf[1];
	while ((*dst++ = *src++) != '\0' && dst < &buf[256])
		;
	buf[0] = dst - &buf[1] - 1; /* XXX Recent bugfix! */
	return buf;
}

#endif /* CLEVERGLUE */

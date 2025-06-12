/* Perror emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

#include <stdio.h>
#include <errno.h>

perror(str)
	char *str;
{
	if (str != NULL)
		fprintf(stderr, "%s: ", str);
	fprintf(stderr, "Error %d\n", errno);
}

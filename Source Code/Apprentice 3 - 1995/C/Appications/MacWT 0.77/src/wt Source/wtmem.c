/*
**  MacWT -- a 3d game engine for the Macintosh
**  © 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/


#include <stdlib.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"


/* malloc or die . . . */
void *wtmalloc(size_t size)
{
	void *new;

	if ((new = malloc(size)) == NULL)
		fatal_error("could not allocate %x bytes", size);

	return new;
}


/* realloc or die . . . */
void *wtrealloc(void *v, size_t size)
{
	void *new;

	if ((new = realloc(v, size)) == NULL)
		fatal_error("could not reallocate %x bytes", size);

	return new;
}


void wtfree(void *v)
{
	if (v == NULL)
		fatal_error("wtfree(NULL)");
	else
		free(v);
}

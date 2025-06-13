/*
**  MacWT -- a 3d game engine for the Macintosh
**  © 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: gopher.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "color.h"
#include "graphfile.h"


static void skip_cruft(FILE *fp);


Graphic_file *LoadPPM(FILE *fp, char *filename)
{
     int width, height;
     int unknown;
     Graphic_file *gfile;


     skip_cruft(fp);
     if (getc(fp) != 'P' || getc(fp) != '6')
	  fatal_error("%s is not a ppm file.", filename);

     skip_cruft(fp);
     if (fscanf(fp, "%d", &width) != 1)
	  fatal_error("%s: bad ppm file.", filename);
     skip_cruft(fp);
     if (fscanf(fp, "%d", &height) != 1)
	  fatal_error("%s: bad ppm file.", filename);
     skip_cruft(fp);
     if (fscanf(fp, "%d\n", &unknown) != 1)
	  fatal_error("%s: bad ppm file.", filename);

     gfile = new_graphic_file();
     gfile->type = gfTrueColor;
     gfile->palette = NULL;
     gfile->width = width;
     gfile->height = height;
     gfile->bitmap = wtmalloc(height * width * 3);

     if (fread(gfile->bitmap, height * width * 3, 1, fp) != 1)
	  fatal_error("%s: incomplete data", filename);

     return gfile;
}


/* Skip white space and comments. */
static void skip_cruft(FILE *fp)
{
	int c;

	c = getc(fp);
	for (;;)
		{
		if (c == '#')
			{
			while (c != '\n')
				c = getc(fp);
			}
		else if (c == EOF)
	       return;
		else if (!isspace(c))
			{
			ungetc(c, fp);
			return;
			}
		else
	       c = getc(fp);
		}
}

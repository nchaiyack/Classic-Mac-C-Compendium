/*
**  MacWT -- a 3d game engine for the Macintosh
**  � 1995, Bill Hayden and Nikol Software
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



#include <stdio.h>
#include <string.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "color.h"
#include "graphfile.h"


/* MAGIC_LENGTH must be the length of the longest MAGIC string. */
#define MAGIC_LENGTH 5
#define GIF_MAGIC "GIF87"
#define GIF89_MAGIC "GIF89"


extern Graphic_file *LoadGIF(FILE *fp, char *filename);

static Graphic_file_format check_format(FILE *fp, char *filename);


Graphic_file *new_graphic_file()
{
     Graphic_file *gf = wtmalloc(sizeof(Graphic_file));

     gf->transparent_entry = -1;

     return gf;
}


void free_graphic_file(Graphic_file *gfile)
{
     if (gfile != NULL) {
	  if (gfile->palette != NULL)
	       wtfree(gfile->palette);
	  if (gfile->bitmap != NULL)
	       wtfree(gfile->bitmap);
     }
     wtfree(gfile);
}


/* Return false if the pixel is transparent, true if it is opaque.  In any
**   case, stuff dest with the pixel RGB value.
*/
Boolean graphic_file_pixel(Graphic_file *gfile, int x, int y, RGBcolor *rgb)
{
	if (gfile->type == gfPaletted)
		{
		int index;

		index = gfile->bitmap[y * gfile->width + x];
		if (index == gfile->transparent_entry)
			return False;
		else
			{
			*rgb = gfile->palette[index];
			return True;
			}
		}
	else
		{
		unsigned char *pixel = gfile->bitmap + (y * gfile->width + x) * 3;

		rgb->red = *pixel++;
		rgb->green = *pixel++;
		rgb->blue = *pixel;

	  /* For now, let pure cyan be the transparent color for true color images. */
		if (rgb->red == 0 && rgb->green == 255 && rgb->blue == 255)
			return False;
		else
			return True;
		}
}

	  
Graphic_file *read_graphic_file(char *filename)
{
	FILE *fp;
	Graphic_file_format format;
	Graphic_file *gfile = NULL;


	if ((fp = fopen(filename, "rb")) == NULL)
		fatal_error("Could not open texture %s", filename);

	format = check_format(fp, filename);
	rewind(fp);

	switch (format)
		{
		case formatGIF89:
		case formatGIF87:
			gfile = LoadGIF(fp, filename);
			break;

		case formatPICTR:
			gfile = LoadPICTR(fp, filename);
			break;

		case formatUnknown:
			fatal_error("Unknown graphic file format.\n");
			break;

		default:
			fatal_error("The graphic file reading code is really broken.\n");
			break;
		}

	if (gfile == NULL)
		fatal_error("Error reading texture %s\n");

	fclose(fp);

	return gfile;
}
	  

static Graphic_file_format check_format(FILE *fp, char *filename)
{
	char magic[MAGIC_LENGTH];

#if __MWERKS__ || __THINKC__
	// to read PICT or PICTR files, which are further along on the
	// evolutionary chain than to need MagicCookies or the like.
	fread(magic, 1, MAGIC_LENGTH, fp);
#else
	if (fread(magic, 1, MAGIC_LENGTH, fp) != MAGIC_LENGTH)
		fatal_error("Error reading texture %s.", filename);
#endif

	if (strncmp(magic, GIF_MAGIC, sizeof(GIF_MAGIC) - 1) == 0)
		return formatGIF87;
	else if (strncmp(magic, GIF89_MAGIC, sizeof(GIF89_MAGIC) - 1) == 0)
		return formatGIF89;
	else
		return formatPICTR;
		
	// return formatUnknown;
}

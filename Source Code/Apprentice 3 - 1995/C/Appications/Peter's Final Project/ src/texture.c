/*
 *  Peter's Final Project -- A texture mapping demonstration
 *  © 1995, Peter Mattis
 *
 *  E-mail:
 *  petm@soda.csua.berkeley.edu
 *
 *  Snail-mail:
 *   Peter Mattis
 *   557 Fort Laramie Dr.
 *   Sunnyvale, CA 94087
 *
 *  Avaible from:
 *  http://www.csua.berkeley.edu/~petm/final.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  Parts of this module are code excerpted from the Independent JPEG
 *  Group's software. Specifically the function "texture_read" is a nearly
 *  verbatim copy of the function "read_JPEG_file" located in the file
 *  "example.c" in the March 15, 1995 release of the libjpeg library.
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <jpeglib.h>

#include "sys.stuff.h"
#include "texture.h"
#include "utils.h"

/*
 * Declare the functions private to this module.
 */

static void texture_put_scanline (TEXTURE, JSAMPROW, long);

/*
 * Make a camera object and initialize its values to something decent.
 */

TEXTURE
make_texture ()
{
	TEXTURE texture;

	texture = (TEXTURE) ALLOC (sizeof (_TEXTURE));
	assert (texture != NULL);

	set_texture_address (texture, NULL);
	set_texture_size (texture, 0);
	set_texture_size_log2 (texture, 0);

	return texture;
}

/*
 * Free a texture.
 */

void
free_texture (t)
	TEXTURE t;
{
	if (t)
		FREE (texture_address (t));
	FREE (t);
}

/*
 * Initialize the texture to a checker board pattern.
 * (Used mainly for debugging since it show texture
 *  mapping problems very nicely.)
 */

void
texture_checker (t, size)
	TEXTURE t;
	short size;
{
	short i, j;
	long pixel_size;
	long c1, c2;

	assert (t != NULL);

	pixel_size = get_frame_buffer_pixel ();

	switch (get_frame_buffer_depth ())
	{
	case 8:
		c1 = 100;
		c2 = 200;
		break;
	case 24:
		c1 = 0x00FF0000;
		c2 = 0x000000FF;
		break;
	}

	set_texture_size (t, size);
	set_texture_size_log2 (t, (short) ((float) (log (texture_size (t))) /log (2)));
	set_texture_bytes_per_texel (t, pixel_size);
	set_texture_address (t, ALLOC (pixel_size * texture_size (t) * texture_size (t)));
	assert (texture_address (t) != NULL);

	for (i = 0; i < texture_size (t); i++)
		for (j = 0; j < texture_size (t); j++)
			switch (pixel_size)
			{
			case 1:
				set_texel8 (t, i, j, ((i + j) % 2) ? c1 : c2);
				break;
			case 2:
				set_texel16 (t, i, j, ((i + j) % 2) ? c1 : c2);
				break;
			case 4:
				set_texel32 (t, i, j, ((i + j) % 2) ? c1 : c2);
				break;
			}
}

/*
 * Initialize a texture from a jpeg file.
 *
 * The following routine was taken (almost) entirely from the
 *  "example.c" file distributed with the "libjpeg" library. That is
 *  to say, the following code is excerpted from the Independent
 *  JPEG Group's software.
 *
 * Thank you Independent JPEG Group for writing an amazingly easy
 *  to use and portable library for reading and writing JPEG files.
 */

void
texture_read (t, file_name)
	TEXTURE t;
	char *file_name;
{
	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler. */
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE *infile; /* source file */
	JSAMPARRAY buffer; /* Output row buffer */
	int row_stride; /* physical row width in output buffer */
	long pixel_size;

	/* In this example we want to open the input file before doing anything else,
	 * so that the setjmp() error recovery below can assume the file is open.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to read binary files.
	 */

	if ((infile = fopen (file_name, "rb")) == NULL)
	{
		fprintf (stderr, "can't open %s\n", file_name);
		exit (0);
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines. */
	cinfo.err = jpeg_std_error (&jerr);

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress (&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src (&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header (&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	 * jpeg_read_header(), so we do nothing here.
	 */

	/* Step 5: Start decompressor */

	jpeg_start_decompress (&cinfo);

	/* We may need to do some setup of our own at this point before reading
	 * the data.  After jpeg_start_decompress() we have the correct scaled
	 * output image dimensions available, as well as the output colormap
	 * if we asked for color quantization.
	 * In this example, we need to make an output work buffer of the right size.
	 */
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

	assert (cinfo.output_width == cinfo.output_height);

	pixel_size = get_frame_buffer_pixel ();
	set_texture_size (t, cinfo.output_width);
	set_texture_size_log2 (t, (short) ((float) (log (texture_size (t))) /log (2)));
	set_texture_bytes_per_texel (t, pixel_size);
	set_texture_address (t, ALLOC (pixel_size * texture_size (t) * texture_size (t)));
	assert (texture_address (t) != NULL);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void) jpeg_read_scanlines (&cinfo, buffer, 1);
		texture_put_scanline (t, buffer[0], cinfo.output_scanline);
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress (&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress (&cinfo);

	/* After finish_decompress, we can close the input file.
	 * Here we postpone it until after no more JPEG errors are possible,
	 * so as to simplify the setjmp error logic above.  (Actually, I don't
	 * think that jpeg_destroy can do an error exit, but why assume anything...)
	 */
	fclose (infile);

	/* At this point you may want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.num_warnings is nonzero).
	 */

	/* And we're done! */
}

/*
 * Take a scanline of texture read by the library
 *  and place it in the texture. Appropriate conversion
 *  to a new depth takes place at this point.
 * Note: Textures are assumed to be 24-bit jpegs. Grayscale
 *       jpegs which have only 8-bits per pixel will probably
 *       crash and burn.
 */

static void
texture_put_scanline (t, buf, row)
	TEXTURE t;
	JSAMPROW buf;
	long row;
{
	long i;
	long pix;

	row--;
	switch (texture_bytes_per_texel (t))
	{
	case 1:
		/*
		 * We're in 8bit mode, so take the average
		 *  of the red, green, and blue bytes as the
		 *  grayscale byte.
		 */
		
		for (i = 0; i < texture_size (t); i++)
		{
			pix = *buf++;
			pix += *buf++;
			pix += *buf++;
			pix /= 3;
			set_texel8 (t, i, row, (PIXEL8) pix);
		}
		break;
	case 4:
		/*
		 * We're in 24bit mode, so just plop the pixels
		 *  right in the texture. (A little bit shifting
		 *  is needed).
		 */
		
		for (i = 0; i < texture_size (t); i++)
		{
			pix = *buf++ << 16;
			pix += *buf++ << 8;
			pix += *buf++;
			set_texel32 (t, i, row, pix);
		}
		break;
	}
}

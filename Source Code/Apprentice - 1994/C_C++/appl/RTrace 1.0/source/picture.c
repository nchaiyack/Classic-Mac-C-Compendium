/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd		- improvements
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "defs.h"
#include "extern.h"

/**********************************************************************
 *    RAY TRACING - Picture - Version 7.3.1                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

/***** Write picture *****/
void
init_picture()
{
  switch (output_format)
  {
  case 0:	/* PIC */
    /* Screen Dimensions */
    WRITE_CHAR(picture, (unsigned char) (screen_size_x MOD 256));
    WRITE_CHAR(picture, (unsigned char) (screen_size_x DIV 256));
    WRITE_CHAR(picture, (unsigned char) (screen_size_y MOD 256));
    WRITE_CHAR(picture, (unsigned char) (screen_size_y DIV 256));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    if (raw_mode == 1)
    {
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_x MOD 256));
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_x DIV 256));
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_y MOD 256));
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_y DIV 256));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
    }
    break;
  case 1:	/* PPM */
    WRITE(picture, "P6\n");
    WRITE(picture, "%d\n", screen_size_x);
    WRITE(picture, "%d\n", screen_size_y);
    WRITE(picture, "255\n");
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE in PPM format");
    if (raw_mode == 1)
    {
      WRITE(raw_picture, "P6\n");
      WRITE(raw_picture, "%d\n", screen_size_x);
      WRITE(raw_picture, "%d\n", screen_size_y);
      WRITE(raw_picture, "255\n");
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE in PPM format");
    }
    break;
  }
  if (background_mode == 1)
  {
    WRITE_CHAR(background, (unsigned char) (screen_size_x MOD 256));
    WRITE_CHAR(background, (unsigned char) (screen_size_x DIV 256));
    WRITE_CHAR(background, (unsigned char) (screen_size_y MOD 256));
    WRITE_CHAR(background, (unsigned char) (screen_size_y DIV 256));
    if (IO_status != IO_OK)
      runtime_abort("unable to write BACKGROUND MASK");
  }
}
#define INDEX(v)\
(ROUND(MAX(0.0, MIN((real) INDEX_MAX, (v) * (real) SUCC(INDEX_MAX)))))

void
line_picture()
{
  REG int         i;

  for (i = 1; i <= screen_size_x; POSINC(i))
  {
    WRITE_CHAR(picture, (unsigned char) INDEX(true_color[i].r));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    WRITE_CHAR(picture, (unsigned char) INDEX(true_color[i].g));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    WRITE_CHAR(picture, (unsigned char) INDEX(true_color[i].b));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    if (background_mode == 1)
    {
      WRITE_CHAR(background, (unsigned char) INDEX(back_mask[i]));
      if (IO_status != IO_OK)
        runtime_abort("unable to write BACKGROUND MASK");
    }
    if (raw_mode == 1)
    {
      WRITE_CHAR(raw_picture, (unsigned char) INDEX(new_line[i].color.r));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
      WRITE_CHAR(raw_picture, (unsigned char) INDEX(new_line[i].color.g));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
      WRITE_CHAR(raw_picture, (unsigned char) INDEX(new_line[i].color.b));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
    }
  }
  FLUSH(picture);
  if (background_mode == 1)
    FLUSH(background);
  if (raw_mode == 1)
    FLUSH(raw_picture);
}

/*
 * Copyright (c) 1991, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
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
#ifdef TURBOC
#define MAIN_MODULE
#endif
#include "scn2sff_defs.h"
#include "scn2sff_extern.h"

/**********************************************************************
 *    SCENE - SFF output - Version 1.3                                *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1992              *
 **********************************************************************/

#define ABORT(message)\
{\
  WRITE(ERROR, "Error: %s\n", message);\
  HALT;\
}

static void
read_scene_file(name, preprocessor)
  char            name[];
  char            preprocessor[];
{
  char            command[STRING_MAX];
  char            message[STRING_MAX];

#ifndef dos
  if (preprocessor != NULL)
  {
    if (name == NULL)
    {
      sprintf(command, "%s 2>/dev/null", preprocessor);
      if ((yyin = (FILE *) fopen(command, "r")) == NULL)
      {
        sprintf(message, "cannot use \"%s\"", preprocessor);
	ABORT(message);
      }
    } else
    {
      if ((yyin = fopen(name, "r")) == NULL)
      {
        sprintf(message, "cannot open \"%s\"", name);
	ABORT(message);
      }
      fclose(yyin);
      sprintf(command, "%s %s 2>/dev/null", preprocessor, name);
      if ((yyin = (FILE *) fopen(command, "r")) == NULL)
      {
        sprintf(message, "cannot use \"%s\"", preprocessor);
	ABORT(message);
      }
    }
  } else
#endif
  {
    if (name == NULL) 
      yyin = stdin;
    else
    {
      if ((yyin = fopen(name, "r")) == NULL)
      {
        sprintf(message, "cannot open \"%s\"", name);
	ABORT(message);
      }
    }
  }
  if (yyparse())
    ABORT("invalid input specification");
#ifndef dos
  if (preprocessor != NULL)
    fclose(yyin);
 
#endif
  if (csg_level != -1)
    ABORT("incomplete CSG tree");
  if (scn2sff_lights == 0)
    ABORT("no lights");
  if (scn2sff_objects == 0)
    ABORT("no objects");
}

static void
scn2sff_defaults()
{
  from.x = 5.0;
  from.y = 0.0;
  from.z = 0.0;

  at.x = 0.0;
  at.y = 0.0;
  at.z = 0.0;

  scn2sff_up.x = 0.0;
  scn2sff_up.y = 1.0;
  scn2sff_up.z = 0.0;
 
  angle_h = 22.5;
  angle_v = 22.5;

  scn2sff_background.r = 0.2;
  scn2sff_background.g = 0.5;
  scn2sff_background.b = 0.7;

  ambient.r = 0.1;
  ambient.g = 0.1;
  ambient.b = 0.1;
}

static void
write_sff(short sff_wd_id, char *sff_filename)
{
  int			i;
  file_ptr		sff_file;

	/* Open the sff file. */
	SetVol( (StringPtr) NULL, sff_wd_id);
	OPEN(sff_file, sff_filename, WRITE_TEXT);

  WRITE(sff_file, "view\n");
  WRITE(sff_file, "%g %g %g\n", from.x, from.y, from.z);
  WRITE(sff_file, "%g %g %g\n", at.x, at.y, at.z);
  WRITE(sff_file, "%g %g %g\n", scn2sff_up.x, scn2sff_up.y, scn2sff_up.z);
  WRITE(sff_file, "%g %g\n", angle_h, angle_v);

  WRITE(sff_file, "colors\n");
  WRITE(sff_file, "%g %g %g\n", scn2sff_background.r, scn2sff_background.g, scn2sff_background.b);
  WRITE(sff_file, "%g %g %g\n", ambient.r, ambient.g, ambient.b);

  WRITE(sff_file, "lights\n");
  for (i = 0; i < scn2sff_lights; i++)
    WRITE(sff_file, "%s\n", light_list[i]);
  WRITE(sff_file, "\n");

  WRITE(sff_file, "surfaces\n");
  for (i = 0; i < scn2sff_surfaces; i++)
    WRITE(sff_file, "%s\n", surface_list[i]);
  WRITE(sff_file, "\n");

  WRITE(sff_file, "objects\n");
  for (i = 0; i < scn2sff_objects; i++)
    WRITE(sff_file, "%s\n", object_list[i]);
  WRITE(sff_file, "\n");

  WRITE(sff_file, "end\n");
 
	CLOSE(sff_file);
}

/***** Main *****/
#define MIN_OPTIONS 1

#ifndef dos
#define M4_PREPROCESSOR  "/usr/bin/m4"
#define CPP_PREPROCESSOR "/usr/lib/cpp -P"
#endif

void scn2sff(short scn_wd_id, char *scn_filename,
				short sff_wd_id, char *sff_filename);
void init_scn2sff_globals(void);
void init_scn2sff_textures(void);
extern int yy_init;

/********************************************************************\
* Procedure scn2sff                                                  *
*                                                                    *
* Purpose: This procedure converts a SCN file to an SFF file.        *
*                                                                    *
* Parameters: scn_wd_it:    ID of working directory of the SCN file  *
*             scn_filename: Name of file to read SCN from            *
*             sff_wd_it:    ID of working directory of the SFF file  *
*             sff_filename: Name of file to write SFF to             *
*                                                                    *
* Created January 26, 1995 by Greg Ferrar                            *
\********************************************************************/

void scn2sff(short scn_wd_id, char *scn_filename,
				short sff_wd_id, char *sff_filename)
{

	FileParam pb;

	/* Make sure the scn file gets opened from the right
		directory. */
	SetVol( (StringPtr) NULL, scn_wd_id);
	
	/* Initialize scn2sff */
	init_scn2sff_globals();
	init_scn2sff_textures();
	
	/* Force yacc and lex to reinitialize */
	yy_init = 1;

	/* Set view parameters to default values */
	scn2sff_defaults();

	/* Read the scn file */
	read_scene_file(scn_filename, NULL);
	
	/* Write the converted scn file to the sff file */
	write_sff(sff_wd_id, sff_filename);

}	/* scn2sff() */




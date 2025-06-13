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
#include "scn2sff_defs.h"

/**********************************************************************
 *    SCENE - Data - Version 1.3.1                                    *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

int             yylinecount = 1;

/***** Global data *****/
boolean         from_defined = FALSE;
boolean         at_defined = FALSE;
boolean         up_defined = FALSE;
boolean         angle_defined = FALSE;
boolean         ambient_defined = FALSE;
boolean         background_defined = FALSE;

xyz_struct      from, at, scn2sff_up;
real            angle_h, angle_v;
rgb_struct      scn2sff_background, ambient;

int             scn2sff_lights = 0;
int             scn2sff_surfaces = 0;
int             scn2sff_objects = 0;
int             transforms = 0;
int             textures = 0;
int             csg_level = -1;
int             list_level = -1;

int             current_surface = 0;
int             current_text_surface = 0;

real            current_refraction = 1.0;

char            buffer[STRING_MAX];

char_ptr        light_list[LIGHTS_MAX];
char_ptr        surface_list[SURFACES_MAX];
char_ptr        object_list[OBJECTS_MAX];
char_ptr        texture_list[TEXTURES_MAX];
char_ptr        transform_list[TRANSFORMS_MAX];

int             level_scene = -1;
char            level_type[LEVELS_MAX];
int             level_textures[LEVELS_MAX];
int             level_transforms[LEVELS_MAX];
int             level_current_surface[LEVELS_MAX];
int             level_current_text_surface[LEVELS_MAX];
real            level_current_refraction[LEVELS_MAX];


void init_scn2sff_globals(void);

/* This is necessary because MacRTrace calls this REPEATEDLY.
   We need to initialize the globals to their starting values
   each time. */
void init_scn2sff_globals(void)
{

	from_defined = FALSE;
	at_defined = FALSE;
	up_defined = FALSE;
	angle_defined = FALSE;
	ambient_defined = FALSE;
	background_defined = FALSE;

	scn2sff_lights = 0;
	scn2sff_surfaces = 0;
	scn2sff_objects = 0;
	transforms = 0;
	textures = 0;
	csg_level = -1;
	list_level = -1;

	current_surface = 0;
	current_text_surface = 0;

	current_refraction = 1.0;

	level_scene = -1;

}	/* init_globals() */


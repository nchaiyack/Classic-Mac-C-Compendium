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
#include "scn2sff_extern.h"

/**********************************************************************
 *    SCENE - Local - Version 1.3.1                                   *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, January 1992           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

void
save_level()
{
  level_scene++;
  if (level_scene == LEVELS_MAX)
    yyerror("too many GROUP LEVELS");
  level_type[level_scene] = 0;
  level_textures[level_scene] = textures;
  level_transforms[level_scene] = transforms;
  level_current_surface[level_scene] = current_surface;
  level_current_text_surface[level_scene] = current_text_surface;
  level_current_refraction[level_scene] = current_refraction;
}

void
restore_level()
{
  for (; textures > level_textures[level_scene];)
    FREE(texture_list[--textures]);
  for (; transforms > level_transforms[level_scene];)
    FREE(transform_list[--transforms]);
  current_surface = level_current_surface[level_scene];
  current_text_surface = level_current_text_surface[level_scene];
  current_refraction = level_current_refraction[level_scene];
  level_scene--;
}

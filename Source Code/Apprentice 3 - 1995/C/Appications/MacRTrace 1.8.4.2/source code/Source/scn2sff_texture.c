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
 *    SCENE - Textures - Version 1.2                                  *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, May 1992               *
 **********************************************************************/


#define TEXT_TRANSFORMS_MAX 2

static char     text_transform[STRING_MAX];
static int      text_transforms = 0;

void init_scn2sff_textures(void);

/* This is necessary because MacRTrace calls this REPEATEDLY.
   We need to initialize the globals to their starting values
   each time. */
void init_scn2sff_textures(void)
{
	text_transforms = 0;
}


#define CREATE_TEXTURE(counter_max, counter, list, data)\
{\
  if (counter >= counter_max)\
    yyerror("maximum number of ENTITIES exceeded");\
  strcpy(list, data);\
  counter++;\
}

/***** Transformations (textures) *****/
void
create_text_transf_scale(sx, sy, sz)
  real            sx, sy, sz;
{
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         sx, 0.0, 0.0, 0.0,
         0.0, sy, 0.0, 0.0,
         0.0, 0.0, sz, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_translate(vector)
  xyz_ptr         vector;
{
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         1.0, 0.0, 0.0, vector->x,
         0.0, 1.0, 0.0, vector->y,
         0.0, 0.0, 1.0, vector->z,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_rotatex(angle)
  real            angle;
{
  REG real        s, c;

  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         1.0, 0.0, 0.0, 0.0,
         0.0, c, -s, 0.0,
         0.0, s, c, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_rotatey(angle)
  real            angle;
{
  REG real        s, c;

  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         c, 0.0, s, 0.0,
         0.0, 1.0, 0.0, 0.0,
         -s, 0.0, c, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_rotatez(angle)
  real            angle;
{
  REG real        s, c;

  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         c, -s, 0.0, 0.0,
         s, c, 0.0, 0.0,
         0.0, 0.0, 1.0, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_rotate(axis, angle)
  xyz_ptr         axis;
  real            angle;
{
  REG real        s, c, c1;
  real            xy, xz, yz;

  NORMALIZE(*axis);
  xy = axis->x * axis->y;
  xz = axis->x * axis->z;
  yz = axis->y * axis->z;
  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  c1 = 1.0 - c;
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
	 SQR(axis->x) + c * (1.0 - SQR(axis->x)),
         xy * c1 - axis->z * s,
         xz * c1 + axis->y * s,
         0.0,
         xy * c1 + axis->z * s,
	 SQR(axis->y) + c * (1.0 - SQR(axis->y)),
         yz * c1 - axis->x * s,
         0.0,
         xz * c1 - axis->y * s,
         yz * c1 + axis->x * s,
	 SQR(axis->z) + c * (1.0 - SQR(axis->z)),
         0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_general(p1, p2, p3)
  xyz_ptr         p1, p2, p3;
{
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         p1->x, p1->y, p1->z, 0.0,
         p2->x, p2->y, p2->z, 0.0,
         p3->x, p3->y, p3->z, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_general_4(p1, p2, p3, p4)
  xyz_ptr         p1, p2, p3, p4;
{
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         p1->x, p1->y, p1->z, 0.0,
         p2->x, p2->y, p2->z, 0.0,
         p3->x, p3->y, p3->z, 0.0,
         p4->x, p4->y, p4->z, 1.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
create_text_transf_local()
{
  int             l, s, t;

  if (transforms == 0)
    return;
  l = 0;
  for (s = 0; s <= level_scene; s++)
  {
    for (t = level_transforms[s] - 1; t >= l; t--)
    {
      DEFINE(buffer, "%s", &transform_list[t][5]);
      CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform,
                     buffer);
      create_text_null();
    }
    l = level_transforms[s];
  }
  for (t = transforms - 1; t >= l; t--)
  {
    DEFINE(buffer, "%s", &transform_list[t][5]);
    CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform,
                   buffer);
    create_text_null();
  }
}

void
create_text_transf_default()
{
  DEFINE(buffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         0.0, 0.0, 0.0, 0.0,
         0.0, 0.0, 0.0, 0.0,
         0.0, 0.0, 0.0, 0.0,
         0.0, 0.0, 0.0, 0.0);
  CREATE_TEXTURE(TEXT_TRANSFORMS_MAX, text_transforms, text_transform, buffer);
}

void
define_text_transform()
{
  if (text_transforms == 0)
    create_text_transf_default();
  text_transforms = 0;
}

void
remove_text_level()
{
  int             t;

  t = textures - 1;
  if (level_scene >= 0)
    textures = level_textures[level_scene];
  else
    textures = 0;
  for (; t >= textures; t--)
    FREE(texture_list[t]);
}

/***** Textures *****/
#define MAPNAME(name) ((name) ? name : "-")

void
create_text_null()
{
  define_text_transform();
  DEFINE(buffer, "64 0 0 %s", text_transform);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_checkers()
{
  define_text_transform();
  DEFINE(buffer, "64 1 0 %s", text_transform);
  APPEND(END(buffer), " %d", scn2sff_surfaces);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_blotch(scale, name)
  real            scale;
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 2 0 %s", text_transform);
  APPEND(END(buffer), " %g %d %s", scale, scn2sff_surfaces, MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_bump(scale)
  real            scale;
{
  define_text_transform();
  DEFINE(buffer, "64 3 0 %s", text_transform);
  APPEND(END(buffer), " %g", scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_marble(name)
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 4 0 %s", text_transform);
  APPEND(END(buffer), " %s", MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_fbm(offset, scale, omega, lambda, threshold, octaves, name)
  real            offset, scale, omega, lambda, threshold;
  int             octaves;
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 5 0 %s", text_transform);
  APPEND(END(buffer), " %g %g %g %g %g %d %s", offset, scale, omega,
         lambda, threshold, octaves, MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_fbmbump(offset, scale, lambda, octaves, dummy)
  real            offset, scale, lambda;
  int             octaves;
  real				dummy;
{
  define_text_transform();
  DEFINE(buffer, "64 6 0 %s", text_transform);
  APPEND(END(buffer), " %g %g %g %d", offset, scale, lambda, octaves);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_wood(color)
  rgb_ptr         color;
{
  define_text_transform();
  DEFINE(buffer, "64 7 0 %s", text_transform);
  if (color)
    APPEND(END(buffer), " %g %g %g", color->r, color->g, color->b);
  else
    APPEND(END(buffer), " -1");
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_round(scale)
  real            scale;
{
  define_text_transform();
  DEFINE(buffer, "64 8 0 %s", text_transform);
  APPEND(END(buffer), " %g", scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_bozo(turbulence, name)
  real            turbulence;
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 9 0 %s", text_transform);
  APPEND(END(buffer), " %g %s", turbulence, MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_ripples(frequency, phase, scale)
  real            frequency, phase, scale;
{
  define_text_transform();
  DEFINE(buffer, "64 10 0 %s", text_transform);
  APPEND(END(buffer), " %g %g %g", frequency, phase, scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_waves(frequency, phase, scale)
  real            frequency, phase, scale;
{
  define_text_transform();
  DEFINE(buffer, "64 11 0 %s", text_transform);
  APPEND(END(buffer), " %g %g %g", frequency, phase, scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_spotted(name)
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 12 0 %s", text_transform);
  APPEND(END(buffer), " %s", MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_dents(scale)
  real            scale;
{
  define_text_transform();
  DEFINE(buffer, "64 13 0 %s", text_transform);
  APPEND(END(buffer), " %g", scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_agate(name)
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 14 0 %s", text_transform);
  APPEND(END(buffer), " %s", MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_wrinkles(scale)
  real            scale;
{
  define_text_transform();
  DEFINE(buffer, "64 15 0 %s", text_transform);
  APPEND(END(buffer), " %g", scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_granite(name)
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 16 0 %s", text_transform);
  APPEND(END(buffer), " %s", MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_gradient(turbulence, direction, name)
  real            turbulence;
  xyz_ptr         direction;
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 17 0 %s", text_transform);
  APPEND(END(buffer), " %g %g %g %g %s", turbulence,
         direction->x, direction->y, direction->z, MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_imagemap(turbulence, mode, u_axis, v_axis, name)
  real            turbulence;
  int             mode, u_axis, v_axis;
  char_ptr        name;
{
  define_text_transform();
  DEFINE(buffer, "64 18 0 %s", text_transform);
  APPEND(END(buffer), " %g %d %d %d %s", turbulence,
         mode, u_axis, v_axis, MAPNAME(name));
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_gloss(scale)
  real            scale;
{
  define_text_transform();
  DEFINE(buffer, "64 19 0 %s", text_transform);
  APPEND(END(buffer), " %g", scale);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

void
create_text_bump3(scale, size)
  real            scale, size;
{
  define_text_transform();
  DEFINE(buffer, "64 20 0 %s", text_transform);
  APPEND(END(buffer), " %g %g", scale, size);
  CREATE(TEXTURES_MAX, textures, texture_list, buffer);
}

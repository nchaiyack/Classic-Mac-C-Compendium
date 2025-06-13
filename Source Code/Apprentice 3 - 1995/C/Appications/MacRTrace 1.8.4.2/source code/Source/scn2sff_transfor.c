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
 *    SCENE - Transformations - Version 1.2                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, May 1992               *
 **********************************************************************/


/***** Transformations (objects) *****/
void
remove_transf_level()
{
  int             t;

  t = transforms - 1;
  if (level_scene >= 0)
    transforms = level_transforms[level_scene];
  else
    transforms = 0;
  for (; t >= transforms; t--)
    FREE(transform_list[t]);
}

void
create_transf_scale(sx, sy, sz)
  real            sx, sy, sz;
{
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         sx, 0.0, 0.0, 0.0,
         0.0, sy, 0.0, 0.0,
         0.0, 0.0, sz, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_translate(vector)
  xyz_ptr         vector;
{
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         1.0, 0.0, 0.0, vector->x,
         0.0, 1.0, 0.0, vector->y,
         0.0, 0.0, 1.0, vector->z,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_rotatex(angle)
  real            angle;
{
  REG real        s, c;

  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         1.0, 0.0, 0.0, 0.0,
         0.0, c, -s, 0.0,
         0.0, s, c, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_rotatey(angle)
  real            angle;
{
  REG real        s, c;

  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         c, 0.0, s, 0.0,
         0.0, 1.0, 0.0, 0.0,
         -s, 0.0, c, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_rotatez(angle)
  real            angle;
{
  REG real        s, c;

  s = SIN(DEGREE_TO_RADIAN(angle));
  c = COS(DEGREE_TO_RADIAN(angle));
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         c, -s, 0.0, 0.0,
         s, c, 0.0, 0.0,
         0.0, 0.0, 1.0, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_rotate(axis, angle)
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
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_general(p1, p2, p3)
  xyz_ptr         p1, p2, p3;
{
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         p1->x, p1->y, p1->z, 0.0,
         p2->x, p2->y, p2->z, 0.0,
         p3->x, p3->y, p3->z, 0.0,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

void
create_transf_general_4(p1, p2, p3, p4)
  xyz_ptr         p1, p2, p3, p4;
{
  DEFINE(buffer, "65 0 %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         p1->x, p1->y, p1->z, p4->x,
         p2->x, p2->y, p2->z, p4->y,
         p3->x, p3->y, p3->z, p4->z,
         0.0, 0.0, 0.0, 1.0);
  CREATE(TRANSFORMS_MAX, transforms, transform_list, buffer);
}

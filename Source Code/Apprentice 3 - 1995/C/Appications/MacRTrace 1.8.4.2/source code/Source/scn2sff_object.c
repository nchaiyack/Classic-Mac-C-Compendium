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
 *    SCENE - Objects - Version 1.3.1                                 *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1991          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

#define SMALL (0.001)

static char       level_type_main;

static void
define_transforms()
{
  int             t, s, i;

  if (transforms == 0)
    return;
  t = transforms;
  for (s = level_scene; s >= 0; s--)
  {
    if (level_type[s] == 1)
      return;
    for (i = level_transforms[s]; i < t; i++)
      APPEND(END(buffer), "\n%s", transform_list[i]);
    t = level_transforms[s];
  }
  if (level_type_main == 1)
    return;
  for (i = 0; i < t; i++)
    APPEND(END(buffer), "\n%s", transform_list[i]);
}

static void
define_textures()
{
  int             t, s, i;

  if (textures == 0)
    return;
  t = textures;
  for (s = level_scene; s >= 0; s--)
  {
    if (level_type[s] == 1)
      return;
    for (i = level_textures[s]; i < t; i++)
      APPEND(END(buffer), "\n%s", texture_list[i]);
    t = level_textures[s];
  }
  if (level_type_main == 1)
    return;
  for (i = 0; i < t; i++)
    APPEND(END(buffer), "\n%s", texture_list[i]);
}

#define CHECK_SURFACE \
if (current_surface == 0) create_surface_default()

/***** Objects (Primitive) *****/
void
create_sphere(center, radius)
  xyz_ptr         center;
  real            radius;
{
  
  CHECK_SURFACE;
  DEFINE(buffer, "1 %d %g %g %g %g %g",
         current_surface, current_refraction,
         center->x, center->y, center->z, radius);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
}

void
create_box(center, sx, sy, sz)
  xyz_ptr         center;
  real            sx, sy, sz;
{
  CHECK_SURFACE;
  DEFINE(buffer, "2 %d %g %g %g %g %g %g %g",
         current_surface, current_refraction,
         center->x, center->y, center->z, sx, sy, sz);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
}

void
create_open_cone(apex, base, radius)
  xyz_ptr         apex, base;
  real            radius;
{
  CHECK_SURFACE;
  DEFINE(buffer, "4 %d %g %.9g %.9g %.9g %g %.9g %.9g %.9g %g",
         current_surface, current_refraction,
         base->x, base->y, base->z, radius,
         apex->x, apex->y, apex->z, SMALL * SMALL * radius);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_disc(center, normal, radius)
  xyz_ptr         center, normal;
  real            radius;
{
  xyz_struct      apex;

  NORMALIZE(*normal);
  apex.x = center->x + normal->x * SMALL;
  apex.y = center->y + normal->y * SMALL;
  apex.z = center->z + normal->z * SMALL;
  create_open_cone(&apex, center, radius);
}

void
create_ring(center, normal, oradius, iradius)
  xyz_ptr         center, normal;
  real            oradius, iradius;
{
  xyz_struct      apex;

  NORMALIZE(*normal);
  apex.x = center->x + normal->x * SMALL;
  apex.y = center->y + normal->y * SMALL;
  apex.z = center->z + normal->z * SMALL;
  create_open_truncated_cone(&apex, iradius, center, oradius);
}

void
create_cone(apex, base, radius)
  xyz_ptr         apex, base;
  real            radius;
{
  xyz_struct      normal;

  create_open_cone(apex, base, radius);
  normal.x = base->x - apex->x;
  normal.y = base->y - apex->y;
  normal.z = base->z - apex->z;
  create_disc(base, &normal, radius);
}

void
create_open_cylinder(apex, base, radius)
  xyz_ptr         apex, base;
  real            radius;
{
  CHECK_SURFACE;
  DEFINE(buffer, "4 %d %g %g %g %g %g %g %g %g %g",
         current_surface, current_refraction,
         base->x, base->y, base->z, radius,
         apex->x, apex->y, apex->z, radius);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_cylinder(apex, base, radius)
  xyz_ptr         apex, base;
  real            radius;
{
  xyz_struct      normal;

  create_open_cylinder(apex, base, radius);
  normal.x = base->x - apex->x;
  normal.y = base->y - apex->y;
  normal.z = base->z - apex->z;
  create_disc(base, &normal, radius);
  normal.x = -normal.x;
  normal.y = -normal.y;
  normal.z = -normal.z;
  create_disc(apex, &normal, radius);
}

void
create_open_truncated_cone(apex, aradius, base, bradius)
  xyz_ptr         apex;
  real            aradius;
  xyz_ptr         base;
  real            bradius;
{
  CHECK_SURFACE;
  DEFINE(buffer, "4 %d %g %.9g %.9g %.9g %g %.9g %.9g %.9g %g",
         current_surface, current_refraction,
         base->x, base->y, base->z, bradius,
         apex->x, apex->y, apex->z, aradius);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_truncated_cone(apex, aradius, base, bradius)
  xyz_ptr         apex;
  real            aradius;
  xyz_ptr         base;
  real            bradius;
{
  xyz_struct      normal;

  create_open_truncated_cone(apex, aradius, base, bradius);
  normal.x = base->x - apex->x;
  normal.y = base->y - apex->y;
  normal.z = base->z - apex->z;
  create_disc(base, &normal, bradius);
  normal.x = -normal.x;
  normal.y = -normal.y;
  normal.z = -normal.z;
  create_disc(apex, &normal, aradius);
}

void
create_patch(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)
  xyz_ptr         p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12;
{
  CHECK_SURFACE;
  DEFINE(buffer, "3 %d %g 0 0 0 1 1 1 -",
         current_surface, current_refraction);
  APPEND(END(buffer), "\n1 2 3 4 5 6 7 8 9 10 11 12\n\n");
  APPEND(END(buffer), "%g %g %g\n%g %g %g\n%g %g %g\n%g %g %g\n",
         p1->x, p1->y, p1->z, p2->x, p2->y, p2->z,
         p3->x, p3->y, p3->z, p4->x, p4->y, p4->z);
  APPEND(END(buffer), "%g %g %g\n%g %g %g\n%g %g %g\n%g %g %g\n",
         p5->x, p5->y, p5->z, p6->x, p6->y, p6->z,
         p7->x, p7->y, p7->z, p8->x, p8->y, p8->z);
  APPEND(END(buffer), "%g %g %g\n%g %g %g\n%g %g %g\n%g %g %g\n",
         p9->x, p9->y, p9->z, p10->x, p10->y, p10->z,
         p11->x, p11->y, p11->z, p12->x, p12->y, p12->z);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_patch_file(vector, scale, name)
  xyz_ptr         vector, scale;
  char_ptr        name;
{
  CHECK_SURFACE;
  DEFINE(buffer, "3 %d %g %g %g %g %g %g %g %s",
         current_surface, current_refraction,
         vector->x, vector->y, vector->z, scale->x, scale->y, scale->z, name);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_polygon(points, point_list)
  int             points;
  xyz_ptr         point_list;
{
  int             i;

  CHECK_SURFACE;
  DEFINE(buffer, "5 %d %g 0 0 0 1 1 1 -\n%d",
         current_surface, current_refraction, points);
  for (i = 1; i <= points; i++)
    APPEND(END(buffer), " %d", i);
  APPEND(END(buffer), "\n");
  for (i = 0; i < points; i++)
    APPEND(END(buffer), "\n%g %g %g",
	   point_list[i].x,
           point_list[i].y,
           point_list[i].z);
  APPEND(END(buffer), "\n");
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_polygon_file(vector, scale, name)
  xyz_ptr         vector, scale;
  char_ptr        name;
{
  CHECK_SURFACE;
  DEFINE(buffer, "5 %d %g %g %g %g %g %g %g %s",
         current_surface, current_refraction,
         vector->x, vector->y, vector->z, scale->x, scale->y, scale->z, name);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_triangle_normal(p1, n1, p2, n2, p3, n3)
  xyz_ptr         p1, n1, p2, n2, p3, n3;
{
  CHECK_SURFACE;
  DEFINE(buffer, "6 %d %g 0 0 0 1 1 1 -",
         current_surface, current_refraction);
  APPEND(END(buffer), "\n%g %g %g %g %g %g",
         p1->x, p1->y, p1->z, n1->x, n1->y, n1->z);
  APPEND(END(buffer), " %g %g %g %g %g %g",
         p2->x, p2->y, p2->z, n2->x, n2->y, n2->z);
  APPEND(END(buffer), " %g %g %g %g %g %g\n",
         p3->x, p3->y, p3->z, n3->x, n3->y, n3->z);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
}

void
create_triangle_normal_file(vector, scale, name)
  xyz_ptr         vector, scale;
  char_ptr        name;
{
  CHECK_SURFACE;
  DEFINE(buffer, "6 %d %g %g %g %g %g %g %g %s",
         current_surface, current_refraction,
         vector->x, vector->y, vector->z, scale->x, scale->y, scale->z, name);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_wedge(p1, p2, p3, width)
  xyz_ptr         p1, p2, p3;
  real            width;
{
  xyz_struct      t1, t2, t3;

  t1.x = p3->x - p2->x;
  t1.y = p3->y - p2->y;
  t1.z = p3->z - p2->z;
  t2.x = p3->x - p1->x;
  t2.y = p3->y - p1->y;
  t2.z = p3->z - p1->z;
  CROSS_PRODUCT(t3, t1, t2);
  NORMALIZE(t3);
  t1.x = p1->x + width * t3.x;
  t1.y = p1->y + width * t3.y;
  t1.z = p1->z + width * t3.z;
  t2.x = p2->x + width * t3.x;
  t2.y = p2->y + width * t3.y;
  t2.z = p2->z + width * t3.z;
  t3.x = p3->x + width * t3.x;
  t3.y = p3->y + width * t3.y;
  t3.z = p3->z + width * t3.z;
  CHECK_SURFACE;
  DEFINE(buffer, "5 %d %g 0 0 0 1 1 1 -",
         current_surface, current_refraction);
  APPEND(END(buffer), "\n3 1 2 3\n3 4 6 5\n");
  APPEND(END(buffer), "4 1 3 6 4\n4 1 4 5 2\n4 3 2 5 6\n\n");
  APPEND(END(buffer), "%g %g %g\n", p1->x, p1->y, p1->z);
  APPEND(END(buffer), "%g %g %g\n", p2->x, p2->y, p2->z);
  APPEND(END(buffer), "%g %g %g\n", p3->x, p3->y, p3->z);
  APPEND(END(buffer), "%g %g %g\n", t1.x, t1.y, t1.z);
  APPEND(END(buffer), "%g %g %g\n", t2.x, t2.y, t2.z);
  APPEND(END(buffer), "%g %g %g\n", t3.x, t3.y, t3.z);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
}

void
create_tetra(p1, p2, p3, p4)
  xyz_ptr         p1, p2, p3, p4;
{
  CHECK_SURFACE;
  DEFINE(buffer, "5 %d %g 0 0 0 1 1 1 -",
         current_surface, current_refraction);
  APPEND(END(buffer), "\n3 1 2 3\n3 1 3 4\n3 1 4 2\n3 2 4 3\n\n");
  APPEND(END(buffer), "%g %g %g\n", p1->x, p1->y, p1->z);
  APPEND(END(buffer), "%g %g %g\n", p2->x, p2->y, p2->z);
  APPEND(END(buffer), "%g %g %g\n", p3->x, p3->y, p3->z);
  APPEND(END(buffer), "%g %g %g\n", p4->x, p4->y, p4->z);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
}

void
create_triangle(p1, p2, p3)
  xyz_ptr         p1, p2, p3;
{
  xyz_ptr         tlist;

  ALLOCATE(tlist, xyz_struct, 3 * sizeof(xyz_struct), PARSE_TYPE);
  STRUCT_ASSIGN(tlist[0], *p1);
  STRUCT_ASSIGN(tlist[1], *p2);
  STRUCT_ASSIGN(tlist[2], *p3);
  create_polygon(3, tlist);
  FREE(tlist);
}

void
create_quadrangle(p1, p2, p3, p4)
  xyz_ptr         p1, p2, p3, p4;
{
  xyz_ptr         tlist;

  ALLOCATE(tlist, xyz_struct, 4 * sizeof(xyz_struct), PARSE_TYPE);
  STRUCT_ASSIGN(tlist[0], *p1);
  STRUCT_ASSIGN(tlist[1], *p2);
  STRUCT_ASSIGN(tlist[2], *p3);
  STRUCT_ASSIGN(tlist[3], *p4);
  create_polygon(4, tlist);
  FREE(tlist);
}

void
create_open_truncated_pyramid(height, scale, points, point_list)
  real            height, scale;
  int             points;
  xyz_ptr         point_list;
{
  int             i, j;
  xyz_struct      th, t1, t2, apex;
  xyz_ptr         tlist;

  t1.x = point_list[0].x - point_list[1].x;
  t1.y = point_list[0].y - point_list[1].y;
  t1.z = point_list[0].z - point_list[1].z;
  t2.x = point_list[2].x - point_list[1].x;
  t2.y = point_list[2].y - point_list[1].y;
  t2.z = point_list[2].z - point_list[1].z;
  if (ABS(DOT_PRODUCT(t1, t2)) > 1.0 - ROUNDOFF)
    yyerror("initial 3 VERTICES are colinear");
  CROSS_PRODUCT(th, t1, t2);
  NORMALIZE(th);
  th.x *= height;
  th.y *= height;
  th.z *= height;
  ALLOCATE(tlist, xyz_struct, points * sizeof(xyz_struct), PARSE_TYPE);
  if (scale < 1.0 - ROUNDOFF)
  {
    apex.x = 0.0;
    apex.y = 0.0;
    apex.z = 0.0;
  }
  for (i = 0; i < points; i++)
  {
    tlist[i].x = point_list[i].x + th.x;
    tlist[i].y = point_list[i].y + th.y;
    tlist[i].z = point_list[i].z + th.z;
    if (scale < 1.0 - ROUNDOFF)
    {
      apex.x += point_list[i].x;
      apex.y += point_list[i].y;
      apex.z += point_list[i].z;
    }
  }
  if (scale < 1.0 - ROUNDOFF)
  {
    apex.x /= (real) points;
    apex.y /= (real) points;
    apex.z /= (real) points;
    apex.x += th.x;
    apex.y += th.y;
    apex.z += th.z;
    for (i = 0; i < points; i++)
    {
      tlist[i].x = apex.x + (tlist[i].x - apex.x) * scale;
      tlist[i].y = apex.y + (tlist[i].y - apex.y) * scale;
      tlist[i].z = apex.z + (tlist[i].z - apex.z) * scale;
    }
  }
  j = 1;
  for (i = 0; i < points; i++)
  {
    create_quadrangle(&point_list[i], &tlist[i], &tlist[j], &point_list[j]);
    j++;
    if (j == points)
      j = 0;
  }
  FREE(tlist);
}

void
create_open_prism(height, points, point_list)
  real            height;
  int             points;
  xyz_ptr         point_list;
{
  create_open_truncated_pyramid(height, 1.0, points, point_list);
}

void
create_prism(height, points, point_list)
  real            height;
  int             points;
  xyz_ptr         point_list;
{
  int             i, j;
  xyz_struct      th, t1, t2;
  xyz_ptr         tlist;

  create_open_prism(height, points, point_list);
  create_polygon(points, point_list);
  t1.x = point_list[0].x - point_list[1].x;
  t1.y = point_list[0].y - point_list[1].y;
  t1.z = point_list[0].z - point_list[1].z;
  t2.x = point_list[2].x - point_list[1].x;
  t2.y = point_list[2].y - point_list[1].y;
  t2.z = point_list[2].z - point_list[1].z;
  CROSS_PRODUCT(th, t1, t2);
  NORMALIZE(th);
  th.x *= height;
  th.y *= height;
  th.z *= height;
  ALLOCATE(tlist, xyz_struct, points * sizeof(xyz_struct), PARSE_TYPE);
  j = points - 1;
  for (i = 0; i < points; i++)
  {
    tlist[i].x = point_list[j].x + th.x;
    tlist[i].y = point_list[j].y + th.y;
    tlist[i].z = point_list[j].z + th.z;
    j--;
  }
  create_polygon(points, tlist);
  FREE(tlist);
}

void
create_open_pyramid(height, points, point_list)
  real            height;
  int             points;
  xyz_ptr         point_list;
{
  int             i, j;
  xyz_struct      th, t1, t2, apex;

  t1.x = point_list[0].x - point_list[1].x;
  t1.y = point_list[0].y - point_list[1].y;
  t1.z = point_list[0].z - point_list[1].z;
  t2.x = point_list[2].x - point_list[1].x;
  t2.y = point_list[2].y - point_list[1].y;
  t2.z = point_list[2].z - point_list[1].z;
  if (ABS(DOT_PRODUCT(t1, t2)) > 1.0 - ROUNDOFF)
    yyerror("initial 3 VERTICES are colinear");
  CROSS_PRODUCT(th, t1, t2);
  NORMALIZE(th);
  th.x *= height;
  th.y *= height;
  th.z *= height;
  apex.x = 0.0;
  apex.y = 0.0;
  apex.z = 0.0;
  for (i = 0; i < points; i++)
  {
    apex.x += point_list[i].x;
    apex.y += point_list[i].y;
    apex.z += point_list[i].z;
  }
  apex.x /= (real) points;
  apex.y /= (real) points;
  apex.z /= (real) points;
  apex.x += th.x;
  apex.y += th.y;
  apex.z += th.z;
  j = 1;
  for (i = 0; i < points; i++)
  {
    create_triangle(&point_list[i], &apex, &point_list[j]);
    j++;
    if (j == points)
      j = 0;
  }
}

void
create_pyramid(height, points, point_list)
  real            height;
  int             points;
  xyz_ptr         point_list;
{
  create_polygon(points, point_list);
  create_open_pyramid(height, points, point_list);
}

void
create_open_torus(oradius, iradius, sangle, eangle, osamples, isamples)
  real            oradius, iradius, sangle, eangle;
  int             osamples, isamples;
{
  int             i, j, jn;
  real            iangle, oangle, tradius, sin_angle, cos_angle;
  xyz_struct      old_center, new_center, normal1, normal2, normal3;
  xyz_ptr         old_list, new_list, tmp_list;

  ALLOCATE(old_list, xyz_struct, isamples * sizeof(xyz_struct), PARSE_TYPE);
  ALLOCATE(new_list, xyz_struct, isamples * sizeof(xyz_struct), PARSE_TYPE);
  while (eangle < sangle)
    eangle += 360.0;
  sangle = DEGREE_TO_RADIAN(sangle);
  eangle = DEGREE_TO_RADIAN(eangle) - sangle;
  sin_angle = SIN(sangle);
  cos_angle = COS(sangle);
  for (i = 0; i < isamples; i++)
  {
    iangle = 2.0 * PI * (real) i / (real) isamples;
    tradius = oradius + iradius * COS(iangle);
    old_list[i].x = tradius * cos_angle;
    old_list[i].y = iradius * SIN(iangle);
    old_list[i].z = tradius * sin_angle;
  }
  old_center.x = oradius * cos_angle;
  old_center.y = 0.0;
  old_center.z = oradius * sin_angle;
  for (i = 1; i <= osamples; i++)
  {
    oangle = sangle + eangle * (real) i / (real) osamples;
    sin_angle = SIN(oangle);
    cos_angle = COS(oangle);
    for (j = 0; j < isamples; j++)
    {
      iangle = 2.0 * PI * (real) j / (real) isamples;
      tradius = oradius + iradius * COS(iangle);
      new_list[j].x = tradius * cos_angle;
      new_list[j].y = iradius * SIN(iangle);
      new_list[j].z = tradius * sin_angle;
    }
    new_center.x = oradius * cos_angle;
    new_center.y = 0.0;
    new_center.z = oradius * sin_angle;
    jn = 1;
    for (j = 0; j < isamples; j++)
    {
      normal1.x = old_list[j].x - old_center.x;
      normal1.y = old_list[j].y - old_center.y;
      normal1.z = old_list[j].z - old_center.z;
      normal2.x = old_list[jn].x - old_center.x;
      normal2.y = old_list[jn].y - old_center.y;
      normal2.z = old_list[jn].z - old_center.z;
      normal3.x = new_list[j].x - new_center.x;
      normal3.y = new_list[j].y - new_center.y;
      normal3.z = new_list[j].z - new_center.z;
      create_triangle_normal(&old_list[j], &normal1,
			     &old_list[jn], &normal2,
			     &new_list[j], &normal3);
      normal1.x = new_list[jn].x - new_center.x;
      normal1.y = new_list[jn].y - new_center.y;
      normal1.z = new_list[jn].z - new_center.z;
      normal2.x = new_list[j].x - new_center.x;
      normal2.y = new_list[j].y - new_center.y;
      normal2.z = new_list[j].z - new_center.z;
      normal3.x = old_list[jn].x - old_center.x;
      normal3.y = old_list[jn].y - old_center.y;
      normal3.z = old_list[jn].z - old_center.z;
      create_triangle_normal(&new_list[jn], &normal1,
			     &new_list[j], &normal2,
			     &old_list[jn], &normal3);
      jn++;
      if (jn == isamples)
	jn = 0;
    }
    tmp_list = old_list;
    old_list = new_list;
    new_list = tmp_list;
    STRUCT_ASSIGN(old_center, new_center);
  }
  FREE(old_list);
  FREE(new_list);
}

void
create_torus(oradius, iradius, sangle, eangle, osamples, isamples)
  real            oradius, iradius, sangle, eangle;
  int             osamples, isamples;
{
  xyz_struct      up, center, normal;

  create_open_torus(oradius, iradius, sangle, eangle, osamples, isamples);
  if (ABS(eangle - sangle) > 360.0 - ROUNDOFF)
    return;
  up.x = 0.0;
  up.y = 1.0;
  up.z = 0.0;
  sangle = DEGREE_TO_RADIAN(sangle);
  center.x = oradius * COS(sangle);
  center.y = 0.0;
  center.z = oradius * SIN(sangle);
  CROSS_PRODUCT(normal, center, up);
  create_disc(&center, &normal, iradius);
  eangle = DEGREE_TO_RADIAN(eangle);
  center.x = oradius * COS(eangle);
  center.y = 0.0;
  center.z = oradius * SIN(eangle);
  CROSS_PRODUCT(normal, up, center);
  create_disc(&center, &normal, iradius);
}

void
create_text3d_file(name)
  char_ptr        name;
{
  CHECK_SURFACE;
  DEFINE(buffer, "7 %d %g %s", current_surface, current_refraction, name);
  define_transforms();
  define_textures();
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_csg_begin(level, op, surface, refraction)
  int             level, op, surface;
  double          refraction;
{
  CHECK_SURFACE;
  DEFINE(buffer, "66 0 %d %g %d - csg begin %d",
	 surface, refraction, op, level);
  define_transforms();
  define_textures();
  if (level_scene >= 0)
    level_type[level_scene] = 1;
  else
    level_type_main = 1;
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_csg_next(level)
  int             level;
{
  DEFINE(buffer, "66 1 - csg middle %d", level);
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_csg_end(level)
  int             level;
{
  if (level_scene >= 0)
    level_type[level_scene] = 0;
  else
    level_type_main = 0;
  DEFINE(buffer, "66 2 - csg end %d    ", level);
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_list_begin(level, surface, refraction)
  int             level, surface;
  double          refraction;
{
  CHECK_SURFACE;
  DEFINE(buffer, "67 0 %d %g - list begin %d", surface, refraction, level);
  define_transforms();
  define_textures();
  if (level_scene >= 0)
    level_type[level_scene] = 1;
  else
    level_type_main = 1;
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

void
create_list_end(level)
  int             level;
{
  if (level_scene >= 0)
    level_type[level_scene] = 0;
  else
    level_type_main = 0;
  DEFINE(buffer, "67 1 - list end %d        ", level);
  CREATE(OBJECTS_MAX, scn2sff_objects, object_list, buffer);
} 

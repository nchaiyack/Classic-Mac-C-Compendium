/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - CSG
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
#include "csg.h"

/**********************************************************************
 *    RAY TRACING - CSG - Version 7.3.1                               *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, June 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

/***** CSG *****/
tree_struct tree;
hit_struct hit_global;

void
csg_copy_hit(hit_from, hit_to)
  hit_ptr         hit_from, hit_to;
{
  REG int         i;

  hit_to->nodes = hit_from->nodes;
  for (i = 0; i < hit_from->nodes; POSINC(i))
    STRUCT_ASSIGN(hit_to->data[i], hit_from->data[i]);
}
static unsigned int
csg_enter(position, vector, hit)
  xyz_ptr         position, vector;
  hit_ptr         hit;
{
  xyz_struct      tmp_position, normal;

  if (hit->data[0].enter != 0)
    return (hit->data[0].enter - 1);
  tmp_position.x = position->x + hit->data[0].distance * vector->x;
  tmp_position.y = position->y + hit->data[0].distance * vector->y;
  tmp_position.z = position->z + hit->data[0].distance * vector->z;
  if (hit->data[0].object->object_type == CSG_TYPE)
    csg_copy_hit(hit, &hit_global);
  OBJECT_NORMAL(&tmp_position, hit->data[0].object, &normal);
  if (DOT_PRODUCT(normal, *vector) < 0.0)
    return 1;
  return 0;
}
static void
csg_add_hit(hit, distance, vector, object, tree)
  hit_ptr         hit;
  real            distance;
  xyz_ptr         vector;
  object_ptr      object;
  tree_ptr        tree;
{
  if (hit->nodes == CSG_NODES_MAX)
    runtime_abort("too many CSG HIT NODES");
  hit->data[hit->nodes].distance = distance;
  STRUCT_ASSIGN(hit->data[hit->nodes].vector,  *vector);
  hit->data[hit->nodes].object = object;
  hit->data[hit->nodes].enter = 0;
  STRUCT_ASSIGN(hit->data[hit->nodes].tree, *tree);
  POSINC(hit->nodes);
}

boolean
csg_union(position, vector, hit1, hit2, distance1, distance2,
          hit, distance)
  xyz_ptr         position, vector;
  hit_ptr         hit1, hit2;
  real            distance1, distance2;
  hit_ptr        *hit;
  real           *distance;
{
  REG real        distance3;
  hit_struct      hit3;
  hit_ptr         hit_tmp;

  while (TRUE)
  {
    if ((hit2->nodes == 0) OR(csg_enter(position, vector, hit2) != 0))
    {
      /* Hit 1st */
      *hit = hit1;
      *distance = distance1;
      CSG_SET_ENTER(hit1, csg_enter(position, vector, hit1));
      return FALSE;
    }
    hit3.nodes = 0;
    distance3 = object_intersect(hit1->data[hit1->nodes - 1].object,
				 position, vector, &hit3,
				 distance2 + threshold_distance);
    if (distance3 < distance2 + threshold_distance)
    {
      /* Leaving 2nd */
      *hit = hit2;
      *distance = distance2;
      CSG_SET_ENTER(hit2, 0);
      return FALSE;
    }
    hit_tmp = hit1;
    hit1 = hit2;
    hit2 = hit_tmp;
    distance1 = distance2;
    csg_copy_hit(&hit3, hit2);
    distance2 = distance3;
  }
}
boolean
csg_subtraction(position, vector, hit1, hit2, distance1, distance2,
                hit, distance)
  xyz_ptr         position, vector;
  hit_ptr         hit1, hit2;
  real            distance1, distance2;
  hit_ptr        *hit;
  real           *distance;
{
  REG real        distance3;
  hit_struct      hit3;

  while (TRUE)
  {
    if (distance1 < distance2)
    {
      if ((hit2->nodes == 0) OR(csg_enter(position, vector, hit2) != 0))
      {
        /* Hit 1st */
        *hit = hit1;
        *distance = distance1;
        CSG_SET_ENTER(hit1, csg_enter(position, vector, hit1));
        return FALSE;
      }
      hit3.nodes = 0;
      distance3 = object_intersect(hit1->data[hit1->nodes - 1].object,
                                   position, vector, &hit3,
                                   distance2 + threshold_distance);
      if (distance3 < distance2 + threshold_distance)
        /* Miss */
        return TRUE;
      distance1 = distance3;
      csg_copy_hit(&hit3, hit1);
      continue;
    }
    if (hit1->nodes == 0)
      /* Miss */
      return TRUE;
    if (csg_enter(position, vector, hit1) == 0)
    {
      /* Hit 2nd inverted */
      *hit = hit2;
      *distance = distance2;
      CSG_SET_ENTER(hit2, NOT csg_enter(position, vector, hit2));
      return FALSE;
    }
    hit3.nodes = 0;
    distance3 = object_intersect(hit2->data[hit2->nodes - 1].object,
                                 position, vector, &hit3,
                                 distance1 + threshold_distance);
    if (distance3 < distance1 + threshold_distance)
    {
      /* Entering 1st */
      *hit = hit1;
      *distance = distance1;
      CSG_SET_ENTER(hit1, 1);
      return FALSE;
    }
    distance2 = distance3;
    csg_copy_hit(&hit3, hit2);
  }
}
boolean
csg_intersection(position, vector, hit1, hit2, distance1, distance2,
                 hit, distance)
  xyz_ptr         position, vector;
  hit_ptr         hit1, hit2;
  real            distance1, distance2;
  hit_ptr        *hit;
  real           *distance;
{
  REG real        distance3;
  hit_struct      hit3;
  hit_ptr         hit_tmp;

  while (TRUE)
  {
    if (csg_enter(position, vector, hit2) == 0)
    {
      /* Hit 1st */
      *hit = hit1;
      *distance = distance1;
      CSG_SET_ENTER(hit1, csg_enter(position, vector, hit1));
      return FALSE;
    }
    hit3.nodes = 0;
    distance3 = object_intersect(hit1->data[hit1->nodes - 1].object,
				 position, vector, &hit3,
				 distance2 + threshold_distance);
    if (distance3 < distance2 + threshold_distance)
      /* Miss */
      return TRUE;
    hit_tmp = hit1;
    hit1 = hit2;
    hit2 = hit_tmp;
    distance1 = distance2;
    csg_copy_hit(&hit3, hit2);
    distance2 = distance3;
  }
}


real
object_intersect(intersect_object, position, vector, hit, min_distance)
  object_ptr      intersect_object;
  xyz_ptr         position, vector;
  hit_ptr         hit;
  real            min_distance;
{
  REG int         octant;
  REG real        distance;
  xyz_struct      tmp_position;

  tmp_position.x = position->x + min_distance * vector->x;
  tmp_position.y = position->y + min_distance * vector->y;
  tmp_position.z = position->z + min_distance * vector->z;
  FIND_OCTANT(octant, *vector);
  if (NOT octant_intersect(octant, &tmp_position, vector,
                           intersect_object->min, intersect_object->max))
    return 0.0;
  if (CHECK_BOUNDS(intersect_object->object_type))
    if (bound_intersect(&tmp_position, vector, intersect_object->min,
                        intersect_object->max) <= 0.0)
      return 0.0;
  /* Primitive objects */
  if (intersect_object->object_type != CSG_TYPE)
  {
    OBJECT_INTERSECT(distance, &tmp_position, vector, intersect_object);
    if (distance < threshold_distance)
      return 0.0;
    distance += min_distance;
    hit->nodes = 0;
    csg_add_hit(hit, distance, vector, intersect_object, &tree);
    return distance;
  }
  /* CSG objects */
  distance = csg_sec_intersect(intersect_object, position, vector, hit,
			       min_distance);
  if (distance < threshold_distance)
    return 0.0;
  csg_add_hit(hit, distance, vector, intersect_object, &tree);
  return distance;
}


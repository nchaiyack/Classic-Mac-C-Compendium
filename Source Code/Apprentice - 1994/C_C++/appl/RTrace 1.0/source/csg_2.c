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

extern tree_struct tree;
extern hit_struct hit_global;

/* Main CSG functions */

real
csg_sec_intersect(csg_object, position, vector, csg_hit, min_distance)
  object_ptr      csg_object;
  xyz_ptr         position, vector;
  hit_ptr         csg_hit;
  real            min_distance;
{
  real            distance;
  REG real        distance1, distance2;
  xyz_struct      new_position, new_vector;
  csg_ptr         csg;
  hit_ptr         hit1, hit2, hit;
  static hit_struct      hit1_list;
  hit_struct	  hit2_list;

  if (csg_object->transf != NULL)
  {
    transform(csg_object->transf, position, &new_position);
    transform_vector(csg_object->transf, position, vector, &new_position,
                     &new_vector);
    NORMALIZE(new_vector);
  } else
  {
    STRUCT_ASSIGN(new_position, *position);
    STRUCT_ASSIGN(new_vector, *vector);
  }
  REALINC(csg_tests);
  csg = (csg_ptr) csg_object->data;
  /* CSG intersection */
  hit1 = &hit1_list;
  hit2 = &hit2_list;
  hit1->nodes = 0;
  hit2->nodes = 0;
  POSINC(tree.depth);
  if (tree.depth >= CSG_NODES_MAX)
    runtime_abort("too big CSG TREE PATH");
  SAVE_TREE_PATH(PATH_LEFT);
  distance1 = object_intersect(object[csg->left], &new_position,
			       &new_vector, hit1, min_distance);
  if ((distance1 < min_distance) AND((csg->op == CSG_SUBTRACTION)
      OR(csg->op == CSG_INTERSECTION)))
  {
    POSDEC(tree.depth);
    return 0.0;
  }
  SAVE_TREE_PATH(PATH_RIGHT);
  distance2 = object_intersect(object[csg->right], &new_position,
			       &new_vector, hit2, min_distance);
  POSDEC(tree.depth);
  if ((distance2 < min_distance) AND((csg->op == CSG_INTERSECTION)
      OR((hit1->nodes == 0) AND(csg->op == CSG_UNION))))
    return 0.0;
  if (distance1 < min_distance)
    distance1 = INFINITY;
  if (distance2 < min_distance)
    distance2 = INFINITY;
  if ((distance1 > distance2) AND((csg->op == CSG_UNION)
      OR(csg->op == CSG_INTERSECTION)))
  {
    distance = distance2;
    distance2 = distance1;
    distance1 = distance;
    hit1 = &hit2_list;
    hit2 = &hit1_list;
  }
  switch(csg->op)
  {
  case CSG_UNION:
    if (csg_union(&new_position, &new_vector, hit1, hit2,
		  distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  case CSG_SUBTRACTION:
    if (csg_subtraction(&new_position, &new_vector, hit1, hit2,
			distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  case CSG_INTERSECTION:
    if (csg_intersection(&new_position, &new_vector, hit1, hit2,
			 distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  }
 if (distance < min_distance)
    return 0.0;
  csg_copy_hit(hit, csg_hit);
  if (csg_object->transf != NULL)
  {
    csg->position->x = new_position.x + distance * new_vector.x;
    csg->position->y = new_position.y + distance * new_vector.y;
    csg->position->z = new_position.z + distance * new_vector.z;
    return transform_distance(csg_object->inv_transf, distance,
                              &new_position, &new_vector, position);
  } else
    return distance;
}

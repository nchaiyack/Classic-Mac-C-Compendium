/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
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
 *    RAY TRACING - Patch (part1) - Version 7.3.2                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, August 1992            *
 **********************************************************************/

/***** Patch *****/
#include "patch.h"

static real     small_distance;
static xyz_struct k0, k1, k2, k3, k4, k5, k6, k7, p[4];
static patch_ptr patch;

real
patch_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        u, v, b0, b1, b2, b3, distance;
  xyz_struct      p0, p1, p2, p3, p4, p5, p6, p7, p8, pmin, pmax;

  REALINC(patch_tests);
  patch = (patch_ptr) object->data;
  STRUCT_ASSIGN(p[0], patch->p[3]->coords);
  STRUCT_ASSIGN(p[1], patch->p[4]->coords);
  STRUCT_ASSIGN(p[2], patch->p[7]->coords);
  STRUCT_ASSIGN(p[3], patch->p[8]->coords);
  STRUCT_ASSIGN(p0, p[0]);
  STRUCT_ASSIGN(p2, p[1]);
  STRUCT_ASSIGN(p6, p[2]);
  STRUCT_ASSIGN(p8, p[3]);
  /* Points 1 and 7 */
  u = 0.5;
  b0 = BLEND0(u);
  b1 = BLEND1(u);
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b1, k5);
  MULTIPLY(p[2], b0, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k0, k4, k5, k0);
  SUBTRACT(k2, k6, k7, k2);
  /* Point 1 */
  v = 0.0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p1.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p1.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p1.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Point 7 */
  v = 1.0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p7.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p7.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p7.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Points 3, 4 and 5 */
  v = 0.5;
  b0 = BLEND0(v);
  b1 = BLEND1(v);
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b0, k5);
  MULTIPLY(p[2], b1, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k1, k4, k6, k1);
  SUBTRACT(k3, k5, k7, k3);
  /* Point 3 */
  u = 0.0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p3.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p3.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p3.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 4 */
  u = 0.5;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p4.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p4.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p4.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 5 */
  u = 1.0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p5.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p5.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p5.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Intersect */
  small_distance = INFINITY;
  subpatch_enclose(&p0, &p1, &p3, &p4, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p0, &p1, &p3, &p4, 0.0, 0.0, 0.5, 1, position, vector);
  subpatch_enclose(&p1, &p2, &p4, &p5, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p1, &p2, &p4, &p5, 0.5, 0.0, 0.5, 1, position, vector);
  subpatch_enclose(&p3, &p4, &p6, &p7, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p3, &p4, &p6, &p7, 0.0, 0.5, 0.5, 1, position, vector);
  subpatch_enclose(&p4, &p5, &p7, &p8, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p4, &p5, &p7, &p8, 0.5, 0.5, 0.5, 1, position, vector);
  if (small_distance > INFINITY * 0.5)
    return 0.0;
  return small_distance;
}

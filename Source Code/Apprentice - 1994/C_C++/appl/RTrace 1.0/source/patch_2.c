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

#define BIG_DISTANCE(p0, p1) ((boolean) (\
SQR((p0).x - (p1).x) + SQR((p0).y - (p1).y) + SQR((p0).z - (p1).z) >=\
SQR(threshold_distance)))

static void
patch_bissect(p0, p2, p6, p8, u0, v0, delta, position, vector)
  xyz_ptr         p0, p2, p6, p8;
  real            u0, v0, delta;
  xyz_ptr         position, vector;
{
  REG real        u, v, b0, b1, b2, b3, distance, delta2;
  xyz_struct      p1, p3, p4, p5, p7, pmin, pmax, temp;

  delta2 = delta * 0.5;
  /* Points 1 and 7 */
  u = u0 + delta2;
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
  v = v0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p1.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p1.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p1.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Point 7 */
  v = v0 + delta;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p7.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p7.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p7.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Points 3, 4 and 5 */
  v = v0 + delta2;
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
  u = u0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p3.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p3.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p3.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 4 */
  u = u0 + delta2;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p4.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p4.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p4.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 5 */
  u = u0 + delta;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p5.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p5.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p5.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  subpatch_enclose(p0, &p1, &p3, &p4, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p0, p4))
      patch_bissect(p0, &p1, &p3, &p4, u0, v0, delta2, position, vector);
    else
    {
      temp.x = (p0->x + p4.x) * 0.5 - position->x;
      temp.y = (p0->y + p4.y) * 0.5 - position->y;
      temp.z = (p0->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 0.5;
        patch->v_hit = v0 + delta2 * 0.5;
      }
    }
  subpatch_enclose(&p1, p2, &p4, &p5, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p2, p4))
      patch_bissect(&p1, p2, &p4, &p5, u0 + delta2, v0, delta2,
                    position, vector);
    else
    {
      temp.x = (p2->x + p4.x) * 0.5 - position->x;
      temp.y = (p2->y + p4.y) * 0.5 - position->y;
      temp.z = (p2->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 1.5;
        patch->v_hit = v0 + delta2 * 0.5;
      }
    }
  subpatch_enclose(&p3, &p4, p6, &p7, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p6, p4))
      patch_bissect(&p3, &p4, p6, &p7, u0, v0 + delta2, delta2,
                    position, vector);
    else
    {
      temp.x = (p6->x + p4.x) * 0.5 - position->x;
      temp.y = (p6->y + p4.y) * 0.5 - position->y;
      temp.z = (p6->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 0.5;
        patch->v_hit = v0 + delta2 * 1.5;
      }
    }
  subpatch_enclose(&p4, &p5, &p7, p8, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p8, p4))
      patch_bissect(&p4, &p5, &p7, p8, u0 + delta2, v0 + delta2, delta2,
                    position, vector);
    else
    {
      temp.x = (p8->x + p4.x) * 0.5 - position->x;
      temp.y = (p8->y + p4.y) * 0.5 - position->y;
      temp.z = (p8->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 1.5;
        patch->v_hit = v0 + delta2 * 1.5;
      }
    }
}
void
patch_divide(p0, p2, p6, p8, u0, v0, delta, level, position, vector)
  xyz_ptr         p0, p2, p6, p8;
  real            u0, v0, delta;
  int             level;
  xyz_ptr         position, vector;
{
#define LEVEL_MAX (8)
  real            u, v, distance;
  REG real        b0, b1, b2, b3, delta2;
  xyz_struct      p1, p3, p4, p5, p7, pmin, pmax;

  delta2 = delta * 0.5;
  /* Points 1 and 7 */
  u = u0 + delta2;
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
  v = v0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p1.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p1.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p1.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Point 7 */
  v = v0 + delta;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p7.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p7.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p7.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Points 3, 4 and 5 */
  v = v0 + delta2;
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
  u = u0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p3.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p3.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p3.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 4 */
  u = u0 + delta2;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p4.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p4.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p4.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 5 */
  u = u0 + delta;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p5.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p5.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p5.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  subpatch_enclose(p0, &p1, &p3, &p4, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(p0, &p1, &p3, &p4, u0, v0, delta2, SUCC(level),
                   position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 0.5, v0 + delta2 * 0.5,
                              position, vector))
        patch_bissect(p0, &p1, &p3, &p4, u0, v0, delta2, position, vector);
    }
  subpatch_enclose(&p1, p2, &p4, &p5, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(&p1, p2, &p4, &p5, u0 + delta2, v0, delta2, SUCC(level),
                   position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 1.5, v0 + delta2 * 0.5,
                              position, vector))
        patch_bissect(&p1, p2, &p4, &p5, u0 + delta2, v0, delta2,
                      position, vector);
    }
  subpatch_enclose(&p3, &p4, p6, &p7, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(&p3, &p4, p6, &p7, u0, v0 + delta2, delta2, SUCC(level),
                   position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 0.5, v0 + delta2 * 1.5,
                              position, vector))
        patch_bissect(&p3, &p4, p6, &p7, u0, v0 + delta2, delta2,
                      position, vector);
    }
  subpatch_enclose(&p4, &p5, &p7, p8, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(&p4, &p5, &p7, p8, u0 + delta2, v0 + delta2, delta2,
                   SUCC(level), position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 1.5, v0 + delta2 * 1.5,
                              position, vector))
        patch_bissect(&p4, &p5, &p7, p8, u0 + delta2, v0 + delta2, delta2,
                      position, vector);
    }
#undef LEVEL_MAX
}

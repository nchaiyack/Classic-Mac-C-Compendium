/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - lights
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
 *    RAY TRACING - Light - Version 7.0                               *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1991           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1991           *
 **********************************************************************/

/***** Lights *****/
void
get_point_light()
{
  int             i;
  real            value;

  light[lights].light_type = POINT_LIGHT_TYPE;
  light[lights].data = NULL;
  get_valid(scene, &value, X_MIN, X_MAX, "LIGHT X");
  light[lights].coords.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LIGHT Y");
  light[lights].coords.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LIGHT Z");
  light[lights].coords.z = value;
  get_valid(scene, &value, -LIGHTING_FACTOR_MAX, LIGHTING_FACTOR_MAX,
            "LIGHT BRIGHTNESS Red");
  light[lights].brightness.r = ABS(value);
  light[lights].attenuation[0] = (value >= 0.0);
  get_valid(scene, &value, -LIGHTING_FACTOR_MAX, LIGHTING_FACTOR_MAX,
            "LIGHT BRIGHTNESS Green");
  light[lights].brightness.g = ABS(value);
  light[lights].attenuation[1] = (value >= 0.0);
  get_valid(scene, &value, -LIGHTING_FACTOR_MAX, LIGHTING_FACTOR_MAX,
            "LIGHT BRIGHTNESS Blue");
  light[lights].brightness.b = ABS(value);
  light[lights].attenuation[2] = (value >= 0.0);
  for (i = 0; i < LIGHT_CACHE_LEVEL_MAX; POSINC(i))
    light[lights].cache_id[i] = NO_OBJECTS;
}
void
get_dir_light()
{
  real            value;
  dir_light_ptr   light_ptr;

  get_point_light();
  light[lights].light_type = DIRECT_LIGHT_TYPE;
  ALLOCATE(light_ptr, dir_light_struct, 1);
  light[lights].data = (void_ptr) light_ptr;
  get_valid(scene, &value, X_MIN, X_MAX, "LIGHT VECTOR X");
  light_ptr->vector.x = -value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LIGHT VECTOR Y");
  light_ptr->vector.y = -value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LIGHT VECTOR Z");
  light_ptr->vector.z = -value;
  if (LENGTH(light_ptr->vector) <= ROUNDOFF)
    runtime_abort("no LIGHT Vector");
  NORMALIZE(light_ptr->vector);
  get_valid(scene, &value, 0.0, 180.0, "LIGHT ILLUMINATION Angle");
  light_ptr->cos_angle = MIN(1.0 - ROUNDOFF,
                             COS(DEGREE_TO_RADIAN(value)));
  light_ptr->t = 1.0 / (1.0 - light_ptr->cos_angle);
  get_valid(scene, &value, 1.0, LIGHTING_FACTOR_MAX, "LIGHT LIGHTING Factor");
  light_ptr->factor = 1.0 / value;
}
void
get_ext_light()
{
  real            value;
  ext_light_ptr   light_ptr;

  get_point_light();
  light[lights].light_type = EXTENDED_LIGHT_TYPE;
  ALLOCATE(light_ptr, ext_light_struct, 1);
  light[lights].data = (void_ptr) light_ptr;
  get_valid(scene, &value, 0.0, X_MAX, "LIGHT Radius");
  light_ptr->diameter = 2.0 * value;
  get_valid(scene, &value, 1.0, 256.0, "LIGHT Samples");
  light_ptr->samples = ROUND(value) - 1;
}

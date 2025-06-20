/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - textures
 *  David Buck          - textures
 *  Reid Judd		- portability
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
 *    RAY TRACING - Shade texture (part1) - Version 8.1.0             *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1991           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, December 1992          *
 **********************************************************************/


/***** Textures *****/
#define ADJUST(color)\
do {\
  (color).r = MIN(1.0, MAX(0.0, (color).r));\
  (color).g = MIN(1.0, MAX(0.0, (color).g));\
  (color).b = MIN(1.0, MAX(0.0, (color).b));\
} while (0)

#define SCALE(color, red, green, blue)\
do {\
  (color).r *= 2.0 * (red);\
  (color).g *= 2.0 * (green);\
  (color).b *= 2.0 * (blue);\
  ADJUST(color);\
} while (0)

void
interpolate(t, colors, color)
  real            t;
  rgb_ptr         colors, color;
{
  REG int         i1, i2;
  REG real        t1, t2;
  if (t < 0.0)
  {
    STRUCT_ASSIGN(*color, colors[0]);
    return;
  }
  if (t >= 1.0)
  {
    STRUCT_ASSIGN(*color, colors[255]);
    return;
  }
  t1 = t * 256.0;
  i1 = (int) t1;
  if (i1 < 255)
    i2 = SUCC(i1);
  else
  {
    i1 = 255;
    i2 = 0;
  }
  t1 = FRAC(t1);
  t2 = 1.0 - t1;
  color->r = t1 * colors[i1].r + t2 * colors[i2].r;
  color->g = t1 * colors[i1].g + t2 * colors[i2].g;
  color->b = t1 * colors[i1].b + t2 * colors[i2].b;
}
void
checker_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             x, y, z;
  checker_ptr     checker;

  checker = (checker_ptr) texture->data;
  x = position->x > 0.0 ? (int) position->x : (int) (1.0 - position->x);
  y = position->y > 0.0 ? (int) position->y : (int) (1.0 - position->y);
  z = position->z > 0.0 ? (int) position->z : (int) (1.0 - position->z);
  if (ODD(x + y + z))
    STRUCT_ASSIGN(*old_surface, *(surface[checker->surface_id]));
}
void
blotch_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            t;
  blotch_ptr      blotch;

  blotch = (blotch_ptr) texture->data;
  t = black_noise(position);
  if (t > blotch->scale)
  {
    t = (t - blotch->scale) / (1.0 - blotch->scale);
    old_surface->color.r = old_surface->color.r * (1.0 - t) +
      surface[blotch->surface_id]->color.r * t;
    old_surface->color.g = old_surface->color.g * (1.0 - t) +
      surface[blotch->surface_id]->color.g * t;
    old_surface->color.b = old_surface->color.b * (1.0 - t) +
      surface[blotch->surface_id]->color.b * t;
    ADJUST(old_surface->color);
  }
}
void
bump_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  xyz_struct      p;
  bump_ptr        bump;

  bump = (bump_ptr) texture->data;
  black_noise2(position, &p);
  normal->x += p.x * bump->scale;
  normal->y += p.y * bump->scale;
  normal->z += p.z * bump->scale;
  NORMALIZE(*normal);
}
void
marble_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            t;
  rgb_struct      color;
  marble_ptr      marble;

  marble = (marble_ptr) texture->data;
  t = marble_noise(position);
  if (marble->color != NULL)
  {
    interpolate(t, marble->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
void
fbm_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            t;
  rgb_struct      color;
  fbm_ptr         fbm;

  fbm = (fbm_ptr) texture->data;
  t = fbm_noise(position, fbm->omega, fbm->lambda, fbm->octaves);
  if (t < fbm->threshold)
    t = fbm->offset;
  else
    t = fbm->offset + fbm->scale * (t - fbm->threshold);
  if (fbm->color != NULL)
  {
    interpolate(t, fbm->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
void
fbm_bump_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  xyz_struct      p;
  fbm_bump_ptr    fbm_bump;

  fbm_bump = (fbm_bump_ptr) texture->data;
  fbm_noise2(position, fbm_bump->omega, fbm_bump->lambda, fbm_bump->octaves,
             &p);
  normal->x += (p.x + fbm_bump->offset) * fbm_bump->scale;
  normal->y += (p.y + fbm_bump->offset) * fbm_bump->scale;
  normal->z += (p.z + fbm_bump->offset) * fbm_bump->scale;
  NORMALIZE(*normal);
}
void
wood_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        r, g, b;
  REG real        brown_layer, green_layer;
  real            brown_noise, green_noise, green0_noise;
  real            chaos, t;
  wood_ptr        wood;

  wood = (wood_ptr) texture->data;
  chaos = chaos_noise(position, 7);
  t = SIN(SIN(8.0 * chaos + 7.0 * position->x + 3.0 * position->y));
  green_layer = ABS(t);
  brown_layer = green_layer;
  t = ABS(SIN(40.0 * chaos + 50.0 * position->z));
  brown_noise = 0.7 * t + 0.3;
  green_noise = 0.2 * t + 0.8;
  green0_noise = 0.15 * t + 0.85;
  g = POWER(ABS(brown_layer), 0.3);
  brown_layer = POWER(0.5 * (brown_layer + 1.0), 0.6) * brown_noise;
  green_layer = POWER(0.5 * (green_layer + 1.0), 0.6) * green_noise;
  r = ((wood->color.r + wood->color.b) * 0.5 * brown_layer +
       wood->color.g * green_layer) * g;
  b = ((wood->color.r + wood->color.b) * 0.25 * brown_layer +
       wood->color.g * green_layer) * g;
  g *= MAX(brown_layer, green_layer) * green0_noise;
  SCALE(old_surface->color, r, g, b);
}
void
round_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            k1, k2;
  xyz_struct      u, v;
  round_ptr       round;

  round = (round_ptr) texture->data;
  if (round->scale < ROUNDOFF)
    return;
  v.x = 0.0;
  v.y = 1.0;
  v.z = 0.0;
  if (ABS(DOT_PRODUCT(*normal, v)) > 1.0 - ROUNDOFF)
  {
    v.x = 1.0;
    v.y = 0.0;
  }
  CROSS_PRODUCT(u, *normal, v);
  NORMALIZE(u);
  CROSS_PRODUCT(v, u, *normal);
  k1 = black_noise(position) * 2.0 - 1.0;
  position->x += k1;
  position->y += k1;
  position->z += k1;
  k2 = black_noise(position) * 2.0 - 1.0;
  k2 *= SQRT(1.0 - SQR(k1)) * round->scale;
  k1 *= round->scale;
  normal->x += k1 * u.x + k2 * v.x;
  normal->y += k1 * u.y + k2 * v.y;
  normal->z += k1 * u.z + k2 * v.z;
  NORMALIZE(*normal);
}
void
bozo_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t;
  xyz_struct      temp;
  rgb_struct      color;
  bozo_ptr        bozo;

  bozo = (bozo_ptr) texture->data;
  if (bozo->turbulence > ROUNDOFF)
  {
    turbulence2(position, &temp);
    position->x += temp.x * bozo->turbulence;
    position->y += temp.y * bozo->turbulence;
    position->z += temp.z * bozo->turbulence;
  }
  t = black_noise(position);
  if (bozo->color != NULL)
  {
    interpolate(t, bozo->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  if (t < 0.4)
  {
    SCALE(old_surface->color, 1.0, 1.0, 1.0);
    return;
  }
  if (t < 0.6)
  {
    SCALE(old_surface->color, 0.5, 1.0, 0.5);
    return;
  }
  if (t < 0.8)
  {
    SCALE(old_surface->color, 0.5, 0.5, 1.0);
    return;
  }
  SCALE(old_surface->color, 1.0, 0.5, 0.5);
}
void
ripples_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             i;
  REG real        k, l, t;
  xyz_struct      temp;
  ripples_ptr     ripples;

  ripples = (ripples_ptr) texture->data;
  k = ripples->scale / (real) WAVES_MAX;
  for (i = 0; i < WAVES_MAX; POSINC(i))
  {
    temp.x = position->x - wave.source[i].x;
    temp.y = position->y - wave.source[i].y;
    temp.z = position->z - wave.source[i].z;
    l = LENGTH(temp);
    if (l < ROUNDOFF)
      l = 1.0;
    t = l * ripples->frequency + ripples->phase;
    t = cycloidal(t) * k / l;
    normal->x += temp.x * t;
    normal->y += temp.y * t;
    normal->z += temp.z * t;
  }
  NORMALIZE(*normal);
}
void
waves_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             i;
  REG real        k, l, t;
  xyz_struct      temp;
  waves_ptr       waves;

  waves = (waves_ptr) texture->data;
  k = waves->scale / (real) WAVES_MAX;
  for (i = 0; i < WAVES_MAX; POSINC(i))
  {
    temp.x = position->x - wave.source[i].x;
    temp.y = position->y - wave.source[i].y;
    temp.z = position->z - wave.source[i].z;
    l = LENGTH(temp);
    if (l < ROUNDOFF)
      l = 1.0;
    t = l * waves->frequency * wave.frequency[i] + waves->phase;
    t = cycloidal(t) * k * wave.frequency[i] / l;
    normal->x += temp.x * t;
    normal->y += temp.y * t;
    normal->z += temp.z * t;
  }
  NORMALIZE(*normal);
}
void
spotted_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t;
  rgb_struct      color;
  spotted_ptr     spotted;

  spotted = (spotted_ptr) texture->data;
  t = black_noise(position);
  if (spotted->color != NULL)
  {
    interpolate(t, spotted->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}

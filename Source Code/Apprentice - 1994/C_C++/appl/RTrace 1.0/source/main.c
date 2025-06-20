/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - stereo view mode
 *  Rodney Bogart       - depth of field (lens camera model)
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
#ifdef TURBOC
#define MAIN_MODULE
#endif
#ifndef lint
char            copyright[] =
"@(#) Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.\n\
All rights reserved.\n";
#endif
#include "defs.h"
#include "extern.h"

 int ytest;
 int xtest;

/**********************************************************************
 *    RAY TRACING - Version 7.3.1                                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1992              *
 **********************************************************************/

/***** Main *****/

void
runtime_abort(s)
  char_ptr        s;
{

#ifdef THINK_C

	/* On the mac, we handle a runtime abort with an error dialog */
	abortive_string_error (s);

#else

  WRITE(ERROR, "Error: runtime - %s\n", s);
  HALT;

#endif

}
real
tang(x)
  real            x;
{
  if (ABS(COS(x)) < ROUNDOFF)
    return (COS(x) >= 0.0 ? INFINITY : -INFINITY);
  return (real) tan((double) x);
}
vertex_ptr
vertex_pointer(v, vertices, vertex_top, vertex_bottom)
  REG int         v;
  int            *vertices;
  vertex_ptr     *vertex_top, *vertex_bottom;
{
  REG int         i;
  REG vertex_ptr  vertex;

  if (v > *vertices)
  {
    if (*vertices == 0)
    {
      ALLOCATE(*vertex_top, vertex_struct, 1);
      *vertex_bottom = *vertex_top;
      *vertices = 1;
    } else
    {
      *vertex_bottom = *vertex_top;
      for (i = 2; i <= *vertices; POSINC(i))
        *vertex_bottom = (vertex_ptr) ((*vertex_bottom)->next);
    }
    for (i = SUCC(*vertices); i <= v; POSINC(i))
    {
      ALLOCATE(vertex, vertex_struct, 1);
      (*vertex_bottom)->next = (void_ptr) vertex;
      *vertex_bottom = vertex;
    }
    *vertices = v;
    return *vertex_bottom;
  }
  vertex = *vertex_top;
  for (i = 2; i <= v; POSINC(i))
    vertex = (vertex_ptr) (vertex->next);
  return vertex;
}
static real     tangent_x, tangent_y;
static real     opening_x, opening_y;
static xyz_struct eye_pin_hole, screen_unit_x, screen_unit_y;

void
make_vector(p, x, y, delta)
  xyz_ptr         p;
  real            x, y, delta;
{
  REG real        kx, ky;
  xyz_struct      aperture;

  if (jittering_mode == 1)
  {
    x += JITTER * delta;
    y += JITTER * delta;
  }
  kx = x * opening_x - tangent_x;
  ky = y * opening_y - tangent_y;
  p->x = kx * screen_x.x + ky * screen_y.x + gaze.x;
  p->y = kx * screen_x.y + ky * screen_y.y + gaze.y;
  p->z = kx * screen_x.z + ky * screen_y.z + gaze.z;
  NORMALIZE(*p);
  if (focal_aperture > ROUNDOFF)
  {
    UNIT_CIRCLE_POINT(kx, ky);
    kx *= focal_aperture;
    ky *= focal_aperture;
    aperture.x = kx * screen_unit_x.x + ky * screen_unit_y.x;
    aperture.y = kx * screen_unit_x.y + ky * screen_unit_y.y;
    aperture.z = kx * screen_unit_x.z + ky * screen_unit_y.z;
    eye.x = eye_pin_hole.x + aperture.x;
    eye.y = eye_pin_hole.y + aperture.y;
    eye.z = eye_pin_hole.z + aperture.z;
    p->x = focal_distance * p->x - aperture.x;
    p->y = focal_distance * p->y - aperture.y;
    p->z = focal_distance * p->z - aperture.z;
    NORMALIZE(*p);
  }
}
void
transform(t, p1, p2)
  xyzw_ptr       t;
  xyz_ptr        p1, p2;
{
  REG real       w;

  p2->x = p1->x * t[0].x + p1->y * t[0].y + p1->z * t[0].z + t[0].w;
  p2->y = p1->x * t[1].x + p1->y * t[1].y + p1->z * t[1].z + t[1].w;
  p2->z = p1->x * t[2].x + p1->y * t[2].y + p1->z * t[2].z + t[2].w;
  if (t[3].w == 0.0)
    return;
  w = p1->x * t[3].x + p1->y * t[3].y + p1->z * t[3].z + t[3].w;
  if (ABS(w) < ROUNDOFF)
    return;
  w = 1.0 / w;
  p2->x *= w;
  p2->y *= w;
  p2->z *= w;
}
void
transform_vector(t, p1, v1, p2, v2)
  xyzw_ptr       t;
  xyz_ptr        p1, v1, p2, v2;
{
  xyz_struct     temp;

  temp.x = p1->x + v1->x;
  temp.y = p1->y + v1->y;
  temp.z = p1->z + v1->z;
  transform(t, &temp, v2);
  v2->x -= p2->x;
  v2->y -= p2->y;
  v2->z -= p2->z;
}
void
transform_normal_vector(t, p1, v1, v2)
  xyzw_ptr       t;
  xyz_ptr        p1, v1, v2;
{
  /*
   * REG real       w;
   */
  xyz_struct     p2, temp;

  STRUCT_ASSIGN(temp, *p1);
  /* Transpost matrix */
  p2.x = temp.x * t[0].x + temp.y * t[1].x + temp.z * t[2].x + t[3].x;
  p2.y = temp.x * t[0].y + temp.y * t[1].y + temp.z * t[2].y + t[3].y;
  p2.z = temp.x * t[0].z + temp.y * t[1].z + temp.z * t[2].z + t[3].z;
  /*
   * w = temp.x * t[0].w + temp.y * t[1].w + temp.z * t[2].w + t[3].w;
   * if (ABS(w) > ROUNDOFF)
   * {
   *   w = 1.0 / w;
   *   p2.x *= w;
   *   p2.y *= w;
   *   p2.z *= w;
   * }
   */
  temp.x += v1->x;
  temp.y += v1->y;
  temp.z += v1->z;
  /* Transpost matrix */
  v2->x = temp.x * t[0].x + temp.y * t[1].x + temp.z * t[2].x;
  v2->y = temp.x * t[0].y + temp.y * t[1].y + temp.z * t[2].y;
  v2->z = temp.x * t[0].z + temp.y * t[1].z + temp.z * t[2].z;
  /*
   * w = temp.x * t[0].w + temp.y * t[1].w + temp.z * t[2].w + t[3].w;
   * if (ABS(w) > ROUNDOFF)
   * {
   *   w = 1.0 / w;
   *   v2->x *= w;
   *   v2->y *= w;
   *   v2->z *= w;
   * }
   */
  v2->x -= p2.x;
  v2->y -= p2.y;
  v2->z -= p2.z;
}
real
transform_distance(t, distance, p1, v1, p2)
  xyzw_ptr       t;
  real           distance;
  xyz_ptr        p1, v1, p2;
{
  xyz_struct     temp1, temp2;

  temp1.x = p1->x + v1->x * distance;
  temp1.y = p1->y + v1->y * distance;
  temp1.z = p1->z + v1->z * distance;
  transform(t, &temp1, &temp2);
  temp2.x -= p2->x;
  temp2.y -= p2->y;
  temp2.z -= p2->z;
  return LENGTH(temp2);
}
void
inverse_transform(t1, t2)
  xyzw_ptr        t1, t2;
{
  real            a[4][4];
  real            b[4], c[4], temp[4];
  REG int         row_pivot, i, j, k;
  REG real        row_max, col_max, col_ratio, temp0;
  int             pivot[4];

  a[0][0] = t1[0].x;
  a[0][1] = t1[0].y;
  a[0][2] = t1[0].z;
  a[0][3] = t1[0].w;
  a[1][0] = t1[1].x;
  a[1][1] = t1[1].y;
  a[1][2] = t1[1].z;
  a[1][3] = t1[1].w;
  a[2][0] = t1[2].x;
  a[2][1] = t1[2].y;
  a[2][2] = t1[2].z;
  a[2][3] = t1[2].w;
  a[3][0] = t1[3].x;
  a[3][1] = t1[3].y;
  a[3][2] = t1[3].z;
  a[3][3] = t1[3].w;
  /* Pivoting */
  for (i = 0; i <= 3; POSINC(i))
  {
    pivot[i] = i;
    row_max = 0.0;
    for (j = 0; j <= 3; POSINC(j))
      row_max = MAX(row_max, ABS(a[i][j]));
    if (ABS(row_max) < ROUNDOFF)
      runtime_abort("TRANSFORMATION Matrix not invertible");
    temp[i] = row_max;
  }
  for (i = 0; i <= 2; POSINC(i))
  {
    col_max = ABS(a[i][i]) / temp[i];
    row_pivot = i;
    for (j = SUCC(i); j <= 3; POSINC(j))
    {
      col_ratio = ABS(a[j][i]) / temp[j];
      if (col_ratio > col_max)
      {
        col_max = col_ratio;
        row_pivot = j;
      }
    }
    if (ABS(col_max) < ROUNDOFF)
      runtime_abort("TRANSFORMATION Matrix not invertible");
    if (row_pivot > i)
    {
      j = pivot[row_pivot];
      pivot[row_pivot] = pivot[i];
      pivot[i] = j;
      temp0 = temp[row_pivot];
      temp[row_pivot] = temp[i];
      temp[i] = temp0;
      for (j = 0; j <= 3; POSINC(j))
      {
        temp0 = a[row_pivot][j];
        a[row_pivot][j] = a[i][j];
        a[i][j] = temp0;
      }
    }
    for (j = SUCC(i); j <= 3; POSINC(j))
    {
      a[j][i] /= a[i][i];
      temp0 = a[j][i];
      for (k = SUCC(i); k <= 3; POSINC(k))
        a[j][k] -= temp0 * a[i][k];
    }
  }
  if (ABS(a[3][3]) < ROUNDOFF)
    runtime_abort("TRANSFORMATION Matrix not invertible");
  /* Column 1 */
  b[0] = 1.0;
  b[1] = 0.0;
  b[2] = 0.0;
  b[3] = 0.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].x = c[0];
  t2[1].x = c[1];
  t2[2].x = c[2];
  t2[3].x = c[3];
  /* Column 2 */
  b[0] = 0.0;
  b[1] = 1.0;
  b[2] = 0.0;
  b[3] = 0.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].y = c[0];
  t2[1].y = c[1];
  t2[2].y = c[2];
  t2[3].y = c[3];
  /* Column 3 */
  b[0] = 0.0;
  b[1] = 0.0;
  b[2] = 1.0;
  b[3] = 0.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].z = c[0];
  t2[1].z = c[1];
  t2[2].z = c[2];
  t2[3].z = c[3];
  /* Column 4 */
  b[0] = 0.0;
  b[1] = 0.0;
  b[2] = 0.0;
  b[3] = 1.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].w = c[0];
  t2[1].w = c[1];
  t2[2].w = c[2];
  t2[3].w = c[3];
}
void
normalize_transform(t)
  xyzw_ptr        t;
{
  REG real        k;

  if (t[3].w == 0.0)
    return;
  if (ABS(1.0 - t[3].w) < ROUNDOFF)
  {
    if ((ABS(t[3].x) < ROUNDOFF) AND(ABS(t[3].y) < ROUNDOFF)
        AND(ABS(t[3].z) < ROUNDOFF))
      t[3].w = 0.0;
    return;
  }
  k = 1.0 / t[3].w;
  t[0].x *= k;
  t[0].y *= k;
  t[0].z *= k;
  t[0].w *= k;
  t[1].x *= k;
  t[1].y *= k;
  t[1].z *= k;
  t[1].w *= k;
  t[2].x *= k;
  t[2].y *= k;
  t[2].z *= k;
  t[2].w *= k;
  t[3].x *= k;
  t[3].y *= k;
  t[3].z *= k;
  if ((ABS(t[3].x) < ROUNDOFF) AND(ABS(t[3].y) < ROUNDOFF)
      AND(ABS(t[3].z) < ROUNDOFF))
    t[3].w = 0.0;
}
static void
make_view()
{
  REG real        kx, ky;
  xyz_struct      screen_xy;

  CROSS_PRODUCT(screen_x, gaze, up);
  NORMALIZE(screen_x);
  CROSS_PRODUCT(screen_y, gaze, screen_x);
  if (view_mode != 0)
  {
    if (stereo_separation < -ROUNDOFF)
      stereo_separation = gaze_distance * -stereo_separation;
    if (view_mode == STEREO_LEFT)
      kx = -0.5 * stereo_separation;
    if (view_mode == STEREO_RIGHT)
      kx = 0.5 * stereo_separation;
    eye.x += kx * screen_x.x;
    eye.y += kx * screen_x.y;
    eye.z += kx * screen_x.z;
    gaze.x = look.x - eye.x;
    gaze.y = look.y - eye.y;
    gaze.z = look.z - eye.z;
    gaze_distance = LENGTH(gaze);
    if (gaze_distance < ROUNDOFF)
      runtime_abort("EYE POINT equal to LOOK POINT in STEREO MODE");
    NORMALIZE(gaze);
    if (ABS(DOT_PRODUCT(gaze, up)) > COS(ANGLE_MIN))
      runtime_abort("bad UP VECTOR in STEREO MODE");
    CROSS_PRODUCT(screen_x, gaze, up);
    NORMALIZE(screen_x);
    CROSS_PRODUCT(screen_y, gaze, screen_x);
  }
  if (focal_distance < ROUNDOFF)
    focal_distance = gaze_distance;
  else
    gaze_distance = focal_distance;
  tangent_x = tang(view_angle_x);
  tangent_y = tang(view_angle_y);
  opening_x = 2.0 / (real) screen_size_x * tangent_x;
  opening_y = 2.0 / (real) screen_size_y * tangent_y;
  kx = gaze_distance * opening_x;
  ky = gaze_distance * opening_y;
  screen_xy.x = kx * screen_x.x + ky * screen_y.x;
  screen_xy.y = kx * screen_x.y + ky * screen_y.y;
  screen_xy.z = kx * screen_x.z + ky * screen_y.z;
  pixel_distance = LENGTH(screen_xy);
  threshold_distance = pixel_distance * DISTANCE_FACTOR;
  if (focal_aperture > ROUNDOFF)
  {
    STRUCT_ASSIGN(eye_pin_hole, eye);
    kx *= (real) screen_size_x;
    ky *= (real) screen_size_y;
    screen_unit_x.x = kx * screen_x.x;
    screen_unit_x.y = kx * screen_x.y;
    screen_unit_x.z = kx * screen_x.z;
    screen_unit_y.x = ky * screen_y.x;
    screen_unit_y.y = ky * screen_y.y;
    screen_unit_y.z = ky * screen_y.z;
  }
}
static void
init_globals()
{
  REG int         i;

  /* Rays */
  eye_rays = 0.0;
  shadow_rays = 0.0;
  reflected_rays = 0.0;
  refracted_rays = 0.0;
  ambient_rays = 0.0;
  shadow_hits = 0.0;
  shadow_cache_hits = 0.0;
  /* Intersection tests */
  octant_tests = 0.0;
  bound_tests = 0.0;
  sphere_tests = 0.0;
  box_tests = 0.0;
  patch_tests = 0.0;
  cone_tests = 0.0;
  polygon_tests = 0.0;
  triangle_tests = 0.0;
  text_tests = 0.0;
  csg_tests = 0.0;
  /* Others */
  shade_level = 0;
  shade_level_max = 0;
  ray_node = 0;
  ray_hits = 0.0;
  ray_cache_resets = 0.0;
  ray_cache_hits = 0.0;
  distributed_cache_resets = 0.0;
  distributed_cache_hits = 0.0;
  for (i = 0; i < RAY_SIZE_MAX; POSINC(i))
    ray_cache[i] = NO_OBJECTS;
  pqueue_resets = 0.0;
  pqueue_insertions = 0.0;
  pqueue_extractions = 0.0;
  pixel_divisions = 0.0;
  RANDOM_START;
  if (texture_mode != 0)
    init_texture();
}

#ifdef THINK_C

	/* On the mac, we define this macro (from picture.c) to computer rgb
		colors on the fly. */

#define INDEX(v)\
(ROUND(MAX(0.0, MIN((real) INDEX_MAX, (v) * (real) SUCC(INDEX_MAX)))))

/* externals */
extern Boolean		show_image_rendering;	/* if TRUE, draw image as we render */
extern WindowPtr	image_window;			/* the image window */
extern CGrafPtr		image_port;				/* offscreen port which contains the image */
extern Boolean		keep_image_in_memory;	/* TRUE if we keep a pixmap in memory */
extern Boolean		image_complete;			/* TRUE if the image is completely rendered */

	/* We also declare ray_trace to be void ray_trace() rather than the
		original static void ray_trace() so we can access it from
		macstuff.c */

void

#else

static void

#endif

ray_trace()
{
  int             x, x0, x1, x_count, y, step;
  real            total_time;
  rgb_struct      color;
  ray_struct      ray;
  pixel_ptr       temp_line;

#ifdef THINK_C

	RGBColor		this_color;			/* color of the current pixel */
	short			i;					/* index */
	long			*line_address;		/* base address of one bitmap line */
	unsigned char	red, green, blue;	/* RGB components of one pixel */
	long			pixel;				/* 32-bit pixel */
	long			pixel_count;		/* number of pixels we've drawn */
	GrafPtr			temp_port;			/* temporary storage for current GrafPort */
	char			string[80];			/* used to output to the log window */

#endif

  make_view();
  total_time = CPU_CLOCK;
 
#ifdef THINK_C

	/* Mac only encloses if do_enclose is TRUE */

	if (do_enclose)
		{
		enclose_all();

		/* Update the status window to say we're rendering */
		if (status_dialog_visible) set_status_text("\pRenderingÉ");
		}


	/* On the mac, we have a status bar to show the proportion of the
		image which has been rendered.  Here we set the maximum to the
		height of the image */
	set_progress_bar_max(screen_size_y);

#else
  
  enclose_all();

#endif

  if (verbose_mode > 1)
  {
    WRITE(results, "Info: enclosing - %g second(s) CPU time\n",
          (CPU_CLOCK - total_time) / 1000.0);
    FLUSH(results);
  }

  init_picture();
  init_globals();
  ray.level.r = 1.0;
  ray.level.g = 1.0;
  ray.level.b = 1.0;
  ray.inside = FALSE;
  step = -1;
  total_time = CPU_CLOCK;
 
 ytest = 10;
 xtest = 10; 
  
  
  for (y = 0; y <= screen_size_y; POSINC(y))
  {
    if (step < 0)
    {
      step = 1;
      x = 0;
      x1 = 1;
      x0 = 0;
    } else
    {
      step = -1;
      x = screen_size_x;
      x1 = screen_size_x;
      x0 = PRED(screen_size_x);
    }
    for (x_count = 0; x_count <= screen_size_x; POSINC(x_count))
    {


/***** (enable test to stop at a pixel for debugging) *****
if (y == ytest)
 {
   if (x == xtest) {
      int gotcha = 1;
   }
 }
*****/

      REALINC(eye_rays);
      make_vector(&(ray.vector), (real) x, (real) y, 1.0);
      if (intersect_all(NO_OBJECTS, &eye, &ray, &color) > 0.0)
        STRUCT_ASSIGN(new_line[x].color, color);
      else
        STRUCT_ASSIGN(new_line[x].color, back_color);
      new_line[x].id = ray_cache[0];
      if ((y > 0) AND(x_count > 0))
      {
        find_true_color(x1, y, &(old_line[x0]), &(old_line[x1]),
                        &(new_line[x0]), &(new_line[x1]), &(true_color[x1]));
        x1 += step;
        x0 += step;
      }
      x += step;
      
      PROCESS_MAC_EVENT
      
    }
    if (y > 0)
    {
    
#ifdef THINK_C

	/* Mac-specific code, executed once each row */
	
	/* Set the progress bar to reflect the portion of the image rendered */
	if (status_dialog_visible) set_progress_bar_value(y);

	/* Find base address of line in offscreen bitmap */
	if (keep_image_in_memory)
		line_address = (long *) ( (*(image_port->portPixMap))->baseAddr +
									((y-1) * screen_size_x * 4) );

	/* If we are showing the image in the window, or if we are saving it to
		an offscreen pixmap, draw/store this row. */
	if ((show_image_rendering) || (keep_image_in_memory))
		{
		
		/* If we're drawing to a window, set the port to it */
		if (show_image_rendering)
			{
			/* Remember the current port */
			GetPort (&temp_port);
		
			/* Set the port to the image window */
			SetPort (image_window);
			}
		
		for (i = 1; i <= screen_size_x; i++)
			{
			/* Get the pixel color as a 24-bit RGB color */
			red = (unsigned char) (INDEX(true_color[i].r));
			green = (unsigned char) (INDEX(true_color[i].g));
			blue = (unsigned char) (INDEX(true_color[i].b));

			/* If we are generating an offscreen bitmap, store this
				pixel directly into the offscreen bitmap */
			if (keep_image_in_memory)
				{
								
				pixel = (long) (red << 16) + (long) (green << 8) + (long) blue;;
				*(line_address + i - 1) = pixel;
				}

			/* If we are drawing in the window, translate the color
				to 48-bit RGB and plot the pixel */
			if (show_image_rendering)
				{
				this_color.red = (short) red << 8;
				this_color.green = (short) green << 8;
				this_color.blue = (short) blue << 8;
				SetCPixel (i-1, y-1, &this_color);
				}
			
			PROCESS_MAC_EVENT
			
			}
			
		/* If we're drawing to a window, restore the port */
		if (show_image_rendering)
			SetPort (temp_port);
			
		}
	
#endif

      line_picture();
      if (verbose_mode > 2)
      {
        WRITE(ERROR, "." );
        FLUSH(ERROR);
        if ( (y % 20) == 0 )            
          WRITE(ERROR, "Line %d completed\n", y);
      }
    }
    temp_line = old_line;
    old_line = new_line;
    new_line = temp_line;

  }


			/* update the amount of free memory */
			update_status_free_memory();

  if (picture != OUTPUT)
    CLOSE(picture);
  if (background_mode == 1)
    CLOSE(background);
  if (raw_mode == 1)
    CLOSE(raw_picture);
  if (verbose_mode < 2)
    return;
  WRITE(results, "Resolution %dx%d\n", screen_size_x, screen_size_y);
  WRITE(results, "Total eye rays:          %-0.0f\n", eye_rays);
  WRITE(results, "Total shadow rays:       %-0.0f\n", shadow_rays);
  if (reflected_rays > 0)
    WRITE(results, "Total reflected rays:    %-0.0f\n", reflected_rays);
  if (refracted_rays > 0)
    WRITE(results, "Total refracted rays:    %-0.0f\n", refracted_rays);
  if (ambient_rays > 0)
    WRITE(results, "Total ambient rays:      %-0.0f\n", ambient_rays);
  WRITE(results, "Total shadow hits:       %-0.0f\n", shadow_hits);
  WRITE(results, "Total shadow cache hits: %-0.0f\n", shadow_cache_hits);
  WRITE(results, "Total ray hits:          %-0.0f\n", ray_hits);
  WRITE(results, "Total ray cache resets:  %-0.0f\n", ray_cache_resets);
  WRITE(results, "Total ray cache hits:    %-0.0f\n", ray_cache_hits);
  if (distributed_cache_mode != 0)
  {
    WRITE(results, "Total amb. cache resets: %-0.0f\n",
          distributed_cache_resets);
    WRITE(results, "Total amb. cache hits:   %-0.0f\n",
          distributed_cache_hits);
  }
  WRITE(results, "Total queue resets:      %-0.0f\n", pqueue_resets);
  WRITE(results, "Total queue insertions:  %-0.0f\n", pqueue_insertions);
  WRITE(results, "Total queue extractions: %-0.0f\n", pqueue_extractions);
  WRITE(results, "Total pixel divisions:   %-0.0f\n", pixel_divisions);
  WRITE(results, "Maximum shading level:   %d\n", shade_level_max);
  WRITE(results, "%d Object(s), %d Light(s), %d Surface(s)\n",
        objects, lights, surfaces);
  WRITE(results, "Total octant inclusion tests:      %-0.0f\n", octant_tests);
  if (bound_tests > 0.0)
    WRITE(results, "Total BV intersection tests:       %-0.0f\n",
	  bound_tests);
  if (sphere_tests > 0.0)
    WRITE(results, "Total sphere intersection tests:   %-0.0f\n",
          sphere_tests);
  if (box_tests > 0.0)
    WRITE(results, "Total box intersection tests:      %-0.0f\n",
          box_tests);
  if (patch_tests > 0.0)
    WRITE(results, "Total patch intersection tests:    %-0.0f\n",
          patch_tests);
  if (cone_tests > 0.0)
    WRITE(results, "Total cone intersection tests:     %-0.0f\n",
          cone_tests);
  if (polygon_tests > 0.0)
    WRITE(results, "Total polygon intersection tests:  %-0.0f\n",
          polygon_tests);
  if (triangle_tests > 0.0)
    WRITE(results, "Total triangle intersection tests: %-0.0f\n",
          triangle_tests);
  if (text_tests > 0.0)
    WRITE(results, "Total text intersection tests:     %-0.0f\n",
          text_tests);
  if (csg_tests > 0.0)
    WRITE(results, "Total CSG intersection tests:      %-0.0f\n",
          csg_tests);
  if (list_tests > 0.0)
    WRITE(results, "Total list intersection tests:     %-0.0f\n",
          list_tests);
  WRITE(results, "Info: tracing - %g second(s) CPU time\n",
        (CPU_CLOCK - total_time) / 1000.0);
  FLUSH(results);


			/* update the amount of free memory */
			update_status_free_memory();

}
/***** Main program *****/
void
main(parameters, parameter)
  int             parameters;
  char_ptr        parameter[];
{

#ifdef THINK_C

  /* the Mac does things a little differently here.  It initializes
     RTrace as a Macintosh application, and then it enters a standard
     Macintosh event loop.  It allows the user to open a .sff file using
     the Open... menu, and then it prompts for the RTrace options with
     a dialog box.  Finally, it generates a command line and feeds it
     to RTrace. */

  /* Go into the Mac segment (never returns) */
  do_mac();

#endif

  get_parameters(PRED(parameters), &(parameter[1]));
  get_scene();                  /* Scene in    */
  ray_trace();                  /* Picture out */
  EXIT;                         /* Quit        */
}

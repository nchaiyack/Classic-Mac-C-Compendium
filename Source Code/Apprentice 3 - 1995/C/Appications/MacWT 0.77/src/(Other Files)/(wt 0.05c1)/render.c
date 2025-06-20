/*
**  wt -- a 3d game engine
**
**  Copyright (C) 1994 by Chris Laurel
**  email:  claurel@mr.net
**  snail mail:  Chris Laurel, 5700 W Lake St #208,  St. Louis Park, MN  55416
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "wt.h"
#include "error.h"
#include "fixed.h"
#include "wtmem.h"
#include "table.h"
#include "list.h"
#include "view.h"
#include "framebuf.h"
#include "texture.h"
#include "world.h"
#include "object.h"
#include "render.h"


/* These macros convert 16.16 fixed point numbers to and from a 2.30 format.
**   The extra fractional precision is needed when doing doing 1 / distance
**   calculations for perspective.
*/
#define TO_FIX_2_30(f)   ((f) << 14)
#define FROM_FIX_2_30(f) ((f) >> 14) 
#define TO_FIX_8_24(f)   ((f) << 8)
#define FROM_FIX_8_24(f) ((f) >> 8) 

#define MAX_ERROR  view_width


/*** typedefs for wall event lists ***/

typedef struct {
     Wall *wall;
     fixed z, dz;
     Boolean is_back_view;
} Wall_start;

typedef Wall *Wall_end;

#define MAX_WALL_EVENTS 30
typedef struct {
     int n_events;
     Wall_start events[MAX_WALL_EVENTS];
} Wall_start_list;

typedef struct {
     int n_events;
     Wall_end events[MAX_WALL_EVENTS];
} Wall_end_list;

typedef struct {
     Boolean is_back_view;
     Wall *wall;
     Boolean visible;
     fixed pstart1, pend1, pstart2, pend2;
     fixed dpstart1, dpend1, dpstart2, dpend2;
     fixed z;
     fixed dz;
} Active_wall;


/*** typedefs for object event lists ***/

typedef struct {
     Object *o;
     fixed z;
     fixed first_slice;
} Object_start;

typedef Object *Object_end;

#define MAX_OBJECT_EVENTS 10
typedef struct {
     int n_events;
     Object_start events[MAX_OBJECT_EVENTS];
} Object_start_list;

typedef struct {
     int n_events;
     Object_end events[MAX_OBJECT_EVENTS];
} Object_end_list;

typedef struct {
     Object *o;
     fixed x, dx;
     fixed z;
} Active_object;

/***/

typedef struct {
     fixed screen_dy, screen_dx;
     fixed view_sin, view_cos;
     fixed sin_dx, cos_dx;
     fixed *sin_tab, *cos_tab, *row_view;
} View_constants;


static void transform_vertices(World *w, View *view);
static void clip_walls(World *w, View *v);
static void add_wall_events(View *v, Wall *wall,
			    fixed x1, fixed px1, fixed x2, fixed px2);
static void render_walls(World *w, View *v);
static int add_events(Active_wall *active, int n_active, int column);
static Boolean wall_obscured(Vertex *common, Vertex *v1, Vertex *v2);
static int remove_events(Active_wall *active, int n_active, int column);
static void add_objects(World *w, View *v);
static int add_obj_events(Active_object *active, int n_active, int column);
static int remove_obj_events(Active_object *active, int n_active, int column);

static void draw_floor_slices(Region *r, fixed start, fixed end, View *v,
			      int column);
static void draw_ceiling_slices(Region *r, fixed start, fixed end,
				View *v, int column);
static fixed wall_ray_intersection(fixed Vx, fixed Vy, Wall *wall);
static void init_buffers(void);
static void calc_view_constants(View *v, int screen_width, int screen_height);
static void do_walls(Active_wall *active, int n_active,
		     Active_object *active_obj, int n_active_obj,
		     int column, View *v,
		     fixed Vx, fixed Vy);

/* Event lists */
static Wall_start_list   *start_events     = NULL;
static Wall_end_list     *end_events       = NULL;
static Object_start_list *obj_start_events = NULL;
static Object_end_list   *obj_end_events   = NULL;

static Framebuffer *fb = NULL;
static int *fb_rows = NULL;
static int view_width, view_height;
static View_constants view_constants;


#if defined(__GNUC__) && defined(ARCH_i86) && !defined(DEBUG)
/* It would be a Bad Thing to use the Intel assembly functions while debugging,
**   as they clobber the frame pointer.
*/
#include "slice-gas86.c"
#else
#include "slice.c"
#endif


void init_renderer(int fb_width, int fb_height)
{
     int i;


     /* Allocate space for event lists. */
     start_events     = wtmalloc(sizeof(Wall_start_list) * (fb_width + 1));
     end_events       = wtmalloc(sizeof(Wall_end_list) * (fb_width + 1));
     obj_start_events = wtmalloc(sizeof(Object_start_list) * (fb_width + 1));
     obj_end_events   = wtmalloc(sizeof(Object_end_list) * (fb_width + 1));

     fb = new_framebuffer(fb_width, fb_height);

     /* Precalculate the offsets of rows in the frame buffer--this is another
     **   'avoid multiplication' optimization.
     */
     fb_rows = wtmalloc(sizeof(int) * fb_height);
     for (i = 0; i < fb_height; i++)
	  fb_rows[i] = i * fb_width;

     view_width = fb->fb_width;
     view_height = fb->fb_height;
}


Framebuffer *render(World *w, View *v)
{
     init_buffers();
     calc_view_constants(v, view_width, view_height);
     transform_vertices(w, v);
     clip_walls(w, v);
     add_objects(w, v);
     clear_framebuffer(fb);
     render_walls(w, v);

     return fb;
}


static void transform_vertices(World *w, View *view)
{
     Vertex *vertex;
     fixed view_sin, view_cos;
     int i;


     vertex = TABLE_ELEMENTS(w->vertices, Vertex);
     view_sin = view_constants.view_sin;
     view_cos = view_constants.view_cos;

     for (i = 0; i < TABLE_SIZE(w->vertices); i++, vertex++) {
	  fixed x = vertex->x - view->x;
	  fixed y = vertex->y - view->y;

	  vertex->tx = fixmul(x, view_cos) - fixmul(y, view_sin);
	  vertex->ty = fixmul(x, view_sin) + fixmul(y, view_cos);
	  if (vertex->tx > view->eye_distance)
	       /* project point onto view plane */
	       vertex->proj = fixdiv(vertex->ty, vertex->tx);
     }
}


static void clip_walls(World *w, View *v)
{
     Wall *wall = (Wall *) w->walls->table;
     int i;

     for (i = 0; i < TABLE_SIZE(w->walls); i++, wall++) {
	  fixed x1, y1, px1, x2, y2, px2;
	  unsigned int outcode1, outcode2;

	  x1 = wall->vertex1->tx;
	  x2 = wall->vertex2->tx;
	  /* See if the wall lies completely behid the view plane. */
	  if (x1 < v->eye_distance && x2 < v->eye_distance)
	       continue;

	  y1 = wall->vertex1->ty;
	  px1 = wall->vertex1->proj;
	  y2 = wall->vertex2->ty;
	  px2 = wall->vertex2->proj;

	  /*** Clipping ***/

   	  /* First, clip to the view plane (line, really, since we're
	  **   working in only two dimensions.)
	  */
	  if (x1 <= v->eye_distance) {
	       /* be careful for division overflow */
	       if (x2 - x1 < FIXED_EPSILON)
		    continue;
	       y1 = y1 + fixmul(v->eye_distance - x1,
				fixdiv(y2 - y1, x2 - x1));
	       px1 = y1;
	       x1 = v->eye_distance;
	  }
	  if (x2 <= v->eye_distance) {
	       if (x1 - x2 < FIXED_EPSILON)
		    continue;
	       y2 = y2 + fixmul(v->eye_distance - x2,
				fixdiv(y1 - y2, x1 - x2));
	       px2 = y2;
	       x2 = v->eye_distance;
	  }

	  /* Now, clip to the sides of the view polygon. */
	  outcode1 = FIXED_SIGN(v->view_plane_size + px1);
	  outcode1 |= FIXED_SIGN(v->view_plane_size - px1) << 1;
	  outcode2 = FIXED_SIGN(v->view_plane_size + px2);
	  outcode2 |= FIXED_SIGN(v->view_plane_size - px2) << 1;

	  /* trivial reject */
	  if ((outcode1 & outcode2) != 0)
	       continue;
	  /* check for trivial accept */
	  if ((outcode1 | outcode2) != 0) {
	       /* Damn . . . we need to clip. */
	       fixed base_slope, slope, denom, y_diff;

	       denom = (x2 - x1);
	       if (FIXED_ABS(denom) < FIXED_EPSILON) {
		    if (denom < 0)
			 base_slope = FIXED_MIN + v->view_plane_size;
		    else
			 base_slope = FIXED_MAX - v->view_plane_size;
	       } else
		    base_slope = fixdiv(y2 - y1, denom);

	       if (outcode1 == 1) {
		    px1 = -v->view_plane_size;
		    y_diff = y1 - fixmul(x1, -v->view_plane_size);
		    slope = base_slope + v->view_plane_size;
		    if (FIXED_ABS(slope) > FIXED_EPSILON)
			 x1 -= fixdiv(y_diff, slope);
		    else
			 x1 = FIXED_MAX;
	       } else if (outcode1 == 2) {
		    px1 = v->view_plane_size;
		    y_diff = y1 - fixmul(x1, v->view_plane_size);
		    slope = base_slope - v->view_plane_size;
		    if (FIXED_ABS(slope) > FIXED_EPSILON)
			 x1 -= fixdiv(y_diff, slope);
		    else
			 x1 = FIXED_MAX;
	       }

	       if (outcode2 == 1) {
		    px2 = -v->view_plane_size;
		    y_diff = y2 - fixmul(x2, -v->view_plane_size);
		    slope = base_slope + v->view_plane_size;
		    if (FIXED_ABS(slope) > FIXED_EPSILON)
			 x2 -= fixdiv(y_diff, slope);
		    else
			 x2 = FIXED_MAX;
	       } else if (outcode2 == 2) {
		    px2 = v->view_plane_size;
		    y_diff = y2 - fixmul(x2, v->view_plane_size);
		    slope = base_slope - v->view_plane_size;
		    if (FIXED_ABS(slope) > FIXED_EPSILON)
			 x2 -= fixdiv(y_diff, slope);
		    else
			 x2 = FIXED_MAX;
	       }
	  }

	  add_wall_events(v, wall, x1, px1, x2, px2);
     }
}


/* Add a wall to the event list--one event is added for the start of the
**   wall, and another is added to mark the end of the wall.
*/
static void add_wall_events(View *v, Wall *wall,
			    fixed x1, fixed px1, fixed x2, fixed px2)
{
     int fb1, fb2;
     fixed z1, z2;
     Wall_start *event;


     /* convert to frame buffer coordinates */
     px1 = fixdiv(px1, view_constants.screen_dx + 1);
     px2 = fixdiv(px2, view_constants.screen_dx + 1);
     fb1 = FIXED_TO_INT(px1) + (view_width >> 1);
     fb2 = FIXED_TO_INT(px2) + (view_width >> 1);

     /* There's no need to deal with walls that start and end in the same
     **   screen column.  In a properly contructed world, we're guaranteed
     **   that throwing them away won't leave any gaps.
     */
     if (fb1 == fb2)
	  return;

     /* Here we use a 2.30 fixed point format.  The result of this calculation
     **   is always between 1 and zero, as the distance can never be less
     **   than the view plane distance.  The extra fractional bits are
     **   critical for the inverses.  Note that using 2.30 restricts the
     **   size of the view plane to something less than 2.
     */
     z1 = fixdiv(TO_FIX_2_30(v->eye_distance), x1);
     z2 = fixdiv(TO_FIX_2_30(v->eye_distance), x2);

     if (fb1 < fb2) {

	  event = &start_events[fb1].events[start_events[fb1].n_events];

	  event->wall = wall;
	  event->z = z1;
	  event->dz = fixdiv(z2 - z1, INT_TO_FIXED(fb2 - fb1));
	  event->is_back_view = False;
	  start_events[fb1].n_events++;

	  end_events[fb2].events[end_events[fb2].n_events] = wall;
	  end_events[fb2].n_events++;

     } else {

	  event = &start_events[fb2].events[start_events[fb2].n_events];

	  event->wall = wall;
	  event->z = z2;
	  event->dz = fixdiv(z1 - z2, INT_TO_FIXED(fb1 - fb2));
	  event->is_back_view = True;
	  start_events[fb2].n_events++;

	  end_events[fb1].events[end_events[fb1].n_events] = wall;
	  end_events[fb1].n_events++;

     }
}


static void add_objects(World *w, View *v)
{
     List *l;
     fixed view_sin, view_cos;


     view_sin = view_constants.view_sin;
     view_cos = view_constants.view_cos;

     for (l = w->objects; l->next != NULL; l = l->next) {
	  fixed x, y, z;
	  fixed tx, ty;
	  fixed height, width;
	  Object *o = LIST_NODE(l, Object *);

	  
	  /* Skip the object if it is invisible. */
	  if (o->image == NULL)
	       return;

	  /* Convert object coordinates to fixed point and transform. */
	  x = FLOAT_TO_FIXED(o->x) - v->x;
	  y = FLOAT_TO_FIXED(o->y) - v->y;
	  z = FLOAT_TO_FIXED(o->z);
	  height = FLOAT_TO_FIXED(o->height);
	  width  = FLOAT_TO_FIXED(o->xsize);
	  /* Rotate into viewer's coordinate system. */
	  tx = fixmul(x, view_cos) - fixmul(y, view_sin);
	  ty = fixmul(x, view_sin) + fixmul(y, view_cos);

	  /* Only worry about the object if it is in front of the view plane */
	  if (tx > v->eye_distance + FIXED_EPSILON) {
	       int fb1, fb2;
	       fixed pstart, pend;
	       fixed z;

	       /* Project the object onto the view plane . . . */
	       pstart = fixdiv(ty - FIXED_HALF(width), tx);
	       pend   = fixdiv(ty + FIXED_HALF(width), tx);

	       /* Convert to frame buffer coordinates. */
	       pstart = fixdiv(pstart, view_constants.screen_dx + 1);
	       pend   = fixdiv(pend,   view_constants.screen_dx + 1);
	       fb1 = FIXED_TO_INT(pstart) + (view_width >> 1);
	       fb2 = FIXED_TO_INT(pend)   + (view_width >> 1) - 1;
	  
	       if (fb2 >= 0 && fb1 < view_width) {
		    Object_start_list *start_list;
		    Object_end_list   *end_list;
		    Object_start      *start;
		    fixed             first_slice = FIXED_ZERO;

		    z = fixdiv(TO_FIX_2_30(v->eye_distance), tx);
		    if (fb1 < 0) {
			 first_slice = fixdiv(INT_TO_FIXED(-fb1),
					      INT_TO_FIXED(fb2 - fb1));
			 fb1 = 0;
		    }
		    start_list = &obj_start_events[fb1];

		    if (fb2 >= view_width)
			 fb2 = view_width - 1;
		    end_list   = &obj_end_events[fb2];

		    start = &start_list->events[start_list->n_events];
		    start->z = z;
		    start->o = o;
		    start->first_slice = first_slice;
		    end_list->events[end_list->n_events] = o;

		    start_list->n_events++;
		    end_list->n_events++;
	       }
	  }
     }
}


static void render_walls(World *w, View *v)
{
     static int last_wall_count = 0;
     static Active_wall *active;
     static Active_object *active_obj;
     int column;
     int active_count = 0, active_obj_count = 0;
     fixed Vx, Vy, dVy;


     /* Make sure that the active list is large enough to hold all the
     **   walls in a world.
     */
     if (last_wall_count != TABLE_SIZE(w->walls)) {
	  last_wall_count = TABLE_SIZE(w->walls);
	  if (active == NULL)
	       active = wtmalloc(sizeof(Active_wall) * last_wall_count);
	  else
	       active = wtrealloc(active,
				  sizeof(Active_wall) * last_wall_count);

     }
     if (active_obj == NULL)
	  active_obj = wtmalloc(sizeof(Active_object) * 100);

     /* Set up for fast calculation of view rays. */
     Vx = v->eye_distance;
     Vy = -v->view_plane_size;
     dVy = fixdiv(FIXED_SCALE(v->view_plane_size, 2),
                  INT_TO_FIXED(view_width));

     for (column = 0; column < view_width; column++) {
	  Active_wall   *current, *last;
	  Active_object *current_obj, *last_obj;

	  active_count     = add_events(active, active_count, column);
	  active_obj_count = add_obj_events(active_obj, active_obj_count,
					    column);

	  do_walls(active, active_count,
		   active_obj, active_obj_count,
		   column, v, Vx, Vy);

	  active_count     = remove_events(active, active_count, column);
	  active_obj_count = remove_obj_events(active_obj, active_obj_count,
					       column);

	  /* Keep track of distances of walls in the active list.  Notice that
	  **   we're not actually tracking the distances of walls, but 
	  **   1 / distance instead.  That's because we can linearly
	  **   interpolate 1 / distance.  Also, most calculations that
	  **   use distance are really using 1 / distance (i.e. distance
	  **   appears in the denominator.
	  */
	  last = active + active_count;
	  for (current = active; current < last; current++) {
	       current->z += current->dz;
	       if (current->visible) {
		    current->pstart1 += current->dpstart1;
		    current->pend1 += current->dpend1;
		    current->pstart2 += current->dpstart2;
		    current->pend2 += current->dpend2;
	       }
	  }

	  last_obj = active_obj + active_obj_count;
	  for (current_obj = active_obj; current_obj < last_obj;
	       current_obj++)
	       current_obj->x += current_obj->dx;

	  Vy += dVy;
     }

}


/* Add new walls to the active list.  The active list is kept
**   depth sorted.  We have to be careful here.  Correct depth
**   ordering of the walls is vital for rendering.  At corners
**   we have two or more walls at the same distance; however,
**   there is still a correct and incorrect ordering.  If one
**   wall is obscured by another, the visible wall must be placed
**   in front in the list.
*/
static int add_events(Active_wall *active, int n_active, int column)
{
     int i, j;
     Wall_start *event;
     Wall *wall;
     fixed z;

     for (i = 0; i < start_events[column].n_events; i++) {
	  event = start_events[column].events + i;

	  wall = event->wall;
	  z = event->z;

	  for (j = 0; j < n_active; j++) {
	       Wall *wall2 = active[j].wall;
	       Vertex *common, *v1, *v2;

	       if (z < active[j].z - MAX_ERROR)
		    continue;
	       else if (z > active[j].z + MAX_ERROR)
		    break;

	       /* See if the walls share a vertex. */
	       if (wall->vertex1 == wall2->vertex1) {
		    common = wall->vertex1;
		    v1 = wall->vertex2;
		    v2 = wall2->vertex2;
	       } else if (wall->vertex1 == wall2->vertex2) {
		    common = wall->vertex1;
		    v1 = wall->vertex2;
		    v2 = wall2->vertex1;
	       } else if (wall->vertex2 == wall2->vertex1) {
		    common = wall->vertex2;
		    v1 = wall->vertex1;
		    v2 = wall2->vertex2;
	       } else if (wall->vertex2 == wall2->vertex2) {
		    common = wall->vertex2;
		    v1 = wall->vertex1;
		    v2 = wall2->vertex1;
	       } else {
		    /* We have two walls which are really close
		    **   together, but share no vertices.  Because
		    **   of roundoff error, we don't know for certain
		    **   which one is really in front.  Ideally, this
		    **   situation will be avoided by creating worldfiles
		    **   which don't place non-adjoining walls extremely
		    **   close together.
		    */
		    if (z > active[j].z)
			 break;
		    else
			 continue;
	       }

	       if (!wall_obscured(common, v1, v2) &&
		   wall_obscured(common, v2, v1))
		    break;
	  }

	  /* Insert the wall into the active list. */
	  memmove(active + j + 1, active + j,
		  sizeof(Active_wall) * (n_active - j));
	  active[j].wall = wall;
	  active[j].z = z;
	  active[j].dz = event->dz;
	  active[j].visible = False;
	  active[j].is_back_view = event->is_back_view;
	  n_active++;
     }

     return n_active;
}


/* Insert an object into a depth sorted list. */
static int add_obj_events(Active_object *active, int n_active, int column)
{
     int i, j;
     
     for (i = 0; i < obj_start_events[column].n_events; i++) {
	  Object_start *event = obj_start_events[column].events + i;
	  fixed z = event->z;
	  Object *o = event->o;

	  for (j = 0; j < n_active && z < active[j].z; j++);
	  
	  memmove(active + j + 1, active + j,
		  sizeof(Active_object) * (n_active - j));
	  active[j].z = z;
	  active[j].o = o;
	  if (event->first_slice == FIXED_ZERO)
	       active[j].x = FIXED_ZERO;
	  else
	       active[j].x = FIXED_SCALE(event->first_slice, o->image->width);
	  active[j].dx =
	       fixdiv(INT_TO_FIXED(o->image->width * 2),
		      fixmul(FLOAT_TO_FIXED(o->xsize),
			     FIXED_SCALE(FROM_FIX_2_30(z), view_width)));
	  
	  n_active++;
     }
	  
     return n_active;
}


     

/* Determine whether wall 1 is obscured by wall 2 from the view point.
**   This will be the case if a halfplane defined by wall 1 contains both the
**   view point and wall 2.  Wall 1 is defined by the points common and v1;
**   wall2 is defined by command and v2.  Note that this function uses the
**   transformed coordinates of the vertices, so the view point and view
**   direction need not be passed explicitly.
*/
static Boolean wall_obscured(Vertex *common, Vertex *v1, Vertex *v2)
{
     fixed x1, y1, x2, y2;
     unsigned int sign1, sign2;


     x1 = common->tx - v1->tx;
     y1 = common->ty - v1->ty;
     x2 = common->tx - v2->tx;
     y2 = common->ty - v2->ty;

     /* There's some tricky stuff done here to try to find the signs of
     **   cross products without actually doing any multiplication.  I'm
     **   really not sure if avoiding a few multiplies is worth the extra
     **   overhead of sign checking, but the profiler shows this function as
     **   taking a surprisingly small percentage of execution time.
     */
     if (FIXED_PRODUCT_SIGN(x1, y2) ^ FIXED_PRODUCT_SIGN(x2, y1))
	  sign1 = FIXED_PRODUCT_SIGN(x1, y2);
     else
	  sign1 = FIXED_SIGN(fixmul(x1, y2) - fixmul(x2, y1));
     if (FIXED_PRODUCT_SIGN(x1, common->ty) ^
	 FIXED_PRODUCT_SIGN(common->tx, y1))
	  sign2 = FIXED_PRODUCT_SIGN(x1, common->ty);
     else
	  sign2 = FIXED_SIGN(fixmul(x1, common->ty) - fixmul(common->tx, y1));
     
     if (sign1 ^ sign2)
	  return False;
     else
	  return True;
}


/* Remove walls from the active list.  Return the number of remaining
** active walls.
*/
static int remove_events(Active_wall *active, int n_active, int column)
{
     int i, j;
     Wall *wall;


     /* This is really inefficient, so I hope these event lists are small. */
     for (i = 0; i < end_events[column].n_events; i++) {
	  wall = end_events[column].events[i];

	  for (j = 0; j < n_active && active[j].wall != wall; j++);
	  n_active--;
	  memmove(active + j, active + j + 1,
		  sizeof(Active_wall) * (n_active - j));
     }

     return n_active;
}


static int remove_obj_events(Active_object *active, int n_active, int column)
{
     int i, j;
     Object *o;


     /* This is really inefficient, so I hope these event lists are small. */
     for (i = 0; i < obj_end_events[column].n_events; i++) {
	  o = obj_end_events[column].events[i];

	  for (j = 0; j < n_active && active[j].o != o; j++);
	  n_active--;
	  memmove(active + j, active + j + 1,
		  sizeof(Active_object) * (n_active - j));
     }

     return n_active;
}



/**********************************************************************
**
** This section contains the guts of the wall drawing functions.  There
**   are a number of static variables declared here which are used to pass
**   information between these functions.  They are not used anyplace else.
**   If only C had a block structure like Pascal . . .
*/

static fixed pstart1, pend1, pstart2, pend2;
static fixed top, bottom;
Region *front, *back;


/* Walls can have up to two segments--one attached to the floor and one
**   attached to the ceiling.
*/
inline void draw_wall_segment(fixed pstart, fixed pend,
			      fixed start,
			      fixed z,
			      fixed height,
			      int fb_column,
			      Pixel *tex_base,
			      Wall *wall,
			      fixed tex_dy)
{
     Pixel *fb_byte, *last_byte;
     int fb_start, fb_end;
     fixed tex_y;


     /* Clip the wall slice. */
     if (pstart < bottom) {
	  pstart = bottom;
	  start = fixdiv(pstart, z) + height;
     }
     if (pend >= top)
	  pend = top - 1;

     fb_start = (view_height >> 1) - 1 -
	  FIXED_TO_INT(FIXED_SCALE(pstart, view_height));
     fb_end   = (view_height >> 1) - 1 - 
	  FIXED_TO_INT(FIXED_SCALE(pend, view_height));

     fb_column = view_width - fb_column - 1;
     fb_byte   = fb->pixels + fb_column + fb_rows[fb_start];
     last_byte = fb->pixels + fb_column + fb_rows[fb_end];

     if (wall->sky)
	  tex_y = fixmul(view_constants.row_view[fb_start], wall->yscale) +
	       wall->yphase;
     else
	  tex_y = fixmul(start, wall->yscale) + wall->yphase;

     if (wall->opaque)
	  draw_wall_slice(fb_byte, last_byte, tex_base, tex_y, tex_dy,
			  view_width, wall->surface_texture->log2height,
			  fb_start - fb_end);
     else
	  draw_transparent_slice(fb_byte, last_byte, tex_base, tex_y, tex_dy,
				 view_width, wall->surface_texture->log2height,
				 fb_start - fb_end);
}


inline void draw_wall(Wall *wall,
		      fixed z,
		      fixed Vx, fixed Vy,
		      View *v,
		      int column)
{
     Pixel *tex_base;
     fixed tex_dy;
     int tex_column;
     Boolean do_floor, do_ceiling;
     Texture *texture = wall->surface_texture;
     fixed start1, start2;
     fixed t;


     start1 = front->floor;
     start2 = back->ceiling;

     if (pend1 > pend2)
	  pend1 = pend2;
     if (pstart2 < pstart1)
	  pstart2 = pstart1;

     texture = wall->surface_texture;
     do_floor   = (pstart1 < top) &&
	          (pend1 > bottom) &&
		  (pend1 - pstart1 > FIXED_EPSILON);
     do_ceiling = (pstart2 < top) &&
	          (pend2 > bottom) &&
		  (pend2 - pstart2 > FIXED_EPSILON);
	  
     /* Don't do anything more with this wall if there's nothing
     **   to draw.
     */
     if (!do_floor && !do_ceiling)
	  return;

     /* We compute the texture coordinates for sky walls and normal walls
     **   in fundamentally different ways.
     */
     if (wall->sky) {
	  fixed angle;

	  /* Compute the angle of this column.  We'll use the angle
	  **   exclusively to determine which texture column to
	  **   display for this slice of sky.
	  */
	  angle = v->angle +
	          fixdiv(FIXED_SCALE(v->arc, column - (view_width >> 1)),
			 INT_TO_FIXED(view_width));
	  angle -= FIXED_SCALE(FIXED_2PI, 
			       FIXED_TO_INT(fixdiv(angle, FIXED_2PI)));
	  angle = fixdiv(angle, FIXED_2PI);
	  if (angle < FIXED_ZERO)
	       angle = FIXED_ONE - angle;
	  tex_column = FIXED_TO_INT(fixmul(angle, wall->xscale)) &
	       (texture->width - 1);
	  tex_dy = fixdiv(wall->yscale, INT_TO_FIXED(view_height));
     } else {
	  t = wall_ray_intersection(Vx, Vy, wall);
	  /* From t, calculate the integer coordinates in the texture
	  **   bitmap.  For efficiency, we assume that the width of the
	  **   texture is a power of two.
	  */
	  tex_column = FIXED_TO_INT(wall->xphase + fixmul(t, wall->xscale)) &
	       (texture->width - 1);
	  /* Test to avoid overflow here . . .  if the wall is so far
	  **   away that z (which is 1 / distance) is less than
  	  **   FIXED_EPSILON, then it will be so small when rendered
	  **   that we can use a bogus value for tex_dy.
	  */
          if (z < FIXED_EPSILON)
	       tex_dy = 0;
	  else
	       tex_dy = fixdiv(wall->yscale, FIXED_SCALE(z, view_height));
     }

     tex_base = TEXTURE_COLUMN(texture, tex_column);

     if (do_floor)
	  draw_wall_segment(pstart1, pend1, start1,
			    z,
			    v->height,
			    column,
			    tex_base,
			    wall,
			    tex_dy);

     if (do_ceiling)
	  draw_wall_segment(pstart2, pend2, start2,
			    z,
			    v->height,
			    column,
			    tex_base,
			    wall,
			    tex_dy);
}


inline void draw_object(Object *o,
			fixed z,
			int tex_column,
			View *v,
			int fb_column)
{
     fixed pstart, pend;
     fixed tex_y, tex_dy;
     int fb_start, fb_end;
     Pixel *fb_byte, *last_byte;
     Pixel *tex_base;
     fixed obj_height = FLOAT_TO_FIXED(o->height);
     

     /* Project the bottom and top of the object onto the slice. */
     
     pstart = fixmul(z, FLOAT_TO_FIXED(o->z) - v->height);
     pend   = fixmul(z, obj_height) + pstart;
     
     /* See if the object is visible in this column. */
     if (pstart > top || pend < bottom || pend - pstart < FIXED_EPSILON)
	  return;

     tex_dy = fixdiv(INT_TO_FIXED(o->image->height),
		     FIXED_SCALE(pend - pstart, view_height));
     /* Clip the object slice. */
     if (pstart < bottom) {
	  tex_y = fixmul(bottom - pstart, FIXED_SCALE(tex_dy, view_height));
	  pstart = bottom;
     } else
	  tex_y = FIXED_ZERO;
     if (pend > top)
	  pend = top;

     fb_start = (view_height >> 1) - 1 -
	  FIXED_TO_INT(FIXED_SCALE(pstart, view_height));
     fb_end   = (view_height >> 1) - 1 - 
	  FIXED_TO_INT(FIXED_SCALE(pend, view_height));
     fb_column = view_width - fb_column - 1;
     fb_byte   = fb->pixels + fb_column + fb_rows[fb_start];
     last_byte = fb->pixels + fb_column + fb_rows[fb_end];

     tex_base = TEXTURE_COLUMN(o->image, tex_column);

     draw_transparent_slice(fb_byte, last_byte,
			    tex_base,
			    tex_y, tex_dy,
			    view_width,
			    o->image->log2height,
			    fb_start - fb_end);
}


typedef struct {
     fixed top, bottom;
     Active_wall *active;
     Active_object *active_obj;
} Saved_wall;

#define MAX_TRANSPARENT_WALLS 100

Saved_wall transparent_walls[MAX_TRANSPARENT_WALLS];


/* do_walls() draws all the walls for a screen column. */
static void do_walls(Active_wall *active, int n_active,
		     Active_object *active_obj, int n_active_obj,
		     int column, View *v,
		     fixed Vx, fixed Vy)
{
     fixed height = v->height;
     Saved_wall *last_saved = transparent_walls;


     /* As we walk front to back through the active wall list, we track
     **   the top and bottom of the view port for this column.  Because of
     **   the restrictions placed on world geometry, the view port will
     **   always be a single interval, making clipping extremely simple.
     **   When bottom is greater than top, the view port is closed--nothing
     **   behind the current wall will be visible.
     */
     top    = FIXED_ONE_HALF;
     bottom = -FIXED_ONE_HALF;

     /* This loop moves from front to back through the list of 'active'
     **   walls.  We stop when we reach the last wall in the list, or
     **   when the walls we've already looked at completely obscure anything
     **   behind them.
     */
     while (n_active-- > 0 && bottom < top) {
	  Wall *wall = active->wall;
	  fixed z = active->z;
	  fixed dz = active->dz;


	  /* See if there are any objects in front of this wall which need
          **   to be drawn.  Since objects have transparent parts, drawing 
          **   them must be deferred until after the opaque wall slices have
          **   been drawn.  Transparent walls are handled in a similar
          **   manner later on in the loop.
          */
	  while (n_active_obj > 0 && active_obj->z >= z) {
	       /* Record the object in the save buffer. */
	       last_saved->top = top;
	       last_saved->bottom = bottom;
	       last_saved->active_obj = active_obj;
	       last_saved->active = NULL;
	       last_saved++;

               /* Advance through the object list. */
	       n_active_obj--;
	       active_obj++;
	  }


	  /* Set up the front and back region pointers--if we're behind
          **   a wall, these need to be reversed.
	  */
	  if (active->is_back_view) {
	       front = wall->back;
	       back = wall->front;
	  } else {
	       front = wall->front;
	       back = wall->back;
	  }

	  /* If this wall has not been visible before, set the visible flag 
          **   and set up the projected coordinates and deltas.
	  */
	  if (!active->visible) {
	       active->pstart1 =
		    fixmul2_30(TO_FIX_8_24(front->floor - height), z);
	       active->pend1 =
		    fixmul2_30(TO_FIX_8_24(back->floor - height), z);
	       active->pstart2 =
		    fixmul2_30(TO_FIX_8_24(back->ceiling - height), z);
	       active->pend2 =
		    fixmul2_30(TO_FIX_8_24(front->ceiling - height), z);
	       active->dpstart1 =
		    fixmul2_30(TO_FIX_8_24(front->floor - height), dz);
	       active->dpend1 =
		    fixmul2_30(TO_FIX_8_24(back->floor - height), dz);
	       active->dpstart2 =
		    fixmul2_30(TO_FIX_8_24(back->ceiling - height), dz);
	       active->dpend2 =
		    fixmul2_30(TO_FIX_8_24(front->ceiling - height), dz);
	       active->visible = True;
	  }

	  /* If this wall is opaque, we draw it now and adjust bottom
          **   and top appropriately.  Otherwise, we defer the drawing of
          **   the wall slice until later.
          */
	  if (active->wall->opaque) {

	       pstart1 = FROM_FIX_8_24(active->pstart1);
	       pend1 = FROM_FIX_8_24(active->pend1);
	       pstart2 = FROM_FIX_8_24(active->pstart2);
	       pend2 = FROM_FIX_8_24(active->pend2);

	       draw_wall(wall, FROM_FIX_2_30(z), Vx, Vy, v, column);

	       if (bottom < pend1)
		    bottom = pend1;
	       if (bottom < pstart1)
		    bottom = pstart1;
	       if (top > pstart2)
		    top = pstart2;
	       if (top > pend2)
		    top = pend2;

	  } else {

	       last_saved->top        = top;
	       last_saved->bottom     = bottom;
	       last_saved->active     = active;
	       last_saved->active_obj = NULL;
	       last_saved++;

	  }

	  active++;
     }

     /* Now, handle the objects and transparent walls.  last_saved points
     **   to an entry in the saved walls buffer.  Each entry consists of the
     **   the active list entry plus the top and bottom clipping info.
     */
     while (last_saved > transparent_walls) {
	  Active_wall *active;
	  Active_object *active_obj;


	  last_saved--;

	  top = last_saved->top;
	  bottom = last_saved->bottom;

	  active = last_saved->active;

	  /* If active is not NULL, this entry is a wall;  if it is NULL,
          **    this entry is an object and active_obj is not null.
          */
	  if (active != NULL) {
	       pstart1 = FROM_FIX_8_24(active->pstart1);
	       pend1   = FROM_FIX_8_24(active->pend1);
	       pstart2 = FROM_FIX_8_24(active->pstart2);
	       pend2   = FROM_FIX_8_24(active->pend2);

	       draw_wall(active->wall, FROM_FIX_2_30(active->z),
			 Vx, Vy, v, column);
	  } else {
	       active_obj = last_saved->active_obj;
	       draw_object(active_obj->o, FROM_FIX_2_30(active_obj->z),
			   FIXED_TO_INT(active_obj->x), v, column);
	  }
     }
}

/***********************************************************************/


			    
static void draw_floor_slices(Region *r, fixed start, fixed end,
			      View *v, int column)
{
     int screen_column = view_width - column - 1;
     int start_row, end_row;
     Texture *texture = r->floor_tex;
     fixed height;
     fixed sin_x, cos_x;


     start_row = FIXED_TO_INT(FIXED_SCALE(start, view_height));
     end_row = FIXED_TO_INT(FIXED_SCALE(end, view_height));
     /* Bail out early and save time if there's nothing to draw. */
     if (start_row == end_row)
	  return;

     start_row = (view_height >> 1) - 1 - start_row;
     end_row = (view_height >> 1) - 1 - end_row;
     if (start_row >= view_height)
	  start_row = view_height - 1;
     if (end_row < 0)
	  end_row = 0;

     height = r->floor - v->height;
     sin_x = view_constants.sin_tab[column];
     cos_x = view_constants.cos_tab[column];


     while (start_row >= end_row) {
	  fixed x, dx, y, dy;
	  fixed y1;
	  Pixel *fb_byte = fb->pixels + fb_rows[start_row] + screen_column;

	  if (FIXED_ABS(view_constants.row_view[start_row]) < FIXED_EPSILON)
	       y1 = FIXED_ONE;
	  else
	       y1 = fixdiv(height, view_constants.row_view[start_row]) << 4;

	  x = fixmul(view_constants.view_sin - cos_x, y1) - (v->y << 4);
	  y = fixmul(-view_constants.view_cos - sin_x, y1) - (v->x << 4);
	  dx = fixmul(view_constants.cos_dx, y1);
	  dy = fixmul(view_constants.sin_dx, y1);

	  draw_floor_slice(fb_byte, texture->texels, x, y, dx, dy, 
			   texture->width);

	  start_row--;
     }
}


static void draw_ceiling_slices(Region *r, fixed start, fixed end,
				View *v, int column)
{
     int screen_column = view_width - column - 1;
     int start_row, end_row;
     Texture *texture = r->ceiling_tex;
     fixed height;
     fixed sin_x, cos_x;


     start_row = FIXED_TO_INT(FIXED_SCALE(start, view_height));
     end_row = FIXED_TO_INT(FIXED_SCALE(end, view_height));
     /* Bail out early and save time if there's nothing to draw. */
     if (start_row == end_row)
	  return;

     start_row = (view_height >> 1) - 1 - start_row;
     end_row = (view_height >> 1) - 1 - end_row;
     if (start_row >= view_height)
	  start_row = view_height - 1;
     if (end_row < 0)
	  end_row = 0;

     height = r->ceiling - v->height;
     sin_x = view_constants.sin_tab[column];
     cos_x = view_constants.cos_tab[column];


     while (start_row >= end_row) {
	  fixed x, dx, y, dy;
	  fixed y1;
	  Pixel *fb_byte = fb->pixels + fb_rows[start_row] + screen_column;

	  if (FIXED_ABS(view_constants.row_view[start_row]) < FIXED_EPSILON)
	       y1 = FIXED_ONE;
	  else
	       y1 = fixdiv(height, view_constants.row_view[start_row]) << 4;

	  x = fixmul(view_constants.view_sin - cos_x, y1) - (v->y << 4);
	  y = fixmul(-view_constants.view_cos - sin_x, y1) - (v->x << 4);
	  dx = fixmul(view_constants.cos_dx, y1);
	  dy = fixmul(view_constants.sin_dx, y1);

	  draw_floor_slice(fb_byte, texture->texels, x, y, dx, dy, 
			   texture->width);

	  start_row--;
     }
}


/* Calculate the value of the parameter t at the intersection
**   of the view ray and the wall.  t is 0 at the origin of
**   the wall, and 1 at the other endpoint.
*/
static fixed wall_ray_intersection(fixed Vx, fixed Vy, Wall *wall)
{
     fixed denominator, Nx, Ny, Wx, Wy;
     
     Nx = -Vy;
     Ny = Vx;
     Wx = wall->vertex2->tx - wall->vertex1->tx;
     Wy = wall->vertex2->ty - wall->vertex1->ty;

     denominator = fixmul(Nx, Wx) + fixmul(Ny, Wy); /* N dot W */
     if (denominator < FIXED_EPSILON)
	  return FIXED_ONE - fixdiv(fixmul(Nx, wall->vertex1->tx) +
				    fixmul(Ny, wall->vertex1->ty),
				    -denominator);
     else if (denominator > FIXED_EPSILON)
	  return fixdiv(fixmul(Nx, wall->vertex1->tx) +
			fixmul(Ny, wall->vertex1->ty),
			-denominator);
     else
	  return FIXED_ZERO;
}


static void init_buffers(void)
{
     int i;

     for (i = 0; i < view_width + 1; i++) {
	  start_events[i].n_events = 0;
	  end_events[i].n_events = 0;
	  obj_start_events[i].n_events = 0;
	  obj_end_events[i].n_events = 0;
     }
}


/* Calculate values that are dependent only on the screen dimensions and
**   the view.
*/
static void calc_view_constants(View *v, int screen_width, int screen_height)
{
     static int last_height = 0, last_width = 0;
     int i;
     fixed x, y;

     
     /* Make sure that enough memory has been allocated for the tables. */
     if (screen_height != last_height) {
	  if (last_height == 0)
	       view_constants.row_view =
		    wtmalloc(screen_height * sizeof(fixed));
	  else
	       view_constants.row_view =
		    wtrealloc(view_constants.row_view,
			     screen_height * sizeof(fixed));
	  last_height = screen_height;
     }
     if (screen_width != last_width) {
	  if (last_width == 0) {
	       view_constants.sin_tab = wtmalloc(screen_width * sizeof(fixed));
	       view_constants.cos_tab = wtmalloc(screen_width * sizeof(fixed));
	  } else {
	       view_constants.sin_tab =
		    wtrealloc(view_constants.sin_tab,
			      screen_width * sizeof(fixed));
	       view_constants.cos_tab = 
		    wtrealloc(view_constants.cos_tab,
			      screen_width * sizeof(fixed));
	  }
	  last_width = screen_width;
     }
	  
     view_constants.view_sin =
	  FLOAT_TO_FIXED(sin(- FIXED_TO_FLOAT(v->angle)));
     view_constants.view_cos =
	  FLOAT_TO_FIXED(cos(- FIXED_TO_FLOAT(v->angle)));
     view_constants.screen_dx = fixdiv(FIXED_DOUBLE(v->view_plane_size),
				       INT_TO_FIXED(screen_width));
     view_constants.screen_dy = fixdiv(FIXED_ONE,
				       INT_TO_FIXED(screen_height));
     view_constants.sin_dx = fixmul(view_constants.view_sin,
				    view_constants.screen_dx);
     view_constants.cos_dx = fixmul(view_constants.view_cos,
				    view_constants.screen_dx);
     y = FIXED_SCALE(view_constants.sin_dx, -(screen_width >> 1));
     x = FIXED_SCALE(view_constants.cos_dx, -(screen_width >> 1));
     for (i = 0; i < screen_width; i++) {
	  view_constants.sin_tab[i] = y;
	  view_constants.cos_tab[i] = x;
	  y += view_constants.sin_dx;
	  x += view_constants.cos_dx;
     }
     
     y = FIXED_SCALE(view_constants.screen_dy, screen_height >> 1);
     for (i = 0; i < screen_height; i++) {
	  view_constants.row_view[i] = y;
	  y -= view_constants.screen_dy;
     }
}

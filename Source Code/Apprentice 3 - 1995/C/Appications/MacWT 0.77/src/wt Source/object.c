/*
**  MacWT -- a 3d game engine for the Macintosh
**  © 1995, Bill Hayden and Nikol Software
**
**  On the Internet:
**  bmoc1@aol.com (my personal address)
**  nikolsw@grove.ufl.edu (my school address)
**	MacWT anonymous FTP site: ftp.circa.ufl.edu/pub/software/ufmug/mirrors/LocalSW/Hayden/
**  http://grove.ufl.edu:80/~nikolsw (my WWW page, containing MacWT info)
**
**  based on wt, by Chris Laurel (claurel@mr.net)
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/



#include <stdlib.h>
#include "wt.h"
#include "error.h"
#include "wtmem.h"
#include "fixed.h"
#include "view.h"
#include "table.h"
#include "list.h"
#include "framebuf.h"
#include "texture.h"
#include "world.h"
#include "object.h"
#include "collision.h"


#define FP_ZERO(f) (((f) > -1.0e-5) && ((f) < 1.0e-5))

double gravity = -0.05;
Bounds	WorldBounds = {0.0, 0.0, 0.0, 0.0};



Object *new_object(double mass, double xsize, double ysize, double height, double drag)
{
	Object *o;

	o = wtmalloc(sizeof(Object));
	o->mass = mass;
	o->xsize = xsize;
	o->ysize = ysize;
	o->height = height;
	o->drag = drag;
	
	o->dx = o->dy = o->dz = 0.0;
	o->angle = 0.0;
	o->angular_v = 0.0;
	
	o->region = NULL;

	return o;
}


void object_update(Object *o)
{
	if (o->drag != 1.0)
		{
		o->dx *= o->drag;
		o->dy *= o->drag;
		o->angular_v *= o->drag;
		}

	o->x += o->dx;
	o->y += o->dy;
	o->z += o->dz;
	o->angle += o->angular_v;

	if (!FP_ZERO(o->dx) || !FP_ZERO(o->dy) || (o->region == NULL))
		o->region = in_region(o->world, FLOAT_TO_FIXED(o->x /*+ (o->dx * o->xsize)*/), FLOAT_TO_FIXED(o->y /*+ (o->dy * o->ysize)*/) );

	// Prevent the object from leaving the world entirely

	if (o->x < WorldBounds.left + o->xsize)
		o->x = WorldBounds.left + o->xsize;
	else if (o->x > WorldBounds.right - o->xsize)
		o->x = WorldBounds.right - o->xsize;
		
	if (o->y < WorldBounds.top + o->ysize)
		o->y = WorldBounds.top + o->ysize;
	else if (o->y > WorldBounds.bottom - o->ysize)
		o->y = WorldBounds.bottom - o->ysize;


	// Prevent the object from rising through the ceiling

	if (o->z + o->height > FIXED_TO_FLOAT(o->region->ceiling))
		{
		o->z = FIXED_TO_FLOAT(o->region->ceiling) - o->height;
		o->dz = 0.0;
		}


	// Prevent the object from passing through walls
		
	CheckCollision(o);
	
	#if 0
	// Prevent the object from sinking through the floor

	if (o->z < FIXED_TO_FLOAT(o->region->floor))
		if (FIXED_TO_FLOAT(o->region->floor) < (o->z + o->height))	// Limit height of what you can climb over
			{
			o->z = FIXED_TO_FLOAT(o->region->floor);
			o->dz = 0.0;
			}
	}
	#endif
	
	if (o->z < FIXED_TO_FLOAT(o->region->floor))
		{
		o->z = FIXED_TO_FLOAT(o->region->floor);
		o->dz = 0.0;
		}
	else
		o->dz += gravity;	// only drop if not already on the floor
}


void object_apply_force(Object *o, double fx, double fy, double fz)
{
	o->dx += fx / o->mass;
	o->dy += fy / o->mass;
	o->dz += fz / o->mass;
}


/* This is a really simple model of torque--since there's no information in
**   the object about mass distribution, there is no moment of inertia.  For
**   lack of something better, we just use the mass.
*/
void object_apply_torque(Object *o, double torque)
{
     o->angular_v += torque / o->mass;
}


/* Set the position of an object in a world--setting an object's
**   should only be done by calling this function; it should never be
**   set directly by modifying structure fields.
*/
extern void object_set_position(Object *o, double x, double y, double z)
{
	if (x != o->x || y != o->y || o->region == NULL)
		o->region = in_region(o->world, FLOAT_TO_FIXED(x), FLOAT_TO_FIXED(y));
		
	o->x = x;
	o->y = y;
	o->z = z;
}


void add_object(World *w, Object *o)
{
     /* Add an object to a world.  The object is added to the beginning of
     **   the list.  When an object is destroyed, we search for it in the
     **   the list starting from the beginning.  This ends up being quite
     **   efficient since the object being destroyed is likely to have
     **   been created recently.
     */
     add_node(w->objects, o);
}


Boolean remove_object(World *w, Object *o)
{
	List *l;

	l = scan_list(w->objects, o, find_node);
	if (l == NULL)
		return False;
	else
		{
		remove_node(l);
		return True;
		}
}


/* Derive a view from an object.  Assume that some fields of the view struct
**   have already been filled in:  view_plane_size, view_arc, and others that
**   cannot be determined from the object.
*/
void object_view(Object *o, View *v)
{
     v->x = FLOAT_TO_FIXED(o->x);
     v->y = FLOAT_TO_FIXED(o->y);
     v->height = FLOAT_TO_FIXED(o->z + o->height);
     v->angle = FLOAT_TO_FIXED(o->angle);
}


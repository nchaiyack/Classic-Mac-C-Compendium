/*
**  MacWT -- a 3d game engine for the Macintosh
**  � 1995, Bill Hayden and Nikol Software
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
#include <stdio.h>
#include <math.h>
#include "wt.h"
#include "error.h"
#include "fixed.h"
#include "wtmem.h"
#include "framebuf.h"
#include "texture.h"
#include "table.h"
#include "list.h"
#include "view.h"
#include "world.h"
#include "object.h"


World *new_world(void)
{
     World *w;

     w = wtmalloc(sizeof(World));
     w->vertices = new_table(sizeof(Vertex), MAX_VERTICES);
     w->walls = new_table(sizeof(Wall), MAX_WALLS);
     w->regions = new_table(sizeof(Region), MAX_REGIONS);
     w->textures = new_table(sizeof(Texture *), MAX_TEXTURES);
     w->objects = new_list();
     
     return w;
}


int add_texture(World *w, Texture *tex)
{
     return add_table_entry(w->textures, &tex);
}


int add_wall(World *w, Wall *wall)
{
     return add_table_entry(w->walls, wall);
}


int add_vertex(World *w, Vertex *v)
{
     return add_table_entry(w->vertices, v);
}


int add_region(World *w, Region *r)
{
     return add_table_entry(w->regions, r);
}


void update_wall_scale(World *w, int wall_num, fixed xscale, fixed yscale)
{
     Wall *wall;
     Texture *texture;
     fixed wall_length;

     if (wall_num < 0 || wall_num > TABLE_SIZE(w->walls))
	  fatal_error("invalid wall number");

     wall = (Wall *) w->walls->table + wall_num;
     texture = wall->surface_texture;

     wall_length =
	  FLOAT_TO_FIXED(sqrt(FIXED_TO_FLOAT(wall->vertex2->x -
					     wall->vertex1->x) *
			      FIXED_TO_FLOAT(wall->vertex2->x -
					     wall->vertex1->x) +
			      FIXED_TO_FLOAT(wall->vertex2->y -
					     wall->vertex1->y) *
			      FIXED_TO_FLOAT(wall->vertex2->y -
					     wall->vertex1->y)));
     wall->yscale = fixmul(yscale, INT_TO_FIXED(texture->height));
     wall->xscale = fixmul(fixmul(xscale, INT_TO_FIXED(texture->width)),
			   wall_length);
}


void update_wall_phase(World *w, int wall_num, fixed xphase, fixed yphase)
{
     Wall *wall;

     if (wall_num < 0 || wall_num > TABLE_SIZE(w->walls))
	  fatal_error("invalid wall number");

     wall = (Wall *) w->walls->table + wall_num;

     wall->xphase = xphase;
     wall->yphase = yphase;
}


/* Return the region in which a point falls.  The method for determining
**   this is simple--probably too much so.  We shoot a ray horizontally
**   from the point.  The region containing the point is either the front
**   or back region of the wall which intersects the ray closest to the
**   point.
*/
Region *in_region(World *w, fixed x, fixed y)
{
	Wall *wall = (Wall *) w->walls->table;
	Region *closest_region = NULL;
	fixed closest = FIXED_MAX;
	int i;
     
	for (i = 0; i < TABLE_SIZE(w->walls); i++, wall++)
		{
		fixed y1 = wall->vertex1->y;
		fixed y2 = wall->vertex2->y;

		if ((y >= y1 && y < y2) || (y >= y2 && y < y1))
			{
			fixed dist;
			Vertex *v1 = wall->vertex1, *v2 = wall->vertex2;

			// Kludge to avoid division overflows for near-horizontal walls.

			if (FIXED_ABS(y1 - y2) < FIXED_EPSILON)
				dist = MIN(v1->x - x, v2->x - x);
			else
				dist = fixmul(fixdiv(v1->x - v2->x, v1->y - v2->y), y - v1->y) + v1->x - x;
				
			if (dist > FIXED_ZERO && dist < closest)
				{
				closest = dist;
				if (y1 < y2)
					closest_region = wall->front;
				else
					closest_region = wall->back;
				}
			}
		}

     return closest_region;
}

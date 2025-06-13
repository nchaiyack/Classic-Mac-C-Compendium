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
**  based on on code provided by an unknown contributor via magoo.uwsuper.edu
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/



#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "wt.h"
#include "fixed.h"
#include "table.h"
#include "view.h"
#include "framebuf.h"
#include "texture.h"
#include "graphics.h"
#include "list.h"
#include "world.h"
#include "render.h"
#include "object.h"
#include "collision.h"

static double LineDistance( double cx, double cy, double ax, double ay, double bx, double by );
static void get_vector_xy( double *vx, double *vy, Wall *wall );

static Wall   *s_wall = NULL;

/*
	LineDistance returns the distance between a point c, and a line a-b.
*/

static double LineDistance( double cx, double cy, double ax, double ay, double bx, double by )
{
  double delta_x = bx-ax;
  double delta_y = by-ay;
  double line_len, r, s;

  /* the lenght of the line in **2 */
  line_len = delta_x * delta_x + delta_y * delta_y;

  r = ((ay-cy)*(-delta_y)-(ax-cx)*delta_x)/line_len;

  if ( r < 0.0 || r > 1.0 )
    return 512.0;

  /* If s becomes negative we are facing the line clockwise */
  s = ((ay-cy)*(delta_x)-(ax-cx)*delta_y)/sqrt( line_len );

  return s;
}


static void get_vector_xy( double *vx, double *vy, Wall *wall )
{
  double dx, dy, line_len;

  dx = (double)FIXED_TO_FLOAT(wall->vertex2->x - wall->vertex1->x);
  dy = (double)FIXED_TO_FLOAT(wall->vertex2->y - wall->vertex1->y);

  line_len = sqrt( dx*dx + dy*dy );

  *vx = dx/line_len;
  *vy = dy/line_len;
}


/*
  Check how MacWT should behave when an object slides against a wall. 
*/

static void slide_wall( double *x, double *y, double x2, double y2,
		  				double dist_old, double dist_new )
{
  double percent, line_len;
  double vector_x, vector_y;
  double dx = x2-*x;
  double dy = y2-*y;
  double suck1, suck2;

 
  line_len = sqrt( dx*dx + dy*dy );

  /* (dist_old-dist_new) becomes one side in a triangle. */
  /* percent can not be over 1.0. */
  /* How long is the distance towards the line compared to the line_length. */
  percent = (1.0 - fabs( dist_old - dist_new )/line_len);

  get_vector_xy( &vector_x, &vector_y, s_wall );

  /* in what direction are the vection compared to the walk ? */
  suck1 = (vector_x+dx)*(vector_x+dx) + (vector_y+dy)*(vector_y+dy);
  suck2 = (vector_x-dx)*(vector_x-dx) + (vector_y-dy)*(vector_y-dy);

  if ( suck1 > suck2 )
		{
		*x += percent * line_len * vector_x;
		*y += percent * line_len * vector_y;
		}
	else
		{
		*x -= percent * line_len * vector_x;
		*y -= percent * line_len * vector_y;
		}

}


void CheckCollision( Object *o )
{
	Wall   *wall = (Wall *) o->world->walls->table, *mem_wall;
	double  tmp, min = 512.0;
	int i;
	static double side	= 0.0,
                  x   	= 0.0,
                  y   	= 0.0;

	if ( o->x == x && o->y == y )
		return;

	for (i = 0; i < TABLE_SIZE(o->world->walls); i++, wall++ )
		{

		/* Check Distance */
		tmp = LineDistance( o->x, o->y, FIXED_TO_FLOAT(wall->vertex1->x ), 
										FIXED_TO_FLOAT(wall->vertex1->y ), 
										FIXED_TO_FLOAT(wall->vertex2->x ), 
										FIXED_TO_FLOAT(wall->vertex2->y ) );

		if ( fabs(tmp) < fabs( min ) )
			{
			min = tmp;
			mem_wall = wall; 
			}

		}

		/* is there any point in checking further. */
	if ( fabs( min ) < o->xsize )	// Note that objects are assumed to be round
		{
		/* start examine the wall */
		fixed ceiling, floor;

      /* Check the other side of the object */
		if ( min > 0.0 )
			{
			floor      = mem_wall->front->floor;
			ceiling    = mem_wall->front->ceiling;
			}
		else
			{
			floor      = mem_wall->back->floor;
			ceiling    = mem_wall->back->ceiling;
			}

      /* if the wall is less than the obejct height and the head has room */
		if (FIXED_TO_FLOAT(floor) > (o->z + o->height) && fabs( min ) < fabs( side ))
			{
			s_wall = mem_wall;
			slide_wall( &x, &y, o->x, o->y, side, min );

			o->x   = x;
			o->y   = y;

			return;
			}
		else
			{
			o->z = FIXED_TO_FLOAT(floor);
			o->dz = 0.0;
			}
		}

	side = min;
	x    = o->x;
	y    = o->y;

	return;
}


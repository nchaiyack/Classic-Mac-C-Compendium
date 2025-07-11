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
#include <math.h>
#include "wt.h"
#include "error.h"
#include "fixed.h"
#include "wtmem.h"
#include "view.h"


View *new_view(fixed arc)
{
     View *v;

     if (arc >= FIXED_PI)
	  return NULL;
     v = wtmalloc(sizeof(View));
     v->arc = arc;
     v->eye_distance = FLOAT_TO_FIXED(1.0);
     v->view_plane_size =
	  fixmul(FLOAT_TO_FIXED(tan(FIXED_TO_FLOAT(arc) / 2.0)), 
		 v->eye_distance);

     return v;
}

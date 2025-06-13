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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wt.h"
#include "error.h"
#include "fixed.h"
#include "view.h"
#include "framebuf.h"
#include "texture.h"
#include "table.h"
#include "list.h"
#include "world.h"
#include "worldfile.h"
#include "render.h"
#include "graphics.h"
#include "input.h"
#include "object.h"


fixed	start_x = 0, start_y = 0;


void WTMain(char *WorldFile, short width, short height, Boolean blocky)
{
	World		*w;
	FILE		*fp;
	Boolean		quit = False;
	Intent		*intent;
	Graphics_info *ginfo;
	Object		*me;
	View		*view;
	PhysicsModel	pm;
	extern double	gravity;

     
	ginfo = init_graphics(width, height, blocky);

	init_renderer(ginfo->width, ginfo->height);
	init_input_devices();

	if ((fp = fopen(WorldFile, "r")) == NULL)
		{
		fatal_error("The file '%s' could not be found.", WorldFile);
		return;
		}
     
	w = read_world_file(fp);
	fclose(fp);
	
	pm = (PhysicsModel)Get1Resource('Phys', 128);
	
	if (pm)
		{
		HLock((Handle)pm);
		gravity = -(*pm)->gravity / 100.0;
		me = new_object( (*pm)->mass,
						 (*pm)->xsize / 10.0,
						 (*pm)->ysize / 10.0,
						 (*pm)->height / 10.0,
						 (*pm)->drag / 100.0);	// load physics model
		HUnlock((Handle)pm);
		ReleaseResource((Handle)pm);
		}
	else
		me = new_object(70.0, 1.5, 1.5, 1.8, 0.6);	// load in default physics model values

	
	/* setup view */
	view = new_view(FLOAT_TO_FIXED(3.1415926536 / 2.0));
     
	me->x = FIXED_TO_FLOAT(start_x);
	me->y = FIXED_TO_FLOAT(start_y);
	me->z = 0.0;
	me->world = w;

	while (!quit)
		{
		double sin_facing, cos_facing;
		double fx, fy, fz;
		Framebuffer *fb;

		intent = read_input_devices();

		while (intent->n_special--)
			{
			if (intent->special[intent->n_special] == INTENT_END_GAME)
				quit = True;
			else
				object_apply_force(me, 0.0, 0.0, 50.0);
			}

		/* Determine forces on viewer. */
		sin_facing = sin(me->angle);
		cos_facing = cos(me->angle);
		fx = cos_facing * intent->force_x - sin_facing * intent->force_y;
		fy = sin_facing * intent->force_x + cos_facing * intent->force_y;
		fz = gravity * me->mass;  /* gravity */

		/* Apply the forces. */
		object_apply_force(me, fx, fy, fz);
		object_apply_torque(me, intent->force_rotate);
		object_update(me);

		/* Determine the view. */
		object_view(me, view);

		/* Display the world. */
		fb = render(w, view);
		update_screen(fb);
		}

	end_input_devices();
	end_graphics();

	return;
}
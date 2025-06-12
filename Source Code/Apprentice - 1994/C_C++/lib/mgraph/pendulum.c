/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		pendulum.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	Nov. 20, 1991
*	MODIFIED:	March 15, 1994 to agree with new mgraph version
*
*	Sample application using mgraph/xgraph library.
*
*	PROJECT CONTENTS:
*		pendulum.c/.cc, mgraph.c/xgraph.cc
*/

# ifdef THINK_C
# include "mgraph.h"
# else
# include "xgraph.h"
# endif
# include <stdio.h>
# include <math.h>

# define CENTER_X	0.
# define CENTER_Y	0.
# define RADIUS1	.7
# define RADIUS2	.2
# ifndef PI
# define PI			3.1415926
# endif

main()
{
	double	x,
			y,
			old_x,
			old_y,
			angle = 0.,
			increment,
			mouse_x,
			mouse_y;

	init_graphics();	/* Don't forget to do this FIRST! */
	
	printf("Move mouse vertically to change speed.\n");
	printf("Press mouse button when done...\n");
	
	graphics_to_front();
	background_color(RED);
	erase_graphics();
	old_x = RADIUS1 + CENTER_X;
	old_y = CENTER_Y;
	
	while (!mouse_button_is_down())
	{
		x = cos(angle)*RADIUS1 + CENTER_X;
		y = sin(angle)*RADIUS1 + CENTER_Y;
		
		erase_graphics();

		pen_color(WHITE);
		draw_line(CENTER_X,CENTER_Y,x,y);
		draw_circle(x,y,RADIUS2);
		
		old_x = x;
		old_y = y;
		get_mouse_location(&mouse_x,&mouse_y);
		increment = mouse_y/10.;
		angle += increment;
	}
	
	printf("All done...\n");
}
		

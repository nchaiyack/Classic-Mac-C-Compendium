/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		sample.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	April 16, 1993
*
*	Sample application of mgraph/xgraph functions. Try substituting
*	other colors from mgraph.h/xgraph.h in place of WHITE, if you're
*	using a color monitor.
*/

# ifdef THINK_C
# include "mgraph.h"
# else
# include "xgraph.h"
# endif
# include <stdlib.h>

main()
{
	double	x,
		y,
		old_x,
		old_y;

	init_graphics();

	background_color(BLACK);
	erase_graphics();
	draw_line(-1.,-1.,1.,1.);
	pen_color(WHITE);
	draw_circle(0.,0.,1.);
	pen_color(WHITE);
	fill_circle(.5,.5,.2);

	get_mouse_location(&x,&y);
	pen_color(WHITE);
	fill_circle(x,y,.1);
	old_x = x;
	old_y = y;
	while (!mouse_button_is_down())
	{
		get_mouse_location(&x,&y);
		if (x!=old_x || y!=old_y)
		{
			pen_color(BLACK);
			fill_circle(old_x,old_y,.1);
			pen_color(WHITE);
			fill_circle(x,y,.1);
		}
		old_x = x;
		old_y = y;
	}
}



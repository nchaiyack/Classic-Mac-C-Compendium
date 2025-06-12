//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		project.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 3, 1990
*
*	methods for projector class, which displays 2D images on screen
*/

# include	"project.h"
# include	"error.h"
# include	<stdlib.h>

extern Error	*gerror;
static int		old_window_num = -1;
static double	old_c2_x;
static double	old_c2_y;

/******************************************************************
*	initialize.
******************************************************************/
Projector::Projector(void)
{
	cropping_frame = new Frame;
	projection_frame = new Frame;
	window_num = -1;
	window_frame = NULL;
	screen_ptr = NULL;
	
//	derived projectors may change following initializations: 

	set_cropping_frame(0.,0.,1.,1.);
	set_projection_frame(0.,0.,1.,.1);
	set_background_color(BLACK);
}

/******************************************************************
*	set background color - duh!  (Default is set in constructor.)
******************************************************************/
void	Projector::set_background_color(color bck_color_val)
{
	background_color = bck_color_val;
}

/******************************************************************
*	set cropping frame to indicate what part of 2D space will
*	be drawn.  Otherwise the default will be used.  In 3D case,
*	use coordinates of focal plane of camera.
******************************************************************/
void	Projector::set_cropping_frame(double x,double y,
									double width,double height)
{
	cropping_frame->set(x,y,width,height);
}

/******************************************************************
*	set projection frame to indicate where the window will appear
*	on the screen.  This must be called before set_screen(), if
*	at all!  Use normalized screen coordinates.
******************************************************************/
void	Projector::set_projection_frame(double x,double y,
									double width,double height)
{
	projection_frame->set(x,y,width,height);
}

/******************************************************************
*	set pointer to screen used for projection, and allocate a
*	window on the screen.  This must be called before clearing
*	or drawing!  Do not call this more than once!
******************************************************************/
void	Projector::set_screen(Generic_Screen* screen_ptr_val)
{
	if (screen_ptr != NULL)
		gerror->report("Screen already set");
	else
	{
		screen_ptr = screen_ptr_val;
		window_num = screen_ptr->new_window(projection_frame);
		window_frame = new Frame;
		screen_ptr->get_window_device_frame(window_num,window_frame);
		clear();
	}
}

/******************************************************************
*	clear window using background color
******************************************************************/
void	Projector::clear(void)
{
	if (screen_ptr == NULL)
		gerror->report("Can't clear() with no screen");
	else
	{
		screen_ptr->set_current_window(window_num);
		screen_ptr->set_pen_color(background_color);
		screen_ptr->fill_window();
		old_window_num = -1;
	}
}

/******************************************************************
*	make corresponding window closest
******************************************************************/
void	Projector::overlap(void)
{
	if (screen_ptr == NULL)
		gerror->report("Can't overlap() with no screen");
	else
		screen_ptr->make_closest(window_num);
}

/******************************************************************
*	show line on screen.  If the window hasn't changed and the
*	new line continues where the previous one left off, then
*	drawing may be faster.  2D world coordinates are used (in 3D,
*	those of camera focal plane).
******************************************************************/
void	Projector::show_line(Coord2 *c1,Coord2 *c2,color line_color)
{
	Coord2	*c3,
			*c4;
			
	if (screen_ptr == NULL)
		gerror->report("Can't show_line() with no screen");
	else
	{
		if (old_window_num == window_num &&
			old_c2_x == c1->x && old_c2_y == c1->y)
		{
			screen_ptr->set_pen_color(line_color);
			c4 = new Coord2;
			c4->convert(c2,cropping_frame,window_frame);
			screen_ptr->draw_to(c4);
			delete c4;
		}
		else
		{
			screen_ptr->set_current_window(window_num);
			screen_ptr->set_pen_color(line_color);
			c3 = new Coord2;
			c3->convert(c1,cropping_frame,window_frame);
			c4 = new Coord2;
			c4->convert(c2,cropping_frame,window_frame);
			screen_ptr->draw_line(c3,c4);
			delete c3;
			delete c4;
		}
		
		old_window_num = window_num;
		old_c2_x = c2->x;
		old_c2_y = c2->y;
	}
}

/******************************************************************
*	destroy
******************************************************************/
Projector::~Projector(void)
{
	delete cropping_frame;
	delete projection_frame;
	if (window_frame != NULL)
		delete window_frame;
}

/******************************************************************
*	initialize corner projector
******************************************************************/
Corner_Projector::Corner_Projector(void)
{
	set_cropping_frame(0.,0.,2.,2.);
	set_projection_frame(.6,-.4,.4,.4);
	set_background_color(MAGENTA);
}


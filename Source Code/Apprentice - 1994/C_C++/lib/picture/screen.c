//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		screen.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	Aug. 5, 1990
*
*	generic methods for multi-window screens.  
*/

# include	"screen.h"
# include	"error.h"
# include	<stdlib.h>

extern Error	*gerror;

/******************************************************************
*	Initialize screen.  Derived classes initialize device_frame.
******************************************************************/
Generic_Screen::Generic_Screen(void)
{
	device_frame = new Frame;
	normalized_frame = new Frame;
	set_normalized_frame(0.,0.,2.,2.);
	num_windows = 0;
}

/******************************************************************
*	Add new window to screen - up to derived class.
******************************************************************/
int		Generic_Screen::new_window(Frame *frame)
{
	return	0; // must return something in Turbo C++? 
}

/******************************************************************
*	Bring window to front.  This is up to the derived class.
******************************************************************/
void	Generic_Screen::make_closest(int window_num)
{
}

/******************************************************************
*	Get coordinate frame of window in device coordinates.
*	Up to derived class.
******************************************************************/
void	Generic_Screen::get_window_device_frame(int window_num,
										Frame *frame)
{
}

/******************************************************************
*	Return screen aspect ratio: width/height in device coords.
******************************************************************/
double	Generic_Screen::get_device_aspect_ratio(void)
{
	double	ratio;
	
	ratio = device_frame->width/device_frame->height;
	if (ratio > 0.)
		return ratio;
	else
		return -ratio;
}

/******************************************************************
*	Change coordinate system for windows and drawing routines.
*	This is in terms of the width, height, and origin of
*	the desired coordinate system for the whole screen.  Use
*	get_device_aspect_ratio to maintain aspect ratio.
******************************************************************/
void	Generic_Screen::set_normalized_frame(double x,double y,
									double width,double height)
{
	normalized_frame->set(x,y,width,height);
}

/******************************************************************
*	Sets the current drawing window.  Up to derived class.
******************************************************************/
void	Generic_Screen::set_current_window(int window_num)
{
}

/******************************************************************
*	Sets the current drawing pen color.  Up to derived class.
******************************************************************/
void	Generic_Screen::set_pen_color(color x)
{
}

/******************************************************************
*	Sets window background to current color.  Up to derived class. 
******************************************************************/
void	Generic_Screen::fill_window(void)
{
}

/******************************************************************
*	draw_line() is used to draw lines using device coordinates.
******************************************************************/
void	Generic_Screen::draw_line(Coord2* c1,Coord2* c2)
{
	move_to(c1);
	draw_to(c2);
}

/******************************************************************
*	Move present pen position to new position using device
*	coordinates.  Nothing is drawn.  Up to derived class.
******************************************************************/
void	Generic_Screen::move_to(Coord2* c)
{
}

/******************************************************************
*	Draw from present pen position to new position using device
*	coordinates.  Up to derived class.
******************************************************************/
void	Generic_Screen::draw_to(Coord2* c)
{
}

/******************************************************************
*	mouse_button_is_down() checks whether the mouse button is down,
*	returns TRUE if so, FALSE if not.  Up to derived class.  Make
*	sure to override this if you use Screen::wait(), or override
*	Screen::wait().  (Else you'll get an infinite "wait"!)
******************************************************************/
boolean	Generic_Screen::mouse_button_is_down(void)
{
	return FALSE;
}

/******************************************************************
*	Wait until button is pressed.
******************************************************************/
void	Generic_Screen::wait(void)
{
	while (!mouse_button_is_down())
		;
	while (mouse_button_is_down())
		;
}

/******************************************************************
*	Destroy screen.
******************************************************************/
Generic_Screen::~Generic_Screen(void)
{
	delete device_frame;
	delete normalized_frame;
}

  




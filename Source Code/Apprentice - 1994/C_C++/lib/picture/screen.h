//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		screen.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	Oct. 3, 1990
*
*	Declaration of Generic_Screen class, to encapsulate
*	machine-specific graphics code.
*/

# ifndef	screen_h
# define	screen_h

# include	"class.h"
# include	"frame.h"
# include	"coord.h"
# include	"color.h"

# define	MAX_WINDOWS	100

/******************************************************************
*	Generic_Screen abstract class to isolate graphics I/O
******************************************************************/
class	Generic_Screen:public Generic_Class
{
protected:
	int				num_windows;
	Frame			*device_frame;

public:
	Frame			*normalized_frame;

	Generic_Screen(void);
	virtual int		new_window(Frame*);
	virtual void	make_closest(int);
	virtual void	get_window_device_frame(int,Frame*);
	virtual double	get_device_aspect_ratio(void);
	virtual void	set_normalized_frame(double,double,double,double);
	virtual void	set_current_window(int);
	virtual void	set_pen_color(color);
	virtual void	fill_window(void);
	virtual void	draw_line(Coord2*,Coord2*);	
	virtual void	move_to(Coord2*);
	virtual void	draw_to(Coord2*);
	virtual boolean	mouse_button_is_down(void);
	virtual void	wait(void);
	virtual			~Generic_Screen(void);
};


# endif



//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		project.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 2, 1990
*
*	Defines projector (for 2D transformation and display) for picture
*	application.
*/

# ifndef	project_h
# define	project_h

# include	"class.h"
# include	"coord.h"
# include	"screen.h"
# include	"frame.h"
# include	"color.h"

/******************************************************************
*   projector
******************************************************************/
class	Projector:public Generic_Class
{
private:
	Generic_Screen	*screen_ptr;
	color			background_color;
	Frame			*cropping_frame;
	Frame			*projection_frame;
	Frame			*window_frame;
	int				window_num;
	
public:
	Projector(void);
	virtual void	set_background_color(color);
	virtual void	set_cropping_frame(double,double,double,double);
	virtual void	set_projection_frame(double,double,double,double);
	virtual void	set_screen(Generic_Screen*);
	virtual void	clear(void);
	virtual void	overlap(void);
	virtual void	show_line(Coord2*,Coord2*,color);
	virtual			~Projector(void);
};

/******************************************************************
*   corner projector - sample of specialized projector
******************************************************************/
class	Corner_Projector:public Projector
{
public:
	Corner_Projector(void);
};

# endif
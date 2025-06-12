/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		mgraph.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	March 19, 1990
*	MODIFIED:	March 15, 1994
*				Adapted macgraph.h to agree with xgraph version
*
*	declarations for mgraph.c
*/

# include	"boolean.h"

# define	BLACK		0
# define	WHITE		1
# define	RED			2
# define	YELLOW		3
# define	GREEN		4
# define	BLUE		5
# define	CYAN		6
# define	MAGENTA		7
typedef int	color;

void	init_graphics(void);
double	get_screen_aspect_ratio(void);
void	set_graphics_coords(double,double,double,double);
void	pen_color(color);
void	background_color(color);
void	graphics_to_front(void); 
void	erase_graphics(void);
void	draw_line(double,double,double,double);	
void	move_to(double,double);
void	draw_to(double,double);
void	draw_circle(double,double,double);	
void	fill_circle(double,double,double);	
boolean	mouse_button_is_down(void);
void	wait(void);
void	get_mouse_location(double*,double*);


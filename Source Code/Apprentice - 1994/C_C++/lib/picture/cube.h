//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		cube.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	Defines cube for picture application.
*/

# ifndef	cube_h
# define	cube_h

# include	"segment.h"
# include	"color.h"

/******************************************************************
*   cube.  Nested segment consisting of 12 lines.
******************************************************************/
class	Cube:public Nested_Segment
{
public:
	Cube(void);
};

/******************************************************************
*   fast cube.  Not a nested segment.  Faster because you only
*	calculate perspective projection for each line endpoint once.
******************************************************************/
class	Fast_Cube:public Segment
{
private:
	Coord3			*c[8];
	color			cube_color;

public:
	Fast_Cube(void);
//	not implemented:
//	virtual void	set(...);
	virtual void	set_color(color);
	virtual void	draw(Camera*,Projector*,Transformation*);
	virtual void	move(Transformation*);
	virtual			~Fast_Cube(void);
};

# endif
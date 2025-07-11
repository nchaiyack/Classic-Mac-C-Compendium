//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		dycube.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	Defines dynamic cube for picture application.
*/

# ifndef	dycube_h
# define	dycube_h

# include	"dynamic.h"
# include	"color.h"

/******************************************************************
*   Dynamic cube.
******************************************************************/
class	Dynamic_Cube:public Dynamic_Segment
{
public:
	Dynamic_Cube(void);
//	not implemented:
//	virtual void	set(...);
};

/******************************************************************
*   fast cube.  No nested segments.  Faster because you only
*	calculate perspective projection for each line endpoint once.
******************************************************************/
class	Fast_Dynamic_Cube:public Dynamic_Segment
{
private:
	Coord3			*c[8];
	color			cube_color;

public:
	Fast_Dynamic_Cube(void);
//	not implemented:
//	virtual void	set(...);
	virtual void	set_color(color);
	virtual void	draw(Camera*,Projector*,Transformation*);
	virtual void	move(Transformation*);
	virtual			~Fast_Dynamic_Cube(void);
};

# endif
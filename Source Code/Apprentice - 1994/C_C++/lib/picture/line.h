//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		line.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	Defines 3D line segment for picture application.
*/

# ifndef	line_h
# define	line_h

# include	"segment.h"
# include	"trans.h"
# include	"camera.h"
# include	"project.h"
# include	"coord.h"

/******************************************************************
*   3D line
******************************************************************/
// in TC5 "Line" is already declared, so I use "Lline"
class	Lline:public Segment
{
private:
	Coord3			*c1;
	Coord3			*c2;
	color			line_color;

public:	
	Lline(void);
	virtual void	set_coord(double,double,double,double,double,double);
	virtual void	set_color(color);
	virtual void	draw(Camera*,Projector*,Transformation*);
	virtual void	move(Transformation*);
	virtual			~Lline(void);
};

# endif
//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		coord.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 2, 1990
*
*	Defines 2D and 3D coordinate classes for the picture
*	application.
*/

# ifndef	coord_h
# define	coord_h

# include	"class.h"
# include	"frame.h"
# include	"trans.h"

/******************************************************************
*   2D coordinate
******************************************************************/
class	Coord2:public Generic_Class
{
public:
	double			x;
	double			y;
	
	Coord2(void);
	virtual void	set(double,double);
	virtual void	convert(Coord2*,Frame*,Frame*);
};

/******************************************************************
*   3D coordinate
******************************************************************/
class	Coord3:public Generic_Class
{
public:
	double			x;
	double			y;
	double			z;
	
	Coord3(void);
	virtual void	set(double,double,double);
	virtual void	apply(Coord3*,Transformation*);
	virtual void	equate(Coord3*);
};

# endif
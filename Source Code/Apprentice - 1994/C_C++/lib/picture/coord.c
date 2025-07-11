//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		coord.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 4, 1990
*
*	methods for 2D and 3D coordinates
*/

# include	"coord.h"

/******************************************************************
*	initialize
******************************************************************/
Coord2::Coord2(void)
{
	set(0.,0.);
}

/******************************************************************
*	set values
******************************************************************/
void	Coord2::set(double x_val,double y_val)
{
	x = x_val;
	y = y_val;
}

/******************************************************************
*	convert 2D coordinate from one frame into a new one
******************************************************************/
void	Coord2::convert(Coord2* c,Frame* old_frame,Frame* new_frame)
{
	double	ratio;
	
	ratio = new_frame->width/old_frame->width;
	x = new_frame->x + (c->x-old_frame->x) * ratio;
	
	ratio = new_frame->height/old_frame->height;
	y = new_frame->y + (c->y-old_frame->y) * ratio;
}

/******************************************************************
*	initialize
******************************************************************/
Coord3::Coord3(void)
{
	set(0.,0.,0.);
}

/******************************************************************
*	set values
******************************************************************/
void	Coord3::set(double x_val,double y_val,double z_val)
{
	x = x_val;
	y = y_val;
	z = z_val;
}

/******************************************************************
*	applies the transformation to the coordinate.  This is done
*	by converting the 3D point to a 4D homogeneous coordinate and
*	right-multiplying it by the 4D homogeneous transformation
*	matrix to produce a new point.
******************************************************************/
void	Coord3::apply(Coord3* c,Transformation* t)
{
	x = c->x*t->m[0][0] + c->y*t->m[1][0] + c->z*t->m[2][0] + t->m[3][0];
	y = c->x*t->m[0][1] + c->y*t->m[1][1] + c->z*t->m[2][1] + t->m[3][1];
	z = c->x*t->m[0][2] + c->y*t->m[1][2] + c->z*t->m[2][2] + t->m[3][2];
}

/******************************************************************
*	set coord equal to another.
******************************************************************/
void	Coord3::equate(Coord3* c)
{
	x = c->x;
	y = c->y;
	z = c->z;
}


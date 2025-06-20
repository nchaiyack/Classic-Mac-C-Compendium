//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		dynamic.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 25, 1991
*
*	defines methods for dynamic nested segment
*/

# include	"dynamic.h"

/******************************************************************
*	initialize.
*	Acceleration and velocity are initialized to zero.
******************************************************************/
Dynamic_Segment::Dynamic_Segment(void)
{
	center = new Coord3;
	
	set_center(0.,0.,0.);
	set_acceleration(0.,0.,0.);
	set_velocity(0.,0.,0.);
}

/******************************************************************
*	set acceleration values
******************************************************************/
void	Dynamic_Segment::set_acceleration(double x,double y,double z)
{
	ax = x;
	ay = y;
	az = z;
}

/******************************************************************
*	set velocity values
******************************************************************/
void	Dynamic_Segment::set_velocity(double x,double y,double z)
{
	vx = x;
	vy = y;
	vz = z;
}

/******************************************************************
*	set center of segment
******************************************************************/
void	Dynamic_Segment::set_center(double x,double y,double z)
{
	center->set(x,y,z);
}

/******************************************************************
*	get y coordinate of center of segment
******************************************************************/
double	Dynamic_Segment::get_center_y(void)
{
	return center->y;
}

/******************************************************************
*	simulate - no need to override
*	Note that in general we do NOT also 'simulate' all the nested
*	segments here, since these segments may not themselves be
*	dynamic.  (They just move along with the main segment.)
*	Note smaller values of 'time' produce greater accuracy.
*	Note we could allow acceleration and velocity to be
*	general transformation matrices if we used matrix
*	exponentiation (by 'time') instead of the approach used here.
******************************************************************/
void	Dynamic_Segment::simulate(double time)
{
	Translation	*temp;
	
	vx = vx + ax * time;
	vy = vy + ay * time;
	vz = vz + az * time;
	
	temp = new Translation;
	temp->set(vx * time,vy * time,vz*time);

	move(temp);	/* changes the segment's 'transformation' */
	
	delete temp;
}

/******************************************************************
*	move dynamic segment including center coordinate
******************************************************************/
void	Dynamic_Segment::move(Transformation* trans)
{
	Coord3	*temp;

	Animated_Segment::move(trans);
		
	temp = new Coord3;
	
	temp->equate(center);
	center->apply(temp,trans);

	delete temp;
}

/******************************************************************
*	destroy dynamic segment
******************************************************************/
Dynamic_Segment::~Dynamic_Segment(void)
{
	delete center;
}


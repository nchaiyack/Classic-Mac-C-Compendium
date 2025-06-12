//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		dynamic.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 25, 1991
*
*	Defines dynamic nested segment, following rules of physics.
*/

# ifndef	dynamic_h
# define	dynamic_h

# include	"animate.h"
# include	"trans.h"
# include	"coord.h"

/******************************************************************
*   Animated segment affected by physics.
******************************************************************/
class	Dynamic_Segment:public Animated_Segment
{
protected:
	Coord3			*center;
	
public:
	double			ax,
					ay,
					az,
					vx,
					vy,
					vz;
					
	Dynamic_Segment(void);
	virtual void	set_acceleration(double,double,double);
	virtual void	set_velocity(double,double,double);	
	virtual void	set_center(double,double,double);
	virtual void	simulate(double);
	virtual double	get_center_y(void);
	virtual void	move(Transformation*);
	virtual			~Dynamic_Segment(void);
};

# endif
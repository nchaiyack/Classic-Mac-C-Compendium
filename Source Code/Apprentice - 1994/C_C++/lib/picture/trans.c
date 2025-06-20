//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		trans.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 3, 1990
*
*	methods for Transformation class and descendants.  Use to
*	transform 3D coordinates.
*/

# include	"trans.h"
# include	<math.h>

/******************************************************************
*	initialize
******************************************************************/
Transformation::Transformation(void)
{
	int		row,
			col;
			
	for (row=0 ; row<4 ; row++)
		for (col=0 ; col<4 ; col++)
			if (row == col)
				m[row][col] = 1.;
			else
				m[row][col] = 0.;
}

/******************************************************************
*	combines two transformations to produce a new one.  This is
*	done by right-multiplying the first homogeneous transformation
*	matrix with the next one.  That is, the transformations are
*	sequenced from left to right.
******************************************************************/
void	Transformation::combine(Transformation* first,
								Transformation* next)
{
	int		row,
			col;
	
	for (row=0 ; row<4 ; row++)
		for (col=0 ; col<4 ; col++)
			m[row][col] =	first->m[row][0] * next->m[0][col] +
							first->m[row][1] * next->m[1][col] +
							first->m[row][2] * next->m[2][col] +
							first->m[row][3] * next->m[3][col];
}

/******************************************************************
*	sets the transformation equal to the argument.
******************************************************************/
void	Transformation::equate(Transformation* nnew)
{
	int		row,
			col;
	
	for (row=0 ; row<4 ; row++)
		for (col=0 ; col<4 ; col++)
			m[row][col] = nnew->m[row][col];
}

/******************************************************************
*	set translation
******************************************************************/
void	Translation::set(double tx,double ty,double tz)
{
	m[3][0] = tx;
	m[3][1] = ty;
	m[3][2] = tz;
}

/******************************************************************
*	set scaling
******************************************************************/
void	Scaling::set(double sx,double sy,double sz)
{
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;
}

/******************************************************************
*	set rotation about x
******************************************************************/
void	Rotation_X::set(double theta)
{
	double	ct,
			st;
	
	ct = cos(theta);
	st = sin(theta);
	
	m[1][1] = ct;
	m[1][2] = st;
	m[2][1] = -st;
	m[2][2] = ct;
}

/******************************************************************
*	set rotation about y
******************************************************************/
void	Rotation_Y::set(double theta)
{
	double	ct,
			st;
	
	ct = cos(theta);
	st = sin(theta);
	
	m[0][0] = ct;
	m[0][2] = -st;
	m[2][0] = st;
	m[2][2] = ct;
}

/******************************************************************
*	set rotation about z
******************************************************************/
void	Rotation_Z::set(double theta)
{
	double	ct,
			st;
	
	ct = cos(theta);
	st = sin(theta);
	
	m[0][0] = ct;
	m[0][1] = st;
	m[1][0] = -st;
	m[1][1] = ct;
}

/******************************************************************
*	set view transformation - converts from world coordinates to
*	view (camera) coordinates.
*
*	This matrix is obtained by successively multiplying 5 basic tran-
*	sformation matrices: (1) translation by (-x,-y,-z), (2) rotation
*	about the world y axis by (-yaw), (3) rotation about the world x
*	axis by (-pitch), (4) rotation about the world z axis by (-roll),
*	and (5) negation of the x coordinate (to convert from right-handed
*	world coordinates to a left-handed view coordinate system).
*	Ref.: Hearn & Baker, "Computer Graphics", Prentice-Hall, 1986
******************************************************************/
void	World_To_Camera::set(double x,double y,double z,
							double yaw,double pitch,double roll)
{
	double	cy,
			cp,
			cr,
			sy,
			sp,
			sr,
			k1,
			k2;

	cy = cos(-yaw);
	cp = cos(-pitch);
	cr = cos(-roll);
	sy = sin(-yaw);
	sp = sin(-pitch);
	sr = sin(-roll);
	k1 = -x*cy-z*sy;
	k2 = x*sy-z*cy;
	
	m[0][0] = -cy*cr+sy*sp*sr;
	m[0][1] = cy*sr+sy*sp*cr;
	m[0][2] = -sy*cp;
	m[0][3] = 0.;
	
	m[1][0] = cp*sr;
	m[1][1] = cp*cr;
	m[1][2] = sp;
	m[1][3] = 0.;
	
	m[2][0] = -sy*cr-cy*sp*sr;
	m[2][1] = sy*sr-cy*sp*cr;
	m[2][2] = cy*cp;
	m[2][3] = 0.;
	
	m[3][0] = -k1*cr-y*cp*sr-k2*sp*sr;
	m[3][1] = k1*sr-y*cp*cr-k2*sp*cr;
	m[3][2] = -y*sp+k2*cp;
	m[3][3] = 1.;
}




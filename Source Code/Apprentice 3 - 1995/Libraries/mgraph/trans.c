/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		trans.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	March 15, 1994
*
*	functions for 3D transformation. (Based on trans.c in my Picture
*	class library.)
*/

# include	"trans.h"
# include	<math.h>

/******************************************************************
*	create identity
******************************************************************/
void	initialize_trans(double m[4][4])
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
*	Right-multiplies transformation m by transformation n; that is,
*	transformations are sequenced left to right.
******************************************************************/
void	combine_trans(double m[4][4],double n[4][4])
{
	int		row,
			col;
	double	temp[4][4];
	
	equate_trans(temp,m);
	
	for (row=0 ; row<4 ; row++)
		for (col=0 ; col<4 ; col++)
			m[row][col] =	temp[row][0] * n[0][col] +
							temp[row][1] * n[1][col] +
							temp[row][2] * n[2][col] +
							temp[row][3] * n[3][col];
}

/******************************************************************
*	sets the transformation equal to another one.
******************************************************************/
void	equate_trans(double m[4][4],double n[4][4])
{
	int		row,
			col;
	
	for (row=0 ; row<4 ; row++)
		for (col=0 ; col<4 ; col++)
			m[row][col] = n[row][col];
}

/******************************************************************
*	set translation
******************************************************************/
void	set_translation(double m[4][4],double tx,double ty,double tz)
{
	initialize_trans(m);
	m[3][0] = tx;
	m[3][1] = ty;
	m[3][2] = tz;
}

/******************************************************************
*	set scaling
******************************************************************/
void	set_scaling(double m[4][4],double sx,double sy,double sz)
{
	initialize_trans(m);
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;
}

/******************************************************************
*	set rotation about x
******************************************************************/
void	set_rotation_x(double m[4][4],double theta)
{
	double	ct,
			st;
	
	initialize_trans(m);

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
void	set_rotation_y(double m[4][4],double theta)
{
	double	ct,
			st;
	
	initialize_trans(m);

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
void	set_rotation_z(double m[4][4],double theta)
{
	double	ct,
			st;
	
	initialize_trans(m);

	ct = cos(theta);
	st = sin(theta);
	
	m[0][0] = ct;
	m[0][1] = st;
	m[1][0] = -st;
	m[1][1] = ct;
}

/******************************************************************
*	set camera parameters.
******************************************************************/
void	set_camera(Camera *c,double x,double y,double z,
		double yaw,double roll,double pitch)
{
	c->x = x;
	c->y = y;
	c->z = z;
	c->yaw = yaw;
	c->roll = roll;
	c->pitch = pitch;
}

/******************************************************************
*	set Coord3 parameters.
******************************************************************/
void	set_coord(Coord3 *p,double x,double y,double z)
{
	p->x = x;
	p->y = y;
	p->z = z;
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
*	world coordinates to a left-handed view coordinate system). See
*	definition of the camera struct for details about yaw-pitch-roll.
*	Ref.: Hearn & Baker, "Computer Graphics", Prentice-Hall, 1986
******************************************************************/
void	set_world_to_camera(double m[4][4],Camera c)
{
	double	cy,
			cp,
			cr,
			sy,
			sp,
			sr,
			k1,
			k2;

	cy = cos(-c.yaw);
	cp = cos(-c.pitch);
	cr = cos(-c.roll);
	sy = sin(-c.yaw);
	sp = sin(-c.pitch);
	sr = sin(-c.roll);
	k1 = -c.x*cy-c.z*sy;
	k2 = c.x*sy-c.z*cy;
	
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
	
	m[3][0] = -k1*cr-c.y*cp*sr-k2*sp*sr;
	m[3][1] = k1*sr-c.y*cp*cr-k2*sp*cr;
	m[3][2] = -c.y*sp+k2*cp;
	m[3][3] = 1.;
}

/******************************************************************
*	copy a 3D coordinate.
******************************************************************/
void	equate_coord(Coord3 *p,Coord3 q)
{
	p->x = q.x;
	p->y = q.y;
	p->z = q.z;
}

/******************************************************************
*	applies the transformation to the coordinate.  This is done
*	by converting the 3D point to a 4D homogeneous coordinate and
*	right-multiplying it by the 4D homogeneous transformation
*	matrix to produce a new point.
******************************************************************/
void	apply_trans(Coord3 *p,double m[4][4])
{
	Coord3	temp;
	
	equate_coord(&temp,*p);
	
	p->x = temp.x*m[0][0] + temp.y*m[1][0] + temp.z*m[2][0] + m[3][0];
	p->y = temp.x*m[0][1] + temp.y*m[1][1] + temp.z*m[2][1] + m[3][1];
	p->z = temp.x*m[0][2] + temp.y*m[1][2] + temp.z*m[2][2] + m[3][2];
}

/******************************************************************
*	convert 3D world coordinate to 2D image on focal plane.
*	Returns FALSE if thing is in back of camera.
*
*	The focal length is the distance from the camera "lens" (pro-
*	jection point) to the "film" (focal/projection plane). A larger
*	value corresponds to a "telephoto lens", for larger projected
*	images. This works in conjunction with the 2D viewing trans-
*	formation, which maps from the focal plane to the display
*	device. This transformation is built into the mgraph/xgraph
*	line drawing (etc.) routines: you just identify the area
*	of the projection plane you're interested in, and define the
*	"view" coordinate system accordingly.
******************************************************************/
boolean	project(Coord2 *image,Coord3 thing,
		double world_to_camera[4][4],double focal_length)
{
	boolean	success = TRUE;
	
	apply_trans(&thing,world_to_camera);
	
	if (thing.z < 1e-4 && thing.z > -1e-4)
	{
		image->x = thing.x * focal_length/(thing.z+1e-4);
		image->y = thing.y * focal_length/(thing.z+1e-4);
	}
	else
	{
		image->x = thing.x * focal_length/thing.z;
		image->y = thing.y * focal_length/thing.z;
	}
	
	if (thing.z < 1e-4)
		success = FALSE;
		
	return success;
}



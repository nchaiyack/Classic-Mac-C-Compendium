/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		trans.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	March 15, 1994
*
*	Defines 3D transformation (using trans. matrix) for mgraph
*	application, and other coordinate operations.
*/

# include	"boolean.h"

/******************************************************************
*	2D coordinate
******************************************************************/
typedef struct	Coord2
{
	double	x,
			y;
} Coord2;

/******************************************************************
*	3D coordinate
******************************************************************/
typedef struct	Coord3
{
	double	x,
			y,
			z;
} Coord3;

/******************************************************************
*   camera struct.  The camera's location is given in world
*	coordinates.  If you were to translate the camera to the
*	origin of the world coordinate system, then: YAW is the angle
*	the camera is pointing as rotated about the world's vertical
*	(y) axis (with respect to the direction of the world z axis);
*	PITCH is the angle it is pointing with respect to the world
*	horizontal (x-z) plane; ROLL is the angle the camera's "up"
*	direction makes from the world's y axis, as measured about the
*	axis in which the camera is pointing.  Note that yaw and roll
*	are undefined when pitch is PI/2 or -PI/2; i.e., when the
*	camera points straight up or down.
******************************************************************/
typedef struct	Camera
{
	double	x,
			y,
			z,
			yaw,
			pitch,
			roll;
} Camera;

/******************************************************************
*	functions
******************************************************************/
/*	Coord3 functions:
*/
void	set_coord(Coord3*,double,double,double);
void	equate_coord(Coord3*,Coord3);

/*	Camera and projection functions:
*/
void	set_camera(Camera*,double,double,double,double,double,double);
void	set_world_to_camera(double[4][4],Camera);
boolean	project(Coord2*,Coord3,double[4][4],double);

/*	transformation functions:
*/
void	initialize_trans(double[4][4]);
void	combine_trans(double[4][4],double[4][4]);
void	equate_trans(double[4][4],double[4][4]);
void	set_translation(double[4][4],double,double,double);
void	set_scaling(double[4][4],double,double,double);
void	set_rotation_x(double[4][4],double);
void	set_rotation_y(double[4][4],double);
void	set_rotation_z(double[4][4],double);
void	apply_trans(Coord3*,double[4][4]);


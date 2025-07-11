//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		camera.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 2, 1990
*
*	Defines camera (parameters for 3D projection) for the picture
*	application.
*/

# ifndef	camera_h
# define	camera_h

# include	"class.h"
# include	"coord.h"
# include	"trans.h"

/******************************************************************
*   camera class.  The camera's location is given in world
*	coordinates.  If you were to translate the camera to the
*	origin of the world coordinate system, then (1) yaw is the angle
*	the camera is pointing as rotated about the world's vertical
*	(y) axis (with respect to the direction of the world z axis);
*	(2) pitch is the angle it is pointing with respect to the world
*	horizontal (x-z) plane; (3) roll is the angle the camera's "up"
*	direction makes from the world's y axis, as measured about the
*	axis in which the camera is pointing.  Note that yaw and roll
*	are undefined when pitch is PI/2 or -PI/2; i.e., when the
*	camera points straight up or down.
******************************************************************/
class	Camera:public Generic_Class
{
private:
	World_To_Camera	*world_to_camera;
	double			focal_length;
	
public:
	Camera(void);
	virtual void	set_position(double,double,double,double,double,double);
	virtual void	set_focal_length(double);
	virtual boolean	take_photo(Coord2*,Coord3*);
	virtual			~Camera(void);
};

# endif
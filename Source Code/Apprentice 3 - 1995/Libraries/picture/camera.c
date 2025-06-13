//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		camera.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 3, 1990
*
*	Methods for camera class, which defines the view location.
*/

# include	"camera.h"

/******************************************************************
*	initialize
******************************************************************/
Camera::Camera(void)
{
	world_to_camera = new World_To_Camera;
	
	set_position(0.,0.,0.,0.,0.,0.);
	set_focal_length(1.);
}

/******************************************************************
*	set location and orientation of camera using world coordinates.
*	See camera.h for description.
******************************************************************/
void	Camera::set_position(double x,double y,double z,
					double yaw,double pitch,double roll)
{
	world_to_camera->set(x,y,z,yaw,pitch,roll);
}

/******************************************************************
*	set focal length of camera. This can be used for "zoom" effect.
*	Represents distance from camera origin to focal plane.
******************************************************************/
void	Camera::set_focal_length(double f)
{
	focal_length = f;
}

/******************************************************************
*	convert 3D world coordinate to 2D image on focal plane.
*	Returns FALSE if thing is in back of camera.
******************************************************************/
boolean	Camera::take_photo(Coord2 *image,Coord3 *thing)
{
	boolean	success = TRUE;
	Coord3	*temp;
	
	temp = new Coord3;
	temp->apply(thing,world_to_camera);
	
	if (temp->z < 1e-4 && temp->z > -1e-4)
	{
		image->x = temp->x * focal_length;
		image->x /= (temp->z+1e-4);	// broke up this expression because Think C choked
		image->y = temp->y * focal_length;
		image->y /= (temp->z+1e-4);
	}
	else
	{
		image->x = temp->x * focal_length/temp->z;
		image->y = temp->y * focal_length/temp->z;
	}
	
	if (temp->z < 1e-4)
		success = FALSE;
		
	delete temp;
	
	return success;
}

/******************************************************************
*	destroy
******************************************************************/
Camera::~Camera(void)
{
	delete world_to_camera;
}

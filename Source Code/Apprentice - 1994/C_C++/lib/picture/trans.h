//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		trans.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 3, 1990
*
*	Defines 3D transformation (using trans. matrix) for picture
*	application.
*/

# ifndef	trans_h
# define	trans_h

# include	"class.h"
# define	PI		3.1415926

/******************************************************************
*   abstract transformation
******************************************************************/
class	Transformation:public Generic_Class
{
public:
	double			m[4][4];
	
	Transformation(void);
	virtual void	combine(Transformation*,Transformation*);
	virtual void	equate(Transformation*);
};

/******************************************************************
*   translation
******************************************************************/
class	Translation:public Transformation
{
public:
	virtual void	set(double,double,double);
};

/******************************************************************
*   scaling
******************************************************************/
class	Scaling:public Transformation
{
public:
	virtual void	set(double,double,double);
};

/******************************************************************
*   rotation about x
******************************************************************/
class	Rotation_X:public Transformation
{
public:
	virtual void	set(double);
};

/******************************************************************
*   rotation about y
******************************************************************/
class	Rotation_Y:public Transformation
{
public:
	virtual void	set(double);
};

/******************************************************************
*   rotation about z
******************************************************************/
class	Rotation_Z:public Transformation
{
public:
	virtual void	set(double);
};

/******************************************************************
*   convert from world to camera coordinates
******************************************************************/
class	World_To_Camera:public Transformation
{
public:
	virtual void	set(double,double,double,double,double,double);
};

# endif
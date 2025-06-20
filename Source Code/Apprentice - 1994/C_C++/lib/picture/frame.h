//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		frame.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 3, 1990
*
*	Defines window frame (center, width, and height) for picture
*	application.  Useful for 2D mapping.
*/

# ifndef	frame_h
# define	frame_h

# include	"class.h"

/******************************************************************
*   general-purpose frame
******************************************************************/
class	Frame:public Generic_Class
{
public:
	double			x;	
	double			y;	
	double			width;
	double			height;
		
	Frame(void);
	virtual void	set(double,double,double,double);
};

# endif
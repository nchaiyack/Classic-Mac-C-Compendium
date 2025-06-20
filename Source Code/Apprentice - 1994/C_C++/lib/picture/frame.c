//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		frame.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 3, 1990
*
*	methods for frame class
*/

# include	"frame.h"

/******************************************************************
*	initialize
******************************************************************/
Frame::Frame(void)
{
	set(0.,0.,2.,2.);
}

/******************************************************************
*	set values
******************************************************************/
void	Frame::set(double x_val,double y_val,double width_val,
					double height_val)
{
	x = x_val;
	y = y_val;
	width = width_val;
	height = height_val;
}




//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		atring.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 9, 1990
*
*	Defines atomic animated ring of satellites for pict application.
*/

# ifndef	atring_h
# define	atring_h

# include	"animate.h"

/******************************************************************
*   animated ring.  Animated segment consisting of several 
*	satellites of any type.
******************************************************************/
class	Atomic_Ring:public Animated_Segment
{
public:
	Atomic_Ring(void);
};

# endif
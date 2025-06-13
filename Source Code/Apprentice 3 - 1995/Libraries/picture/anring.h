//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		anring.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 8, 1990
*
*	Defines animated ring of satellites for picture application.
*/

# ifndef	anring_h
# define	anring_h

# include	"animate.h"

/******************************************************************
*   animated ring.  Animated segment consisting of several
*	satellites of any type.
******************************************************************/
class	Animated_Ring:public Animated_Segment
{
public:
	Animated_Ring(void);
};

# endif
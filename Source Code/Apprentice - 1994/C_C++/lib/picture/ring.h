//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		ring.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 8, 1990
*
*	Defines ring of cubes for picture application.
*/

# ifndef	ring_h
# define	ring_h

# include	"segment.h"

/******************************************************************
*   ring.  Nested segment consisting of several cubes.
******************************************************************/
class	Ring:public Nested_Segment
{
public:
	Ring(void);
};

# endif
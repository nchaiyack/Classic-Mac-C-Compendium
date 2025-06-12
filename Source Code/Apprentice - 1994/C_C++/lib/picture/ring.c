//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		ring.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 8, 1990
*
*	defines methods for ring nested segment
*/

# include	"ring.h"
# include	"trans.h"
# include	"cube.h"

# define	NUM_CUBES	8
# define	RADIUS		3.
# define	CUBE_TYPE	Fast_Cube

/******************************************************************
*	initialize
******************************************************************/
Ring::Ring(void)
{
	int				i;
	Translation		*transl;
	Rotation_Y		*roty;
	Transformation	*combination;
	
	transl = new Translation;
	transl->set(-.5,-.5,RADIUS-.5);
	roty = new Rotation_Y;
	combination = new Transformation;
	
	num_segments = NUM_CUBES;
	
	for (i=0 ; i<NUM_CUBES ; i++)
	{
		segment[i] = new CUBE_TYPE;
		roty->set(i*2.*PI/NUM_CUBES);
		combination->combine(transl,roty);
		segment[i]->move(combination);
	}
	
	delete transl;
	delete roty;
	delete combination;
}


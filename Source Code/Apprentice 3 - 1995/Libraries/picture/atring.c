//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		atring.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 9, 1990
*
*	defines methods for atomic ring animated segment
*/

# include	"atring.h"
# include	"trans.h"
# include	"cube.h"

# define	NUM_SATELLITES		2
# define	RADIUS				2.
# define	SATELLITE_TYPE		Fast_Cube
# define	ANIMATED_SATELLITES	FALSE
# define	ANGULAR_VELOCITY	-PI/10.

/******************************************************************
*	initialize
******************************************************************/
Atomic_Ring::Atomic_Ring(void)
{
	int				i;
	Translation		*transl;
	Scaling			*scale;
	Rotation_Z		*roty;
	Transformation	*combination1,
					*combination2;
	
	transl = new Translation;
	transl->set(RADIUS-.5,-.5,-.5);
	scale = new Scaling;
	scale->set(.2,.2,.2);
	roty = new Rotation_Z;
	combination1 = new Transformation;
	combination2 = new Transformation;
	
	num_segments = NUM_SATELLITES;
	
	for (i=0 ; i<NUM_SATELLITES ; i++)
	{
		segment[i] = new SATELLITE_TYPE;
		roty->set(i*2.*PI/NUM_SATELLITES);
		combination1->combine(transl,roty);
		combination2->combine(combination1,scale);
		segment[i]->move(combination2);
		animation[i] = new Rotation_Z;
		((Rotation_Z*) animation[i])->set(ANGULAR_VELOCITY);
		if (ANIMATED_SATELLITES)
			log_animated_segment(segment[i]);
	}
	
	segment[i=num_segments++] = new Fast_Cube;
	transl->set(-.5,-.5,-.5);
	segment[i]->move(transl);
	animation[i] = new Transformation;
	
	delete transl;
	delete scale;
	delete roty;
	delete combination1;
	delete combination2;
}


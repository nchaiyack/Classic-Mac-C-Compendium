//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		anring.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 8, 1990
*
*	defines methods for ring animated segment
*/

# include	"anring.h"
# include	"trans.h"
# include	"atring.h"
# include	"cube.h"

# define	NUM_SATELLITES		2
# define	RADIUS				3.
# define	SATELLITE_TYPE		Atomic_Ring
# define	ANIMATED_SATELLITES	TRUE
# define	ANGULAR_VELOCITY	PI/20.

/******************************************************************
*	initialize
******************************************************************/
Animated_Ring::Animated_Ring(void)
{
	int				i;
	Translation		*transl;
	Rotation_Y		*roty;
	Transformation	*combination;
	
	transl = new Translation;
	transl->set(0.,0.,RADIUS);
	roty = new Rotation_Y;
	combination = new Transformation;
	
	num_segments = NUM_SATELLITES;
	
	for (i=0 ; i<NUM_SATELLITES ; i++)
	{
		segment[i] = new SATELLITE_TYPE;
		roty->set(i*2.*PI/NUM_SATELLITES);
		combination->combine(transl,roty);
		segment[i]->move(combination);
		animation[i] = new Rotation_Y;
		((Rotation_Y*) animation[i])->set(ANGULAR_VELOCITY);
		if (ANIMATED_SATELLITES)
			log_animated_segment(segment[i]);
	}
	
	delete transl;
	delete roty;
	delete combination;
}


//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		animate.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 8, 1990
*
*	defines methods for animated nested segment
*/

# include	"animate.h"
# include	"error.h"
# include	<stdlib.h>

extern Error	*gerror;

/******************************************************************
*	initialize.
*	Be sure to allocate and initialize an element of the array
*	animation[] for each segment.  This element indicates how
*	the respective segment is transformed when the nested segment
*	is animated.  If any of the segments are themselves animated
*	segments, then you must call:
*		log_animated_segment(segment[i]);
******************************************************************/
Animated_Segment::Animated_Segment(void)
{
	int		i;
	
	for (i=0 ; i<MAX_SEGMENTS ; i++)
		animation[i] = NULL;
	
	num_animated_segments = 0;
}

/******************************************************************
*	animate - no need to override
******************************************************************/
void	Animated_Segment::animate(void)
{
	int		i;
	
	for (i=0 ; i<num_animated_segments ; i++)
		animated_segment_ptr[i]->animate();
	
	for (i=0 ; i<num_segments ; i++)
	{
		if (animation[i] == NULL)
			gerror->report("Missing animation for this segment");
		else
			segment[i]->move(animation[i]);
	}
}

/******************************************************************
*	Log a pointer to an animated segment in the array
*	animated_segment_ptr[] and increment num_animated_segments.
******************************************************************/
void	Animated_Segment::log_animated_segment(Segment* seg)
{
	animated_segment_ptr[num_animated_segments++] =
		(Animated_Segment*) seg;
}

/******************************************************************
*	destroy - no need to override.
******************************************************************/
Animated_Segment::~Animated_Segment(void)
{
	int		i;
	
	for (i=0 ; i<num_segments ; i++)
		if (animation[i] != NULL)
			delete animation[i];
}



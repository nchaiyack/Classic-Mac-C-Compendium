//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		animate.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 8, 1990
*
*	Defines animated nested segment.
*/

# ifndef	animate_h
# define	animate_h

# include	"segment.h"

/******************************************************************
*   Nested segment which may be animated.
******************************************************************/
class	Animated_Segment:public Nested_Segment
{
private:
	Animated_Segment	*animated_segment_ptr[MAX_SEGMENTS];
	int					num_animated_segments;
	
protected:
	Transformation		*animation[MAX_SEGMENTS];

public:	
	Animated_Segment(void);
	virtual void	log_animated_segment(Segment*);
	virtual void	animate(void);
	virtual			~Animated_Segment(void);
};

# endif
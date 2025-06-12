//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		anringpi.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 8, 1990
*
*	Defines sample animated pict application class.
*/

# ifndef	anringpi_h
# define	anringpi_h

# include	"pict.h"
# include	"camera.h"
# include	"trans.h"
# include	"animate.h"

/******************************************************************
*   an_ring_pict application
******************************************************************/
class	An_Ring_Pict:public Generic_Pict
{
private:
	Projector		*projector1,
					*projector2;
	Camera			*camera1,
					*camera2;
	Animated_Segment	*segment;
			
public:
	An_Ring_Pict(void);
	virtual void	run(void);
	virtual			~An_Ring_Pict(void);
};

# endif

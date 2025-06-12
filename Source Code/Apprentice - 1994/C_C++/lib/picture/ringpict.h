//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		ringpict.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 7, 1990
*
*	Defines sample pict application class.
*/

# ifndef	ringpict_h
# define	ringpict_h

# include	"pict.h"
# include	"camera.h"
# include	"trans.h"
# include	"segment.h"

/******************************************************************
*   ring_pict application
******************************************************************/
class	Ring_Pict:public Generic_Pict
{
private:
	Projector		*projector1,
					*projector2,
					*projector3;
	Camera			*camera1,
					*camera2;
	Segment			*segment;
			
public:
	Ring_Pict(void);
	void			run(void);
	virtual			~Ring_Pict(void);
};

# endif
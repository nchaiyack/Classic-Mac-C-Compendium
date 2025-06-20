//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		dycubpic.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 25, 1991
*
*	Defines sample dynamic pict application class.
*/

# ifndef	dycubpic_h
# define	dycubpic_h

# include	"pict.h"
# include	"camera.h"
# include	"trans.h"
# include	"dynamic.h"

/******************************************************************
*   an_ring_pict application
******************************************************************/
class	Dy_Cube_Pict:public Generic_Pict
{
private:
	Projector		*projector1,
					*projector2;
	Camera			*camera1,
					*camera2;
	Dynamic_Segment	*segment;

public:
	Dy_Cube_Pict(void);
	void			run(void);
	virtual			~Dy_Cube_Pict(void);
};

# endif
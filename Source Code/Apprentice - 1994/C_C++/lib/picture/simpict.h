//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		simpict.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	Defines simple pict application class.
*/

# ifndef	simpict_h
# define	simpict_h

# include	"pict.h"
# include	"coord.h"

/******************************************************************
*   simple pict application
******************************************************************/
class	Simple_Pict:public Generic_Pict
{
private:
	Projector		*projector1,
					*projector2;
	Coord2			*c1,
					*c2;

public:
	Simple_Pict(void);
	virtual void	run(void);
	virtual			~Simple_Pict(void);
};

# endif
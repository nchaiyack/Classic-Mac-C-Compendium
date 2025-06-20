//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		pict.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 7, 1990
*
*	Defines generic pict application class.  All picture applications
*	should descend from this class.
*/

# ifndef	pict_h
# define	pict_h

# include	"class.h"
# include	"screen.h"
# include	"backdrop.h"

/******************************************************************
*   generic pict application - derived classes should add one or
*	more cameras, projectors, and segments
******************************************************************/
class	Generic_Pict:public Generic_Class
{
protected:
	Generic_Screen		*screen;
	
private:
	Backdrop_Projector	*backdrop;

public:
	Generic_Pict(void);
	virtual void		run(void);
	virtual				~Generic_Pict(void);
};

# endif
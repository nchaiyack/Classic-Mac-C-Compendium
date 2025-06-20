//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		backdrop.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 7, 1990
*
*	Defines backdrop projector class.
*/

# ifndef	backdrop_h
# define	backdrop_h

# include	"project.h"

/******************************************************************
*   backdrop projector - blacks out whole screen
******************************************************************/
class	Backdrop_Projector:public Projector
{
public:
	virtual void	set_screen(Generic_Screen*);
};

# endif
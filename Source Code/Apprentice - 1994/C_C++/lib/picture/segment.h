//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		segment.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 2, 1990
*
*	Defines general-purpose segment and nested segments for picture
*	application.  All graphics figures descend from Segment.
*/

# ifndef	segment_h
# define	segment_h

# include	"class.h"
# include	"trans.h"
# include	"camera.h"
# include	"project.h"
# include	"color.h"

# define	MAX_SEGMENTS	50

/******************************************************************
*   abstract segment
******************************************************************/
class	Segment:public Generic_Class
{
public:
	virtual void	set_color(color);
	virtual void	draw(Camera*,Projector*,Transformation*);
	virtual void	move(Transformation*);
};

/******************************************************************
*   abstract nested segment
******************************************************************/
class	Nested_Segment:public Segment
{
private:
	Transformation	*transformation;
	
protected:
	Segment			*segment[MAX_SEGMENTS];
	int				num_segments;
	
public:
	Nested_Segment(void);
	virtual void	set_color(color);
	virtual void	draw(Camera*,Projector*,Transformation*);
	virtual void	move(Transformation*);
	virtual			~Nested_Segment(void);
};

# endif

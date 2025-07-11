//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		segment.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 2, 1990
*
*	defines methods for abstract segment classes, from which
*	all graphics figures descend
*/

# include	"segment.h"

/******************************************************************
*	generic set-color.  derived classes should override!
******************************************************************/
void	Segment::set_color(color segment_color)
{
}

/******************************************************************
*	generic draw.  derived classes should override!
******************************************************************/
void	Segment::draw(Camera* camera,Projector* projector,
					Transformation* trans)
{
}

/******************************************************************
*	generic move.  derived classes should override!
******************************************************************/
void	Segment::move(Transformation* trans)
{
}

/******************************************************************
*	initialize abstract nested segment.  Derived class should init-
*	ialize segments and num_segments appropriately.  Make sure
*	num_segments <= MAX_SEGMENTS.
******************************************************************/
Nested_Segment::Nested_Segment(void)
{
	transformation = new Transformation;
	num_segments = 0;
}

/******************************************************************
*	Set nested segment color.  No need to override.
******************************************************************/
void	Nested_Segment::set_color(color segment_color)
{
	int				i;
	
	for (i=0 ; i<num_segments ; i++)
		segment[i]->set_color(segment_color);
}

/******************************************************************
*	Nested segment draw method.  No need to override.
******************************************************************/
void	Nested_Segment::draw(Camera* camera,Projector* projector,
							Transformation* received_trans)
{
	int				i;
	Transformation	*sent_trans;
	
	sent_trans = new Transformation;
	
	sent_trans->combine(transformation,received_trans);
	
	for (i=0 ; i<num_segments ; i++)
		segment[i]->draw(camera,projector,sent_trans);
	
	delete sent_trans;
}

/******************************************************************
*	Nested segments move by altering their transformation.
*	No need to override.
******************************************************************/
void	Nested_Segment::move(Transformation* trans)
{
	Transformation	*temp;
	
	temp = new Transformation;
	temp->equate(transformation);
	transformation->combine(temp,trans);
	delete temp;
}

/******************************************************************
*	destroy.  No need to override.
******************************************************************/
Nested_Segment::~Nested_Segment(void)
{
	int		i;
	
	delete transformation;
	
	for (i=0 ; i<num_segments ; i++)
		delete segment[i];
}


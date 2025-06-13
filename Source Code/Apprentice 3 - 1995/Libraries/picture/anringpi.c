//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		anringpi.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 8, 1990
*
*	Sample animated pict application.
*/

# include	"anringpi.h"
# include	"anring.h"
# define	NUM_ITERATIONS	100

/******************************************************************
*	initialize
******************************************************************/
An_Ring_Pict::An_Ring_Pict(void)
{
	Translation		*transl;
	
	projector1 = new Projector;
	projector1->set_background_color(RED);
	projector1->set_cropping_frame(0.,-.05,1.,.5);
	projector1->set_projection_frame(0.,0.,1.8,.9);

	projector2 = new Corner_Projector;
	
	projector1->set_screen(screen);
	projector2->set_screen(screen);
	
	camera1 = new Camera;
	camera1->set_position(0.,3.,0.,0.,.29,0.);
	camera2 = new Camera;
	camera2->set_position(0.,100.,10.,0.,PI/2.,0.);
	camera2->set_focal_length(20.);
	
	transl = new Translation;
	transl->set(0.,0.,10.);
	
	segment = new Animated_Ring;
	segment->move(transl);
	
	delete transl;
}

/******************************************************************
*	run
******************************************************************/
void	An_Ring_Pict::run(void)
{
	int		i;
	Transformation	*identity;
	
	identity = new Transformation;
	
	for (i=0 ; i<NUM_ITERATIONS && !screen->mouse_button_is_down() ; i++)
	{
		projector1->clear();
		segment->set_color(YELLOW);
		segment->draw(camera1,projector1,identity);
		projector2->clear();
		segment->set_color(BLUE);
		segment->draw(camera2,projector2,identity);
		segment->animate();
	}
	
	screen->wait();
	
	delete identity;
}

/******************************************************************
*	destroy
******************************************************************/
An_Ring_Pict::~An_Ring_Pict(void)
{
	delete projector1;
	delete projector2;

	delete camera1;
	delete camera2;
	
	delete segment;
}

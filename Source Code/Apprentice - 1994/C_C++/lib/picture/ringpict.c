//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		ringpict.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 7, 1990
*
*	Sample pict application.
*/

# include	"ringpict.h"
# include	"ring.h"

/******************************************************************
*	initialize
******************************************************************/
Ring_Pict::Ring_Pict(void)
{
	Rotation_X		*rotx;
	Translation		*transl;
	Transformation	*combination;
	
	projector1 = new Projector;
	projector1->set_background_color(BLUE);
	projector1->set_cropping_frame(0.,-.05,1.,.5);
	projector1->set_projection_frame(-.2,0.,1.4,.7);

	projector2 = new Corner_Projector;
	
	projector3 = new Projector;
	projector3->set_background_color(YELLOW);
	projector3->set_cropping_frame(0.,-.05,1.,.5);
	projector3->set_projection_frame(.8,.15,.3,.8);
	
	projector1->set_screen(screen);
	projector2->set_screen(screen);
	projector3->set_screen(screen);
	
	camera1 = new Camera;
	camera2 = new Camera;
	camera2->set_position(0.,10.,10.,0.,PI/2.,0.);
	
	rotx = new Rotation_X;
	rotx->set(-PI/12.);
	transl = new Translation;
	transl->set(0.,0.,10.);
	combination = new Transformation;
	combination->combine(rotx,transl);
	
	segment = new Ring;
	segment->move(combination);
	
	delete rotx;
	delete transl;
	delete combination;
}

/******************************************************************
*	run
******************************************************************/
void	Ring_Pict::run(void)
{
	Transformation	*identity;
	
	identity = new Transformation;
	
	segment->set_color(CYAN);
	segment->draw(camera1,projector1,identity);
	segment->set_color(GREEN);
	segment->draw(camera2,projector2,identity);
	segment->set_color(RED);
	segment->draw(camera1,projector3,identity);

	screen->wait();
	
	delete identity;
}

/******************************************************************
*	destroy
******************************************************************/
Ring_Pict::~Ring_Pict(void)
{
	delete projector1;
	delete projector2;
	delete projector3;

	delete camera1;
	delete camera2;
	
	delete segment;
}
	
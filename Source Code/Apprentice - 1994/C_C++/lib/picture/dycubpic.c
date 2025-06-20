//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		dycubpic.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 25, 1991
*
*	Sample dynamic pict application.
*/

# include	"dycubpic.h"
# include	"dycube.h"
# define	NUM_ITERATIONS	400
# define	FRICTION		.95
# define	TRAMPOLINE		-6.

/******************************************************************
*	initialize
******************************************************************/
Dy_Cube_Pict::Dy_Cube_Pict(void)
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
	camera1->set_focal_length(.6);
	camera2 = new Camera;
	camera2->set_position(0.,10.,10.,0.,PI/2.,0.);
	camera2->set_focal_length(2.);
	
	transl = new Translation;
	transl->set(0.,2.,10.);
	
	segment = new Fast_Dynamic_Cube;
	segment->move(transl);
	segment->set_acceleration(0.,-1.,0.);
	
	delete transl;
}

/******************************************************************
*	run
******************************************************************/
void	Dy_Cube_Pict::run(void)
{
	int				i;
	Transformation	*identity;
	Translation		*bouncer;
	double			time = .2,	// should be length of an iteration 
					depth;
									
	identity = new Transformation;
	bouncer = new Translation;
	
	for (i=0 ; i<NUM_ITERATIONS && !screen->mouse_button_is_down() ; i++)
	{
		projector1->clear();
		segment->set_color(YELLOW);
		segment->draw(camera1,projector1,identity);
		projector2->clear();
		segment->set_color(BLUE);
		segment->draw(camera2,projector2,identity);
		segment->animate();		/* nothing happens for Dynamic_Cube */
		segment->simulate(time);
		if ((depth = segment->get_center_y()) < TRAMPOLINE)
		{
			segment->vy = -segment->vy * FRICTION;
			bouncer->set(0.,2.*(TRAMPOLINE-depth),0.);
			segment->move(bouncer);
		}
	}
	
	screen->wait();
	
	delete identity;
	delete bouncer;
}

/******************************************************************
*	destroy
******************************************************************/
Dy_Cube_Pict::~Dy_Cube_Pict(void)
{
	delete projector1;
	delete projector2;

	delete camera1;
	delete camera2;
	
	delete segment;
}


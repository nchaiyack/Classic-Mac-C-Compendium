//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		simpict.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 4, 1990
*
*	Test simple pict application.
*/

# include	"simpict.h"

/******************************************************************
*	initialize
******************************************************************/
Simple_Pict::Simple_Pict(void)
{
	projector1 = new Projector;
	projector1->set_cropping_frame(5.,5.,10.,10.);
	projector1->set_projection_frame(0.,0.,1.,1.);
	projector1->set_background_color(CYAN);
	projector1->set_screen(screen);

	projector2 = new Corner_Projector;
	projector2->set_cropping_frame(5.,5.,10.,10.);
	projector2->set_screen(screen);
	
	c1 = new Coord2;
	c2 = new Coord2;
}

/******************************************************************
*	run
******************************************************************/
void	Simple_Pict::run(void)
{
	c1->set(1.,1.);
	c2->set(9.,9.);
	
	projector1->show_line(c1,c2,RED);
	projector2->show_line(c1,c2,BLUE);
	
	screen->wait();
}

/******************************************************************
*	destroy
******************************************************************/
Simple_Pict::~Simple_Pict(void)
{
	delete c1;
	delete c2;
	
	delete projector1;
	delete projector2;
}

	
	
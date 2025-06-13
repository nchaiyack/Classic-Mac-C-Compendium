//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		backdrop.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 7, 1990
*
*	methods for backdrop projector class, which blacks out screen.
*/

# include	"backdrop.h"

/******************************************************************
*	Set projection frame to fill screen, then call inherited.
*	Can't do this in constructor since we don't know screen_ptr yet
*	at that point.
******************************************************************/
void	Backdrop_Projector::set_screen(Generic_Screen* screen_ptr_val)
{
	set_projection_frame(screen_ptr_val->normalized_frame->x,
						screen_ptr_val->normalized_frame->y,
						screen_ptr_val->normalized_frame->width,
						screen_ptr_val->normalized_frame->height);

	Projector::set_screen(screen_ptr_val);
}


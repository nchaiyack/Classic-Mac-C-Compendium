//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		pict.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	November 7, 1990
*
*	generic pict application methods.  You should define a single
*	object of a class descending from Generic_Pict in your application,
*	and your main() function should simply send this object a run()
*	message.
*
*   REVISED:         November 7, 1991
*   AUTHOR:          Donald C. Snow
*   CHANGES:
*
*	eliminated the one instance of conditional compilation in 
*	the code by having all derived screen header files define 
*	"SCREEN" to something appropraite. Thus, new SCREEN, as 
*	opposed to, conditionally, new Mac_Screen or 
*	new PC_Screen. 
*/


# include	"pict.h"
# include	"error.h"

#ifdef THINK_C
#include "macscrn.h"
#endif

#ifdef TURBO_C
#include "pcscrn.h"
#endif

#ifdef __GNUG__
#include "xscrn.h"
#endif

Error	*gerror;		// global to report errors

// # include	<stdio.h>	// to allow stdio in pict application

/******************************************************************
*	initialize
******************************************************************/
Generic_Pict::Generic_Pict(void)
{
	double	aspect;

//	printf("\n");	// activate Think C console first to enable stdio

	gerror = new Error;
	screen = new SCREEN;
//	screen constructor should do this, but just to be sure: 
	aspect = screen->get_device_aspect_ratio();
	screen->set_normalized_frame(0.,0.,2.,2./aspect); 

	backdrop = new Backdrop_Projector;
	backdrop->set_screen(screen); 
}

/******************************************************************
*	run - override freely
*	If stdio is enabled as mentioned above, then the graphics
*	windows are supplemented with a console for text i/o.  Send
*	overlap() and clear() messages to projectors to bring their
*	associated windows back to the front after console i/o.  This
*	assumes that stdio calls automatically bring the console to the
*	front, as is the case with Think C.
******************************************************************/
void	Generic_Pict::run(void)
{
	screen->wait();
}

/******************************************************************
*	destroy
******************************************************************/
Generic_Pict::~Generic_Pict(void)
{
	delete backdrop; 
	delete screen;
	delete gerror;
}

	
	

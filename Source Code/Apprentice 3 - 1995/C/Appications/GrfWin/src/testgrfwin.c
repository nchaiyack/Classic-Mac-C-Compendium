// test grfwin.c
// Darrell Anderson

#include "GrfWin.h"

#define WIN_WIDTH 	300
#define WIN_HEIGHT 	300
#define MIN_X 		0.0
#define MIN_Y		0.0
#define MAX_X		605.0
#define MAX_Y		605.0

void main(void) {
	OSErr err;
	Point mousePt;
	GrfWinPtr grfwin;
	double lineValue;
	
	// initialize the toolbox
	GrfWin_InitToolbox();

	// allocate and initialize a grfwin for us.
	err = GrfWin_New( &grfwin, WIN_WIDTH, WIN_HEIGHT, MIN_X, MIN_Y, MAX_X, MAX_Y );

	// start the pen at the current mouse location (after mapping)
	GetMouse( &mousePt );
	GrfWin_MoveTo( grfwin, mousePt.h, mousePt.v );

	// follow the mouse until a click
	while( !Button() ) {
		GetMouse( &mousePt );
		GrfWin_LineTo( grfwin, mousePt.h, mousePt.v );
	}
	// debounce the mouse!
	while( Button() );
	
	// test the line control/mouse routine
	err = GrfWin_LoopUntilClose( grfwin, &lineValue );
	
	// finished with the grfwin, clean things up.
	err = GrfWin_Dispose( grfwin );
}
// grfwin.h
// Darrell Anderson 5/95

/*

GrfWin is a simple graphing package with extensions to allow the user 
to pass a vertical line over the graph at run time, in order to specify
a threshold value.

---

General usage: Make sure the ToolBox is initialized.  If you haven't done
that yourself, call GrfWin_InitToolbox() to set things up (but ONLY if the
toolbox has not beed initiallized already! if it has, this will crash!)

Then the main program contains a GrfWinPtr variable, an unitialized 
pointer to a GrfWin structure.  One call is made to GrfWin_New() to allocate 
this structure and display the window.

GrfWin_MoveTo() and GrfWin_LineTo() provide line drawing ability, so that 
the graph may be displayed.

Once the graph is drawn, GrfWin_LoopUntilClose() causes the program to 
enter a loop where the user may adjust the scrollbar in the window to
move a vertical line on the graph (also, the mouse coordinates are 
reported according to the graph's frame of reference).  When the 
window's close box is clicked, GrfWin_LoopUntilClose() returns, with it
the y-coordinate value of the line at the moment of close (the 
main purpose of GrfWin is to obtain user-decided threshold values)

Once the value is obtained, call GrfWin_Dispose() to hide the window
and release it's memory.

---

Internally, GrfWin maintains an offscreen copy of the window, mainly
to facilitate moving the line without having to replot the whole 
graph, but if GrfWin were to be made modal (not yet implemented) it
would also make dealing with UpdateEvent(s) easier.

*/

#ifndef __grfwin__
#define __grfwin__

#include <Windows.h>
#include <QuickDraw.h>
#include <QDOffscreen.h>
#include <Controls.h>

// note: don't worry about the internals to the GrfWin struct..
// hopefully the interface hides this stuff.. (:
typedef struct GrfWin {
	// the on-screen window
	CWindowPtr win;
	// an off-screen counterpart (for restoring under line)
	GWorldPtr ghostWin;
	// the scroll bar
	ControlHandle scrollbar;
	// the graph area rect
	Rect graphRect;
	// the status bar rect
	Rect statusRect;

	// the quickdraw coordinates of minx, etc, adjusted for margins
	short qd_min_x, qd_max_x, qd_min_y, qd_max_y;

	// the "real" coordinates of minx, etc (for mapping to qd coords)
	double min_x, min_y, max_x, max_y;

	// where the line is (mapping controlvalue to "real" coords)
	double line_y;
	
	// where the mouse is (mapping to "real" coords)
	double mouse_x, mouse_y;
} GrfWin, *GrfWinPtr;

//----------------------------------------------------------------
// Before any of the GrfWin calls will work, the toolbox must be
// initiallized.  If you don't do it elsewhere, call this function
// *EARLY* in your program.
//----------------------------------------------------------------
// takes no parameters, returns nothing.
//----------------------------------------------------------------
void GrfWin_InitToolbox(void);

//----------------------------------------------------------------
// NewGrfWin allocates and initializes and displays a GrfWin object. 
// note that the first argument is a pointer to a pointer, NewGrfWin
// allocates a new GrfWin object and makes this pointer point to it. 
//
// note that aspect ratio may be distorted.  ie the "real" frame
// is mapped onto the "size_x * size_y" frame, so be careful to 
// choose appropriately scaled size_x and size_y!
//
// note that the window's hieght and width will be slightly larger
// than these variable, in order to accomidate a scrollbar and
// status bar.
//
// also note that the GrfWin should be disposed of via DisposeGrfWin(..)
//----------------------------------------------------------------
// <- grfwinPtr :: a pointer to a GrfWinPtr to be set up.
// -> size_x :: the width of the drawing area to be created, in pixels
// -> size_y :: the height of the drawing area to be created, in pixels
// -> minx :: the minimum x value for the range
// -> miny :: the minimum y value for the range
// -> maxx :: the maximum x value for the range
// -> maxy :: the maximum y value for the range
// returns noErr is all is well, otherwise something went wrong.
//----------------------------------------------------------------
OSErr GrfWin_New( GrfWinPtr *grfwinPtrPtr, short size_x, short size_y, double minx, double miny, double maxx, double maxy );

//----------------------------------------------------------------
// Dispose of a GrfWin object previously allocated by NewGrfWin(..)
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object to be deallocated.
// returns noErr is all is well, otherwise something went wrong.
//----------------------------------------------------------------
OSErr GrfWin_Dispose( GrfWinPtr grfwin );

//----------------------------------------------------------------
// move an invisible pen to the corresponding quickdraw coordinates
// within the GrfWin.
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object we're dealing with.
// -> x :: the "real" x value to move the pen to.
// -> y :: the "real" y value to move the pen to.
//----------------------------------------------------------------
void GrfWin_MoveTo( GrfWinPtr grfwin, double x, double y );

//----------------------------------------------------------------
// draw a line from the current pen position to the specified 
// point.  also move the pen to this new position.
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object we're dealing with.
// -> x :: the "real" x value to draw a line to.
// -> y :: the "real" y value to draw a line to.
//----------------------------------------------------------------
void GrfWin_LineTo( GrfWinPtr grfwin, double x, double y );

//----------------------------------------------------------------
// drop into an event loop, updating the horizontal line in the 
// GrfWin and the mouse coordinates until the close box is clicked.
// note that this does not dispose of the GrfWin, merely causes a 
// return from this function.
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object we're dealing with.
// <- line_y :: stores the y value of the line at the moment when
//              the window close box is clicked.
//----------------------------------------------------------------
OSErr GrfWin_LoopUntilClose( GrfWinPtr grfwin, double *line_y );

//----------------------------------------------------------------
// utility routines (used internally)
//----------------------------------------------------------------

//----------------------------------------------------------------
// internal function, called by TrackControl to deal with clicks in 
// the scrollbar
//----------------------------------------------------------------
// -> theControl :: a handle to the scrollbar being clicked on
// -> partCode :: which piece of the scrollbar is being clicked
//----------------------------------------------------------------
pascal void myScrollProc( ControlHandle theControl, short partCode );

//----------------------------------------------------------------
// internal function, map from one frame to another
//----------------------------------------------------------------
double GrfWin_QDtoReal_x( GrfWinPtr grfwin, short x );
double GrfWin_QDtoReal_y( GrfWinPtr grfwin, short y );
short GrfWin_RealtoQD_x( GrfWinPtr grfwin, double x );
short GrfWin_RealtoQD_y( GrfWinPtr grfwin, double y );

//----------------------------------------------------------------
// internal function, compute the line's Y value in the real frame
//----------------------------------------------------------------
double GrfWin_GetLineY( GrfWinPtr grfwin );

//----------------------------------------------------------------
// internal function, draw a double with 'precision' decimal digits
// at the current mouse location in the current color
//----------------------------------------------------------------
void DrawDouble( double num, short precision );

//----------------------------------------------------------------
// internal function, redraw a grfwin, optionally only the status bar
// for faster response
//----------------------------------------------------------------
void GrfWin_Update( GrfWinPtr grfwin, Boolean statusBarOnly );

#endif // __grfwin__

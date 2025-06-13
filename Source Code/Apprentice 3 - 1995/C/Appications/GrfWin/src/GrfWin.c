// GrfWin.c
// Darrell Anderson 5/95

#include "GrfWin.h"
#include <Events.h>

//----------------------------------------------------------------
// Before any of the GrfWin calls will work, the toolbox must be
// initiallized.  If you don't do it elsewhere, call this function
// *EARLY* in your program.
//----------------------------------------------------------------
// takes no parameters, returns nothing.
//----------------------------------------------------------------
void GrfWin_InitToolbox(void) {
	InitGraf( &qd.thePort ); // NOTE: under THINK C this would be 'thePort'
							 // under CodeWarrior it's 'qd.thePort' --DA
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
}

//----------------------------------------------------------------
// GrfWin_New allocates and initializes and displays a GrfWin object. 
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
OSErr GrfWin_New( GrfWinPtr *grfwinPtrPtr, short size_x, short size_y, double minx, double miny, double maxx, double maxy ) {
	OSErr err;
	Rect box;
	GWorldPtr curGWorld;
	GDHandle curDevice;
	GrfWinPtr grfwinPtr;
	RGBColor rgbcolor;
	
	// get the GWorld info
	GetGWorld( &curGWorld, &curDevice );
	
	// adjust the dimenstions to account for the scrollbar/statusbar
	size_x += 15;
	size_y += 15;
	
	// allocate a new GrfWin object
	*grfwinPtrPtr = (GrfWin *)NewPtr( sizeof(GrfWin) );
	err = MemError();
	if( err != noErr ) return( err );
	grfwinPtr = *grfwinPtrPtr;
	
	// copy in the variables
	grfwinPtr->qd_min_x = 0;
	grfwinPtr->qd_min_y = 0;
	grfwinPtr->qd_max_x = size_x - 15;
	grfwinPtr->qd_max_y = size_y - 15;

	grfwinPtr->min_x = minx;
	grfwinPtr->min_y = miny;
	grfwinPtr->max_x = maxx;
	grfwinPtr->max_y = maxy;

	grfwinPtr->line_y = 0;
	grfwinPtr->mouse_x = 0;
	grfwinPtr->mouse_y = 0;
	
	// set the window size
	SetRect( &box, 0, 0, size_x, size_y );
	OffsetRect( &box, 20, 40 );
	
	// create the window (store the grfwinPtr in the window's refcon for control handling procs)
	grfwinPtr->win = (CWindowPtr) NewCWindow( nil, &box, "\pGrfWin", true, documentProc, (WindowPtr)-1L, true, (long)grfwinPtr );
	if( !grfwinPtr->win ) return( 1 ); // error
	
	// create the offscreen GWorld
	SetRect( &box, 0, 0, size_x, size_y );
	err = NewGWorld( &(grfwinPtr->ghostWin), 0, &box, 0, curDevice, 0 );
	if( err ) return( err );
	
	// clear the offscreen buffer
	SetGWorld( grfwinPtr->ghostWin, nil );
	EraseRect( &grfwinPtr->ghostWin->portRect );
	SetGWorld( grfwinPtr->win, curDevice );
		
	// create the scrollbar
	SetRect( &box, size_x-15, -1, size_x+1, size_y-14 );
	grfwinPtr->scrollbar = NewControl( (WindowPtr)grfwinPtr->win, &box, "\p", true, 
		grfwinPtr->qd_min_y, grfwinPtr->qd_min_y, grfwinPtr->qd_max_y, 
		scrollBarProc, 0L );

	// create the statusbar
	SetGWorld( grfwinPtr->win, curDevice );
	SetRect( &box, -1, size_y-15, size_x-15+1, size_y+1 );
	grfwinPtr->statusRect = box; // set the statusRect
	rgbcolor.red = rgbcolor.blue = rgbcolor.green = (unsigned short)0xBB00;
	RGBForeColor( &rgbcolor );
	PaintRect( &(grfwinPtr->statusRect) );
	ForeColor( blackColor );
	FrameRect( &box );
	SetGWorld( curGWorld, curDevice );
	
	// remember the graph rect
	SetRect( &(grfwinPtr->graphRect), 0,0, size_x-15, size_y-15 );
	
	// make the window the active port
	SetGWorld( grfwinPtr->win, curDevice );

	return( noErr );
}

//----------------------------------------------------------------
// Dispose of a GrfWin object previously allocated by NewGrfWin(..)
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object to be deallocated.
// returns noErr is all is well, otherwise something went wrong.
//----------------------------------------------------------------
OSErr GrfWin_Dispose( GrfWinPtr grfwin ) {
	OSErr err;
	
	// dispose of the GWorld
	DisposeGWorld( grfwin->ghostWin );
	if( err=MemError() != noErr ) return( err );
	
	// dispose of the control
	DisposeControl( grfwin->scrollbar );
	if( err=MemError() != noErr ) return( err );
	
	// dispose of the window
	DisposeWindow( (WindowPtr)grfwin->win );
	if( err=MemError() != noErr ) return( err );
	
	// dealloce the grfwin
	DisposePtr( (Ptr)grfwin );
	if( err=MemError() != noErr ) return( err );

	return( noErr );	
}

//----------------------------------------------------------------
// move an invisible pen to the corresponding quickdraw coordinates
// within the GrfWin.
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object we're dealing with.
// -> x :: the "real" x value to move the pen to.
// -> y :: the "real" y value to move the pen to.
//----------------------------------------------------------------
void GrfWin_MoveTo( GrfWinPtr grfwin, double x, double y ) {
	short qd_x, qd_y;
	GWorldPtr curGWorld;
	GDHandle curDevice;
	
	// get the GWorld info
	GetGWorld( &curGWorld, &curDevice );
	
	// map point
	qd_x = GrfWin_RealtoQD_x(grfwin,x);
	qd_y = GrfWin_RealtoQD_y(grfwin,y);
	
	// off-screen
	SetGWorld( grfwin->ghostWin, nil );
	MoveTo( qd_x, qd_y );

	// on-screen
	SetGWorld( grfwin->win, curDevice );
	MoveTo( qd_x, qd_y );
	
	// restore port
	SetGWorld( curGWorld, curDevice );
}

//----------------------------------------------------------------
// draw a line from the current pen position to the specified 
// point.  also move the pen to this new position.
//----------------------------------------------------------------
// -> grfwin :: a pointer to the GrfWin object we're dealing with.
// -> x :: the "real" x value to draw a line to.
// -> y :: the "real" y value to draw a line to.
//----------------------------------------------------------------
void GrfWin_LineTo( GrfWinPtr grfwin, double x, double y ) {
	short qd_x, qd_y;
	GWorldPtr curGWorld;
	GDHandle curDevice;
	
	// get the GWorld info
	GetGWorld( &curGWorld, &curDevice );
	
	// map point
	qd_x = GrfWin_RealtoQD_x(grfwin,x);
	qd_y = GrfWin_RealtoQD_y(grfwin,y);	
	
	// off-screen
	SetGWorld( grfwin->ghostWin, nil );
	LineTo( qd_x, qd_y );

	// on-screen
	SetGWorld( grfwin->win, curDevice );
	LineTo( qd_x, qd_y );
	
	// restore port
	SetGWorld( curGWorld, curDevice );
}

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
OSErr GrfWin_LoopUntilClose( GrfWinPtr grfwin, double *line_y ) {
	OSErr err;
	Boolean done = false;
	double old_line_y;

	EventRecord event;
	WindowPtr whichWindow;
	short thePart;
	Point thePoint;
	ControlRef theControl;
			
	while( !done ) {
		// remember the old line_y
		old_line_y = grfwin->line_y;
		
		// deal with any events
		if( WaitNextEvent( everyEvent, &event, 15, nil ) ) {
			switch( event.what ) {
				case mouseDown:
					thePart = FindWindow( event.where, &whichWindow );
					switch( thePart ) {
						case inSysWindow:
							// this means some System window was clicked in (DA..), just pass it on..
							SystemClick( &event, whichWindow );
							break;
						case inDrag:
							// the user clicked the drag bar, drag the window.
							DragWindow( whichWindow, event.where, &qd.screenBits.bounds );
							break;
						case inGoAway:
							// the user clicked the go-away box (upper left) signals us to exit.
							if( TrackGoAway( whichWindow, event.where ))
								done = true;
							break;
						case inContent:
							// the user clicked inside the window somewhere.
							thePoint = event.where;
							GlobalToLocal( &thePoint );
							
							thePart = FindControl( thePoint, whichWindow, &theControl );
							switch( thePart ) {
								case inThumb:
									thePart = TrackControl( theControl, thePoint, nil );
									break;
								case inUpButton:
								case inDownButton:
								case inPageUp:
								case inPageDown:
									thePart = TrackControl( theControl, thePoint, myScrollProc );
								case 0:
									// not in a control
									break;
								default:
									break;
							}									
						default:
							break;	
					}
				default:
					break;
			}
		}
		
		// read and interpret the control value
		grfwin->line_y = GrfWin_GetLineY( grfwin );
		
		// if the value has changed, redraw the screen, control line, and status bar
		if( grfwin->line_y != old_line_y ) {
			GrfWin_Update( grfwin, false );
		} else {
			GrfWin_Update( grfwin, true );
		}
	}
	
	// store the line value (already mapped)
	*line_y = grfwin->line_y;
	
	return( noErr );
}

//----------------------------------------------------------------
// internal function, called by TrackControl to deal with clicks in 
// the scrollbar
//----------------------------------------------------------------
// -> theControl :: a handle to the scrollbar being clicked on
// -> partCode :: which piece of the scrollbar is being clicked
//----------------------------------------------------------------
pascal void myScrollProc( ControlHandle theControl, short partCode )
{
	short		curCtlValue, maxCtlValue, minCtlValue;
	WindowPtr	window;
	Rect		listScreenRect;
	short		jumpDelta, controlDelta = 0;
		
	maxCtlValue = GetCtlMax( theControl );
	curCtlValue = GetCtlValue( theControl );
	minCtlValue = GetCtlMin( theControl );
	
	window = (**theControl).contrlOwner;
	
	switch( partCode )
	{
		case inPageDown:
			controlDelta = 12;
			break;
		case inDownButton:
			controlDelta = 1;
			break;
		case inPageUp:
			controlDelta = -12;
			break;
		case inUpButton:
			controlDelta = -1;
			break;
	}
	
	while( Button() )
	{
		// while the button is down, update the scroll value, check the range, then do it.
		curCtlValue += controlDelta;
		if( curCtlValue < minCtlValue )
			curCtlValue = minCtlValue;
		if( curCtlValue > maxCtlValue )
			curCtlValue = maxCtlValue;
		
		SetCtlValue( theControl, curCtlValue );				
		
		// redraw the window, including the line
		GrfWin_Update( (GrfWinPtr)GetWRefCon(window), false );
	}
}

//----------------------------------------------------------------
// internal function, map from one frame to another
//----------------------------------------------------------------
double GrfWin_QDtoReal_x( GrfWinPtr grfwin, short x ) {
	double ret;
	
	// map to real
	ret = grfwin->min_x + ((grfwin->max_x - grfwin->min_x) * 
	((double)(x - grfwin->qd_min_x) / (double)(grfwin->qd_max_x - grfwin->qd_min_x)));	
	
	return( ret );
}

//----------------------------------------------------------------
// internal function, map from one frame to another
//----------------------------------------------------------------
double GrfWin_QDtoReal_y( GrfWinPtr grfwin, short y ) {
	double ret;

	// map to real
	ret = grfwin->min_y + ((grfwin->max_y - grfwin->min_y) * 
	((double)(y - grfwin->qd_min_y) / (double)(grfwin->qd_max_y - grfwin->qd_min_y)));	
		
	// account for upside-down coordinate system
	ret = grfwin->max_y - ret;
		
	return( ret );
}

//----------------------------------------------------------------
// internal function, map from one frame to another
//----------------------------------------------------------------
short GrfWin_RealtoQD_x( GrfWinPtr grfwin, double x ) {
	short ret;

	// map to qd	
	ret = grfwin->qd_min_x + ((grfwin->qd_max_x - grfwin->qd_min_x) * 
	((x - grfwin->min_x) / (grfwin->max_x - grfwin->min_x)));
	
	return( ret );	
}

//----------------------------------------------------------------
// internal function, map from one frame to another
//----------------------------------------------------------------
short GrfWin_RealtoQD_y( GrfWinPtr grfwin, double y ) {
	short ret;
	
	// map to qd
	ret = grfwin->qd_min_y + ((grfwin->qd_max_y - grfwin->qd_min_y) * 
	((y - grfwin->min_y) / (grfwin->max_y - grfwin->min_y)));

	// correct for upside-down coordinate system
	ret = grfwin->qd_max_y - ret;

	return( ret );	
}

//----------------------------------------------------------------
// internal function, compute the line's Y value in the real frame
//----------------------------------------------------------------
double GrfWin_GetLineY( GrfWinPtr grfwin ) {
	double ret;
	
	ret = GrfWin_QDtoReal_y( grfwin, GetCtlValue(grfwin->scrollbar) );

	return( ret );
}

//----------------------------------------------------------------
// internal function, draw a double with 'precision' decimal digits
// at the current mouse location in the current color
//----------------------------------------------------------------
void DrawDouble( double num, short precision ) {
	long intPart, decPart;
	Str255 str;
	
	intPart = (long) num;
	
	num -= intPart;
	
	while( precision-- )
		num *= 10; 
		
	decPart = (long) num;
		
	// make sure the decimal part it positive
	if( decPart < 0 )
		decPart *= -1;

	NumToString( intPart, str );
	DrawString( str );
	DrawString( "\p." );
	NumToString( decPart, str );
	DrawString( str );
}

//----------------------------------------------------------------
// internal function, redraw a grfwin, optionally only the status bar
// for faster response
//----------------------------------------------------------------
void GrfWin_Update( GrfWinPtr grfwin, Boolean statusBarOnly ) {
	OSErr err;
	Point mousePt;
	Str255 textStr;
	RGBColor rgbcolor;
	GWorldPtr curGWorld;
	GDHandle curDevice;
	
	// get the GWorld info
	GetGWorld( &curGWorld, &curDevice );
	
	rgbcolor.red = rgbcolor.blue = rgbcolor.green = (unsigned short)0xBB00;
	
	// read and interpret the control value
	grfwin->line_y = GrfWin_GetLineY( grfwin );
	
	if( !statusBarOnly ) {	
		// redraw the screen and control line
		SetGWorld( grfwin->win, curDevice );
		CopyBits( (BitMap *) *((grfwin->ghostWin)->portPixMap),
				&(((GrafPtr)(grfwin->win))->portBits),
				&(grfwin->graphRect),
				&(grfwin->graphRect),
				srcCopy, nil );			
		MoveTo( 0, GrfWin_RealtoQD_y(grfwin,grfwin->line_y) );
		ForeColor( redColor );
		Line( grfwin->qd_max_x, 0 );
		ForeColor( blackColor );
	}
			
	// update the status bar
	SetGWorld( grfwin->win, curDevice );
	GetMouse( &mousePt ); // read the mouse location

	SetGWorld( grfwin->ghostWin, nil );
	
	RGBForeColor( &rgbcolor );
	PaintRect( &(grfwin->statusRect) );
	ForeColor( blackColor );
	FrameRect( &(grfwin->statusRect) );
		
	MoveTo( grfwin->statusRect.left + 4, grfwin->statusRect.bottom - 4 );
	DrawString( "\pLine: " );
	DrawDouble( grfwin->line_y, 2 );
	DrawString( "\p   " );
	DrawString( "\pMouse: " );
	DrawDouble( GrfWin_QDtoReal_x(grfwin,mousePt.h), 2 );
	DrawString( "\p, " );
	DrawDouble( GrfWin_QDtoReal_y(grfwin,mousePt.v), 2 );
	
	// copy it onscreen
	SetGWorld( grfwin->win, curDevice );
		
	CopyBits( (BitMap *) *((grfwin->ghostWin)->portPixMap),
			&(((GrafPtr)(grfwin->win))->portBits),
			&(grfwin->statusRect),
			&(grfwin->statusRect),
			srcCopy, nil );
				
	// redraw the controls
	DrawControls( (WindowRef)grfwin->win );
}
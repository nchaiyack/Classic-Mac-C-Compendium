/* window.c */

#include <Values.h>
#include <TransSkel.h>
#include "windows.h"
#include "globals.h"
#include "nyi.h"
#include "constants.h"
#include "error.h"
#include "dil.h"
#include "other_drawing.h"
#include "list.h"
#include "controls.h"
#include "status_bar.h"
#include "resources.h"

WindowPtr g_win_port;
CGrafPtr g_stored_port[k_port_stack_size];
GDHandle g_stored_device[k_port_stack_size];
short g_stored_port_frame = 0;

WindowPtr GetGWinPort( void )
{
	return( g_win_port );
}

void myInitWindows( void )
{	
	// get the window
	g_win_port = (WindowPtr) GetNewCWindow( k_window_id, nil, (WindowPtr)-1L );
	
	// tell the Skel shell about it!
	SkelWindow( (WindowPtr) g_win_port, myContentClick, nil, myUpdate, nil, myCloseBoxClick, nil, nil, false );	
	
	// put in a null zoom procedure (we don't want zooms here!)
	SkelSetZoom( (WindowPtr) g_win_port, myZoomBoxClick );
	
	// remember its position from last time
	RestoreWinLoc();
}

void myDisposeWindows( void )
{
	// store the window's location for next time
	StoreWinLoc();
}

pascal void myContentClick( Point where, long when, short modifiers )
{
	Point localPt;
	Rect listRect;
	ControlHandle theControl;
	short thePart;
	
// POINT IS ALREADY LOCAL!	
//	SetPortToWindow();
	localPt = where;
//	GlobalToLocal( &localPt );
	
	GetListRect( &listRect );
	
	if( PtInRect( localPt, &listRect ))
	{
		ListClick( localPt );
	}
	
	thePart = FindControl( localPt, GetGWinPort(), &theControl );
	if( theControl == ((WindowPeek) GetGWinPort())->controlList )
	{
		if( thePart == inThumb )
		{
			thePart = TrackControl( theControl, localPt, nil );
			DrawList();
		}
		else
			thePart = TrackControl( theControl, localPt, &myScrollProc );
	}
}

pascal void myUpdate( Boolean resized )
{
	myDrawControls();
	DrawList();
	DrawStatusBar();
	OtherDrawing();
	DrawDil();
}

pascal void myCloseBoxClick( void )
{
	// terminate the event loop, cleanup, and quit
	SkelStopEventLoop();
}

pascal void myZoomBoxClick( WindowPtr w, short zoomDir )
{
	; // do nothing
}

void ScaleWindow( void )
{
	short win_width, win_height, cur_height_additive, cur_height, cur_width;
	static float last_scale_percent = 100.0;
	
	cur_height = g_win_port->portRect.bottom - g_win_port->portRect.top;
	cur_width = g_win_port->portRect.right - g_win_port->portRect.left;
	
	cur_height_additive = cur_height - (last_scale_percent * k_dil_height);
	if( cur_height_additive < k_min_win_height_additive )
		cur_height_additive = k_min_win_height_additive;
	
	win_width = k_dil_width * GetScalePercent();
	win_height = (k_dil_height * GetScalePercent()) + cur_height_additive;

	// prevent rounding errors!
	if( ((win_height - cur_height) >= -2) && ((win_height - cur_height) <= 2) )
		win_height = cur_height;
	if( ((win_width - cur_width) >= -2) && ((win_width - cur_width) <= 2) )
		win_width = cur_width;

	SizeWindow( g_win_port, win_width, win_height, false );
	
	SkelSetGrowBounds( g_win_port, win_width, win_height, win_width, MAXSHORT );
	
	last_scale_percent = GetScalePercent();
}

Rect *GetWinRect( Rect *rect )
{
	// copy its content rect into our var
	*rect = g_win_port->portRect;
	
	return( rect );
}

void myShowWindow( void )
{
	ShowWindow( g_win_port );
}

void CopyGWorldToWindow( GWorldPtr theGWorld, Rect *srcRect, Rect *destRect, short mode )
//
// copy (a piece of) the GWorld to the window.  If srcRect is nil, copy over the whole
// GWorld.  use 'mode' for copybits (srcCopy, ditherCopy, ... )
//
{
	Rect real_srcRect;
		
	// make sure the GWorld exists (could've been lost if UpdateGWorld failed..)
	if( theGWorld == nil )
		myError( "CanÕt copy from non-existant buffer!  Try increasing my memory allocation!", true );
		
	// if srcRect is nil, copy the whole GWorld..
	if( srcRect == nil )
		real_srcRect = theGWorld->portRect;
	else
		real_srcRect = *srcRect;
	
	// set the port to be the window
	SetPortToWindow();
	
	// lock down the GWorld
	LockPixels( GetGWorldPixMap( theGWorld ) );

	// copy the gworld
	CopyBits( (BitMap *) *(theGWorld->portPixMap), &(g_win_port->portBits),
				&real_srcRect, destRect, mode, nil );
	
	// unlock the GWorld
	UnlockPixels( GetGWorldPixMap( theGWorld ) );
}

void SetPortToWindow( void )
//
// set the graphics port to be the window
//
{
	SetGWorld( (CGrafPtr) g_win_port, GetMainDevice() );
}

void SetPortToGWorld( GWorldPtr theGWorld )
{
	// store the current port
	GetGWorld( &g_stored_port[g_stored_port_frame], &g_stored_device[g_stored_port_frame] );
	
	// set the port to the new one, and lock the pixels down
	SetGWorld( theGWorld, GetMainDevice() );
	LockPixels( GetGWorldPixMap(theGWorld) );
	
	// point to the new next available spot on the stack
	g_stored_port_frame++;
	
	// make sure we haven't filled the stack!
	if( g_stored_port_frame > k_port_stack_size )
		myError( "Programmer Error: stack overflow while storing port!", true );
}

void RestorePort( void )
{
	PixMapHandle temp;

	// move the frame down one, so it points to the last pushed port
	g_stored_port_frame--;
	
	// make sure we haven't an empty stack!
	if( g_stored_port_frame < 0 )
		myError( "Programmer Error: stack underflow while popping port!", true );

	// unlock the pixels
//DebugStr( "\pgetting pixmap.." );
	temp = GetGWorldPixMap(g_stored_port[g_stored_port_frame]);
//DebugStr( "\pgot it.  Unlocking pixels" );
		UnlockPixels( temp );
//DebugStr( "\pdid it.  Setting GWorld" );
	// restore the previous port setting
	SetGWorld( g_stored_port[g_stored_port_frame], g_stored_device[g_stored_port_frame] );
//DebugStr( "\pit's set." );
}

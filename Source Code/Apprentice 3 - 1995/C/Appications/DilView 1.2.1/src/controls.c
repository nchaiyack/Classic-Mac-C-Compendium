/* controls.c */

#include "controls.h"
#include "windows.h"
#include "constants.h"
#include "error.h"
#include "globals.h"
#include "list.h"
#include "nyi.h"

ControlHandle g_scrollbar;

short GetListBufferOffsetFromControl( void )
{	
	return( GetCtlValue(g_scrollbar) );
}

Rect *GetScrollBarRect( Rect *rect )
{
	Rect screenRect;
	
	GetWinRect( &screenRect );
	
	SetRect( rect, screenRect.right - k_scrollbar_width,
		k_dil_height * GetScalePercent() + k_seperator_height - 1,
		screenRect.right + 1,
		screenRect.bottom - k_scrollbar_width + 1 );

	return( rect );
}

void myInitControls()
{
	Rect scroll_rect;
	
	GetScrollBarRect( &scroll_rect );
	
	g_scrollbar = NewControl( GetGWinPort(), &scroll_rect, "\p", true, 0, 0, 0, 
		scrollBarProc, 0L );
		
	if( g_scrollbar == nil )
		myError( "Couldn't create scrollbar! Sorry!", true );
}

void myDisposeControls( void )
{
	; // do nothing
}

void myDrawControls( void )
{
	Rect newClip, scrollRect;
	RgnHandle oldClip;

	// make sure the control range is valid (not always necessary, but its quick..)
	mySetControlMax();

	// set the clipping region to be the window - the dil rect
	GetWinRect( &newClip );
	newClip.top += k_dil_height * GetScalePercent(); 

	SetPortToWindow();

	oldClip = NewRgn();
	GetClip( oldClip );
	ClipRect( &newClip );
	
	GetScrollBarRect( &scrollRect );
	((*g_scrollbar)->contrlRect) = scrollRect;
//	MoveControl( g_scrollbar, scrollRect.left, scrollRect.top );

	DrawControls( GetGWinPort() );
	ShowControl( g_scrollbar );
//	UpdtControl( GetGWinPort(), GetGWinPort()->visRgn );
	
	DrawGrowIcon( GetGWinPort() );
	
	SetClip( oldClip );
	DisposeRgn( oldClip );
}

void mySetControlMax( void )
{
	GWorldPtr list_buf;
	Rect bufRect, screenRect;
	short rectDelta;
	
	// get the rects themselves
	list_buf = GetGListPort();
	bufRect = list_buf->portRect;
	GetListRect( &screenRect );
	
	// calculate the height difference
	rectDelta = (bufRect.bottom - bufRect.top) - (screenRect.bottom - screenRect.top);

	// can't have a negative value! (ie not long enough to warrent scrolling)
	if( rectDelta < 0 )
		rectDelta = 0;

	SetCtlMax( g_scrollbar, rectDelta );
}

pascal void myScrollProc( ControlHandle theControl, short partCode )
{
	short		curCtlValue, maxCtlValue, minCtlValue;
	WindowPtr	window;
	Rect		listScreenRect;
	short		jumpDelta, controlDelta = 0;
	
	// a jump should be a list rect height!
	GetListRect( &listScreenRect );
	jumpDelta = listScreenRect.bottom - listScreenRect.top;
	
	maxCtlValue = GetCtlMax( theControl );
	curCtlValue = GetCtlValue( theControl );
	minCtlValue = GetCtlMin( theControl );
	
	window = (**theControl).contrlOwner;
	
	switch( partCode )
	{
		case inPageDown:
			controlDelta = jumpDelta;
			break;
		case inDownButton:
			controlDelta = k_list_row_height;
			break;
		case inPageUp:
			controlDelta = -jumpDelta;
			break;
		case inUpButton:
			controlDelta = -k_list_row_height;
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
		DrawList();
	}
}

void myZeroScrollBar( void )
{
	SetCtlValue( g_scrollbar, 0 );
}


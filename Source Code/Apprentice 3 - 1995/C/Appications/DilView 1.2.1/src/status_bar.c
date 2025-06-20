/* status_bar.c */

#include <QuickDraw.h>
#include "status_bar.h"
#include "windows.h"
#include "globals.h"
#include "error.h"
#include "nyi.h"

GWorldPtr g_status_buffer;

void myInitStatusBar( void )
{
	Rect rect;
	QDErr err;

	// the full size offscreen buffer
	SetRect( &rect, 0, 0, (k_dil_width * GetScalePercent()) - k_scrollbar_width, 
		k_scrollbar_width - 1 );
	err = NewGWorld( &g_status_buffer, k_status_buffer_depth, &rect, nil, nil, 0 );
	if( err != noErr ) myError( "Couldn't create offscreen status bar buffer.  Try increasing memory allocation", true );

	// setup the font info for the port
	SetPortToGWorld( g_status_buffer );
	TextFont( geneva );
	TextSize( 9 );
	RestorePort();
}

void myDisposeStatusBar( void )
{
	DisposeGWorld( g_status_buffer );
}

void DrawStatusBar( void )
{
	Rect rect;
	static short last_scale = 0;
	short cur_scale;
	
	cur_scale = GetScale();
	
	if( last_scale != cur_scale )
	{
		last_scale = cur_scale;
		SetRect( &rect, 0, 0, (k_dil_width * GetScalePercent()) - k_scrollbar_width, 
			k_scrollbar_width - 1 );
		UpdateGWorld( &g_status_buffer, k_status_buffer_depth, &rect, nil, nil, 0 );
	}
	
	StatusBarMsg( k_std_message, "\p" );
}

Rect *GetStatusBarRect( Rect *rect )
{
	Rect win_rect;
	
	GetWinRect( &win_rect );
	
	SetRect( rect, 
		0, 
		win_rect.bottom - k_scrollbar_width + 1,
		win_rect.right - k_scrollbar_width,
		win_rect.bottom );
		
	return( rect );
}

void StatusBarMsg( short type, Str63 fname )
{
	Rect rect;
	RGBColor myColor;
	static short last_scale = 0;
	short cur_scale;
		
	cur_scale = GetScale();
	
	if( last_scale != cur_scale )
	{
		last_scale = cur_scale;
		SetRect( &rect, 0, 0, (k_dil_width * GetScalePercent()) - k_scrollbar_width, 
			k_scrollbar_width - 1 );
		UpdateGWorld( &g_status_buffer, k_status_buffer_depth, &rect, nil, nil, 0 );
	}
	
// WHEN PROCESSING AEOPENDOC, RESTORING THE BUFFER SOMETIMES CAUSES BUS ERRORS, UNTIL\
// I FIGURE THAT OUT, HERE's A LESS FRIENDLY BUT VIABLE WORK-AROUND, IE, SCREW THE PORT,
// JUST MAKE IT WORK.  HEY, YOU GET WHAT YOU PAY FOR..
//	SetPortToGWorld( g_status_buffer );
	SetGWorld( g_status_buffer, GetMainDevice() );

	myColor.red = myColor.blue = myColor.green = k_status_gray;
	RGBForeColor( &myColor );
	PaintRect( &(g_status_buffer->portRect) );
	ForeColor( blackColor );
	
	MoveTo( g_status_buffer->portRect.left, g_status_buffer->portRect.top );
	Move( k_status_text_offset_h, k_status_text_offset_v );
	
	switch( type )
	{
		case k_std_message:
				DrawString( k_standard_status_msg );
				break;
		case k_processing:
				DrawString( "\pProcessing �" );
				DrawString( fname );
				DrawString( "\p�" );
				break;
		case k_printing:
				DrawString( "\pPrinting �" );
				DrawString( fname );
				DrawString( "\p�" );
				break;
	}
//	RestorePort();
	
// UNCOMMENT THE FOLLOWING LINE TO TURN ON NEXT/PREVIOUS CONTROL DRAWING! (Not fully implemented)	
//	StatusBarControls();
		
	CopyGWorldToWindow( g_status_buffer, nil, GetStatusBarRect(&rect), srcCopy );
}

void StatusBarControls( void )
{
	Rect control_rect;
	short control_size;

	control_rect = g_status_buffer->portRect;
	control_rect.left = control_rect.right - 25;
	InsetRect( &control_rect, 0, 1 );

	control_size = control_rect.bottom - control_rect.top - 1;

	SetPortToGWorld( g_status_buffer );

	MoveTo( control_rect.left, control_rect.bottom - 1 );
	ForeColor( blackColor );
	Line( control_size, 0 );
	Line( 0, -control_size );
	ForeColor( whiteColor );
	Line( -control_size, 0 );
	Line( 0, control_size );
		
	RestorePort();
}


#include <stdio.h>
#include "mac-specific.h"
#include "TextDisplay.h"

extern WindowPtr	g_page_window,
					g_freemem_window,
					g_console_window,
					g_cancel_dialog;
extern Rect			g_screenrect;

typedef struct WPos {  /* custom resource 'WPos' to record window positions */
	Rect preferred;
	Rect fallback;
} Wpos, *PWPos, **HWPos;

/* routines defined externally */

/* routines used externally */
void	Reposition_windows( void );
void	Save_window_positions( void );
void	Center_window(	WindowPtr my_window );

/* private routines */
Boolean	Window_on_screen( Rect *content_rect, Boolean title_bar );
void	Reposition_window( WindowPtr the_window, HWPos pos, Boolean title_bar );
void	Set_window_rect( WindowPtr the_window, Rect *the_rect );
Point	Get_window_location( WindowPtr the_window );
void	Update_WPos( char *res_name, WindowPtr window );

/* ----------------------- Save_window_positions ----------------------- */
void
Save_window_positions()
{
	Update_WPos( "page", g_page_window );
	Update_WPos( "console", g_console_window );
	Update_WPos( "FreeMem", g_freemem_window );
	Update_WPos( "cancel", g_cancel_dialog );
	UpdateResFile( g_pref_resfile );
	(void) FlushVol( NIL, g_pref_vRefNum );
}

/* ---------------------- Update_WPos ---------------------------- */
void Update_WPos( char *res_name, WindowPtr window )
{
	short	res_id;
	HWPos	res_h;
	Rect	actual_rect;
	
	Get_resource( 'WPos', res_name, (Handle *)&res_h, &res_id );
	/*
		Previously I was using the bounding box of the content region
		as the window's rectangle.  But that doesn't work if the
		window is invisible.
	*/
	actual_rect = window->portRect;
	SetPort( window );
	LocalToGlobal( &topLeft(actual_rect) );
	LocalToGlobal( &botRight(actual_rect) );
	if (!EqualRect( &actual_rect, &(**res_h).preferred ))
	{
		(**res_h).preferred = actual_rect;
		ChangedResource( (Handle) res_h );
	}
}

/* ---------------------- Reposition_windows ---------------------------- */
void
Reposition_windows()
{
	HWPos pos;
	short res_id;
	
	Get_resource( 'WPos', "page", (Handle *)&pos, &res_id );
	Reposition_window( g_page_window, pos, FALSE );
	Get_resource( 'WPos', "console", (Handle *)&pos, &res_id );
	Reposition_window( g_console_window, pos, TRUE );
	SetPort( g_console_window );
	TD_resize();
	ShowWindow( g_console_window );
	Get_resource( 'WPos', "FreeMem", (Handle *)&pos, &res_id );
	Reposition_window( g_freemem_window, pos, FALSE );
	Get_resource( 'WPos', "cancel", (Handle *)&pos, &res_id );
	Reposition_window( g_cancel_dialog, pos, TRUE );
}

/* ---------------------- Reposition_window ---------------------------- */
void
Reposition_window( WindowPtr the_window, HWPos pos, Boolean title_bar )
{
	HLock( (Handle) pos );
	if ( Window_on_screen( &(**pos).preferred, title_bar ) )
		Set_window_rect( the_window, &(**pos).preferred );
	else
		Set_window_rect( the_window, &(**pos).fallback );
	HUnlock( (Handle) pos );
}

/* ---------------------- Set_window_rect ----------------------------- */
void
Set_window_rect( WindowPtr the_window, Rect *the_rect )
{
	MoveWindow( the_window, the_rect->left, the_rect->top, FALSE );
	if (!EqualPt( Get_window_location(the_window), topLeft(*the_rect)))
	{
		printf("\nThat's funny, a MoveWindow failed. I'll try again.\n");
		MoveWindow( the_window, the_rect->left, the_rect->top, FALSE );
	}
	SizeWindow( the_window, the_rect->right - the_rect->left,
		the_rect->bottom - the_rect->top, FALSE );
}

/* --------------------- Window_on_screen ------------------------------- */
Boolean
Window_on_screen( Rect *content_rect, Boolean title_bar )
{
	Rect	structure_rect;
	RgnHandle	structure_rgn, intersect_rgn, gray_rgn;
	Boolean		retval;
	
	structure_rect = *content_rect;
	if (title_bar)
		structure_rect.top -= 20;
	structure_rgn = NewRgn();
	RectRgn( structure_rgn, &structure_rect );
	gray_rgn = GetGrayRgn();
	if (EmptyRgn(gray_rgn))
		printf("\nThat's weird, the gray region is empty.\n");
	intersect_rgn = NewRgn();
	SectRgn( gray_rgn, structure_rgn, intersect_rgn );
	retval = EqualRgn( structure_rgn, intersect_rgn ) &&
		!EmptyRgn(structure_rgn);
	DisposeRgn( intersect_rgn );
	DisposeRgn( structure_rgn );
	return( retval );
}

/* ---------------------- Get_window_location ------------------------- */
/* 
	Find a window's top left corner.  Sort of an inverse to MoveWindow.
*/
Point
Get_window_location( WindowPtr the_window )
{
	Point corner;
	GrafPtr	save_port;
	
	GetPort( &save_port );
	SetPort( the_window );
	corner = topLeft( the_window->portRect );
	LocalToGlobal( &corner );
	SetPort( save_port );
	return( corner );
}

/* ----------------------- Center_window ----------------------- */
void
Center_window( the_window )
WindowPtr the_window;
{
	Rect oldrect;
	Point corner;
	
	oldrect = the_window->portRect; /* local coords */
	corner.h = g_screenrect.left +
		(g_screenrect.right - g_screenrect.left - oldrect.right) / 2;
	corner.v = g_screenrect.top +
		(g_screenrect.bottom - g_screenrect.top - oldrect.bottom) / 3;
	MoveWindow( the_window, corner.h, corner.v, FALSE );
	if (!EqualPt( Get_window_location(the_window), corner ))
	{
		printf("That's funny, a MoveWindow failed.  I'll try again.\n");
		MoveWindow( the_window, corner.h, corner.v, FALSE );
	}
}

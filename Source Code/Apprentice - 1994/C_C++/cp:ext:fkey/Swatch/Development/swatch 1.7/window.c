/**

	window.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#ifndef __content__
#include "content.h"
#endif
#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __display__
#include "display.h"
#endif
#ifndef __heap_list__
#include "heap_list.h"
#endif
#ifndef __prefs__
#include "prefs.h"
#endif
#ifndef __pstring__
#include "pstring.h"
#endif
#ifndef __resources__
#include "resources.h"
#endif
#ifndef __swatch__
#include "swatch.h"
#endif
#ifndef __window__
#include "window.h"
#endif


/**-----------------------------------------------------------------------------
 **
 ** Private Constants
 **
 **/

#define HORIZ_SCROLL_UNIT_DELTA		10

#define MIN_WINDOW_WIDTH	100
#define MIN_WINDOW_HEIGHT	100


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

static MBox_proc_t mbox_proc;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static void draw_grow_icon( Boolean active_state, Boolean rgn_state );

static void size_controls( void );

static pascal void vert_scroll_proc( ControlHandle the_control, int16 part );
static pascal void horiz_scroll_proc( ControlHandle the_control, int16 part );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

ControlHandle horiz_scroll, vert_scroll;
WindowPtr App_window;
int16 MBox_right;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Window_init( void )
{
	Rect bounds;

	bounds = Prefs.window_rect;
	App_window = This_mac.hasColorQD ?
			NewCWindow( nil, &bounds, (StringPtr) pstr(Window_title_STR_x), true,
					documentProc, (WindowPtr) -1, true, 0 ) :
			NewWindow( nil, &bounds, (StringPtr) pstr(Window_title_STR_x), true,
					documentProc, (WindowPtr) -1, true, 0 );

	SetPort( App_window );
	TextFont( geneva );
	TextSize( 9 );

	Window_clip_with_controls();
	SetRect( &bounds, 16384, 16384, 16384, 16384 );
	vert_scroll = NewControl( App_window, &bounds, nil, true, 0, 0, 0, scrollBarProc, 0 );
	horiz_scroll = NewControl( App_window, &bounds, nil, true, 0, 0, 0, scrollBarProc, 0 );
	size_controls();

	Check_color_usage();
}


void Window_idle( void )
{
	int32 old_largest;
	int16 update;

	Check_color_usage();
	old_largest = Heap_list_largest_heap_size();
	if ( Heap_list_update() ) {
		Window_reset_vert_scroll();
		update = UPDATE_COLUMNS | UPDATE_BOTTOM | FORCE_UPDATE;
	}
	else update = UPDATE_IDLE_COLUMNS;

	if ( Heap_list_largest_heap_size() != old_largest )	// largest has changed
		Window_reset_horiz_scroll();

	if ( Swatch_in_foreground() ) {
		Heap_info_handle_t h = nil;
		while ( h = l_next( heaps, h ) ) {
			Heap_info_t *hi = l_access( h );
			if ( hi->has_selection ) hi->selection_phase = (hi->selection_phase + 1) & 7;
			l_release( h );
		}
	}

	Display_lines( update, DISPLAY_ALL, DISPLAY_ALL );
}


Boolean Window_mousedown( EventRecord *the_event, int16 part )
{
	Boolean quit = false;
	Rect old_global_window, new_global_window;

	old_global_window = App_window->portRect;
	LocalToGlobal( (Point *)&old_global_window );
	LocalToGlobal( (Point *)&old_global_window.bottom );
	
	switch ( part ) {
	case inDrag: {
		Rect drag_rect;
		drag_rect = (**GrayRgn).rgnBBox;
		DragWindow( App_window, the_event->where, &drag_rect );
		break;
		}

	case inContent: {
		ControlHandle the_control;
		int16 i, j;

		GlobalToLocal( &the_event->where );
		Content_set_cursor( the_event );
		i = FindControl( the_event->where, App_window, &the_control );
		if ( the_control == vert_scroll ) {
			if ( i == inThumb ) {
				i = GetCtlValue( the_control );
				TrackControl( the_control, the_event->where, nil );
				if ( GetCtlValue( the_control ) != i ) {
					Display_update_row_tops();
					Display_lines( UPDATE_EVERYTHING | FORCE_UPDATE,
							DISPLAY_ALL, DISPLAY_ALL );
				}
			}
			else
				TrackControl( the_control, the_event->where,
						(ProcPtr) vert_scroll_proc );
		}
		else if ( the_control == horiz_scroll ) {
			if ( i == inThumb ) {
				i = GetCtlValue( the_control );
				TrackControl( the_control, the_event->where, nil );
				if ( ( j = GetCtlValue( the_control ) ) != i ) {
					Content_horiz_scrolled( j );
					Display_lines( UPDATE_HEAP | UPDATE_SELECTION,
							DISPLAY_ALL, DISPLAY_ALL );
				}
			}
			else
				TrackControl( the_control, the_event->where,
						(ProcPtr) horiz_scroll_proc );
		}
		else Content_mousedown( the_event );
		break;
	}

	case inGrow: {
		uns32 new_size;
		Rect size_rect;

		SetRect( &size_rect, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, 32767, 32767 );
		new_size = GrowWindow( App_window, the_event->where, &size_rect );
		if ( new_size ) {
			draw_grow_icon( false, false );
			HideControl( horiz_scroll );
			HideControl( vert_scroll );
			Window_erase_mbox();
			SizeWindow( App_window, (int16) new_size, new_size >> 16, true );
			Window_clip_with_controls();
			size_controls();
			Window_update_mbox();
			draw_grow_icon( true, true );
			Display_header( UPDATE_HEAP_SCALE );
		}
		break;
	}

	case inGoAway:
		if ( TrackGoAway( App_window, the_event->where ) )
			quit = true;
		break;

	default:
		break;
	}

	new_global_window = App_window->portRect;
	LocalToGlobal( (Point *)&new_global_window );
	LocalToGlobal( (Point *)&new_global_window.bottom );
	if ( !EqualRect( &old_global_window, &new_global_window ) ) {
		Prefs.window_rect = new_global_window;
		Prefs.dirty = true;
	}

	return quit;
}


void Window_update( EventRecord *the_event )
{
	BeginUpdate( App_window );
	UpdtControl( App_window, App_window->visRgn );
	Window_update_mbox();
	draw_grow_icon( Swatch_in_foreground(), true );
	Display_lines( UPDATE_COLUMNS_AND_BORDER | FORCE_UPDATE, DISPLAY_ALL, DISPLAY_ALL );
	EndUpdate( App_window );
}


void Window_activate( EventRecord *the_event, Boolean active_state )
{
	int16 control_state;

	draw_grow_icon( active_state, true );
	control_state = active_state ? 0 : 255;
	HiliteControl( vert_scroll, control_state );
	HiliteControl( horiz_scroll, control_state );
	GlobalToLocal( &the_event->where );
	Content_set_cursor( the_event );
}


void Window_close( void )
{
	CloseWindow( App_window );
}


ControlHandle Window_horiz_scroll( void )
{
	return horiz_scroll;
}


ControlHandle Window_vert_scroll( void )
{
	return vert_scroll;
}


Boolean Window_reset_horiz_scroll( void )
{
	return Content_reset_horiz( horiz_scroll );
}


void Window_reset_vert_scroll( void )
{
	int16 i;
	int16 old_rows, old_value;
	Rect r;

	old_rows = Content_rows();
	old_value = GetCtlValue( vert_scroll );
	Content_reset_vert();

	i = Heap_list_count() - Content_rows();
	if ( i < 0 ) i = 0;
	SetCtlMax( vert_scroll, i );

	if ( old_value && Content_rows() > old_rows ) {
		old_value -= Content_rows() - old_rows;
		SetCtlValue( vert_scroll, ( old_value < 0 ? 0 : old_value ) );
		r = App_window->portRect;
		r.top += CELL_HEIGHT;
		r.right -= 15;
		r.bottom -= 15;
		InvalRect( &r );
	}
}



void Window_set_mbox( MBox_proc_t *handler )
{
	mbox_proc = handler;
}


void Window_erase_mbox( void )
{
	Rect r;

	r.left = 0;
	r.right = MBox_right;
	r.top = App_window->portRect.bottom - 15;
	r.bottom = App_window->portRect.bottom;

	EraseRect( &r );
	InvalRect( &r );
}


void Window_update_mbox( void )
{
	Rect r;
	RgnHandle save_clipRgn;

	r.left = 0;
	r.right = MBox_right;
	r.top = App_window->portRect.bottom - 14;
	r.bottom = App_window->portRect.bottom;
	EraseRect( &r );

	save_clipRgn = NewRgn();
	GetClip( save_clipRgn );
	InsetRect( &r, 1, 1 );
	ClipRect( &r );

	if ( mbox_proc ) mbox_proc( &r );

	SetClip( save_clipRgn );
	DisposeRgn( save_clipRgn );
}


void Window_clip_with_controls( void )
{
	ClipRect( &App_window->portRect );
}


void Window_clip_without_controls( void )
{
	Rect r;

	r = App_window->portRect;
	r.right -= 15;
	r.bottom -= 15;
	ClipRect( &r );
}


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static void draw_grow_icon( Boolean active_state, Boolean rgn_state )
{
	Boolean save_hilited;
	Rect r;

	r = App_window->portRect;

	Set_fore_color_or_pattern( HEADER_BORDER_COLOR );
	MoveTo( MBox_right, BORDER_BOTTOM );
	LineTo( MBox_right, r.bottom - 15 );
	Set_fore_color_or_pattern( BLACK_COLOR );

	r.top += BORDER_BOTTOM;
	ClipRect( &r );
	save_hilited = ((WindowPeek) App_window)->hilited;
	((WindowPeek) App_window)->hilited = active_state;
	DrawGrowIcon( App_window );
	((WindowPeek) App_window)->hilited = save_hilited;
	r.top = r.bottom - 15;
	r.left = r.right - 15;
	if ( rgn_state )
		ValidRect( &r );
	else
		InvalRect( &r );
	Window_clip_with_controls();
}


static void size_controls( void )
{
	int16 top, left, bottom, right;

	Window_reset_horiz_scroll();
	Window_reset_vert_scroll();

	left = App_window->portRect.right - 15;
	right = App_window->portRect.right + 1;
	top = BORDER_BOTTOM - 1;
	bottom = App_window->portRect.bottom - 14;
	HideControl( vert_scroll );
	MoveControl( vert_scroll, left, top );
	SizeControl( vert_scroll, right - left, bottom - top );
	ShowControl( vert_scroll );

	left = MBox_right;
	right = App_window->portRect.right - 14;
	top = App_window->portRect.bottom - 15;
	bottom = App_window->portRect.bottom + 1;
	HideControl( horiz_scroll );
	MoveControl( horiz_scroll, left, top );
	SizeControl( horiz_scroll, right - left, bottom - top );
	ShowControl( horiz_scroll );
}


static pascal void vert_scroll_proc( ControlHandle the_control, int16 part )
{
	Rect r;
	int16 old, new;
	RgnHandle aRgn;

	aRgn = NewRgn();
	r = App_window->portRect;
	r.right -= 15;
	r.top = BORDER_BOTTOM;
	r.bottom -= 15;
	old = GetCtlValue( the_control );

	if ( part == inUpButton ) {
		if ( old ) {
			SetCtlValue( the_control, old - 1 );
			ScrollRect( &r, 0, CELL_HEIGHT, aRgn );
			Display_update_row_tops();
			Display_lines( UPDATE_COLUMNS | FORCE_UPDATE, 0, 0 );
		}
	}

	else if ( part == inDownButton ) {
		if ( old < Heap_list_count() - Content_rows() ) {
			SetCtlValue( the_control, old + 1 );
			ScrollRect( &r, 0, -CELL_HEIGHT, aRgn );
			Display_update_row_tops();
			Display_lines( UPDATE_COLUMNS | UPDATE_BOTTOM | FORCE_UPDATE,
					Content_rows() - 1,
					Content_rows() + Content_partial_row() - 1 );
		}
	}

	else if ( part == inPageUp ) {
		new = old - ( Content_rows() - 1 );
		if ( new < 0 )
			new = 0;
		if ( new != old ) {
			SetCtlValue( the_control, new );
			Display_update_row_tops();
			Display_lines( UPDATE_COLUMNS | FORCE_UPDATE, DISPLAY_ALL, DISPLAY_ALL );
		}
	}

	else if ( part == inPageDown ) {
		new = old + ( Content_rows() - 1 );
		if ( new > Heap_list_count() - Content_rows() )
			new = Heap_list_count() - Content_rows();
		if ( new != old ) {
			SetCtlValue( the_control, new );
			Display_update_row_tops();
			Display_lines( UPDATE_COLUMNS | UPDATE_BOTTOM | FORCE_UPDATE, DISPLAY_ALL,
					DISPLAY_ALL );
		}
	}

	DisposeRgn( aRgn );

	Set_fore_color_or_pattern( HEADER_BORDER_COLOR );
	MoveTo( MBox_right, BORDER_BOTTOM );
	LineTo( MBox_right, App_window->portRect.bottom - 15 );
	Set_fore_color_or_pattern( BLACK_COLOR );
}


static pascal void horiz_scroll_proc( ControlHandle the_control, int16 part )
{
	Content_scroll_horiz( the_control, part, HORIZ_SCROLL_UNIT_DELTA );
}

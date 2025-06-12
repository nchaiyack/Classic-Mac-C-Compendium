/**

	content.c
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
#ifndef __info__
#include "info.h"
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
 ** Private Variables
 **
 **/

static int16 heap_cursor;
static int16 num_display_lines;		// number of whole rows visible in current window
static int16 partial_display_line;	// partial row at bottom also visible

static int32 display_heap_bytes,	// number of heap bytes visible in current window
		horiz_scroll_scale,
		heap_offset_pixels,
		heap_right_pixels;
static int16 heap_cursor;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static void mousedown_in_updating( EventRecord *the_event, Heap_info_handle_t h );
static void mousedown_in_name( EventRecord *the_event, Heap_info_handle_t h );
static void mousedown_in_heap( EventRecord *the_event, Heap_info_handle_t h );
static void mousedown_move_border( EventRecord *the_event );
static void mousedown_in_border( EventRecord *the_event );
static void mousedown_in_bomb( EventRecord *the_event );

static void swatch( unsigned char *str );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

int16 mbox_type = 0;
unsigned char mbox_heap_appname[32];
THz mbox_heap_zone;
Ptr mbox_heap_current_selection_logical_start;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Content_init( void )
{
}


Boolean Content_find_point( Point where, Heap_info_handle_t *hp, int16 *type )
{
	Heap_info_handle_t h;
	Heap_info_t *hi;
	column_t *col;
	int i;
	Boolean found;
	Rect contents_rect;

	contents_rect = App_window->portRect;
	contents_rect.right -= 15;
	contents_rect.bottom -= 15;
	if ( !PtInRect( where, &contents_rect ) ) return false;

	h = nil;
	found = false;
	while ( !found && (h = l_next( heaps, h )) ) {

 		hi = l_access( h );

		if ( where.v >= hi->row.top && where.v <= hi->row.bottom ) {

			for ( i = hi->row.max_columns, col = &hi->row.column[i]; i >= 0; --i, --col ) {
				if ( where.h >= col->left &&
					 where.h <= col->right ) {

					if ( hp ) *hp = h;
					if ( type ) *type = col->cell->type;
					found = true;
					break;

				}

			}

		}

		l_release( h );
	}

	return found;
}


void Content_mousedown( EventRecord *the_event )
{
	Heap_info_handle_t h;
	int16 type;

	if ( Content_find_point( the_event->where, &h, &type ) ) {

		if ( type == colUpdating ) mousedown_in_updating( the_event, h );
		else if ( type == colName ) mousedown_in_name( the_event, h );
		else if ( type == colHeap ) mousedown_in_heap( the_event, h );

	}
	else if ( heap_cursor == MOVE_BORDER_CURS ) mousedown_move_border( the_event );
	else if ( the_event->where.v <= BORDER_BOTTOM ) mousedown_in_border( the_event );
	else if ( Debugger_installed &&
			  the_event->where.v >= App_window->portRect.bottom - 15 &&
			  the_event->where.h < 19 ) mousedown_in_bomb( the_event );
}


void Content_set_cursor( EventRecord *the_event )
{
	Heap_info_handle_t h;
	int16 type;
	Rect r;
	int16 new_cursor;

	new_cursor = ARROW_CURS;

	if ( the_event->where.h >= MBox_right - 1 &&
		 the_event->where.h <= MBox_right + 1 ) new_cursor = MOVE_BORDER_CURS;

	else if ( Content_find_point( the_event->where, &h, &type ) ) {

		if ( type == colHeap ) {

			if ( the_event->modifiers & optionKey )

				new_cursor = (Prefs.heap_scale < MAX_HEAP_SCALE) ?
					ZOOM_OUT_CURS : NO_ZOOM_CURS;
	
			else if ( the_event->modifiers & cmdKey )

				new_cursor = INFO_CURS;

			else new_cursor = (Prefs.heap_scale > MIN_HEAP_SCALE) ?
						ZOOM_IN_CURS : NO_ZOOM_CURS;

		}

	}


	if ( new_cursor != heap_cursor ) {

		heap_cursor = new_cursor;

		if ( heap_cursor == ARROW_CURS ) InitCursor();

		else {
			Handle cursor_h;

			cursor_h = GetResource( 'CURS', new_cursor );
			assert( cursor_h, "\p Content_set_cursor(): couldn't get cursor" );

			HLock( cursor_h );
			SetCursor( (Cursor *) *cursor_h );
			HUnlock( cursor_h );
			HPurge( cursor_h );

		}
	}
}


int16 Content_rows( void )
{
	return num_display_lines;
}


int16 Content_partial_row( void )
{
	return partial_display_line;
}


void Content_reset_vert( void )
{
	int16 i;

	i = App_window->portRect.bottom - 15 - BORDER_BOTTOM;
	partial_display_line = (i % CELL_HEIGHT) != 0;
	num_display_lines = i / CELL_HEIGHT;
}


void Content_scroll_horiz( ControlHandle the_control, int16 part, int16 unit_delta )
{
	int32 delta, old_heap_offset_pixels;

	if ( !part )
		return;

	old_heap_offset_pixels = heap_offset_pixels;
	if ( part == inUpButton )
		delta = -unit_delta;
	else if ( part == inDownButton )
		delta = unit_delta;
	else {
		delta = (display_heap_bytes >> Prefs.heap_scale_2n) - unit_delta;
		if ( part == inPageUp )
		delta = -delta;
	}

	heap_offset_pixels += delta;
	if ( heap_offset_pixels < 0 )
		heap_offset_pixels = 0;
	else if ( heap_offset_pixels > heap_right_pixels )
		heap_offset_pixels = heap_right_pixels;
	
	if ( heap_offset_pixels != old_heap_offset_pixels ) {
		SetCtlValue( the_control, heap_offset_pixels / horiz_scroll_scale );
		Display_lines( UPDATE_HEAP | UPDATE_SELECTION, DISPLAY_ALL, DISPLAY_ALL );
	}
}


void Content_horiz_scrolled( int cvalue )
{
	heap_offset_pixels = (int32) cvalue * horiz_scroll_scale;
}


Boolean Content_reset_horiz( ControlHandle the_control )
{
	int32 i, largest_heap_size;
	Heap_info_t *hi;
	int16 first_heap_pixel;
	Heap_info_handle_t h;
	int16 max, value;

	h = l_next( heaps, nil );
	hi = l_access( h );

	largest_heap_size = Heap_list_largest_heap_size();
	first_heap_pixel = hi->row.column[hi->row.max_columns].left;

	l_release( h );

	display_heap_bytes = (int32) (App_window->portRect.right - 15 - first_heap_pixel) *
			Prefs.heap_scale;
	i = largest_heap_size - display_heap_bytes;
	if ( i > 0 ) {
		heap_right_pixels = ( i >> Prefs.heap_scale_2n ) +
				( (i & (Prefs.heap_scale-1)) ? 1 : 0 ) + 20;
		horiz_scroll_scale = heap_right_pixels / 32768L + 1;
	}
	else {
		heap_right_pixels = 0;
		horiz_scroll_scale = 1;
	}

	max = heap_right_pixels / horiz_scroll_scale +
			((heap_right_pixels % horiz_scroll_scale) ? 1 : 0);

	value = GetCtlValue( the_control );
	SetCtlMax( the_control, max );
	if ( value > max ) {
		Content_horiz_scrolled( max );  // scroll position changed
		return true;
	}
	return false;
}


int32 Content_heap_offset_pixels( void )
{
	return heap_offset_pixels;
}


void *Content_pixel_to_address( Heap_info_t *hi, int32 pixel_h )
{
	int32 pixel;
	int16 first_heap_pixel;

	first_heap_pixel = hi->row.column[hi->row.max_columns].left;

	pixel = heap_offset_pixels + pixel_h - (first_heap_pixel);
	return (void *)((pixel << Prefs.heap_scale_2n) + (int32)&hi->zone->heapData);
}


int32 Content_address_to_pixel( Heap_info_t *hi, void *address )
{
	int32 pixel;
	int16 first_heap_pixel;

	first_heap_pixel = hi->row.column[hi->row.max_columns].left;

	pixel = ((int32)address - (int32)&hi->zone->heapData) >> Prefs.heap_scale_2n;
	return pixel - heap_offset_pixels + first_heap_pixel;
}


/*--------*/

#ifdef __IGNORE__
	colUpdating		= 1 << 0,
	colName			= 1 << 1,
	colSize			= 1 << 2,
	colFree			= 1 << 3,
	colIslands		= 1 << 4,
	colHeap			= 1 << 5

if one column:
  Heap Size  -->  Free  -->  Islands  -->  Heap Size

if two columns:
  Heap Size / Free  -->  Heap Size / Islands  -->  Free / Heap Size  -->  Free / Islands
  -->  Islands / Heap Size  --> Islands / Free  -->  Heap Size / Free

if three columns:
  Heap Size / Free / Islands  -->  Heap Size / Islands / Free
  -->  Free / Heap Size / Islands  -->  Free / Islands / Heap Size
  -->  Islands / Heap Size / Free  -->  Islands / Free / Heap Size
  -->  Heap Size / Free / Islands

#endif

static void mousedown_move_border( EventRecord *the_event )
{
	while ( StillDown() ) {

		GetMouse( &the_event->where );
		
	}
}


static void mousedown_in_border( EventRecord *the_event )
{
}


static void mousedown_in_bomb( EventRecord *the_event )
{
	Boolean in_bomb, mouse_in_bomb;
	Rect r;

	SetRect( &r, 0, App_window->portRect.bottom - 14, 20, App_window->portRect.bottom );

	in_bomb = false;
	while ( StillDown() ) {

		GetMouse( &the_event->where );
		mouse_in_bomb = PtInRect( the_event->where, &r );
		if ( ( mouse_in_bomb && !in_bomb ) || ( !mouse_in_bomb && in_bomb ) ) {
			in_bomb ^= 1;
			InvertRect( &r );
		}

	}

	if ( in_bomb ) {
		unsigned char str[256];

		if ( mbox_type == MBOX_HEAP_ADDRESS ) {
			pstrcopy( pstr(MacsBug_Looking_at_STR_x), str );
			pstrappend( mbox_heap_appname, str );
			pstrappend( pstr(MacsBug_hx_STR_x), str );
			phexappend( (int32) mbox_heap_zone, 8, str );
		}

		else if ( mbox_type == MBOX_ADDRESS ) {
			pstrcopy( pstr(MacsBug_Looking_at_STR_x), str );
			pstrappend( mbox_heap_appname, str );
			pstrappend( pstr(MacsBug_hx_STR_x), str );
			phexappend( (int32) mbox_heap_zone, 8, str );
			pstrappend( pstr(MacsBug_wh_STR_x), str );
			phexappend( (int32) mbox_heap_current_selection_logical_start, 8, str );
		}

		else str[0] = 0;

		swatch( str );
		InvertRect( &r );

	}
}


static void mousedown_in_updating( EventRecord *the_event, Heap_info_handle_t h )
{
	unsigned char *mbox_text;
	Heap_info_t *hi;
	int16 i, j;
	unsigned char str[256];
	Boolean scrolled;

	mbox_text = MBox_text();

	hi = l_access( h );
	hi->updating ^= 1;
	l_release( h );


	scrolled = Window_reset_horiz_scroll();  // may've turned on/off longest heap

	if ( Heap_list_resort() )
		Display_lines( UPDATE_COLUMNS | FORCE_UPDATE, DISPLAY_ALL, DISPLAY_ALL );
	else {
		if ( scrolled )
			Display_lines( UPDATE_HEAP | UPDATE_SELECTION | FORCE_UPDATE, DISPLAY_ALL,
					DISPLAY_ALL );  // also erases row just turned off
		else
			Display_row( UPDATE_COLUMNS | FORCE_UPDATE, h );
	}

	while ( StillDown() );
}


static void mousedown_in_name( EventRecord *the_event, Heap_info_handle_t h )
{
	unsigned char *mbox_text;
	Heap_info_t *hi;
	int16 i, j;
	unsigned char str[256];

	hi = l_access( h );

	if ( hi->updating ) {
		THz save_zone, compact_zone;

		mbox_text = MBox_text();

		mbox_type = MBOX_HEAP_ADDRESS;
		record_for_mbox( hi );
		pstrcopy( "\pHeap is at $", mbox_text );
		phexappend( (int32) hi->zone, 8, mbox_text );
		pstrappend( "\p (", mbox_text );
		pstrappend( hi->appname, mbox_text );
		pchappend( ')', mbox_text );
		Window_update_mbox();

		{
			Rect r;
			uns32 ticker;
	
			ticker = TickCount() + 2;
			r.top = hi->row.top;
			r.bottom = hi->row.bottom;
			r.left = hi->row.column[1].left;
			r.right = hi->row.column[1].right;
			InvertRoundRect( &r, 4, 4 );
	
			compact_zone = hi->zone;
			l_release( h );	// so block can move, if this is the block's heap
	
			save_zone = TheZone;
			TheZone = compact_zone;
			PurgeMem( 0x7FFFFFFF );
			CompactMem( 0x7FFFFFFF );
			TheZone = save_zone;
	
			hi = l_access( h );
	
			while ( ticker > TickCount() );
			InvertRoundRect( &r, 4, 4 );
		}

		Display_row( UPDATE_COLUMNS, h );
	}

	l_release( h );

	while ( StillDown() );
}


static void swatch( unsigned char *str )
{
	DebugStr( str );
}


static void mousedown_in_heap( EventRecord *the_event, Heap_info_handle_t h )
{
	Heap_info_t *hi;
	int16 first_heap_pixel;

	hi = l_access( h );

	switch ( heap_cursor ) {

	case ZOOM_IN_CURS:
	case ZOOM_OUT_CURS:

		if ( heap_cursor == ZOOM_IN_CURS ) {

			Prefs.heap_scale >>= 1;
			--Prefs.heap_scale_2n;

		}
		else {

			Prefs.heap_scale <<= 1;
			++Prefs.heap_scale_2n;

		}
		Prefs.dirty = true;

		Window_reset_horiz_scroll();


// adjust offset so that we stay centered on the click location

		first_heap_pixel = hi->row.column[hi->row.max_columns].left;

		if ( (the_event->where.h - first_heap_pixel > 35) || heap_offset_pixels ) {

			heap_offset_pixels = ( heap_cursor == ZOOM_IN_CURS ) ?
					(heap_offset_pixels << 1) + the_event->where.h - first_heap_pixel :
					(heap_offset_pixels - (the_event->where.h - first_heap_pixel)) >> 1;

		}


		if ( heap_offset_pixels < 0 ) heap_offset_pixels = 0;
		else if ( heap_offset_pixels > heap_right_pixels )
			heap_offset_pixels = heap_right_pixels;

		SetCtlValue( Window_horiz_scroll(), heap_offset_pixels / horiz_scroll_scale );
		Display_lines( UPDATE_HEAP | UPDATE_HEAP_SCALE, DISPLAY_ALL, DISPLAY_ALL );
		break;


	case INFO_CURS:
		Info_track_heap( the_event, h );
		break;

	}

	l_release( h );
}


void record_for_mbox( Heap_info_t *hi )
{
	pstrcopy( hi->appname, mbox_heap_appname );
	mbox_heap_zone = hi->zone;
	mbox_heap_current_selection_logical_start = hi->current_selection.logical_start;
}
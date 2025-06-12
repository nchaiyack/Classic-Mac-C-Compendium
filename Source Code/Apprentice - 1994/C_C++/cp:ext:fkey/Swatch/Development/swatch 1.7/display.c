/**

	display.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __display__
#include "display.h"
#endif
#ifndef __content__
#include "content.h"
#endif
#ifndef __heap__
#include "heap.h"
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
 ** Private Macros
 **
 **/

/**-----------------------------------------------------------------------------
 **
 ** Private Constants
 **
 **/

#define MIN_DEPTH_FOR_COLOR				4
#define COLOR8							0
#define COLOR4							1
#define GRAY8							2
#define GRAY4							3

#define MBOX_HEAP_ADDRESS				1
#define MBOX_ADDRESS					2

#define MIN_COLUMN_LEFT			3

#define NAME_TOP_DELTA			2
#define SIZE_TOP_DELTA			2
#define FREE_TOP_DELTA			2
#define ISLANDS_TOP_DELTA		2
#define TEXT_HEIGHT				12


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

static unsigned char mbox_text[256] = {0};

static CTabHandle cluts[4];
static CTabHandle current_clut;
static int16 clut_index;
static Handle patterns, marching_ants;	/* PAT# resource */
static Boolean color_is_dead = false;


typedef void (*display_proc_t)( Heap_info_t *hi, column_t *col, Boolean force );


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

static void display_updating( Heap_info_t *hi, column_t *col, Boolean force );
static void display_name( Heap_info_t *hi, column_t *col, Boolean force );
static void display_current_size( Heap_info_t *hi, column_t *col, Boolean force );
static void display_current_free( Heap_info_t *hi, column_t *col, Boolean force );
static void display_current_islands( Heap_info_t *hi, column_t *col, Boolean force );
static void display_heap( Heap_info_t *hi, column_t *col, Boolean force );
static void display_selection( struct _t_Heap_info *hi, column_t *col, Boolean force );

static void add_commas( unsigned char *num );
static void draw_mbox( Rect *r );


static cell_t cells[] = {
	{ colUpdating,	(ProcPtr) display_updating,			9, 5,		0 },
	{ colName,		(ProcPtr) display_name,				70, 5, 		0 },
	{ colSize,		(ProcPtr) display_current_size,		70, 5, 		Heap_Size_STR_x },
	{ colFree,		(ProcPtr) display_current_free,		70, 5, 		Heap_Free_STR_x },
	{ colIslands,	(ProcPtr) display_current_islands,	50, 10,		Heap_Islands_STR_x },
	{ colHeap,		(ProcPtr) display_heap,				16384, 0,	0 }
};


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

Boolean Use_color;
int16 Max_columns = 5;  // number of elements below - 1
int16 Column_types[6] = { colUpdating, colName, colSize, colFree, colIslands, colHeap };


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

void Display_init( void )
{
	Handle h;

	patterns = GetResource( 'PAT#', BLACK_WHITE_PAT_ );
	if ( !patterns ) Bail( OUT_OF_MEMORY_ALRT );

	marching_ants = GetResource( 'PAT#', MARCHING_ANTS_PAT_ );
	if ( !marching_ants ) Bail( OUT_OF_MEMORY_ALRT );

	Window_set_mbox( draw_mbox );


	h = GetResource( 'STUF', 1 );
	if ( h ) {

		int16 *p;
		int i;

		HLock( h );

		p = (int16 *) *h;
		Max_columns = *p++;
		for ( i = 0; i <= Max_columns; ++i ) Column_types[i] = *p++;

		HUnlock( h );
		ReleaseResource( h );

	}

}


void Display_update_row_tops( void )
{
	Heap_info_handle_t h;
	Heap_info_t *hi;
	int16 top;

	top = BORDER_BOTTOM - GetCtlValue( vert_scroll ) * CELL_HEIGHT;
	h = nil;
	while ( h = l_next( heaps, h ) ) {
		Heap_info_t *hi = l_access( h );
		hi->row.top = top;
		hi->row.bottom = top + CELL_HEIGHT;
		top += CELL_HEIGHT;
		l_release( h );
	}
}


void Display_new_row_arrangement( int16 max_columns, int16 *column_types )
{
	Heap_info_handle_t h;
	int i;

	assert( max_columns >= 2, "\p Display_new_row_arrangement(): too few columns" );
	Max_columns = max_columns;

	for ( i = 0; i <= max_columns; ++i ) Column_types[i] = *column_types++;

	h = nil;
	while ( (h = l_next( heaps, h )) ) Display_new_row( h );

}


void Display_new_row( Heap_info_handle_t h )
{
	Heap_info_t *hi;
	int i, j;
	int16 left;
	column_t *col;

	hi = l_access( h );

	hi->row.max_columns = Max_columns;

	left = MIN_COLUMN_LEFT;
	for ( i = 0, col = &hi->row.column[0]; i <= hi->row.max_columns; ++i, ++col ) {

		for ( j = MAX_COLUMNS - 1; j >= 0; --j )
			if ( cells[j].type == Column_types[i] ) break;
		assert ( j >= 0, "\p Display_build_row(): couldn't find column type" );

		col->cell = &cells[j];
		col->left = left;
		col->right = left + col->cell->width;
		left = col->right + col->cell->space;

	}

	col = Display_find_column( hi, colHeap );
	assert( col != &hi->row.column[0], "\p Display_new_row(): can't handle heap as first column" );
	MBox_right = (col->left + (col-1)->right) / 2 - 1;

	hi->last_islands = -1;

	l_release( h );
}


int16 Display_row( int16 update, Heap_info_handle_t h )
{
	Boolean force;
	Heap_info_t *hi;
	column_t *col;
	int j;
	Rect clip, contents;
	int16 bottom;

	if ( !update ) return;

	force = (update & FORCE_UPDATE) != 0;

	Heap_list_update_one( h );
	hi = l_access( h );

	contents.left = 0;
	contents.top = BORDER_BOTTOM;
	contents.right = App_window->portRect.right - 15;
	contents.bottom = App_window->portRect.bottom - 15;
	SetRect( &clip, 0, hi->row.top, 16384, hi->row.bottom );
	SectRect( &clip, &contents, &clip );

	if ( !EmptyRect( &clip ) ) {

		if ( hi->updating ) {

			hi->last_updating = true;

			ClipRect( &clip );

			for ( j = hi->row.max_columns, col = &hi->row.column[j]; j >= 0; --j, --col ) {
		
				if ( col->cell->type & update )
					((display_proc_t)col->cell->draw)( hi, col, force );
		
			}
			if ( update & UPDATE_SELECTION ) display_selection( hi, nil, force );

			Window_clip_with_controls();

		}

		else {

			if ( colName & update ) {

				ClipRect( &clip );
				col = Display_find_column( hi, colName );
				((display_proc_t)col->cell->draw)( hi, col, force );
				Window_clip_with_controls();

			}

			if ( hi->last_updating ) {

				int16 right = clip.right;
	
				hi->last_updating = false;  // = hi->updating

				col = Display_find_column( hi, colUpdating );
				clip.left = col->left;
				clip.right = col->right;
				EraseRect( &clip );

				col = Display_find_column( hi, colName );
				clip.left = col->right;
				clip.right = MBox_right;
				EraseRect( &clip );

				clip.left = MBox_right + 1;
				clip.right = right;
				EraseRect( &clip );

			}

		}


	}
	bottom = hi->row.bottom;


	l_release( h );

	return bottom;
}


void Display_lines( int16 update, int16 line_start, int16 line_end )
{
	Heap_info_handle_t h;
	int16 bottom;
	int i;

	if ( !update ) return;

	if ( line_start == DISPLAY_ALL ) {
		Display_header( update );
		line_start = 0;
		line_end = Content_rows() - 1;
		if ( Content_partial_row() )
			++line_end;
	}

	line_start += GetCtlValue( vert_scroll );
	line_end += GetCtlValue( vert_scroll );

	h = nil;
	for ( i = line_start; i; --i ) h = l_next( heaps, h );


	bottom = BORDER_BOTTOM + 1;
	for ( i = line_start; (h = l_next( heaps, h )) && i <= line_end; ++i )
		bottom = Display_row( update, h );

	
	if ( update & UPDATE_BOTTOM ) {
		Rect erase;

		erase.bottom = App_window->portRect.bottom - 15;

		if ( bottom < erase.bottom ) {

			erase.top = bottom;
			erase.left = 0;
			erase.right = MBox_right;
			EraseRect( &erase );
			erase.left = MBox_right + 1;
			erase.right = App_window->portRect.right - 15;
			EraseRect( &erase );

		}
	}
}


void Display_header( int16 update )
{
	unsigned char header_text1[50], header_text2[50];
	unsigned char num[30];
	uns32 pixels_visible;
	Rect erase;
	Point pen;
	int16 header_text2_pos;
	unsigned char *str;
	int16 width;
	int i, x;
	Heap_info_handle_t h;
	Heap_info_t *hi;

 	h = l_next( heaps, nil );
 	hi = l_access( h );

	if ( update & UPDATE_HEADER ) {

		erase.left = 0;
		erase.right = MBox_right;
		erase.top = BORDER_TOP;
		erase.bottom = BORDER_BOTTOM - 1;
		EraseRect( &erase );
	
		Set_fore_color( HEADER_COLOR );
	
		for ( i = hi->row.max_columns; i >= 0; --i ) {
	
			x = hi->row.column[i].cell->name;
			if ( x ) {
	
				str = pstr( x );
				width = StringWidth( str );
	
				MoveTo( hi->row.column[i].right - width, CELL_HEIGHT - 2 );
				DrawString( str );
	
			}
	
		}
	}


	if ( update & UPDATE_HEAP_SCALE ) {

		erase.left = MBox_right;
		erase.right = 16384;
		erase.top = BORDER_TOP;
		erase.bottom = BORDER_BOTTOM - 1;
		EraseRect( &erase );

		pstrcopy( pstr(Heap_pixel_1_STR_x), header_text1 );
		pnumcopy( Prefs.heap_scale, num );
		add_commas( num );
		pstrappend( num, header_text1 );
		pstrappend( pstr(Heap_pixel_bytes_STR_x), header_text1 );
	
		pixels_visible = App_window->portRect.right - 15 -
				Display_find_column( hi, colHeap )->left;
		pnumcopy( pixels_visible * Prefs.heap_scale, header_text2 );
		add_commas( header_text2 );
		pstrappend( pstr(Heap_pixel_visible_STR_x), header_text2 );
	
		Set_fore_color_or_pattern( HEADER_BORDER_COLOR );
		MoveTo( 0, BORDER_BOTTOM - 1 );
		Line( 16384, 0 );
	
		Set_fore_color( HEADER_COLOR );
		MoveTo( MBox_right + 10, BORDER_BOTTOM - 2 );
		DrawString( header_text1 );
		GetPen( &pen );
		pen.h += 5;
		header_text2_pos = App_window->portRect.right - 15 -
				StringWidth( header_text2 );
		MoveTo( header_text2_pos < pen.h ? pen.h : header_text2_pos, CELL_HEIGHT - 2 );
		DrawString( header_text2 );

	}

	Set_fore_color( BLACK_COLOR );

	l_release( h );
}


#define CHECK_LEFT		(BOX_FIELD_LEFT + 2)
#define CHECK_BOTTOM	(hi->row.top + CELL_HEIGHT - 5)

static void display_updating( Heap_info_t *hi, column_t *col, Boolean force )
{
	if ( force ) {
		MoveTo( col->left, CHECK_BOTTOM - 2 );
		LineTo( col->left + 2, CHECK_BOTTOM  );
		LineTo( col->left + 7, CHECK_BOTTOM - 5 );
	}
}


static void display_name( Heap_info_t *hi, column_t *col, Boolean force )
{
	unsigned char name[32];
	int16 len, length_pixel_limit;
	unsigned char *p;
	Rect field;

	if ( force ) {
		field.left = col->left;
		field.right = col->right;
		field.top = hi->row.top;
		field.bottom = hi->row.bottom;
	
		length_pixel_limit = col->right - col->left - 5;
		pstrcopy( hi->appname, name );
		if ( (len = StringWidth(name)) > length_pixel_limit ) {
			p = name + *name;
			do {
				len -= CharWidth( *p );
				--p;
				--*name;
			} while ( len > length_pixel_limit );
			*p = 'É';
			len += CharWidth( 'É' );
		}
		EraseRect( &field );
	
		MoveTo( field.left + 1, hi->row.top + NAME_TOP_DELTA + TEXT_HEIGHT - 2 );
		Set_fore_color( NAME_COLOR );
		DrawString( name );
		Set_fore_color( BLACK_COLOR );
	}
}


static void display_current_size( Heap_info_t *hi, column_t *col, Boolean force )
{
	unsigned char num[20];
	Rect field;
	int left;

	if ( hi->current_size != hi->last_size || force ) {

		hi->last_size = hi->current_size;

		field.left = col->left;
		field.right = col->right;
		field.top = hi->row.top + SIZE_TOP_DELTA;
		field.bottom = field.top + TEXT_HEIGHT;
	
		pnumcopy( hi->current_size, num );
		add_commas( num );

		EraseRect( &field );

		field.left = field.right - StringWidth( (StringPtr)num );

		MoveTo( field.left, field.bottom - 2 );
		Set_fore_color( SIZE_COLOR );
		DrawString( num );
		Set_fore_color( BLACK_COLOR );

	}
}


static void display_current_free( Heap_info_t *hi, column_t *col, Boolean force )
{
	unsigned char num[20];
	Rect field;

	if ( hi->current_free != hi->last_free || force ) {

		hi->last_free = hi->current_free;

		field.left = col->left;
		field.right = col->right;
		field.top = hi->row.top + FREE_TOP_DELTA;
		field.bottom = field.top + TEXT_HEIGHT;

		pnumcopy( hi->current_free, num );
		add_commas( num );

		EraseRect( &field );

		field.left = field.right - StringWidth( (StringPtr)num );

		MoveTo( field.left, field.bottom - 2 );
		Set_fore_color( FREE_COLOR );
		DrawString( num );
		Set_fore_color( BLACK_COLOR );

	}
}


static void display_current_islands( Heap_info_t *hi, column_t *col, Boolean force )
{
	unsigned char num[20];
	Rect field;

	if ( hi->current_islands != hi->last_islands || force ) {

		hi->last_islands = hi->current_islands;

		field.left = col->left;
		field.right = col->right;
		field.top = hi->row.top + ISLANDS_TOP_DELTA;
		field.bottom = field.top + TEXT_HEIGHT;
	
		pnumcopy( hi->current_islands, num );
		add_commas( num );

		EraseRect( &field );

		field.left = field.right - StringWidth( (StringPtr)num );

		MoveTo( field.left, field.bottom - 2 );
		Set_fore_color( HEADER_COLOR );
		DrawString( num );
		Set_fore_color( BLACK_COLOR );

	}
}


static void display_heap( Heap_info_t *hi, column_t *col, Boolean force )
{
	int32 *p, *q;
	int16 i, right_pixel_limit;
	int32 right, offset;
	unsigned char run;
	Boolean update;
	Rect field;

	field.left = col->left - 1;
	field.right = App_window->portRect.right - 15;
	field.top = hi->row.top + 2;
	field.bottom = hi->row.bottom - 2;

	right_pixel_limit = field.right;


	update = force;

	if ( hi->current_heap_ok != hi->last_heap_ok ) {
		hi->last_heap_ok = hi->current_heap_ok;
		update = true;
	}

	if ( hi->current_heap_transitions != hi->last_heap_transitions ) {
		hi->last_heap_transitions = hi->current_heap_transitions;
		update = true;
	}

	for ( i = hi->current_heap_transitions, p = hi->current_heap, q = hi->last_heap; i; --i ) {
		if ( !update && *p != *q )
			update = true;
		*q++ = *p++;
	}

	hi->current_heap_offset_pixels = Content_heap_offset_pixels();
	if ( hi->current_heap_offset_pixels != hi->last_heap_offset_pixels ) {
		hi->last_heap_offset_pixels = hi->current_heap_offset_pixels;
		update = true;
	}

	if ( !update ) return;


	if ( !hi->current_heap_ok ) {
		EraseRect( &field );
		MoveTo( col->left, field.bottom - 2 );
		TextFace( bold );
		DrawString( pstr(Heap_damaged_STR_x) );
		TextFace( 0 );
		return;
	}

	offset = 0;
	i = hi->current_heap_transitions;
	p = hi->current_heap;
	do {
		offset += *p++ & 0x00FFFFFF;
	} while ( offset < hi->current_heap_offset_pixels && --i );
	if ( !i ) {
		EraseRect( &field );
		return;
	}

	Set_fore_color_or_pattern( ( !hi->current_heap_offset_pixels ?
			HEAP_BORDER_COLOR : BACK_COLOR ) );
	MoveTo( field.left, field.top );
	Line( 0, CELL_HEIGHT - 5 );

	InsetRect( &field, 1, 1 );
	field.left = col->left;
	offset -= hi->current_heap_offset_pixels;
	run = *(unsigned char *) (p - 1);
	for ( ;; ) {

		switch ( run ) {
		case HEAP_FREE_RUN:
			Set_fore_color_or_pattern( HEAP_FREE_COLOR );
			break;
		case HEAP_LOCKED_RUN:
			Set_fore_color_or_pattern( HEAP_LOCKED_COLOR );
			break;
		case HEAP_UNLOCKED_RUN:
			Set_fore_color_or_pattern( HEAP_UNLOCKED_COLOR );
			break;
		case HEAP_PURGEABLE_RUN:
			Set_fore_color_or_pattern( HEAP_PURGEABLE_COLOR );
			break;
		default:
			Set_fore_color_or_pattern( BLACK_COLOR );
			break;
		}

		if ( offset < 16384 ) {
			field.right = field.left + offset;
			if ( field.right > right_pixel_limit ) field.right = right_pixel_limit;
		}
		else field.right = right_pixel_limit;

		PaintRect( &field );
		field.left = field.right;

		if ( field.left >= right_pixel_limit || !--i ) break;

		run = *(unsigned char *) p;
		offset = *p++ & 0x00FFFFFF;
	}

	InsetRect( &field, 0, -1 );
	if ( field.right < right_pixel_limit ) {
		Set_fore_color_or_pattern( HEAP_BORDER_COLOR );
		MoveTo( field.right, field.top );
		Line( 0, CELL_HEIGHT - 5 );
	}
	Set_fore_color_or_pattern( BLACK_COLOR );

	MoveTo( col->left, field.top );
	LineTo( field.right, field.top );
	MoveTo( col->left, field.bottom - 1 );
	LineTo( field.right, field.bottom - 1 );

	++field.left;
	if ( hi->last_heap_x != field.left || force ) {
		if ( hi->last_heap_x > field.left || force ) {
			field.right = right_pixel_limit;
			if ( field.left < field.right )
				EraseRect( &field );
		}
		hi->last_heap_x = field.left;
	}
}


static void display_selection( Heap_info_t *hi, column_t *col, Boolean force )
{
	Boolean update_heap;

	if ( !col ) col = Display_find_column( hi, colHeap );

	update_heap = false;

	if ( hi->has_selection != hi->last_has_selection ) {
		hi->last_has_selection = hi->has_selection;
		update_heap = true;
	}

	if ( (hi->current_selection.physical_start != hi->last_selection.physical_start) ||
		 (hi->current_selection.physical_end   != hi->last_selection.physical_end) ) {
		hi->last_selection = hi->current_selection;
		update_heap = true;
	}

	if ( update_heap ) display_heap( hi, col, true );


	if ( hi->has_selection ) {
		int32 pixel_left, pixel_right, right_pixel_limit;
		int16 t, b, phase_offset_top, phase_offset_bottom;
		PenState save_pen;
		Boolean left_off, right_off;

		pixel_left = Content_address_to_pixel( hi, hi->current_selection.physical_start );
		pixel_right = Content_address_to_pixel( hi, hi->current_selection.physical_end );
		right_pixel_limit = App_window->portRect.right - 15;

		if ( !((pixel_left > right_pixel_limit) || (pixel_right < col->left)) ) {

			left_off = pixel_left < col->left;
			right_off = pixel_right > right_pixel_limit;

			t = hi->row.top + 3;
			b = hi->row.bottom - 4;

			GetPenState( &save_pen );
			PenMode( patCopy );
			if ( Swatch_in_foreground() ) {
				phase_offset_top = hi->selection_phase * 8 + 2;
				phase_offset_bottom = (7 - hi->selection_phase) * 8 + 2;
			}
			else phase_offset_top = phase_offset_bottom = 8 * 8 + 2;

			if ( !left_off ) {
				PenPat( (ConstPatternParam) ((Ptr)(*marching_ants) + phase_offset_bottom) );
				MoveTo( pixel_left, t );
				LineTo( pixel_left, b );
			}
			else pixel_left = col->left;

			if ( !right_off ) {
				PenPat( (ConstPatternParam) ((Ptr)(*marching_ants) + phase_offset_top) );
				MoveTo( pixel_right, t );
				LineTo( pixel_right, b );
			}
			else pixel_right = right_pixel_limit;

			PenPat( (ConstPatternParam) ((Ptr)(*marching_ants) + phase_offset_top) );
			MoveTo( pixel_left, t );
			LineTo( pixel_right, t );
			PenPat( (ConstPatternParam) ((Ptr)(*marching_ants) + phase_offset_bottom) );
			MoveTo( pixel_left, b );
			LineTo( pixel_right, b );

			SetPenState( &save_pen );
		}
	}
}


column_t *Display_find_column( Heap_info_t *hi, int16 cell_type )
{
	int j;
	column_t *col;

	for ( j = hi->row.max_columns, col = &hi->row.column[j]; j >= 0; --j, --col )
		if ( col->cell->type == cell_type ) return col;

	assert( false, "\p Display_find_column(): couldn't find column" );
}


static void add_commas( unsigned char *num )
{
	int16 num_commas, len;
	unsigned char *p;

	if ( ( len = *(unsigned char *) num ) < 4 )
		return;

	num_commas = (len - 1) / 3;
	*(unsigned char *) num += num_commas;
	p = num + len - 2;
	for ( ; num_commas; --num_commas, p-= 3 ) {
		BlockMove( p, p + num_commas, 3 );
		*(p + num_commas - 1) = ',';
	}
}


void Check_color_usage( void )
{
	static Boolean checked_once = false;
	Rect window_rect, t_rect;
	int16 min_depth, t_depth, t_index;
	GDHandle gdevice;
	PixMapHandle pmap;
	CTabHandle clut;
	Boolean use_gray;

	if ( !This_mac.hasColorQD || color_is_dead )
		Use_color = false;
	else {
		window_rect = App_window->portRect;
		LocalToGlobal( (Point *) &window_rect );
		LocalToGlobal( (Point *) &window_rect.bottom );
		for ( min_depth = 32, use_gray = false, gdevice = GetDeviceList(); gdevice;
				gdevice = GetNextDevice( gdevice ) ) {
			if ( TestDeviceAttribute( gdevice, screenDevice ) &&
					TestDeviceAttribute( gdevice, screenActive ) ) {
				pmap = (**gdevice).gdPMap;
				if ( SectRect( &(**pmap).bounds, &window_rect, &t_rect ) ) {
					t_depth = (**pmap).pixelSize;
					if ( t_depth <= 8 && !TestDeviceAttribute( gdevice, gdDevType ) )
						use_gray = true;
					if ( t_depth < min_depth )
						min_depth = t_depth;
				}
			}
		}


		if ( ( Use_color = (min_depth >= MIN_DEPTH_FOR_COLOR) ) ) {
/**
	clut indexes are:
		0	8-bit color
		1	4-bit color
		2	8-bit gray
		3	4-bit gray
 **/
			if ( use_gray )
				t_index = 2;
			else
				t_index = 0;
			if ( min_depth == 4 /* 4-bit color */)
				++t_index;
		}
		else
			t_index = -1;

		if ( checked_once || t_index != clut_index ) {
			if ( !checked_once ) {
				InvalRect( &App_window->portRect );
				if ( current_clut )
					HPurge( (Handle) current_clut );
			}
			clut_index = t_index;
			if ( t_index == -1 )
				current_clut = NULL;
			else {
				current_clut = cluts[t_index];
				if ( !current_clut ) {
					current_clut = cluts[t_index] =
							(CTabHandle) GetResource( 'clut', COLOR8_clut + t_index );
					LoadResource( (Handle) current_clut );	/* make sure we've got it */
					if ( !current_clut || !*current_clut ) {
						color_is_dead = true;
						Use_color = false;
					}
					else
						HNoPurge( (Handle) current_clut );
				}
			}
		}
	}
	checked_once = true;
}


void Set_fore_color_or_pattern( int16 color )
{
	if ( Use_color ) {
		HLock( (Handle) current_clut );
		RGBForeColor( &(**current_clut).ctTable[color].rgb );
		HUnlock( (Handle) current_clut );
	}
	else
		PenPat( (ConstPatternParam) ( (Ptr) (*patterns) + color * 8 + 2 ) );
}


void Set_fore_color( int16 color )
{
	if ( Use_color )
		Set_fore_color_or_pattern( color );
}


void Set_back_color( int16 color )
{
	if ( Use_color ) {
		HLock( (Handle) current_clut );
		RGBBackColor( &(**current_clut).ctTable[color].rgb );
		HUnlock( (Handle) current_clut );
	}
}


unsigned char *MBox_text( void )
{
	return mbox_text;
}


static void draw_mbox( Rect *br )
{
	BitMap bm;
	Handle h;
	Point pen;
	Rect r = *br;

	if ( Debugger_installed ) {
		bm.bounds.right = (bm.bounds.left = 3) + 16;
		bm.bounds.top = (bm.bounds.bottom = r.bottom) - 12;
		bm.rowBytes = 2;
		h = GetResource( 'SICN', MACSBUG_SICN );
		HLock( h );
		bm.baseAddr = *h + 8;
		CopyBits( &bm, &App_window->portBits, &bm.bounds, &bm.bounds,
				srcCopy, NULL );
		HUnlock( h );
	}

	MoveTo( 26, r.bottom - 2 );
	//TextMode( srcCopy );
	//TextFont( monaco );
	DrawString( mbox_text );
	//TextMode( srcOr );
	//TextFont( geneva );
	++r.top;
	GetPen( &pen );
	r.left = pen.h;
	EraseRect( &r );
}

/**

	swatch.c
	Copyright (c) 1990,1991, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include <GestaltEqu.h>
#include <Processes.h>
#include <pstring.h>

#include "swatch.resources.h"
#include "swatch.types.h"
#include "swatch.prefs.h"
#include "swatch.heap.h"
#include "swatch.about.h"
#include "swatch.h"


/**-----------------------------------------------------------------------------
 **
 ** Private Macros
 **
 **/

#define TRAP_MFTempMemory		0xA88F
#define TRAP_Unimplemented		0xA89F

#define MacJmp					0x120


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

#define JUST_LOCK_MEMORY_MASK			0
#define RESIZE_PROCESS_LIST_MASK		1
#define RESIZE_DISPLAY_LIST_MASK		2

#define CHECK_ALL_SIZES_START			-1
#define CHECK_ALL_SIZES_END				0

#define FORCE_UPDATE_NAME_MASK			1
#define FORCE_UPDATE_SIZE_MASK			2
#define FORCE_UPDATE_FREE_MASK			4
#define FORCE_UPDATE_HEAP_MASK			8
#define FORCE_UPDATE_HEADER_MASK		16
#define FORCE_UPDATE_HEAP_SCALE_MASK	32
#define FORCE_UPDATE_BOTTOM_MASK		64

#define NO_UPDATE						0
#define FORCE_UPDATE_FIELDS				15
#define FORCE_UPDATE_FIELDS_AND_BORDER	63
#define FORCE_UPDATE_EVERYTHING_MASK	127

#define HORIZ_ARROW_DELTA				8

#define MBOX_HEAP_ADDRESS				1
#define MBOX_ADDRESS					2

#define MBOX_ERASE						1
#define MBOX_FRAME_AND_CONTENTS			2
#define MBOX_CONTENTS					3

#define MIN_DISPLAY_LIST_SIZE			4


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

char system_heap_name[] = "\pSystem";
char multifinder_name[] = "\pMultiFinder";
char swatch_name[] = "\pSystem Watch";

Swatch_globals_t *Globals_P;
Display_stat_t **display_list_h, *display_list;
int16 display_list_elems;
ProcessSerialNumber **process_list_h;
int16 process_list_elems;
Boolean too_many_apps;

WindowPtr App_window;
ControlHandle horiz_scroll, vert_scroll;
int16 num_display_lines;
Boolean partial_display_line;

int32 display_heap_bytes, horiz_scroll_scale;
int32 heap_offset_pixels, heap_right_pixels;
int32 max_heap_size = 0;
int16 heap_cursor;

Boolean in_foreground;
Boolean Process_heaps_are_32_bit;
MenuHandle Apple_menu, File_menu, Edit_menu;

EventRecord the_event;

int16 mbox_type = 0;
char mbox_text[256] = {0};
int32 mbox_value1;
int32 mbox_value2;

CTabHandle cluts[4];
CTabHandle current_clut;
int16 clut_index;
Handle patterns;	/* PAT# resource */
Boolean color_is_dead;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

int main( void );

OSErr resize_lists( int16 resize_mask );
Boolean process_list_changed( void );
void update_display_list( int16 update_mask );
void check_sizes( int16 update_mask, int16 line_start, int16 line_end );

void display_header( int16 update_mask );
void display_heap( Display_stat_t *d, Rect *display_rect, int16 update_mask );
void display_appname( Display_stat_t *d, Rect *display_rect, int16 update_mask );
void display_current_size( Display_stat_t *d, Rect *display_rect, int16 update_mask );
void display_current_free( Display_stat_t *d, Rect *display_rect, int16 update_mask );
void add_commas( register char *num );

void draw_grow_icon( WindowPtr the_window, Boolean active_state, Boolean rgn_state );
void activate_window( WindowPtr the_window, Boolean active_state );
Boolean handle_menu_select( uns32 menu_long );
void check_color_usage( Boolean first_thru );

void mouse_down_in_names( void );
void mouse_down_in_heap( void );
void set_cursor( void );

void clip_with_controls( void );
void clip_without_controls( void );
void size_controls( void );
void reset_vert_scroll( void );
void reset_horiz_scroll( void );
void draw_mbox( int16 what );

pascal void vert_scroll_proc( ControlHandle the_control, int16 part );
pascal void horiz_scroll_proc( ControlHandle the_control, int16 part );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

SysEnvRec This_mac;
Boolean Pre_system_7;
Boolean Use_color;
Boolean Debugger_installed;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

/*******************************************************************************
 ***
 *** prototype
 ***
 *** summary
 ***
 *** History:
 ***
 *** To Do:
 ***
 ***/

int main( void )
{
	register int16 i;
	register boolean done;
	ParamBlockRec open_PB, control_PB;
	Rect bounds;
	int16 wne_in_foreground, wne_in_background;
	int16 **wne;

	MaxApplZone();
	InitGraf( &thePort );
	InitFonts();
	InitWindows();
	InitDialogs( NULL );
	TEInit();
	InitMenus();
	InitCursor();
	MoreMasters();

	SysEnvirons( 1, &This_mac );
	if ( This_mac.systemVersion < 0x0603 ) {
		StopAlert( BAD_SYSTEM_VERSION_ALRT, NULL );
		return;
	}

	Debugger_installed = *(uns32 *) MacJmp != 0;

	for ( i = 5; i; --i )
		EventAvail( 0, &the_event );
	in_foreground = true;

	if ( ( Pre_system_7 = This_mac.systemVersion < 0x0700 ) ) {
		open_PB.ioParam.ioNamePtr = (StringPtr) "\p._swatch";
		open_PB.ioParam.ioPermssn = 0;
		if ( PBOpen( &open_PB, false ) ) {
			StopAlert( NO_INIT_ALRT, NULL );
			return;
		}
	
		control_PB.cntrlParam.ioCRefNum = open_PB.ioParam.ioRefNum;
		control_PB.cntrlParam.csCode = csGetGlobals;
		if ( PBControl( &control_PB, false ) ) {
			StopAlert( INIT_PROBLEMS_ALRT, NULL );
			return;
		}
		Globals_P = (Swatch_globals_t *) control_PB.cntrlParam.ioNamePtr;
	
		if ( Globals_P->version != VERSION ) {
			StopAlert( INIT_INCOMPATIBLE_ALRT, NULL );
			return;
		}

		if ( NGetTrapAddress( TRAP_MFTempMemory, ToolTrap ) ==
				NGetTrapAddress( TRAP_Unimplemented, ToolTrap ) ) {
			StopAlert( NOT_MULTIFINDER_ALRT, NULL );
			return;
		}
	}

	process_list_h = (ProcessSerialNumber **) NewHandle( 0 );
	process_list_elems = 0;
	display_list_h = (Display_stat_t **) NewHandle( sizeof( Display_stat_t ) );
	display_list_elems = 1;
	if ( !process_list_h || !display_list_h ) {
		StopAlert( OUT_OF_MEMORY_ALRT, NULL );
		return;
	}
	resize_lists( JUST_LOCK_MEMORY_MASK );
	pstrcopy( system_heap_name, display_list[0].app.appname );
	display_list[0].app.zone = *(THz *) SysZone;
	display_list[0].heap_is_32_bit = false;
	Process_heaps_are_32_bit = false;
	if ( !Pre_system_7 ) {
		int32 response;

		if ( !Gestalt( gestaltAddressingModeAttr, &response ) ) {
			display_list[0].heap_is_32_bit = (response & (1 << gestalt32BitSysZone)) != 0;
			Process_heaps_are_32_bit = (response & (1 << gestalt32BitAddressing)) != 0;
		}
	}
	display_list[0].last_size = display_list[0].last_free = 0;
	display_list[0].last_heap_ok = true;
	display_list[0].last_heap_transitions = 0;

	patterns = GetResource( 'PAT#', BLACK_WHITE_PAT_ );
	if ( !patterns ) {
		StopAlert( OUT_OF_MEMORY_ALRT, NULL );
		return;
	}

	Read_prefs();
	if ( !( wne = (int16 **) GetResource( 'WNE ', 1000 ) ) ) {
		StopAlert( WNE_DAMAGED_ALRT, NULL );
		return;
	}
	wne_in_foreground = (*wne)[0];
	wne_in_background = (*wne)[1];

	if ( This_mac.hasColorQD )
		App_window = NewCWindow( NULL, &Swatch_prefs.window_rect, (StringPtr) swatch_name, true,
				documentProc, (WindowPtr) -1, true, 0 );
	else
		App_window = NewWindow( NULL, &Swatch_prefs.window_rect, (StringPtr) swatch_name, true,
				documentProc, (WindowPtr) -1, true, 0 );

	SetPort( App_window );
	clip_with_controls();
	SetRect( &bounds, 0, 0, 0, 0 );
	vert_scroll = NewControl( App_window, &bounds, NULL, true, 0, 0, 0, scrollBarProc, 0 );
	horiz_scroll = NewControl( App_window, &bounds, NULL, true, 0, 0, 0, scrollBarProc, 0 );
	size_controls();

	TextFont( geneva );
	TextSize( 9 );
	check_color_usage( true );

	Apple_menu = GetMenu( APPLE_MENU );
	AddResMenu( (Apple_menu = GetMenu( APPLE_MENU )), 'DRVR' );
	InsertMenu( Apple_menu, 0 );
	InsertMenu( (File_menu = GetMenu( FILE_MENU )), 0 );
	InsertMenu( (Edit_menu = GetMenu( EDIT_MENU )), 0 );
	DrawMenuBar();

	if ( process_list_changed() )
		update_display_list( NO_UPDATE );

	for ( done = false; !done; ) {
		WaitNextEvent( everyEvent, &the_event, ( in_foreground ? wne_in_foreground :
				wne_in_background ), NULL );
		SetPort( App_window );
		if ( FrontWindow() == App_window ) {
			DisableItem( Edit_menu, 1 );
			DisableItem( Edit_menu, 3 );
			if ( mbox_text[0] )
				EnableItem( Edit_menu, 4 );
			else
				DisableItem( Edit_menu, 4 );
			DisableItem( Edit_menu, 5 );
			DisableItem( Edit_menu, 6 );
		}
		else {
			EnableItem( Edit_menu, 1 );
			EnableItem( Edit_menu, 3 );
			EnableItem( Edit_menu, 4 );
			EnableItem( Edit_menu, 5 );
			EnableItem( Edit_menu, 6 );
		}
		check_color_usage( false );

		switch ( the_event.what ) {
		case nullEvent:
			GlobalToLocal( &the_event.where );
			set_cursor();
			if ( process_list_changed() )
				update_display_list( FORCE_UPDATE_EVERYTHING_MASK );
			else
				check_sizes( NO_UPDATE, CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
			break;

		case mouseDown:
			i = FindWindow( the_event.where, (WindowPtr *) &the_event.message );
			if ( i != inContent )
				InitCursor();
			if ( i == inMenuBar )
				done = handle_menu_select( MenuSelect( the_event.where ) );
			else if ( (WindowPtr) the_event.message == App_window ) {
				switch ( i ) {
				case inDrag:
					DragWindow( App_window, the_event.where, &(***(RgnHandle *)GrayRgn).rgnBBox );
					break;

				case inContent: {
					ControlHandle the_control;
					register int16 i, j;

					GlobalToLocal( &the_event.where );
					set_cursor();
					i = FindControl( the_event.where, App_window, &the_control );
					if ( the_control == vert_scroll ) {
						if ( i == inThumb ) {
							i = GetCtlValue( the_control );
							TrackControl( the_control, the_event.where, NULL );
							if ( GetCtlValue( the_control ) != i )
								check_sizes( FORCE_UPDATE_FIELDS | FORCE_UPDATE_BOTTOM_MASK,
										CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
						}
						else
							TrackControl( the_control, the_event.where,
									(ProcPtr) vert_scroll_proc );
					}
					else if ( the_control == horiz_scroll ) {
						if ( i == inThumb ) {
							i = GetCtlValue( the_control );
							TrackControl( the_control, the_event.where, NULL );
							if ( ( j = GetCtlValue( the_control ) ) != i ) {
								heap_offset_pixels = (int32) j * horiz_scroll_scale;
								check_sizes( FORCE_UPDATE_HEAP_MASK, CHECK_ALL_SIZES_START,
										CHECK_ALL_SIZES_END );
							}
						}
						else
							TrackControl( the_control, the_event.where,
									(ProcPtr) horiz_scroll_proc );
					}
					else if ( the_event.where.h <= FREE_FIELD_RIGHT )
						mouse_down_in_names();
					else if ( the_event.where.h >= HEAP_FIELD_LEFT )
						mouse_down_in_heap();
					break;
				}

				case inGrow: {
					register uns32 new_size;
					Rect size_rect;

					SetRect( &size_rect, HEAP_FIELD_LEFT + 64, CELL_HEIGHT + 64,
							32767, 32767 );
					new_size = GrowWindow( App_window, the_event.where, &size_rect );
					if ( new_size ) {
						draw_grow_icon( App_window, false, false );
						HideControl( horiz_scroll );
						HideControl( vert_scroll );
						draw_mbox( MBOX_ERASE );
						SizeWindow( App_window, (int16) new_size, new_size >> 16, true );
						clip_with_controls();
						size_controls();
						draw_mbox( MBOX_FRAME_AND_CONTENTS );
						draw_grow_icon( App_window, true, true );
					}
					break;
				}

				case inGoAway:
					if ( TrackGoAway( App_window, the_event.where ) )
						done = true;
					break;

				default:
					break;
				}
			}
			break;

		case keyDown:
		case autoKey: {
			char key;

			key = the_event.message & charCodeMask;
			if ( the_event.modifiers & cmdKey )
				done = handle_menu_select( MenuKey( key ) );
			break;
		}

		case updateEvt:
			if ( (WindowPtr) the_event.message == App_window ) {
				BeginUpdate( App_window );
				UpdtControl( App_window, App_window->visRgn );
				draw_mbox( MBOX_FRAME_AND_CONTENTS );
				draw_grow_icon( App_window, in_foreground, true );
				check_sizes( FORCE_UPDATE_FIELDS_AND_BORDER, CHECK_ALL_SIZES_START,
						CHECK_ALL_SIZES_END );
				EndUpdate( App_window );
			}
			break;

		case activateEvt:
			if ( (WindowPtr) the_event.message == App_window )
				activate_window( App_window, the_event.modifiers & activeFlag );
			break;

		case app4Evt:
			if ( *(unsigned char *) &the_event.message == 1 /* suspend/resume */ ) {
				if ( the_event.message & 1 /* resume */ )
					in_foreground = true;
				else
					in_foreground = false;
				HiliteWindow( App_window, in_foreground );
				activate_window( App_window, in_foreground );
			}
			break;

		default:
			break;
		}
	}

	Swatch_prefs.window_rect = App_window->portRect;
	LocalToGlobal( (Point *)&Swatch_prefs.window_rect );
	LocalToGlobal( (Point *)&Swatch_prefs.window_rect.bottom );
	Write_prefs();

	CloseWindow( App_window );
}


OSErr resize_lists( int16 resize_mask )
{
	OSErr err;

	err = noErr;
	HUnlock( (Handle) process_list_h );
	HUnlock( (Handle) display_list_h );
	if ( resize_mask & RESIZE_PROCESS_LIST_MASK ) {
		SetHandleSize( (Handle) process_list_h,
				process_list_elems * sizeof( ProcessSerialNumber ) );
		if ( (err = MemError()) )
			process_list_elems = 0;
	}
	if ( (resize_mask & RESIZE_DISPLAY_LIST_MASK) && !err ) {
		SetHandleSize( (Handle) display_list_h,
				display_list_elems * sizeof( Display_stat_t ) );
		if ( (err = MemError()) )
			display_list_elems = 1;
	}
	MoveHHi( (Handle) process_list_h );
	HLock( (Handle) process_list_h );
	MoveHHi( (Handle) display_list_h );
	HLock( (Handle) display_list_h );
	display_list = (Display_stat_t *) StripAddress( *display_list_h );
	too_many_apps = err != noErr;
	return err;
}


Boolean process_list_changed( void )
{
	register Boolean need_major_update;
	Boolean same;
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	register int16 old_process_list_elems, i;

	if ( Pre_system_7 ) {
		need_major_update = Globals_P->apps_need_updating;
		Globals_P->apps_need_updating = false;
		return ( need_major_update );
	}

	need_major_update = false;
	old_process_list_elems = process_list_elems;
	process_list_elems = 0;
	for ( psn.highLongOfPSN = psn.lowLongOfPSN = 0; !GetNextProcess( &psn );
			++process_list_elems );

	if ( old_process_list_elems != process_list_elems ) {
		need_major_update = true;
		if ( resize_lists( RESIZE_PROCESS_LIST_MASK ) )
			return true;
	}

	for ( psn.highLongOfPSN = psn.lowLongOfPSN = 0, i = 0; i < process_list_elems; ++i ) {
		if ( GetNextProcess( &psn ) ) {
			process_list_elems = 0;
			return true;
		}
		if ( !need_major_update ) {
			SameProcess( &psn, &(*process_list_h)[i], &same );
			if ( !same )
				need_major_update = true;
		}
		if ( need_major_update )
			(*process_list_h)[i] = psn;
	}
	return need_major_update;
}


void update_display_list( int16 update_mask )
{
	register App_stat_t *a;
	register Display_stat_t *d;
	register int16 i, old_display_list_elems;
	ProcessInfoRec info;
	register OSErr err;

	old_display_list_elems = display_list_elems;
	display_list_elems = 1;
	if ( Pre_system_7 ) {
		for ( a = Globals_P->apps, i = Globals_P->max_apps; i; --i, ++a ) {
			if ( !a->free && !pstrequal( a->appname, multifinder_name ) )
				++display_list_elems;
		}
		if ( !resize_lists( RESIZE_DISPLAY_LIST_MASK ) ) {
			d = &display_list[1];
			for ( a = Globals_P->apps, i = display_list_elems - 1; i; ++a ) {
				if ( !a->free && !pstrequal( a->appname, multifinder_name ) ) {
					d->app = *a;
					d->heap_is_32_bit = false;
					d->last_size = d->last_free = 0;
					d->last_heap_ok = true;
					d->last_heap_transitions = 0;
					++d;
					--i;
				}
			}
		}
	}

	/*** System 7 Process Manager stuff ***/

	else {
		display_list_elems += process_list_elems;
		if ( !resize_lists( RESIZE_DISPLAY_LIST_MASK ) ) {
			d = &display_list[1];
			for ( display_list_elems = 1, i = 0; i < process_list_elems; ++i ) {
				info.processInfoLength = sizeof( ProcessInfoRec );
				info.processName = (StringPtr) d->app.appname;
				info.processAppSpec = NULL;
				if ( !GetProcessInformation( &(*process_list_h)[i], &info ) ) {
					d->app.zone = (THz) info.processLocation;
					d->heap_is_32_bit = Process_heaps_are_32_bit;
					d->last_size = d->last_free = 0;
					d->last_heap_ok = true;
					d->last_heap_transitions = 0;
					++d;
					++display_list_elems;
				}
			}
		}
	}

	if ( old_display_list_elems != display_list_elems && display_list_elems > num_display_lines )
		SetCtlMax( vert_scroll, display_list_elems - num_display_lines );
	if ( update_mask )
		check_sizes( update_mask, CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
}


void check_sizes( int16 update_mask, int16 line_start, int16 line_end )
{
	register Display_stat_t *d;
	register int16 i;
	register int32 temp_max_heap_size;
	Rect display_rect;

	clip_without_controls();
	if ( line_start == -1 ) {
		display_header( update_mask );
		line_start = 0;
		line_end = num_display_lines - 1;
		if ( partial_display_line )
			++line_end;
	}

	display_rect.top = CELL_HEIGHT + CELL_HEIGHT * line_start;
	display_rect.bottom = display_rect.top + CELL_HEIGHT;

	line_start += GetCtlValue( vert_scroll );
	line_end += GetCtlValue( vert_scroll );

	temp_max_heap_size = 0;
	for ( d = display_list, i = 0; i < display_list_elems; ++i, ++d ) {
		d->current_size = (int32) d->app.zone->bkLim -
				(int32) &d->app.zone->bkLim - 52 /* MF bytes */;
		if ( d->current_size > temp_max_heap_size )
			temp_max_heap_size = d->current_size;

		if ( i < line_start || i > line_end )
			continue;

		display_rect.left = NAME_FIELD_LEFT;
		display_rect.right = HEAP_FIELD_RIGHT;

		d->current_free = d->app.zone->zcbFree;

		if ( update_mask & FORCE_UPDATE_NAME_MASK ) {
			display_rect.left = NAME_FIELD_LEFT;
			display_rect.right = NAME_FIELD_RIGHT;
			display_appname( d, &display_rect, update_mask );
		}

		if ( (update_mask & FORCE_UPDATE_SIZE_MASK) || d->current_size != d->last_size ) {
			display_rect.left = SIZE_FIELD_LEFT;
			display_rect.right = SIZE_FIELD_RIGHT;
			display_current_size( d, &display_rect, update_mask );
			d->last_size = d->current_size;
		}

		if ( (update_mask & FORCE_UPDATE_FREE_MASK) || d->current_free != d->last_free ) {
			display_rect.left = FREE_FIELD_LEFT;
			display_rect.right = FREE_FIELD_RIGHT;
			display_current_free( d, &display_rect, update_mask );
			d->last_free = d->current_free;
		}

		display_rect.left = HEAP_FIELD_LEFT;
		display_rect.right = App_window->portRect.right - 15;
		make_current_heap( d );
		display_heap( d, &display_rect, update_mask );

		display_rect.top += CELL_HEIGHT;
		display_rect.bottom += CELL_HEIGHT;
	}

	if ( update_mask & FORCE_UPDATE_BOTTOM_MASK ) {
		display_rect.bottom = App_window->portRect.bottom;
		if ( display_rect.bottom > display_rect.top ) {
			display_rect.left = NAME_FIELD_LEFT;
			display_rect.right = FREE_FIELD_RIGHT;
			EraseRect( &display_rect );
			display_rect.left = HEAP_FIELD_LEFT;
			display_rect.right = HEAP_FIELD_RIGHT;
			EraseRect( &display_rect );
		}
	}
	clip_with_controls();
	if ( temp_max_heap_size != max_heap_size ) {
		max_heap_size = temp_max_heap_size;
		reset_horiz_scroll();
	}
}


void display_header( int16 update_mask )
{
	static char size_title[] = "\pHeap Size";
	static char free_title[] = "\pFree";
	static char heap_title1[] = "\p1 pixel : ";
	static char heap_title2[] = "\p bytes";

	if ( update_mask & (FORCE_UPDATE_HEADER_MASK | FORCE_UPDATE_HEAP_SCALE_MASK) ) {
		char num[20];
		Rect erase;

		MoveTo( 0, CELL_HEIGHT - 1 );
		set_fore_color_or_pattern( HEADER_BORDER_COLOR );
		Line( 16384, 0 );
		set_fore_color_or_pattern( BLACK_COLOR );
	
		erase.left = HEAP_FIELD_LEFT + 10 + StringWidth( (StringPtr) heap_title1 );
		erase.right = HEAP_FIELD_RIGHT;
		erase.top = 0;
		erase.bottom = CELL_HEIGHT - 1;
		pnumcopy( Swatch_prefs.heap_scale, num );
		add_commas( num );
		EraseRect( &erase );
		MoveTo( HEAP_FIELD_LEFT + 10, CELL_HEIGHT - 2 );
		set_fore_color( HEADER_COLOR );
		DrawString( (StringPtr) heap_title1 );
		DrawString( (StringPtr) num );
		DrawString( (StringPtr) heap_title2 );
		set_fore_color( BLACK_COLOR );
	}

	if ( update_mask & FORCE_UPDATE_HEADER_MASK ) {
		set_fore_color( HEADER_COLOR );
		MoveTo( SIZE_FIELD_RIGHT - StringWidth( (StringPtr) size_title ), CELL_HEIGHT - 2 );
		DrawString( (StringPtr) size_title );

		set_fore_color( FREE_COLOR );
		MoveTo( FREE_FIELD_RIGHT - StringWidth( (StringPtr) free_title ), CELL_HEIGHT - 2 );
		DrawString( (StringPtr) free_title );
		set_fore_color( BLACK_COLOR );
	}
}


void display_appname( register Display_stat_t *d, register Rect *display_rect,
		int16 update_mask )
{
	char name[32];
	Rect field;
	register int16 len;
	register char *p;

	field.left = NAME_FIELD_LEFT;
	field.right = NAME_FIELD_RIGHT;
	field.top = display_rect->top + NAME_TOP_DELTA;
	field.bottom = field.top + TEXT_HEIGHT;

	pstrcopy( d->app.appname, name );
	if ( ( len = StringWidth( (StringPtr) name ) ) > NAME_FIELD_RIGHT - NAME_FIELD_LEFT ) {
		p = name + *name;
		do {
			len -= CharWidth( *p );
			--p;
			--*name;
		} while ( len > NAME_FIELD_RIGHT - NAME_FIELD_LEFT );
		*p = '�';
	}
	MoveTo( field.left, field.bottom - 2 );
	EraseRect( &field );
	set_fore_color( NAME_COLOR );
	DrawString( (StringPtr) name );
	set_fore_color( BLACK_COLOR );
}


void display_current_size( register Display_stat_t *d, register Rect *display_rect,
		int16 update_mask )
{
	char num[20];
	Rect field;

	field.left = SIZE_FIELD_LEFT;
	field.right = SIZE_FIELD_RIGHT;
	field.top = display_rect->top + SIZE_TOP_DELTA;
	field.bottom = field.top + TEXT_HEIGHT;

	pnumcopy( d->current_size, num );
	add_commas( num );
	MoveTo( field.right - StringWidth( (StringPtr) num ), field.bottom - 2 );
	EraseRect( &field );
	set_fore_color( SIZE_COLOR );
	DrawString( (StringPtr) num );
	set_fore_color( BLACK_COLOR );
}


void display_current_free( register Display_stat_t *d, register Rect *display_rect,
		int16 update_mask )
{
	char num[20];
	Rect field;

	field.left = FREE_FIELD_LEFT;
	field.right = FREE_FIELD_RIGHT;
	field.top = display_rect->top + FREE_TOP_DELTA;
	field.bottom = field.top + TEXT_HEIGHT;

	pnumcopy( d->current_free, num );
	add_commas( num );
	MoveTo( field.right - StringWidth( (StringPtr) num), field.bottom - 2 );
	EraseRect( &field );
	set_fore_color( FREE_COLOR );
	DrawString( (StringPtr) num );
	set_fore_color( BLACK_COLOR );
}


void add_commas( register char *num )
{
	register int16 num_commas, len;
	register char *p;

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


void display_heap( register Display_stat_t *d, Rect *display_rect, int16 update_mask )
{
	register int32 *p, *q;
	register int16 i;
	register int32 right, offset;
	register unsigned char run;
	Boolean update;
	Rect field;

	update = false;
	if ( d->current_heap_ok != d->last_heap_ok ) {
		d->last_heap_ok = d->current_heap_ok;
		update = true;
	}
	if ( d->current_heap_transitions != d->last_heap_transitions ) {
		d->last_heap_transitions = d->current_heap_transitions;
		update = true;
	}
	for ( i = d->current_heap_transitions, p = d->current_heap, q = d->last_heap; i; --i ) {
		if ( !update && *p != *q )
			update = true;
		*q++ = *p++;
	}

	if ( !update && !(update_mask & FORCE_UPDATE_HEAP_MASK) )
		return;

	field = *display_rect;
	field.left = HEAP_FIELD_LEFT;
	field.top += 2;
	field.bottom -= 2;

	if ( !d->current_heap_ok ) {
		EraseRect( &field );
		MoveTo( field.left, field.bottom - 2 );
		TextFace( bold );
		DrawString( (StringPtr) "\pBAD   BAD   BAD   BAD" );
		TextFace( 0 );
		return;
	}

	offset = 0;
	i = d->current_heap_transitions;
	p = d->current_heap;
	do {
		offset += *p++ & 0x00FFFFFF;
	} while ( offset < heap_offset_pixels && --i );
	if ( !i ) {
		EraseRect( &field );
		return;
	}

	set_fore_color_or_pattern( ( !heap_offset_pixels ? HEAP_BORDER_COLOR : BACK_COLOR ) );
	MoveTo( HEAP_FIELD_LEFT, field.top );
	Line( 0, CELL_HEIGHT - 5 );

	InsetRect( &field, 1, 1 );
	offset -= heap_offset_pixels;
	run = *(unsigned char *) (p - 1);
	for ( ;; ) {

		switch ( run ) {
		case HEAP_FREE_RUN:
			set_fore_color_or_pattern( HEAP_FREE_COLOR );
			break;
		case HEAP_LOCKED_RUN:
			set_fore_color_or_pattern( HEAP_LOCKED_COLOR );
			break;
		case HEAP_UNLOCKED_RUN:
			set_fore_color_or_pattern( HEAP_UNLOCKED_COLOR );
			break;
		case HEAP_PURGEABLE_RUN:
			set_fore_color_or_pattern( HEAP_PURGEABLE_COLOR );
			break;
		default:
			set_fore_color_or_pattern( BLACK_COLOR );
			break;
		}

		if ( offset < 16384 )
			field.right = field.left + offset;
		else
			field.right = display_rect->right;
		PaintRect( &field );
		field.left = field.right;
		if ( field.left > display_rect->right || !--i )
			break;

		run = *(unsigned char *) p;
		offset = *p++ & 0x00FFFFFF;
	}
	InsetRect( &field, 0, -1 );
	if ( field.right < display_rect->right ) {
		set_fore_color_or_pattern( HEAP_BORDER_COLOR );
		MoveTo( field.right, field.top );
		Line( 0, CELL_HEIGHT - 5 );
	}
	set_fore_color_or_pattern( BLACK_COLOR );

	MoveTo( HEAP_FIELD_LEFT + 1, field.top );
	LineTo( field.right, field.top );
	MoveTo( HEAP_FIELD_LEFT + 1, field.bottom - 1 );
	LineTo( field.right, field.bottom - 1 );

	if ( update_mask & FORCE_UPDATE_HEAP_MASK ) {
		++field.left;
		field.right = display_rect->right;
		if ( field.left < field.right )
			EraseRect( &field );
	}
}


void draw_grow_icon( WindowPtr the_window, Boolean active_state, Boolean rgn_state )
{
	Boolean save_hilited;
	Rect r;

	r = the_window->portRect;
	r.top += CELL_HEIGHT;
	ClipRect( &r );
	save_hilited = ((WindowPeek) the_window)->hilited;
	((WindowPeek) the_window)->hilited = active_state;
	DrawGrowIcon( the_window );
	((WindowPeek) the_window)->hilited = save_hilited;
	r.top = r.bottom - 15;
	r.left = r.right - 15;
	if ( rgn_state )
		ValidRect( &r );
	else
		InvalRect( &r );
	clip_with_controls();
}


void activate_window( WindowPtr the_window, Boolean active_state )
{
	int16 control_state;

	draw_grow_icon( the_window, active_state, true );
	control_state = active_state ? 0 : 255;
	HiliteControl( vert_scroll, control_state );
	HiliteControl( horiz_scroll, control_state );
	heap_cursor = 0;
}


Boolean handle_menu_select( uns32 menu_long )
{
	switch ( menu_long >> 16 ) {
	case APPLE_MENU:
		if ( (int16) menu_long == 1 ) {
			Do_about( App_window );
			UnloadSeg( Do_about );
		}
		else {
			char DA_name[256];

			GetItem( Apple_menu, (int16) menu_long, (StringPtr) DA_name );
			OpenDeskAcc( (StringPtr) DA_name );
		}
		HiliteMenu( 0 );
		return false;

	case FILE_MENU:
		return true;

	case EDIT_MENU:
		ZeroScrap();
		PutScrap( *(unsigned char *) mbox_text, 'TEXT', &mbox_text[1] );
		HiliteMenu( 0 );
		return false;

	default:
		return false;
	}
}


void check_color_usage( Boolean first_thru )
{
	Rect window_rect, t_rect;
	register int16 min_depth, t_depth, t_index;
	register GDHandle gdevice;
	register PixMapHandle pmap;
	register CTabHandle clut;
	register Boolean use_gray;

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
					if ( !TestDeviceAttribute( gdevice, gdDevType ) )
						use_gray = true;
					if ( (t_depth = (**pmap).pixelSize) < min_depth )
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

		if ( first_thru || t_index != clut_index ) {
			if ( !first_thru ) {
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
}


void set_fore_color_or_pattern( int16 color )
{
	if ( Use_color ) {
		HLock( (Handle) current_clut );
		RGBForeColor( &(**current_clut).ctTable[color].rgb );
		HUnlock( (Handle) current_clut );
	}
	else
		PenPat( (ConstPatternParam) ( (Ptr) (*patterns) + color * 8 + 2 ) );
}


void set_fore_color( int16 color )
{
	if ( Use_color )
		set_fore_color_or_pattern( color );
}


void set_back_color( int16 color )
{
	if ( Use_color ) {
		HLock( (Handle) current_clut );
		RGBBackColor( &(**current_clut).ctTable[color].rgb );
		HUnlock( (Handle) current_clut );
	}
}


void mouse_down_in_names( void )
{
	register int16 i, old_i;
	register Display_stat_t *d;
	char str[256];
	boolean in_bomb, mouse_in_bomb;
	Rect r;

	if ( the_event.where.v >= App_window->portRect.bottom - 15 ) {
		if ( Debugger_installed && the_event.where.h < 19 ) {
			SetRect( &r, 0, App_window->portRect.bottom - 14, 20,
					App_window->portRect.bottom );
			in_bomb = false;
			while ( StillDown() ) {
				GetMouse( &the_event.where );
				mouse_in_bomb = PtInRect( the_event.where, &r );
				if ( ( mouse_in_bomb && !in_bomb ) || ( !mouse_in_bomb && in_bomb ) ) {
					in_bomb ^= 1;
					InvertRect( &r );
				}
			}
			if ( in_bomb ) {
				if ( mbox_type == MBOX_HEAP_ADDRESS ) {
					pstrcopy( "\p;hx ", str );
					phexappend( mbox_value1, 8, str );
					DebugStr( (StringPtr) str );
				}
				else if ( mbox_type == MBOX_ADDRESS ) {
					pstrcopy( "\p;hx ", str );
					phexappend( mbox_value1, 8, str );
					pstrappend( "\p;wh ", str );
					phexappend( mbox_value2, 8, str );
					DebugStr( (StringPtr) str );
				}
				else
					Debugger();
				InvertRect( &r );
			}
		}
	}
	else {
		old_i = -1;
		while ( StillDown() ) {
			GetMouse( &the_event.where );
			i = the_event.where.v / CELL_HEIGHT;
			if ( i ) {
				i += GetCtlValue( vert_scroll ) - 1;
				if ( i < display_list_elems && i != old_i ) {
					old_i = i;
					d = display_list + i;
					mbox_type = MBOX_HEAP_ADDRESS;
					mbox_value1 = (int32) d->app.zone;
					pstrcopy( "\pHeap is at $", mbox_text );
					phexappend( mbox_value1, 8, mbox_text );
					pstrappend( "\p (", mbox_text );
					pstrappend( d->app.appname, mbox_text );
					pchappend( ')', mbox_text );
					draw_mbox( MBOX_CONTENTS );
				}
			}
		}
		if ( old_i >= 0 ) {
			THz save_zone;

			save_zone = *(THz *) TheZone;
			*(THz *) TheZone = display_list[old_i].app.zone;
			PurgeMem( 0x7FFFFFFF );
			CompactMem( 0x7FFFFFFF );
			*(THz *) TheZone = save_zone;
		}
	}
}


void mouse_down_in_heap( void )
{
	register int16 i;
	register int32 address, old_address;
	register Display_stat_t *d;
	char str[256];
	Rect r;

	if ( the_event.where.v >= App_window->portRect.bottom - 15 ||
			the_event.where.v < CELL_HEIGHT )
		return;

	switch ( heap_cursor ) {
	case ZOOM_IN_CURS:
	case ZOOM_OUT_CURS:
		if ( heap_cursor == ZOOM_IN_CURS ) {
			Swatch_prefs.heap_scale >>= 1;
			--Swatch_prefs.heap_scale_2n;
			reset_horiz_scroll();
	/* pin heap to left edge if clicking close enough to start */
			if ( the_event.where.h - HEAP_FIELD_LEFT > 35 || heap_offset_pixels)
				heap_offset_pixels = (heap_offset_pixels << 1) +
						the_event.where.h - HEAP_FIELD_LEFT - 2;
		}
		else {
			Swatch_prefs.heap_scale <<= 1;
			++Swatch_prefs.heap_scale_2n;
			reset_horiz_scroll();
	/* pin heap to left edge if clicking close enough to start */
			if ( the_event.where.h - HEAP_FIELD_LEFT > 35 || heap_offset_pixels)
				heap_offset_pixels = (heap_offset_pixels - (the_event.where.h -
						HEAP_FIELD_LEFT - 1)) >> 1;
		}
		if ( heap_offset_pixels < 0 )
			heap_offset_pixels = 0;
		else if ( heap_offset_pixels > heap_right_pixels )
			heap_offset_pixels = heap_right_pixels;
		SetCtlValue( horiz_scroll, heap_offset_pixels / horiz_scroll_scale );
		check_sizes( FORCE_UPDATE_HEAP_MASK | FORCE_UPDATE_HEAP_SCALE_MASK,
				CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
		break;

	case INFO_CURS:
		old_address = 0;
		SetRect( &r, HEAP_FIELD_LEFT + 1, CELL_HEIGHT, App_window->portRect.right - 15,
				 App_window->portRect.bottom - 15 );
		while ( StillDown() ) {
			GetMouse( &the_event.where );
			if ( PtInRect( the_event.where, &r ) ) {
				i = the_event.where.v / CELL_HEIGHT + GetCtlValue( vert_scroll ) - 1;
				if ( i < display_list_elems ) {
					d = display_list + i;
					address = ( (heap_offset_pixels + the_event.where.h - HEAP_FIELD_LEFT - 2) <<
							Swatch_prefs.heap_scale_2n ) + (int32) &d->app.zone->bkLim +
									60 /* MF bytes and block header */;
					if ( address != old_address && address <= (int32) d->app.zone->bkLim ) {
						old_address = address;
						mbox_type = MBOX_ADDRESS;
						mbox_value1 = (int32) d->app.zone;
						mbox_value2 = address;
						pchcopy( '$', mbox_text );
						phexappend( address, 8, mbox_text );
						draw_mbox( MBOX_CONTENTS );
					}
				}
			}
		}
		break;

	default:
		break;
	}
}


void set_cursor( void )
{
	Rect r;
	unsigned char keys[16];
	register int16 new_heap_cursor;
	Handle h;

	SetRect( &r, HEAP_FIELD_LEFT, CELL_HEIGHT, App_window->portRect.right - 15,
			 App_window->portRect.bottom - 15 );
	if ( PtInRect( the_event.where, &r ) ) {
		GetKeys( (KeyMap *) keys );
		if ( keys[6] & 0x80 )
			new_heap_cursor = INFO_CURS;
		else if ( keys[7] & 0x04 ) {
			if ( Swatch_prefs.heap_scale < MAX_HEAP_SCALE )
				new_heap_cursor = ZOOM_OUT_CURS;
			else
				new_heap_cursor = NO_ZOOM_CURS;
		}
		else {
			if ( Swatch_prefs.heap_scale > MIN_HEAP_SCALE )
				new_heap_cursor = ZOOM_IN_CURS;
			else
				new_heap_cursor = NO_ZOOM_CURS;
		}
	}
	else
		new_heap_cursor = ARROW_CURS;

	if ( new_heap_cursor != heap_cursor ) {
		heap_cursor = new_heap_cursor;
		if ( heap_cursor == ARROW_CURS )
			InitCursor();
		else {
			SetResLoad( true );
			h = GetResource( 'CURS', new_heap_cursor );
			if ( h ) {
				SetCursor( (Cursor *) *h );
				HPurge( h );
			}
		}
	}
}


/*----------------------------------------*/


void draw_mbox( int16 what )
{
	Rect r;
	RgnHandle save_clipRgn;
	BitMap bm;
	Handle h;
	Point pen;

	r.left = 0;
	r.right = HEAP_FIELD_LEFT - 3;
	r.top = App_window->portRect.bottom - 15;
	r.bottom = App_window->portRect.bottom;

	if ( what == MBOX_FRAME_AND_CONTENTS ) {
		set_fore_color_or_pattern( HEADER_BORDER_COLOR );
		EraseRect( &r );
		MoveTo( r.right, CELL_HEIGHT );
		LineTo( r.right, r.top - 1 );
		set_fore_color_or_pattern( BLACK_COLOR );

		if ( Debugger_installed ) {
			bm.bounds.right = (bm.bounds.left = 3) + 16;
			bm.bounds.top = (bm.bounds.bottom = r.bottom - 1) - 12;
			bm.rowBytes = 2;
			h = GetResource( 'SICN', MACSBUG_SICN );
			HLock( h );
			bm.baseAddr = *h + 8;
			CopyBits( &bm, &App_window->portBits, &bm.bounds, &bm.bounds,
					srcCopy, NULL );
			HUnlock( h );
			HPurge( h );
		}
	}
	if ( what == MBOX_CONTENTS || what == MBOX_FRAME_AND_CONTENTS ) {
		ValidRect( &r );
		save_clipRgn = NewRgn();
		GetClip( save_clipRgn );
		InsetRect( &r, 2, 2 );
		ClipRect( &r );

		MoveTo( 24, r.bottom - 2 );
		TextMode( srcCopy );
		TextFont( monaco );
		DrawString( (StringPtr) mbox_text );
		TextMode( srcOr );
		TextFont( geneva );
		++r.top;
		GetPen( &pen );
		r.left = pen.h;
		EraseRect( &r );

		SetClip( save_clipRgn );
		DisposeRgn( save_clipRgn );
	}
	else if ( what == MBOX_ERASE ) {
		EraseRect( &r );
		InvalRect( &r );
	}
}


void clip_with_controls( void )
{
	ClipRect( &App_window->portRect );
}


void clip_without_controls( void )
{
	Rect r;

	r = App_window->portRect;
	r.right -= 15;
	r.bottom -= 15;
	ClipRect( &r );
}


void size_controls( void )
{
	register int16 top, left, bottom, right;

	reset_horiz_scroll();
	reset_vert_scroll();

	left = App_window->portRect.right - 15;
	right = App_window->portRect.right + 1;
	top = CELL_HEIGHT - 1;
	bottom = App_window->portRect.bottom - 14;
	HideControl( vert_scroll );
	MoveControl( vert_scroll, left, top );
	SizeControl( vert_scroll, right - left, bottom - top );
	ShowControl( vert_scroll );

	left = HEAP_FIELD_LEFT - 3;
	right = App_window->portRect.right - 14;
	top = App_window->portRect.bottom - 15;
	bottom = App_window->portRect.bottom + 1;
	HideControl( horiz_scroll );
	MoveControl( horiz_scroll, left, top );
	SizeControl( horiz_scroll, right - left, bottom - top );
	ShowControl( horiz_scroll );
}


void reset_vert_scroll( void )
{
	register int16 ti;
	register int16 old_num_display_lines, old_value;
	Rect r;

	old_num_display_lines = num_display_lines;
	old_value = GetCtlValue( vert_scroll );
	ti = App_window->portRect.bottom - 15 - CELL_HEIGHT;
	partial_display_line = (ti % CELL_HEIGHT) != 0;
	num_display_lines = ti / CELL_HEIGHT;

	SetCtlMax( vert_scroll, ( (ti = display_list_elems - num_display_lines) > 0 ? ti : 0 ) );

	if ( old_value && num_display_lines > old_num_display_lines ) {
		old_value -= num_display_lines - old_num_display_lines;
		SetCtlValue( vert_scroll, ( old_value < 0 ? 0 : old_value ) );
		r = App_window->portRect;
		r.top += CELL_HEIGHT;
		r.right -= 15;
		r.bottom -= 15;
		InvalRect( &r );
	}
}


void reset_horiz_scroll( void )
{
	register int32 t;

	display_heap_bytes = (int32) (App_window->portRect.right - 15 - HEAP_FIELD_LEFT) *
			Swatch_prefs.heap_scale;
	if ( ( t = max_heap_size - display_heap_bytes ) > 0 ) {
		heap_right_pixels = ( t >> Swatch_prefs.heap_scale_2n ) +
				( (t & (Swatch_prefs.heap_scale-1)) ? 1 : 0 ) + 20;
		horiz_scroll_scale = heap_right_pixels / 32768L + 1;
	}
	else {
		heap_right_pixels = 0;
		horiz_scroll_scale = 1;
	}
	SetCtlMax( horiz_scroll, heap_right_pixels / horiz_scroll_scale +
			( ( heap_right_pixels % horiz_scroll_scale ) ? 1 : 0 ) );
}


pascal void vert_scroll_proc( ControlHandle the_control, int16 part )
{
	Rect r;
	register int16 old, new;
	RgnHandle aRgn;

	aRgn = NewRgn();
	r = App_window->portRect;
	r.right -= 15;
	r.top = CELL_HEIGHT;
	r.bottom -= 15;
	old = GetCtlValue( the_control );

	if ( part == inUpButton ) {
		if ( old ) {
			SetCtlValue( the_control, old - 1 );
			ScrollRect( &r, 0, CELL_HEIGHT, aRgn );
			check_sizes( FORCE_UPDATE_FIELDS, 0, 0 );
		}
	}
	else if ( part == inDownButton ) {
		if ( old < display_list_elems - num_display_lines ) {
			SetCtlValue( the_control, old + 1 );
			ScrollRect( &r, 0, -CELL_HEIGHT, aRgn );
			check_sizes( FORCE_UPDATE_FIELDS | FORCE_UPDATE_BOTTOM_MASK, num_display_lines - 1,
					num_display_lines + partial_display_line - 1 );
		}
	}
	else if ( part == inPageUp ) {
		new = old - ( num_display_lines - 1 );
		if ( new < 0 )
			new = 0;
		if ( new != old ) {
			SetCtlValue( the_control, new );
			check_sizes( FORCE_UPDATE_FIELDS, CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
		}
	}
	else if ( part == inPageDown ) {
		new = old + ( num_display_lines - 1 );
		if ( new > display_list_elems - num_display_lines )
			new = display_list_elems - num_display_lines;
		if ( new != old ) {
			SetCtlValue( the_control, new );
			check_sizes( FORCE_UPDATE_FIELDS, CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
		}
	}
	DisposeRgn( aRgn );
}


pascal void horiz_scroll_proc( ControlHandle the_control, int16 part )
{
	int32 delta, old_heap_offset_pixels;

	old_heap_offset_pixels = heap_offset_pixels;
	if ( part == inUpButton )
		delta = -HORIZ_ARROW_DELTA;
	else if ( part == inDownButton )
		delta = HORIZ_ARROW_DELTA;
	else if ( part == inPageUp )
		delta = -( (display_heap_bytes >> Swatch_prefs.heap_scale_2n) - HORIZ_ARROW_DELTA );
	else if ( part == inPageDown )
		delta = (display_heap_bytes >> Swatch_prefs.heap_scale_2n) - HORIZ_ARROW_DELTA;
	else
		return;

	heap_offset_pixels += delta;
	if ( heap_offset_pixels < 0 )
		heap_offset_pixels = 0;
	else if ( heap_offset_pixels > heap_right_pixels )
		heap_offset_pixels = heap_right_pixels;
	
	if ( heap_offset_pixels != old_heap_offset_pixels ) {
		SetCtlValue( horiz_scroll, heap_offset_pixels / horiz_scroll_scale );
		check_sizes( FORCE_UPDATE_HEAP_MASK, CHECK_ALL_SIZES_START, CHECK_ALL_SIZES_END );
	}
}

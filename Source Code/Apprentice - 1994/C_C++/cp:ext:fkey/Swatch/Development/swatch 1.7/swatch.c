/**

	swatch.c
	Copyright (c) 1990-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#ifndef __about__
#include "about.h"
#endif
#ifndef __content__
#include "content.h"
#endif
#ifndef __ctypes__
#include "ctypes.h"
#endif
#ifndef __display__
#include "display.h"
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

#define MacJmp	(*(int32 *)0x120)


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

static Boolean in_foreground;
static MenuHandle Apple_menu, File_menu, Edit_menu, Heap_menu;

static EventRecord the_event;

static unsigned char *strings;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

int main( void );

static Boolean handle_menu_select( uns32 menu_long );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/

SysEnvRec This_mac;
Boolean Debugger_installed;


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

int main( void )
{
	int16 i;
	Boolean quit, edit_enabled;
	Handle h;

	MaxApplZone();
	InitGraf( &thePort );
	InitFonts();
	InitWindows();
	InitDialogs( NULL );
	TEInit();
	InitMenus();
	InitCursor();

	for ( i = 5; i; --i )
		EventAvail( 0, &the_event );
	in_foreground = true;

	CouldAlert( OUT_OF_MEMORY_ALRT );

	SysEnvirons( 1, &This_mac );
	if ( This_mac.systemVersion < 0x0700 ) Bail( BAD_SYSTEM_VERSION_ALRT );
	Debugger_installed = MacJmp != 0;


	h = GetResource( 'STR#', SWATCH_STR_ );
	if ( !h ) Bail( OUT_OF_MEMORY_ALRT );
	MoveHHi( h );
	HLock( h );
	strings = (unsigned char *) StripAddress( *h );


// the order here is necessary...

	Prefs_init();
	Display_init();
	Heap_list_init();
	Content_init();
	Window_init();


	if	(	!(Apple_menu = GetMenu(APPLE_MENU))
			|| !(File_menu = GetMenu(FILE_MENU))
			|| !(Edit_menu = GetMenu(EDIT_MENU))
		) Bail( OUT_OF_MEMORY_ALRT );

	AddResMenu( Apple_menu, 'DRVR' );
	InsertMenu( Apple_menu, 0 );
	InsertMenu( File_menu, 0 );
	InsertMenu( Edit_menu, 0 );
	DrawMenuBar();
	edit_enabled = true;

	quit = false;
	while ( !quit ) {

		WaitNextEvent( everyEvent, &the_event, (in_foreground ? Prefs.wne_in_foreground :
				Prefs.wne_in_background), NULL );
		SetPort( App_window );
		if ( FrontWindow() == App_window && edit_enabled ) {
			DisableItem( Edit_menu, 0 );
			DrawMenuBar();
			edit_enabled = false;
		}
		else if ( FrontWindow() != App_window && !edit_enabled ) {
			EnableItem( Edit_menu, 0 );
			DrawMenuBar();
			edit_enabled = true;
		}
		Window_idle();

		switch ( the_event.what ) {
		case nullEvent:
			GlobalToLocal( &the_event.where );
			Content_set_cursor( &the_event );
			break;

		case mouseDown:
			i = FindWindow( the_event.where, (WindowPtr *) &the_event.message );
			if ( i != inContent )
				InitCursor();
			if ( i == inMenuBar )
				quit = handle_menu_select( MenuSelect( the_event.where ) );
			else if ( (WindowPtr) the_event.message == App_window )
				quit = Window_mousedown( &the_event, i );
			break;

		case keyDown:
		case autoKey: {
			char key;

			key = the_event.message & charCodeMask;
			if ( the_event.modifiers & cmdKey )
				quit = handle_menu_select( MenuKey( key ) );
			break;
		}

		case updateEvt:
			if ( (WindowPtr) the_event.message == App_window )
				Window_update( &the_event );
			break;

		case activateEvt:
			if ( (WindowPtr) the_event.message == App_window )
				Window_activate( &the_event, (the_event.modifiers & activeFlag) != 0 );
			break;

		case app4Evt:
			if ( *(unsigned char *) &the_event.message == 1 /* suspend/resume */ ) {
				if ( the_event.message & 1 /* resume */ )
					in_foreground = true;
				else
					in_foreground = false;
				HiliteWindow( App_window, in_foreground );
				Window_activate( &the_event, in_foreground );
			}
			break;

		default:
			break;
		}
	}

	Window_close();
	Prefs_save();
	Heap_list_exit();
}


static Boolean handle_menu_select( uns32 menu_long )
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
		break;

	case FILE_MENU:
		return true;

	case EDIT_MENU:
		break;

	default:
		break;
	}
	HiliteMenu( 0 );
	return false;
}


void Bail( int16 alert_number )
{
	StopAlert( alert_number, NULL );
	ExitToShell();
}


unsigned char *pstr( int16 index )
{
	unsigned char *p;

	if ( index < 1 || index > *(int16 *)strings ) return "\p<bad string index>";

	for ( p = strings + 2; index > 1; --index, p += *p + 1 );
	return p;
}


Boolean Swatch_in_foreground( void )
{
	return in_foreground;
}
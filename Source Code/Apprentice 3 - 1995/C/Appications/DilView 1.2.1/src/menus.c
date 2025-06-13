/* menus.c */

#include <TransSkel.h>
#include "menus.h"
#include "about.h"
#include "globals.h"
#include "windows.h"
#include "load_files.h"
#include "controls.h"
#include "printing.h"
#include "list.h"
#include "nyi.h"

void myInitMenus( void )
{
	MenuHandle menuH;
	
	// add the apple menu
	SkelApple( k_aboutItem, DoAboutBox );
	
	// add other menus
	menuH = GetMenu( mFile );
	SkelMenu( menuH, myFileMenu, nil, false, false );
	
	menuH = GetMenu( mEdit );
	SkelMenu( menuH, nil, nil, false, false );
	
	menuH = GetMenu( mScale );
	SkelMenu( menuH, myScaleMenu, nil, false, false );
	
	menuH = GetMenu( mSelect );
	SkelMenu( menuH, mySelectMenu, nil, false, false );
	
	// update the menu bar
	DrawMenuBar();
}
	
void myDisposeMenus( void )
{
	; // skelcleanup does this for me!! (:
}

pascal void myFileMenu( short item )
{
	switch( item )
	{
		// open a single gif file
		case iOpen:
				OpenOne();
				SetPortToWindow();
				InvalRect( &((GetGWinPort())->portRect) );
				break;
				
		// open a bunch of gif files at once
		case iOpenMultiple:
				OpenMany();
				SetPortToWindow();
				InvalRect( &((GetGWinPort())->portRect) );
				break;
				
		// do the standard page setup dialog
		case iPageSetup:
				myPageSetup();
				break;
				
		// print the current dil
		case iPrintOne:
				myPrint( k_print_cur );
				break;
				
		// print all the marked dils
		case iPrintMarked:
				myPrint( k_print_marked );
				break;
				
		// print all the dils
		case iPrintAll:
				myPrint( k_print_all );
				break;
		
		// for a quit menu selection, terminate the event loop, cleanup, and exit
		case iQuit:
				SkelStopEventLoop();
				break;
	}
}

pascal void myScaleMenu( short item )
{
	MenuHandle menuH;
	
	// get a handle for the scale menu
	menuH = GetMHandle( mScale );

	// remove the current checkmark
	SetItemMark( menuH, GetScale(), noMark );

	// place the new checkmark
	SetItemMark( menuH, item, noMark );
	
	// store the new scale
	SetScale( item );
	
	// adjust the window
	ScaleWindow();

	// set the scrollbar to 0	
	myZeroScrollBar();

	// redraw it
	myUpdate( false );
}

pascal void mySelectMenu( short item )
{
	dil_rec *dil;
	
	switch( item )
	{
		case iPrevious:
				dil = GetPrevDil( GetCurDil() );
				break;
		case iNext:
				dil = GetNextDil( GetCurDil() );
				break;
	}
	
	// if we didn't run off either end of the list then set to the proper new dil
	if( dil != nil )
		SetCurDil( dil );
	
	// redraw
	myUpdate( false );
}

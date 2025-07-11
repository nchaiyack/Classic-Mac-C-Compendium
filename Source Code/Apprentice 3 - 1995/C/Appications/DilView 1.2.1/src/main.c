/* main.c */

#include <TransSkel.h>
#include "menus.h"
#include "windows.h"
#include "globals.h"
#include "dil.h"
#include "list.h"
#include "controls.h"
#include "status_bar.h"
#include "resources.h"
#include "printing.h"
#include "about.h"
#include "apple_event_support.h"

void main( void )
{
	SkelInit( nil );
	
	myInitResource();
	myInitWindows();
	myInitMenus();
	myInitDil();
	myInitList();
	myInitControls();
	myInitStatusBar();
	myInitPrinting();
	myEventInit();

	// set the scale to 50%, so as to check the proper item _and_ set the proper resize bounds
	myScaleMenu( i50scale );
		
	// make our window visible
	myShowWindow();

	// an initial splash screen		
	SplashScreen( false );
		
	// deal with apple events in queue
	SkelDoEvents( highLevelEventMask );

	// make the first dil loaded the current dil (only applies if AEOpenDocs was done..)
	SetCurToHead();
	myUpdate( false );
	
	// enter the event loop
	SkelEventLoop();
	
	myDisposePrinting();
	myDisposeStatusBar();
	myDisposeControls();
	myDisposeList();
	myDisposeDil();
	myDisposeWindows();
	myDisposeMenus();
	myDisposeResource();

	SkelCleanup();
}
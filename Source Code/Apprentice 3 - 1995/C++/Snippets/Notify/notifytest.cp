
#include <string.h>
#include <math.h>

#include <QuickDraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <Events.h>
#include <OSEvents.h>
#include <SegLoad.h>
#include <Memory.h>
#include <Dialogs.h>
#include <Events.h>
#include <Notification.h>
#include <Processes.h>

#include "notification_request.h"

void main()
{
	MaxApplZone();
	InitGraf( &(qd.thePort));
	InitFonts();
	InitWindows();
	InitMenus();
	InitDialogs( 0);
	TEInit();
	InitCursor();
	FlushEvents( everyEvent, 0);

	notify_user( "this is a test");
	notify_user( "It can also beep", true);
	//
	// NB: never call ExitToShell. The destructors won't get called that way
	// This leads to a memory leak in notify_user.
	// (Alternative: encapsulate the entire program, except the 'ExitToShell'
	// in a pair of accolades)
	// (or, your development system patches ExitToShell behind your back, and
	//  does call destructors for you, anyway)
	//
//	ExitToShell();
}

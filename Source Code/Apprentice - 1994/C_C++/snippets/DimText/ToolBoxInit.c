#include <Fonts.h>
#include <Events.h>
#include <Dialogs.h>
#include <Menus.h>

#include "ToolBoxInit.h"

#define	REMOVE_ALL_EVENTS	0

void ToolBoxInit( void )
{
	InitGraf( (Ptr)&qd.thePort );
	InitFonts();
	FlushEvents( everyEvent, REMOVE_ALL_EVENTS );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0 );
	InitCursor();
}

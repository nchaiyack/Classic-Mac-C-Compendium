#include <Balloons.h>
#include "mac-specific.h"

Boolean TrapAvailable( short trapnum );

#ifndef GESTALT
#define GESTALT			0xA1AD
#endif

extern Boolean			g_help_under_apple;
extern MenuHandle		g_AppleMenu;

void	Install_help( void );

void	Install_help( void )
{
	MenuHandle	balloon_menu;
	long		response;
	
	g_help_under_apple = !TrapAvailable(GESTALT) ||
		(Gestalt(gestaltHelpMgrAttr, &response) != noErr) ||
		((response & 1) == 0L) ||
		(HMGetHelpMenuHandle(&balloon_menu) != noErr);
	if (g_help_under_apple)
	{
		InsMenuItem( g_AppleMenu, "\pDVIM72-Mac Manual", 1 );
	}
	else
	{
		AppendMenu( balloon_menu, "\pDVIM72-Mac Manual" );
	}
}

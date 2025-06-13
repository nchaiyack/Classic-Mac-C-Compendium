// System Utilities.c v1.0
//
// by Kenneth Worley
// Public Domain
//
// Contact me at: America Online: KNEworley
//		internet: KNEworley@aol.com or kworley@fascination.com
//

#include "System Utilities.h"

void InitToolbox( void )
{
	/* Initialize the Mac Toolbox Managers */
	
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

short	GetSysVersion( void )
{
	SysEnvRec		env;
	
	SysEnvirons( 2, &env );
	
	if ( env.systemVersion >= 0x0700 )
		return 7;
	else if ( env.systemVersion >= 0x0600 )
		return 6;
	else
		return 0;
}

Boolean HasColorQuickDraw( void )
{
	SysEnvRec		env;
	
	SysEnvirons( 2, &env );
	
	return env.hasColorQD;
}

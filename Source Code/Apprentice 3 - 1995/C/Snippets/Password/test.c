#include "Password.h"

main()
{
	Str32	password;
	short	r;
	
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

	BlockMove("\pPASSWORD", (Ptr)password, 9);
	
	r = SetPassword(password);
	r = AskPassword(password);
}

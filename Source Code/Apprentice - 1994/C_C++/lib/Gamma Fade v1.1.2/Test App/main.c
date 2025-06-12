#include "about.h"


main() {

	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0);

	doAboutBox();

	ExitToShell();
	}
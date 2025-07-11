#include <QuickDraw.h>
#include <stdio.h>

extern OSErr musInitialize(void);
extern void musFinalize(void);
extern OSErr musPlay(void);

main()
{
	OSErr err;
	
	printf("hit return to start...\n");
	{
		char buf[256];
		gets(buf);
	}
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();

	err = musInitialize();
	printf("Init got err %d\n", err);
	err = musPlay();
	printf("Play got err %d\n", err);
	
	{
		char buf[256];
		gets(buf);
	}
	musFinalize();
	ExitToShell();
}

//-- MAIN.C

// The main entry point for all this.


#include <stdio.h>


main()
{
	int mode;

	InitMacintosh();
	UnloadSeg(InitMacintosh);
	while (!DoEvent());
	ExitToShell();
}

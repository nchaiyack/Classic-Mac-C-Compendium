/*______________________________________________________________________

	main.h - Main Window Manager Interface.
	
	Copyright � 1988, 1989, 1990, Northwestern University.
_____________________________________________________________________*/

#ifndef __main__
#define __main__

extern void main_SetPort (void);
extern void main_DoScan (short theMenu, short theItem);
extern void main_WaitInsert (Boolean firstDisk, short *vRefNum, 
	Boolean *canceled);
extern void main_Disk (long message);
extern Boolean main_Save (void);
extern void main_Open (void);

#endif


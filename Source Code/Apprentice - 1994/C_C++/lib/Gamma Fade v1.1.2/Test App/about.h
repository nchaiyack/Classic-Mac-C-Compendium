// File "about.h" - Header for the Cool About Box

// * ****************************************************************************** *

#define kAboutBoxDLOG		 128

#define kActiveBut		   0
#define kInactiveBut	 255
#define kPassItOn		   0
#define kDontPassIt		  -1

// * ****************************************************************************** *


void doAboutBox(void);
pascal Boolean aboutFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItem);

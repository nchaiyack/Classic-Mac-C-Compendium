//-- windows.h

// The various structures used by the drafting program for managing windows.


#ifndef WINDOWSH
#define WINDOWSH



//-- Structures.


//-- DirectData

// Directory data.

struct DirectData {
	short indent;							/* How far does this indent (in units of 12 pix) */
	char data[42];							/* Name of this object, and data (in Pascal form) */
	char auxdata[20];						/* Size of this object (auxdata) */
	char auxdata2[20];						/* File aux data */
};


//-- DrawWindow

// The windows used for drawing.

struct DrawWindow {
	WindowRecord w;
	short inuse;
	
	/* Controls */
	ControlHandle yScroll;					/* The two scroll bars in this window */
	
	/* Data */
	short state;
	struct DirectData **data;				/* Directory data handle */
	short vRefNum;							/* VRefNum of this volume */
	char vName[64];							/* Volume name (in C) */
};





//-- Constants.

#define	MAXWINDOWS			10				/* # of drawing windows that can be opened */
#define WK_PLAN				10				/* Window kind of a drawing window */
#define	WK_CONTROL			11				/* The Control window */




//-- Globals.

extern struct DrawWindow *drawList;			/* The list of windows to bring up */

#endif
#ifndef GLOBALS_H
#define GLOBALS_H

#include "complex.h"
#include "mathtype.h"
#include "morph.h"

// ---------------  global variables

extern MenuHandle	gAppleM;
extern MenuHandle	gFileM;
extern MenuHandle	gEditM;
extern MenuHandle	gTypeM;	// menus
extern WindowPtr	gMainWindow;
extern WindowPtr	gSelectWindow;
extern WindowPtr	gCurrentWindow;
extern WindowPtr	gControlDialog;
extern MorphProcH	gMorphProcH;
extern MorphProcP	gDefaultProcP;
extern MorphProcH	gDefaultProcH;		// Default morph eqn
extern int			gPrintY;			// height of printing font
extern int			gTypeChkItem;
extern CursHandle	gCrossCursor;
extern int			gSleepTime;
extern BitMap		gOffBM;				// our offscreen bitmap
extern GrafPort		gOffGP;				// our offscreen graphics port

// ---------------  Coordinate/image information
extern MathType gXmin;
extern MathType gXmax;
extern MathType gYmin;
extern MathType gYmax;
extern MathType gCreal;
extern MathType gCimag;
extern MathType gCenterX;
extern MathType gCenterY;

// ---------------  Functions

void Cleanup(void);		// main.c

#include "utils.h"

#include "default.h"

#include "redraw.h"

#include "trackcoords.h"

#include "offscreen.h"

#include "savepict.h"

#endif
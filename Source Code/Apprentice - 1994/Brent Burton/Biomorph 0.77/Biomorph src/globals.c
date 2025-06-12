// This has all the globals variables for the biomorph program.
#include <MacHeaders>
#include "complex.h"
#include "constants.h"
#include "mathtype.h"
#include "morph.h"

// ---------------  Global variables
MenuHandle	gAppleM;
MenuHandle	gFileM;
MenuHandle	gEditM;
MenuHandle	gTypeM;	// menus
WindowPtr	gMainWindow		=NULL;
WindowPtr	gSelectWindow	=NULL;
WindowPtr	gCurrentWindow	=NULL;
WindowPtr	gControlDialog	=NULL;
MorphProcH	gMorphProcH		=NULL;
MorphProcP	gDefaultProcP	=NULL;
MorphProcH	gDefaultProcH   =NULL;		// Default morph eqn
int			gPrintY;					// height of printing font
int			gTypeChkItem;
CursHandle	gCrossCursor;
int			gSleepTime = 5;
BitMap		gOffBM;						// our offscreen bitmap
GrafPort	gOffGP;						// our offscreen graphics port

// ---------------  Coordinate/image information
MathType gXmin = -1.0;
MathType gXmax = 1.0;
MathType gYmin = -1.0;
MathType gYmax = 1.0;
MathType gCreal = 0.0;
MathType gCimag = 0.0;
MathType gCenterX = 0.0;
MathType gCenterY = 0.0;


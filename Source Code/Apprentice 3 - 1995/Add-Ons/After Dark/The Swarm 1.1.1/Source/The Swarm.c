/*
 *    The Swarm 1.1.1 - A Screensaver Module by Leo Breebaart.
 *					  Copyright � 1994-95 Kronto Software.
 *
 *
 *	  For non-technical information about this module and for the credits,
 *    see the README file.
 * 	  
 * 	  This module displays a variable number of line-segments (the �Bees�), which 
 * 	  chase another line segment (the �Queen�) across the screen.
 * 	  
 * 	  I have commented the code in a way that experienced programmers will find
 * 	  overkill, but it was done in the hope that beginning programmers
 * 	  will find �The Swarm� a useful starting point for writing their own 
 * 	  screensaver modules.
 * 	  
 * 	  For general information about how to write an After Dark module, 
 * 	  see the After Dark Programmer�s Manual, but make sure you have the most recent
 * 	  version (released with After Dark 3.0 and later). The comments in this
 *    module do assume you are at least *aware* of the basic After Dark mechanisms.
 *
 *	  Here we go...
 */
 
 	// Include file for the GWorld offscreen drawing stuff.
#include <QDoffscreen.h>

	// Include file for After Dark type definitions.
#include "AfterDarkTypes.h"

	// Interface to Jonas Englund's CLUT fade library for nice fade-in/fade-out effects.
#include "fade.h"

#define kClutID 132					// Resource ID of color lookup table (clut)

	// Constants
#define kMaxBees 100				// Maximum number of Bees.
#define kQueenMaxVelocity 12		// Maximum Queen velocity (in pixels per frame).
#define kQueenMaxAcceleration 5		// Maximum Queen acceleration.
#define kBeeMaxVelocity  11			// Maximum Bee velocity.
#define kBeeMaxAcceleration  3		// Maximum Bee acceleration.
#define kBorderWidth  50			// Queen won't go any nearer than this many pixels to the
									// edge of the screen and the demo rectangle.
						
	// Data structures
typedef short	**ShortHandle;		// Our Bee position arrays will be dynamic arrays
									// allocated using handles.

typedef struct		// All data we use is stored in this struct.
{
    short queenX[2], queenY[2];				// Storage for Queen line segment.
    short queenVelocityX, queenVelocityY;	// Current Queen velocity components.
    ShortHandle beeX[2], beeY[2];			// Bee line segments.
    ShortHandle beeVelocityX, beeVelocityY;	// Bee velocities components.

    Rect swarmRect; 				// Bounding rectangle for swarm at time '0'.
    Rect oldRect;					// Bounding rectangle for swarm at time '1'.
    
    short maxQueenVelocity, maxBeeVelocity;			// These variables store the constants
    short maxQueenAcceleration, maxBeeAcceleration;	// defined earlier.
    short border;									//
    
    short nBees;			// The current number of active Bees.
    
    long delay;				// The current animation speed depends on this value.
    long startDrawing;		// Works together with delay to control animation speed.
    
    long switchColor;		// The current bee color-changing speed depends on this value.
    long colStepCnt;		// Counter to 'count-up' until switchColor.
    long colDirection;		// Controls the direction of the bee color changes.
    long colIndex;			// CLUT index for the current bee color.
    
    Boolean demoMode;		// Are we currently in After Dark demo Mode?
    Boolean doFade;			// Should we do a smooth fade?

    RGBColor whiteRGB, blackRGB;	// Color QuickDraw colors: white and black.
    RGBColor queenRGB, beeRGB;		// Color QuickDraw colors for Queen and Bees.
    RGBColor backRGB;				// Color QuickDraw color for animation background.
      
    Rect monitorRect;		// Rectangle corresponding to main monitor.
    short winW, winH;		// The info about monitorRect we really need.

   	GWorldPtr gMyOffG;		// A pointer to an offscreen graphics world.
}
TSwarmData, *TSwarmDataPtr;

							
	// Here we define some handy macros for accessing the 
	// Swarm's position and velocity values. These macros work under the
	// assumption that you have a valid handle variable named 'swarm', pointing
	// to a storage struct as defined above.

	// Position of Bee b at time t (where t is either 0 or 1).
#define BX(t,b)  (*((*(swarm->beeX[t]))+(b)))
#define BY(t,b)  (*((*(swarm->beeY[t]))+(b)))

	// Current velocity of Bee b.
#define BXV(b)	(*((*swarm->beeVelocityX)+(b)))
#define BYV(b)	(*((*swarm->beeVelocityY)+(b)))

	// Position of the Queen at time t (where t is either 0 or 1).
#define QX(t)  (swarm->queenX[t])
#define QY(t)  (swarm->queenY[t])

	// Current velocity of the Queen.
#define QXV	(swarm->queenVelocityX)
#define QYV	(swarm->queenVelocityY)

	// A comment about this 'time t' business: each swarm member's
	// line segment is drawn 'from' position 0 to position 1. So
	// t=1 is the 'old' position, t=0 the 'new' position. After one
	// frame of animation, e.g. QX(0) will be assigned to QX(1) (making
	// that the 'old' position), and QX[0] will get a new value. In other 
	// words, every swarm creature essentially traces a continuous path
	// across the screen.
	
	
	// Some more handy macros.

#define RAND(v) (RangedRdm(-(v)/2, (v)/2))		// Random integer around 0.  
#define abs(x) (((x) > 0) ? (x) : -(x))			// Standard macro for 'abs'.
 
#define setmin(x, min) if (x < min) min = x;	// Keep track of a minimum value.
#define setmax(x, max) if (x > max) max = x;	// Keep track of a maxmimum value.

	// After Dark's way of showing an error to the user.
	// '##' is an ANSI-ism meaning meta-concatenation.
#define ErrorMsg(m) BlockMove(CtoPstr(m), params->errorMessage, 1 + m##[0]);

	// In order to avoid unexpected unpleasant surprises...
#define SafeDisposHandle(h) if ((Handle)(h)) DisposeHandle((Handle)(h))

	// QuickDraw color constants used throughout the program.
RGBColor gWhite = { 0xFFFF, 0xFFFF, 0xFFFF };
RGBColor gBlack = { 0, 0, 0 };
RGBColor gGold  = { 0xFFFF, 0x9C9C, 0x0808 };
RGBColor gBlue	= {	0x0808, 0x0000, 0x2929 };
RGBColor gRed	= { 0xBDBD, 0x0000, 0x0000 };

	// Let's be good ANSI citizens, and define some function prototypes.
	// These functions will be called from the AfterDarkShell.c code. 
OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoHelp(RgnHandle blankRgn, GMParamBlockPtr params);

	// Some local functions of our own.
static OSErr AboutBoxError(Rect graf);
static long RangedRdm(long min, long max);
static Handle BestNewHandle(Size s);
static Boolean XYInRect(Rect *r, short x, short y);
static void DrawStringAt (short x, short y, Str255 str);
static void CenterString (short y, StringPtr str);
static void CenterRectHorizontal (Rect *r, short y);
static void CenterRectVertical (Rect *r, short x);
static void CenterRect (Rect *r, short x, short y);
static void LineFromTo (short xFrom, short yFrom, short xTo, short yTo);
static Boolean RandomBool (void);
static void SetPortTextStyle (TextStyle *style);
static void GetPortTextStyle (TextStyle *style);
static void RedAlert (StringPtr theStr);

	// Now we come to the fun part: the actual implementations of the functions.
	// First: DoInitialize, which allocates our swarm data structure as defined
	// above, initializes the variables in that struct, and does a gazillion checks
	// for possible problems.

OSErr
DoInitialize (Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params)
{
    register TSwarmDataPtr swarm;	// The swarm, obviously.
    register short b;				// Index for Bee loops.
        
        // Local variables in order to avoid having to write
        // 'swarm->...' all the time. Could have used macro's here as
        // well, I suppose.
	RGBColor whiteRGB, blackRGB, queenRGB, beeRGB;
	short nBees;
	long delay;
	
		// If After Dark is in demo-mode, the demoRect can be found in the
		// params argument to this function, but we need to make temporary
		// changes to it, so we use a local copy.
	Rect borderedDemoRect;	

     	// Our offscreen graphics world needs Color QuickDraw.
	if (!params->colorQDAvail)
	{
		DisposHandle(*storage);
		ErrorMsg("The Swarm:  Sorry, I need Color QuickDraw to run!");
		return ModuleError;
	}
    
    	// Allocate 'master' handle to the storage struct.
    if ((*storage = BestNewHandle(sizeof(TSwarmData))) == NULL)
    { 	
    	ErrorMsg("The Swarm:  Couldn't allocate enough memory!");
    	return ModuleError;
    }

    	// Lock down the storage so we can safely refer to it by pointer. 
    HLockHi(*storage);
    swarm = (TSwarmDataPtr) **storage;
        
        // Initialize the random number generator.
	params->qdGlobalsCopy->qdRandSeed = TickCount();

		// Set up and initialize the colors we'll be using.
					
		// Absolute black and white.
	whiteRGB.red = whiteRGB.green = whiteRGB.blue = 0xFFFF;
	blackRGB.red = blackRGB.green = blackRGB.blue = 0;
	
		// A golden color for the Bees, a bright red for the Queen.
	beeRGB.red = 0xFFFF; beeRGB.green = 0x9C9C; beeRGB.blue = 0x0808;
	queenRGB.red = 0xBDBD; queenRGB.green = 0; queenRGB.blue = 0;
	
		// The Queen's red color gets mapped to black by the Mac
		// if the screen is less then 8-bit. We don't want that...
	if (params->monitors->monitorList[0].curDepth < 4)
		queenRGB = whiteRGB;

		// This is a very crude and stupid way to try and detect if the main screen
		// is in grayscale mode. If so, I make the queen white-on-black,
		// since anything else is guaranteed to look bad.
		// In the next version, this will all be solved much cleaner.
	if (!(params->systemConfig & (1L << 3)))
		queenRGB = whiteRGB;

		// Initialize the struct variables from our local convenience ones.
	swarm->whiteRGB = whiteRGB;
	swarm->blackRGB = blackRGB;
	swarm->queenRGB = queenRGB;
	swarm->beeRGB   = beeRGB;
	swarm->backRGB  = blackRGB;
	
		// We get our number of bees from a slide control in the
		// After Dark control panel interface. This number can be zero!
	swarm->nBees = nBees = params->controlValues[0];
	
		// We get the animation speed from a second slide control.
		// This is primarily intended for Macs which are *too fast*,
		// which is why we express speed in terms of the delay between frames.
		// This delay will be either 0, 2, 4, 6, 8 or 10 system ticks long.
	swarm->delay = delay = (long)10 - (2*params->controlValues[1] / 20);
	
		// We get the color changing speed from a third slide control.
		// There is no neat functional mapping from the slider value to
		// this speed value, hence the cascading if-statements.
		// The speed values are expressed in animation frames, i.e.
		// a switchColor of 25 means that the color will changes every
		// 25 animation steps, and a value of 1 means the color changes
		// at every step. The value of 0 is a special, and signifies
		// no color change at all. Instead the bees will have the golden
		// color used in version 1.0 of this module.
	if (params->controlValues[2] == 0)
		swarm->colStepCnt = swarm->switchColor = 0;
	else if (params->controlValues[2] > 0 && params->controlValues[2] < 25)
		swarm->colStepCnt = swarm->switchColor = 25;
	else if (params->controlValues[2] >= 25 && params->controlValues[2] < 50)
		swarm->colStepCnt = swarm->switchColor = 10;
	else if (params->controlValues[2] >= 50 && params->controlValues[2] < 75)
		swarm->colStepCnt = swarm->switchColor = 5;
	else if (params->controlValues[2] >= 75 && params->controlValues[2] < 100)
		swarm->colStepCnt = swarm->switchColor = 3;
	else if (params->controlValues[2] == 100)
		swarm->colStepCnt = swarm->switchColor = 1;
	
		// A random start color, which I don't want to be red (because that
		// doesn't look neat enough), hence the 30-220 rage of clut indexes.	
	swarm->colIndex = RangedRdm(30, 220);
	
		// Our index can go up or down the clut table (with wrap around at the
		// ends.
	swarm->colDirection = (RandomBool() ? 1 : -1);
	
		// My clut fading routines currently only work or 8-bit clut devices.
	swarm->doFade = (params->monitors->monitorList[0].curDepth == 8);
	
		// Sanity checks. These things should never occur.
	if (nBees < 0 || nBees > 100)
	{
		DoClose(*storage, (RgnHandle) nil, (GMParamBlockPtr) nil);
		ErrorMsg("The Swarm:  Internal Error � insane number of bees!");
		return ModuleError;
	}
	
	if (delay < 0 || delay > 10)
	{
		DoClose(*storage, (RgnHandle) nil, (GMParamBlockPtr) nil);
		ErrorMsg("The Swarm:  Internal Error � insane bee speed value!");
		return ModuleError;
	}
	
	if (swarm->switchColor < 0 || swarm->switchColor > 100)
	{
		DoClose(*storage, (RgnHandle) nil, (GMParamBlockPtr) nil);
		ErrorMsg("The Swarm:  Internal Error � insane color speed value!");
		return ModuleError;
	}
	
		// See DoDrawFrame for more info on how this variable is used to implement
		// the delay.
	swarm->startDrawing = Ticks;

		// We need to know if we are in demo Mode in the DoDrawFrame function,
		// but we want to avoid having to call the EmptyRect function every time,
		// so we store it in a Boolean here.
	swarm->demoMode = !EmptyRect((&params->demoRect));
		
		// Allocate handles, and afterwards (a) check if the allocation
		// succeeded, and (b) move the handles to high memory and lock them.
		// I could have done this with static arrays (beeX[2][kMaxBees]),
		// I suppose, but I wanted to learn how to work with handles.
		// The speed difference in access is, to my surprise, negligible,
		// So I see no reason to change it back.
		//
		// Notice that we allocate enough memory for the maximum number
		// of Bees in advance. This is because in Demo Mode we want to
		// be able to let the user dynamically change nBees, which is
		// the *current* number of Bees, if you'll remember. In order to
		// do that neatly, we want to have the storage ready and initialized
		// beforehand.
		
	swarm->beeX[0] = (ShortHandle) BestNewHandle(kMaxBees*sizeof(short));
	swarm->beeX[1] = (ShortHandle) BestNewHandle(kMaxBees*sizeof(short));
	swarm->beeY[0] = (ShortHandle) BestNewHandle(kMaxBees*sizeof(short));
	swarm->beeY[1] = (ShortHandle) BestNewHandle(kMaxBees*sizeof(short));

	swarm->beeVelocityX = (ShortHandle) BestNewHandle(kMaxBees*sizeof(short));
	swarm->beeVelocityY = (ShortHandle) BestNewHandle(kMaxBees*sizeof(short));
	
	if (!(swarm->beeX[0] && swarm->beeX[1] && swarm->beeY[0] && swarm->beeY[1]
		  && swarm->beeVelocityX && swarm->beeVelocityY))
	{
		DoClose(*storage, (RgnHandle) nil, (GMParamBlockPtr) nil);
		ErrorMsg("The Swarm:  Couldn't allocate enough internal memory!");
		return ModuleError;
	}
	
		// Lok all the handles so we won't have to worry about things
		// being moved out from under our feet.
	HLockHi((Handle) swarm->beeX[0]);
    HLockHi((Handle) swarm->beeX[1]);
    HLockHi((Handle) swarm->beeY[0]);
    HLockHi((Handle) swarm->beeY[1]);
    
    HLockHi((Handle) swarm->beeVelocityX); 
    HLockHi((Handle) swarm->beeVelocityY);

		// Swarm animation uses *only* the main monitor (...[0]).
    swarm->monitorRect = params->monitors->monitorList[0].bounds;
    swarm->winW   = swarm->monitorRect.right - swarm->monitorRect.left;     
    swarm->winH   = swarm->monitorRect.bottom - swarm->monitorRect.top;
    
    	// For different monitor sizes, we want the relation between
    	// border and winW to be the same as that between kBorderWidth and 640
    	// (my 13" screen). The cast to 'long' is because the multiplication
    	// will overflow a 2-byte short for large monitors. You have no *idea*
    	// how long it took me to discover this one (primarily because I don't
    	// *have* a large monitor).
    swarm->border = ((long) kBorderWidth * swarm->winW) / 640;
    
    	// Initialize variables with earlier defined constants.
    swarm->maxQueenVelocity 	= kQueenMaxVelocity;
    swarm->maxBeeVelocity  		= kBeeMaxVelocity;
    swarm->maxQueenAcceleration = kQueenMaxAcceleration;
    swarm->maxBeeAcceleration  	= kBeeMaxAcceleration;
    
    	// Make both bounding rectangles intially empty.
    SetRect(&swarm->oldRect, 0, 0, 0, 0);
    SetRect(&swarm->swarmRect, 0, 0, 0, 0);
        
        // Initial Queen position.
    QX(0) = swarm->monitorRect.left + RangedRdm(swarm->border, swarm->winW - swarm->border);
    if (QX(0) < 10)
		RedAlert("\pInitial Queen position: X coordinate is smaller then 10!");
	QY(0) = swarm->monitorRect.top + RangedRdm(swarm->border, swarm->winH - swarm->border);
    if (QY(0) < 10)
		RedAlert("\pInitial Queen position: Y coordinate is smaller then 10!");

		// If in demo mode, then make sure that the
		// initial Queen position lies at least 'border' pixels *outside* the demoRect!
    if (swarm->demoMode)
    {
	    borderedDemoRect = params->demoRect;
	    InsetRect(&borderedDemoRect, -swarm->border, -swarm->border);

	    while (XYInRect(&borderedDemoRect, QX(0), QY(0)))
	    {
	    	QX(0) = swarm->monitorRect.left + RangedRdm(swarm->border, swarm->winW - swarm->border);
	    	QY(0) = swarm->monitorRect.top + RangedRdm(swarm->border, swarm->winH - swarm->border);
	    }
	}
    if (QX(0) < 10)
		RedAlert("\pInitial Queen position: X coordinate is smaller then 10, after the demo check!");
    if (QY(0) < 10)
		RedAlert("\pInitial Queen position: Y coordinate is smaller then 10, after the demo check!");

		// For the first frame, our line segment is just a point.
    QX(1) = QX(0);
    QY(1) = QY(0);
    QXV = QYV = 0;
    
    	// Ditto for the Bees, although (a) all Bees start from the same
    	// physical position as the Queen, and (b) all Bees have a different
    	// initial velocity. This gives a nice 'fountaining' effect on startup
    	// of the animation.
    for (b = 0; b < kMaxBees; b++)
    {
        BX(0,b) = QX(0);
        BX(1,b) = BX(0,b);
        BY(0,b) = QY(0);
        BY(1,b) = BY(0,b);

        BXV(b) = RAND(7);
        BYV(b) = RAND(7);
    }
		
		// Now allocate an offscreen graphics world, where we can do
		// fast drawing without disturbing the real screen.
		// I recommend looking up this call in Inside Macintosh or Think Reference!
	if (NewGWorld(&(swarm->gMyOffG), 0, &(swarm->monitorRect), nil, nil, noNewDevice+useTempMem) != noErr)  	
	{ 	
		DoClose(*storage, (RgnHandle) nil, (GMParamBlockPtr) nil);
		ErrorMsg("The Swarm:  Not enough memory for offscreen graphics world!");
		return ModuleError;			
	}
		
	return noErr;
}


	// Next, the DoBlank function. This function performs two tasks: it blacks
	// out the real screen (on all monitors, not just the main monitor we use for the
	// animation), and it also blacks out the offworld screen.

OSErr
DoBlank (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params)
{
    register TSwarmDataPtr swarm;
    short i;
        
    GWorldPtr		currPort;	// The 'real' screen consists of two components
	GDHandle		currDev;	// which we'll save in these variables.

		// It's only safe to do this because we know the handle is still locked!
    swarm = (TSwarmDataPtr) *storage;
    
    	// Save the 'real' screen.
	GetGWorld(&currPort,&currDev);
	
		// Switch to the offscreen world.
	SetGWorld((swarm->gMyOffG), nil);
	
		// Set the backgroundcolor, and erase the whole world.
   	RGBBackColor(&swarm->backRGB);
	EraseRgn(blankRgn);
	
		// Switch back to the 'real' screen.
	SetGWorld (currPort, currDev);
	
		// This is not redundant! We are in a different world now...
   	RGBBackColor(&swarm->backRGB);

		// Do a nice fade-out/fade-in if the user specified
		// that check box in the Control Panel, and if the monitor
		// is capable of doing so.
		// The routines I use are *only* fit for 8-bit CLUT displays, so
		// don't ever try changing this!
	if (swarm->doFade && params->monitors->monitorList[0].curDepth == 8)
    {
    	fade_screen(96,true);
		EraseRgn(blankRgn);
   		fade_screen(64,false);
   	}
   	else
		EraseRgn(blankRgn);
   	
   		// If we'll be doing color animation then install the new color table.
	if (swarm->doFade && swarm->switchColor > 0)
   		install_clut(132);
   	
    return noErr;
}


	// Ha, finally we come to the meat of our module: the DoDrawFrame function.
	// Although there is a lot of code here, what actually happens is quite simple.
	// First, all the position and velocity variables are updated and checked for
	// bouncing etc. Then, we *erase* (in the offscreen world) the 'old' swarm
	// by filling the swarm's bounding rectangle (which will vary from frame to frame!)
	// with the background color. Then, we draw the 'new' swarm. Finally, we use
	// the famous 'CopyBits' function to move the changed areas of the offscreen
	// world to the real screen world. And that's all.
	
	// One more comment: a lot of the array accesses could have been optimized
	// to make the code run faster. Local variables could have been used to avoid
	// the pointer arithmetic caused by all those global handles and arrays.
	// The BlockMove function could have been called to updates entire arrays in
	// one sweep.
	// However, all those accesses taken together still take up only a negligable fraction
	// of this function's total execution time, when compared to the cost of
	// doing the graphics. CopyBits is an expensive function, and for large numbers
	// of Bees the drawing of the line segments takes even more time.
	// This is why I have decided to refrain from optimizing. It doesn't matter very
	// much speedwise, and such optimizations would only obscure the underlying algorithm.
	
OSErr
DoDrawFrame (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params)
{	    
    register TSwarmDataPtr swarm;
    
    	// Again, lots of local variables in order to avoid
    	// too much 'swarm->...' stuff.
    RGBColor beeRGB, queenRGB, whiteRGB, backRGB;
    Rect monitorRect, *swarmRect, *oldRect;
    short winW, winH, nBees, border;
    long delay, dummy;		// 'dummy' variable is unused, but needed
    						// for Delay toolbox call.
    
    	// Variables used for calculating the swarm's bounding rectangle.
    short xMin=10000, xMax=-10000, yMin=10000, yMax=-10000;

    GWorldPtr currPort;		// The 'real' screen consists of two components.
	GDHandle  currDev;		// which we'll save in these variables.
	
		// The actual pixel maps of offscreen and real screen, respectively.
	PixMapHandle offBase, realBase;

		// Various rectangles...:-)
	Rect unionRect, targetRect, helpRect, pixRect, screenRect;
	
		// Distances from a Bee to the Queen.
    short dx, dy, distance;			

		// Bee counter for loops.
    register short b;

	//RedAlert("\p[1] Entering the DoDrawFrame function");

		// storage was already locked in DoInitialize, so this is safe.
    swarm = (TSwarmDataPtr) *storage;
    
    nBees = swarm->nBees;
    delay = swarm->delay;

    	// If we are in Demo mode, reread the nBees and delay values -- the user may have
    	// changed them dynamically.
    	// At present it is not possible to change the color speed value in Demo
    	// mode. Not because it is so difficult to do, but it means lots of stupid
    	// extra code, and I just don't feel like spending the time, frankly.
    if (swarm->demoMode)
    {
    	if (nBees != params->controlValues[0])
    	    nBees = swarm->nBees = params->controlValues[0];

    	if (delay != params->controlValues[1]) 
		    delay = swarm->delay = (long)10 - (2*params->controlValues[1] / 20);
	}

    	// The next thing to do is to check for delay. This can be done
    	// naively by simply using the Delay system function (with 'swarm->delay' as
    	// parameter), but that would mean that we would simply spend those
		// ticks *doing nothing* -- and not allowing anything else to do anything
		// either. That's why we do it differently: if the delay has not passed yet,
		// we simply exit from this function at once, and wait until we are called again.
		// That way, almost all the delay time is given to the After Dark parent
		// process which can then presumeably use it to check for system activity etc.
		
	if (delay != 0)
		if (TickCount() < swarm->startDrawing)
			return noErr;
		else
			swarm->startDrawing = TickCount() + delay;
	
		// Initialize the other local variables from their swarm counterparts.
	monitorRect = swarm->monitorRect;
	winW   = swarm->winW;
	winH   = swarm->winH;
	border = swarm->border;
	
	backRGB  = swarm->backRGB;
	whiteRGB = swarm->whiteRGB;
	queenRGB = swarm->queenRGB;
	beeRGB   = swarm->beeRGB;
	
    swarmRect = &swarm->swarmRect;
    oldRect   = &swarm->oldRect;
    
    	// Age the swarm bouding rectangle.
    *oldRect = *swarmRect;
    
		// First, we do the Queen Stuff:

    	// Age the position arrays. 
    QX(1) = QX(0);
    QY(1) = QY(0);
    
    if (QX(1) < 5)
    {
		RedAlert("\pDoDrawFrame: Queen's X coordinate smaller than 5!");
		return ModuleError;
	}
    if (QY(1) < 5)
    {
		RedAlert("\pDoDrawFrame: Queen's Y coordinate smaller than 5!");
		return ModuleError;
	}
	
    	// Accelerate.
    QXV += RAND(swarm->maxQueenAcceleration);
    QYV += RAND(swarm->maxQueenAcceleration);

    	// Speed limit checks.
    if (QXV > swarm->maxQueenVelocity)
        QXV = swarm->maxQueenVelocity;
    else 
    	if (QXV < -swarm->maxQueenVelocity)
        	QXV = -swarm->maxQueenVelocity;
    if (QYV > swarm->maxQueenVelocity)
        QYV = swarm->maxQueenVelocity;
    else
    	if (QYV < -swarm->maxQueenVelocity)
        	QYV = -swarm->maxQueenVelocity;

    	// Fill new 'current' positions.
    QX(0) = QX(1) + QXV;
    QY(0) = QY(1) + QYV;

    	// Bounce Checks.
    if ((QX(0) < monitorRect.left + border) || (QX(0) > monitorRect.left + winW - border - 1))
    {
    		// These two statements (and all similar ones further on)
    		// cause a swarm element to 'bounce off' according to a
    		// "angle of entry is angle of exit" rule. It looks much more
    		// cryptic than it really is. Trust me.
        QXV = -QXV;
        QX(0) += QXV << 1;
    }
    if ((QY(0) < monitorRect.top + border) || (QY(0) > monitorRect.top + winH - border - 1))
    {
        QYV = -QYV;
        QY(0) += QYV << 1;
    }
    
 		// If we are in demo Mode, we want the Queen to 'bounce' off
 		// the demoRect as well. This takes some hairy additional testing :-)
    
    	// Without these macro's 'hairy' would become 'bloody incomprehensible'.
 #define DL (params->demoRect.left - border/2)
 #define DR (params->demoRect.right + border/2)
 #define DT (params->demoRect.top - border/2)
 #define DB (params->demoRect.bottom + border/2)
 
 		// The actual tests.
    if (!EmptyRect(&params->demoRect))
    {
     	if ((QX(0) < DR) && (QX(0) > DL) && (QY(0) < DB) && (QY(0) > DT))
    	{
    		if ((QX(1) <= DL) || (QX(1) >= DR))
    		{
    			QXV = -QXV;
         		QX(0) += QXV << 1;
         	}
         	if ((QY(1) <= DT) || (QY(1) >= DB))
         	{
    			QYV = -QYV;
         		QY(0) += QYV << 1;
    		}
    	}
   	}
        
        // Keep track of the minimal bouding rect of the swarm so far.
    setmin(QX(0), xMin);
    setmin(QY(0), yMin);
	setmax(QX(0), xMax);
    setmax(QY(0), yMax);

	setmin(QX(1), xMin);
    setmin(QY(1), yMin);
	setmax(QX(1), xMax);
    setmax(QY(1), yMax);
    
   
   		// Now we get to the Bee stuff, which is basically
   		// the same, except that (a) Bees do *not* bounce off walls or
   		// demoRects, and (b) Bees will try to 'follow' the Queen.
   		 
    	// First, don't ever let things settle down. 
    if (nBees > 0)          // Avoid later division by 0!
    {
        BXV(RangedRdm(0, nBees)) += RAND(3);
        BYV(RangedRdm(0, nBees)) += RAND(3);
    }
	
    for (b = 0; b < nBees; b++)
    {
        	// Age the arrays. 
        BX(1, b) = BX(0, b);
        BY(1, b) = BY(0, b);
        
        	// Accelerate.
        dx = QX(1) - BX(1, b);
        dy = QY(1) - BY(1, b);
        	
        	// This calculation of the true distance from the dx/dy values
        	// is an approximation that allows us to keep everything in 
        	// integer math. Otherwise we'd have do square root operations...
        distance = abs(dx) + abs(dy); 
        if (distance == 0)
            distance = 1;
        BXV(b) += (dx * swarm->maxBeeAcceleration) / distance;         
        BYV(b) += (dy * swarm->maxBeeAcceleration) / distance;

        	// Speed limit checks.
        if (BXV(b) > swarm->maxBeeVelocity)
            BXV(b) = swarm->maxBeeVelocity;
        else 
        	if (BXV(b) < -swarm->maxBeeVelocity)
            	BXV(b) = -swarm->maxBeeVelocity;
        if (BYV(b) > swarm->maxBeeVelocity)
            BYV(b) = swarm->maxBeeVelocity;
        else
        	if (BYV(b) < -swarm->maxBeeVelocity)
            	BYV(b) = -swarm->maxBeeVelocity;

    		// Fill new 'current' positions.
        BX(0, b) = BX(1, b) + BXV(b);
        BY(0, b) = BY(1, b) + BYV(b);
        
        	// Keep track of the minimal bouding rect of the swarm so far.
        setmin(BX(0,b), xMin);
        setmax(BX(0,b), xMax);
        setmin(BX(1,b), xMin);
        setmax(BX(1,b), xMax);

        setmin(BY(0,b), yMin);
        setmax(BY(0,b), yMax);
        setmin(BY(1,b), yMin);
        setmax(BY(1,b), yMax);   
    }       

		// swarmRect will now be set to the minimal bounding rectangle we've been maintaining,
		// which we want to 'clip' by intersecting it with the screen rectangle,
		// and which we finally need to combine with the *old* bounding rectangle,
		// to give as a result the minimal bounding rectangle of the entire screen
		// area that has been affected by this step of the animation.
		
    SetRect(swarmRect, xMin-1, yMin-1, xMax+1, yMax+1);
    SectRect(&monitorRect, swarmRect, swarmRect);
    UnionRect(oldRect, swarmRect, &unionRect);
    
    	// Save the real screen world.
	GetGWorld(&currPort, &currDev);
	
		// Switch to the offscreen world.
    SetGWorld (swarm->gMyOffG, nil);
    	
    	// Erase the old swarm bounding rectangle.
	RGBBackColor(&swarm->backRGB);
    EraseRect(oldRect);
    
    	// Draw the new swarm, first Queen, then Bees.
    RGBForeColor(&queenRGB);
    LineFromTo(QX(0),QY(0), QX(1),QY(1));

		// Handle the color 'animation'.
		
		// Are we (capable of) animating?
	if (swarm->doFade && swarm->switchColor > 0)
	{		
			// Should we do a color switch?
		if (swarm->colStepCnt == swarm->switchColor)
		{				
				// Update bee color.
			Index2Color(swarm->colIndex, &swarm->beeRGB);
									
				// Once in a while, change the direction in
				// which we're traversing the color lookup table.
			if (RangedRdm(0, 300) == 0)
				swarm->colDirection = -swarm->colDirection;

				// Update counter (with wrap around).
				// For reasons I do not currently understand the color
				// animation will not work correctly if I use the clut
				// indexes 0-4 and 251-254. If you do know why this is
				// so, please drop me a line!
			swarm->colIndex += swarm->colDirection;
			if (swarm->colIndex > 250)
				swarm->colIndex = 5;
			if (swarm->colIndex < 5)
				swarm->colIndex = 250;
				
			swarm->colStepCnt = 0;
		}
		else
			swarm->colStepCnt++;
	}
	
		
    RGBForeColor(&swarm->beeRGB);

		// Draw the Bees.
    for (b = 0; b < nBees; b++)
        LineFromTo(BX(0, b),BY(0, b), BX(1, b),BY(1, b));

		// Switch back to the real screen.    
	SetGWorld(currPort, currDev);	   		
   	
		// Retrieve actual pixel maps for both offscreen and real screen.
	realBase = GetGWorldPixMap((GWorldPtr) currPort);
	offBase  = GetGWorldPixMap(swarm->gMyOffG);

	   	// These next two calls appear to make no sense, but
   		// are absolutely necessary for CopyBits to function
   		// correctly. See the Apple TechNote on this subject (or Inside Macintosh)
    RGBBackColor(&whiteRGB);
	RGBForeColor(&swarm->blackRGB);			

   	// Blit the changed area from offscreen to realscreen.
	CopyBits((BitMap *) (*offBase), (BitMap *) (*realBase), 
			  &unionRect, &unionRect,
			  srcCopy, nil);
           	    
    return noErr;
}


	// Well, the hard part is now over -- almost. "The Swarm" also features a
	// funky About Box, which has a miniature version of the swarm animation
	// going on inside of it. Creating that animation was simpler than I feared it would
	// be: for the most part I just call the previous functions, i.e. DoInitialize,
	// DoBlank, and DoDrawFrame -- and that's it. The tricky part lies in getting
	// some correct variables in place, and setting up the right graphics port.
	//
	// One thing you should realize about DoHelp: when After Dark calls this
	// function, it will already have set the currPort and the blankRgn to the help rectangle. 
	// So you are at this point no longer drawing to the entire screen.
	//
	// Final note: if you want to use a DoHelp function yourself, realize that (a) 
	// you need to tell After Dark you want to 'take over' (see the Cals resource in
	// the Programmer's Manual), and (b) 'storage' is *not* initialized in DoHelp,
	// because After Dark calls DoHelp without calling DoInitialize first, in contrast
	// to e.g. DoDrawFrame. Unfortunately, a lot of code shows example 'DoAbout' or
	// 'DoHelp' functions with the 'storage' handle as a parameter. But this parameter
	// will *not* be intialized, and if you use it, you will crash horribly.

OSErr
DoHelp (RgnHandle blankRgn, GMParamBlockPtr params)
{
	// If you've read and understood DoInitialize and DoDrawFrame, I think you'll
	// have enough knowledge so that I won't have to annotate every single variable
	// here, right?

	TSwarmData **miniSwarm;
   	
	long dummy;
	GrafPtr helpGraf;
	TextStyle savedStyle;
	short oldCount, oldDelay;
	char oldFade;
	Rect oldBounds;
	RgnHandle miniBlankRgn = NewRgn();
	RGBColor miniSwarmBackground;
	
   	PicHandle helpPict;				
	Rect picRect, helpRect, miniRect;
	Rect r;
	StringHandle str;
	short helpRectHeight;
	Boolean runningAD30;
	short onefourth, onethird, ninetenths;


		// The miniswarm has a dark blue instead of a black background.
	miniSwarmBackground = gBlue; 

		// Get the real screen.
	GetPort(&helpGraf);

		// Get the real screen rectangle. miniRect is the version
		// we�ll use in the rest of this function, helpRect is the
		// �original� version we�ll need to pass to the error handling 
		// routine. Again, that last part is an ugly hack, and will be
		// cleaned up in the next version.
	helpRect = miniRect = helpGraf->portRect;
	helpRectHeight = helpRect.bottom - helpRect.top;
	
		// AD 3.0 has this really weird bug: the portRect field of the helpPort
		// doesn't correspond to what the portRect area *really* is. The following
		// hack tries to determine if we are running under 3.0 , and if so it
		// adjusts the portRect to what it really should be.
	runningAD30 = (helpRectHeight > 270);
	if (runningAD30)
	{
		helpRect.left++; 
		helpRect.top++;
	}
	
	RGBBackColor(&miniSwarmBackground);
	EraseRect(&helpRect);

		// We want the layout of the about box elements to look right
		// both for the large AD 3.0 box area and for the smaller AD 2.0
		// area. That's why we use 'relative' height coordinates like
		// 'onefourth' or 'onethird' instead of absolute coordinates.
		
	onefourth = helpRectHeight / 4;
	onethird = helpRectHeight / 3;
	ninetenths = (9*helpRectHeight) / 10;

		// Draw the PICT, and release the resource.
		// Load the 'about' PICT resource.
	if ((helpPict = GetPicture(2000)) == nil) 
	{
		ErrorMsg("The Swarm:  Couldn�t load PICT resource for help picture!");
		return AboutBoxError(helpRect);
	}
	picRect = (**helpPict).picFrame;
	CenterRectHorizontal(&picRect, onefourth / 2);
	DrawPicture(helpPict, &picRect);
	ReleaseResource((Handle) helpPict);
		
	GetPortTextStyle(&savedStyle);
	RGBForeColor(&gGold);
	LineFromTo(4, 5+onethird, helpRect.right-4, 5+onethird);
	LineFromTo(4, ninetenths+5, helpRect.right-4, ninetenths+5);
	TextFont(geneva); TextSize(9); TextFace(bold);
	CenterString(34+onefourth/2, "\pA Screensaver Module");
	CenterString(12+34+onefourth/2, "\pby Leo Breebaart");

	RGBForeColor(&gRed);
	LineFromTo(4, 5+onethird+10, helpRect.right-4, 5+onethird+10);
	LineFromTo(4, ninetenths-5, helpRect.right-4, ninetenths-5);
	CenterString((ninetenths + helpRectHeight)/ 2 + 6, "\pKronto Software 1995");
			     
	SetRect(&miniRect, 0, 5+onethird+10+1, helpRect.right, ninetenths-5-1);

		// We now manually change the graphics environment
		// from the entire help area to the subarea where we want the
		// mini animation to take place (look at the about box in action if this
		// is not clear to you). The current Port is moved and made smaller.
		// The Toolbox calls take care of all the nasty details.
	LocalToGlobal(&topLeft(miniRect));
	LocalToGlobal(&botRight(miniRect));
	MovePortTo(miniRect.left, miniRect.top);
	PortSize(miniRect.right-miniRect.left, miniRect.bottom - miniRect.top);
	
		// Save some relevant parameters that the 'real' After Dark
		// animation will need to have restored later on.
	oldCount  = params->monitors->monitorCount;
	oldBounds = params->monitors->monitorList[0].bounds;
	oldFade   = params->controlValues[2];
	oldDelay  = params->controlValues[1];

		// Change the parameters temporarily. One monitor (not that the current main
		// animation handles multiple monitors, mind you, but in a
		// future version it will, while here we really want just one screen.
	params->monitors->monitorCount = 1;
	params->monitors->monitorList[0].bounds = helpGraf->portRect;
		// Don't use fading/color animation!
	params->controlValues[2] = 0;
	params->controlValues[1] = 100;
	
	RectRgn(miniBlankRgn, &helpGraf->portRect);
	
		// Initialize the miniSwarm struct. Note that we have a problem
		// with error management here: I can use ErrorMsg all I want,
		// but After Dark will do nothing with the return value of
		// the DoHelp function, for some reason. So I have implemented
		// a special AboutBoxError() function of my own to handle
		// errors.
	if (DoInitialize((Handle *) &miniSwarm, miniBlankRgn, params) != noErr) 
	{
		ErrorMsg("The Swarm:  Initialization of helpSwarm failed!");
		return AboutBoxError(helpRect);
	}

		// Change some values to make them better suited for
		// miniature animation. Notice that unlike 'params'
		// before, we do not need to save the old values here,
		// since miniSwarm is entirely local to this function.
    (**miniSwarm).maxQueenVelocity = 7;
    (**miniSwarm).maxBeeVelocity   = 6;
	(**miniSwarm).nBees = 20;
	(**miniSwarm).switchColor = 0;
	(**miniSwarm).doFade = 0;
	(**miniSwarm).backRGB = miniSwarmBackground;

		// Blank the miniSwarm region
	if (DoBlank((Handle) miniSwarm, miniBlankRgn, params) != noErr)
	{
		ErrorMsg("The Swarm:  DoBlank of helpSwarm failed!");
		return AboutBoxError(helpRect);
	}
	
		// Wait for the user to release the mouse button, if necessary.
	while (Button())
		;
		
		// Animate, until the user presses the mouse button.
	while (!Button())
	{
		if (DoDrawFrame((Handle) miniSwarm, miniBlankRgn, params) != noErr)
		{
			ErrorMsg("The Swarm:  DoDrawFrame of helpSwarm failed!");
			return AboutBoxError(helpRect);
		}
			// uncrippled mini-animation is too fast!!
		Delay(3, &dummy);
	}
		// Close it all up.
	DoClose((Handle) miniSwarm, miniBlankRgn, params);

		// Restore old params values.
	params->monitors->monitorCount = oldCount;
	params->monitors->monitorList[0].bounds = oldBounds;
	params->controlValues[2] = oldFade;
	params->controlValues[1] = oldDelay;
		
		// Finally, display some text in the area where
		// the animation used to be.

	if ((helpPict = GetPicture(2001)) == nil) 
	{
		ErrorMsg("The Swarm:  Couldn�t load PICT resource for help picture!");
		return AboutBoxError(helpRect);
	}
	
	RGBBackColor(&miniSwarmBackground);
	SetRect(&r, 0, 0, miniRect.right-miniRect.left, miniRect.bottom - miniRect.top);
	EraseRect(&r);
	picRect = (**helpPict).picFrame;
	CenterRect(&picRect, ((miniRect.right-miniRect.left) / 2), 
						 ((miniRect.bottom-miniRect.top) / 2));
	DrawPicture(helpPict, &picRect);
	ReleaseResource((Handle) helpPict);
	DisposeRgn(miniBlankRgn);
	DisposeHandle((Handle) miniSwarm);

		// Wait for a final mouse click, and then exit.
	while (Button())
		;
 	while (!Button())
 		;
 	
	SetPortTextStyle(&savedStyle);
 	FlushEvents(everyEvent, 0);	

	return noErr;
}
	

	// The DoClose function merely disposes of all those handles and 
	// offscreen worlds. Nothing interesting here.

OSErr
DoClose (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params)
{
	TSwarmData **swarm = (TSwarmData **) storage;
	short i;
    
	if ((**swarm).doFade && params->monitors->monitorList[0].curDepth == 8)
    {	
    	if ((**swarm).doFade)
	    	fade_screen(1,true);
		FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);
    	if ((**swarm).doFade)
			fade_screen(1,false);
   	}
   	
    if (swarm)
    {
        SafeDisposHandle((**swarm).beeVelocityX);
		SafeDisposHandle((**swarm).beeVelocityY);
		SafeDisposHandle((**swarm).beeX[0]);
		SafeDisposHandle((**swarm).beeX[1]);
		SafeDisposHandle((**swarm).beeY[0]);
		SafeDisposHandle((**swarm).beeY[1]);
		
		DisposeGWorld((**swarm).gMyOffG);
        SafeDisposHandle(storage);
    }

    return noErr;
}


	// This function will one day, in the next version of the swarm, 
	// become a special error manager for handling errors that occur in
	// situations where After Dark itself won�t take action, e.g. in 
	// animated �About� boxes.
	// For now it is a very rudimentary function, which simply draws
	// some text. This is not a good example of how to handle situations
	// like this, but I don�t have time for anything better right now.
static OSErr
AboutBoxError (Rect r)
{	
		// These tedious calls simply have the total effect of moving
		// the drawing area back to exactly the part of the screen
		// corresponding with the original About Box contents.
		
	Rect currRect = thePort->portRect;
	TextStyle savedStyle;

	MovePortTo(r.left, r.top);
	PortSize(r.right-r.left, r.bottom-r.top);

	ForeColor(blackColor);
	BackColor(whiteColor);
	
	GlobalToLocal(&topLeft(r));
	GlobalToLocal(&botRight(r));

	FillRect(&r, white);

	GetPortTextStyle(&savedStyle);
	TextFont(geneva); TextSize(9);
	
 	DrawStringAt(15,20, "\pI'm sorry � an error has occurred.");
 	DrawStringAt(15,30, "\pNothing serious, mind you. The module");
 	DrawStringAt(15,40, "\pprobably just ran out of memory.");
 	DrawStringAt(15,60, "\pYou see, I have not implemented decent");
 	DrawStringAt(15,70, "\perror management routines for this");
 	DrawStringAt(15,80, "\pAbout Box yet. Next version, I promise.");
 	DrawStringAt(15,100, "\pIf you *do* have lots of memory, but you");
 	DrawStringAt(15,110, "\pstill see this message, then something");
 	DrawStringAt(15,120, "\p*is* probably very wrong, and I would");
 	DrawStringAt(15,130, "\preally appreciate an e-mail bug report.");
 	DrawStringAt(15,150, "\pThanks!");
	SysBeep(1); SysBeep(1);
	
	MovePortTo(currRect.left, currRect.top);
	PortSize(currRect.right-currRect.left, currRect.bottom-currRect.top);
 	
	while (Button())
		;
 	while (!Button())
 		;
 	
 	SetPortTextStyle(&savedStyle);
	FlushEvents(everyEvent, 0);	
	return ModuleError;
}

	// Random functions yield a number between min and max. The function
	// originated from Think Reference, but this version is an adaptation
	// by Joseph "Peek-a-Boo" Judge. 

static long
RangedRdm (long min, long max)
{
	unsigned	qdRdm;
	long	range, t;
	
	if (max < min)
		RedAlert("\pRangedRdm: max is smaller than min!");
		
	qdRdm = Random();
	range = (max - min) + 1;
	t = ((long)qdRdm * range) / 65536; 	// now 0 <= t <= range 
	return( t+min );
}


	// This function tries to allocate a handle from temporary memory
	// first, and only if that fails from the reserved memory.
	// This function is taken from the DarkSide of the Mac example code.
	// Note that the return value can be nil and should be checked.
static Handle	
BestNewHandle (Size s)
{
	Handle theHandle;
	OSErr  anErr;
	
	if ((theHandle = TempNewHandle(s, &anErr)) == nil)
		theHandle = NewHandle(s);
		
	return(theHandle);
}

	// Whew, finally a support function I wrote all by myself!
	// This next function is like the ToolBox call PtInRect, but works directly
	// with two x/y values instead of with a Point structure.
	
static Boolean
XYInRect (Rect *r, short beeX, short beeY)
{
	return (beeX > r->left && beeX < r->right && beeY > r->top && beeY < r->bottom);
}


	// Draws a string at specified coordinates. Duh.
static void
DrawStringAt (short x, short y, Str255 str)
{
	MoveTo(x, y);
	DrawString(str);
}


	// Slightly more interesting: draws a string at the given
	// y-coordinate, centered horizontally in the current port.
static void
CenterString (short y, StringPtr str)
{
	GrafPtr port;
	short left, right, width;
	
	GetPort(&port);
	width = TextWidth(str, 1, *str);
	left  = port->portRect.left;
	right = port->portRect.right;

	DrawStringAt(left+(((right-left)-width)/2), y, str);
}


	// Adjusts a rectangle so that it is centered horizontally across
	// the port, with the vertical mid-line at the given y-coordinate.
static void
CenterRectHorizontal (Rect *r, short y)
{
	GrafPtr port;
	short left, right, width, height;
	
	GetPort(&port);
	width = r->right - r->left;
	height = r->bottom - r->top;
	left  = port->portRect.left;
	right = port->portRect.right;

	r->left = left+(((right-left)-width)/2);
	r->right = r->left + width;
	r->top = y - height/2;
	r->bottom = r->top + height;
}


	// Adjusts a rectangle so that it is centered vertically across
	// the port, with the horizontal mid-line at the given x-coordinate.
static void
CenterRectVertical (Rect *r, short x)
{
	GrafPtr port;
	short top, bottom, width, height;
	
	GetPort(&port);

	width = r->right - r->left;
	height = r->bottom - r->top;
	top  = port->portRect.top;
	bottom = port->portRect.bottom;

	r->top = top+(((bottom-top)-height)/2);
	r->bottom = r->top + height;
	r->left = x - width/2;
	r->right = r->left + width;
}


	// Adjusts a rectangle so that it is centered horizontally and vertically 
	// around the given x and y coordinates.
static void
CenterRect (Rect *r, short x, short y)
{
	GrafPtr port;
	short top, bottom, left, right, width, height;
	
	GetPort(&port);
	width = r->right - r->left;
	height = r->bottom - r->top;
	left  = port->portRect.left;
	right = port->portRect.right;
	top  = port->portRect.top;
	bottom = port->portRect.bottom;

	r->left = x - width/2;
	r->right = r->left + width;
	r->top = y - height/2;
	r->bottom = r->top + height;
}


	// I am a very, very, lazy person.
static void
LineFromTo (short xFrom, short yFrom, short xTo, short yTo)
{
	MoveTo(xFrom, yFrom);
	LineTo(xTo, yTo);
}


	// Returns a random true/false value.
static Boolean
RandomBool (void)
{
	return (RangedRdm(0,1) == 1);
}


// The following code was stolen from the NewsWatcher source code.
/*----------------------------------------------------------------------------
	SetPortTextStyle 
	
	Set the font, size, and style of the current port.
			
	Entry:	*style = text style record.
----------------------------------------------------------------------------*/

static void SetPortTextStyle (TextStyle *style)
{
	TextFont(style->tsFont);
	TextFace(style->tsFace);
	TextSize(style->tsSize);
}



/*----------------------------------------------------------------------------
	GetPortTextStyle 
	
	Get the font, size, and style of the current port.
			
	Exit:	*style = text style record.
----------------------------------------------------------------------------*/

static void GetPortTextStyle (TextStyle *style)
{
	style->tsFont = qd.thePort->txFont;
	style->tsFace = qd.thePort->txFace;
	style->tsSize = qd.thePort->txSize;
}


	// This function stolen from the Glypha III source code.
static void RedAlert (StringPtr theStr)
{
	#define		kRedAlertID		128
	short		whoCares;
	
	ParamText(theStr, "\p", "\p", "\p");
	whoCares = StopAlert(kRedAlertID, 0L);
}



	// This is THE END.
	// If you've learned anything from this code, or found errors in it, or
	// have questions about it, or whatever: feel free to drop me a note.
	// My e-mail address is: leo@cp.tn.tudelft.nl

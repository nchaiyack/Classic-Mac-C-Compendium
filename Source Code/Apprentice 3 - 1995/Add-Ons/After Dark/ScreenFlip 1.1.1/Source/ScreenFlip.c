/*
 *    ScreenFlip 1.1.1 � a screensaver module by Leo Breebaart, Kronto Software 1994-95.
 *
 *	  For non-technical information about this module and for the credits,
 *    see the README file.
 * 	  
 * 	  This module keeps flipping your screen�s contents horiontally and vertically. 
 * 	  The flips can be instantaneous or �animated�, i.e. with a column- or row-wise
 *	  ripple effect.
 * 	  
 * 	  I have commented the code in a way that experienced programmers may find
 * 	  overkill, but it was done in the hope that beginning After Dark programmers
 * 	  will find ScreenFlip a useful starting point for writing their own 
 * 	  modules. If you want to look at some more documented code, check out The Swarm, 
 *	  another freeware screensaver module I wrote.
 * 	  
 * 	  For general information about how to write an After Dark-compatible module, 
 * 	  see the After Dark Programmer�s Manual, but make sure you have the most recent
 * 	  version (released when After Dark 3.0 was released). The comments in this
 *    module do assume you are at least *aware* of the basic After Dark mechanisms.
 *
 *	  Here we go...
 */
 
 	// GWorlds.
#include <QDoffscreen.h>

	// After Dark.
#include "AfterDarkTypes.h"


	// Flipping the screen is implemented as a kind of rudimentary state machine.
	// At any time the module is in one of four flip states. kHorizontal and
	// kVertical mean we're in the processing of doing a horizontal resp. vertical
	// screen flip (duh). kSwitch means we are going to determine which direction
	// the next flip will be in. kNone means we are in a state of rest
	// between flips. kSwitch always comes at the end of a kNone period.
typedef enum 
{ 
	kHorizontal, 
	kVertical, 
	kSwitch, 
	kNone 
} 
TFlipStates;


	// This is a secondary state, which controls the direction the ripples of
	// the current animation are moving in (for the kVertical and kHorizontal flip states).
typedef enum 
{ 
	kInwards, 
	kOutwards
}
TDirectionStates;


// In the following data structure most two-element arrays store information
// pertaining horizontal flips in the first element, and the same information
// for the vertical case in the second element. This makes it possible to
// write things like 'bufworld[kVertical]' or 'maxStep[kHorizontal]', which is
// not only very clear, but also allows us to eliminate a lot of code duplication.

typedef struct
{		        
	GWorldPtr bufWorld[2];		// Small buffers for storing a row resp. column of pixels.
	GWorldPtr offWorld;			// Large buffer for storing a copy of the entire screen.
	
	PixMapHandle screenMap;		// Bitmap for an entire screen.
	PixMapHandle realMap;		// Bitmap for the physical screen.
	PixMapHandle bufMap[2];		// Bitmaps associated with the bufWorld array.
	
	Rect strip[2];				// Which two columns/rows to swap this step.
	Rect bufRect[2];			// Bounding rectangles associated with bufWorld.
	Rect r;						// Bounding rectangle of the screen.
	
	TFlipStates movement;		// Current flip state.
	TDirectionStates direction;	// Current ripple movement.
	int t;						// The current step (each step, two columns/rows get switched).

	long startTick, delay;		// Variables for keeping track of delays between flips.
	int maxStep[2];				// After this many steps before flip is finished.

    Boolean demoMode;			// Are we currently in After Dark demo mode?
    Boolean instantFlip;		// Corresponds to check box in module interface.
}
TFlipData, *TFlipDataPtr;


	// After Dark's way of showing an error to the user.
	// '##' is an ANSI-ism meaning meta-concatenation.
#define ErrorMsg(m) BlockMove(CtoPstr(m), params->errorMessage, 1 + m##[0]);

	// QuickDraw color constants used throughout the program.
RGBColor gWhite = { 0xFFFF, 0xFFFF, 0xFFFF };
RGBColor gBlack = { 0, 0, 0 };
RGBColor gGold  = { 0xFFFF, 0x9999, 0x0000 };

	// This array is for mapping delay values received from the AD control panel
	// to values in ticks (60 ticks == 1 second).
static short gDelayMap[6] = { 0, 1*60, 5*60, 10*60, 30*60, 60*60 };


	// Let's be good ANSI citizens, and define some function prototypes.
	
	// These functions will be called from the AfterDarkShell.c code. 
OSErr DoInitialize (Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoHelp (RgnHandle blankRgn, GMParamBlockPtr params);

	// Local functions.
static OSErr AboutBoxError (void);
static int RangedRdm (int min, int max);
static Handle BestNewHandle (Size s);
static void SetRects (TFlipDataPtr fs, int movement);
static void	SwapStrips (TFlipDataPtr fs, int movement);
static OSErr InitStructures (TFlipDataPtr fs, int movement);
static void DrawStringAt (short x, short y, Str255 str);
static void CenterString (short y, StringPtr str);
static void CenterRectHorizontal (Rect *r, short y);


	// DoInitialize allocates the Flip data structure as defined
	// above, initializes the variables in that structure, and checks
	// for possible problems.

OSErr
DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params)
{
	TFlipDataPtr fs;		// The flip structure, obviously.
	RgnHandle tmpRgn = NewRgn();
	GrafPtr screenPort;

     	// Our offscreen graphics worlds need Color QuickDraw.
	if (!params->colorQDAvail)
	{
		DisposHandle(*storage);
		ErrorMsg("ScreenFlip:  Sorry, I need Color QuickDraw to run!");
		return ModuleError;
	}

    	// Allocate 'master' handle to the storage struct.
    if ((*storage = BestNewHandle(sizeof(TFlipData))) == nil)
    { 	
    	ErrorMsg("ScreenFlip:  Couldn't allocate enough memory!");
    	return ModuleError;
    }

    	// Lock down the storage so we can refer to it by pointer. 
    HLockHi(*storage);
    fs = (TFlipDataPtr) **storage;
    
    	// Get the delay-between-flips value from the slider in the module interface
    	// and the instantFlip boolean from the checkbox.
    fs->delay = params->controlValues[0];
	fs->instantFlip = params->controlValues[1];

		// Sanity check. This should never happen.
	if (fs->delay < 0 || fs->delay > 100)
	{
			ErrorMsg("ScreenFlip:  Internal Error � insane delay value!");
			return ModuleError;
    }

		// Initialize the random number generator.
	params->qdGlobalsCopy->qdRandSeed = TickCount();
	fs->demoMode = !EmptyRect((&params->demoRect));

		// Initialize the globals variables that describe the
		// real, 'physical' screen.
	GetPort(&screenPort);
	fs->realMap = GetGWorldPixMap((GWorldPtr) screenPort);

		// The CopyBits function expect these back- and foreground values.
	RGBBackColor(&gWhite);
	RGBForeColor(&gBlack);

		// Find the screen's bounding rectangle, as well as the coordinates 
		// of a single column and row.
    fs->r = params->monitors->monitorList[0].bounds;	
	SetRect(&(fs->bufRect[kVertical]), 0, 0, 1, fs->r.bottom);
	SetRect(&(fs->bufRect[kHorizontal]), 0, 0, fs->r.right, 1);

		// Initialize data structures for row/column buffers.
	(void) InitStructures(fs, kVertical);
	(void) InitStructures(fs, kHorizontal);
	
	fs->maxStep[kVertical] = fs->r.right / 2;
	fs->maxStep[kHorizontal] = fs->r.bottom / 2;
		
		// We start our flipping in a random direction, moving either
		// inwards or outwards, at time (or step) t = 0.	
	fs->movement  = RangedRdm(0,1);
	fs->direction = RangedRdm(0,1);
	fs->t = 0;
	
		// If instantFlip is true, we do not show the animation 'ripples' on the
		// screen (caused by successive swaps of pairs of rows/columns), but instead
		// perform the swapping in an offscreen buffer copy of the entire screen, 
		// which we blit onto the 'real' screen when the flip is completed.
	if (fs->instantFlip)
	{
		if (NewGWorld(&(fs->offWorld), 0, &(fs->r), nil, nil, noNewDevice+useTempMem) != noErr)  	
		{ 	
			DoClose(*storage, (RgnHandle) nil, (GMParamBlockPtr) nil);
			ErrorMsg("ScreenFlip:  Not enough memory for offscreen graphics world!");
			return ModuleError;			
		}
		fs->screenMap = GetGWorldPixMap((GWorldPtr) fs->offWorld);
		
			// Copy the real screen contents to the offscreen pixmap.
		CopyBits((BitMap *) (*fs->realMap), (BitMap *) (*fs->screenMap), 
				  &fs->r, &fs->r,
				  srcCopy, nil);
	}
	else
			// If we do want ripple animation, the algorithm stays exactly the
			// same, only we make screenMap refer to the physical screen, so that
			// any changes to it will be reflected on the monitor immediately,
			// thus causing the ripples to appear.
		fs->screenMap = fs->realMap;
			
	return noErr;
}


	// Initialize small offscreen buffers for storing a column or row.

static OSErr
InitStructures (TFlipDataPtr fs, int movement)
{
 	if (NewGWorld(&fs->bufWorld[movement], 0, &fs->bufRect[movement], nil, nil, noNewDevice+useTempMem) != noErr)  	
		return ModuleError;

	fs->bufMap[movement] = GetGWorldPixMap(fs->bufWorld[movement]);
	
	return noErr;
}


	// The DoBlank function blanks out all available screens, *except* for the
	// main screen. If we are in 'instantFlip' mode, care should be taken to
	// switch to the 'real' screen world (and back to offscreen afterwards) before blanking.

OSErr
DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params)
{
	RgnHandle otherScreens, mainScreenRgn;
	Rect r = params->monitors->monitorList[0].bounds;	
	
	TFlipDataPtr fs = (TFlipDataPtr) *storage;
	
		// If just one monitor, then we don't need to bother at all...
	if (params->monitors->monitorCount != 1)
	{		
		otherScreens = NewRgn();
		mainScreenRgn = NewRgn();
		
		RGBBackColor(&gBlack);
	
		SetRectRgn(mainScreenRgn, r.left, r.top, r.right, r.bottom);
		DiffRgn(blankRgn, mainScreenRgn, otherScreens);
		EraseRgn(otherScreens);
	
			// In order for CopyBits calls to work, the destination
			// world's backgroundworld *has* to be white (and the foreground
			// black).
		RGBBackColor(&gWhite);
	
		DisposeRgn(mainScreenRgn);
		DisposeRgn(otherScreens);
	}

    return noErr;
}


	// At any given step 't' of the animation process, this function
	// calculates which two rows or columns are to be swapped, taking
	// into account that the animation can move inwards or outwards.
	
static void
SetRects(TFlipDataPtr fs, int movement)
{
	int step;
	
	if (fs->direction == kInwards)
		step = fs->t;
	else /* kOutwards */
		step = fs->maxStep[movement] - fs->t - 1;
	
	if (movement == kHorizontal)
	{
		SetRect(&fs->strip[0], 0, step, fs->r.right, step+1);
		SetRect(&fs->strip[1], 0, fs->r.bottom-step-1, fs->r.right, fs->r.bottom-step);
	}
	else /* kVertical */
	{
		SetRect(&fs->strip[0], step, 0, step+1, fs->r.bottom);
		SetRect(&fs->strip[1], fs->r.right-step-1, 0, fs->r.right-step, fs->r.bottom);
	}
}

	
	// The true bottleneck of this module. Three CopyBits calls are necessary to
	// swap the two columns or rows calculated in SetRects. If you look closely
	// at the parameters, you'll notice that these calls are just a bitmap-moving
	// way of doing the well-known "temp = a; a = b; b = temp;" variable swap.
	
static void 
SwapStrips(TFlipDataPtr fs, int movement)
{
	CopyBits((BitMap *) (*fs->screenMap), (BitMap *) (*fs->bufMap[movement]), 
			  &fs->strip[0], &fs->bufRect[movement],
			  srcCopy, nil);
	CopyBits((BitMap *) (*fs->screenMap), (BitMap *) (*fs->screenMap), 
	  		  &fs->strip[1], &fs->strip[0],
	  		  srcCopy, nil);
	CopyBits((BitMap *) (*fs->bufMap[movement]), (BitMap *) (*fs->screenMap), 
	  		  &fs->bufRect[movement], &fs->strip[1],
	  		  srcCopy, nil);
}

	// How many ticks have passed since we started counting?
#define TicksPassed (TickCount() - fs->startTick)

	// The main animation routine. Each call to DoDrawFrame causes (when in 
	// an animation state) two rows or columns to exchange places.
OSErr
DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params)
{	
	TFlipDataPtr fs = (TFlipDataPtr) *storage;
	
		// If we are in After Dark demo mode, the user may have changed
		// the slider value, so we must re-read it.
    if (fs->demoMode)
    {
    	if (fs->delay != params->controlValues[0]) 
		    fs->delay = params->controlValues[0];
	}

	switch (fs->movement)
	{			
		case kHorizontal:
		case kVertical:
	
			if (fs->instantFlip)
			{
					// If we are instant-flipping there is no need to swap just
					// one pair of columns/rows for each call of this function.
					// Instead, we can avoid many seconds of delay by flipping
					// the entire screen in one go. This of course at the price
					// of a slightly less responsive attitude towards module
					// interruptions -- we now spend much more time in DoDrawFrame.
					
					// Delay period includes offscreen drawing time, so we start here.
				fs->startTick = TickCount();
				
				while (fs->t < fs->maxStep[fs->movement])
				{
					SetRects(fs, fs->movement);
					SwapStrips(fs, fs->movement);
					fs->t++;
				}
			
					// Blit the completed flip to the real screen.
				CopyBits((BitMap *) (*fs->screenMap), (BitMap *) (*fs->realMap), 
						  &fs->r, &fs->r,
						  srcCopy, nil);
				
					// Start of (possible) extra delay period.
				fs->movement = kNone;
			}
			else
			{
				SetRects(fs, fs->movement);
				SwapStrips(fs, fs->movement);
	
					// Increment time variable and test for stop criteria.
				if (++fs->t >= fs->maxStep[fs->movement])
				{	
						// Start (possible) delay period.
						// In this case the delay period does *not* include on-screen drawing time.
					fs->movement = kNone;
					fs->startTick = TickCount();
				}
			}
			break;

		case kNone:
		
				// Do nothing until user-specified delay has passed.
			if (TicksPassed < gDelayMap[fs->delay / 20])
				return noErr;
			else
				fs->movement = kSwitch;
			break;
				
		case kSwitch:
				// Change to a random new flip type.
			fs->movement = RangedRdm(0,1);
			
				// Animation direction always goes in-out-in-out-in-out...
			if (fs->direction == kInwards)
				fs->direction = kOutwards;
			else
				fs->direction = kInwards;
				
				// Re-initialize time variable.
			fs->t = 0;
			break;
	}
    return noErr;
}


	// ScreenFlip features a funky About Box, which has a miniature logo flip
	// going on inside of it. Creating that animation is mostly straightforward: I
	// just call all the previous functions, i.e. DoInitialize,
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
DoHelp(RgnHandle blankRgn, GMParamBlockPtr params)
{
	TFlipData **miniFlip;
 
	long dummy;
	GrafPtr helpGraf;
	short oldInstantFlip, oldDelay, oldCount;

	Rect oldBounds, r;
	RgnHandle miniBlankRgn = NewRgn();
   	PicHandle helpPict;				
	Rect picRect, helpRect, miniRect;
	short helpRectHeight;
	Boolean runningAD30;
		
		// Get the real screen.
	GetPort(&helpGraf);

	helpRect = helpGraf->portRect;
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
	
	RGBBackColor(&gBlack);
	EraseRect(&helpRect);
	r = helpRect;
	
	if (runningAD30)
		InsetRect(&r, 3, 3);
	else
		InsetRect(&r, 2, 2);
		
	RGBForeColor(&gWhite);
	FrameRect(&r);
	
	SetRect(&r, 0, 0, 174, 89);
	CenterRectHorizontal(&r, helpRectHeight/3);
	miniRect = r; 
	if ((helpPict = GetPicture(2000)) == nil) 
	{
		ErrorMsg("ScreenFlip:  Couldn�t load PICT resource for help picture!");
		return AboutBoxError();
	}
	DrawPicture(helpPict, &r);
	ReleaseResource((Handle) helpPict);

	RGBForeColor(&gGold);
	InsetRect(&r, -1, -1);
	FrameRect(&r);

	TextFont(geneva); TextSize(9); TextFace(bold);
	RGBForeColor(&gWhite);
	CenterString(helpRectHeight*2/3, "\pA Screensaver Module");
	CenterString(helpRectHeight*2/3 + 12, "\pby Leo Breebaart");
	
	RGBForeColor(&gGold);	
	CenterString(helpRectHeight*11/12, "\pKronto Software 1995");

		// We now manually change the graphics environment
		// from the entire help area to the subarea where we want the
		// mini animation to take place (look at the about box in action if this
		// is not clear to you). The current Port is moved and made smaller.
		// The Toolbox calls take care of all the nasty details.
	LocalToGlobal(&topLeft(miniRect));
	LocalToGlobal(&botRight(miniRect));
	MovePortTo(miniRect.left, miniRect.top);
	PortSize(174, 89);

		// Save some relevant parameters that the 'real' After Dark
		// animation will need to have restored later on.
	oldCount  = params->monitors->monitorCount;
	oldBounds = params->monitors->monitorList[0].bounds;
	oldDelay = params->controlValues[0];
	oldInstantFlip = params->controlValues[1];

		// Change the parameters temporarily to values appropriate for
		// the miniFlip.
	params->monitors->monitorCount = 1;
	params->monitors->monitorList[0].bounds = helpGraf->portRect;
	params->controlValues[0] = 0;
	params->controlValues[1] = 0;
	
	RectRgn(miniBlankRgn, &helpGraf->portRect);	
	
		// Initialize the miniFlip struct. Note that we have a problem
		// with error management here: I can use ErrorMsg all I want,
		// but After Dark will do nothing with the return value of
		// the DoHelp function, for some reason. So I have implemented
		// a separate (and currently trivial) AboutBoxError() function of 
		// my own to call in case of an error.
	if (DoInitialize((Handle *) &miniFlip, miniBlankRgn, params) != noErr) 
	{
		ErrorMsg("ScreenFlip:  Initialization of miniFlip failed!");
		return AboutBoxError();
	}
		
		// Notice that DoBlank() does not need to be called for this miniFlip.
	
		// Wait for the user to release the mouse button, if necessary.
	while (Button())
		;
		
		// Animate, until the user presses the mouse button.
	while (!Button())
	{
		if (DoDrawFrame((Handle) miniFlip, miniBlankRgn, params) != noErr)
		{
			ErrorMsg("ScreenFlip:  DoDrawFrame of miniFlip failed!");
			return AboutBoxError();
		}

			// MiniFlip animation is too fast for such a small area!		
		Delay(1, &dummy);
	}
	
		// Close it all up.
	DoClose((Handle) miniFlip, miniBlankRgn, params);

		// Restore parameters.
	params->monitors->monitorCount = oldCount;
	params->monitors->monitorList[0].bounds = oldBounds;
	params->controlValues[0] = oldDelay;
	params->controlValues[1] = oldInstantFlip;

	DisposeRgn(miniBlankRgn);
	DisposeHandle((Handle) miniFlip);

 	FlushEvents(everyEvent, 0);	

	return noErr;
}


	// The DoClose function merely disposes of all those handles and 
	// offscreen worlds. Nothing interesting here.

OSErr
DoClose (Handle storage, RgnHandle blankRgn, GMParamBlockPtr params)
{
	TFlipData **fs = (TFlipData **) storage;
    
    if (fs)
    {
		DisposeGWorld((**fs).bufWorld[0]);
		DisposeGWorld((**fs).bufWorld[1]);
		if ((**fs).instantFlip)
			DisposeGWorld((**fs).offWorld);

        DisposeHandle(storage);
    }

    return noErr;
}


	// Random functions yield a number between min and max. The function
	// originated from Think Reference, but this version is an adaptation
	// by Joseph "Peek-a-Boo" Judge. 

static int
RangedRdm (int min, int max)
{
	unsigned	qdRdm;
	long	range, t;
	
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


	// When displaying the animated About Box, normal After Dark error
	// handling doesn't work. One day, I'll implement some decent routine
	// of my own, but for now if anything goes wrong during animation,
	// there will just be an uninformative beep.
	
static OSErr
AboutBoxError (void)
{	
	SysBeep(2);
	return ModuleError;
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
	r->bottom = r->top + height;;
}


// This is THE END.
// If you've learned anything from this code, or found errors in it, or
// have questions about it, or whatever: feel free to drop me a note.
// My e-mail address is: leo@cp.tn.tudelft.nl

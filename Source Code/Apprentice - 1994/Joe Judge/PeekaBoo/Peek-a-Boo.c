// ver 1.1.3 changes - 6 Nov 93
//		- added a check for running under System 7. I think the colorQD check
//		kept a lot of the non-system7 users from crashing ... but since I use
// 		offscreen Gworlds which are (I think?) a System-7ism, I should check that.
// ver 1.1.2 - was a resource change to the 'sysz'
// ver 1.1.1 changes - 3 Oct 93
//		- added Kramer sound to rsrc file
//		- fixed RangedRdm() code and update the controlvalues to match as it is running
// ver 1.1 changes 	-	26 Sept 93
//		- did the GetGWorldPixMap() in the slide_up and down routines
// 		I had forgotten to do that in a case
//		- added a control checkbox for randomness and code
// 		- added a STR#/tVal to put a message underneath the faces menu (in the .rsrc)	

#include <QuickDraw.h>
#include <Memory.h>
#include <Resources.h>

#include <QDoffscreen.h>

#include "GraphicsModule_Types.h"
#include "Sounds.h"

unsigned short RangedRdm( unsigned short min, unsigned short max );

// these are the functs that need defined ...
OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params);

// extra ones
OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params);
OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params);
Boolean HasSystemSeven(void);

#define ONE_KNOCK_SND	256
#define BASE_PICTID		127			// this is 1 less than the starting ID number (128)

#define NUMPICTS		26	

// states for the simple state machine ...
#define STATE_DOWN			0
#define STATE_GOING_UP		1
#define STATE_UP			2
#define STATE_LOOKING		3
#define STATE_GOING_DOWN	4
#define RANDOM_CHECK		5		// new


// this is borrowed from the example code from Bouncing Ball ...
/* some macros to simplify synchronizing to the vertical retrace. */
#define SynchFlag(m) (params->monitors->monitorList[m].synchFlag)
#define SynchVBL(m) synchFlag = &SynchFlag(m); *synchFlag = false; while(!*synchFlag);



typedef struct infostruct {
	Boolean		soundAvailable;			// do we have sound?
	Handle		oneKnockSound;			// one knock sound
	GWorldPtr	gMyOffG;				// offscreen graphics world

// the face stuff
	PicHandle	thePict;				// the face to slide up and down
	PicHandle	eyes, eyesleft, eyesright;	// the eyes to slide left/right
	Point		eyesPt;					// where they eyes are supposed to be drawn
	Handle		faceSound;				// a sound associated with a face
	short		thePictNumber;			// the number of the picture we're using
	Rect		theRect;				// rects for the sliding face
	short		thePictWidth;			// time saving info on rect's width
	short		thePictHeight;			// "	"		"		" 	  height

	SoundInfoHandle	soundInfo;			// AD sound info handle
	Rect		theScreen;				// the monitor's screen rect
	long		nextSoundTick;			// last tick when a the knocking sound was played
	short		state;					// the state we're in
} infostruct, *infostructPtr, **infostructHandle;



// some prototypes ...
void do_some_knocking(infostructPtr info, GMParamBlockPtr params);
void slide_picture_up(infostructPtr info, GMParamBlockPtr params);
void slide_picture_down(infostructPtr info, RgnHandle blankRgn, GMParamBlockPtr params);
void slide_eyes_around(infostructPtr info, GMParamBlockPtr params);

//////////////////////////////////////////////////////////////////////////////////////
// this is the first funct called by AD ... we need to allocate and initialize here
OSErr
DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params) {

// ERROR MESSAGES ....
	StringPtr 		memoryMessage = (StringPtr)"\pmodule: Not enough memory!";
	StringPtr 		loadMessage = (StringPtr)"\pmodule: Could not load pict!";
	StringPtr		offscreenMessage = (StringPtr)"\pmodule: Offscreen Bitmap failed!";
	StringPtr		colorMessage = (StringPtr)"\pmodule: Must have color!";
	StringPtr		menuMessage = (StringPtr)"\pmodule: Failed to get MENU!!";
	StringPtr		systemMessage = (StringPtr)"\pmodule: Must have System 7!";
// the variables ...
	Handle 			h;
	Handle			tempHandle;
	infostructPtr	mystorage;
	GWorldPtr		currPort;		// something to save the port + device
	GDHandle		currDev;
	PixMapHandle	pixBase;
	
	// lets ensure we're running on a screen of some depth ... (yes? no? do we care?)
	// no ... I don't care ...just go on and run and let it look wierd
	// but we do need some color Quickdraw - I haven't put in the B&W offscreen 
	// routines to do offscreen bitmaps 
	if (!params->colorQDAvail) {
		BlockMove(colorMessage, params->errorMessage, 1 + colorMessage[0]);
		return ModuleError;					// could not allocate space
	}
	// are offscreen gworlds a System 7 thing? or a colorqd thing?
	// if it's system 7 ... I need to test!!
	if (!HasSystemSeven()) {
		BlockMove( systemMessage, params->errorMessage, 1 + systemMessage[0] );
		return ModuleError;
	}
	
	// allocate space here for my structure - whatever it may be
	h = NewHandle( sizeof(infostruct) );	
	
	if (h == (Handle)nil) {
		*storage = nil;
		h = nil;
		BlockMove(memoryMessage, params->errorMessage, 1 + memoryMessage[0]);
		return ModuleError;					// could not allocate space
	}
	
	// Randomize...
	params->qdGlobalsCopy->qdRandSeed = TickCount();
	
	// handle pointer to have
	*storage = h;
	
	// lock down our storage so we can refer to it by pointer safely
	MoveHHi(h);
	HLock(h);	
	
	mystorage = (infostructPtr) *h;

///////// load the sounds ...
	// do we have sound support?
	mystorage->soundAvailable = (params->systemConfig & SoundAvailable) != 0;

	if (mystorage->soundAvailable) {
	
		/* load the resources for our  sounds. */
		mystorage->oneKnockSound = GetResource('snd ', ONE_KNOCK_SND);
	
		/* to use the sound functions in AD 2.0u we must pass in "params" */
		mystorage->soundInfo = OpenSound(params);
	}

// check randomness first ...
	if (params->controlValues[1]) {		// We're random
		MenuHandle theMenu;
		short rCount, i;
		
		// how many menu items are there?
		theMenu = (MenuHandle)GetResource('MENU', 1002);
		if (theMenu == (MenuHandle)nil) {
			DebugStr("\pBig problems!");
		}
		rCount = CountMItems( theMenu);						// how many picts?
		// pick one
		params->controlValues[2] = mystorage->thePictNumber = RangedRdm(1, rCount);
		// by setting the controlValues[2] parameter ... the menu changes in demo mode!
		// Neat!
			
		ReleaseResource( (Handle)theMenu);
	} else {
		mystorage->thePictNumber = params->controlValues[2];
	}
		
	mystorage->thePict = 
			(PicHandle)GetResource('PICT', BASE_PICTID + mystorage->thePictNumber);

	if (mystorage->thePict == (PicHandle)nil) {
		DoClose( h, (RgnHandle) nil, (GMParamBlockPtr) nil);
		BlockMove(loadMessage, params->errorMessage, 1 + loadMessage[0]);
		return ModuleError;						// could not load pict
	}
		

////// get it's sound ... if it's possible
		mystorage->faceSound = GetResource('snd ', BASE_PICTID + mystorage->thePictNumber);

////// get the eye pictures ... if it's possible
	mystorage->eyes = (PicHandle)GetResource('PICT', 
			((BASE_PICTID + mystorage->thePictNumber) * 10) + 1);
	mystorage->eyesleft = (PicHandle)GetResource('PICT', 
			((BASE_PICTID + mystorage->thePictNumber) * 10) + 2);
	mystorage->eyesright = (PicHandle)GetResource('PICT', 
			((BASE_PICTID + mystorage->thePictNumber) * 10) + 3);

////// get the eye Point resource ... if it's possible
	if ( (tempHandle = GetResource('eyes', (BASE_PICTID + mystorage->thePictNumber))) != 
			(Handle)nil) {
		PointPtr p;
		
		p = (PointPtr)(*tempHandle);
		mystorage->eyesPt = (*p);
		ReleaseResource( tempHandle);
	} else 
		mystorage->eyesPt.h = mystorage->eyesPt.v = -1;
	
///////
	mystorage->theRect =  (*mystorage->thePict)->picFrame;
	mystorage->thePictWidth = mystorage->theRect.right - mystorage->theRect.left;
	mystorage->thePictHeight = mystorage->theRect.bottom - mystorage->theRect.top;
	
	
//////////////////////////////////////////////////////////////////
// lets setup the offscreen world
	// save the current info
	GetGWorld(&currPort,&currDev);
	
	// create the offscreen world (with the bounds of the picture)
	if (NewGWorld(&(mystorage->gMyOffG), 0, &mystorage->theRect, nil, nil, 0) != noErr) {
		DoClose( h, (RgnHandle) nil, (GMParamBlockPtr) nil);
		BlockMove(offscreenMessage, params->errorMessage, 1 + offscreenMessage[0]);
		return ModuleError;			
	}
	// keep it from moving
	pixBase = GetGWorldPixMap( mystorage->gMyOffG );
	LockPixels (pixBase);
	
	// point to the offscreen world
	SetGWorld ((mystorage->gMyOffG), nil);
	
	// draw it
	DrawPicture( mystorage->thePict, &mystorage->theRect );
	// could release the resource now, you know.

	// done drawing, set the world back 
	SetGWorld (currPort, currDev);
	
	// unlock those puppies
	UnlockPixels (pixBase);
//////////////////////////////////////////////////////////////////

	
	mystorage->theScreen = params->monitors->monitorList[0].bounds;
	mystorage->nextSoundTick = 0;
	
	mystorage->state = STATE_DOWN;

	HUnlock(h);
	
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// the screen saver has been awakened! time to ditch the storage and wave goodbye
OSErr 
DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	
	infostructPtr	mystorage;
	
	HLock(storage);
	
	mystorage = (infostructPtr)*storage;
	
	if (mystorage->soundAvailable) {
		CloseSound( mystorage->soundInfo, params->sndChannel);
		
		// knocking sound
		if (mystorage->oneKnockSound != (Handle)nil)
			ReleaseResource(mystorage->oneKnockSound);

		// the face's sound
		if (mystorage->faceSound != (Handle)nil)
			ReleaseResource(mystorage->faceSound);

	}
	
	
	ReleaseResource( (Handle)mystorage->thePict);
	// release the eye picts ... if they're there
	if (mystorage->eyes != (PicHandle)nil)
		ReleaseResource( (Handle)mystorage->eyes);
	if (mystorage->eyesleft != (PicHandle)nil)
		ReleaseResource( (Handle)mystorage->eyesleft);
	if (mystorage->eyesright != (PicHandle)nil)
		ReleaseResource( (Handle)mystorage->eyesright);
		
		
	DisposeGWorld( mystorage->gMyOffG);
	HUnlock(storage);
	DisposHandle( storage);
	return noErr;
}



//////////////////////////////////////////////////////////////////////////////////////
// make the screen go black
OSErr
DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {

	FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);
	return noErr;

}

//////////////////////////////////////////////////////////////////////////////////////
// this is the workhorse routine. It does the continual screen work to make
// this screen saver what it is.
OSErr 
DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	infostructPtr	info;


	HLock(storage);
	info = (infostructPtr)*storage;

	// pay attention to the controls in demo mode ...
	// - the menu (2) is different than the picture number ... someone must
	// have changed the menu! -  so restart the module
	if ( (params->controlValues[2]) != info->thePictNumber) {
		DoClose(storage, blankRgn, params);	// ditch the storage
		return RestartMe;					// tell AD to reinit us
	}

	// when sufficient time has passed ... the face should peep up
	
	switch (info->state) {
	case RANDOM_CHECK:		// check for randomness ... 
			// random is the current option  - so restart the module
			if ( params->controlValues[1])  {	
				DoClose(storage, blankRgn, params);	// ditch the storage
				return RestartMe;					// tell AD to reinit us
			} else info->state = STATE_DOWN;		// not random?, then move on
			break;
	case STATE_DOWN:
			do_some_knocking(info, params);
			break;
	case STATE_GOING_UP:
			slide_picture_up(info, params);		
			break;
	case STATE_UP:
			info->state = STATE_LOOKING;
			break;
	case STATE_LOOKING:
			slide_eyes_around(info, params);
			break;		
	case STATE_GOING_DOWN:
			slide_picture_down(info, blankRgn,  params);
			break;
	default:
		info->state = STATE_DOWN; // this should not happen
		break;
	}
	HUnlock(storage);
	return noErr;
}
//////////////////////////////////////////////////////////////////////////////////////
// this is called when they click on something in the control panel
OSErr 
DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params) {

	return noErr;
}


void
slide_eyes_around(infostructPtr info, GMParamBlockPtr params)
{

	// if there are eyes, eyesleft, eyesright, an eyes point *AND* a random chance
	// then go ahead and play with the eyes
	if (info->eyes != (PicHandle)nil &&
			info->eyesleft != (PicHandle)nil &&
				info->eyesright != (PicHandle)nil &&
					info->eyesPt.h != -1 &&
					info->eyesPt.v != -1 &&
					(!(Random()%10)) ) {
		Rect r;
		short tempnum;
		
		r = (*info->eyes)->picFrame;
		// what if the picFrame is not 0,0 origined? 

		// offset to where the pixmap's rect actually is on screen
		OffsetRect( &r, info->theRect.left, info->theRect.top);
		
		// offset to where the eyes resource says it is
		OffsetRect( &r, info->eyesPt.h, info->eyesPt.v); 
		
		// randomly choose which eye pict to display
		tempnum = RangedRdm(0, 4);		// was 5 ... I like things a bit more abnormal
		switch(tempnum) {
			case 0:  DrawPicture( info->eyesleft, &r); break;
			case 1:  DrawPicture( info->eyesright, &r); break;
			// case 2, 3, 4, 5 ... all do normal eye things in the default
			default: DrawPicture( info->eyes, &r); break;
		}
	}
	
	// make some noise, also?
	if (!SoundBusy(info->soundInfo, params->sndChannel) && 
			(info->faceSound != (Handle)nil)  &&
			 (!(Random()%100)) ) {
			
			PlaySound( info->soundInfo, params->sndChannel, info->faceSound);
	}


	// random chance to stop all this peering about and start sliding down
	if (!(Random()%300) ) info->state = STATE_GOING_DOWN;
	
}


void
slide_picture_up(infostructPtr info, GMParamBlockPtr params)  {
	Boolean *synchFlag;		/* pointer to speed up access to synch */
	short DELTA;
	GWorldPtr	worldPtr;		// temp ptr to help address stuff
	GWorldPtr	currPort;		// something to save the port + device
	GDHandle	currDev;
			
	// amount of pixels to slide is 1/5 the slider value ( {0..100} => {0..20} )
	DELTA = 1 + params->controlValues[0] / 5;
	
	// if this is the first time, then setup theRect (where to draw on the screen)
	if (info->theRect.top == 0 && info->theRect.left == 0) {
		short leftrand;
		// random horizontal offset on the screen so we don't always slide in 
		// up from the same place
		leftrand = RangedRdm( 0, (info->theScreen.right - info->theScreen.left)/2);
		OffsetRect( &(info->theRect), 
				(info->theScreen).left + leftrand,  (info->theScreen).bottom);
	}
	
	// slide them up a DELTA amount
	OffsetRect( &(info->theRect), 0, -DELTA);
	
	// if they've bumped the top, or moved up so far that their bottom is above
	// the screen's bottom, 
	// or if they've moved so the eyes show and random chance
	// it's time to stop moving up all together
	
	if ( (info->theRect.top <= info->theScreen.top) ||
			(info->theRect.bottom <= info->theScreen.bottom) ||
				// if we're up past where the eyes show ... then randomly stop here
				(((info->theRect.top + info->eyesPt.v +
					((*info->eyes)->picFrame.bottom - (*info->eyes)->picFrame.top ))
					< info->theScreen.bottom) && 
					(info->eyesPt.v != -1) &&
					!(Random()%20))
			) 
	{
		OffsetRect( &(info->theRect), 0, DELTA);	// move it back down a bit
		info->state = STATE_UP;						// we're all the way up now
	} else {
		PixMapHandle	pixBase, screenBase;

		worldPtr = info->gMyOffG;
		GetGWorld(&currPort,&currDev);
		
		ForeColor (blackColor);
		BackColor (whiteColor);
		
		pixBase = GetGWorldPixMap( worldPtr );
		screenBase = GetGWorldPixMap( (GWorldPtr)currPort);
		LockPixels ( pixBase );
		LockPixels ( screenBase);
		
		// wait for vertical retrace
		SynchVBL(0);
		// blit it into place onscreen
		CopyBits ( (BitMap *) (*pixBase),
					&((GrafPtr)currPort)->portBits, 
																	//&info->theRect, 
					&(worldPtr->portRect),
					&info->theRect, 
					srcCopy, nil);
		UnlockPixels(pixBase);
		UnlockPixels(screenBase);
	}
}

void
slide_picture_down(infostructPtr info, RgnHandle blankRgn, GMParamBlockPtr params)  {
	Boolean *synchFlag;		/* pointer to speed up access to synch */
	Rect diffRect;
	short DELTA;
	GWorldPtr	worldPtr;		// temp ptr to help address stuff
	GWorldPtr	currPort;		// something to save the port + device
	GDHandle	currDev;
	
	DELTA = 1 + params->controlValues[0] / 5;
	
	OffsetRect( &(info->theRect), 0, DELTA);		// slide downward
	
	// if we've slid so far down that our top leaves the screen, we're done going down
	if ( info->theRect.top >= info->theScreen.bottom) {
		// new change ... time to do RANDOM check
		//info->state = STATE_DOWN;	
		info->state = RANDOM_CHECK;		// we've slid off the screen - random?
		
		// I'm anal retentive, so let's just make sure the screen if cleared...
		FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);
		// set the rect to 0,0 offset so we recognize the initial condition in slide_up
		SetRect( &(info->theRect), 0, 0, info->thePictWidth, info->thePictHeight);
	} else {
		PixMapHandle	pixBase, screenBase;
		
		worldPtr = info->gMyOffG;
		GetGWorld(&currPort,&currDev);
	
		ForeColor (blackColor);
		BackColor (whiteColor);
		
		pixBase = GetGWorldPixMap( worldPtr);
		screenBase = GetGWorldPixMap( (GWorldPtr)currPort);
		LockPixels ( pixBase );
		LockPixels ( screenBase);
		
		// wait for vertical retrace
		SynchVBL(0);

		CopyBits ( (BitMap *) (*(worldPtr->portPixMap)),
					&((GrafPtr)currPort)->portBits, 
					&(worldPtr->portRect),
					&info->theRect, 
					srcCopy, nil);
					
		UnlockPixels(pixBase);
		UnlockPixels(screenBase);
	}
}


// face is offscreen ... so it knocks on the glass a bit.
void
do_some_knocking(infostructPtr temp, GMParamBlockPtr params) {

	
	
	// if it is time to make noise again ...
	if (TickCount() >= temp->nextSoundTick) {
		// if it's not busy and we have sound and we have a sound resource, then do it
		if (!SoundBusy(temp->soundInfo, params->sndChannel) && 
				(temp->oneKnockSound != (Handle)nil) )
		{
			short i, knocks;
			long tempticks;
			
			// random number of knocks
			knocks = RangedRdm(2, 6);
			for (i=0; i<=knocks; i++) {
				PlaySound( temp->soundInfo, params->sndChannel, temp->oneKnockSound);
				// delay a random amount to give it some "human" like feel
				Delay( 	RangedRdm(7, 12), &tempticks);
				
			}
			// lets not make sound until some amount later {5..15} seconds
			temp->nextSoundTick = TickCount() + RangedRdm(300, 900);
		}
	} 
	
	// random chance to just stop all this knocking and start the show
	if (!(Random()%200)) {
		temp->nextSoundTick = 0;
		temp->state = STATE_GOING_UP;
	}
}



OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params)
{
	// I tried playing with params here and they don't seem instantiated,
	// so don't play too much in this routine
	return noErr;
}


// this is from the Think C reference code example ...
unsigned short RangedRdm( unsigned short min, unsigned short max )
/* assume that min is less than max */
{
	// uh ... not this isn't quite right - it's between 0 and 65535, not 65536
	unsigned	qdRdm;	/* treat return value as 0-65536 */
	long	range, t;
	
	// just to be safe, I'll put this here
	if (min > max) DebugStr("\pMin greater then Max in RangedRdm");
	
	qdRdm = Random();
	range = max - min;
	// max - min gives us the the difference between max and min ... that is 
	// not inclusive. It gives us { min <= range < max }
	// so we never see that max number!!
	range++;
	t = ((long)qdRdm * range) / 65536; 	/* now 0 <= t <= range */
	return( t+min );
}



OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params)
{
	return noErr;
}



#include <GestaltEqu.h>
static Boolean
HasSystemSeven( void )

{
	long	gestResponse;
	short	version;

	if (!Gestalt( gestaltVersion, &gestResponse )) {	
	
		// � Ensure that machine is running System 7 or higher. 
	
		Gestalt( gestaltSystemVersion, &gestResponse );
		version = HiWord( gestResponse );
		if (gestResponse >= 7) {			
			return TRUE;
		}

	}  
	
	return FALSE;

}

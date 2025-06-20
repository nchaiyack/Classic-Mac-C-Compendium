/*
	Boom!  A quick Hack to Show off PPC graphics (68k version)

	@MacHack94	 bh 	 noEsis Software Construction 
*/

/*
	Last Build: 
					MWerks D/R 4, 68k v1.1

	DOCS:
	
	This is an INIT installing a patch to _TrackGoAway for both
	68k and PowerPC.  At patch execution time, if _TrackGoAway 
	returns a TRUE, we create a nifty animation for the people...
	
	I only call the animation if one of Shift, Option, Caps-Lock,
	etc.. keys are down.  That way the Finder isn't constantly
	bogged down with demo booms (!)  Of course, since this is a 
	patch, the effect works in any application with std Macintosh
	windows.
	
	The animation is a bona-fide simulation in floating point.
	To do the animation we use a number of offscreen GWorlds, 
	and create an array of animation elements for a particular
	scene. The array looks like this: 
	
	globals ->  ...
				gBits ->	b[0]	theta (angle)
									speed
									curPos
									curDispSize
									GWorld		-> *Gworld -> bits...

				gBits ->	b[1]	theta (angle)
									speed
									curPos
									curDispSize
									GWorld		-> *Gworld -> bits...

				gBits ->	b[n]	theta (angle)
									speed
									curPos
									curDispSize
									GWorld		-> *Gworld -> bits...


	All of the simulation constants are set by compile time defines
	listed below. There are currently 5 to 11 'bits' in the explosion.
	The whole thing is init'd, run and cleaned up on each invocation
	(except for the SndChannel in the System Heap).  You can safely
	swap monitor depths between invocations, no problem...
	
	There were even grander plans for this 
	code, but the clock ran out, so to speak, so this is what you
	get.  I have found this patch to very stable, and should not 
	conflict at all (who else wants possesion of _TrackGoAway ?!?)
	
	Spread the code and INIT far and wide, as long as all notices 
	remain intact.  Any modifications, or copies of the binary built
	from modifications, must be marked clearly as such.  Enjoy!
	
	28sep94  -bh
*/

//****************************************************************
//												I N C L U D E S

#include <Types.h>
#include <Memory.h>
#include <A4Stuff.h>
#include <SetUpA4.h>
#include <Quickdraw.h>
#include <QDOffscreen.h>
#include <LowMem.h>
#include <Errors.h>
#include <GestaltEqu.h>
#include <Resources.h>
#include <Events.h>
#include <Windows.h>
#include <Sound.h>
#include <Traps.h>

#include <MixedMode.h>

#include <fp.h>


//****************************************************************
//												C O N S T A N T S
#define FALSE				false
#define TRUE				true
#define NIL					0L

 
//******************************
//	The 68k code goes in a normal INIT resource.
//	Be sure this is set to "system heap/locked".

#define kInitRezType		'INIT'
#define kInitRezID			300

#define kMinSystemVersion	(0x0603)



//******************************
// from Windows.h:
// pascal Boolean TrackGoAway( WindowPtr theWindow, Point thePt)
//   ONEWORDINLINE(0xA91E);

enum {

	kTrackGoAwayInfo = kCStackBased 
							| RESULT_SIZE(SIZE_CODE(sizeof(Boolean))) 
							| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowPtr)))
							| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Point)))
};


typedef pascal Boolean ( *TrackGoAwayFuncPtr ) ( WindowPtr theWindow, Point thePt );
typedef UniversalProcPtr UPP;


// ShowInit 
#define kOkOldMac	128
#define kOkPPC		130
#define kNotOldMac	132
#define kNotPPC		134

// explosion constants
struct exVector {
	short			theta;		// 0-360
	short			speed;		// small integer
	Point			pPos;		// current position
	Rect			pDispR;		// current drawn size of 'bit
	GWorldPtr		tPixGwP;	// map  0,0,kSrcPixSiz,kSrcPixSiz
};
typedef struct exVector exVector;

#define kModBits 6
#define kFrameCnt 20
#define kModSpeed 12
#define kSrcPixSiz {0,0,3,3}
#define kScratchSize 265


//****************************************************************
//												G L O B A L S

struct NInitGlobals
{
	UPP				gOrigTrackGoAway;	// old Addr
	SysEnvRec		gSystemInfo;
	long			gInfo;		// 1=68k,2=PPC

	Handle			gSndHandle;
	SndChannelPtr	gSndChP;
	exVector		*gBits;
	short 			gNumBits;
	short 			gFrameCnt;
	GWorldPtr		gPad;			// working image
	GWorldPtr		gSavPad;		// pristine screen copy
};
typedef struct NInitGlobals NInitGlobals;

NInitGlobals				*gP;


//****************************************************************
//												F O R W A R D S

pascal Boolean nTrackGoAway68k( WindowPtr theWindow, Point thePt);
OSErr DoInitForOldMacs( void);

void PlayBoom(void);
OSErr SetUpSim(void);
void Animate(Point cPt);
void AnimateDone(void);
void ImageFrame(short frameNum, GWorldPtr dstGW, Point exPt);
void PlotImage( GWorldPtr gw, short theta, short speed);

extern void ShowIconFamily(short iconId);


//****************************************************************
void main( void )
{
	long		oldA4, lRes;
	Handle		initH = nil;		/* Handle to our own INIT resource */
	OSErr		err = noErr;
	
	oldA4 = SetCurrentA4();			/* Get the proper value of A4 into A4 */
	RememberA4();					/* save into self-modifying code */


	// Allocate globals struct
	gP = (NInitGlobals*) NewPtrSysClear( sizeof(NInitGlobals));
	if ( !gP ) {
		err = memFullErr;
		goto DONE;
	}
	
 	// Get minimal System Info:
	err = SysEnvirons( 1, &gP->gSystemInfo );
	if ( err )
		goto DONE;
  	if ( gP->gSystemInfo.systemVersion < kMinSystemVersion ) {
 		err = -1;
 		goto DONE;
 	}

	// Prepare to Detach ourselves...
	initH = Get1Resource( kInitRezType, kInitRezID );
	if ( !initH ) {
		err = resNotFound;
		goto DONE;
	}

	// Call Gestalt:
	// Gestalt will return an err when SysArc is unimplemented
	//  we know we are 68k if so  (thanks to Richard Clark)
	err = Gestalt( gestaltSysArchitecture, &gP->gInfo );
	if ( err) gP->gInfo = gestalt68k;

	// Load 'snd ':
	// this snd is marked 'sysHeap', so it'll land in the right place,
	// and we have to detach anything we want to keep around
	gP->gSndHandle = GetResource ('snd ', 128);
	DetachResource( gP->gSndHandle);
	
	{ // Make a new SndChannel in the System Heap
	THz savZone = GetZone();
	SetZone( SystemZone());
	err = SndNewChannel( &gP->gSndChP, sampledSynth, initMono, 0);
	if ( err != noErr) goto DONE;
	SetZone( savZone);
	}
	
	err = DoInitForOldMacs();	// sorry, 68k setup only


	DONE:
	if ( err ) {
		ShowIconFamily( kNotOldMac);
		//ShowIconFamily((gP->gInfo==gestalt68k)?(kNotOldMac):(kNotPPC));
		if ( gP )
			DisposPtr( (Ptr)gP );
	} else {
		ShowIconFamily( kOkOldMac);
		//ShowIconFamily((gP->gInfo==gestalt68k)?(kOkOldMac):(kOkPPC));
		DetachResource( initH);	
		// resource is already marked 'locked', so no HLock() here...
	}
	
	RestoreA4( oldA4 );					/* restore previous value of A4 */
}



//****************************************************************
//	DoInitForOldMacs
//****************************************************************
OSErr DoInitForOldMacs( void )
{
	long	oldA4;
	NInitGlobals *locGPtr;

	oldA4 = SetUpA4();	
	locGPtr = gP;
	RestoreA4( oldA4 );	

	locGPtr->gOrigTrackGoAway = NGetTrapAddress( _TrackGoAway, ToolTrap );
	NSetTrapAddress( (UPP)nTrackGoAway68k, _TrackGoAway, ToolTrap );
	
	return noErr;
}


//****************************************************************
pascal Boolean nTrackGoAway68k( WindowPtr theWindow, Point thePt )
{
	Boolean		res;
	long		oldA4;
	GrafPtr		tPort;
	Point		savPt;
	TrackGoAwayFuncPtr ptr;
	NInitGlobals *locGPtr;

	oldA4 = SetUpA4();		
	locGPtr = gP;
	
	ptr = (TrackGoAwayFuncPtr)locGPtr->gOrigTrackGoAway;
	savPt = thePt;
	res = ( *ptr )( theWindow, thePt );
	
	if ( res == true) { 

		if ( *(short*)0x17A) {
			THz savZone = GetZone();
			SetZone( SystemZone());
			
			if ( SetUpSim() == noErr) {
				HideCursor();
				Animate( savPt);
				AnimateDone();
				ShowCursor();
			}
			SetZone( savZone);
		}	
	}

	RestoreA4( oldA4 );	
	return res;
} 


//***********************************************************
void AnimateDone()
{
	short cnt;

	DisposeGWorld( gP->gPad);
	DisposeGWorld( gP->gSavPad);
	for ( cnt=0;cnt<gP->gNumBits; cnt++)
		DisposeGWorld( gP->gBits[cnt].tPixGwP);

	DisposPtr( (Ptr)gP->gBits);
}


//***********************
#define abs(x) ((x>0)?(x):(-x))

OSErr SetUpSim(void)
{
	short cnt, prevT;
	Rect r = kSrcPixSiz;
	Rect r2;
	OSErr err;

	// init scratch pads
	SetRect( &r2, 0, 0, kScratchSize, kScratchSize);
	err = NewGWorld( &gP->gPad,0,&r2,0,0,0);	// use 0 for best results
	if ( err != noErr) return err;
	err = NewGWorld( &gP->gSavPad,0,&r2,0,0,0);	// hope we have the RAM !
	if ( err != noErr) return err;

	// allocate 'bits array
	gP->gNumBits = abs(TickCount() % kModBits) + 5;	//ok, max+n

	gP->gBits = (exVector*)NewPtrClear( gP->gNumBits*sizeof(exVector));
	for ( cnt=0;cnt<gP->gNumBits; cnt++){
		gP->gBits[cnt].theta = abs((TickCount()*10+cnt) % 360);
		gP->gBits[cnt].speed = abs(TickCount() % kModSpeed) + 4;
		gP->gBits[cnt].pPos.h = 0;
		gP->gBits[cnt].pPos.v = 0;
		gP->gBits[cnt].pDispR = r;

		err = NewGWorld( &gP->gBits[cnt].tPixGwP,
					8,&r,0,0,0);
		if ( err != noErr) return err;

		// now, based on speed and theta, make an image
		PlotImage( gP->gBits[cnt].tPixGwP, gP->gBits[cnt].theta, gP->gBits[cnt].speed);
	}
	return err;
}


//**************************************************************
//
//	Order of Events: 
//	Prestine PixMap is in gP->gSavPad, working PixMap is in gP->gPad
//	For each cell of animation, copy the savPad to workPad,
//	Draw all 'bits for this frame, copy gP->gPad to the screen.
//
//	Try This: always set the current Port to the _destination_ of
//	a CopyBits() operation..	
				
void Animate( Point cPt)
{
	GWorldPtr	 savGW;
	GDHandle  	savGdH;
	GrafPtr savP; GrafPort dPort;
	Rect wR; long t;
	short offX=0;
	short offY=0;
	Point	exPt;

	PlayBoom();

	// Create a new GrafPort on top of the current port so as to 
	// disturb the graphics world as little as possible. This
	// new GrafPort is the default through our copying operations.
	
	GetPort( &savP);
	OpenPort( &dPort);
	SetPort( &dPort);

	wR = gP->gPad->portRect;	

	// if the point is closer to the side of the screen than
	// half the width of our buffered draw area, then adjust
	// the explosion center

	if ( cPt.h > wR.right/2) {
		offX = cPt.h - wR.right/2;
		exPt.h = wR.right/2;
	} else exPt.h = cPt.h;
	if ( cPt.v > wR.bottom/2) {
		offY = cPt.v - wR.bottom/2;
		exPt.v = wR.bottom/2;
	} else exPt.v = cPt.v;
	OffsetRect( &wR, offX, offY);

	// init savMap 
	GetGWorld( &savGW, &savGdH);	SetGWorld( gP->gSavPad, 0);
	CopyBits(	(BitMap*)(&(dPort.portBits)),
				(BitMap*)(*(gP->gSavPad->portPixMap)), 
				&wR, &gP->gPad->portRect,
				patCopy, 0);
	SetGWorld( savGW, savGdH);
	
	// special effects
	{
		#define kMax 6
		Rect tR, exR; short cnt;
		RGBColor tC = {0x0000,0x0000,0x0000};
		PenMode( blend);	ForeColor( whiteColor);
		exR.left = cPt.h - kScratchSize/2;
		exR.right = cPt.h + kScratchSize/2;
		exR.top = cPt.v - kScratchSize/2;
		exR.bottom = cPt.v + kScratchSize/2;
		for ( cnt=1; cnt<=kMax; cnt++){
			tR = exR;
			tC.red += 0x2A00; tC.green += 0x2A00; tC.blue += 0x2A00;
			OpColor( &tC);
			InsetRect( &tR, cnt*(kScratchSize/kMax), cnt*(kScratchSize/kMax));
			PaintOval( &tR);
			if ( gP->gInfo == gestaltPowerPC)
				Delay( 1, &t);
		}
		PenNormal();	ForeColor( blackColor);
		if ( gP->gInfo == gestaltPowerPC)
			Delay( 2, &t);
		#undef kMax
	}

	// animate
	for (gP->gFrameCnt=0;gP->gFrameCnt<kFrameCnt;gP->gFrameCnt++) {
		// copy savPad to workPad
		GetGWorld( &savGW, &savGdH);	SetGWorld( gP->gPad, 0);
		CopyBits(	(BitMap*)(*(gP->gSavPad->portPixMap)), 
					(BitMap*)(*(gP->gPad->portPixMap)),
					&gP->gSavPad->portRect, &gP->gPad->portRect,
					patCopy, 0);
		SetGWorld( savGW, savGdH);

		ImageFrame(gP->gFrameCnt, gP->gPad, exPt);	// image 'bits

		// copy workPad to screen
		CopyBits(	(BitMap*)(*(gP->gPad->portPixMap)), 
					(BitMap*)(&(dPort.portBits)),
					&gP->gPad->portRect, &wR,
					patCopy, 0);

		if ( gP->gInfo == gestaltPowerPC)
			Delay( gP->gFrameCnt/4, &t);
	}

	// restore screen
	CopyBits(	(BitMap*)(*(gP->gSavPad->portPixMap)),
				(BitMap*)(&(dPort.portBits)), 
				&gP->gPad->portRect, &wR,
				patCopy, 0);

	SetPort( savP);
	ClosePort( &dPort);
	
	return;
}

//***********************************
void ImageFrame( short frameNum, GWorldPtr dstGW, Point exPt) 
{
	GWorldPtr	 savGW;
	GDHandle  	savGdH;
	short	posX, posY;
	short	cnt, f, tSpd;
	long	t;
	Rect	srcR = kSrcPixSiz;
	Rect	dstR;

	// r is frameCnt * speed+frameCnt
	// posX = r*cos(t)
	// posY = r*sin(t)
	
	GetGWorld( &savGW, &savGdH);	SetGWorld( dstGW, 0);
	for (f=frameNum,cnt=0;cnt<gP->gNumBits;cnt++) {
		tSpd = gP->gBits[cnt].speed;
		posX = (short)(f*(tSpd+cnt)) * cos(gP->gBits[cnt].theta);
		posY = (short)(f*(tSpd+cnt)) * sin(gP->gBits[cnt].theta);

		dstR = gP->gBits[cnt].pDispR;
 
		dstR.left += exPt.h+posX;
		dstR.right += exPt.h+posX;
		dstR.top += exPt.v+posY;
		dstR.bottom += exPt.v+posY;

		CopyBits(	(BitMap*)(*(gP->gBits[cnt].tPixGwP->portPixMap)), 
					(BitMap*)(*(dstGW->portPixMap)),
					&srcR, &dstR,
					patCopy, 0);
	}
	SetGWorld( savGW, savGdH);
}

//**********************
// Create one explosion 'bit
void PlotImage( GWorldPtr gw, short theta, short speed)
{
	GWorldPtr	 savGW;
	GDHandle  	savGdH;
	Rect		r = kSrcPixSiz;
	Point		pR1a, pR1b, pR1c;
	Point		pR2, pO1, pY1a, pY1b;
	RGBColor	c;

	GetGWorld( &savGW, &savGdH);
	SetGWorld( gw, 0);

	c.red = 0xFFFF; c.green = 0xFFFF; c.blue = 0xFFFF;
	RGBForeColor( &c);
	PaintRect( &r);
	PenNormal();

	if ( theta <= 22) {												// 22
		pR1a.h=1; pR1a.v=2; pR1b.h=2; pR1b.v=1; pR1c.h=1; pR1c.v=0;
		pR2.h=2; pR2.v=0; pO1.h=2; pO1.v=2; 
		pY1a.h=0; pY1a.v=2; pY1b.h=0; pY1b.v=0;
	} else if ( theta <= 77)  {										// 77
		pR1a.h=2; pR1a.v=2; pR1b.h=2; pR1b.v=0; pR1c.h=0; pR1c.v=0;
		pR2.h=1; pR2.v=0; pO1.h=2; pO1.v=1; 
		pY1a.h=1; pY1a.v=2; pY1b.h=0; pY1b.v=1;
	} else if ( theta <= 112)  {									// 112
		pR1a.h=2; pR1a.v=1; pR1b.h=1; pR1b.v=0; pR1c.h=0; pR1c.v=1;
		pR2.h=0; pR2.v=0; pO1.h=2; pO1.v=0; 
		pY1a.h=2; pY1a.v=2; pY1b.h=0; pY1b.v=2;
	} else if ( theta <= 157)  {									// 157
		pR1a.h=2; pR1a.v=0; pR1b.h=0; pR1b.v=0; pR1c.h=0; pR1c.v=2;
		pR2.h=0; pR2.v=1; pO1.h=1; pO1.v=0; 
		pY1a.h=2; pY1a.v=1; pY1b.h=1; pY1b.v=2;
	} else if ( theta <= 202)  {									// 202
		pR1a.h=1; pR1a.v=0; pR1b.h=0; pR1b.v=1; pR1c.h=1; pR1c.v=2;
		pR2.h=0; pR2.v=2; pO1.h=0; pO1.v=0; 
		pY1a.h=2; pY1a.v=0; pY1b.h=2; pY1b.v=2;
	} else if ( theta <= 247)  {									// 247
		pR1a.h=0; pR1a.v=0; pR1b.h=0; pR1b.v=2; pR1c.h=2; pR1c.v=2;
		pR2.h=1; pR2.v=2; pO1.h=0; pO1.v=1; 
		pY1a.h=1; pY1a.v=0; pY1b.h=2; pY1b.v=1;
	} else if ( theta <= 292)  {									// 292
		pR1a.h=0; pR1a.v=1; pR1b.h=1; pR1b.v=2; pR1c.h=2; pR1c.v=1;
		pR2.h=2; pR2.v=2; pO1.h=0; pO1.v=2; 
		pY1a.h=0; pY1a.v=0; pY1b.h=2; pY1b.v=0;
	} else if ( theta <= 337)  {									// 337
		pR1a.h=0; pR1a.v=2; pR1b.h=2; pR1b.v=2; pR1c.h=2; pR1c.v=0;
		pR2.h=2; pR2.v=1; pO1.h=1; pO1.v=2; 
		pY1a.h=0; pY1a.v=1; pY1b.h=1; pY1b.v=0;
	} else {														// +-22
		pR1a.h=1; pR1a.v=2; pR1b.h=2; pR1b.v=1; pR1c.h=1; pR1c.v=0;
		pR2.h=2; pR2.v=0; pO1.h=2; pO1.v=2; 
		pY1a.h=0; pY1a.v=2; pY1b.h=0; pY1b.v=0;
	}

	// NOTE: assumes 'bit pix size of 3x3 !!

	c.red = 0xFFFF; c.green = 0x2000; c.blue = 0;		// r1
	RGBForeColor( &c);
	MoveTo( pR1a.h,pR1a.v);	Line(0,0);
	MoveTo( pR1b.h,pR1c.v);	Line(0,0);
	MoveTo( pR1c.h,pR1c.v);	Line(0,0);

	c.red = 0xBAC7; c.green = 0; c.blue = 0;			// r2
	RGBForeColor( &c);
	MoveTo( pR2.h,pR2.v);	Line(0,0);
	MoveTo( pO1.h,pO1.v);	Line(0,0);

	c.red = 0xFFFF; c.green = 0xCCC8; c.blue = 0;		// o2 (always center)
	RGBForeColor( &c);
	MoveTo( 1,1);	Line(0,0);

	c.red = 0xFFFF; c.green = 0xFFFF; c.blue = 0;		// y
	RGBForeColor( &c);
	MoveTo( pY1a.h,pY1a.v);	Line(0,0);
	MoveTo( pY1b.h,pY1b.v);	Line(0,0);

	SetGWorld( savGW, savGdH);
	return;
}


//***********************************************************
void PlayBoom()
{
	OSErr err;

	if ( gP->gSndHandle != nil ) 
		err = SndPlay( gP->gSndChP, (SndListHandle)gP->gSndHandle, true);
}


//***********************************************************************************
//														E N D   O F   L I S T I N G

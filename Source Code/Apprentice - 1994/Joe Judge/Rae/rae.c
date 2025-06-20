

// see the README file for version notes

#include "GraphicsModule_Types.h"
#include "Sounds.h"



typedef struct sicn {
	unsigned char bits[32];
} sicnStruct, *sicnPtr, **sicnHandle;

typedef struct infostruct {
	/*unsigned short*/
	short			GND[128];						// ??
	BitMap			face;

	Handle			faceSICN;						// faces resource
	short			numFaces;
	short			curFace;
	short			lastFace;
	short			whichFace;						// which item # in the menu?
	
	short			failedToStart;					// watchdog 
	
	short			time;							// "timer" for decay, etc
	short			state;
	int				x, y,
					xvel, yvel,
					xacc, yacc,
					xpos, ypos;
	int				xx, yy;							// temp/save values
	Rect			myRect;
	Handle			sndBounce;						// handle to 'boink' sound
	Handle			sndScream;						// scream sound
	SoundInfoHandle	soundInfo;						// handle to AD sound thing
	long			soundTicks;						// sound timer to keep us from 
													// sounding yucky
	
} infostruct, *infostructPtr, **infostructHandle;

#define BOING_SOUND_ID	256
#define SCREAM_SOUND_ID 129

#define BASE_FACE_ID	256						// MENUs items are 1-based, not 0

#define DOUBLE_FACE

#define SOUND_TIMER		3						// wait for # ticks
												// before playing the next one
#define MAX_ITERATIONS	(128)

unsigned short RangedRdm( unsigned short min, unsigned short max );
void SetRGBColor(void);

int drawit(GMParamBlockPtr params, infostructPtr info, int x, int y );
int decay(int v);
void ChooseFace(GMParamBlockPtr params, infostructPtr info);

#if 1
#define XMIN	(params->qdGlobalsCopy->qdThePort->portRect.left)
#define YMIN	(params->qdGlobalsCopy->qdThePort->portRect.top)
#define XMAX	(params->qdGlobalsCopy->qdThePort->portRect.right)
#define YMAX	(params->qdGlobalsCopy->qdThePort->portRect.bottom)
#define TOP		YMIN
#define BOT		(YMAX-1)

#else

#define HEIGHT  16
#define WIDTH   16
#define XMIN    0
#define YMIN    0
#define XMAX    511
#define YMAX    341
#define TOP     YMIN
#define BOT     (YMAX-1)
#endif

enum {
	RAE_STARTING,
	RAE_FALLING,
	RAE_SETTLING,
	RAE_DONE,
	RAE_RESET
};


// these are the functs that need defined ...
OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
OSErr DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params);

// extra ones
OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params);
OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params);


// this is borrowed from the example code from Bouncing Ball ...
/* some macros to simplify synchronizing to the vertical retrace. */
#define SynchFlag(m) (params->monitors->monitorList[m].synchFlag)
#define SynchVBL(m) synchFlag = &SynchFlag(m); *synchFlag = false; while(!*synchFlag);


static unsigned char facebits[32] = {
	0x07, 0xe0, 0x18, 0x18, 0x20, 0x04, 0x42, 0x42, 0x42, 0x42,
	0x82, 0x41, 0x80, 0x01, 0x80, 0x01, 0x84, 0x21, 0x88, 0x11,
	0x94, 0x29, 0x43, 0xc2, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18,
	0x07, 0xe0
};

//////////////////////////////////////////////////////////////////////////////////////
// this is the first funct called by AD ... we need to allocate and initialize here
OSErr
DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	Handle 			h;
	infostructPtr	info;
	short			offRowBytes;
	// error messages
	StringPtr spaceErrMsg = (StringPtr)"\prae: Not enough memory!!";
	// misc vars  ..
	short			i; 

	// Randomize...
	params->qdGlobalsCopy->qdRandSeed = TickCount();

	h = NewHandle( sizeof(infostruct) );
	if (h == (Handle)nil) {
		BlockMove(spaceErrMsg, params->errorMessage, spaceErrMsg[0]+1);
		return ModuleError;
	}
	*storage = h;
	MoveHHi(h);
	HLock(h);
	info = (infostructPtr)(*h);

	info->myRect = params->qdGlobalsCopy->qdThePort->portRect;
//	SetRect( &(info->myRect), 0, 0, XMAX+1, YMAX+1);
	

	info->soundTicks = TickCount();

	info->whichFace = params->controlValues[1];


	info->faceSICN = NULL;
	info->sndBounce = NULL;
	info->sndScream = NULL;
	ChooseFace(params,  info);

// set up the GND 
	for (i=0; i<128; i++) {
		info->x = i<<3;
		if (info->x <= XMIN+8 || info->x >= XMAX-8)
			info->GND[i] = 0;
		else {
			info->GND[i] = BOT - 9;
			if (i&01)
				if (info->GND[i-1]==0)
					info->GND[i] = 0;
				else
					info->GND[i] -= 7;
		}
	}

	info->state = RAE_STARTING;
	info->failedToStart = 0;
	
	HUnlock( h);
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// the screen saver has been awakened! time to ditch the storage and wave goodbye
OSErr 
DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {

	// soundInfoHandle to ditch ??
	if ( ( (infostructPtr)(*storage) )->soundInfo  != NULL)
		CloseSound( ((infostructPtr)(*storage) )->soundInfo, 
				params->sndChannel);
	
	// ditch the bounce sound
	if (((infostructPtr)(*storage) )->sndBounce != (Handle)NULL)
		ReleaseResource( ((infostructPtr)(*storage) )->sndBounce );
		
	// ditch the scream sound also
	if (  ((infostructPtr)(*storage) )->sndScream  != (Handle)NULL)
		ReleaseResource( ((infostructPtr)(*storage) )->sndScream);

	// ditch the SICN handle
	if ( ((infostructPtr)(*storage) )->faceSICN != (Handle)NULL)
		ReleaseResource( ((infostructPtr)(*storage) )->faceSICN );

	// finally, ditch the memory we had allocated
	DisposHandle( storage);
	return noErr;
}



//////////////////////////////////////////////////////////////////////////////////////
// make the screen go black
OSErr
DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) {
	infostructPtr		info;
	
	// darken the screen ...
	FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);

	return noErr;

}

//////////////////////////////////////////////////////////////////////////////////////
// this is the workhorse routine. It does the continual screen work to make
// this screen saver what it is.
OSErr 
DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params) 
{
	infostructPtr	info;
	Rect 			tmpRect;
	short 			a, b;


	HLock(storage);
	info = (infostructPtr)(*storage);

#ifdef DEBUGIT
SetRect( &tmpRect, 0, 0, 50, 20);
EraseRect( &tmpRect);
MoveTo( 10, 15);
a = info->xvel;
b = info->yvel;
if (a < 0) {
	DrawChar('-');
	DrawChar('0' - a);
} else {
	DrawChar( '+');
	DrawChar( '0' + a);
}
DrawChar('.');
if (b < 0) {
	DrawChar('-');
	DrawChar('0' - (b/10) );
	DrawChar('0' - (b%10) );
} else {
	DrawChar('+');
	DrawChar('0' + (b/10) );
	DrawChar('0' + (b%10) );
}

#endif

// quick check ... to help out demo mode
	if (info->whichFace != params->controlValues[1] ) {
	// someone changed the menu - choose that face for them...
	// first - erase us, if we're onscreen
	
		//info->curFace = info->lastFace;
		drawit( params, info, info->x, info->y);

	// set new face ... then go get it + it's sound
		info->whichFace = params->controlValues[1];
		ChooseFace(params, info);
		
	// put a new face onscreen ... so when we erase it, it's correct!
		drawit( params, info, info->x, info->y);
	}

// -------------------------------------------------------------------------
//			ALL DONE ... or major reset coming ... boom
// -------------------------------------------------------------------------
		if (info->state == RAE_RESET) {
			short i;
			
			// aaggghhh!!!
			if (info->sndScream != (Handle)NULL ) {
				long	waste;
				
				PlaySound( info->soundInfo, params->sndChannel, info->sndScream);
				Delay(30L, &waste);
			}
			
			
			// set up the GND 
			for (i=0; i<128; i++) {
				info->x = i<<3;
				if (info->x <= XMIN+8 || info->x >= XMAX-8)
					info->GND[i] = 0;
				else {
					info->GND[i] = BOT - 9;
					if (i&01)
						if (info->GND[i-1]==0)
							info->GND[i] = 0;
						else
							info->GND[i] -= 7;
				}
			}
	
			// darken the screen ...
			FillRgn(blankRgn, params->qdGlobalsCopy->qdBlack);
			info->state = RAE_STARTING;
			info->failedToStart = 0;
			HUnlock(storage);
			return noErr;
		}
	
	
// -------------------------------------------------------------------------
//			BALL DONE  start another now
// -------------------------------------------------------------------------
		if (info->state == RAE_DONE) {

			info->state = RAE_STARTING;
			info->failedToStart = 0;
			HUnlock(storage);
			return noErr;
		}
	
// -------------------------------------------------------------------------
//			STARTING OUT
// -------------------------------------------------------------------------
	if (info->state == RAE_STARTING) {	

		if (info->failedToStart > MAX_ITERATIONS) {
			info->state = RAE_RESET;
			HUnlock(storage);
			return noErr;
		}

			info->xpos = Random() & 0177; // 0177 == 127 == 0x7F
			if (info->GND[info->xpos] <= TOP) {
				info->failedToStart++;
				HUnlock(storage);
				return noErr;
			}
			info->failedToStart = 0;

			info->x = (info->xpos) << 3;
			//  |01 means an odd number
			//	&07 means to ensure only the last 3 bits are on
			// info->xvel = 4 - ((Random()|01) & 07);
			info->xvel = 4 - 
				( (RangedRdm(0, 255) | 01) & 07 );
				
			info->yacc = 1;
			info->yvel = 1;		// was 0
			info->y = TOP;
			info->state = RAE_FALLING;

			info->curFace = info->lastFace =  0;

			info->xx = info->yy = 0;		// joe

			drawit(params, info, info->x, info->y);			

			HUnlock(storage);
			return noErr;
	}
	
// -------------------------------------------------------------------------
// 			FALLING 
// -------------------------------------------------------------------------
	if (info->state == RAE_FALLING) {
		long		waste;
		short		eraseX, eraseY;			// saved values of where to erase
		
	
		// we'll get called into this section with the info->time var incremented
		// through each iteration ...
		info->time++;
		
		
		// save the values - so we erase closer to the time
		// when we draw (reduce that flicker)
		eraseX = info->x;
		eraseY = info->y;
		
		// save the x and the y 
		info->xx = info->x; 
		info->yy = info->y; 
		
		// update the acceleration
		info->yvel += info->yacc;

		// move it along it's way
		info->y += info->yvel;
		info->x += info->xvel;
		
		if (info->y > info->GND[info->x>>3]) {	/* bounce? */

			// make a noise
			if (info->sndBounce != (Handle)NULL  
#ifdef SOUND_TIMER
					&& ( TickCount() > (info->soundTicks + params->controlValues[2] ))
#else
					&& !SoundBusy( info->soundInfo, params->sndChannel)
#endif
					 ) {
				info->soundTicks = TickCount();
				PlaySound( info->soundInfo, params->sndChannel, info->sndBounce);
			}

			
#ifdef DOUBLE_FACE
			// if we're going fast, show an extra face, at the bounce
			if (info->yvel>5) {
				short savedFace;
				savedFace = info->lastFace;
				
				if (info->numFaces > 1)  // more than 1 face? show the second one!!
					info->curFace = 1;
				drawit(params, info, info->x, info->y);		// place the face
				Delay(1L, &waste);
				drawit(params, info, info->x, info->y);		// erase the face
				info->lastFace = savedFace;
				
			} else 
#endif // DOUBLE_FACE
				if (info->numFaces > 1) 	// more than 1 face? show second one!!
					info->curFace = 1;
				
			if (info->y <= info->GND[info->xx>>3]) { /* side collision? */
				info->x = info->xx;
				info->xvel = -info->xvel;

			} else if (info->yy <= info->GND[info->x>>3]) { /*bottom? */

				info->y = info->yy;
				info->yvel = -info->yvel;
				
			} else {	/* corner */
			
				info->x = info->xx;
				info->y = info->yy;
				
				info->xvel = -info->xvel;
				info->yvel = -info->yvel;
			}
			
			// every 15 bounces (?) (\017) we decay the x velocity
			if ((info->time & 017) == 0)
				info->xvel = decay(info->xvel);
			
			// is it down to zero ?
			if (info->xvel == 0) {
			
				info->xpos = (info->x) >> 3;

					// go right 
				if (info->GND[info->xpos-1] < info->GND[info->xpos]
				 && info->GND[info->xpos]   < info->GND[info->xpos+1])
					info->xvel = 1;
					/* go left */
				else if (info->GND[info->xpos-1] > info->GND[info->xpos]
				      && info->GND[info->xpos]   > info->GND[info->xpos+1])
					info->xvel = -1;
					/* on hilltop */
				else if (info->GND[info->xpos-1] > info->GND[info->xpos]
				      && info->GND[info->xpos]   < info->GND[info->xpos+1]) {
					if (Random() & 01)
						info->xvel = 1;
					else
						info->xvel = -1;
				}
			}
			
			// decay the y velocity at each bounce
			info->yvel = decay(info->yvel);	
						
		} // end of 'if BOUNCE' 
			else info->curFace = 0;	// set the face back to the starting one!!



		if (params->controlValues[0]) {
				short saveIt;
				
				saveIt = info->curFace;
				
				info->curFace = info->lastFace;
				drawit( params, info, eraseX, eraseY);
				
				info->curFace = saveIt;
		}

		// place the face (was ->xx, ->yy)
		drawit(params, info, info->x, info->y);
	
		
		if (info->xvel==0 && info->yvel==0 && info->y > info->GND[info->x>>3]-4) {
			info->state = RAE_SETTLING;
			HUnlock(storage);
			return noErr;
		}
	} // end of the falling section

	
	
	
// -------------------------------------------------------------------------
// 			SETTLING 
// -------------------------------------------------------------------------
	if (info->state == RAE_SETTLING) {
	

		// time to erase it
		drawit(params, info, info->x, info->y);	

		info->curFace = 0;

		/* find stable position */
		if (info->GND[(info->xpos)-1] <	info->GND[info->xpos] && 
			info->GND[info->xpos]   >	info->GND[(info->xpos)+1]) {
			
			// place it?
			drawit(params, info, (info->xpos)<<3, info->GND[info->xpos]);
			// update the "ground" array to know there is something here
			info->GND[info->xpos] -= 21;
			
			if (info->GND[(info->xpos)-1] <= 0)
				info->GND[info->xpos] -= 7;
				
			info->GND[(info->xpos)+1] -= 7;
			info->state = RAE_DONE;	// all done
			HUnlock(storage);
			return noErr;
		}
		
		/* roll right */
		if (	info->GND[(info->xpos)-1]	<	info->GND[info->xpos] && 
				info->GND[info->xpos]		<	info->GND[(info->xpos)+1]) {
			info->xpos++;
			HUnlock(storage);
			return noErr;
		}
		/* roll left */
		if (	info->GND[(info->xpos)-1]	>	info->GND[info->xpos] && 
				info->GND[info->xpos]		>	info->GND[(info->xpos)+1]) {
			info->xpos--;
			HUnlock(storage);
			return noErr;
		}
		/* on hilltop, choose at Random */
		if (	info->GND[(info->xpos)-1]	>	info->GND[info->xpos] && 
				info->GND[info->xpos]		<	info->GND[(info->xpos)+1]) {
			if (Random() & 01)
				info->xpos++;
			else
				info->xpos--;
			HUnlock(storage);
			return noErr;
		}
		
		// none of the above tests worked ... so something is wrong!!!!!
		/* else botch */
		if (info->sndScream != (Handle)NULL ) {
			long waste;
			
			info->soundTicks = TickCount();
			PlaySound( info->soundInfo, params->sndChannel, info->sndScream);
			Delay(30L, &waste);
		}

		// place the face
		// drawit(params, info, (info->xpos)<<3, info->GND[info->xpos]);
		
		// erase the screen
		//PaintRect(&(info->myRect) );
		info->state = RAE_RESET;	// was RAE_DONE all done
		
		HUnlock(storage);
		return noErr;
	}	


AllDone:
	HUnlock(storage);
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////
// this is called when they click on something in the control panel
OSErr 
DoSetUp(RgnHandle blankRgn, short message, GMParamBlockPtr params) {

	return noErr;
}



OSErr DoSelected(RgnHandle blankRgn, short message, GMParamBlockPtr params) {
	return noErr;
}



OSErr DoAboutBox(RgnHandle blankRgn, short message, GMParamBlockPtr params) {

	return noErr;
}



// drawit - place the rae on the screen 
// pass the params + the locked down pointer to the infostruct
drawit(params, info, x, y)
GMParamBlockPtr params;
infostructPtr info;
int x,y;
{       
	int i;
	Rect trect;
	RGBColor		black = {0,0,0}, White = {65535,65535,65535};
volatile	register Boolean *synchFlag;
	long	waste;


	if (x<XMIN)
		x = XMIN+8;
		
	if (y<TOP)
		y = TOP+8;
		
	if (x>XMAX)
		x = XMAX-8;
	
	info->xpos = x>>3;
	if (y > info->GND[info->xpos])
		y = info->GND[info->xpos];

	/* was a call to bitblt() */
	SetRect(&trect, x-12, y-8, x+4, y+8);


	ForeColor( whiteColor);
	BackColor( blackColor);
	
	if (info->faceSICN != NULL) {
		HLock( info->faceSICN );
		info->face.baseAddr = *info->faceSICN + (info->curFace * sizeof( sicnStruct ) );
		info->lastFace = info->curFace;
	} else {
		info->face.baseAddr = (QDPtr) &facebits[0];
		info->lastFace = info->curFace;
	}

	SynchVBL( 0 );
	CopyBits(&info->face, &params->qdGlobalsCopy->qdThePort->portBits, 
			&info->face.bounds, &trect, 
				srcXor, 0L);
				
	if (info->faceSICN != NULL)
		HUnlock( info->faceSICN);

}

decay(v)
register v;
{
	if (v==0)
		return(v);
	if (v > 0)
		return(v-1-(v>>3));
	return(v+1-(v>>3));
}



// this is from the Think C reference code example ...
unsigned short RangedRdm( unsigned short min, unsigned short max )
/* assume that min is less than max */
{
	// uh ... not this isn't quite right - it's between 0 and 65535, not 65536
	unsigned	qdRdm;	/* treat return value as 0-65536 */
	long	range, t;
	
	// just to be safe, I'll put this here
	if (min > max) {
		DebugStr("\pMin greater then Max in RangedRdm. Type 'G' and return to continue.");
		return min;
	}
	
	qdRdm = Random();
	range = max - min;
	// max - min gives us the the difference between max and min ... that is 
	// not inclusive. It gives us { min <= range < max }
	// so we never see that max number!!
	range++;
	t = ((long)qdRdm * range) / 65536; 	/* now 0 <= t <= range */
	return( t+min );
}

// ONLY works with color QD - CHECK first!
void 
SetRGBColor( void) {
RGBColor	r;

//#define MIN_VALUE	32767	// brigher colors :)
//#define MIN_VALUE 16384
//#define MIN_VALUE 8192
//#define MIN_VALUE 4096
#define MIN_VALUE 2048

	r.red = (unsigned short) RangedRdm( MIN_VALUE, 65535);
	r.green = (unsigned short) RangedRdm( MIN_VALUE, 65535);
	r.blue = (unsigned short) RangedRdm( MIN_VALUE, 65535);
	
	RGBForeColor(&r);

}



void
ChooseFace(GMParamBlockPtr params, infostructPtr info ) {

// ditch old ...
	if (info->faceSICN != NULL) {
		ReleaseResource( info->faceSICN);
		info->faceSICN = NULL;
	}
	if (info->sndBounce != NULL) {
		ReleaseResource( info->sndBounce);
		info->sndBounce = NULL;
	}
	if (info->sndScream != NULL) {
		ReleaseResource( info->sndScream);
		info->sndScream = NULL;
	}

// get new ...
	info->faceSICN = GetResource('SICN', BASE_FACE_ID + info->whichFace);
	
	if (info->faceSICN != (Handle)NULL) {
		MoveHHi( info->faceSICN );
		info->numFaces = SizeResource( info->faceSICN ) / sizeof(sicnStruct) ;
		info->face.baseAddr = *info->faceSICN;
		info->face.rowBytes = 2;
		SetRect( &(info->face.bounds), 0, 0, 16, 16);
		info->curFace = info->lastFace = 0;
		
	} else { // fall back to the default, hard-coded facebits
		SysBeep(0);
		// set up the bitmap
		info->face.baseAddr = (QDPtr) &facebits[0];
		info->face.rowBytes = 2;
		SetRect( &(info->face.bounds), 0, 0, 16, 16);
		info->curFace = info->lastFace = 0;
		info->numFaces = 1;
	}

			
// sound?
	if (params->systemConfig & SoundAvailable)  {
		info->sndBounce = GetResource('snd ', BOING_SOUND_ID + info->whichFace);
		info->sndScream = GetResource('snd ', SCREAM_SOUND_ID);
	} else {
		info->sndBounce = NULL;
		info->sndScream = NULL;
	}
	
	if (info->sndBounce != (Handle)NULL || info->sndScream != (Handle)NULL ) 
		info->soundInfo = OpenSound(params);
	else 
		info->soundInfo = NULL;

}
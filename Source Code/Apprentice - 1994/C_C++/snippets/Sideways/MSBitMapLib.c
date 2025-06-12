/******************************************************************************

This contains a routine to draw rotated text. There are a number of useful
bitmap routines here as well. This code is written to be simple, flexible and 
understandable but is clearly not especially efficient.

The usual disclaimers apply: if you use this code and there are errors, you're
on your own. There is no charge for this trifle, and you don't have to reference
me if you use it in an application. If someone is foolish enough to want to 
include this stuff in source code libraries that are for sale, talk to me and 
we'll work something out. 

It would be interesting for me if anyone that uses this stuff were to let me know
what you're using it for, but that's up to you. Certainly lemme know if there are
any boo-boos or gotchas so that I can fix them.
 
           Written by Mark Stockwell
							  Compuserve: 76354,2303
								AOL: 				MarkS77
								Internet: 	76354.2303@compuserve.com
														MarkS77@aol.com

 ******************************************************************************/

#include "MSUtil.h"
#include "MSBitMapLib.h"

struct MSTextState {
	int 		ftxFont;
	Style		ftxFace;
	int			ftxMode, ftxSize;
	Fixed		fspExtra;
	RGBColor	fForeColor, fBackColor;
};

typedef struct MSTextState MSTextState;


MSTextState MSGetCTextState (
	CGrafPtr	aPort)
{
	RGBColor 		theForeColor, theBackColor;
	MSTextState	holdTextState;

	holdTextState.ftxFont = (*aPort).txFont;
	holdTextState.ftxFace = (*aPort).txFace;
	holdTextState.ftxMode = (*aPort).txMode;
	holdTextState.ftxSize = (*aPort).txSize;
	holdTextState.fspExtra = (*aPort).spExtra;
	GetForeColor(&theForeColor);
	GetBackColor(&theBackColor);
	holdTextState.fForeColor = theForeColor;
	holdTextState.fBackColor = theBackColor;
	return holdTextState;
}

MSTextState MSGetTextState (
	GrafPtr	aPort)
{
	RGBColor 		theForeColor, theBackColor;
	MSTextState	holdTextState;

	holdTextState.ftxFont = (*aPort).txFont;
	holdTextState.ftxFace = (*aPort).txFace;
	holdTextState.ftxMode = (*aPort).txMode;
	holdTextState.ftxSize = (*aPort).txSize;
	holdTextState.fspExtra = (*aPort).spExtra;
	return holdTextState;
}


void MSSetTextState (
	MSTextState theState)
{
	TextFont(theState.ftxFont);
	TextFace(theState.ftxFace);
	TextMode(theState.ftxMode);
	TextSize(theState.ftxSize);
	SpaceExtra(theState.fspExtra);
	if (MSColorIndexesOK())
	{
		RGBForeColor(&theState.fForeColor);
		RGBBackColor(&theState.fBackColor);
	}
}

void MSEraseBitMap (
	BitMap *theBitMap)
{
		// The srcXor mode erases the bitMap.
	CopyBits(theBitMap, theBitMap, &((*theBitMap).bounds), &((*theBitMap).bounds), srcXor, NULL);
}

BitMap *MSNewBitMap (
	short *bitsWide,
	short *bitsHigh)
{
	BitMap		*tempBitMap;
	long	sizeOfImage;

	tempBitMap = (BitMap *) NewPtr(sizeof(BitMap));
	if (tempBitMap == NULL)
		return NULL;
		
	tempBitMap->baseAddr = NULL;

       // Force bitsWide to be a multiple of 8. 
       // We do this because bitMaps want it this way, just because.
	if (*bitsWide != (*bitsWide / 8) * 8) 
		*bitsWide = ((*bitsWide / 8) + 1) * 8L;

	if (*bitsHigh != (*bitsHigh / 8) * 8) 
		*bitsHigh = ((*bitsHigh / 8) + 1) * 8;

	SetRect(&(tempBitMap->bounds), 0, 0, *bitsWide, *bitsHigh);
	tempBitMap->rowBytes = (*bitsWide) / 8;
	sizeOfImage = (*bitsWide) * (*bitsHigh);
		tempBitMap->baseAddr = (Ptr)NewPtr(sizeOfImage);
		
		// Didn't get the memory, bail.
	if (tempBitMap->baseAddr == NULL)
	{
		DisposPtr((Ptr)tempBitMap);
		return NULL;
	}
		
		// Give us a clean slate.
	MSEraseBitMap(tempBitMap);
	return tempBitMap;
}

void MSDisposeBitMap(
	BitMap *theBitMap)
{
	if (theBitMap->baseAddr != NULL)
		DisposPtr(theBitMap->baseAddr);
	if (theBitMap != NULL)
		DisposPtr((Ptr)theBitMap);
}

int MSGoodBitMap (
	BitMap theBitMap)
{
	if (&theBitMap == NULL)
		return 0;
		
	if (theBitMap.baseAddr != NULL) 
		return 1;
	else
		return 0;
}

int MSBitIsSet (
	short 	x, 
	short 	y,
	BitMap	*theBitMap)
{
	long	theIndex;

	theIndex = y * (theBitMap->bounds.right - theBitMap->bounds.left) + x;

	if (BitTst(theBitMap->baseAddr, theIndex))
		return 1;
	else
		return 0;
}

void MSSetTheBit (
	short 	x, 
	short 	y,
	BitMap	*theBitMap)
{
	long	theIndex;

	theIndex = y * (theBitMap->bounds.right - theBitMap->bounds.left) + x;

	BitSet(theBitMap->baseAddr, theIndex);
}

void MSClearTheBit (
	short 	x, 
	short		y,
	BitMap	*theBitMap)
{
	long 	theIndex;

	theIndex = y * (theBitMap->bounds.right - theBitMap->bounds.left) + x;

	BitClr(theBitMap->baseAddr, theIndex);
}


// Rotate a bitmap....
BitMap *MSBitFlip (
 	BitMap	*inMap)
{
	short		x, y, bitsWide, bitsTall;
	BitMap 	*outMap;

		// Check to see if we got a valid BitMap.
	if (!MSGoodBitMap(*inMap))
	{
		outMap = NULL;
		return;
	}
		
		// How big is this puppy.
	bitsWide = inMap->bounds.right - inMap->bounds.left;
	bitsTall = inMap->bounds.bottom - inMap->bounds.top;

		// Make a new bitmap to hold the rotated image.
		// Where the original was wide and tall, this one is tall and wide.
	outMap = MSNewBitMap(&bitsTall, &bitsWide);
	if (!MSGoodBitMap(*outMap)) 
		return NULL; 
	
	  // For all the bits set in inMap ( the dark pixels) , set the 
	  // appropriate bit in outMap. This can be sped up considerably
	  // by 'unwrapping' the BitIsSet and SetTheBit routines, and not
	  // recalculating theIndex each time.
	for (y=0; y < bitsTall; y++)
		{
			for (x=0; x < bitsWide; x++)
				{
					if (MSBitIsSet(x, y, inMap))
						MSSetTheBit(y, (bitsWide -1) - x, outMap);
				}
		}
	return outMap;
}


void MSDrawRotatedText (
	short 	xPixel,
	short		yPixel, 
	Str255	theText)
{
	GrafPort			myPort;
	GrafPtr				oldPortPtr;
	CGrafPtr			oldcPortPtr;
	BitMap				*defaultBitMap, *offscreenBitMap, *rotatedBitMap;
	Rect					destRect;
	FontInfo			Info;
	short					bitsTall, bitsWide, padding;
	Boolean				oldPortIsColorPort;
	MSTextState		theTextState;


		// Figure out how big we need to make our bitmaps.  
		// Add some padding at top and bottom equal to the descent. 
	GetFontInfo(&Info);
	padding = Info.descent;
	bitsTall = Info.ascent + Info.descent + 2 * padding;
	bitsWide = StringWidth(theText) + 2 * padding;


		// Here's where we create a new bitMap to draw the text into that
		// we're gonna rotate.
	offscreenBitMap = MSNewBitMap(&bitsWide, &bitsTall);
	if (!MSGoodBitMap(*offscreenBitMap))
		return;


		// Figure out if we're dealing with a color GrafPort or a B&W one.
  if (MSColorIndexesOK())
  	oldPortIsColorPort = true;
  else
  	oldPortIsColorPort = false;

		// We're getting ready to draw offscreen. If we just set the
		// portbits field of the current grafPort to our offscreen
		// bitmap, we'll draw the unrotated text into the current 
		// port (your window), then rotate it and draw the rotated
		// version into your window as well.
		
		// To only draw the rotated text, we must save the original
		// grafPort (the one attached to your window) and create a
		// new grafPort not attached to any window in which to draw
		// offscreen.
		
		
		
		// Here's where we save the grafPort belonging to your 
		// current window. 
		
		// The new grafport gets default values for font, text size, 
		// etc... We need to capture these values from the current 
		// port before creating the new port.
	if (oldPortIsColorPort)
	{
		GetPort( (GrafPtr *)&oldcPortPtr );
		theTextState = MSGetCTextState(oldcPortPtr);
	}
	else
	{
		GetPort( &oldPortPtr );
		theTextState = MSGetTextState(oldPortPtr);
	}
		

		
		// Now create the new grafPort in which we'll be drawing offscreen.
	OpenPort(&myPort);
	
		// Set the text state for this port that of the original grafPort. 
	MSSetTextState(theTextState);
	
	
		// Save the default bitmap from our new grafPort. It's probably not
		// the right size.
	defaultBitMap = &(myPort.portBits);
		
		
		// Set the portBits field of the new grafPort to our bitmap instead.
		// QuickDraw will now draw into 'offscreenBitMap', instead of 
		// into 'defaultBitMap'. 		
	SetPortBits(offscreenBitMap);


		// Move right 	'padding' 			pixels.
		// Move up 		'descent+padding' 	pixels.
		// Then draw the string.
	MoveTo(padding, bitsTall - Info.descent - padding);
	DrawString(theText);

		
		// Restore the old grafPort as the current port.                              
	if (oldPortIsColorPort)
		SetPort( (GrafPtr)oldcPortPtr );
	else
		SetPort( oldPortPtr );
		
		// Give myPort back its default bitmap, and then get
		// rid of the whole thing. The bitmap with our text
		// drawn into it is offscreenBitMap.
	SetPortBits(defaultBitMap);
	ClosePort(&myPort);


		// This is where the text is actually turned on its side. 
		// The bitmap "rotatedBitMap" is a rotated version of
		// offscreenBitMap.
	rotatedBitMap = MSBitFlip(offscreenBitMap);
	if (!MSGoodBitMap(*rotatedBitMap))
	{
		MSDisposeBitMap(offscreenBitMap);
		return;
	}

		// The rotated bitmap is moved into position and drawn into whatever port
		// was active when you called DrawSideWaysText.                                   
	destRect = rotatedBitMap->bounds;
	OffsetRect(&destRect, xPixel - bitsTall + padding + Info.descent, yPixel - bitsWide + padding);
	CopyBits(rotatedBitMap, &((*thePort).portBits), &(rotatedBitMap->bounds), &destRect, srcOr, NULL);


		// We've still got these two bitmaps we don't need any more.
		// I want my memory back NOW.  
	MSDisposeBitMap(offscreenBitMap);
	MSDisposeBitMap(rotatedBitMap);
}


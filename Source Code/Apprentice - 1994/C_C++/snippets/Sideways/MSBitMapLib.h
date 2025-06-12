#ifndef __MSBitMapLib__
#define __MSBitMapLib__

#include <QuickDraw.h>

#define MSStr255	unsigned char *
			
void MSEraseBitMap (
	BitMap *theBitMap);
BitMap *MSNewBitMap (
	short *bitsWide,
	short  *bitsHigh);
int MSGoodBitMap (
	BitMap theBitMap);
int MSBitIsSet (
	short 	x, 
	short 	y,
	BitMap	*theBitMap);
void MSSetTheBit (
	short 	x, 
	short 	y,
	BitMap	*theBitMap);
void MSClearTheBit (
	short 	x, 
	short	y,
	BitMap	*theBitMap);
void MSDrawRotatedText (
	short 	xPixel,
	short		yPixel, 
	Str255	theText);
	
#endif
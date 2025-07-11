/*
	Offscreen.c
	
	Routines for using offscreen drawing from THINKin� CaP 4
	Using this method of updating the clock window avoids flicker
*/

#include "Offscreen.h"


OffscreenBitsP SetupOffBit(Rect	*bounds)
{
	OffscreenBitsP	offBitsP;
	short			rowBytes;				/* Width of BitMap in bytes			*/
	
	rowBytes = (((bounds->right - bounds->left) + 15) / 16) * 2;
	offBitsP = (OffscreenBitsP) NewPtrClear(sizeof(OffscreenBitsRec) +
									(long) rowBytes * (bounds->bottom - bounds->top));

	if (offBitsP != nil) {
		offBitsP->macBitMap.rowBytes = rowBytes;
		offBitsP->macBitMap.baseAddr = offBitsP->bitImage;
		offBitsP->macBitMap.bounds = *bounds;
	}
		
	return(offBitsP);
}


void BeginOffBitDraw(OffscreenBitsP	offBitsP)
{
	if (offBitsP != nil) {
		offBitsP->saveBitMap = thePort->portBits;
		SetPortBits(&offBitsP->macBitMap);
	}
}


void EndOffBitDraw(OffscreenBitsP offBitsP, Boolean	copyImage)
{
	if (offBitsP != nil) {
		SetPortBits(&offBitsP->saveBitMap);
		
		if (copyImage) {
			CopyOffBitImage(offBitsP, srcCopy, nil);
		}
	}
}


void CopyOffBitImage(OffscreenBitsP	offBitsP, short	xferMode, RgnHandle	maskRgn)
{
	if (offBitsP != nil) {
		CopyBits(&offBitsP->macBitMap, &thePort->portBits,
				&offBitsP->macBitMap.bounds, &offBitsP->macBitMap.bounds,
				xferMode, maskRgn);
	}
}


void TransferOffBitImage(
	OffscreenBitsP	offBitsP,
	Rect			*offBounds,
	Rect			*onBounds,
	short			xferMode,
	RgnHandle		maskRgn)
{
	if (offBitsP != nil) {
		CopyBits(&offBitsP->macBitMap, &thePort->portBits,
				offBounds, onBounds, xferMode, maskRgn);
	}
}


void DestroyOffBit(OffscreenBitsP offBitsP)
{
	if (offBitsP != nil) {
		DisposPtr((Ptr)offBitsP);
	}
}


/*
	Offscreen.h
*/

#pragma once



typedef struct OffscreenBitsRec {			/** Offscreen BitMap **/
	BitMap		macBitMap;						/* Offscreen BitMap						*/
	BitMap		saveBitMap;						/* Current BitMap to restore when done	*/
	char		bitImage[];						/* Offscreen bit image					*/
} OffscreenBitsRec, *OffscreenBitsP;


OffscreenBitsP	SetupOffBit(Rect *bounds);
void			BeginOffBitDraw(OffscreenBitsP offBitsP);
void			EndOffBitDraw(OffscreenBitsP offBitsP, Boolean copyImage);
void			CopyOffBitImage(OffscreenBitsP offBitsP, short xferMode, RgnHandle maskRgn);
void			TransferOffBitImage(OffscreenBitsP offBitsP, Rect *offBounds, Rect *onBounds,
										short xferMode, RgnHandle maskRgn);
void			DestroyOffBit(OffscreenBitsP offBitsP);

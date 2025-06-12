// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FastBits.h

#define __FASTBITS__

struct SicnFastBits
{
	Ptr fBits;
	short fRsrcID;
};

void InitUFastBits();
void InitializeSicnFastBits(SicnFastBits &sicnFastBits);
void GetNewSicnFastBits(SicnFastBits &fb, short id);
void DisposeSicnFastBits(SicnFastBits &fb);
void DrawSicnFastBits(BitMap *portBitsP, const SicnFastBits &fb, CRect srcRect, CRect destRect);

// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// FastBits.cp

#include "FastBits.h"
#include "Tools.h"

#include <Resources.h>

#pragma segment MyTools

BitMap gFastBitsSicnBitMap;

TLongintList *gSicnBitmapCache = nil;

void InitUFastBits()
{
	gFastBitsSicnBitMap.rowBytes = 2;
	gFastBitsSicnBitMap.bounds = CRect(0, 0, 16, 16);
	TLongintList *lList = new TLongintList();
	lList->ILongintList();
	gSicnBitmapCache = lList;
}

void InitializeSicnFastBits(SicnFastBits &sicnFastBits)
{
	sicnFastBits.fBits = nil;
	sicnFastBits.fRsrcID = 0;
}

void GetNewSicnFastBits(SicnFastBits &fb, short id)
{
	fb.fRsrcID = id;
	Handle h = GetResource('ics#', id);
	FailNILResource(h);
	if (gSicnBitmapCache->GetSize())
		fb.fBits = Ptr(gSicnBitmapCache->Pop());
	else
		fb.fBits = NewPermPtr(64);
	BytesMove(*h, fb.fBits, 64);
}

void DisposeSicnFastBits(SicnFastBits &fb)
{
	if (fb.fBits)
		gSicnBitmapCache->Push(long(fb.fBits));
	fb.fBits = nil;
}

void DrawSicnFastBits(BitMap *portBitsP, const SicnFastBits &fb, CRect srcRect, CRect destRect)
{
#if qDebug
	if (!portBitsP)
		ProgramBreak("portBitsP == nil");
#endif
	gFastBitsSicnBitMap.baseAddr = fb.fBits;
	CopyBits(gFastBitsSicnBitMap, *portBitsP, srcRect, destRect, srcCopy, nil);
}

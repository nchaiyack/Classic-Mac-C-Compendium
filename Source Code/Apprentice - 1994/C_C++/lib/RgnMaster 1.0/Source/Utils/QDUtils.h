#ifndef _QDUTILS_H_
#define _QDUTILS_H_


#include <QDOffscreen.h>


// GetGWorldBitMap #define
// CopyBits requires a bitmap, but GWorlds uses pixmaps, so we have
// to do some dereferencing every time we call CopyBits. Use this
// #define instead to save some typing...

#define GetGWorldBitMap(theGWorld) (BitMap*)(*(GetGWorldPixMap(theGWorld)))

void CenterRect(Rect *innerRect, Rect *outerRect);
void MoveRectTo(short h, short v, Rect *theRect);
void MoveRgnToRect(Rect *theRect, RgnHandle theRgn);
void MoveRgnTo(short hLoc, short vLoc, RgnHandle theRgn);

Boolean	NewOffScreenBitMap(GrafPtr *, Rect *, int);
void DestroyOffscreenBitMap(GrafPtr);

#endif	// _QDUTILS_H_
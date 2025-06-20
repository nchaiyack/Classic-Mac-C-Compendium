/*
	GraphicsBuffer.h header file
	by Hiep Dam
	Version 3.0
	Last update: June 1995
	
	Copyright �1995 by Hiep Dam, All Rights Reserved
	You may freely use GraphicsBuffers.h, GraphicsBuffers.c
	in your own applications.
*/

// ---------------------------------------------------------------------------

#ifndef GRAPHICSBUFFER_H_
#define GRAPHICSBUFFER_H_

#ifndef __QDOFFSCREEN__
	#include <QDOffscreen.h>
#endif

#include "Graphics Defines.h"	// Some "shortcuts"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// ---------------------------------------------------------------------------

/*
	These are the pixel copying routines currently supported by
	GraphicsBuffers.
*/

typedef enum {
	kCopyMaskMethod,
	kCopyBitsTransparentMethod,
	kCopyBitsRegionMethod,
	kBlitterTransparentMethod,
	kBlitterDeepMaskMethod
} AnimationMethod;


/*
	Our GraphicsBuffer structure. Similar to GWorlds in theory,
	this structure is used to hold some additional data
	that we can use to speed up animation.
	
	Currently there are two types: standard GraphicsBuffers,
	which are really offscreen GWorlds; and output GraphicsBuffers,
	which encapsulate the monitor device and window we'll be
	"outputting" to...
	Standard GraphicsBuffers are created with calls to NewGraphicsBuffer;
	output GraphicsBuffers are created with calls to Convert2GraphicsBuffer.
*/

typedef enum {
	kGraphicsBuffer,		// A GWorld with some pre-calc values for speed
	kVideoMemoryBuffer		// A graphics buffer equivalent for the monitor
} GraphicsBufferType;

typedef struct {
	short type;							// kGraphicsBuffer or kVideoMemoryBuffer
	short bufferFlushedTopLeft;			// true if topleft of global rect is (0,0)
	short isCached;
	short padding;

	GWorldPtr gworld;					// GWorld itself
										// (if kVideoMemoryBuffer, this is nil).			

	Rect localBounds;					// Bounds of the gworld, in local coords.
	Rect globBounds;					// Bounds, in global coords.
	Rect videoBounds;					// Used only if type == kVideoMemoryBuffer

	PixMapHandle pixmap;
	WindowPtr window;

	unsigned long realRowBytes;			// (**pixmap).rowBytes & 0x7FFF.
	unsigned long rowBytesDiv4;			// stripRowBytes / 4.
	unsigned long rowBytesDiv2;
	unsigned long *rowAddressOffsets;	// Array of pre-calculated
										/// rowByte offsets.
} GraphicsBuffer, *GraphicsBufferPtr;

// ---------------------------------------------------------------------------

/*
	ImageEraser    -> Copy bkgnd patch & put it onto temp buffer
	ImageDrawer    -> Draw sprite (masked) over bkgnd patch in temp buffer
		// Depending on the drawer, typecasted Ptr could be either a
		// RgnHandle or a GraphicsBufferPtr
	ImageRefresher -> Copy this updated patch from temp buffer to screen
*/

typedef void (*ImageEraser)(GraphicsBufferPtr, GraphicsBufferPtr, Rect*, Rect*);
typedef void (*ImageDrawer)(GraphicsBufferPtr, GraphicsBufferPtr, Rect*, Rect*, Ptr);
typedef ImageEraser ImageRefresher;

// ===========================================================================

/*
	Make sure you call InitGraphicsBuffer() before calling any other
	routines in here!
*/
OSErr InitGraphicsBuffers();

// ---------------------------------------------------------------------------

/*
	GraphicsBuffer creation, deletion, utility calls. Most of these
	are fashioned after similar GWorld calls.
*/
OSErr NewGraphicsBuffer(
	GraphicsBufferPtr *buffer,
	long pixelDepth,
	Rect *boundsRect,
	long flags,
	long cache);
	
OSErr UpdateGraphicsBuffer(
	GraphicsBufferPtr buffer,
	long pixelDepth,
	Rect *boundsRect,
	long flags,
	long cache);

OSErr DisposeGraphicsBuffer(GraphicsBufferPtr buffer);

OSErr Convert2GraphicsBuffer(
	GraphicsBufferPtr *buffer,
	WindowPtr srcWind,
	Rect *bounds,
	long cache);

void SetGraphicsBuffer(GraphicsBufferPtr buffer);

Boolean LockGraphicsBuffer(GraphicsBufferPtr buffer);

void UnlockGraphicsBuffer(GraphicsBufferPtr buffer);

// ---------------------------------------------------------------------------

/*
	Pixel copying calls.
*/

	// Uses standard CopyBits calls
void CopyGraphicsBuffer(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR);

	// Explicit source or destination.
void CopyGraphicsBuffer2Window(
	GraphicsBufferPtr srcBuffer,
	WindowPtr destWind,
	Rect *srcR,
	Rect *destR);

void CopyWindow2GraphicsBuffer(
	WindowPtr srcWind,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR);

	// CopyBits with a region mask, no region mask and transparent transfer
	// mode, or CopyMask
void CopyGraphicsBufferRegion(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	RgnHandle maskRgn);

void CopyGraphicsBufferMask(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr maskBuffer);

void CopyGraphicsBufferTransparent(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr _notUsed);

// ---------------------------------------------------------------------------

	// Uses custom pixel-blitting routines (8-bit only)
void BlitGraphicsBuffer_8bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR);

void BlitGraphicsBuffer_Mask8bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr maskBuffer);

void BlitGraphicsBuffer_Transparent8bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr _notUsed);

	// 4-bit blitters
void BlitGraphicsBuffer_4bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR);

void BlitGraphicsBuffer_Mask4bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr maskBuffer);

	// 16-bit blitters
void BlitGraphicsBuffer_16bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR);

void BlitGraphicsBuffer_Transparent16bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr _notUsed);

// ===========================================================================

	// Utilities
void SetMonitorDepth(GDHandle monitor, short depth, Boolean color);

// ---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GRAPHICSBUFFER_H_
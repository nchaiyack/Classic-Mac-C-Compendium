/*
	GraphicsBuffer.c file
	by Hiep Dam
	Version 3.0
	Last update: June 1995
	
	Copyright ©1995 by Hiep Dam, All Rights Reserved
	You may freely use GraphicsBuffers.h, GraphicsBuffers.c
	in your own applications.
*/

// ---------------------------------------------------------------------------

#ifndef __PALETTES__
	#include <Palettes.h>
#endif

#include "GraphicsBuffers.h"

// ---------------------------------------------------------------------------

// Blitting info; only routines that have both a C and 680x0 asm version
// uses this structure. Eventually I'll have all the pixel copying routines
// using this. This structure is long-word aligned.
 
typedef struct {
	unsigned long widthChunksToCopy;
	unsigned long srcRowOffset;
	unsigned long destRowOffset;
	long rowsToCopy;
	long destMem;
	long srcMem;					// typecast to char*, short*, or long*
} BlitInfo;

static BlitInfo sBlitInfo;

// Internal stuff; again, related to the blitters only.
static SignedByte gMMUMode = true32b;
static Boolean gNotIn32Mode = false;

// ---------------------------------------------------------------------------

static OSErr UpdateCachedData(GraphicsBufferPtr buffer, long cache, Rect *bounds);
static void CopyMemoryFast_Transparent8bit();
static void CopyMemoryFast_Transparent16bit();

// ---------------------------------------------------------------------------

#define PtrCopy(s, d) (*(d)++ = *(s)++)
#define PtrMaskCopy(s, d, m) (*(d)++ = (*(d) & *(m)++) | *(s)++)

// ---------------------------------------------------------------------------

/* Code switches */

// Comment out if you only want to use the C versions, otherwise
// the routines with the 680x0 asm versions will be used.
#define USE_ASM

// ==========================================================================

OSErr InitGraphicsBuffers() {
	gMMUMode = GetMMUMode();
	gNotIn32Mode = (gMMUMode != true32b);

	return(noErr);
} // END InitGraphicsBuffers

// ---------------------------------------------------------------------------

OSErr NewGraphicsBuffer(
	GraphicsBufferPtr *buffer,
	long pixelDepth,
	Rect *boundsRect,
	long flags,
	long cache)
/*
	GraphicsBufferPtr *buffer
		Supply the address of a GraphicsBufferPtr. NewGraphicsBuffer will
		allocate a GraphicsBuffer for you.
	long pixelDepth
		Depth of the GraphicsBuffer. 1, 2, 4, 8 16, and 32 are valid values.
		It is suggested that you use 4, 8 or 16. There will probably no
		blitter support for 1, 2 and 32 bit depths.
	Rect *boundsRect
		The boundary rectangle of the GraphicsBuffer, in *global* coordinates.
	long flags
		The same flags as used in NewGWorld (GWorldFlags). Valid ones for
		NewGraphicsBuffer include: pixPurge, noNewDevice, useTempMem,
		and keepLocal. By far you'll use noNewDevice the most.
	long cache
		Pass TRUE or FALSE. If true, NewGraphicsBuffer will allocate some
		data for some precalculations. If you're planning to use any of the
		blitters, you *must* pass true. If you'll be using CopyBits/CopyMask
		instead, pass false.
*/
{
		// Local variables
	GraphicsBufferPtr gb;
	OSErr err = noErr;

	*buffer = NULL;

	/* Allocate the small chunk of memory for a GraphicsBuffer structure */
	gb = (GraphicsBufferPtr)NewPtrClear(sizeof(GraphicsBuffer));
	if (gb == NULL)
		return(MemError());

	gb->gworld = NULL;
	err = NewGWorld(&gb->gworld, pixelDepth, boundsRect, NULL, NULL, flags);
	if (err != noErr)
		return(err);
	else {
		err = UpdateCachedData(gb, cache, boundsRect);
	}

	*buffer = gb;
	return(err);
} // END NewGraphicsBuffer

// ---------------------------------------------------------------------------

OSErr UpdateGraphicsBuffer(
	GraphicsBufferPtr buffer,
	long pixelDepth,
	Rect *boundsRect,
	long flags,
	long cache)
/*
	See NewGraphicsBuffer() for explanation of the arguments.

	NOTE: For argument buffer, onÊinput it should be a pointer to an
	*existing* GraphicsBuffer.
	Notice the difference in arguments here from UpdateGWorld. In
	UpdateGWorld, you pass a pointer to a GWorldPtr; here you just
	pass a pointer to a GraphicsBuffer (not a pointer to a GraphicsBufferPtr).

	Most of the time you'll want to call this after first creating your
	GraphicsBuffer, and then having to change the depth of your monitor
	and buffer. Pass a depth of 0 and flags of 0.
*/
{
	OSErr err;
	GWorldFlags gwErr;
	
	if (buffer == NULL) return(paramErr);

	UnlockGraphicsBuffer(buffer);
	gwErr = UpdateGWorld(&buffer->gworld, pixelDepth, boundsRect, NULL, NULL, flags);
	if (gwErr == gwFlagErr)
		return(QDError());

	/* Update cached data */
	if (buffer->rowAddressOffsets != NULL) {
		DisposePtr((Ptr)buffer->rowAddressOffsets);
	}
	err = UpdateCachedData(buffer, cache, boundsRect);

	return(noErr);
} // END UpdateGraphicsBuffer

// ---------------------------------------------------------------------------

OSErr UpdateCachedData(GraphicsBufferPtr buffer, long cache, Rect *bounds)
/*
	Note: this routine will not work with buffers of type kVideoMemoryBuffer!
*/
{
	GWorldPtr saveWorld; GDHandle saveDev;

	(void)LockPixels(GetGWorldPixMap(buffer->gworld));

		// Make it nice 'n clean: erase gworld first
	GetGWorld(&saveWorld, &saveDev);
	SetGWorld(buffer->gworld, NULL);
	EraseRect(&buffer->localBounds);
	SetGWorld(saveWorld, saveDev);

	buffer->type = kGraphicsBuffer;
	buffer->window = NULL;

	buffer->pixmap = GetGWorldPixMap(buffer->gworld);
	/* The rowBytes value in the pixmap can't be used directly. The highest
	   bit is used to determine whether it's a bitmap or pixmap. So we have
	   to strip this value off, using 7FFF -> 0111 1111 1111 1111
	*/
	buffer->realRowBytes = (0x7FFF & (**(buffer->pixmap)).rowBytes);
	buffer->rowBytesDiv4 = buffer->realRowBytes >> 2;
	buffer->rowBytesDiv2 = buffer->realRowBytes >> 1;

	SetRect(&buffer->videoBounds, 0, 0, 0, 0);
	buffer->localBounds = *bounds;
	buffer->globBounds = *bounds;
	OffsetRect(&buffer->localBounds, -buffer->localBounds.left,
		-buffer->localBounds.top);
	if (buffer->globBounds.left != 0 || buffer->globBounds.top != 0)
		buffer->bufferFlushedTopLeft = false;
	else
		buffer->bufferFlushedTopLeft = true;

	if (cache) {
		short bufferHeight;
		short i;

		buffer->isCached = true;

		// Create our pre-calculated "realRowBytes * vertCoords"
		bufferHeight = buffer->localBounds.bottom - buffer->localBounds.top;
		buffer->rowAddressOffsets = (unsigned long*)NewPtr
			(sizeof(unsigned long) * bufferHeight);
		if (buffer->rowAddressOffsets == NULL)
			return(MemError());

		/* Now do the actual precalculations: long multiply */
		for (i = 0; i < bufferHeight; i++)
			buffer->rowAddressOffsets[i] = buffer->realRowBytes * (long)i;
	}
	else {
		/* If there is no cache, update flag */
		if (buffer->rowAddressOffsets == NULL)
			buffer->isCached = false;
		else
			buffer->isCached = true;
	}
	
	return(noErr);
} // END UpdateCachedData

// ---------------------------------------------------------------------------

OSErr DisposeGraphicsBuffer(GraphicsBufferPtr buffer) {
	if (buffer == NULL)
		return(paramErr);

	/* Dispose contents of GraphicsBuffer */
	if (buffer->gworld != NULL) {
		UnlockPixels(buffer->pixmap);
		DisposeGWorld(buffer->gworld);
	}
	if (buffer->rowAddressOffsets != NULL)
		DisposePtr((Ptr)buffer->rowAddressOffsets);

	/* Dispose GraphicsBuffer itself */
	DisposePtr((Ptr)buffer);

	return(noErr);
} // END DisposeGraphicsBuffer

// ---------------------------------------------------------------------------

OSErr Convert2GraphicsBuffer(
	GraphicsBufferPtr *buffer,
	WindowPtr srcWind,
	Rect *bounds,
	long cache)
/*
	Bounds should be in global coordinates of the window.
*/
{
		// Some local variables...
	Rect testRect;
	GraphicsBufferPtr gb;
	GDHandle monitor;
	long i;

	*buffer = NULL;

	gb = (GraphicsBufferPtr)NewPtrClear(sizeof(GraphicsBuffer));
	if (gb == NULL)
		return(MemError());

		// We're creating a GraphicsBuffer from window and monitor data, so we
		// won't need to create a gworld.
	gb->gworld = NULL;
	gb->window = srcWind;

	gb->type = kVideoMemoryBuffer;

	/* Given the global bounds of the window, find the corresponding
	   monitor; we'll assume the window lies wholly inside one monitor
	   only and it doesn't intersect any other monitors */
	monitor = GetDeviceList();
	while (monitor != NULL) {
		testRect = (**monitor).gdRect;
		if (SectRect(bounds, &testRect, &testRect))
			break;
		monitor = GetNextDevice(monitor);
	}
	if (monitor == NULL)
		return(paramErr);

	gb->pixmap = (**monitor).gdPMap;
	gb->realRowBytes = (0x7FFF & (**(gb->pixmap)).rowBytes);
	gb->rowBytesDiv4 = gb->realRowBytes >> 2;
	gb->rowBytesDiv2 = gb->realRowBytes >> 1;

	gb->videoBounds = (**monitor).gdRect; // or (**gb->pixmap).bounds;
	gb->localBounds = *bounds;
	gb->globBounds = *bounds;
	OffsetRect(&gb->localBounds, -gb->localBounds.left,
		-gb->localBounds.top);
	if (gb->globBounds.left != 0 || gb->globBounds.top != 0)
		gb->bufferFlushedTopLeft = false;
	else
		gb->bufferFlushedTopLeft = true;

	if (cache) {
		short bufferHeight = gb->videoBounds.bottom - gb->videoBounds.top;
		gb->isCached = true;

		// Create our pre-calculated "realRowBytes * vertCoords"
		gb->rowAddressOffsets = (unsigned long*)NewPtr
			(sizeof(unsigned long) * (bufferHeight));
		if (gb->rowAddressOffsets == NULL)
			return(MemError());
	
		for (i = 0; i < bufferHeight; i++)
			gb->rowAddressOffsets[i] = gb->realRowBytes * (long)i;
	}
	else {
		gb->isCached = false;
		gb->rowAddressOffsets = NULL;
	}

	*buffer = gb;
	return(noErr);
} // END Convert2GraphicsBuffer

// ---------------------------------------------------------------------------

void SetGraphicsBuffer(GraphicsBufferPtr buffer) {
/*
	You can call this yourself to remove the overhead...
	Analogous to SetGWorld()
*/
	if (buffer->gworld != NULL)
		SetGWorld(buffer->gworld, NULL);
} // END SetGraphicsBuffer

// ---------------------------------------------------------------------------

Boolean LockGraphicsBuffer(GraphicsBufferPtr buffer) {
/*
	You can call this yourself to remove the overhead...
	Analogous to LockGWorld()
*/
	if (buffer->gworld != NULL)
		return(LockPixels(buffer->pixmap));
	else
		return(false);
} // END LockGraphicsBuffer

// ---------------------------------------------------------------------------

void UnlockGraphicsBuffer(GraphicsBufferPtr buffer) {
/*
	You can call this yourself to remove the overhead...
	Analogous to UnlockGWorld()
*/
	if (buffer->gworld != NULL)
		UnlockPixels(buffer->pixmap);
} // END UnlockGraphicsBuffer

// ---------------------------------------------------------------------------

void CopyGraphicsBuffer(GraphicsBufferPtr srcBuffer, GraphicsBufferPtr destBuffer,
						Rect *srcR, Rect *destR) {
	if (destBuffer->gworld != NULL)
		// Just copying between pixmaps.
		CopyBits((BitMap*)(*srcBuffer->pixmap),
			(BitMap*)(*destBuffer->pixmap), srcR, destR, srcCopy, NULL);
	else if (destBuffer->window != NULL)
		// Copying to window
		CopyBits((BitMap*)(*srcBuffer->pixmap),
			(&destBuffer->window->portBits), srcR, destR, srcCopy, NULL);
	else {
		// do nothing...
		SysBeep(10);
	}
} // END CopyGraphicsBuffer

void CopyGraphicsBuffer2Window(GraphicsBufferPtr srcBuffer, WindowPtr destWind,
								Rect *srcR, Rect *destR) {
	if (srcBuffer->gworld != NULL)
		CopyBits((BitMap*)(*srcBuffer->pixmap), &destWind->portBits, 
				 srcR, destR, srcCopy, NULL);
	else if (srcBuffer->window != NULL)
		CopyBits((&srcBuffer->window->portBits), &destWind->portBits, 
				 srcR, destR, srcCopy, NULL);
	else {
		SysBeep(10);
	}
} // END CopyGraphicsBuffer2Window

void CopyWindow2GraphicsBuffer(WindowPtr srcWind, GraphicsBufferPtr destBuffer,
								Rect *srcR, Rect *destR) {
	if (destBuffer->gworld != NULL)
		CopyBits(&srcWind->portBits, (BitMap*)(*destBuffer->pixmap), 
				 srcR, destR, srcCopy, NULL);
	else if (destBuffer->window != NULL)
		CopyBits(&srcWind->portBits, (&destBuffer->window->portBits), 
				 srcR, destR, srcCopy, NULL);
	else {
		SysBeep(10);
	}
} // END CopyWindow2GraphicsBuffer

// ---------------------------------------------------------------------------

void CopyGraphicsBufferRegion(GraphicsBufferPtr srcBuffer, GraphicsBufferPtr destBuffer,
						  Rect *srcR, Rect *destR, RgnHandle maskRgn) {
	if (destBuffer->gworld != NULL)
		CopyBits((BitMap*)(*srcBuffer->pixmap),
			(BitMap*)(*destBuffer->pixmap), srcR, destR, srcCopy, maskRgn);
	else if (destBuffer->window != NULL)
		CopyBits((BitMap*)(*srcBuffer->pixmap),
			(&destBuffer->window->portBits), srcR, destR, srcCopy, maskRgn);
} // END CopyGraphicsBufferRegion

void CopyGraphicsBufferMask(GraphicsBufferPtr srcBuffer, GraphicsBufferPtr destBuffer,
						Rect *srcR, Rect *destR, GraphicsBufferPtr maskBuffer) {
	if (destBuffer->gworld != NULL)
		CopyMask((BitMap*)(*srcBuffer->pixmap),
			(BitMap*)(*maskBuffer->pixmap),
			(BitMap*)(*destBuffer->pixmap), srcR, srcR, destR);
	else if (destBuffer->window != NULL)
		CopyMask((BitMap*)(*srcBuffer->pixmap),
			(BitMap*)(*maskBuffer->pixmap),
			(&destBuffer->window->portBits), srcR, srcR, destR);
} // END CopyGraphicsBufferMask

void CopyGraphicsBufferTransparent(GraphicsBufferPtr srcBuffer, GraphicsBufferPtr destBuffer,
						Rect *srcR, Rect *destR, GraphicsBufferPtr _notUsed) {
	if (destBuffer->gworld != NULL)
		// Just copying between pixmaps.
		CopyBits((BitMap*)(*srcBuffer->pixmap),
			(BitMap*)(*destBuffer->pixmap), srcR, destR, transparent, NULL);
	else if (destBuffer->window != NULL)
		// Copying to window
		CopyBits((BitMap*)(*srcBuffer->pixmap),
			(&destBuffer->window->portBits), srcR, destR, transparent, NULL);
	else {
		// do nothing...
	}
} // END CopyGraphicsBufferTransparent

// ==========================================================================

// Comment out if you don't need clipping (you're sure your rect won't
// be out of bounds of the pixmap's boundaries; if so you'll be writing
// to other areas of memory, a dangerous situation).
#define BLIT_CLIPPING

// Comment out if you're sure the rects you pass to the blitters
// aren't empty. If they are, the blitters will loop forever!
#define BLIT_CHECKEMPTY

// ---------------------------------------------------------------------------

#ifdef BLIT_CLIPPING
	#define BlitClipBounds(srcRect, destRect, buffer)						\
		if (destRect.top < buffer->localBounds.top) {						\
			srcRect.top += buffer->localBounds.top - destRect.top;			\
			destRect.top = buffer->localBounds.top;							\
			if (destRect.top >= destRect.bottom) return;					\
		}																	\
		else if (destRect.bottom > buffer->localBounds.bottom) {			\
			srcRect.bottom -= destRect.bottom - buffer->localBounds.bottom; \
			destRect.bottom = buffer->localBounds.bottom;					\
			if (destRect.bottom <= destRect.top) return;					\
		}																	\
		if (destRect.left < buffer->localBounds.left) {						\
			srcRect.left += buffer->localBounds.left - destRect.left;		\
			destRect.left = buffer->localBounds.left;						\
			if (destRect.left >= destRect.right) return;					\
		}																	\
		else if (destRect.right > buffer->localBounds.right) {				\
			srcRect.right -= destRect.right - buffer->localBounds.right;	\
			destRect.right = buffer->localBounds.right;						\
			if (destRect.right <= destRect.left ) return;					\
		}																	\
		if (buffer->type == kVideoMemoryBuffer && 							\
			buffer->bufferFlushedTopLeft == false) { 						\
			FastOffsetRect(destRect, buffer->globBounds.left, buffer->globBounds.top);\
		}
#else
	#define BlitClipBounds(srcRect, destRect, buffer)
#endif


#ifdef BLIT_CHECKEMPTY
	#define CheckEmptyRect(destRect)				\
		if (destRect.right <= destRect.left ||		\
			destRect.bottom <= destRect.top) return
#else
	#define CheckEmptyRect(destRect)
#endif

// ---------------------------------------------------------------------------

void BlitGraphicsBuffer_8bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR)
/*
	Assumes: 8 bit! (256 colors)
			 srcR and destR are the same size, dimensions.
			 Width is a multiple of 4 [pixels]!
			 If destBuffer is to screen, make sure cursor is hidden!
*/
{
	register long* srcMem;
	register long* destMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long numRowLongs;
	register unsigned long numBytesPerRow;

	Rect srcRect = *srcR;
	Rect destRect = *destR;

		// Clip off edges so we don't draw more than we have to
		// (besides, not doing so we might draw into other areas
		// of memory, a big no-no).
	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;

			// Calculate starting addresses
		srcMem = (long*)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) + srcRect.left);
		destMem = (long*)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) + destRect.left);

			// Calculate the number of rows to blit (copy)
		rowsToCopy = destRect.bottom - destRect.top;
			// Calculate the width of the row
		numBytesPerRow = destRect.right - destRect.left;

			// How many longs in the width of the row; this is
			// a fast numBytesPerRow / 4
		numRowLongs = numBytesPerRow >> 2;
			// How many pixels left over that don't fit into
			// a multiple of four (a fast numBytesPerRow % 4)
		//pixelsLeftOver = (long)(numBytesPerRow & 3);

			// Calculate the offset from one end of one row to
			// beginning of next row
		srcRowOffset = (srcBuffer->rowBytesDiv4) - numRowLongs;
			// Do it for the dest image...
		destRowOffset = (destBuffer->rowBytesDiv4) - numRowLongs;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		do {
			loopsPerRow = numRowLongs;

			do {
				PtrCopy(srcMem, destMem);
			} while (--loopsPerRow);

				// Bump to start of next row
			srcMem += srcRowOffset;
			destMem += destRowOffset;
		} while (--rowsToCopy);

		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_8bit

// ---------------------------------------------------------------------------

void BlitGraphicsBuffer_Mask8bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr maskBuffer)
/*
	Assumes: 8 bit! (256 colors)
			 srcR and destR are the same size, dimensions
			 srcBuffer, destBuffer, and *maskBuffer* are all 8-bit!
			 Width is a multiple of 4 [pixels]!
			 If destBuffer is to screen, make sure cursor is hidden!
*/
{
	register long* srcMem;
	register long* destMem;
	register long* maskMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long numRowLongs;
	unsigned long numBytesPerRow;

	Rect srcRect = *srcR;
	Rect destRect = *destR;

	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;

			// Calculate starting addresses
		srcMem = (long*)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) + srcRect.left);
		destMem = (long*)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) + destRect.left);
		maskMem = (long*)(GetPixBaseAddr(maskBuffer->pixmap) +
				(maskBuffer->rowAddressOffsets[srcRect.top]) + srcRect.left);

			// Calculate the number of rows to blit (copy)
		rowsToCopy = destRect.bottom - destRect.top;
		numBytesPerRow = destRect.right - destRect.left;
		numRowLongs = numBytesPerRow >> 2;

			// Calculate the offset from one end of one row to beginning of next row
		srcRowOffset = (srcBuffer->rowBytesDiv4) - numRowLongs;
			// Do it for the dest image...
		destRowOffset = (destBuffer->rowBytesDiv4) - numRowLongs;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		do {
			loopsPerRow = numRowLongs;

			do {
				PtrMaskCopy(srcMem, destMem, maskMem);
			} while (--loopsPerRow);
	
				// Bump to start of next row
			srcMem += srcRowOffset;
			maskMem += srcRowOffset;
			destMem += destRowOffset;
		} while (--rowsToCopy);
		
		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_Mask8bit

// ==========================================================================

void BlitGraphicsBuffer_4bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR)
/*
	Assumes: 4 bit! (16 colors)
			 srcR and destR are the same size, dimensions
			 Width must be a multiple of 8 [pixels]!
			 Left coordinates of the rectangles *must* be even.
*/
{
	register long* srcMem;
	register long* destMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long numRowLongs;
	unsigned long numBytesPerRow;

	Rect srcRect = *srcR;
	Rect destRect = *destR;
	
	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;

		numBytesPerRow = (destRect.right - destRect.left);
			// For 4-bit, we divide numBytesPerRow by 8. Unfortunately, this
			// means rect width must be a multiple of 8!!
		numRowLongs = numBytesPerRow >> 3;

			// Calculate starting addresses; we also
			// have to divide the left coords by 2 for
			// 4-bit blitting. This means the left coords
			// of your source, dest rects must be even!
		srcMem = (long*)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) +
				(srcRect.left >> 1));
		destMem = (long*)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) +
				(destRect.left >> 1));

			// Calculate the number of rows to blit (copy)
		rowsToCopy = destRect.bottom - destRect.top;

			// Calculate the offset from one end of one row to
			// beginning of next row
		srcRowOffset = (srcBuffer->rowBytesDiv4) - numRowLongs;
			// Do it for the dest image...
		destRowOffset = (destBuffer->rowBytesDiv4) - numRowLongs;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		do {
			loopsPerRow = numRowLongs;

			do {
				PtrCopy(srcMem, destMem);
			} while (--loopsPerRow);
	
			// Bump to start of next row
			srcMem += srcRowOffset;
			destMem += destRowOffset;
		} while (--rowsToCopy);

		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_4bit

// ---------------------------------------------------------------------------

void BlitGraphicsBuffer_Mask4bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr maskBuffer)
/*
	Assumes: 4 bit! (16 colors)
			 srcR and destR are the same size, dimensions
			 srcBuffer, destBuffer, and maskBuffer are all 4-bit!
*/
{
	register long* srcMem;
	register long* destMem;
	register long* maskMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long numRowLongs;
	unsigned long numBytesPerRow;

	Rect srcRect = *srcR;
	Rect destRect = *destR;
	
	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;
		short leftEdge = srcRect.left >> 1;
		numBytesPerRow = (destRect.right - destRect.left);
		numRowLongs = numBytesPerRow >> 3;

			// Calculate starting addresses
		srcMem = (long*)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) +
				leftEdge);
		destMem = (long*)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) +
				(destRect.left >> 1));
		maskMem = (long*)(GetPixBaseAddr(maskBuffer->pixmap) +
				(maskBuffer->rowAddressOffsets[srcRect.top]) +
				leftEdge);

			// Calculate the number of rows to blit (copy)
		rowsToCopy = destRect.bottom - destRect.top;

			// Calculate the offset from one end of one row to beginning of next row
		srcRowOffset = (srcBuffer->rowBytesDiv4) - numRowLongs;
			// Do it for the dest image...
		destRowOffset = (destBuffer->rowBytesDiv4) - numRowLongs;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		do {
			loopsPerRow = numRowLongs;

			do {
				PtrMaskCopy(srcMem, destMem, maskMem);
			} while (--loopsPerRow);
	
			// Bump to start of next row
			srcMem += srcRowOffset;
			maskMem += srcRowOffset;
			destMem += destRowOffset;
		} while (--rowsToCopy);
		
		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_Mask4bit

// ==========================================================================

void BlitGraphicsBuffer_16bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR)
/*
	Assumes: 16 bit!
			 srcR and destR are the same size, dimensions.
			 Width must be a multiple of 2!
*/
{
	register long* srcMem;
	register long* destMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long numRowLongs;
	//register long pixelsLeftOver;
	register unsigned long numBytesPerRow;

	Rect srcRect = *srcR;
	Rect destRect = *destR;

		// Clip off edges so we don't draw more than we have to
		// (besides, not doing so we might draw into other areas
		// of memory, a big no-no).
	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;

			// Calculate starting addresses
		srcMem = (long*)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) + (srcRect.left << 1));
		destMem = (long*)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) + (destRect.left << 1));

			// Calculate the number of rows to blit (copy)
		rowsToCopy = destRect.bottom - destRect.top;
			// Calculate the width of the row
		numBytesPerRow = destRect.right - destRect.left;

			// How many longs in the width of the row; this is
			// a fast numBytesPerRow / 4
		numRowLongs = numBytesPerRow >> 1;
			// How many pixels left over that don't fit into
			// a multiple of four (a fast numBytesPerRow % 4)
		//pixelsLeftOver = (long)(numBytesPerRow & 3);

			// Calculate the offset from one end of one row to
			// beginning of next row
		srcRowOffset = (srcBuffer->rowBytesDiv4) - numRowLongs;
			// Do it for the dest image...
		destRowOffset = (destBuffer->rowBytesDiv4) - numRowLongs;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		do {
			loopsPerRow = numRowLongs;

			do {
				PtrCopy(srcMem, destMem);
			} while (--loopsPerRow);

				// Bump to start of next row
			srcMem += srcRowOffset;
			destMem += destRowOffset;
		} while (--rowsToCopy);

		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_16bit

// ---------------------------------------------------------------------------

void BlitGraphicsBuffer_Transparent8bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr _notUsed)
/*
	Assumes: 8 bit! (256 colors)
			 srcR and destR are the same size, dimensions.
			 If destBuffer is to screen, make sure cursor is hidden!

			 If you're using the assembly version, width must be a multiple of 4.
			 If you're using the C version, width can be any value.
			 
			 Note: maskPtr is NOT used; pass NULL. It is only there for common
			  calling conventions with the other routines. It's replaced by _notUsed
*/
{
	Rect srcRect = *srcR;
	Rect destRect = *destR;
	register unsigned long numBytesPerRow;

		// Clip off edges so we don't draw more than we have to
		// (besides, not doing so we might draw into other areas
		// of memory, a big no-no).
	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;

			// Calculate starting addresses
		sBlitInfo.srcMem = (long)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) + srcRect.left);
		sBlitInfo.destMem = (long)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) + destRect.left);

			// Calculate the number of rows to blit (copy)
		sBlitInfo.rowsToCopy = destRect.bottom - destRect.top;
			// Calculate the width of the row
		numBytesPerRow = destRect.right - destRect.left;

			// Here we'll be copying pretty much one byte at a time,
			// so widthChunksToCopy == num pixels per row == numBytesPerRow
		sBlitInfo.widthChunksToCopy = numBytesPerRow;

			// Calculate the offset from one end of one row to
			// beginning of next row
		sBlitInfo.srcRowOffset = (srcBuffer->realRowBytes) - numBytesPerRow;
			// Do it for the dest image...
		sBlitInfo.destRowOffset = (destBuffer->realRowBytes) - numBytesPerRow;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		CopyMemoryFast_Transparent8bit();

		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_Transparent8bit

#ifdef USE_ASM
asm void CopyMemoryFast_Transparent8bit()
/*
	A0	srcMem
	A1	destMem

	D0	loopsPerRow
	D1	numRowLongs
	D2	srcRowOffset
	D3	destRowOffset
	D4	rowsToCopy
*/
{
	fralloc+
	MOVE.L		D3, -(SP)
	MOVE.L		D4, -(SP)
	
	LEA			sBlitInfo, A0
	MOVE.L		(A0)+, D1		;// widthChunksToCopy
	MOVE.L		(A0)+, D2		;// srcRowOffset
	MOVE.L		(A0)+, D3		;// destRowOffset
	MOVE.L		(A0)+, D4		;// rowsToCopy
	MOVEA.L		(A0)+, A1		;// destMem
	MOVEA.L		(A0),  A0		;// srcMem

	SUBQ.L		#1, D4			;// rowsToCopy--

	@copy_start:
	MOVE.L		D1, D0			;// loopsPerRow = widthChunksToCopy
	SUBQ.L		#1, D0			;// loopsPerRow--

		@copy:
		TST.L		(A0)
		BEQ.S		@increment_long
		
		;// Hmm. Have to copy byte-by-byte
		TST.B		(A0)
		BEQ.S		@increment_firstbyte
		MOVE.B		(A0)+, (A1)+
		BRA.S		@copy_secondbyte
		@increment_firstbyte:
		ADDQ.L		#1, A0
		ADDQ.L		#1, A1

		@copy_secondbyte:
		TST.B		(A0)
		BEQ.S		@increment_secondbyte
		MOVE.B		(A0)+, (A1)+
		BRA.S		@copy_thirdbyte
		@increment_secondbyte:
		ADDQ.L		#1, A0
		ADDQ.L		#1, A1

		@copy_thirdbyte:
		TST.B		(A0)
		BEQ.S		@increment_thirdbyte
		MOVE.B		(A0)+, (A1)+
		BRA.S		@copy_fourthbyte
		@increment_thirdbyte:
		ADDQ.L		#1, A0
		ADDQ.L		#1, A1

		@copy_fourthbyte:
		TST.B		(A0)
		BEQ.S		@increment_fourthbyte
		MOVE.B		(A0)+, (A1)+
		BRA.S		@decrement
		@increment_fourthbyte:
		ADDQ.L		#1, A0
		ADDQ.L		#1, A1
		BRA.S		@decrement
		
		@increment_long:
		ADDQ.L		#4, A0
		ADDQ.L		#4, A1
		
		@decrement:
		SUBQ.L		#3, D0

		@end_copy:
		DBRA		D0, @copy

								;// All are byte values so below is OK
	ADDA.L		D2, A0			;// srcMem += srcRowOffset
	ADDA.L		D3, A1			;// destMem += destMemOffset
	DBRA		D4, @copy_start

	MOVE.L		(SP)+, D4
	MOVE.L		(SP)+, D3
	frfree
	RTS
} // END CopyMemoryFast_Transparent8bit
#else
void CopyMemoryFast_Transparent8bit()
/*
	The masking works by checking for a value of 0. If a pixel
	value is 0, this means that it's white and we skip it.
	Any other value and we copy. So any part of your sprite
	you want to be transparent, just make it white.

	Note: 0 is NOT the RGB value for white. 0 is the index
	into the device clut; on 8-bit devices index 0 of a clut
	is always *white*. It is important to realize this
	distinction, especially when using the 16-bit transparent blitter.
*/
{
	register unsigned char* srcMem;
	register unsigned char* destMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long widthChunksToCopy;

	widthChunksToCopy = sBlitInfo.widthChunksToCopy;
	srcRowOffset = sBlitInfo.srcRowOffset;
	destRowOffset = sBlitInfo.destRowOffset;
	rowsToCopy = sBlitInfo.rowsToCopy;
	srcMem = (unsigned char*)sBlitInfo.srcMem;
	destMem = (unsigned char*)sBlitInfo.destMem;

	do {
		loopsPerRow = widthChunksToCopy;

		do {
			if (*srcMem)
				*(destMem)++ = *(srcMem)++;
			else {
				destMem++; srcMem++;
			}
		} while (--loopsPerRow);

			// Bump to start of next row
		srcMem += srcRowOffset;
		destMem += destRowOffset;
	} while (--rowsToCopy);
} // END CopyMemoryFast_Transparent8bit
#endif

// ---------------------------------------------------------------------------

void BlitGraphicsBuffer_Transparent16bit(
	GraphicsBufferPtr srcBuffer,
	GraphicsBufferPtr destBuffer,
	Rect *srcR,
	Rect *destR,
	GraphicsBufferPtr _notUsed)
/*
	Assumes: 16 bit!
			 srcR and destR are the same size, dimensions
			 srcBuffer, destBuffer are all 16-bit!

			 If you're using the assembly version, width must be a 
			 	multiple of 2 [pixels]!
			 If you're using the C version, the width can be any value.

			 maskBuffer is NOT used. Pass NULL instead.
*/
{
	unsigned long numPixelsPerRow;

	Rect srcRect = *srcR;
	Rect destRect = *destR;

	BlitClipBounds(srcRect, destRect, destBuffer);
	CheckEmptyRect(destRect);

	{
		SignedByte mmuMode;

			// Calculate starting addresses
		sBlitInfo.srcMem = (long)(GetPixBaseAddr(srcBuffer->pixmap) +
				(srcBuffer->rowAddressOffsets[srcRect.top]) + (srcRect.left << 1));
		sBlitInfo.destMem = (long)(GetPixBaseAddr(destBuffer->pixmap) +
				(destBuffer->rowAddressOffsets[destRect.top]) + (destRect.left << 1));

			// Calculate the number of rows to blit (copy)
		sBlitInfo.rowsToCopy = destRect.bottom - destRect.top;
		numPixelsPerRow = destRect.right - destRect.left;
		sBlitInfo.widthChunksToCopy = numPixelsPerRow;

			// Calculate the offset from one end of one row to beginning of next row
		sBlitInfo.srcRowOffset = (srcBuffer->rowBytesDiv2) - numPixelsPerRow;
			// Do it for the dest image...
		sBlitInfo.destRowOffset = (destBuffer->rowBytesDiv2) - numPixelsPerRow;

		if (gNotIn32Mode) {
			mmuMode = true32b;
			SwapMMUMode(&mmuMode);
		}

		CopyMemoryFast_Transparent16bit();

		if (gNotIn32Mode)
			SwapMMUMode(&mmuMode);
	}
} // END BlitGraphicsBuffer_Transparent16bit

// ---------------------------------------------------------------------------

//#undef USE_ASM

#if defined(USE_ASM)
asm void CopyMemoryFast_Transparent16bit()
/*
	A0	srcMem
	A1	destMem

	D0	loopsPerRow
	D1	numRowLongs
	D2	srcRowOffset
	D3	destRowOffset
	D4	rowsToCopy
*/
{
	fralloc+
	MOVE.L		D3, -(SP)
	MOVE.L		D4, -(SP)

	LEA			sBlitInfo, A0
	MOVE.L		(A0)+, D1		;// widthChunksToCopy
	MOVE.L		(A0)+, D2		;// srcRowOffset
	MOVE.L		(A0)+, D3		;// destRowOffset
	MOVE.L		(A0)+, D4		;// rowsToCopy
	MOVEA.L		(A0)+, A1		;// destMem
	MOVEA.L		(A0),  A0		;// srcMem

	SUBQ.L		#1, D4			;// rowsToCopy--

	@copy_start:
	MOVE.L		D1, D0			;// loopsPerRow = widthChunksToCopy

		@copy:

		CMPI.L		#0x7FFF7FFF, (A0)
		BEQ.S		@increment_long

		;// Test first pixel (first word)
		CMPI.W		#0x7FFF, (A0)
		BEQ.S		@increment_firstword
		MOVE.W		(A0)+, (A1)+
		BRA.S		@test_secondword
		@increment_firstword:
		ADDQ.L		#2, A0
		ADDQ.L		#2, A1
		
		;// Test second pixel (second word)
		@test_secondword:
		CMPI.W		#0x7FFF, (A0)
		BEQ.S		@increment_secondword
		MOVE.W		(A0)+, (A1)+
		BRA.S		@end_copy
		@increment_secondword:
		ADDQ.L		#2, A0
		ADDQ.L		#2, A1
		BRA.S		@end_copy

		@increment_long:
		ADDQ.L		#4, A0
		ADDQ.L		#4, A1

		@end_copy:
		SUBQ.L		#2, D0
		BNE.S		@copy

	;// Note: srcRowOffset, destMemOffset is in same memory chunks as srcMem
	;// and destMem. So if srcMem is a short*, srcRowOffset is in shorts;
	;// if srcMem is a char*, srcRowOffset is in bytes; etc. We have to adjust
	;// the adding accordingly (pointer arithmetic).


	ADDA.L		D2, A0			;// srcMem += srcRowOffset
	ADDA.L		D2, A0
	ADDA.L		D3, A1			;// destMem += destMemOffset
	ADDA.L		D3, A1
	DBF			D4, @copy_start

	MOVE.L		(SP)+, D4
	MOVE.L		(SP)+, D3
	frfree
	RTS
} // END CopyMemoryFast_Transparent16bit
#else
void CopyMemoryFast_Transparent16bit()
/*
	This transparent blitter works exactly the same as the
	8-bit transparent blitter in concept. It looks for a specific
	pixel value-if it finds the value it doesn't copy; any
	other value and it copies the pixel. Because we're working
	on 16-bit colors, to copy 1 pixel at a time we have to copy
	2 bytes per shot (eg a short).
	
	Note however this blitter differs on what the pixel value is
	for white. For 8-bit blitters it's zero (0). This is the index
	into the clut for white; the RGB value being r=g=b=0xFFFFFFFF (65535).
	For the 16-bit blitter, it's the RGB value itself, 0xFFFF.
	However, it's not that simple. The RGB value is a 48-bit data
	chunk (red+green+blue = 16+16+16). This RGB value is "truncated"
	to fit inside the 16-bit value of a pixel. The actual method is
	easy; it's too long to explain here (see IM:Imaging w/ QuickDraw).
	Just suffice to say that the highest bit is unused. So instead of
	white being the value 0xFFFF in 16-bit, it's actually 0x7FFF.
	This is the value we'll be using to check for white...
*/
{
	register unsigned short* srcMem;
	register unsigned short* destMem;
	register long rowsToCopy;
	register unsigned long loopsPerRow;
	register unsigned long srcRowOffset;
	register unsigned long destRowOffset;
	register long widthChunksToCopy;

	widthChunksToCopy = sBlitInfo.widthChunksToCopy;
	srcRowOffset = sBlitInfo.srcRowOffset;
	destRowOffset = sBlitInfo.destRowOffset;
	rowsToCopy = sBlitInfo.rowsToCopy;
	srcMem = (unsigned short*)sBlitInfo.srcMem;
	destMem = (unsigned short*)sBlitInfo.destMem;

	do {
		loopsPerRow = widthChunksToCopy;

		do {
			if ((*srcMem == 0x7FFF)) { // e.g. 0x7FFF == 32767 (RGB white)
				destMem++; srcMem++;
			}
			else {
				*destMem++ = *srcMem++;
			}
		} while (--loopsPerRow);

			// Bump to start of next row
		srcMem += srcRowOffset;
		destMem += destRowOffset;
	} while (--rowsToCopy);
} // END CopyMemoryFast_Transparent16bit
#endif

// ==========================================================================

void SetMonitorDepth(GDHandle monitor, short depth, Boolean color) {
	short flags;

	if (monitor != NULL) {
		flags = 1 << gdDevType;
		(void)SetDepth(monitor, depth, flags, color ? 1 : 0);
	}
} // END SetMonitorDepth

// ---------------------------------------------------------------------------
// ==========================================================================

// END GraphicsBuffers.c
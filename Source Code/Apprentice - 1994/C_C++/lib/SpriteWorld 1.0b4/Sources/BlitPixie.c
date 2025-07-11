///--------------------------------------------------------------------------------------
//	BlitPixie.c
//
//	Created:	Thursday, September 24, 1992 at 11:48 PM
//	By:		Tony Myles
//
//	Ideas and code snippets contributed by:
//		Ben Sharpe, Brigham Stevens, Sean Callahan, and Joe Britt
//
//	Copyright: � 1991-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#ifndef __FRAME__
#include "Frame.h"
#endif

#ifndef __BLITPIXIE__
#include "BlitPixie.h"
#endif

#ifndef __SPRITECOMPILER__
#include "SpriteCompiler.h"
#endif

#include "DebugUtils.h"


///--------------------------------------------------------------------------------------
//		BlitPixieSpriteDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void BlitPixieSpriteDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect)
{
	Rect srcBlitRect = *srcRect;
	Rect dstBlitRect = *dstRect;

		// clip off the top so we dont write into random memory
	if (dstBlitRect.top < dstFrameP->frameRect.top)
	{
		srcBlitRect.top += dstFrameP->frameRect.top - dstBlitRect.top;
		dstBlitRect.top = dstFrameP->frameRect.top;
		if (dstBlitRect.top >= dstBlitRect.bottom) return;
	}
		// clip off the bottom
	else if (dstBlitRect.bottom > dstFrameP->frameRect.bottom)
	{
		dstBlitRect.bottom = dstFrameP->frameRect.bottom;
		if (dstBlitRect.bottom <= dstBlitRect.top) return;
	}

		// clip off the left
	if (dstBlitRect.left < dstFrameP->frameRect.left)
	{
		srcBlitRect.left += dstFrameP->frameRect.left - dstBlitRect.left;
		dstBlitRect.left = dstFrameP->frameRect.left;
		if (dstBlitRect.left >= dstBlitRect.right) return;
	}
		// clip off the right
	else if (dstBlitRect.right > dstFrameP->frameRect.right)
	{
		dstBlitRect.right = dstFrameP->frameRect.right;
		if (dstBlitRect.right <= dstBlitRect.left) return;
	}

	{
		unsigned long numBytesPerRow;

			// calculate the number of bytes in a row
		numBytesPerRow = (dstBlitRect.right - dstBlitRect.left);

		BlitPixie(
				// calculate the address of the first byte of the source
			(PixelChunkPtr)(srcFrameP->frameBaseAddr + 
				(srcFrameP->scanLinePtrArray[srcBlitRect.top]) + srcBlitRect.left),

				// calculate the address of the first byte of the destination
			(PixelChunkPtr)(dstFrameP->frameBaseAddr + 
				(dstFrameP->scanLinePtrArray[dstBlitRect.top]) + dstBlitRect.left),

				// calculate the number of rows to blit
			dstBlitRect.bottom - dstBlitRect.top,

				// number of bytes in a row (duh!)
			numBytesPerRow,

				// calculate offset from end of one row to beginning of next row
			srcFrameP->frameRowBytes - numBytesPerRow,
			dstFrameP->frameRowBytes - numBytesPerRow);
	}
}


///--------------------------------------------------------------------------------------
//		BlitPixieWorldDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void BlitPixieWorldDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *blitRect)
{
	Rect dstBlitRect = *blitRect;
	Rect srcBlitRect = *blitRect;

		// clip off the top so we dont write into random memory
	if (dstBlitRect.top <  dstFrameP->frameRect.top)
	{
		srcBlitRect.top += dstFrameP->frameRect.top - dstBlitRect.top;
		dstBlitRect.top =  dstFrameP->frameRect.top;
		if (dstBlitRect.top >= dstBlitRect.bottom) return;	
	}
		// clip off the bottom
	else if (dstBlitRect.bottom >  dstFrameP->frameRect.bottom)
	{
		dstBlitRect.bottom =  dstFrameP->frameRect.bottom;
		if (dstBlitRect.bottom <= dstBlitRect.top) return;
	}

		// clip off the left
	if (dstBlitRect.left <  dstFrameP->frameRect.left)
	{
		srcBlitRect.left += dstFrameP->frameRect.left - dstBlitRect.left;
		dstBlitRect.left =  dstFrameP->frameRect.left;
		if (dstBlitRect.left >= dstBlitRect.right) return;
	}
		// clip off the right
	else if (dstBlitRect.right >  dstFrameP->frameRect.right)
	{
		dstBlitRect.right =  dstFrameP->frameRect.right;
		if (dstBlitRect.right <= dstBlitRect.left) return;
	}

	{
		unsigned long numBytesPerRow;

			// calculate the number of bytes in a row
		numBytesPerRow = (dstBlitRect.right - dstBlitRect.left);

		BlitPixieChunks(
				// calculate the address of the first byte of the source
			(PixelChunkPtr)(srcFrameP->frameBaseAddr + 
				(srcFrameP->scanLinePtrArray[srcBlitRect.top]) + srcBlitRect.left),

				// calculate the address of the first byte of the destination
				// compensate for the offset of the destination pixmap
				// since we may be drawing to the screen
			(PixelChunkPtr)(dstFrameP->frameBaseAddr + 
				(dstFrameP->scanLinePtrArray[(dstBlitRect.top + -dstFrameP->framePix.pixMapP->bounds.top)])
				 + (dstBlitRect.left + -dstFrameP->framePix.pixMapP->bounds.left)),

				// calculate the number of rows to blit
			dstBlitRect.bottom - dstBlitRect.top,

				// number of chunks per row
			BytesToChunks(numBytesPerRow),

				// calculate offset from end of one row to beginning of next row
			srcFrameP->frameRowBytes - numBytesPerRow,
			dstFrameP->frameRowBytes - numBytesPerRow);
	}
}


///--------------------------------------------------------------------------------------
//		BlitPixieMaskDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void BlitPixieMaskDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect)
{
	Rect dstBlitRect = *dstRect;
	Rect srcBlitRect = *srcRect;

		// clip off the top so we dont write into random memory
	if (dstBlitRect.top <  dstFrameP->frameRect.top)
	{
		srcBlitRect.top += dstFrameP->frameRect.top - dstBlitRect.top;
		dstBlitRect.top =  dstFrameP->frameRect.top;
		if (dstBlitRect.top >= dstBlitRect.bottom) return;	
	}
		// clip off the bottom
	else if (dstBlitRect.bottom >  dstFrameP->frameRect.bottom)
	{
		dstBlitRect.bottom =  dstFrameP->frameRect.bottom;
		if (dstBlitRect.bottom <= dstBlitRect.top) return;
	}

		// clip off the left
	if (dstBlitRect.left <  dstFrameP->frameRect.left)
	{
		srcBlitRect.left += dstFrameP->frameRect.left - dstBlitRect.left;
		dstBlitRect.left =  dstFrameP->frameRect.left;
		if (dstBlitRect.left >= dstBlitRect.right) return;
	}
		// clip off the right
	else if (dstBlitRect.right >  dstFrameP->frameRect.right)
	{
		dstBlitRect.right =  dstFrameP->frameRect.right;
		if (dstBlitRect.right <= dstBlitRect.left) return;
	}

	{
		unsigned long numBytesPerRow;
		unsigned long srcBaseOffset;

			// calculate the offset to the first byte of the source
		srcBaseOffset = srcFrameP->scanLinePtrArray[srcBlitRect.top] + srcBlitRect.left;

			// calculate the number of bytes in a row
		numBytesPerRow = (dstBlitRect.right - dstBlitRect.left);

		BlitPixieMask(
				// calculate the address of the first byte of the source
			(PixelPtr)(srcFrameP->frameBaseAddr + srcBaseOffset),

				// calculate the address of the first byte of the destination
			(PixelPtr)(dstFrameP->frameBaseAddr + 
				(dstFrameP->scanLinePtrArray[dstBlitRect.top]) + dstBlitRect.left),

				// calculate the address of the first byte of the mask
			(PixelPtr)(srcFrameP->maskBaseAddr + srcBaseOffset),

				// calculate the number of rows to blit
			dstBlitRect.bottom - dstBlitRect.top,

				// number of bytes in a row (duh!)
			numBytesPerRow,

				// calculate offset from end of one row to beginning of next row
			srcFrameP->frameRowBytes - numBytesPerRow,
			dstFrameP->frameRowBytes - numBytesPerRow);
	}
}


///--------------------------------------------------------------------------------------
//		CompiledSpriteDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void CompiledSpriteDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect)
{
	Rect dstBlitRect = *dstRect;
	Rect srcBlitRect = *srcRect;
	Boolean useCustomBlitter = true;	// assume we will be able to use the custom blitter

	// we first check to see if any clipping will be necessary, and at the same time
	// we determine whether or not we can use the custom compiled blitter. we must
	// do this since the custom blitter cannot handle clipping.

		// clip off the top so we dont write into random memory
	if (dstBlitRect.top <  dstFrameP->frameRect.top)
	{
		srcBlitRect.top += dstFrameP->frameRect.top - dstBlitRect.top;
		dstBlitRect.top =  dstFrameP->frameRect.top;
		if (dstBlitRect.top >= dstBlitRect.bottom) return;
		useCustomBlitter = false;	
	}
		// clip off the bottom
	else if (dstBlitRect.bottom >  dstFrameP->frameRect.bottom)
	{
		dstBlitRect.bottom =  dstFrameP->frameRect.bottom;
		if (dstBlitRect.bottom <= dstBlitRect.top) return;
		useCustomBlitter = false;	
	}

		// clip off the left
	if (dstBlitRect.left <  dstFrameP->frameRect.left)
	{
		srcBlitRect.left += dstFrameP->frameRect.left - dstBlitRect.left;
		dstBlitRect.left =  dstFrameP->frameRect.left;
		if (dstBlitRect.left >= dstBlitRect.right) return;
		useCustomBlitter = false;	
	}
		// clip off the right
	else if (dstBlitRect.right >  dstFrameP->frameRect.right)
	{
		dstBlitRect.right =  dstFrameP->frameRect.right;
		if (dstBlitRect.right <= dstBlitRect.left) return;
		useCustomBlitter = false;	
	}

	// if we didn't have to clip the sprite, we can call the custom compiled blitter
	// hopefully this will be the most common case (ie. the sprites will be entirely
	// on the screen more often than not)

	if (useCustomBlitter)
	{
		(*srcFrameP->frameBlitterP)(
			srcFrameP->frameRowBytes,
			dstFrameP->frameRowBytes,
			srcFrameP->frameBaseAddr,
			dstFrameP->frameBaseAddr +
				dstFrameP->scanLinePtrArray[dstRect->top] + dstRect->left);
	}
	else
	{
		unsigned long numBytesPerRow;
		unsigned long srcBaseOffset;

			// calculate the offset to the first byte of the source
		srcBaseOffset = srcFrameP->scanLinePtrArray[srcBlitRect.top] + srcBlitRect.left;

			// calculate the number of bytes in a row
		numBytesPerRow = (dstBlitRect.right - dstBlitRect.left);

		BlitPixieMask(
				// calculate the address of the first byte of the source
			(PixelPtr)(srcFrameP->frameBaseAddr + srcBaseOffset),

				// calculate the address of the first byte of the destination
			(PixelPtr)(dstFrameP->frameBaseAddr + 
				(dstFrameP->scanLinePtrArray[dstBlitRect.top]) + dstBlitRect.left),

				// calculate the address of the first byte of the mask
			(PixelPtr)(srcFrameP->maskBaseAddr + srcBaseOffset),

				// calculate the number of rows to blit
			dstBlitRect.bottom - dstBlitRect.top,

				// number of bytes in a row (duh!)
			numBytesPerRow,

				// calculate offset from end of one row to beginning of next row
			srcFrameP->frameRowBytes - numBytesPerRow,
			dstFrameP->frameRowBytes - numBytesPerRow);
	}
}


///--------------------------------------------------------------------------------------
//		FastCompiledSpriteDrawProc
///--------------------------------------------------------------------------------------

SW_FUNC void FastCompiledSpriteDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect)
{
#if MPW
#pragma unused(srcRect)
#endif

		// this draw proc assumes no clipping of the sprite will be necessary.
		// in other words the sprite must be *entirely* on the screen.
		// if the sprite is partially off the edge of the screen,
		//	this will *CRASH* big time

	(*srcFrameP->frameBlitterP)(
		srcFrameP->frameRowBytes,
		dstFrameP->frameRowBytes,
		srcFrameP->frameBaseAddr,
		dstFrameP->frameBaseAddr + dstFrameP->scanLinePtrArray[dstRect->top] + dstRect->left);
}


#if SW_USE_C

///--------------------------------------------------------------------------------------
//		BlitPixie
///--------------------------------------------------------------------------------------

void BlitPixie(
	register PixelChunkPtr srcPixelP,
	register PixelChunkPtr dstPixelP,
	register unsigned long rowsToCopy,
	register unsigned long numBytesPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride)
{
	register long loopsPerRow;
	register long extraPixelChunks;
	register long pixelChunksPerRow;

	pixelChunksPerRow = BytesToChunks(numBytesPerRow); 
	extraPixelChunks = pixelChunksPerRow & 0xF;		
	pixelChunksPerRow >>= 4;

	while (rowsToCopy--)
	{
			// we reuse pixelChunksPerRow to restore loopsPerRow
		loopsPerRow = pixelChunksPerRow;

			// this maniacal, though extremely convenient bit of C code
			// is a Duff-style unrolled loop, an invention of Tom Duff at Bellcore.
			// I've heard it breaks some compilers, but it does have
			//	Dennis Ritchie's blessing as valid C code.
		switch (extraPixelChunks)
		{
			do
			{
							BlitPixelChunk(srcPixelP, dstPixelP);
				case 15:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 14:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 13:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 12:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 11:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 10:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 9:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 8:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 7:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 6:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 5:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 4:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 3:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 2:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 1:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 0: ;
			} while (loopsPerRow--);
		}

			// bump to the next row
		srcPixelP = (PixelChunkPtr)(((char*)srcPixelP) + srcRowStride);
		dstPixelP = (PixelChunkPtr)(((char*)dstPixelP) + dstRowStride);
	}
}


///--------------------------------------------------------------------------------------
//		BlitPixieChunks
///--------------------------------------------------------------------------------------

void BlitPixieChunks(
	register PixelChunkPtr srcPixelP,
	register PixelChunkPtr dstPixelP,
	register unsigned long rowsToCopy,
	register unsigned long pixelChunksPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride)
{
	register long loopsPerRow;
	register long extraPixelChunks;

	extraPixelChunks = pixelChunksPerRow & 0xF;		
	pixelChunksPerRow >>= 4;

	while (rowsToCopy--)
	{
			// we reuse pixelChunksPerRow to restore loopsPerRow
		loopsPerRow = pixelChunksPerRow;

			// this maniacal, though extremely convenient bit of C code
			// is a Duff-style unrolled loop, an invention of Tom Duff at Bellcore.
			// I've heard it breaks some compilers, but it does have
			//	Dennis Ritchie's blessing as valid C code.
		switch (extraPixelChunks)
		{
			do
			{
							BlitPixelChunk(srcPixelP, dstPixelP);
				case 15:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 14:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 13:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 12:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 11:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 10:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 9:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 8:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 7:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 6:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 5:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 4:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 3:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 2:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 1:	BlitPixelChunk(srcPixelP, dstPixelP);
				case 0: ;
			} while (loopsPerRow--);
		}

			// bump to the next row
		srcPixelP = (PixelChunkPtr)(((char*)srcPixelP) + srcRowStride);
		dstPixelP = (PixelChunkPtr)(((char*)dstPixelP) + dstRowStride);
	}
}


///--------------------------------------------------------------------------------------
//		BlitPixieMask
///--------------------------------------------------------------------------------------

void BlitPixieMask(
	register PixelPtr srcPixelP,
	register PixelPtr dstPixelP,
	register PixelPtr maskPixelP,
	register unsigned long rowsToCopy,
	register unsigned long longWordsPerRow,		// this is really numBytesPerRow
	register unsigned long srcRowStride,
	register unsigned long dstRowStride)
{
	register long loopsPerRow;
	register long extraPixelLongs;

	longWordsPerRow >>= 2;
	extraPixelLongs = longWordsPerRow & 0xF;
	longWordsPerRow >>= 4;

	while (rowsToCopy--)
	{
		loopsPerRow = longWordsPerRow;

			// this maniacal, though extremely convenient bit of C code
			// is a Duff-style unrolled loop, an invention of Tom Duff at Bellcore.
			// I've heard it breaks some compilers, but it does have
			//	Dennis Ritchie's blessing as valid C code.
		switch (extraPixelLongs)
		{
			do
			{
							BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 15:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 14:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 13:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 12:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 11:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 10:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 9:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 8:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 7:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 6:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 5:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 4:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 3:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 2:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 1:	BlitPixelLongMask(srcPixelP, dstPixelP, maskPixelP);
				case 0: ;
			} while (loopsPerRow--);
		}

			// bump to the next row
		srcPixelP = (PixelPtr)(((unsigned char*)srcPixelP) + srcRowStride);
		maskPixelP = (PixelPtr)(((unsigned char*)maskPixelP) + srcRowStride);
		dstPixelP = (PixelPtr)(((unsigned char*)dstPixelP) + dstRowStride);
	}
}


#else /* SW_USE_C */


///--------------------------------------------------------------------------------------
//		BlitPixie
///--------------------------------------------------------------------------------------

SW_ASM_FUNC void BlitPixie(
	register PixelPtr srcPixelP,
	register PixelPtr dstPixelP,
	register unsigned long rowsToCopy,
	register unsigned long numBytesPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride)
{
	register unsigned long loopsPerRow;

		SW_ASM_BEGIN

#if __MWERKS__
		movem.l			loopsPerRow/srcPixelP/dstPixelP/rowsToCopy/numBytesPerRow/srcRowStride/dstRowStride, -(sp)

			// move the parameters into registers
		movea.l			28(sp), srcPixelP
		movea.l			32(sp), dstPixelP
		move.l			36(sp), rowsToCopy
		move.l			40(sp), numBytesPerRow
		move.l			44(sp), srcRowStride
		move.l			48(sp), dstRowStride
#endif

			// longWordsPerRow = numBytesPerRow >> 2;
		move.l	numBytesPerRow, d0
		lsr.l		#2, d0

			// numBytesPerRow -= longWordsPerRow << 2;
		move.l	d0, d1
		lsl.l		#2, d1
		sub.l		d1, numBytesPerRow

			// loopsPerRow = longWordsPerRow >> 4;
		move.l	d0, loopsPerRow
		lsr.l		#4, loopsPerRow

		moveq		#0xF, d1
		and.l		d1, d0
		add.l		d0, d0			// multiply longWordsPerRow by 2 (size of move.l (srcPixelP)+,(dstPixelP)+)
		lea 		@loopEnd, a0	// get address of the end of the loop
		suba.l	d0, a0			// calculate where to jmp in the loop

	@forEachRow:
		move.l	loopsPerRow, d0
		jmp		(a0)
	@loopBase:
		move.l	(srcPixelP)+, (dstPixelP)+	// 16
		move.l	(srcPixelP)+, (dstPixelP)+	// 15
		move.l	(srcPixelP)+, (dstPixelP)+	// 14
		move.l	(srcPixelP)+, (dstPixelP)+	// 13
		move.l	(srcPixelP)+, (dstPixelP)+	// 12
		move.l	(srcPixelP)+, (dstPixelP)+	// 11 
		move.l	(srcPixelP)+, (dstPixelP)+	// 10
		move.l	(srcPixelP)+, (dstPixelP)+	//  9
		move.l	(srcPixelP)+, (dstPixelP)+	//  8
		move.l	(srcPixelP)+, (dstPixelP)+	//  7
		move.l	(srcPixelP)+, (dstPixelP)+	//  6
		move.l	(srcPixelP)+, (dstPixelP)+	//  5
		move.l	(srcPixelP)+, (dstPixelP)+	//  4
		move.l	(srcPixelP)+, (dstPixelP)+	//  3
		move.l	(srcPixelP)+, (dstPixelP)+	//  2
		move.l	(srcPixelP)+, (dstPixelP)+	//  1
	@loopEnd:
		subq.l	#1,d0
		bpl		@loopBase

		// now do any leftover bits
		move.l	numBytesPerRow, d0
		beq		@nextRow
		subq.l	#2, d0
		bmi		@moveByte
		move.w	(srcPixelP)+, (dstPixelP)+
		tst		d0
		beq		@nextRow
	@moveByte:	
		move.b	(srcPixelP)+, (dstPixelP)+

	@nextRow:
		adda.l	srcRowStride, srcPixelP
		adda.l	dstRowStride, dstPixelP
		subq.l	#1, rowsToCopy
		bne		@forEachRow

#if __MWERKS__
		movem.l			(sp)+,	loopsPerRow/srcPixelP/dstPixelP/rowsToCopy/numBytesPerRow/srcRowStride/dstRowStride
#endif

		SW_ASM_END
}


///--------------------------------------------------------------------------------------
//		BlitPixieChunks
///--------------------------------------------------------------------------------------

SW_ASM_FUNC void BlitPixieChunks(
	register PixelChunkPtr srcPixelP,
	register PixelChunkPtr dstPixelP,
	register unsigned long rowsToCopy,
	register unsigned long pixelChunksPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride)
{
	register unsigned long loopsPerRow;

			SW_ASM_BEGIN

#if __MWERKS__
			movem.l			loopsPerRow/rowsToCopy/pixelChunksPerRow/srcRowStride/dstRowStride/srcPixelP/dstPixelP, -(sp)

				// move the parameters into registers
			movea.l			32(sp), srcPixelP
			movea.l			36(sp), dstPixelP
			move.l			40(sp), rowsToCopy
			move.l			44(sp), pixelChunksPerRow
			move.l			48(sp), srcRowStride
			move.l			52(sp), dstRowStride
#endif
				// calculate number of loops per row of pixels
				// loopsPerRow = pixelChunksPerRow >> 4;
			move.l			pixelChunksPerRow, loopsPerRow
			lsr.l				#4, loopsPerRow

				// calculate which move.l to jump to
			moveq				#0xF, d0
			and.l				d0, pixelChunksPerRow
			add.l				pixelChunksPerRow, pixelChunksPerRow
			lea 				@preLoopEnd, a0
			suba.l			pixelChunksPerRow, a0

	@forEachRow:
			jmp				(a0)
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
	@preLoopEnd:

			move.l			loopsPerRow, d0
			beq				@skipLoop
	@loopBase:
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
			move.l			(srcPixelP)+, (dstPixelP)+
				// decrement the loopsPerRow counter and loop back
			subq.l			#1, d0
			bne				@loopBase
@skipLoop:
				// bump the source and dest pointers
			adda.l			srcRowStride, srcPixelP
			adda.l			dstRowStride, dstPixelP

				// decrement the row counter and loop back
			subq				#1, rowsToCopy
			bne				@forEachRow

#if __MWERKS__
			movem.l			(sp)+, loopsPerRow/rowsToCopy/pixelChunksPerRow/srcRowStride/dstRowStride/srcPixelP/dstPixelP
#endif

			SW_ASM_END
}


///--------------------------------------------------------------------------------------
//		BlitPixieMask
///--------------------------------------------------------------------------------------

SW_ASM_FUNC void BlitPixieMask(
	register PixelPtr srcPixelP,
	register PixelPtr dstPixelP,
	register PixelPtr maskPixelP,
	register unsigned long rowsToCopy,
	register unsigned long numBytesPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride)
{
	register unsigned long loopsPerRow;

		SW_ASM_BEGIN

#if __MWERKS__
		movem.l	loopsPerRow/srcPixelP/dstPixelP/maskPixelP/rowsToCopy/numBytesPerRow/srcRowStride/dstRowStride, -(sp)

		movea.l	36(sp), srcPixelP
		movea.l	40(sp), dstPixelP
		movea.l	44(sp), maskPixelP
		move.l	48(sp), rowsToCopy
		move.l	52(sp), numBytesPerRow
		move.l	56(sp), srcRowStride
		move.l	60(sp), dstRowStride
#endif

			// longWordsPerRow = numBytesPerRow >> 2;
		move.l	numBytesPerRow, d0
		lsr.l		#2, d0

			// numBytesPerRow -= longWordsPerRow << 2;
		move.l	d0, d1
		lsl.l		#2, d1
		sub.l		d1, numBytesPerRow

			// loopsPerRow = longWordsPerRow >> 4;
		move.l	d0, loopsPerRow
		lsr.l		#4, loopsPerRow

			
		moveq		#0xF, d1
		and.l		d1, d0
		lsl.l		#3, d0					// longWordsPerRow *= 8;
		lea 		@loopEnd, a0			// get address of the end of the loop
		sub.l		d0, a0					// calculate where to jmp in the loop

	@forEachRow:
		move.l	loopsPerRow, d2
		jmp		(a0)
	@loopBase:
			// 16
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
			// 15
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
			// 14
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	// 13
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	// 12
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	// 11
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	// 10
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  9
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  8
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  7
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
			//  6
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  5
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  4
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  3
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
			//  2
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
      	//  1
		move.l	(dstPixelP), d0
		and.l		(maskPixelP)+, d0
		or.l		(srcPixelP)+, d0
		move.l	d0, (dstPixelP)+
	@loopEnd:
		subq.l	#1, d2
		bpl		@loopBase

		// now do any leftover bits
		move.l	numBytesPerRow, d2
		beq		@nextRow
		subq.l	#2, d2
		bmi		@moveByte
		move.w	(dstPixelP), d0
		and.w		(maskPixelP)+, d0
		or.w		(srcPixelP)+, d0
		move.w	d0, (dstPixelP)+
		tst		d2
		beq		@nextRow
	@moveByte:	
		move.b	(dstPixelP), d0
		and.b		(maskPixelP)+, d0
		or.b		(srcPixelP)+, d0
		move.b	d0, (dstPixelP)+

	@nextRow:
		adda.l	srcRowStride, srcPixelP
		adda.l	srcRowStride, maskPixelP
		adda.l	dstRowStride, dstPixelP
		subq.l	#1, rowsToCopy
		bne		@forEachRow

#if __MWERKS__
		movem.l	(sp)+, loopsPerRow/srcPixelP/dstPixelP/maskPixelP/rowsToCopy/numBytesPerRow/srcRowStride/dstRowStride
#endif

		SW_ASM_END
}


#endif /* SW_USE_C */

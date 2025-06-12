///--------------------------------------------------------------------------------------
//	BlitPixie.h
//
//	Created:	Thursday, September 24, 1992 at 11:48 PM
//	By:			Tony Myles
//
//	Copyright: © 1991-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __BLITPIXIE__
#define __BLITPIXIE__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __FRAME__
#include "Frame.h"
#endif


///--------------------------------------------------------------------------------------
//		type definitions & conditional macros
///--------------------------------------------------------------------------------------

#if SW_PPC

typedef double PixelChunk, *PixelChunkPtr, **PixelChunkHdl;

#define SW_USE_C 1

#define BytesToChunks(b) ((b) >> 3)

#define BlitPixelChunk(s, d)	{ *(d) = *(s);(d)++;(s)++; }

#define BlitPixelLongMask(s, d, m)		\
		{								\
			register unsigned long _t;	\
										\
			_t = *(d);					\
			_t &= *(m);					\
			_t |= *(s);					\
			*(d) = _t;					\
			(s)++;(d)++;(m)++;			\
		}

#else	// SW_PPC


typedef unsigned long PixelChunk, *PixelChunkPtr, **PixelChunkHdl;

#if MPW
#define SW_USE_C 1
#else
#define SW_USE_C 0
#endif

#define BytesToChunks(b) ((b) >> 2)

#define BlitPixelChunk(s, d)	(*(d)++ = *(s)++)

#define BlitPixelLongMask(s, d, m)	(*(d)++ = (*(d) & *(m)++) | *(s)++)


#endif	// SW_PPC


typedef unsigned long* PixelPtr;


#define kBitsPerPixel 8

#define kRightAlignFactor (((sizeof(PixelChunk) * kBitsPerByte) / kBitsPerPixel) - 1)
#define kLeftAlignFactor (~kRightAlignFactor)


///--------------------------------------------------------------------------------------
//		function prototypes
///--------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

SW_FUNC void BlitPixieWorldDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *blitRect);

SW_FUNC void BlitPixieSpriteDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect);

SW_FUNC void BlitPixieMaskDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect);

SW_FUNC void CompiledSpriteDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect);

SW_FUNC void FastCompiledSpriteDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	Rect *srcRect,
	Rect *dstRect);

void BlitPixie(
	register PixelChunkPtr srcPixelP,
	register PixelChunkPtr dstPixelP,
	register unsigned long rowsToCopy,
	register unsigned long numBytesPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride);

void BlitPixieChunks(
	register PixelChunkPtr srcPixelP,
	register PixelChunkPtr dstPixelP,
	register unsigned long rowsToCopy,
	register unsigned long longWordsPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride);

void BlitPixieMask(
	register PixelPtr srcPixelP,
	register PixelPtr dstPixelP,
	register PixelPtr maskPixelP,
	register unsigned long rowsToCopy,
	register unsigned long numBytesPerRow,
	register unsigned long srcRowStride,
	register unsigned long dstRowStride);

#ifdef __cplusplus
};
#endif
#endif /* __BLITPIXIE__ */

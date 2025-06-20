/*
	FastBitCopies.h
	
	Interface to FastBitCopies.c
	
	Special bit copy routines for 8-bit graphics
	
	Copyright 1993 by Al Evans
	
	For Graphic Elements release version 1.0b1
	
*/



#ifdef __cplusplus
extern "C" {
#endif

pascal void FastCopyBits(const BitMap *srcBits,const BitMap *dstBits,const Rect *srcRect,
    const Rect *dstRect,short mode,RgnHandle maskRgn);


/*
	Assembly engines
*/

extern void MoveBitsXP(Ptr source, Ptr dest, 
					long nLines, long nBytesPerLine,
					long srcRowBytes, long destRowBytes, long mirrorFlags);
					

extern void MoveBitsCP(Ptr source, Ptr dest, 
					long nLines, long nBytesPerLine,
					long srcRowBytes, long destRowBytes);

#ifdef __cplusplus
}
#endif

/*
** File:		Blit.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
**
** Based on 'BlitCmp' code from Apple Computer, available on the develop 18 CD.
*/


#include "Blit.h"


//static long		*SourceAddresses;
//static long		*DestinationAddresses;
static short	height;
static long		srcRowBytes;
static long		dstRowBytes;
static long		srcLeft;



void InitBlit(PixMapPtr srcPM, PixMapPtr dstPM, Rect *srcRect, Rect *dstRect)
{
	height = srcRect->bottom - srcRect->top;	// No scaling allowed
	
	srcRowBytes = srcPM->rowBytes & 0x3fff;
	dstRowBytes = dstPM->rowBytes & 0x3fff;
	
	// get the bit offset to the src left edge
	srcLeft = (srcRect->left - srcPM->bounds.left) * srcPM->pixelSize;
	
	// eventually, this function will calculate row addresses and other static data beforehand
}



pascal void CopyBlit(PixMapPtr srcPM, PixMapPtr dstPM, Rect *srcRect, Rect *dstRect)
{
	long			dstLeft;
	long			dstRight;
	long *			srcRow;
	long *			dstRow;
	register long *	srcPtr;
	register long *	dstPtr;
	long			leftMask;
	long			notLeftMask;
	long			rightMask;
	long			notRightMask;
	long			dstLong;
	short			dstLongs;
	short			localheight = height;
	long			offset;
	long			timesCopy;

	srcRow = (long *) srcPM->baseAddr;
	dstRow = (long *) dstPM->baseAddr;
		
	// offset the src ptr to the first long
	srcRow += srcLeft >> 5;
	
	// get the bit offset to the dst left and right edges
	dstLeft = (dstRect->left - dstPM->bounds.left) * dstPM->pixelSize;
	dstRight = (dstRect->right - dstPM->bounds.left) * dstPM->pixelSize;
	
	// get the number of middle longs to do minus the left edge long
	dstLongs = ((dstRight - dstLeft) >> 5) - 1;
	
	// offset the dst Ptr to the first long
	dstRow += dstLeft >> 5;

	// now compute left and right masks for the dst
	dstLeft &= 0x1f;
	leftMask = ( 1 << dstLeft ) - 1;
	notLeftMask = ~leftMask;
	
	dstRight &= 0x1f;
	notRightMask = ( 1 << dstRight ) - 1;
	rightMask = ~notRightMask;
	
	// offset the src and dst ptrs to the first row
	offset = (srcRect->top - srcPM->bounds.top) * srcRowBytes;
	srcRow += (long *) offset;
	
	offset = (dstRect->top - dstPM->bounds.top) * dstRowBytes;
	dstRow += (long *) offset;
	
	/* check if we need to do the left and right mask */
	if ( leftMask )
		{
		if ( notLeftMask == 0 )
			{
			leftMask = 0;
			dstLongs++;
			}
		}
		
	if ( rightMask )
		{
		if ( notRightMask == 0 )
			{
			rightMask = 0;
			dstLongs++;
			}
		}
		
	//for ( ; localheight >= 0; --localheight )
	// changing the above 'for()' to the below 'while()' is what made this blit routine
	// faster than CopyBits - a speed improvement equal to all other changes I had made
	// previously.  (about 4 to 5 milliseconds, in case you were wondering)
	// Lesson: the true bottlenecks are not always the obvious ones
	while (localheight--)
		{
		srcPtr = srcRow;
		dstPtr = dstRow;
		
		/* do the masked left edge */
		if ( leftMask )
			{
			dstLong = *srcPtr++ & leftMask;
			dstLong |= *dstPtr & notLeftMask;
			*dstPtr++ = dstLong;
			}

		/* do the middle longs with Duff's device */
		timesCopy = (dstLongs + 15) >> 4;
		
		switch( dstLongs & 0xF )
			{
			case 0:	do
					{	*dstPtr++ = *srcPtr++;
			case 15:	*dstPtr++ = *srcPtr++;
			case 14:	*dstPtr++ = *srcPtr++;
			case 13:	*dstPtr++ = *srcPtr++;
			case 12:	*dstPtr++ = *srcPtr++;
			case 11:	*dstPtr++ = *srcPtr++;
			case 10:	*dstPtr++ = *srcPtr++;
			case 9:		*dstPtr++ = *srcPtr++;
			case 8:		*dstPtr++ = *srcPtr++;
			case 7:		*dstPtr++ = *srcPtr++;
			case 6:		*dstPtr++ = *srcPtr++;
			case 5:		*dstPtr++ = *srcPtr++;
			case 4:		*dstPtr++ = *srcPtr++;
			case 3:		*dstPtr++ = *srcPtr++;
			case 2:		*dstPtr++ = *srcPtr++;
			case 1:		*dstPtr++ = *srcPtr++;
					} while( --timesCopy > 0 );
			}

		/* do the masked right edge */
		if ( rightMask )
			{
			dstLong = *srcPtr & rightMask;
			dstLong |= *dstPtr & notRightMask;
			*dstPtr = dstLong;
			}
		
		/* bump to the next row */
		srcRow += (long *) srcRowBytes;
		dstRow += (long *) dstRowBytes;
		}
}



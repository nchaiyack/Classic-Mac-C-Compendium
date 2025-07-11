/* RectToAddress.c
Finds the 32-bit memory address of a pixel in a PixMap or BitMap. Completely
general: multiple screens, on- or off-screen, BitMaps, PixMaps, and any pixel
size. Doesn't require color quickdraw. The optional parameters return the row
length of the pixmap or bitmap, the pixel size, and the offset in bits into the
byte (for pixels that are smaller than a byte).

The returned address corresponds to the upper-left pixel in *rectPtr in
*pixMapPtr, AFTER clipping by the bounds associated with the pix/bitmap: If
it's an off-screen Pix/BitMap it clips *rectPtr to the PixMap bounds; if it's
on-screen it clips *rectPtr to that screen device. The returned address is NULL
if *rectPtr is empty after clipping.

You should only access the address returned by RectToAddress inside a block of
code that operates in 32-bit addressing mode. Use SwapMMUMode(), which is
documented in Apple's Inside Macintosh.

Copyright �1989-1993 Denis G. Pelli.

HISTORY:
4/89	dgp	v. 1.0 extracted it from CopyBitsQuickly.
2/91	dgp	v. 1.1 no longer requires color quickdraw.
4/5/91	dgp	v. 1.2 fixed overflow bug reported by Brady Duga.
8/24/91	dgp	Made compatible with THINK C 5.0.
4/27/92	dgp	Though I haven't experienced any problems I took the advice of Apple's
			Inside Mac VI and now get the pixmap base address by means of the new
			GetPixBaseAddr() if that trap is available.
7/13/92	dgp	enhanced the documentation above. Removed support for THINK C version 4.
1/11/93	dgp only call GetPixBaseAddr() for PixMap, not for BitMap.
2/7/93	dgp	return NULL if supplied pix/bitmap pointer is NULL.
3/14/93	dgp	Use GetPixBaseAddr() only if the working version is present, i.e.
			not the first version of 32-bit QuickDraw.
4/1/93	dgp	Changed test for valid GetPixBaseAddr() to require version gestalt32BitQD13 
			or later, as specified by New Tech Note "Color QuickDraw Q&As".
4/21/93	dgp	Now always return a 32-bit address, even under System 6.
			Wrote GetPixBaseAddr32() which is a more-dependable substitute for
			Apple's GetPixBaseAddr, and accepts bit/pixmaps to boot.
4/27/93	dgp	Josh discovered a bug; RectToAddress was finding the device that
			held the upper left corner of the rect, BEFORE clipping by the pixmap,
			which resulted in a null rect when the wrong device was later clipped
			by the pixmap. Now I do the right thing, which is to find the device
			that holds the upper left corner of the pixmap. Also, I now always
			return the clipping to the user, which wasn't being done consistently,
			e.g. never for bitmaps.
4/11/94	dgp	I suspect (but didn't test) that what takes so long in RectToAddress 
			is the call to Apple's GetMaxDevice(), so I now optionally replace that
			by a call to my GetRectDevice(), which is a new routine in GetWindowDevice.c.
*/
#include "VideoToolbox.h"
GDHandle GetRectDevice(Rect *r);
	
Ptr GetPixBaseAddr32(PixMap **pm)
// Returns the 32-bit address of the pixels/bits in a pix/bitmap.
{
	Ptr address;
	long qD;
	GDHandle device;
	short i;
	
	// Apple's GetPixBaseAddr() did not work in the early versions of 32-bit QuickDraw
	Gestalt(gestaltQuickdrawVersion,&qD);
	if(qD>=gestalt32BitQD13 && ((**pm).rowBytes&0x8000))
		return GetPixBaseAddr(pm);	// Guaranteed to be 32-bit.
	address=(**pm).baseAddr;
	if(qD>=gestalt8BitQD){
		// If it's the base address of a screen then it's a 32-bit address.
		for(i=0;;i++){
			device=GetScreenDevice(i);
			if(device==NULL)break;
			if(address==(**(**device).gdPMap).baseAddr)return address;
		}
	}
	// Otherwise it must be in RAM and we should strip it, 
	// since we may be in 24-bit mode.
	address=StripAddress(address);
	return address;
}

unsigned char *RectToAddress(PixMap *pixMapPtr,Rect *rectPtr,short *rowBytesPtr, 
	short *pixelSizePtr,short *bitsOffsetPtr)
/*
*pixMapPtr is the PixMap or BitMap that you're working in.
*rectPtr is the Rect you're interested in, in local coordinates. 
	Will be clipped by pixmap/device bounds.
*rowBytesPtr optionally returns rowBytes.
*pixelSizePtr optionally returns pixelSize.
*bitsOffsetPtr optionally returns positive offset in bits from returned
	byte address to the location specified by upper left corner of rectPtr
*/
{
	GDHandle device,mainDevice;
	Rect rect,smallRect;
	register Ptr address;
	short notEmpty;
	long x0,y0;
	long qD;
	
	if(pixMapPtr==NULL)return NULL;
	Gestalt(gestaltQuickdrawVersion,&qD);
	x0=pixMapPtr->bounds.left;
	y0=pixMapPtr->bounds.top;
	if(qD>=gestalt8BitQD)mainDevice=GetMainDevice();
	else mainDevice=NULL;
	address=GetPixBaseAddr32(&pixMapPtr);
	if(address==NULL)return NULL;
	if(mainDevice != NULL && address == (*(*mainDevice)->gdPMap)->baseAddr){
		// When there are multiple screens, all windows refer to main device baseAddr
		// so we have to figure out which is the actual device.
		rect = *rectPtr;
		OffsetRect(&rect,-x0,-y0);		/* convert to global coordinates */
		// Find device that displays the upper left pixel of PixMap
		SetRect(&smallRect,-x0,-y0,-x0+1,-y0+1);	// global coordinates
		if(0)device = GetMaxDevice(&smallRect);		// I wonder which is quicker.
		else device = GetRectDevice(&smallRect);
		if(device==NULL)return NULL;
		pixMapPtr = *(*device)->gdPMap;	/* Use the DEVICE'S PixMap */
		address=pixMapPtr->baseAddr;
		notEmpty=SectRect(&pixMapPtr->bounds,&rect,&rect);
		*rectPtr=rect;					// Clip user's rect too.
		OffsetRect(rectPtr,x0,y0);		// convert that back to local coordinates
										// convert ours to device coordinates
		OffsetRect(&rect,-(*device)->gdRect.left,-(*device)->gdRect.top);
	}
	else {
		/* Just clip and convert to global coordinates */
		rect = *rectPtr;
		notEmpty=SectRect(&pixMapPtr->bounds,&rect,&rect);
		*rectPtr=rect;					// Clip user's rect too.
		OffsetRect(&rect,-x0,-y0);		// convert to global coordinates
	}
	address += rect.top*(long)(pixMapPtr->rowBytes & 0x1fff);
	if(pixMapPtr->rowBytes & 0x8000){					/* Pixmap or Bitmap? */
		// Pixmap
		address += (rect.left*(long)pixMapPtr->pixelSize)/8;
		if(bitsOffsetPtr != NULL) *bitsOffsetPtr=(rect.left*(long)pixMapPtr->pixelSize)%8;
		if(pixelSizePtr != NULL) *pixelSizePtr=pixMapPtr->pixelSize;
	}
	else{
		// Bitmap
		address += rect.left/8;		
		if(bitsOffsetPtr != NULL) *bitsOffsetPtr=rect.left%8;
		if(pixelSizePtr != NULL) *pixelSizePtr=1;
	}
	if(rowBytesPtr != NULL) (*rowBytesPtr)=pixMapPtr->rowBytes & 0x1fff;
	if(!notEmpty)address=NULL;	
	return (unsigned char *) address;
}

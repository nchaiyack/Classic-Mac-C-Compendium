/*
SetOnePixel.c
NOTE: These routines are obsolete, retained solely for backward compatibility. 
Use SetPixelsQuickly.c instead, since it's more than ten times faster in 
most applications.

These are replacements for the official Apple SetCPixel, etc. Their virtue is
that they're faster (but still slow) and don't translate your index via the
color table. GetxxxPixel & SetxxxPixel all ignore the color spec arrays, giving
you direct access to the unsigned number stored in the pixel (whether 1, 2, 4,
8, 16, or 32 bits)..

SetOnePixel(x,y,value);
value=GetOnePixel(x,y);
They use the current port. x and y are local coordinates.

SetPixmapPixel() and GetPixmapPixel() use the specified pixmap or bitmap.

SetDevicePixel() and GetDevicePixel() use the specified graphics device (i.e.
monitor).

NOTE:
If you are setting many contiguous pixels you will be wasting most of your time
on the call overhead (once per pixel). That's why you should use
SetPixelsQuickly.c instead.

These routines (and RectToAddress) do not "move memory", i.e. they don't give the
Memory Manager any pretext for shuffling around the memory allocations, and
change its master pointers. That's why it's OK to dereference the pixmap handle,
i.e. use a local copy of the pixmap's master pointer.

All the routines in this file call either SetPixmapPixel or GetPixmapPixel. Both
run fast by caching the information that they get about your Bit/Pixmap from
RectToAddress. SetPixmapPixel and GetPixmapPixel each have their own cache.

HISTORY:
4/4/89	dgp wrote it as SetIPixel.c
9/8/90	dgp updated to work with 32 bit QuickDraw, if present.
10/15/90 bf renamed SetIPixelGW.c and modified for drawing to off screen pix maps.
4/26/92	dgp	Merged the two variants: SetIPixel.c and SetIPixelGW.c to produce the
			new file SetOnePixel.c. 
			Renamed SetIPixelGW to SetPixmapPixel.
			Renamed GetIPixelGW to GetPixmapPixel.
			Renamed SetIPixel to SetDevicePixel.
			Renamed GetIPixel to GetDevicePixel.
			Added SetOnePixel and GetOnePixel.
			Generalized to handle any pixelSize, and accept bitmaps as well
			as pixmaps.
12/23/92 dgp Doubled the speed of SetPixmapPixel and GetPixmapPixel (and thus sped up
			all the routines that call them) by caching the answers from RectToAddress. 
			The cache doubles the speed, but see CAUTION above.Introduced compile-time 
			flag to optionally disable this new cache.
1/6/93 dgp	Fixed tiny but disastrous bug in GetPixmapPixel (wasn't saving old x and y).
1/22/93	dgp	Check more PixMap fields to make sure cache is not stale. Deleted the
			warning about stale caches from the documentation.
2/7/93	dgp Wrote SetPixelsQuickly.c.
7/9/93	dgp check for 32-bit addressing capability.
*/
#include "VideoToolbox.h"
#define USE_CACHE 1	// true or false

void SetOnePixel(int x,int y,unsigned long value)
// Assumes (x,y) is in the local coordinate system of the current port.
{
	WindowPtr window;
	PixMapPtr pm;
	
	GetPort(&window);
	if(x<window->portRect.left || x>=window->portRect.right
		|| y<window->portRect.top || y>=window->portRect.bottom)return;
	// Is it a CGrafPort or a GrafPort?
	if(((CGrafPtr)window)->portVersion<0)		// It's a CGrafPort,
		pm = *((CGrafPtr)window)->portPixMap;	// pass pixmap ptr.
	else 										// It's a GrafPort,
		pm = (PixMapPtr) &window->portBits;		// pass bitmap ptr.
	SetPixmapPixel(pm,x,y,value);
}

unsigned long GetOnePixel(x,y)
// Assumes (x,y) is in the local coordinate system of the current port.
{
	WindowPtr window;
	PixMapPtr pm;
	Rect *boundsPtr;
	
	GetPort(&window);
	if(x<window->portRect.left || x>=window->portRect.right
		|| y<window->portRect.top || y>=window->portRect.bottom)return 0;
	// Is it a CGrafPort or a GrafPort?
	if(((CGrafPtr)window)->portVersion<0)		// It's a CGrafPort,
		pm = *((CGrafPtr)window)->portPixMap;	// pass pixmap ptr.
	else 										// It's a GrafPort,
		pm = (PixMapPtr) &window->portBits;		// pass bitmap ptr.
	return GetPixmapPixel(pm,x,y);
}

void SetPixmapPixel(register PixMapPtr pmPtr,int x,int y,unsigned long value)
// Pokes a value into a pixel of any size. Accepts either pixmap or bitmap.
// Assumes (x,y) is in the coordinate system of the bit/pixmap.
// Speed is enhanced by reusing the cached information from last time if it's the
// same Pix/Bitmap as last time, i.e. same address, baseAddr,rowBytes, and bounds.
// You can force it to flush its cache by passing a NULL PixMap address.
{
	static PixMapPtr oldPmPtr=(PixMapPtr)-1;
	static int oldX,oldY;
	static short rowBytes,logPixelSize,bitsOffset;
	static unsigned char *pixelPtr;
	static BitMap oldMap;
	int shift;
	unsigned char mask;
	char mode32=true32b;
	Rect r;
	Boolean can32;
	long addrMode=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&addrMode);
	can32=!error && (addrMode&(1<<gestalt32BitCapable));	
	if(!USE_CACHE || pmPtr!=oldPmPtr
		|| pmPtr->baseAddr!=oldMap.baseAddr 
		|| pmPtr->rowBytes!=oldMap.rowBytes 
		|| *(long *)&pmPtr->bounds.top!=*(long *)&oldMap.bounds.top
		|| *(long *)&pmPtr->bounds.bottom!=*(long *)&oldMap.bounds.bottom){
		// Cache is stale. Get fresh values.
		short pixelSize;
		SetRect(&r,x,y,x+1,y+1);
		pixelPtr=RectToAddress(pmPtr,&r,&rowBytes,&pixelSize,&bitsOffset);
		if(pixelPtr==NULL){
			oldPmPtr=(PixMapPtr)-1;	// invalidate cache
			return;
		}
		oldPmPtr=pmPtr;
		oldMap=*(BitMap *)pmPtr;
		logPixelSize=Log2L(pixelSize);
	}else{
		// Cache is fresh. Merely correct for changes in x and y.
		if(pixelPtr==NULL)return;
		if(logPixelSize<3){
			register long bits;
			bits=bitsOffset+(long)(x-oldX)<<logPixelSize;
			pixelPtr+=bits>>3;
			bitsOffset=bits&7;
		}else pixelPtr+=(x-oldX)<<(logPixelSize-3);
		if(y!=oldY)pixelPtr+=(long)(y-oldY)*rowBytes;
	}
	if(can32)SwapMMUMode(&mode32);
	switch(logPixelSize){
		register unsigned char val;
	case 0:
		shift=8-1-bitsOffset;	// from right, instead of from left
		mask=1<<shift;
		val=((unsigned char)value<<shift)&mask;
		mask=~mask;
		*pixelPtr= *pixelPtr & mask | (unsigned char)val;
		break;
	case 1:
		shift=8-2-bitsOffset;	// from right, instead of from left
		mask=3<<shift;
		val=((unsigned char)value<<shift)&mask;
		mask=~mask;
		*pixelPtr= *pixelPtr & mask | (unsigned char)val;
		break;
	case 2:
		shift=8-4-bitsOffset;	// from right, instead of from left
		mask=15<<shift;
		val=((unsigned char)value<<shift)&mask;
		mask=~mask;
		*pixelPtr= *pixelPtr & mask | (unsigned char)val;
		break;
	case 3:
		*pixelPtr=value;
		break;
	case 4:
		*(unsigned short *)pixelPtr=value;
		break;
	case 5:
		*(unsigned long *)pixelPtr=value;
		break;
	}
	if(can32)SwapMMUMode(&mode32);
	oldX=x;
	oldY=y;
}

unsigned long GetPixmapPixel(register PixMapPtr pmPtr,int x,int y)
// Returns the contents of a pixel of any size. Accepts either pixmap or bitmap.
// Assumes (x,y) is in the coordinate system of the bit/pixmap.
// Speed is enhanced by reusing the cached information from last time if it's the
// same Pix/Bitmap as last time, i.e. same address, baseAddr,rowBytes, and bounds.
// You can force it to flush its cache by passing a NULL PixMap address.
{
	static PixMapPtr oldPmPtr=(PixMapPtr)-1;
	static int oldX,oldY;
	static short rowBytes,logPixelSize,bitsOffset;
	static unsigned char *pixelPtr;
	static BitMap oldMap;
	int shift;
	unsigned char mask;
	char mode32=true32b;
	Rect r;
	unsigned long value;
	Boolean can32;
	long addrMode=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&addrMode);
	can32=!error && (addrMode&(1<<gestalt32BitCapable));
	if(!USE_CACHE || pmPtr!=oldPmPtr
		|| pmPtr->baseAddr!=oldMap.baseAddr 
		|| pmPtr->rowBytes!=oldMap.rowBytes 
		|| *(long *)&pmPtr->bounds.top!=*(long *)&oldMap.bounds.top
		|| *(long *)&pmPtr->bounds.bottom!=*(long *)&oldMap.bounds.bottom){
		// Cache is stale. Get fresh values.
		short pixelSize;
		
		SetRect(&r,x,y,x+1,y+1);
		pixelPtr=RectToAddress(pmPtr,&r,&rowBytes,&pixelSize,&bitsOffset);
		if(pixelPtr==NULL){
			oldPmPtr=(PixMapPtr)-1;	// invalidate cache
			return;
		}
		oldPmPtr=pmPtr;
		oldMap=*(BitMap *)pmPtr;
		logPixelSize=Log2L(pixelSize);
	}else{
		// Is already in cache. Merely correct for changes in x and y.
		if(pixelPtr==NULL)return;
		if(logPixelSize<3){
			register long bits;
			bits=bitsOffset+(long)(x-oldX)<<logPixelSize;
			pixelPtr+=bits>>3;
			bitsOffset=bits&7;
		}else pixelPtr+=(x-oldX)<<(logPixelSize-3);
		if(y!=oldY)pixelPtr+=(long)(y-oldY)*rowBytes;
	}
	if(can32)SwapMMUMode(&mode32);
	switch(logPixelSize){
	case 0:
		shift=8-1-bitsOffset;	// from right, instead of from left
		value=*pixelPtr>>shift & 1;
		break;
	case 1:
		shift=8-2-bitsOffset;	// from right, instead of from left
		value=*pixelPtr>>shift & 3;
		break;
	case 2:
		shift=8-4-bitsOffset;	// from right, instead of from left
		value=*pixelPtr>>shift & 15;
		break;
	case 3:
		value=*pixelPtr;
		break;
	case 4:
		value=*(unsigned short *)pixelPtr;
		break;
	case 5:
		value=*(unsigned long *)pixelPtr;
		break;
	}
	if(can32)SwapMMUMode(&mode32);
	oldX=x;
	oldY=y;
	return value;
}

void SetDevicePixel(GDHandle device,int x,int y,unsigned long value)
// Assumes (x,y) is relative to the upper left hand corner of the screen.
{
	if(device==NULL)return;
	x+=(*(*device)->gdPMap)->bounds.left;
	y+=(*(*device)->gdPMap)->bounds.top;
	SetPixmapPixel(*(*device)->gdPMap,x,y,value);
}

unsigned long GetDevicePixel(GDHandle device,int x,int y)
// Assumes (x,y) is relative to the upper left hand corner of the screen.
{
	if(device==NULL)return;
	x+=(*(*device)->gdPMap)->bounds.left;
	y+=(*(*device)->gdPMap)->bounds.top;
	return GetPixmapPixel(*(*device)->gdPMap,x,y);
}

/*
// Slightly faster than RectToAddress, but much less general. 
// Requires 8-bit pixelSize. It will give wrong answer when
// the pixmap is derived from a window in a multi-screen environment.
unsigned char *GetPixmapPixelAddress(PixMapPtr pmPtr,int x,int y)
{
	register unsigned char *pixelPtr;
	
	if(!(pmPtr->rowBytes & 0x8000) || pmPtr->pixelSize != 8) {
		PrintfExit("GetPixmapPixelAddress(): sorry, I require 8 bits/pixel.\007\n");
	}
	if(x<pmPtr->bounds.left || x>=pmPtr->bounds.right
		|| y<pmPtr->bounds.top || y>=pmPtr->bounds.bottom)return NULL;
	x-=pmPtr->bounds.left;
	y-=pmPtr->bounds.top;
	pixelPtr=(unsigned char *)pmPtr->baseAddr;
	pixelPtr+=y*(long)(pmPtr->rowBytes & 0x1fff);
	pixelPtr+=x;
	return pixelPtr;
}
*/

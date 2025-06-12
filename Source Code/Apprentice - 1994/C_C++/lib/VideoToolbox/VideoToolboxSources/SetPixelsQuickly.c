/*
SetPixelsQuickly.c
All the Set... routines poke a row of pixels.
All the Get... routines peek a row of pixels.
SetPixelsQuickly and GetPixelsQuickly use the current port.
SetWindowPixelsQuickly and GetWindowPixelsQuickly use the supplied window.
SetDevicePixelsQuickly and GetDevicePixelsQuickly use the supplied video device.
SetPixmapPixelsQuickly and GetPixmapPixelsQuickly use the supplied pix/bitmap.

int SetPixelsQuickly(int x,int y,unsigned long value[],short n);
int GetPixelsQuickly(int x,int y,unsigned long value[],short n);
int SetWindowPixelsQuickly(WindowPtr window,int x,int y,unsigned long value[],short n);
int GetWindowPixelsQuickly(WindowPtr window,int x,int y,unsigned long value[],short n);
int SetDevicePixelsQuickly(GDHandle device,int x,int y,unsigned long value[],short n);
int GetDevicePixelsQuickly(GDHandle device,int x,int y,unsigned long value[],short n);
int SetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[],short n);
int GetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[],short n);

This is the fastest and easiest way to transform back and forth between a PixMap
or BitMap and a numerical representation of an arbitrary image amenable to
computation in C. You can think of these routines as replacements for the
official Apple SetCPixel, etc. Their virtue is that they're very fast and don't
translate your index via any color table. Set... and Get... ignore the color
spec arrays, giving you direct access to the unsigned number stored in each
pixel (whether 1, 2, 4, 8, 16, or 32 bits). With appropriate casting you may
supply a CWindowPtr in place of the WindowPtr, and you may supply a BitMapPtr in
place of the PixMapPtr.

For all the routines, you supply the (x,y) coordinate of the first pixel you
want to access in the appropriate coordinate system, an unsigned long array
called "value", and the number of pixels to access. x will increment for each
successive pixel. Set... will copy values, one by one, from "value" into the
pix/bitmap. Get... will copy from the pix/bitmap into "value". The routines work
with all pixel sizes: 1 to 32 bits. The "value" array that you supply is always
unsigned long.

All the routines clip to enforce either the pix/bitmap's bounds, or the window's
portRect (if you pass a window or use the current window/port). The only way of
getting into trouble would be to pass a window's PixMap directly. It is
difficult to figure out the clipping of a window's PixMap; RectToAddress does
it, but only on the first call, after which we use the old cached answer, which
doesn't include clipping information.

The returned value of the function is normally zero, indicating success, but
will be nonzero if the request could not be fully satisfied. All the routines
clip to the bit/pixmap bounds; this is treated as normal by the Set... routines
and is reported as an error by the Get... routines. In the latter case some of
the elements of the value array will have been left untouched because the pixels
they correspond to could not be accessed.

These routines (and RectToAddress) do not "move memory", i.e. they don't give
the Memory Manager any pretext for shuffling around the memory allocations and
changing its master pointers. That's why it's OK to dereference the pixmap handle,
passing as an argument a temporary copy of the pixmap's master pointer.

SetPixmapPixelsQuickly and GetPixmapPixelsQuickly run fast by caching the
information that they get about your Bit/Pixmap from RectToAddress. Set.. and
Get.. each have their own cache. The cache is assumed to be fresh (i.e. is not
recomputed) if it receives the same Pix/Bitmap as last time. It checks whether
the pix/bitmap has the same address, baseAddr, rowBytes, and bounds. However, if
the pix/bitmap has the same baseAddr as the mainDevice, yet actually corresponds
to another device, then the cache is not used, because in a multi-screen
environment QuickDraw associates all windows' pixmaps with the baseAddr of the
main device (to create the illusion of a continuous desktop extending across
multiple screens). You can force a flush of the cache by passing a NULL
bit/pixmap pointer. (This will result in a returned value of 0, since the flush
is always successful.)

See the demo Grating.c for an example of how to use this to display a pattern on
the screen.

	// This is a simple write-then-read test of these routines,
	// writing random numbers to the top line of the main screen.
	unsigned long row[100],row2[100];
	int rowLength=100,clutSize,i;
	device=GetMainDevice();
	clutSize=GDClutSize(device);
	for(i=0;i<rowLength;i++)row[i]=nrand(clutSize);
	SetDevicePixelsQuickly(device,0,0,row,rowLength);
	GetDevicePixelsQuickly(device,0,0,row2,rowLength);
	for(i=0;i<rowLength;i++)if(row2[i]!=row[i])
		printf("%d-th pixel: wrote %ld, but read %ld\n",i,row[i],row2[i]);

HISTORY:
4/4/89	 dgp wrote SetIPixel.c
9/8/90	 dgp updated to work with 32 bit QuickDraw, if present.
10/15/90 bf renamed to SetIPixelGW.c and modified for drawing to off screen pix maps.
4/26/92	 dgp Merged the two variants: SetIPixel.c and SetIPixelGW.c to produce the
			new file SetOnePixel.c. Renamed existing routines to SetPixmapPixel,
			GetPixmapPixel,SetDevicePixel,GetDevicePixel. Added SetOnePixel and 
			GetOnePixel. Generalized to handle any pixelSize, and accept bitmaps as well
			as pixmaps.
12/23/92 dgp Doubled the speed of SetPixmapPixel and GetPixmapPixel (and thus sped up
			all the routines that call them) by caching the answers from RectToAddress. 
1/6/93 dgp	Fixed tiny but disastrous bug in GetPixmapPixel (wasn't saving old x and y).
1/22/93	dgp	Check more PixMap fields to make sure cache is not stale.
2/7/93	dgp Wrote SetPixelsQuickly.c
2/8/93	dgp Ironed out some wrinkles in the clipping.
4/27/93	dgp	Invalidate cache if baseAddr==mainDeviceBaseAddr.
6/4/93	 dhb Modified to deal with MEX file weirdness in 24-bit mode.
			These are all conditionally compiled in with the MATLAB
			symbol. No intentional changes to original.
		dgp	Here's my attempt to explain the problem and solution. 
			MATLAB puts garbage in the high byte of the program counter register, 
			so the machine crashes the instant you switch into 32-bit mode. 
			The fix is to call a subroutine whose address has been cleaned by calling
			StripAddress. This puts a 32-bit-clean address into the program counter,
			until the subroutine returns. 
	    dhb The SwapMMUMode calls are arranged so that when MATLAB is defined only
	        a minimal amount of code is run in 32-bit mode.  The problem is that 
	        register A4, which is used in global addressing, also has garbage in
	        the high byte.  Not only is A4 used to reference globals, but as far
	        as I could tell, it is also used as an offset in certain code jumps. 
	        So if it is wrong, all hell breaks loose.  The solution is to run only
	        the code that blits the image in 32-bit mode.  I expect that this problem
	        will be fixed by a future version of MATLAB.
		dgp	THINK C 5 sometimes implemented "switch" by a subroutine call, that might
			explain the need to put the SwapMMUMode calls inside the switch, since
			any subroutine call will fail when the global address register contains
			garbage.
6/23/93	dgp Used THINK C preprocessor and MPW Compare to confirm that
			code is unchanged when MATLAB is false.
7/9/93	dgp	Replaced calls to QD32Exists() by the variable can32, which is quicker,
			and based on a more appropriate test: gestalt32BitCapable.
			Test MATLAB in if() instead of #if. This is easier to read and
			the compiler is smart enough to evaluate it at compile time so there's 
			no runtime penalty.
4/11/94	dgp	In response to request from David Brainard, don't flush cache
			when the pixmap truly refers to the main screen. Pixmaps for
			screens other than the main screen still require a flush and call 
			to RectToAddress in order to clip by the appropriate device bounds.
			This wouldn't be necessary if we retained the old device or its
			bounds, but unfortunately RectToAddress doesn't return them.
*/
#include "VideoToolbox.h"
#define USE_CACHE 1	// set to zero to disable cache

int SetPixelsQuickly(int x,int y,unsigned long value[],short n)
// (x,y) is in the local coordinate system of the current port.
{
	WindowPtr window;

	GetPort(&window);
	return SetWindowPixelsQuickly(window,x,y,value,n);
}

int GetPixelsQuickly(int x,int y,unsigned long value[],short n)
// (x,y) is in the local coordinate system of the current port.
{
	WindowPtr window;

	GetPort(&window);
	return GetWindowPixelsQuickly(window,x,y,value,n);
}

int SetWindowPixelsQuickly(WindowPtr window,int x,int y,unsigned long *value,short n)
// (x,y) is in the local coordinate system of the window.
// Accepts either WindowPtr or CWindowPtr.
{
	PixMapPtr pm;
	Rect r;
	
	if(window==NULL)return 1;
	
	// Clip to portRect.
	SetRect(&r,x,y,x+n,y+1);
	if(!SectRect(&window->portRect,&r,&r))return 0;
	value+=r.left-x;
	x=r.left;
	n=r.right-r.left;
	
	// Is it a CGrafPort or a GrafPort?
	if(((CGrafPtr)window)->portVersion<0)		// It's a CGrafPort, pass pixmap ptr.
		pm = *((CGrafPtr)window)->portPixMap;
	else 										// It's a GrafPort, pass bitmap ptr.
		pm = (PixMapPtr) &window->portBits;
	return SetPixmapPixelsQuickly(pm,x,y,value,n);
}

int GetWindowPixelsQuickly(WindowPtr window,int x,int y,unsigned long *value,short n)
// (x,y) is in the local coordinate system of the window.
// Accepts WindowPtr or CWindowPtr.
{
	PixMapPtr pm;
	int error=0;
	Rect r;

	if(window==NULL)return 1;
	
	// Clip to portRect.
	SetRect(&r,x,y,x+n,y+1);
	if(!SectRect(&window->portRect,&r,&r))return 1;
	if(x!=r.left || x+n!=r.right){	// Update after clipping.
		error=1;
		value+=r.left-x;
		x=r.left;
		n=r.right-r.left;
	}
	
	// Is it a CGrafPort or a GrafPort?
	if(((CGrafPtr)window)->portVersion<0)		// It's a CGrafPort, pass pixmap ptr.
		pm = *((CGrafPtr)window)->portPixMap;
	else 										// It's a GrafPort, pass bitmap ptr.
		pm = (PixMapPtr) &window->portBits;
	error|=GetPixmapPixelsQuickly(pm,x,y,value,n);
	return error;
}

int SetDevicePixelsQuickly(GDHandle device,int x,int y,unsigned long value[],short n)
// (x,y) is relative to the upper left hand corner of the screen.
{
	Rect r;
	
	if(device==NULL)return 1;
	x+=(*(*device)->gdPMap)->bounds.left;
	y+=(*(*device)->gdPMap)->bounds.top;

	// Clip to device bounds.
	SetRect(&r,x,y,x+n,y+1);
	if(!SectRect(&(*(*device)->gdPMap)->bounds,&r,&r))return 0;
	value+=r.left-x;	// Update after clipping.
	x=r.left;
	n=r.right-r.left;
	
	return SetPixmapPixelsQuickly(*(*device)->gdPMap,x,y,value,n);
}

int GetDevicePixelsQuickly(GDHandle device,int x,int y,unsigned long value[],short n)
// (x,y) is relative to the upper left hand corner of the screen.
{
	int error=1;
	Rect r;
	
	if(device==NULL)return 1;
	x+=(*(*device)->gdPMap)->bounds.left;
	y+=(*(*device)->gdPMap)->bounds.top;
	
	// Clip to device bounds.
	SetRect(&r,x,y,x+n,y+1);
	if(!SectRect(&(*(*device)->gdPMap)->bounds,&r,&r))return 1;
	if(x!=r.left || x+n!=r.right){	// Update after clipping.
		error=1;
		value+=r.left-x;
		x=r.left;
		n=r.right-r.left;
	}
	error|=GetPixmapPixelsQuickly(*(*device)->gdPMap,x,y,value,n);
	return error;
}

#if MATLAB
int SetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
	,register short n)
{
  int (*goSet) (PixMapPtr pmPtr,int x,int y,unsigned long value[],register short n);
  int SPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
  	,register short n);

  goSet = (void *) StripAddress(&SPixmapPixelsQuickly);
  return( (*goSet) (pmPtr,x,y,value,n) );
}
static int SPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
	,register short n)
#else
int SetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
	,register short n)
#endif
// Pokes a row of pixels. Accepts either pixmap or bitmap pointer.
// (x,y) is in the coordinate system of the bit/pixmap.
// Speed is enhanced by reusing the cached information from last time if it's the
// same Pix/Bitmap as last time, i.e. same address, baseAddr, rowBytes, and bounds.
// You can flush this cache by passing a NULL bit/pixmap pointer.
{
	static PixMapPtr oldPmPtr=(PixMapPtr)-1;
	static int oldX,oldY;
	static short rowBytes,logPixelSize,bitsOffset;
	static unsigned char *pixelPtr;
	static BitMap oldMap;
	static Ptr mainBaseAddr=NULL;
	static Boolean can32=0;
	char mode32=true32b;
	int shift,error=0;
	unsigned char mask;
	Rect r;
	Boolean otherScreen;	// masquerading as extension of main screen
	
	if(pmPtr==NULL){
		oldPmPtr=(PixMapPtr)-1;	// invalidate cache
		return 0;
	}
	// Clip to pix/bitmap bounds.
	SetRect(&r,x,y,x+n,y+1);
	if(mainBaseAddr==NULL){
		if(QD8Exists())mainBaseAddr=(*(*GetMainDevice())->gdPMap)->baseAddr;
		else mainBaseAddr=(void *)-1;
	}
	otherScreen= pmPtr->baseAddr==mainBaseAddr 
		&& (pmPtr->bounds.top!=0 || pmPtr->bounds.left!=0);
	// Clip unless it's other screen.
	if(!otherScreen)
		if(!SectRect(&pmPtr->bounds,&r,&r))return 0;	// go home if we're done
	if(!USE_CACHE
		|| otherScreen
		|| pmPtr!=oldPmPtr
		|| pmPtr->baseAddr!=oldMap.baseAddr 
		|| pmPtr->rowBytes!=oldMap.rowBytes 
		|| *(long *)&pmPtr->bounds.top!=*(long *)&oldMap.bounds.top
		|| *(long *)&pmPtr->bounds.bottom!=*(long *)&oldMap.bounds.bottom){
		// Cache is stale. Get fresh values.
		short pixelSize;
		long value=0;
		
		error=Gestalt(gestaltAddressingModeAttr,&value);
		can32=!error && (value&(1<<gestalt32BitCapable));
		
		// RectToAddress computes pixelPtr and clips r to the bit/pixmap bounds.
		pixelPtr=RectToAddress(pmPtr,&r,&rowBytes,&pixelSize,&bitsOffset);
		if(pixelPtr==NULL){
			oldPmPtr=(PixMapPtr)-1;	// invalidate cache
			return 0;
		}
		oldPmPtr=pmPtr;
		oldMap=*(BitMap *)pmPtr;
		logPixelSize=Log2L(pixelSize);
		value+=r.left-x;	// Update after clipping.
		x=r.left;
		n=r.right-r.left;
	}else{
		// Cache is fresh. Merely correct for changes in x and y.
		if(pixelPtr==NULL)return 1;
		value+=r.left-x;	// Update after clipping.
		x=r.left;
		n=r.right-r.left;
		if(x!=oldX){
			if(logPixelSize<3){
				register long bits;
				bits=bitsOffset+(long)(x-oldX)<<logPixelSize;
				pixelPtr+=bits>>3;
				bitsOffset=bits&7;
			}else pixelPtr+=(x-oldX)<<(logPixelSize-3);
		}
		if(y!=oldY)pixelPtr+=(long)(y-oldY)*rowBytes;
	}
	if(!MATLAB && can32)SwapMMUMode(&mode32);
	switch(logPixelSize){
	case 0:{
		register unsigned char val,mask,*ptr=pixelPtr;
		shift=sizeof(*ptr)*8;
		shift-=bitsOffset;	// from right, instead of from left
		if(MATLAB && can32)SwapMMUMode(&mode32);
		do{
			val=mask=0;
			do{
				shift-=1;
				val<<=1;
				mask<<=1;
				if(n>0){
					val|=(*value++)&1;
					mask|=1;
					n--;
				}else{
					val<<=shift;
					mask<<=shift;
					break;
				}
			}while(shift>0);
			mask=~mask;
			*ptr= *ptr & mask | (unsigned char)val;
			ptr++;
			shift=sizeof(*ptr)*8;
		}while(n>0);
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 1:{
		register unsigned char val,mask,*ptr=pixelPtr;
		shift=sizeof(*ptr)*8;
		shift-=bitsOffset;	// from right, instead of from left
		if(MATLAB && can32)SwapMMUMode(&mode32);
		do{
			val=mask=0;
			do{
				shift-=2;
				val<<=2;
				mask<<=2;
				if(n>0){
					val|=(*value++)&3;
					mask|=3;
					n--;
				}else{
					val<<=shift;
					mask<<=shift;
					break;
				}
			}while(shift>0);
			mask=~mask;
			*ptr= *ptr & mask | (unsigned char)val;
			ptr++;
			shift=sizeof(*ptr)*8;
		}while(n>0);
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 2:{
		register unsigned char val,mask,*ptr=pixelPtr;
		shift=sizeof(*ptr)*8;
		shift-=bitsOffset;	// from right, instead of from left
		if(MATLAB && can32)SwapMMUMode(&mode32);
		do{
			val=mask=0;
			do{
				shift-=4;
				val<<=4;
				mask<<=4;
				if(n>0){
					val|=(*value++)&15;
					mask|=15;
					n--;
				}else{
					val<<=shift;
					mask<<=shift;
					break;
				}
			}while(shift>0);
			mask=~mask;
			*ptr= *ptr & mask | (unsigned char)val;
			ptr++;
			shift=sizeof(*ptr)*8;
		}while(n>0);
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 3:{
		register unsigned char *pB=pixelPtr;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		for(;n>0;n--) *pB++ = *value++;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 4:{
		register unsigned short *pW=(unsigned short *)pixelPtr;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		for(;n>0;n--) *pW++ = *value++;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 5:{
		register unsigned long *pL=(unsigned long *)pixelPtr;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		for(;n>0;n--) *pL++ = *value++;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	}
	if(!MATLAB && can32)SwapMMUMode(&mode32);
	oldX=x;
	oldY=y;
	return error;
}

#if MATLAB
int GetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
	,register short n)
{
  int (*goSet) (PixMapPtr pmPtr,int x,int y,unsigned long value[],register short n);
  int GPixmapPixelsQuickly (PixMapPtr pmPtr,int x,int y,unsigned long value[],register short n);

  goSet = (void *) StripAddress(&GPixmapPixelsQuickly);
  return( (*goSet) (pmPtr,x,y,value,n) );
}
static int GPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
	,register short n)
#else
int GetPixmapPixelsQuickly(PixMapPtr pmPtr,int x,int y,unsigned long value[]
	,register short n)
#endif
// Peeks a rows of pixels of any size. Accepts either pixmap or bitmap pointer.
// (x,y) is in the coordinate system of the bit/pixmap.
// Speed is enhanced by reusing the cached information from last time if it's the
// same Pix/Bitmap as last time, i.e. same address, baseAddr,rowBytes, and bounds.
// You can force it to flush its cache by passing a NULL bit/pixmap pointer.
{
	static PixMapPtr oldPmPtr=(PixMapPtr)-1;
	static int oldX,oldY;
	static short rowBytes,logPixelSize,bitsOffset;
	static unsigned char *pixelPtr;
	static BitMap oldMap;
	static Ptr mainBaseAddr=NULL;
	static Boolean can32=0;
	char mode32=true32b;
	int shift,error=0;
	unsigned char mask;
	Rect r;
	Boolean otherScreen;	// masquerading as extension of main screen
	
	if(pmPtr==NULL){
		oldPmPtr=(PixMapPtr)-1;	// invalidate cache
		return 0;
	}
	// Clip to pix/bitmap bounds.
	SetRect(&r,x,y,x+n,y+1);
	if(mainBaseAddr==NULL){
		if(QD8Exists())mainBaseAddr=(*(*GetMainDevice())->gdPMap)->baseAddr;
		else mainBaseAddr=(void *)-1;
	}
	otherScreen= pmPtr->baseAddr==mainBaseAddr 
		&& (pmPtr->bounds.top!=0 || pmPtr->bounds.left!=0);
	// Clip unless it's other screen.
	if(!otherScreen)
		if(!SectRect(&pmPtr->bounds,&r,&r))return 1;	// go home if we're done
	if(!USE_CACHE
		|| otherScreen
		|| pmPtr!=oldPmPtr
		|| pmPtr->baseAddr!=oldMap.baseAddr 
		|| pmPtr->rowBytes!=oldMap.rowBytes 
		|| *(long *)&pmPtr->bounds.top!=*(long *)&oldMap.bounds.top
		|| *(long *)&pmPtr->bounds.bottom!=*(long *)&oldMap.bounds.bottom){
		// Cache is stale. Get fresh values.
		short pixelSize;
		long value=0;

		error=Gestalt(gestaltAddressingModeAttr,&value);
		can32=!error && (value&(1<<gestalt32BitCapable));

		// RectToAddress computes pixelPtr and clips r to the bit/pixmap bounds.
		pixelPtr=RectToAddress(pmPtr,&r,&rowBytes,&pixelSize,&bitsOffset);
		if(pixelPtr==NULL){
			oldPmPtr=(PixMapPtr)-1;	// invalidate cache
			return 0;
		}
		oldPmPtr=pmPtr;
		oldMap=*(BitMap *)pmPtr;
		logPixelSize=Log2L(pixelSize);
		if(x!=r.left || x+n!=r.right){	// Update after clipping.
			error=1;
			value+=r.left-x;
			x=r.left;
			n=r.right-r.left;
		}
	}else{
		// Cache is fresh. Merely correct for changes in x and y.
		if(pixelPtr==NULL)return 1;
		if(x!=r.left || x+n!=r.right){	// Update after clipping.
			error=1;
			value+=r.left-x;
			x=r.left;
			n=r.right-r.left;
		}
		if(x!=oldX){
			if(logPixelSize<3){
				register long bits;
				bits=bitsOffset+(long)(x-oldX)<<logPixelSize;
				pixelPtr+=bits>>3;
				bitsOffset=bits&7;
			}else pixelPtr+=(x-oldX)<<(logPixelSize-3);
		}
		if(y!=oldY)pixelPtr+=(long)(y-oldY)*rowBytes;
	}
	if(!MATLAB && can32)SwapMMUMode(&mode32);
	switch(logPixelSize){
	case 0:{
		register unsigned char val,*ptr=pixelPtr;
		shift=sizeof(*ptr)*8;
		shift-=bitsOffset;	// from right, instead of from left
		if(MATLAB && can32)SwapMMUMode(&mode32);
		do{
			val=*ptr++;
			do{
				shift-=1;
				if(n>0){
					*value++=(val>>shift)&1;
					n--;
				}else{
					break;
				}
			}while(shift>0);
			shift=sizeof(*ptr)*8;
		}while(n>0);
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 1:{
		register unsigned char val,*ptr=pixelPtr;
		shift=sizeof(*ptr)*8;
		shift-=bitsOffset;	// from right, instead of from left
		if(MATLAB && can32)SwapMMUMode(&mode32);
		do{
			val=*ptr++;
			do{
				shift-=2;
				if(n>0){
					*value++=(val>>shift)&3;
					n--;
				}else{
					break;
				}
			}while(shift>0);
			shift=sizeof(*ptr)*8;
		}while(n>0);
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 2:{
		register unsigned char val,*ptr=pixelPtr;
		shift=sizeof(*ptr)*8;
		shift-=bitsOffset;	// from right, instead of from left
		if(MATLAB && can32)SwapMMUMode(&mode32);
		do{
			val=*ptr++;
			do{
				shift-=4;
				if(n>0){
					*value++=(val>>shift)&15;
					n--;
				}else{
					break;
				}
			}while(shift>0);
			shift=sizeof(*ptr)*8;
		}while(n>0);
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 3:{
		register unsigned char *pB=pixelPtr;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		for(;n>0;n--) *value++=*pB++;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 4:{
		register unsigned short *pW=(unsigned short *)pixelPtr;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		for(;n>0;n--) *value++=*pW++;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	case 5:{
		register unsigned long *pL=(unsigned long *)pixelPtr;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		for(;n>0;n--) *value++=*pL++;
		if(MATLAB && can32)SwapMMUMode(&mode32);
		break;
	}
	}
	if(!MATLAB && can32)SwapMMUMode(&mode32);
	oldX=x;
	oldY=y;
	return error;
}

/* 
ConvolveX/Y.c
© 1989-1993 Denis G. Pelli
Simple one-dimensional convolution. Convolve image in srcBits by array f[] to
produce image dstBits. src and dst may have any pixelSize, but the calculation
itself is done with 8 bits per pixel. Assumes that everything has its origin at
its center. Before calling ConvolveX/Y you should call SetEntries() to set up
the color tables appropriately (unless source and destination pixmaps are both 8
bits/pixel, in which case the color tables are ignored).

ConvolveX/Y fills each pixel in the destination with a weighted sum over the
source, centered on the corresponding pixel in the source. To avoid edge
effects, it only computes pixels in the destination which have full support in
the source. If necessary, ConvolveX/Y will inset the source or destination rect,
so that the destination width/height plus the array length equals the source
width/height.

It would be nice to convert this to use Get and SetPixelsQuickly instead of
directly accessing the pixels. This would introduce a small overhead, since the
numbers would be copied into a long array and from thence into the pixels, instead
of being stuffed directly, but would probably make this program quite a bit easier
to read.

BUGS:
Setting IMPROVED_ROUNDING to true improves results in some applications, but
screws up others (e.g. the demo Filter.c). I don't have time to figure this out at 
the moment, so I'm leaving a flag that can be turned on or off to suit your 
application. I expect to fix this and eliminate the flag in the future.

HISTORY:
4/1/89 dgp	twice as fast, thanks to recoding to use long instead of float arithmetic.
4/2/89 dgp	made universal by using line buffers for both src and dst, and interfacing
			to the src and dst PixMaps solely through CopyBits.
4/2/89 dgp	ConvolveX/Y.c, a pre-processor conditional was added to generate ConvolveX
			and ConvolveY from the SAME source. The two files are IDENTICAL except
			for the preprocessor definition of CONVOLVE_X as 1 to generate ConvolveX,
			or 0 to generate ConvolveY.
10/9/89 dgp	Fixed overflow bug. Speeded up the convolution loop slightly.
			Added conditional to use CopyBitsQuickly instead of CopyBits when source and
			destination pixelSize are 8 bits. This has two virtues. It's faster, and
			it bypasses the Color Manager's transformations through the color and inverse
			color tables, which costs time and loses accuracy.
10/10/89 dgp Now use RectToAddress to determine the pixelSize of src and dst, to deal
			correctly with window's pixmap.
7/25/91	dgp	Added explicit preprocessor symbol to turn diagnostic printout on and off.
8/24/91	dgp	Made compatible with THINK C 5.0.
8/27/92	dgp	Added Gestalt().
11/10/92 dgp Now initialize all fields of pixmap, especially the PMVersion.
			Identify all error messages as originating from here: ConvolveX/Y.
			Replaced compile-time conditional by a run-time conditional, so
			that the ConvolveY.c file is no longer needed.
1/22/93	dgp	SwapMMUMode().
2/1/93	dgp Improved rounding. Removed calls to Gestalt, as unnecessary overhead.
2/7/93	dgp	Introduced switch IMPROVED_ROUNDING to disable rounding for debugging.
2/8/93	dgp Fixed overflow problem caused by the new rounding.
7/9/93	dgp check for 32-bit addressing capability.
*/
#include "VideoToolbox.h"
#include <math.h>
void ConvolveXY(double f[],int dim,BitMap *srcBits
	,BitMap *dstBits,Rect *srcRectPtr,Rect *dstRectPtr,int convolveX);
#define DIAGNOSTICS 0		// true or false
#define IMPROVED_ROUNDING 1	// true or false

void ConvolveX(double f[],int dim,BitMap *srcBits
	,BitMap *dstBits,Rect *srcRectPtr,Rect *dstRectPtr)
{
	ConvolveXY(f,dim,srcBits,dstBits,srcRectPtr,dstRectPtr,1);
}

void ConvolveY(double f[],int dim,BitMap *srcBits
	,BitMap *dstBits,Rect *srcRectPtr,Rect *dstRectPtr)
{
	ConvolveXY(f,dim,srcBits,dstBits,srcRectPtr,dstRectPtr,0);
}

void ConvolveXY(double f[],int dim,BitMap *srcBits
	,BitMap *dstBits,Rect *srcRectPtr,Rect *dstRectPtr,int convolveX)
{
	Rect mysrcRect,mydstRect;
	int dstLines,srcLines,dstLength,srcLength;
	int lines;
	unsigned char *dstPtr=NULL;
	register unsigned char *srcPtr=NULL;
	register int i;
	Rect dstRect, srcRect;
	register long t;
	register long *ifun, *fPtr;
	long ifunScale;
	int x,y,srcx,imin,imax;
	double tmax;
	PixMap srcTmp,dstTmp;	/* line buffers */
	#if DIAGNOSTICS
		GDHandle device=NULL;
		double a;
	#endif
	short srcPixelSize,dstPixelSize;
	int error;
	long value;
	char mmuMode=true32b;
	Boolean can32;
	
	value=0;
	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	RectToAddress((PixMap *)srcBits,srcRectPtr,NULL,&srcPixelSize,NULL);
	RectToAddress((PixMap *)dstBits,dstRectPtr,NULL,&dstPixelSize,NULL);
	ifun = (long *) malloc(dim*sizeof(*ifun));
	if(ifun == NULL) {
		PrintfExit("ConvolveX/Y: Sorry, couldn't allocate enough memory for working array\007\n");
	}
	/*
	We will sum in a long register.
	The sum over the point spread function f[] will yield at most tmax, defined below,
	so we can safely multiply f[] by LONG_MAX/tmax, without danger of overflowing t.
	*/
	for (tmax=0.0, i=0;i<dim;i++) tmax += fabs(f[i]);
	tmax *= UCHAR_MAX+1;	/* maximum value for a pixel, 255, plus 1 for rounding */
	if(tmax<1.0)tmax=1.0;
	ifunScale = LONG_MAX/tmax;
	for (i=0;i<dim;i++) ifun[i] = f[i]*ifunScale;
	
	#if DIAGNOSTICS
		/* This is a diagnostic printout to track down overflow problems */
		t = 0L;
		a = 0.0;
		for (i=0;i<dim;i++) {
			t += labs(UCHAR_MAX*ifun[i]);
			a += fabs(f[i]);
		}
		device=GetGDevice();
		SetGDevice(GetMainDevice());
		PmForeColor(UCHAR_MAX);
		PmBackColor(0);
		PrintfExit("a=%lf, t=%lx, ifunScale=%lx, t/ifunScale=%lf\n",a,t,ifunScale,t*1.0/ifunScale);
		SetGDevice(device);
	#endif
	
	/*
	Unfortunately CopyBits will crash if the line buffer is too large, e.g.
	640 pixels long. We can minimize this problem by reducing the size of the
	srcRect to be no larger than necessary (i.e. dstRect plus dim).
	*/
	mysrcRect = *srcRectPtr;
	mydstRect = *dstRectPtr;
	if(convolveX){
		i=mysrcRect.right-mysrcRect.left - (dim+mydstRect.right-mydstRect.left);
		if(i>0)InsetRect(&mysrcRect,i/2,0);
		i=mysrcRect.bottom-mysrcRect.top - (mydstRect.bottom-mydstRect.top);
		if(i>0)InsetRect(&mysrcRect,0,i/2);
		if(i<0)InsetRect(&mydstRect,0,-i/2);
	}else{
		i=mysrcRect.right-mysrcRect.left - (mydstRect.right-mydstRect.left);
		if(i>0)InsetRect(&mysrcRect,i/2,0);
		if(i<0)InsetRect(&mydstRect,-i/2,0);
		i=mysrcRect.bottom-mysrcRect.top - (dim+mydstRect.bottom-mydstRect.top);
		if(i>0)InsetRect(&mysrcRect,0,i/2);
	}
	srcRectPtr = &mysrcRect;
	dstRectPtr = &mydstRect;

	if(convolveX){
		dstLines = dstRectPtr->bottom-dstRectPtr->top;
		srcLines = srcRectPtr->bottom-srcRectPtr->top;
		dstLength = dstRectPtr->right-dstRectPtr->left;
		srcLength = srcRectPtr->right-srcRectPtr->left;
	}else{
		dstLength = dstRectPtr->bottom-dstRectPtr->top;
		srcLength = srcRectPtr->bottom-srcRectPtr->top;
		dstLines = dstRectPtr->right-dstRectPtr->left;
		srcLines = srcRectPtr->right-srcRectPtr->left;
	}
	if(dstLines > srcLines) lines = srcLines; 
	else lines = dstLines;
	
	/* Allocate our line buffers. */
	srcTmp = **(*GetGDevice())->gdPMap;	/* Use current device to init PixMap fields */
	srcTmp.pmVersion=srcTmp.packType=srcTmp.packSize=0;
	srcTmp.planeBytes=srcTmp.pmReserved=0;
	srcTmp.cmpCount=1;
	srcTmp.pixelSize=8;					/* The main loop assumes 8 bits per pixel */
	if(convolveX){
		SetRect(&srcTmp.bounds,0,0,srcLength,1);	/* a horizontal line buffer */
	}else{
		SetRect(&srcTmp.bounds,0,0,1,srcLength);	/* a vertical line buffer */
	}
	srcTmp.rowBytes = 2*(((srcTmp.bounds.right-srcTmp.bounds.left)*srcTmp.pixelSize+15)/16);
	srcTmp.baseAddr = (Ptr) malloc(srcTmp.rowBytes*(srcTmp.bounds.bottom-srcTmp.bounds.top));
	srcTmp.rowBytes |= 0x8000;	/* Mark it as a PixMap, not a BitMap */
	dstTmp = srcTmp;
	if(convolveX){
		SetRect(&dstTmp.bounds,0,0,dstLength,1);	/* a horizontal line buffer */
	}else{
		SetRect(&dstTmp.bounds,0,0,1,dstLength);	/* a vertical line buffer */
	}
	dstTmp.rowBytes = 2*(((dstTmp.bounds.right-dstTmp.bounds.left)*dstTmp.pixelSize+15)/16);
	dstTmp.baseAddr = (Ptr) malloc(dstTmp.rowBytes*(dstTmp.bounds.bottom-dstTmp.bounds.top));
	dstTmp.rowBytes |= 0x8000;	/* Mark it as a PixMap, not a BitMap */
	if(srcTmp.baseAddr == NULL || dstTmp.baseAddr == NULL)
		PrintfExit("ConvolveX/Y: Sorry, couldn't allocate enough memory for working arrays\007\n");
	
	/* Define the rectangles that map src and dst to their respective line buffers */
	srcRect = srcTmp.bounds;
	OffsetRect(&srcRect,srcRectPtr->left,srcRectPtr->top);
	dstRect = dstTmp.bounds;
	OffsetRect(&dstRect,dstRectPtr->left,dstRectPtr->top);
	if(convolveX){
		OffsetRect(&srcRect,0,srcLines/2-lines/2);
		OffsetRect(&dstRect,0,dstLines/2-lines/2);
	}else{
		OffsetRect(&srcRect,srcLines/2-lines/2,0);
		OffsetRect(&dstRect,dstLines/2-lines/2,0);
	}
	
	/* Convolve one line per iteration */
	/* y indicates which line, x indicates which pixel along that line */
	for (y= -lines/2; y<(lines+1)/2; y++) {
		/* first copy a line from srcBits into line-buffer srcTmp */
		if(srcPixelSize == srcTmp.pixelSize)
			CopyBitsQuickly(srcBits,(BitMap *) &srcTmp,&srcRect,&srcTmp.bounds,srcCopy,NULL);
		else
			CopyBits(srcBits,(BitMap *) &srcTmp,&srcRect,&srcTmp.bounds,srcCopy,NULL);
		if(can32)SwapMMUMode(&mmuMode);
		if(!convolveX){
			/* Unfortunately the minimum PixMap rowBytes is 2 bytes, so we
			squeeze out the extra space before processing
			*/
			srcPtr= (unsigned char *) srcTmp.baseAddr;
			dstPtr= (unsigned char *) srcTmp.baseAddr;
			for (x=0; x<srcLength; x++) {
				*dstPtr++ = *srcPtr;
				srcPtr += 2;
			}
		}
		dstPtr= (unsigned char *) dstTmp.baseAddr;
		srcx = srcLength/2 - dim/2 - dstLength/2;
		for (x=0; x<dstLength; x++) {
			imax=dim;
			if(srcx+dim>srcLength) imax=srcLength-srcx;
			if(srcx<0){
				imin= -srcx;
				srcPtr=(unsigned char *) srcTmp.baseAddr;
			}
			else {
				imin=0;
				srcPtr=(unsigned char *) srcTmp.baseAddr+srcx;
			}
			fPtr=&ifun[imin];
			for (t=0L, i=imin;i<imax;i++) t += *fPtr++ * (long)(unsigned long) *srcPtr++;
#if IMPROVED_ROUNDING
			if(t>0)t+=ifunScale>>1;		// round t/ifunScale to nearest integer
			else t-=ifunScale>>1;
#endif
			t/=ifunScale;
			if(convolveX){
				*dstPtr++ = (unsigned char) t;
			}else{
				/* Unfortunately the minimum PixMap rowBytes is 2 bytes, so we
				insert a space between pixels
				*/
				*dstPtr = (unsigned char) t;
				dstPtr += 2;
			}
			srcx++;
		}
		if(can32)SwapMMUMode(&mmuMode);
		/* finally, copy the line from buffer dstTmp to dstBits */
		if(dstPixelSize == dstTmp.pixelSize)
			CopyBitsQuickly((BitMap *) &dstTmp,dstBits,&dstTmp.bounds,&dstRect,srcCopy,NULL);
		else
			CopyBits((BitMap *) &dstTmp,dstBits,&dstTmp.bounds,&dstRect,srcCopy,NULL);
		if(convolveX){
			OffsetRect(&srcRect,0,1);	/* adjust Rects to point to next line */
			OffsetRect(&dstRect,0,1);
		}else{
			OffsetRect(&srcRect,1,0);	/* adjust Rects to point to next line */
			OffsetRect(&dstRect,1,0);
		}
	}
	free((void *) ifun);
	free((void *) srcTmp.baseAddr);
	free((void *) dstTmp.baseAddr);
}

	
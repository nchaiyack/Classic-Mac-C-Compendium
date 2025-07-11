/*
CopyBitsQuickly.c

CopyBitsQuickly.c is a dumb substitute for CopyBits that ignores the color
tables and palettes and leaves the clut alone, simply copying the raw pixels
without any translation. It's for doing animations. (Try the demo Sandstorm.)
Besides copying images, it can also add or multiply them. At one time it copied
much faster than CopyBits did, but the latest timing (under System 7.01), by
TimeVideo, indicates that they are of equal speed.

Apple's CopyBits is an Apple Macintosh Toolbox routine for copying images, and
is documented in Inside Macintosh Volumes I,V, and VI. Unlike CopyBits,
CopyBitsQuickly does not cause the Memory Manager to move memory, and thus may
be used in a VBL task.

The returned value is nonzero if an error occurred.

CopyBitsQuickly supports three modes:

srcCopy causes the source to be copied to the destination.

addOver causes the source and destination to be added together. Both must have 
8-bit pixels.  Overflow is ignored.

mulOver causes the source and destination to be multiplied, pixel by
pixel. Both must have 8-bit pixels. After multiplication, the product is divided
by 128 and stored in the destination. Overflow is ignored. All the arithmetic
is unsigned.

RESTRICTIONS:
It insists that srcBits and dstBits both have the same number of bits/pixel.
It insists that dstRect be the same size as srcRect
It insists that mode==srcCopy, addOver, or new multiplyQuickly.
It insists that maskRgn==NULL
If a Rect extends across multiple screens only as much of the upper-left of the
Rect that's on one device will be used. The rest is clipped off.
If mode is addOver or mulOver then the pixel size must be 8 bits, in both
source and destination.

If CopyBitsQuickly detects a violation of any of these restrictions it will return
a nonzero value, indicating that an error occured.

NOTE: For highest speed you should choose your srcRectPtr & dstRectPtr so that the
first point moved to and from each row begins at a memory address that is a multiple
of 4 bytes. The effect on speed is substantial, about 25%. 

NOTE: If your computer boots in 24-bit mode, as set by the Memory Control Panel,
then the THINK C Debugger will crash if it's activated while you've temporarily
switched into 32-bit mode. So don't put any breakpoints in the section of the
code bracketed by calls to SwapMMUMode() unless your computer booted up in
32-bit mode. If your computer boots in 32-bit mode then the calls to SwapMMUMode
do nothing, and you can put Debugger breakpoints anywhere.

Copyright �1989-1993 Denis G. Pelli. 

HISTORY:
1/89 dgp	Version 2.0: added support for PixMaps and multiple screens. Added checking.
6/89 dgp	Version 3.0: now use RectToAddress, which clips to one device.
10/89 dgp	Version 3.5: Improved resolution from longs to bytes.
10/89 dgp	Version 4.0: Added new mode: addOver
3/90  dgp	Version 4.01: Made cosmetic changes:
			renamed srcRect & dstRect to srcRectPtr and dstRectPtr.
			renamed srcAdd to addOver, to conform to CopyBits.
			added a few more comments to explain the initial clipping.
3/20/90	dgp	made compatible with MPW C.
4/20/90	dgp	now uses 32-bit addressing only if QD32 is present.
4/9/91	dgp	v 4.05: changed nudge from short to long, just to be safe
8/24/91	dgp	Made compatible with THINK C 5.0.
4/15/92	dgp	Updated CopyBitsQuickly's function header to Standard C style.
10/5/92 dgp	Dropped support for THINK C 4. Updated the documentation above.
12/2/92 dgp cosmetic changes
12/8/92 dgp fixed major gaffe introduced on 12/2/92: "case" prefix was 
			missing in switch statement. This caused CopyBitsQuickly to do nothing. 
1/31/93	dgp	Added new "multiplyQuickly" mode requested by Josh Solomon. Now 
			insist on 8-bit pixels for both addOver and multiplyQuickly modes.
2/18/93	js	added mulOver to list of allowed modes. (Oops! - dgp.) Works ok now.
2/18/93	dgp	Now return int, nonzero if error occurred.
7/9/93	dgp check for 32-bit addressing capability.
*/
#include "VideoToolbox.h"

typedef unsigned char *UPtr;

static void srcCopyQuickly(UPtr Src,unsigned short srcinc,
	UPtr Dst,unsigned short dstinc,
	unsigned long bytes,unsigned long lines);
static void addOverQuickly(UPtr Src,unsigned short srcinc,
	UPtr Dst,unsigned short dstinc,
	unsigned long bytes,unsigned long lines);
static void mulOverQuickly(register UPtr Src,register unsigned short srcinc,
	register UPtr Dst,register unsigned short dstinc,
	unsigned long bytes,unsigned long lines);

typedef union {
	unsigned long *L;
	unsigned short *W;
	unsigned char *B;
} unsignedPtr;

int CopyBitsQuickly(BitMap *srcBits,BitMap *dstBits
	,Rect *srcRectPtr,Rect *dstRectPtr,int srcMode,RgnHandle maskRgn)
{
	UPtr Src,Dst;
	unsigned short srcinc,dstinc;
	unsigned long lines;
	short srcRowBytes,dstRowBytes;
	short srcPixelSize,dstPixelSize;
	short srcBitsOffset,dstBitsOffset;
	Rect mySrcRect,myDstRect;
	int dx,dy;
	long nudge;
	long bytes;

	if(srcMode != srcCopy && srcMode != addOver && srcMode != mulOver) return 1;
	if(maskRgn != NULL) return 1;

	/* clip the rect to be copied by the bounds of source and destination */
	mySrcRect=*srcRectPtr;
	myDstRect=*dstRectPtr;
	/* first make sure that srcRect and dstRect are the same size */
	if(mySrcRect.bottom-mySrcRect.top != myDstRect.bottom-myDstRect.top || 
		mySrcRect.right-mySrcRect.left != myDstRect.right-myDstRect.left) 
			return 2;
	dx=myDstRect.left-mySrcRect.left;
	dy=myDstRect.top-mySrcRect.top;
	/* clip myDstRect */
	Dst = RectToAddress((PixMap *)dstBits,&myDstRect,&dstRowBytes,&dstPixelSize,&dstBitsOffset);

	/*
	This prevents writing outside the destination.
	The cost is that part of the inside will not be written.
	The problem arises because this routine's code can only write whole bytes,
	and the boundary may be in the middle of a byte. So, rather than writing an
	extra fraction of a byte (outside the destination rect) we leave the byte
	alone and fail to update a small portion inside the destination rect.
	*/
	if(dstBitsOffset>0) {
		nudge=(7+dstBitsOffset)/8;
		dstBitsOffset -= nudge*8;
		Dst += nudge;
		myDstRect.left += nudge*8/dstPixelSize;
	}

	/* Copy any clipping of myDstRect over to mySrcRect */
	mySrcRect=myDstRect;
	OffsetRect(&mySrcRect,-dx,-dy);
	/* clip mySrcRect */
	Src=RectToAddress((PixMap *)srcBits,&mySrcRect
		,&srcRowBytes,&srcPixelSize,&srcBitsOffset);

	/* Copy any clipping of mySrcRect back to myDstRect */
	myDstRect=mySrcRect;
	OffsetRect(&myDstRect,dx,dy);
	Dst=RectToAddress((PixMap *)dstBits,&myDstRect
		,&dstRowBytes,&dstPixelSize,&dstBitsOffset);

	if(Src==NULL || Dst==NULL) return 3;
	if(srcPixelSize != dstPixelSize) return 4;
	bytes = mySrcRect.right - mySrcRect.left;	/* number of pixels per line */
	bytes *= srcPixelSize;						/* number of bits per line */
	bytes /= 8;									/* number of bytes per line */
	srcinc = srcRowBytes - bytes;		/* offset in bytes to beginning of next line */
	dstinc = dstRowBytes - bytes;
	lines=mySrcRect.bottom - mySrcRect.top;		/* number of lines */
	switch(srcMode){
	case srcCopy:
		srcCopyQuickly(Src,srcinc,Dst,dstinc,bytes,lines);
		break;
	case addOver:
		if(srcPixelSize!=8)return 5;
		addOverQuickly(Src,srcinc,Dst,dstinc,bytes,lines);
		break;
	case mulOver:
		if(srcPixelSize!=8)return 5;
		mulOverQuickly(Src,srcinc,Dst,dstinc,bytes,lines);
		break;
	default:
		break;
	}
	return 0;
}

	
static void srcCopyQuickly(UPtr xSrc,unsigned short xsrcinc,
	UPtr xDst,unsigned short xdstinc,
	unsigned long bytes,unsigned long lines)
{
	register unsignedPtr Src,Dst;/* alas, THINK C refuses to place these in registers */
	#if THINK_C
		register unsigned long *SrcR,*DstR;
	#endif
	register unsigned short i,j=lines,longs,srcinc=xsrcinc,dstinc=xdstinc;
	short extra,extra8,extra16;
	char mmumode;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	Src.B=xSrc;
	Dst.B=xDst;
	#if THINK_C
		SrcR=Src.L;
		DstR=Dst.L;
	#endif
	longs = bytes/sizeof(long);					/* number of longs per line */
	extra = bytes - longs*sizeof(long);			/* residue */
	extra16=extra8=FALSE;
	if(extra >= sizeof(short)) {
		extra16=TRUE;
		extra -= sizeof(short);
	}
	if(extra >= 1) {
		extra8=TRUE;
		extra--;
	}
	mmumode=true32b;
	if(can32)SwapMMUMode(&mmumode);	/* set 32-bit mode */
	/* now choose the fastest possible loop */
	if(srcinc != 0 || dstinc !=0 || extra16 || extra8) {
		#if !THINK_C
			for(;j>0;j--) {
				for(i=longs;i>0;i--) *Dst.L++ = *Src.L++;
				if(extra16) *Dst.W++ = *Src.W++;
				if(extra8) *Dst.B++ = *Src.B++;
				Src.B += srcinc;
				Dst.B += dstinc;
			}
		#else
			goto rows1;
	row1:		i=longs;
				goto cols1;
			asm {
	col1:		MOVE.L	(SrcR)+,(DstR)+
	cols1:		DBRA	i,@col1
			}
				if(extra16)	asm {MOVE.W	(SrcR)+,(DstR)+};
				if(extra8)	asm {MOVE.B	(SrcR)+,(DstR)+};
			asm {
				ADDA.W	srcinc,SrcR
				ADDA.W	dstinc,DstR
	rows1:		DBRA	j,@row1
			}
		#endif
		if(can32)SwapMMUMode(&mmumode);	/* restore */
		return;
	}
	if(longs*(long)j > 0x10000L) { /* DBRA only uses the lower 16 bits */
		#if !THINK_C
			for(;j>0;j--) {
				for(i=longs;i>0;i--) *Dst.L++ = *Src.L++;
			}
		#else
			goto rows3;
	row3:		i=longs;
				goto cols3;
			asm {
	col3:		MOVE.L	(SrcR)+,(DstR)+
	cols3:		DBRA	i,@col3
			}
			asm {
	rows3:	DBRA	j,@row3
			}
		#endif
		if(can32)SwapMMUMode(&mmumode);	/* restore */
		return;
	}
	else {
		j *= longs;
		#if !THINK_C
			for(;j>0;j--) *Dst.L++ = *Src.L++;
		#else
			goto cols4;
			asm {
	col4:		MOVE.L	(SrcR)+,(DstR)+
	cols4:		DBRA	j,@col4
			}
		#endif
		if(can32)SwapMMUMode(&mmumode);	/* restore */
		return;
	}
}

static void addOverQuickly(UPtr xSrc,unsigned short xsrcinc,
	UPtr xDst,unsigned short xdstinc,
	unsigned long xbytes,unsigned long lines)
{
	register UPtr Src=xSrc,Dst=xDst;
	register unsigned long i,j=lines,bytes=xbytes,srcinc=xsrcinc,dstinc=xdstinc;
	char mmumode;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));	
	mmumode=true32b;
	if(can32)SwapMMUMode(&mmumode);	/* set 32-bit mode */

	/* now choose the fastest possible loop */
	if(srcinc != 0 || dstinc !=0) {
		#if !THINK_C
			for(;j>0;j--) {
				for(i=bytes;i>0;i--) *Dst++ += *Src++;
				Src += srcinc;
				Dst += dstinc;
			}
		#else
			goto rows1;
	row1:		i=bytes;
				goto cols1;
			asm {
	col1:		MOVE.B	(Src)+,D0
				ADD.B	D0,(Dst)+
	cols1:		DBRA	i,@col1
				ADDA.W	srcinc,Src
				ADDA.W	dstinc,Dst
	rows1:	DBRA	j,@row1
			}
		#endif
		if(can32)SwapMMUMode(&mmumode);	/* restore */
		return;
	}
	if(bytes*(long)j > 0x10000L) { /* the DBRA only uses the lower 16 bits */
		#if !THINK_C
			for(;j>0;j--) {
				for(i=bytes;i>0;i--) *Dst++ += *Src++;
			}
		#else
			goto rows3;
	row3:		i=bytes;
				goto cols3;
			asm {
	col3:		MOVE.B	(Src)+,D0
				ADD.B	D0,(Dst)+
	cols3:		DBRA	i,@col3
	rows3:	DBRA	j,@row3
			}
		#endif
		if(can32)SwapMMUMode(&mmumode);	/* restore */
		return;
	}
	else {
		j *= bytes;
		#if !THINK_C
			for(;j>0;j--) *Dst++ += *Src++;
		#else
			goto cols4;
			asm {
	col4:		MOVE.B	(Src)+,D0
				ADD.B	D0,(Dst)+
	cols4:		DBRA	j,@col4
			}
		#endif
		if(can32)SwapMMUMode(&mmumode);	/* restore */
		return;
	}
}

// Multiply two unsigned 8-bit pixels, and divide the product by 128.
static void mulOverQuickly(register UPtr Src,register unsigned short srcinc,
	register UPtr Dst,register unsigned short dstinc,
	unsigned long bytes,unsigned long lines)
{
	register unsigned long i,j;
	char mmumode;
	Boolean can32;
	long value=0,error;

	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));	
	mmumode=true32b;
	if(can32)SwapMMUMode(&mmumode);	/* set 32-bit mode */
	for(j=lines;j>0;j--) {
		for(i=bytes;i>0;i--) {
			*Dst = (unsigned char)((unsigned short)(*Dst)*(unsigned short)(*Src)>>7);
			Src++;
			Dst++;
		}
		Src += srcinc;
		Dst += dstinc;
	}
	if(can32)SwapMMUMode(&mmumode);	/* restore */
}

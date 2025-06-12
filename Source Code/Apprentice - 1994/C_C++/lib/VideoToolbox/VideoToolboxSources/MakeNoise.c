/* MakeNoise.c
© 1989-1992 Denis G. Pelli.
MakeNoise1() fills a PixMap with noise. The noise is made up of black & white cells,
each dx pixels wide by dy pixels high. Random phase is optional.

If your PixMap baseAddr==NULL then the data area will be allocated for you.
In that case you must set frame.bounds and frame.pixelSize. Everything else will be
filled in by MakeNoise1. It will install a handle to the current device's color table.
The values filled into the fields are necessarily somewhat arbitrary. For more
control, fill in the fields yourself, using NewPixMap(), and allocate the data area
yourself before calling MakeNoise1.

I suggest that new programs use MakeNoise1(). The old MakeNoise() is retained
only for the sake of old programs.

MakeNoise() fills an array of PixMaps with noise, by calling MakeNoise1()
repeatedly. However, it first sets the baseAddr to zero, so each data area
will always be allocated, for compatibility with old programs. 
You must set movie[0].bounds and movie[0].pixelSize to the desired values,
the rest of the parameters (and all the rest of the PixMaps) will be set up for you.
If memory runs out before all the PixMap data areas are allocated then
*maxFramesPtr is reduced accordingly.

MakeNoise1 uses the current ForeColor and BackColor to make the noise, so
make sure you've set the port to something appropriate. (And, if baseAddr==NULL,
it shares the color table of the current device, so the device should be 
reasonable too.) Apple's GetGWorld and SetGWorld calls are a convenient way of
getting, changing, and restoring the port and device.

HISTORY:
6/21/89 dgp	changed frameBytes and bufferBytes to long so that this code won't crash
			if the pixelSize is greater than 1 bit.
8/5/89 dgp	wrote and installed randU(), which yields two random bytes per call, instead
			of rand() which yields only one, allowing the noise generation to go twice
			as fast.
3/11/90 dgp	Changed dx and dy from int to double. This allows nonuniform cell size,
			which arises when CopyBits expands by these nonintegral factors. Results
			will be same as before when integral values are supplied.
			Made the cosmetic change of replacing "unsigned long" by "size_t".
3/20/90	dgp	make compatible with MPW C.
3/31/90	dgp	set pmVersion to zero for compatibility with QD32
8/7/90	dgp	Now uses the current ForeColor and BackColor to make the noise.
			For future compatibility, now zeroes the packType, etc. fields.
10/1/90	dgp	Cosmetic changes: replaced two OffsetRect calls by CenterRectInRect call.
11/16/90 dgp Added RANDOM_PHASE.
4/20/91	dgp	Added new MakeNoise1() that accepts a single PixMap, and allocates
			its data area only if baseAddr==NULL, otherwise assuming that it already
			exists. I rewrote MakeNoise() to simply call MakeNoise1() repeatedly.
			MakeNoise() now seems obsolete, but is retained for compatibility with old
			programs.
4/24/91	dgp	Fixed bug introduced into MakeNoise() on 4/20, which resulted in all the
			PixMaps using the same buffer.
8/6/91	dgp	Replaced use of randU() by RandFill(), which is twice as fast.
8/24/91	dgp	Made compatible with THINK C 5.0.
4/17/92	dgp	Removed hidden assumption that frame->bounds.top and frame->bounds.left
			were both zero. If they were negative MakeNoise1 would allocate too 
			little space, and then overwrite memory and crash.
4/27/92	dgp	Initialize all remaining fields of the frame PixMap, and copy the current
			device's color table handle.
10/22/92 dgp Check for error in CopyBits.
*/
#include "VideoToolbox.h"
#include <math.h>

void MakeNoise(double dx,double dy,PixMap movie[],short *maxFramesPtr)
{
	int i,error;
	Boolean randomPhase=1;

	/*
	Always allocate each frame's data area.
	This is an old decision, which seems silly now, but too many programs
	would break if I changed it, so it stays.
	*/
	for (i=0;i<*maxFramesPtr;i++){
		movie[i]=movie[0];
		movie[i].baseAddr=NULL;
		error=MakeNoise1(dx,dy,randomPhase,&movie[i]);
		if(error)break;
	}
	*maxFramesPtr=i;
}

Boolean MakeNoise1(double dx,double dy,Boolean randomPhase,PixMap *frame)
{
	BitMap buffer,bigBuffer;
	Rect srcRect;
	size_t frameBytes,bufferBytes;
	RGBColor blackRGB={0,0,0},whiteRGB={0xffff,0xffff,0xffff},backRGB,foreRGB;
	PixMap **pm;
	OSErr error;
	
	GetBackColor(&backRGB);
	GetForeColor(&foreRGB);
	
	srandU((unsigned int)rand());	/* seed my unsigned random number generator */

	/* allocate buffer BitMap */
	SetRect(&buffer.bounds,0,0,1+(int)ceil((frame->bounds.right-frame->bounds.left)/dx),
		1+(int)ceil((frame->bounds.bottom-frame->bounds.top)/dy));
	buffer.rowBytes=(buffer.bounds.right + 31 & ~31)/8; /* round up to multiple of 32 bits */
	bufferBytes=(size_t) buffer.rowBytes * (size_t) buffer.bounds.bottom;
	buffer.baseAddr=(Ptr) NewPtr(bufferBytes);
	if(buffer.baseAddr == NULL)
		PrintfExit("\007MakeNoise1: sorry, not enough memory for buffer.\n");

	/* allocate bigBuffer BitMap */
	SetRect (&bigBuffer.bounds,0,0,(int)floor(buffer.bounds.right*dx+0.5),
		(int)floor(buffer.bounds.bottom*dy+0.5));
	bigBuffer.rowBytes=(bigBuffer.bounds.right + 31 & ~31)/8; /* round up to multiple of 32 bits */
	bigBuffer.baseAddr=(Ptr) NewPtr((size_t) bigBuffer.rowBytes * (size_t) bigBuffer.bounds.bottom);
	if(bigBuffer.baseAddr == NULL){
		PrintfExit("\007MakeNoise1: sorry, not enough memory for bigBuffer.\n");
	}

	/* if necessary, fill in frame's fields and allocate its data area */
	if(frame->baseAddr == NULL){
		frame->hRes=frame->vRes=0x480000;	// 72 dpi
		frame->pmVersion=frame->packType=frame->packSize=0;
		frame->planeBytes=frame->pmReserved=0;
		frame->cmpCount=1;
		frame->cmpSize=frame->pixelSize;
		frame->pmTable=(**(**GetGDevice()).gdPMap).pmTable;
		frame->rowBytes=((frame->bounds.right-frame->bounds.left)*frame->pixelSize + 31 & ~31)/8; /* round up to multiple of 32 bits */
		frameBytes=(size_t) frame->rowBytes * (frame->bounds.bottom-frame->bounds.top);
		frame->rowBytes |= 0x8000;	/* Mark it as a PixMap */
		frame->baseAddr=(char *) NewPtr(frameBytes);
		if(frame->baseAddr == NULL)return 1;	/* error return */
	}
	if((size_t)frame->baseAddr%4 != 0L){
		printf("Frame buffer address is not long aligned.\007\n");
	}

	/* create random noise */
	/* First create a random bit image, all bits uncorrelated. */
	RandFill(buffer.baseAddr,bufferBytes);
	/* Now expand image so that each random bit becomes a dx by dy rectangle. */
	RGBBackColor(&whiteRGB);
	RGBForeColor(&blackRGB);
	CopyBits(&buffer,&bigBuffer,&buffer.bounds,&bigBuffer.bounds,srcCopy,NULL);
	error=QDError();
	if(error)printf("MakeNoise1:a: CopyBits error %d\n",error);
	/* Finally copy it into the frame, trimming it down to the correct size. */
	RGBBackColor(&backRGB);
	RGBForeColor(&foreRGB);
	srcRect=frame->bounds;
	OffsetRect(&srcRect,-srcRect.left,-srcRect.top);
	if(randomPhase) OffsetRect(&srcRect,nrand(dx),nrand(dy));
	else CenterRectInRect(&srcRect,&bigBuffer.bounds);
	CopyBits(&bigBuffer,(BitMap *)frame,&srcRect,&frame->bounds,srcCopy,NULL);
	error=QDError();
	if(error)printf("MakeNoise1:b: CopyBits error %d\n",error);
	DisposPtr((void *) buffer.baseAddr);
	DisposPtr((void *) bigBuffer.baseAddr);
	return 0;
}



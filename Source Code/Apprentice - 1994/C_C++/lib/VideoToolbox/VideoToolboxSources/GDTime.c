/*
GDTime.c

All these routines measure the timing of some aspect of the function of a video
device. For background, read “Video synch” and run TimeVideo, which reports the
results of running all these routines.

double GDFrameRate(GDHandle device) measures the frame rate of a video device in
Hz. (A NULL “device” argument to this routine, or any of the routines below,
requests use of the System VBL interrupt, which normally runs at 60.15 Hz.) It
times by counting VBL interrupts (discarding any spurious ones, to deal with the
problem described in the next paragraph).

double GDVBLRate(GDHandle device) measures the rate of VBL interrupts generated
by a video device in Hz. According to Apple’s Designing Cards and Drivers book
and other documentation, the video driver and card are supposed to generate one
VBL interrupt per frame. However, many don’t. E.g. Apple’s 4•8 and 8•24 video
cards issue several interrupts per frame. Read the “Video synch” file.

double GDMovieSize(GDHandle device,int quickly) measures what fraction of the
screen you can fill with a real-time movie (a new image on each frame), using
CopyBitsQuickly (if quickly!=0) or CopyBits (if quickly==0) to copy from memory
to video card. At one time CopyBitsQuickly() was much faster than CopyBits(),
but my latest measurements, using GDMovieRate, indicate that there is no longer
any difference in speed. However, CopyBitsQuickly ignores the color tables and
CopyBits uses them. Of course, when you’re showing movies you don’t want to
waste time with color tables, so GDMovieRate() makes a PixMap that shares the
device’s color table. For reasons that I don’t understand even in that case
using CopyBits to copy from and then back to the screen doesn’t always preserve
the original colors.

double GDMovieRate(GDHandle device,int quickly) measures the rate (images/s) at
which you can show a full-screen movie.

error=GDTimeClut(device,GDSetEntries,clutEntries,&s,&frames,&missingFrames,&frameRate);
measures how long it takes to load the clut. It measures in two kinds of units
simultaneously, frames and seconds. You supply the function to be tested, e.g.
SetEntriesQuickly or GDSetEntries. (If gdType==directType GDTimeClut will
automatically substitute GDDirectSetEntries for GDSetEntries.) The second
argument, “clutEntries”, specifies how many clut entries you want to update each
time, or zero for all. GDTimeClut also measures the frame rate independently,
which it returns, after using it to estimate how many frame interrupts were
missed during each clut load. If there’s at least one frame missing or if the
frame count is very small, less than 0.5 per call, then it estimates the frames
directly from the time. You may substitute NULL for any of the pointer-to-double
arguments.

GDFramesPerClutUpdate, GDClutUpdateRate, and GDTimeClutUpdate are gone. Use 
GDTimeClut instead.

NOTES:
It is of interest to time GDSetEntries (and its sibling GDDirectSetEntries) at
both normal (zero) and high (7) processor interrupt priority, because some
drivers are asynchronous when run at low priority, returning immediately and
deferring the actual clut loading until the vbl interrupt occurs, but Apple
specifies that all drivers must be synchronous when run at high priority. You do
this by supplying the new routine GDSetEntriesByTypeHighPriority as an argument
to GDTimeClut.

Similarly, while the problem of multiple interrupts per frame is dealt with
satisfactorily by VBLInstall.c, using the scheme suggested by Raynald Comtois,
it is of technical interest to follow up the report that there are no extra
interrupts if the processor instruction cache is disabled. One theory to account
for this is that perhaps disabling the cache causes the interrupt service
routine to take long enough that the hardware interrupt pulse has terminated
before the routine exits. Thus I would like to write a routine called
GDSetEntriesNoCache, but I don’t know how to disable the cache.

HISTORY:
8/22/92 dgp	wrote ‘em.
8/26/92	dgp	added clutEntries argument
8/28/92	dgp	updated to use new reentrant Timer.c
9/11/92	dgp enhanced GDFrameRate() to discard bogus VBL interrupts and to return
			true frame rate, as suggested by Raynald Comtois. Added GDVBLRate(), 
			which corresponds to the old GDFrameRate().
9/15/92	dgp	GDMovieRate() now asks the video driver what mode we’re in, just in case
			QuickDraw’s been fooled.
9/17/92	dgp	Added second argument to GDMovieRate() to select CopyBitsQuickly vs CopyBits.
10/5/92	dgp	fixed bug in GDMovieRate() that caused crashes or noop when testing other 
			than the main device.
10/9/92	dgp	now actually initialize the linearTable for direct clut.
10/10/92 dgp Squeezed out extra space from rowBytes in GDMovieRate() so as not to show
			garbage. Now use Temporary memory if there isn’t enough memory in the application 
			heap to show a full-screen movie. Show movie for 3 seconds. 
10/13/92 dgp In response to a report from Tom Busey, that frames were going uncounted
			during the clut timing, which seems to be a problem with some video drivers,
			GDFramesPerClutUpdate() now double checks the timing in secs, and if
			it finds a discrepancy, prints a warning to the screen and reports a
			corrected values based on the timing in secs. This should be more reliable.
10/13/92 dgp Fixed error in printf in GDFrameRate().
11/23/92 dgp Set nominalBits equal to pixelSize instead of Log2L(ctsize).
12/30/92 dgp Commented out warning from GDFramesPerClutUpdate().
			 •Check for SetEntries error in the clut timing routines, and return
			 NAN in that case. •Use trial and error to determine clut size.
1/4/92	dgp	GDTimeClutUpdate now returns NAN on GDSetEntries error.
1/6/92	dgp	Fixed computation of linearTable, so CLUT is preserved by GDTimeClutUpdate.
1/11/93	dgp	Enhanced GDMovieRate() to work even when Color QuickDraw is absent, by 
			calling the new routine GDMovieRateNoColorQuickDraw().
			Check returned Ptr from NewTimer() to make sure it’s ok; will be NULL
			if computer only has Standard Time Manager.
1/24/93	dgp	Reduced timing interval from 3 to 1 s for movies.
3/11/93	dgp created GDTimeClut, based on GDTimeClutUpdate and
			GDFramesPerClutUpdate. The enhancements are 1. you
			supply the function to be tested, e.g. SetEntriesQuickly
			or GDSetEntries. 2. It doesn’t print or exit, always
			returning with an informative OSErr. 3. It measures frame
			rate independently, which it returns, and also uses it to
			estimate how many frame interrupts were missed during
			each clut load. 4. If there’s at least one frame missing
			or if the frame count is very small, less than 0.5 per
			call, then it estimates the frames directly from the
			time.
3/15/93	dgp	Fixed portRect clipping error in GDMovieRateNoColorQuickDraw.
4/17/93	dgp	Merged GDFrameRate.c and GDTimeClut.c to produce GDTime.c
4/19/93	dgp	Fixed bug in GDTimeClut that used garbage color table in place of
			linear color table. Now uses GDNewLinearColorTable.
4/25/93	dgp	Changed struct from static to automatic.
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <math.h>
#if THINK_C
	#include <LoMem.h>
#else
	short MBarHeight : 0xBAA;
#endif
double GDMovieRateNoColorQuickDraw(int quickly);
// Original typedef is in VideoToolbox.h
//typedef OSErr (*SetEntriesFunction)(GDHandle device,short start,short count
//	,ColorSpec *aTable);
#define CALLS 30			// fewer for speed, more for accuracy
#define FRAMES 10			// fewer for speed, more for accuracy
#define SHOW_MOVIE_WINDOW 0	// A matter of taste, but I prefer not to show it.
#define SECONDS 0.5			// Movie duration.

OSErr GDTimeClut(GDHandle device,SetEntriesFunction function,short clutEntries
	,double *sPtr,double *framesPtr,double *missingFramesPtr,double *frameRatePtr)
{
	OSErr error;
	short clutSize,i;
	ColorSpec *table,*linearTable=NULL;
	VBLTaskAndA5 vblData;
	long frames;
	Timer *timer;
	double s,missingFrames,frameRate;

	if(sPtr!=NULL)*sPtr=NAN;
	if(framesPtr!=NULL)*framesPtr=NAN;
	if(missingFramesPtr!=NULL)*missingFramesPtr=NAN;
	if(frameRatePtr!=NULL)*frameRatePtr=NAN;
	if(device==NULL || (*device)->gdType==fixedType){
		if(frameRatePtr!=NULL)*frameRatePtr=GDFrameRate(device);
		return 0;
	}
	clutSize=GDClutSize(device);
	if(clutEntries<0 || clutEntries>clutSize)return 1;
	if(clutEntries==0)clutEntries=clutSize;
	if((*device)->gdType==directType){
		if(function==GDSetEntries)function=GDDirectSetEntries;
		table=linearTable=GDNewLinearColorTable(device);
		if(linearTable==NULL)return MemError();
	}else table=((**(**(**device).gdPMap).pmTable)).ctTable;
	vblData.subroutine=NULL;						// setup frame counter
	error=VBLInstall(&vblData,device,CALLS*20);		// setup frame counter
	if(error)return error;
	timer=NewTimer();								// setup timer
	if(timer==NULL)return 1;						// lacks Revised Time Manager.
	vblData.vbl.vblCount=1;							// Enable interrupt service routine
	for(i=-1;i<CALLS;i++) {
		error=(function)(device,0,clutEntries-1,table);
		if(i==-1){
			StartTimer(timer);
			frames=vblData.framesLeft;
		}
		if(error)break;
	}
	frames-=vblData.framesLeft;
	s=StopTimerSecs(timer);
	VBLRemove(&vblData);
	DisposeTimer(timer);
	if(linearTable!=NULL)DisposePtr((Ptr)linearTable);
	if(error)return error;
	
	// Estimate number of missing frames by discrepancy between frames and secs.
	frameRate=GDFrameRate(device);
	missingFrames=s*frameRate-frames;
	
	// Return results
	if(sPtr!=NULL)*sPtr=s/CALLS;
	if(framesPtr!=NULL){
		if(fabs(missingFrames)>1. || frames<CALLS/2) *framesPtr=frameRate*s/CALLS;
		else *framesPtr=frames/(double)CALLS;
	}
	if(missingFramesPtr!=NULL)*missingFramesPtr=missingFrames/CALLS;
	if(frameRatePtr!=NULL)*frameRatePtr=frameRate;
	return 0;
}

double GDFrameRate(GDHandle device)
{
	VBLTaskAndA5 vblData;
	Timer *timer;
	register long frames;
	int error;
	double s;
	
	timer=NewTimer();
	if(timer==NULL)return NAN;			// lacks Revised Time Manager.
	vblData.subroutine=NULL;
	error=VBLInstall(&vblData,device,FRAMES);
	if(error)PrintfExit("GDFrameRate: VBLInstall: error %d\n",error);
	vblData.vbl.vblCount=1;				// Enable interrupt service routine
	frames=vblData.framesDesired-2;
	while(vblData.framesLeft>frames);	// wait for second frame
	StartTimer(timer);
	while(vblData.framesLeft);			// wait for last frame
	s=StopTimerSecs(timer);
	VBLRemove(&vblData);
	DisposeTimer(timer);
	return frames/s;
}

double GDVBLRate(GDHandle device)
{
	VBLTaskAndA5 vblData;
	Timer *timer;
	register long frames;
	int error;
	double s;
	
	timer=NewTimer();
	if(timer==NULL)return NAN;			// lacks Revised Time Manager.
	vblData.subroutine=SimpleVBLSubroutine;
	error=VBLInstall(&vblData,device,FRAMES);
	if(error)PrintfExit("GDVBLRate: VBLInstall: error %d\n",error);
	vblData.vbl.vblCount=1;				// Enable interrupt service routine
	frames=vblData.framesDesired-2;
	while(vblData.framesLeft>frames);	// wait for second frame
	StartTimer(timer);
	while(vblData.framesLeft);			// wait for last frame
	s=StopTimerSecs(timer);
	VBLRemove(&vblData);
	DisposeTimer(timer);
	return frames/s;
}

double GDMovieSize(GDHandle device,int quickly)
{
	return GDMovieRate(device,quickly)/GDFrameRate(device);
}

double GDMovieRate(GDHandle device,int quickly)
{
	Timer *timer;
	register long image;
	long images;
	OSErr error;
	double s=NAN,fractionOfFrame=NAN;
	PixMap **pm;
	unsigned long bytes;
	GDHandle oldDevice;
	WindowPtr window,oldPort;
	Rect r,rLocal;
	Ptr oldBaseAddr;
	short nominalBits,trueBits;
	Handle saveSpace,bufferHandle;
	long osAttr;
	int tempMem;
	
	if(!QD8Exists())return GDMovieRateNoColorQuickDraw(quickly);
	oldDevice=GetGDevice();
	SetGDevice(device);
	pm=NewPixMap();
	SetGDevice(oldDevice);
	if(pm==NULL)goto done0;
	HLock((Handle)pm);
	// The color table is needed for CopyBits(); CopyBitsQuickly doesn't care.
	(**pm).pmTable=(**(**device).gdPMap).pmTable;	// share device's color table
	if(device==GetMainDevice())(**pm).bounds.top+=MBarHeight;
	if(SHOW_MOVIE_WINDOW){
		(**pm).bounds.top+=19;	// Allow room for window title
		InsetRect(&(**pm).bounds,32,32);
	}
	bufferHandle=NULL;
	bytes=(**pm).bounds.right-(**pm).bounds.left;
	bytes*=(**pm).pixelSize;
	bytes=((bytes+31)/32)*4;	// convert bits to bytes, rounding up to multiple of 4
	(**pm).rowBytes &= ~0x3fff;
	(**pm).rowBytes |= bytes;
	Gestalt(gestaltOSAttr,&osAttr);
	tempMem=osAttr & 1L<<gestaltTempMemSupport;
	while(1){
		bytes=(**pm).rowBytes & 0x3fff;
		bytes*=(**pm).bounds.bottom-(**pm).bounds.top;
		if(bytes==0)goto done1;
		saveSpace=NewHandle(2000);	// save some space
		bufferHandle=NewHandle(bytes+1200);	// extra is for drifting
		if(saveSpace!=NULL)DisposeHandle(saveSpace);
		if(bufferHandle==NULL && tempMem)bufferHandle=TempNewHandle(bytes+1200,&error);
		if(bufferHandle!=NULL)break;
		// Halve the window's height before trying again
		(**pm).bounds.bottom=(**pm).bounds.top+((**pm).bounds.bottom-(**pm).bounds.top)/2;
	}
	HLock(bufferHandle);
	(**pm).baseAddr=*bufferHandle;
	GetPort(&oldPort);
	r=(**pm).bounds;
	window=NewCWindow(NULL,&r,"\pmovie",0,0,(WindowPtr)-1,0,0);	// don't show it yet
	if(window==NULL)goto done2;
	timer=NewTimer();
	if(timer==NULL)goto done2;		// lacks Revised Time Manager.
	SetPort(window);
	HLock((Handle)((CWindowPtr)window)->portPixMap);
	SetGDevice(device);
	rLocal=r;
	GlobalToLocalRect(&rLocal);
	StartTimer(timer);
	if(quickly)CopyBitsQuickly((BitMap *)*((CWindowPtr)window)->portPixMap,(BitMap *)*pm
		,&rLocal,&r,srcCopy,NULL);	// copy screen to memory
	else{
		CopyBits((BitMap *)*((CWindowPtr)window)->portPixMap,(BitMap *)*pm
		,&rLocal,&r,srcCopy,NULL);	// copy screen to memory
		if(error=QDError()){
			printf("GDMovieRate: CopyBits generated QuickDraw error %d\n",error);
			goto done3;
		}
	}
	s=StopTimerSecs(timer);	// rough estimate of time for one image
	if(SHOW_MOVIE_WINDOW || !quickly)ShowWindow(window);// CopyBits won't copy to a hidden window
	StartTimer(timer);
	oldBaseAddr=(**pm).baseAddr;
	images=ceil(SECONDS/s);
	for(image=images;image>0;image--){
		if(image==1)(**pm).baseAddr=oldBaseAddr;
		else (**pm).baseAddr+=4;	// Drift image to prove it's a movie
		// We drift by multiples of 4 bytes because long-aligned copying is faster.
		if(quickly)CopyBitsQuickly((BitMap *)*pm,(BitMap *)*((CWindowPtr)window)->portPixMap
			,&r,&rLocal,srcCopy,NULL);	// copy memory to screen
		else CopyBits((BitMap *)*pm,(BitMap *)*((CWindowPtr)window)->portPixMap
			,&r,&rLocal,srcCopy,NULL);	// copy memory to screen
	}
	s=StopTimerSecs(timer);
	fractionOfFrame=(long)(r.bottom-r.top)*(long)(r.right-r.left);
	r=(**(**device).gdPMap).bounds;
	fractionOfFrame/=(long)(r.bottom-r.top)*(long)(r.right-r.left);
	DisposeTimer(timer);
done3:
	SetPort(oldPort);
	DisposeWindow(window);
done2:
	DisposHandle(bufferHandle);
done1:
	(**pm).pmTable=NULL;
	DisposePixMap(pm);
done0:
	SetGDevice(oldDevice);
	return images*fractionOfFrame/s;
}

double GDMovieRateNoColorQuickDraw(int quickly)
{
	Timer *timer;
	register long image;
	long images;
	OSErr error=0;
	double s=NAN,fractionOfFrame=NAN;
	BitMap bitmap;
	unsigned long bytes;
	Rect r;
	Ptr oldBaseAddr;
	Handle saveSpace,bufferHandle=NULL;
	long osAttr;
	int tempMem;
	char string[100];
	GrafPort portRec,*port=&portRec,*oldPort;
	
	GetPort(&oldPort);
	OpenPort(port);
	SetPort(port);
	bitmap=port->portBits;
	bytes=bitmap.bounds.right-bitmap.bounds.left;
	bitmap.rowBytes=((bytes+31)/32)*4;	// convert bits to bytes, round up to mult of 4
	Gestalt(gestaltOSAttr,&osAttr);
	tempMem=osAttr & 1L<<gestaltTempMemSupport;
	while(1){
		bytes=bitmap.rowBytes & 0x3fff;
		bytes*=bitmap.bounds.bottom-bitmap.bounds.top;
		if(bytes==0)goto done;
		saveSpace=NewHandle(2000);	// save some space
		bufferHandle=NewHandle(bytes+600);	// extra is for drifting
		if(saveSpace!=NULL)DisposeHandle(saveSpace);
		if(bufferHandle==NULL && tempMem)bufferHandle=TempNewHandle(bytes+600,&error);
		if(bufferHandle!=NULL)break;
		// Halve the window's height before trying again
		bitmap.bounds.bottom=bitmap.bounds.top+(bitmap.bounds.bottom-bitmap.bounds.top)/2;
	}
	HLock(bufferHandle);
	bitmap.baseAddr=*bufferHandle;
	timer=NewTimer();
	if(timer==NULL)goto done;	// lacks Revised Time Manager.
	r=bitmap.bounds;
	StartTimer(timer);
	if(quickly)CopyBitsQuickly(&port->portBits,&bitmap
		,&r,&r,srcCopy,NULL);	// copy screen to memory
	else CopyBits(&port->portBits,&bitmap
		,&r,&r,srcCopy,NULL);	// copy screen to memory
	s=StopTimerSecs(timer);		// approximate time for one image
	if(!quickly && (error=QDError())){
		printf("GDMovieRate: CopyBits generated QuickDraw error %d\n",error);
		DisposeTimer(timer);
		goto done;
	}
	oldBaseAddr=bitmap.baseAddr;
	images=ceil(SECONDS/s);	// Let's time for this many seconds.
	StartTimer(timer);
	for(image=images;image>0;image--){
		if(image==1)bitmap.baseAddr=oldBaseAddr;
		else bitmap.baseAddr+=2;	// Drift the image, to prove it's a movie
		if(quickly)CopyBitsQuickly(&bitmap,&port->portBits
			,&r,&r,srcCopy,NULL);	// copy memory to screen
		else CopyBits(&bitmap,&port->portBits
			,&r,&r,srcCopy,NULL);	// copy memory to screen
	}
	s=StopTimerSecs(timer);
	DisposeTimer(timer);
	if(!quickly && (error=QDError())){
		printf("GDMovieRate: CopyBits generated QuickDraw error %d\n",error);
		DisposeTimer(timer);
		goto done;
	}
	fractionOfFrame=(long)(r.bottom-r.top)*(long)(r.right-r.left);
	r=port->portBits.bounds;
	fractionOfFrame/=(long)(r.bottom-r.top)*(long)(r.right-r.left);
done:
	SetPort(oldPort);
	ClosePort(port);
	if(bufferHandle==NULL){
		printf("GDMovieRate: Not enough memory!\n");
		return NAN;
	}
	DisposeHandle(bufferHandle);
	return images*fractionOfFrame/s;
}

double TickRate(void)
{
	Timer *timer;
	double s;
	long t;
	
	timer=NewTimer();
	if(timer==NULL)return NAN;			// lacks Revised Time Manager.
	Delay(1,&t);
	StartTimer(timer);
	Delay(FRAMES,&t);
	s=StopTimerSecs(timer);
	DisposeTimer(timer);
	return FRAMES/s;
}


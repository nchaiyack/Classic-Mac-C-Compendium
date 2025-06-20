/*
NoiseVBL.c

There are two ways to synchronize your program to a video display. Apple
recommends using the vertical blanking level interrupt (VBL). That approach is
illustrated here. The other approach, which I normally prefer, is to use a side
effect of loading the video cards clut: most video drivers don't return until
the vertical blanking interval. The interrupt approach is more or less
guaranteed to work on all video cards, since Apple more or less requires that it
be supported by all video drivers. Its only disadvantage is that 
interrupts must be enabled. I have the impression--though I haven't collected
hard data--that interrupts steal a lot of time, depending on how busy AppleTalk,
the disk, SCSI, keyboard, mouse, and even miscellaneous INITs that are driven by
VBL interrupts. So when I really want to crank, to display movies, I prefer to
use SetPriority(7) to temporarily suspend all interrupts. In that case the
video-driver side effect approach is your only choice. However, be warned that
some video drivers don't wait, and others take more than a frame to load the
clut, so do some careful timing before you rely on it, i.e. run TimeVideo.

This program shows a noise movie, a dynamic random checkerboard, at 66.7 Hz! The
key routine, which shows each frame, is in the file CopyBitsQuickly.c For best
results you should allocate lots of memory to this program, e.g. 3MB. It
pre-computes as many noise frames as space allows. After showing each frame once
it cycles through them again. Thus to get a true impression of white noise you
need to compute lots of noise frames, e.g. 100. This may require several
megabytes, depending what your screen's pixel depth is set to.

HISTORY:
11/23/88 Denis Pelli wrote it.

4/20/91 I updated this. It now has essentially no hardware dependencies. It does
require that the video be in a slot, since it uses slot interrupts. A basic
problem with the scheme is that interrupts seem to be shut out while the
interrupt service routines are running. As a result the timing information is
inaccurate since many 1 ms interrupts are lost during the call to
CopyBitsQuickly(). I think the only solution to this would be to put the time
consuming call to CopyBitsQuickly back in the main program, waiting for the
interrupt service routine to give it permission to start each new frame.

4/15/92 Removed all bugs and updated to work with THINK C 5. Now properly set A5
in the interrupt service routines, using the method suggested in Tech Note 180.
I removed the old attempt to disable AppleTalk, which now seems to hang up when
it attempts to reenable AppleTalk. I also removed the clut manipulations.

4/16/92	Put the noise in its own window. Run essentially continuously until
user quits.

8/19/92 Replaced obsolete TimeIt.c by new Timer.c. Timing seems to be fine now.

8/22/92 dgp Moved most of the interrupt service routine code to 
VBLInterruptServiceRoutine.c. Just for fun, I made the program use an alternate monitor,
if available. Fixed a small bug that prevented display of noise on an alternate
monitor if it had a different pixelSize than the main screen.

9/15/92 dgp Updated to use new Timer.c.

2/18/93	dgp	Added fpu test.
2/20/93	dgp	Call Require(), increased stack space.
7/7/93	dgp replaced call to CopyBitsQuickly by CopyBits, for compatibility with
Radius PowerView.
3/19/94	dgp updated the call to Shuffle(), adding new required argument.
*/

#include <VideoToolbox.h>
#include <assert.h>
#if THINK_C
	#include <console.h>
#endif

/* The user may wish to adjust these. WIDTH should be a multiple of 4. */
#define MAXFRAMES	200		// number of frames in movie
#define WIDTH	256			/* width of displayed noise movie, in pixels */
#define HEIGHT	256			/* height of displayed noise movie, in pixels */
#define RANDOMPHASE 1		/* randomly shift each movie frame */

/* This is just for reference. The original is in VideoToolbox.h
struct VBLTaskAndA5 {
    volatile VBLTask vbl;
    long ourA5;
    void (*subroutine)(struct VBLTaskAndA5 *vblData);
    GDHandle device;
    long slot;
    volatile long newFrame;				// Boolean
    volatile long framesLeft;			// count down to zero
    long framesDesired;
    void *ptr;							// use this for whatever you want
};
typedef struct VBLTaskAndA5 VBLTaskAndA5;
*/

void main(void);

void main()
{
	int i,j,error;
	int dx=32,dy=32,dt=1,duration;
	long frames;
	unsigned long seed;
	double s;
	static char string[64];
	Rect r;
	short noiseI=0,noiseN=MAXFRAMES;
	static PixMap noiseImage[MAXFRAMES];
	short noiseSequence[MAXFRAMES]; /* shuffled sequence */
	VBLTaskAndA5 noiseVBL;
	WindowPtr window,oldPort;
	EventRecord theEvent;
	int frameDone;
	Timer *timer;
	
	StackGrow(10000);
	assert(StackSpace()>4000);
	Require(gestalt8BitQD);
	GetDateTime(&seed);
	srand(seed);
	/* INITIALIZE QuickDraw */
	#if THINK_C
		console_options.nrows = 3;
		printf("\n");
	#else
		InitGraf((Ptr) &thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif

	// pick a screen
	noiseVBL.device=GetScreenDevice(1);
	if(noiseVBL.device==NULL)noiseVBL.device=GetScreenDevice(0);

	printf("Enter width of check in pixels (%d)?",dx);
	gets(string);
	sscanf(string,"%d",&dx);
	printf("%d\n",dx);
	printf("Enter height of check in pixels (%d)?",dy);
	gets(string);
	sscanf(string,"%d",&dy);
	printf("%d\n",dy);
	printf("Enter duration of check in frames (%d)?",dt);
	gets(string);
	sscanf(string,"%d",&dt);
	printf("%d\n",dt);

	GetPort(&oldPort);
	SetRect(&r,0,0,WIDTH,WIDTH);
	CenterRectInRect(&r,&(*noiseVBL.device)->gdRect);
	OffsetRect(&r,-(r.left%32),0);
	window=NewCWindow(NULL,&r,"\pComputing noise ...",1,noGrowDocProc,(WindowPtr) -1L,0,0L);
	SetPort(window);
	for(i=0;i<MAXFRAMES;i++){
		noiseImage[i].pixelSize=(**(**GetWindowDevice(window)).gdPMap).pixelSize;
		noiseImage[i].bounds=window->portRect;
		noiseImage[i].baseAddr=NULL;
		error=MakeNoise1(dx,dy,RANDOMPHASE,&noiseImage[i]);
		if(error)break;
	}
	noiseN=i;
	SetPort(oldPort);
	printf("Using %d different frames of noise.\n",noiseN);
	for(i=0;i<noiseN;i++)noiseSequence[i]=i; /* initialize sequence */
	FlushEvents(-1L,0);
	printf("Hit any key to quit.\n");
	SetWTitle(window,"\pNoiseVBL");
	timer=NewTimer();
	do{
		SelectWindow(window);
		Shuffle(noiseSequence,noiseN,sizeof(*noiseSequence));	/* shuffle the images */
		noiseI=0;
		noiseVBL.subroutine=NULL;	// use default
		error=VBLInstall(&noiseVBL,noiseVBL.device,MAXFRAMES);
		if(error)PrintfExit("VBLInstall: error %d\n",error);
		noiseVBL.vbl.vblCount=1;	// Enable interrupt service routine
		while(!noiseVBL.newFrame);	// wait for first frame
		noiseVBL.newFrame=0;
		while(!noiseVBL.newFrame);	// wait for second frame
		StartTimer(timer);
		frames=noiseVBL.framesLeft;
		while(noiseVBL.framesLeft>1){
			if(noiseVBL.newFrame){
				noiseVBL.newFrame=0;
				CopyBits((BitMap *)&(noiseImage[noiseSequence[noiseI]])
					,(BitMap *)*((CGrafPtr)window)->portPixMap
					,&noiseImage[0].bounds,&window->portRect,srcCopy,NULL);
				if(noiseVBL.framesLeft%dt==0){
					noiseI++;
					if(noiseI>=noiseN) {
						Shuffle(noiseSequence,noiseN,sizeof(*noiseSequence));
						noiseI=0;
					}
				}
			}
		}
		while(!noiseVBL.newFrame);	// wait for last frame
		s=StopTimer(timer)/1000000.;
		frames-=noiseVBL.framesLeft;
		VBLRemove(&noiseVBL);
		printf("%.1f Hz \r",frames/s);
	}while(!GetNextEvent(keyDown+keyUp+mouseDown,&theEvent));
	FlushEvents(-1L,0);
	for (i=0;i<noiseN;i++) DisposPtr((Ptr) noiseImage[i].baseAddr);
	DisposeWindow(window);
	DisposeTimer(timer);
	abort();
}

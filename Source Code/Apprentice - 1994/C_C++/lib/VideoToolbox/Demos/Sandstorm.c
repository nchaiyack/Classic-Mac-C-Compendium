/*
Sandstorm.c
Copyright � 1989-1993 Denis Pelli
This program shows a noise movie, a dynamic random checkerboard, at 66.7 Hz!
The key routine, which shows each frame, is the VideoToolbox routine CopyBitsQuickly().

To get a true impression of white noise you need to compute lots of noise
frames, so give it as much memory as you can, e.g. 3 MB. (Use the Finder Get
Info command.) Sandstorm fills all the memory you give it with pre-computed
noise, which does take some time. Each displayed frame is taken from a random
offset within that noise.

Note: the amount of data that has to be transfered by CopyBitsQuickly() in each
frame is proportional to window area and the bits/pixel of the display, as set by
Control Panel Monitors. At 8 bits/pixel the maximum window that can be updated
at 66 Hz is about 192x192. At 1 bit/pixel we can do at least 480x640.

Sandstorm--not CopyBitsQuickly--calls WaitNextEvent() before each
frame, in order to be responsive to the mouse, and this results in occasional
pauses as the Mac takes time out to do various chores.


HISTORY:
11/23/88 dgp derived from truenoiseDemo2.c
4/7/89 dgp 	I removed nearly all the hardware dependencies. The only remaining one
			is the call to NewFieldTFB(). I don't know how to get rid of that, short of
			writing a VBL task.
9/29/89 dgp I've nearly updated this to THINK C 4.0, but the call to TickCount, 
			refers to the old profiler, and I don't have time to figure this out now.
10/28/89 dgp I replaced the call to NewFieldTFB() by a call to GDSetEntries() which
			is a device-independent way to wait for the end of frame.
7/19/90 dgp Unfortunately the built-in video on the Mac IIci is buggy and doesn't
			support the setEntries call.
10/17/90 dgp Yay. Finally removed last bugs! Replaced SlotToScreenDevice() by
			AddressToScreenDevice() in order to make it compatible with built-in video
			on Mac IIci, IIsi, and LC. Removed the setEntries call for compatibility
			with Mac IIci built-in video.(Apple has acknowledged bug, but bug is
			still present in System 6.07.) Fixed bug whereby resizing window on other
			than the main screen left window blank until it was dragged again. 
			Removed unused variables. Added zoom box.
10/18/90 dgp Tidied up. Now zooms onto screen with largest intersection, and zooms back
			to original size. Uses as much memory as user allocates to program. 
			Window now always confines itself to a single screen, but can be dragged
			anywhere.
10/20/90 dgp Copied code from VideoTest to close window and quit on Command-W or
			Command-. Prevented window title bar from being hidden behind the menu
			bar.
7/22/91	dgp	Made compatible with MPW C. This required changing INT_MAX to SHRT_MAX.
			However, performance is slow, since CopyBitsQuickly.c doesn't generate
			hand-tuned assembly code when using MPW C.
8/6/91	dgp	Replaced randU() by RandFill(), which runs twice as fast.
8/24/91	dgp	Made compatible with THINK C 5.0.
7/20/92	dgp	tidied up the comments slightly. Confirmed compatibility with 32-bit
			addressing.
8/27/92	dgp	replace SysEnvirons() by Gestalt()
1/13/93 dgp	changed defaults to maximize speed.
9/3/93	dgp	added call to ShieldCursor() for compatibility with nonstandard
			video devices, e.g. Radius PowerView.
*/
#include "VideoToolbox.h"
#if THINK_C
	#include <LoMem.h>		// MBarHeight
#else
	short MBarHeight : 0xBAA;
#endif
void main(void);
Boolean ZoomedOut(WindowPtr w);
#define RAISE_PRIORITY	0	/* Optional. Contrary to Apple's rules,	*/
							/* but speeds up the display. */
#define SCROLL_BAR 15		/* Standard width of scroll bar */
#define WAIT_NEXT_EVENT 0

EventRecord theEvent;

void Sandstorm(void);

void main(void)
{
	Require(gestalt8BitQD);
	Sandstorm();
}
void Sandstorm()
{
	register size_t ui;
	register short *bufferPtr;
	int i;
	static char string[40];
	static PixMap sand;
	Rect r,unzoomedRect,gdRect;
	GDHandle device;
	WindowPtr window,aWindow,oldPort;
	PixMap **myPixMapHandle;
	static BitMap buffer;
	long bufferBytes;
	size_t sandBytes;
	Boolean done=0,update;
	long reSize;
	int windowEvent=inDesk;
	int error;
	Boolean zoomedOut;
	WStateData *wStateData;
	RgnHandle ignoreRgn;
	int pixelSize;
	long value;
	Point pt;
	
	/* INITIALIZE QuickDraw */
	MaxApplZone();						/* Expand heap to the limit. */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitCursor();
	GetPort(&oldPort);
	device=GetMainDevice();

	/* open a window on the device */
	ignoreRgn=NewRgn();
	SetRect(&r,0,0,192+SCROLL_BAR,192+SCROLL_BAR);
	CenterRectInRect(&r,&(*device)->gdRect);
	window=NewCWindow(NULL
		,&r,CtoPstr("Now computing noise ..."),TRUE,zoomDocProc,(WindowPtr) -1L,TRUE,0L);
	myPixMapHandle = ((CGrafPtr)window)->portPixMap;
	MoveHHi((Handle) myPixMapHandle);
	HLock((Handle) myPixMapHandle);

	/* Compute noise */
	/* allocate buffer BitMap */
	buffer.rowBytes=1024;
	MaxApplZone();
	bufferBytes = FreeMem();
	bufferBytes-=100000L;	/* Grab most of available space. */
	bufferBytes -= bufferBytes%buffer.rowBytes;
	buffer.baseAddr=NULL;
	for(;bufferBytes>0;bufferBytes -= buffer.rowBytes){
		buffer.baseAddr = NewPtr(bufferBytes);
		if (buffer.baseAddr!=NULL)break;
	}
	if(buffer.baseAddr==NULL)PrintfExit("\007Sorry, not enough memory for buffers.\n");
	SetRect(&buffer.bounds,0,0,buffer.rowBytes*8L,bufferBytes/buffer.rowBytes);

	/* Fill buffer with noise. */
	bufferPtr = (short *) buffer.baseAddr;
	RandFill(buffer.baseAddr,bufferBytes);

	/* Show noise */
	SetPort(window);
	reSize=0;
	while (1) {
		wStateData=(*((WStateData **)((WindowPeek)window)->dataHandle));
		unzoomedRect=wStateData->userState;
		zoomedOut=ZoomedOut(window);
		if(reSize){
			SizeWindow(window,LoWord(reSize),HiWord(reSize),TRUE);
			reSize=0L;
		}
		device=GetWindowDevice(window);
		if(device==NULL){
			done=1;
			break;
		}
		r=window->portRect;
		LocalToGlobalRect(&r);
		gdRect=(*device)->gdRect;
		/* leave room for window's title bar */
		gdRect.top+=r.top-1-(*((WindowPeek)window)->strucRgn)->rgnBBox.top;
		/* leave room for menu bar */
		if(device==GetMainDevice())gdRect.top+=MBarHeight;
		SectRect(&gdRect,&r,&r);
		pixelSize=(**(**device).gdPMap).pixelSize;
		i=r.left-(*device)->gdRect.left;
		i=(i*pixelSize+16 & ~31)/pixelSize-i;	/* move to nearest 32-bit boundary */
		OffsetRect(&r,i,0);						/* Assumes window is >16 bits wide */
		SectRect(&gdRect,&r,&r);
		MoveWindow(window,r.left,r.top,TRUE);
		OffsetRect(&r,-r.left,-r.top);
		sand = **(*device)->gdPMap;	/* Initialize PixMap fields from screen */
		sand.bounds=r;
		sand.bounds.right  -=SCROLL_BAR;	/* Leave room for the scroll bars */
		sand.bounds.bottom -=SCROLL_BAR;
		sand.bounds.right=(sand.bounds.right*sand.pixelSize+14 & ~31)/sand.pixelSize;
			/* round to multiple of 32 bits */
		if(sand.bounds.right<32)sand.bounds.right=32;
		sand.rowBytes = (sand.bounds.right*sand.pixelSize + 31 & ~31)/8;
			/* round up to multiple of 32 bits (redundant) */
		sandBytes = (size_t) sand.rowBytes * (size_t) sand.bounds.bottom;
		if(bufferBytes/2 < sandBytes)
		{
			/* restrict height by amount of noise available */
			sand.bounds.bottom=bufferBytes/sand.rowBytes/2;
			sandBytes = (size_t) sand.rowBytes * (size_t) sand.bounds.bottom;
		}	
		sand.rowBytes |= 0x8000;	/* Mark it as a PixMap */
		sand.pmVersion=0;
		sand.packType=0;
		sand.packSize=0;
		sand.planeBytes=0;
		sand.pmReserved=0;
		SizeWindow(window
			,sand.bounds.right+SCROLL_BAR,sand.bounds.bottom+SCROLL_BAR,TRUE);
		EraseRect(&window->portRect);
		DrawGrowIcon(window);
		if(zoomedOut && windowEvent!=inGrow && windowEvent!=inDrag){
			wStateData=(*((WStateData **)((WindowPeek)window)->dataHandle));
			r=window->portRect;
			LocalToGlobalRect(&r);
			r.left-=1;
			r.right-=2;
			wStateData->stdState=r;		
			wStateData->userState=unzoomedRect;
		}
		sprintf(string,"%dx%dx%d bits"
			,sand.bounds.right,sand.bounds.bottom,sand.pixelSize);
		SetWTitle(window,CtoPstr(string));
		FlushEvents(everyEvent,0);
		DiffRgn(GetGrayRgn(),((WindowPeek)window)->strucRgn,ignoreRgn);
		while (1) {
			update=0;			/* don't update window frame unless necessary */
			#if RAISE_PRIORITY
				SetPriority(2);	/* Get more speed by suppressing keyboard, mouse, and ticks. */
			#endif
			sand.baseAddr = buffer.baseAddr;
			/* choose grain that won't overflow short argument to nrand() */
			i=(bufferBytes/SHRT_MAX+3)&~3;	/* round up to multiple of 4 */
			sand.baseAddr += nrand((bufferBytes-sandBytes)/i)*(long)i;
			if(((WindowPeek)window)->hilited){
				CopyBitsQuickly((BitMap *)&sand,(BitMap *)*myPixMapHandle
					,&sand.bounds,&sand.bounds,srcCopy,NULL);
				// Notify nonstandard video devices that pixels have changed.
				pt.h=-(*myPixMapHandle)->bounds.left;
				pt.v=-(*myPixMapHandle)->bounds.top;
				ShieldCursor(&sand.bounds,pt);
				ShowCursor();
			}
			#if RAISE_PRIORITY
				SetPriority(0);	/* Revive keyboard and mouse. */
			#endif
			#if WAIT_NEXT_EVENT
   			if(WaitNextEvent(mDownMask+keyDownMask,&theEvent,0,ignoreRgn)){
   			#else
   			if(GetNextEvent(mDownMask+keyDownMask,&theEvent)){
   			#endif
				switch(theEvent.what){
				case keyDown:
					if(theEvent.modifiers & cmdKey) switch(theEvent.message & charCodeMask) {
						case '.':
						case 'w':
						done=1;
					}
					break;
				case mouseDown:
					update=1;
					windowEvent=FindWindow(theEvent.where,&aWindow);
					if(aWindow==window){
						switch(windowEvent){
						case inDrag:
							DragWindow(window,theEvent.where,&qd.screenBits.bounds);
							break;
						case inGoAway:
							done=TrackGoAway(window,theEvent.where);
							break;
						case inGrow:
							r.top=r.left=32+SCROLL_BAR;	/* minimum size */
							r.bottom=r.right=1024+SCROLL_BAR;	/* max size */
							reSize = GrowWindow(window,theEvent.where,&r);
							break;
						case inZoomIn:
						case inZoomOut:
							Zoom(window,windowEvent);
							break;
						case inContent:
							break;
						default:
							update=0;
							break;
						}
						break;
					}
					break;
				}
			}			
			if(update || done)break;
		}
		if(done)break;
	}
	SetPort(oldPort);
	DisposeRgn(ignoreRgn);
	DisposPtr(buffer.baseAddr);
	DisposeWindow(window);
}

Boolean ZoomedOut(WindowPtr w)
/* Determine whether window is "standard" size, i.e. zoomed out. */
{
	Point pt;
	Rect r,myRect;
	Boolean zoomedOut;
	
	myRect=(*((WStateData **)((WindowPeek)w)->dataHandle))->stdState;
	pt.h=0;
	pt.v=0;
	LocalToGlobal(&pt);
	r = w->portRect;
	OffsetRect(&r,pt.h,pt.v);
	zoomedOut=r.top==myRect.top
		&& r.left==myRect.left
		&& r.bottom==myRect.bottom
		&& r.right==myRect.right;
	return zoomedOut;
}
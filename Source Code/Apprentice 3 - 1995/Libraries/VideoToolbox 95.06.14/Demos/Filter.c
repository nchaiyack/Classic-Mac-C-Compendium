/* Filter.c
� 1989-1995 Denis G. Pelli 
Display one letter and low-pass filter it.
Also display a gray wedge, to confirm that lookup table has been correctly loaded.

NOTE:
Calling ShieldCursor() has two desirable effects, one direct, one indirect.
Firstly, it keeps the cursor from messing up the grating that we're drawing.
Secondly, ShieldCursor() is the trap that non-standard QuickDraw devices patch in order
to find out what parts of the screen have been updated. Thus the call to ShieldCursor()
will make the grating show up on a Radius PowerView, which drives a video monitor via
SCSI, and needs to know when to copy new info from the memory buffer to the
screen device. My understanding is that all QuickDraw drawing operations automatically
call ShieldCursor() & ShowCursor(), so this is only relevant when we draw bypassing
QuickDraw, i.e. by calling SetPixelsQuickly or CopyBitsQuickly. CopyWindows() calls
ShieldCursor and ShowCursor for you.

HISTORY:
3/31/89 dgp Wrote it, based on simple program by Preeti Verghese & Mike Schechter,
			which just displayed a character.
4/25/89 dgp added code to linearize the clut. Locked the window's pixmap.
10/9/89 dgp	Updated it to use SetLuminances, etc.
7/10/90 dgp Changed the declaration of window to be a CWindowPtr.
			Added �done� message.
10/11/90 dgp Added fpu test.
10/12/90 dgp Changed LuminanceRecord.h to LuminanceRecord1.h
2/27/91	dgp	 Tidied up.
8/24/91	dgp	Made compatible with THINK C 5.0.
2/15/92	dgp	Incorporated Larry Cormack's suggestion of shrinking the console
			so as not to obscure the main window.
7/20/92	dgp	added printf of done message
8/27/92	dgp	replace SysEnvirons() by Gestalt()
1/4/93	dgp	Don't call SetLuminances if device is of fixedType.
2/7/93	dgp	Updated to use SetPixelsQuickly. Tidied up the grayscale wedge.
			Recompiled with the new ConvolveX.c, which has better rounding.
2/8/93	dgp	Recompiled with the new ConvolveX.c, overflow bug now fixed.
7/7/93	dgp	Made rect a tad bigger to allow for slant of italic.
6/23/94	dgp	Use Choose().
4/11/95 dgp Save and restore the display depth and color mode.
6/14/95 dgp added calls to ShieldCursor().
*/
#include "VideoToolbox.h"
#include "Luminance.h"
#include <Fonts.h>
#if UNIVERSAL_HEADERS
	#include <LowMem.h>
#else
	#define LMGetMBarHeight() (* (short *) 0x0BAA)
	#define LMSetMBarHeight(MBarHeightValue) ((* (short *) 0x0BAA) = (MBarHeightValue))
#endif
#if THINK_C
	#include <console.h>
#endif
#if __MWERKS__
	#include <SIOUX.h>
#endif

#define SIZE 127		/* point size of text to be filtered */
#define DIM 64			/* size of point spread function, sigma=DIM/4 */

void Filter(void);

void main(void)
{
	StackGrow(1000+sizeof(luminanceRecord)+512*sizeof(long));
	Require(gestalt8BitQD);
	Filter();
}
void Filter(void)
{
	EventRecord event;
	register short i;
	int j,clutSize,dim=DIM,error;
	short textSize=SIZE,FontNum;
	unsigned char *s;
	char string[64];
	Rect r,dstRect,wedge;
	CWindowPtr window,oldPort;
	GDHandle device,oldDevice;
	double f[DIM],contrast,a,c,sigma;
	luminanceRecord LR,*LP;
	unsigned long row[512];
	Boolean attenuator,oldIsColor;
	RGBColor blackRGB={0,0,0},whiteRGB={0xffff,0xffff,0xffff};
	Point pt;

	assert(StackSpace()>5000);
	/* Gaussian point spread function, normalized to unit integral. */
	sigma=dim/4.0;
	c=0.0;
	for(i=0;i<dim;i++) {
		a=(i-(dim-1)/2.0)/sigma;
		f[i]=exp(-a*a);
		c+=f[i];
	}
	for(i=0;i<dim;i++) f[i] /= c;

	/* INITIALIZE QuickDraw */
	#if (THINK_C || THINK_CPLUS)
		console_options.nrows = 5;
		console_options.left=32;
	#elif __MWERKS__
		SIOUXSettings.toppixel=LMGetMBarHeight()+19;	// allow for menu bar and title bar
		SIOUXSettings.leftpixel=1;
		SIOUXSettings.rows=5;
		SIOUXSettings.autocloseonquit=0;
		SIOUXSettings.showstatusline=0;
		SIOUXSettings.asktosaveonclose=0;
		printf("\n");
	#elif
		InitGraf(&qd.thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
	printf("\n");
	GetGWorld(&oldPort,&oldDevice);

	printf("Welcome to Filter.\n");
	/* Find device corresponding to the experimental screen. */
	for(i=8;i>=0;i--){
		device = GetScreenDevice(i);
		if(device!=NULL) break;
	}
	do{
		if(GetScreenDevice(1)!=NULL)i=ChooseScreen(i,"Which screen?");
		else i=0;
		device=GetScreenDevice(i);
	}while(device==NULL);

	oldIsColor=TestDeviceAttribute(device,gdDevType);
	clutSize=GDClutSize(device);
	printf("%d colors.\n",clutSize);

	/* Use results of last screen calibration to do gamma correction */
	#include "LuminanceRecord1.h"
	LP=&LR;
	attenuator=Choose(0,"Have you installed an ISR Video Attenuator on that monitor?\n"
		,noYes,2);
	window = GDOpenWindow(device);
	if(!attenuator){
		LP->r=0.0;
		LP->g=1.0;
		LP->b=0.0;
	}
	// if not already in color mode, switch to color mode
	if(!oldIsColor)error=SetDepth(device,(**(**device).gdPMap).pixelSize,1<<gdDevType,1);
	if((*device)->gdType!=fixedType) SetLuminances(device,&LR,0,clutSize-1,0.0,LP->LMax);

	contrast = 1.0;
	if(0){
		printf("Contrast? (%f) ",contrast);
		gets(string);
		sscanf((char *)string,"%lf",&contrast);
		printf("=%f\n",contrast);
	}

	/* Display text */
	SetPort((WindowPtr)window);
	BringToFront((WindowPtr)window);
	PmForeColor(0);					/* black in our current clut */
	PmBackColor(clutSize-1);		/* white in our current clut */
	EraseRect(&window->portRect);	/* Fill with background color */
	GetFNum((StringPtr)"\pHelvetica",&FontNum);
	TextFont(FontNum);
	TextFace(bold+italic);
	TextSize(textSize);
	j=(unsigned int)((1.0 - contrast)*(clutSize-1));
	PmForeColor(j);
	s = (unsigned char *)"\pHi";
	SetRect(&r,0,0,StringWidth(s),textSize);
	r.right+=0.3*textSize;	// Allow for slant of italic.
	CenterRectInRect(&r,&window->portRect);
	MoveTo(r.left,r.bottom);
	DrawString(s);

	if(1){
		/* Write a gray wedge, just for debugging, so we can examine clut */
		SetRect(&wedge,0,0,20,256);
		CenterRectInRect(&wedge,&window->portRect);
		OffsetRect(&wedge,2-wedge.left,0);
		pt.h=pt.v=0;
		LocalToGlobal(&pt);
		ShieldCursor(&wedge,pt);
		for (j=0;j<256;j++){
			row[0]=(long)clutSize*j/256;
			for(i=0;i<20;i++)row[i]=row[0];
			SetPixelsQuickly(wedge.left,j+wedge.top,row,20);
		}
		ShowCursor();
		InsetRect(&wedge,-1,-1);
		FrameRect(&wedge);
	}

	/* Filter horizontally */
	SetGDevice(device);				/* Use color table of that screen */
	dstRect=r;						/* dstRect just contains the string ... */
	InsetRect(&dstRect,-dim/2,0);	/* extend dstRect to allow for the blur */
	HLockHi((Handle)window->portPixMap);
	ShieldCursor(&dstRect,pt);
	ConvolveX(f,dim,(BitMap *) *window->portPixMap,(BitMap *) *window->portPixMap,
		&window->portRect,&dstRect);
	ShowCursor();

	/* Filter vertically */
	InsetRect(&dstRect,0,-dim/2);	/* Extend dstRect to allow for the blur */
	ShieldCursor(&dstRect,pt);
	ConvolveY(f,dim,(BitMap *) *window->portPixMap,(BitMap *) *window->portPixMap,
		&window->portRect,&dstRect);
	ShowCursor();
	SetGDevice(oldDevice);			/* Restore original device */

	GetFNum((StringPtr)"\pChicago",&FontNum);
	TextFont(FontNum);
	TextSize(12);
	TextFace(0);
	PmForeColor(0);				/* black in our current clut */
	MoveTo(10,window->portRect.bottom-10);
	DrawString((StringPtr)"\pDone. Click mouse or hit any key to exit.");
	printf("Done. Click mouse or hit any key to exit.\n");
	FlushEvents(everyEvent,0);
	while(!GetNextEvent(keyDownMask+mDownMask,&event)) ;
	SetGWorld(oldPort,oldDevice);
	GDDisposeWindow(window);
	// restore
	error=SetDepth(device,(**(**device).gdPMap).pixelSize,1<<gdDevType,oldIsColor);
	#if (THINK_C || THINK_CPLUS)
		abort();
	#elif __MWERKS__
		SIOUXSettings.autocloseonquit=1;
	#endif
}


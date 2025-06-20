/* Filter.c
� 1989-1993 Denis G. Pelli 
Display one letter and low-pass filter it.
Also display a gray wedge, to confirm that lookup table has been correctly loaded.

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
3/10/92	dgp	include mc68881.h
7/20/92	dgp	added printf of done message
8/27/92	dgp	replace SysEnvirons() by Gestalt()
1/4/93	dgp	Don't call SetLuminances if device is of fixedType.
2/7/93	dgp	Updated to use SetPixelsQuickly. Tidied up the grayscale wedge.
			Recompiled with the new ConvolveX.c, which has better rounding.
2/8/93	dgp	Recompiled with the new ConvolveX.c, overflow bug now fixed.
7/7/3	dgp	Made rect a tad bigger to allow for slant of italic.
*/
#include "VideoToolbox.h"
#include "Luminance.h"
#include <math.h>
#include <Fonts.h>
#if THINK_C
	#include <console.h>
#endif

#define SIZE 127		/* point size of text to be filtered */
#define DIM 64			/* size of point spread function, sigma=DIM/4 */

void Filter(void);

void main(void)
{
	Require(gestalt8BitQD);
	Filter();
}

void Filter(void)
{
	EventRecord myEvent;
	register short i;
	int j,clutSize,dim=DIM;
	short myTextSize=SIZE,FontNum;
	unsigned char *s;
	Rect r,dstRect,wedge;
	CWindowPtr window;
	WindowPtr oldPort;
	char string[100];
	GDHandle device,oldDevice;
	double f[DIM],contrast,a,c,sigma;
	static luminanceRecord LR,*LP;
	static unsigned long row[512];

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
	#if THINK_C
		console_options.nrows = 5;
		console_options.left=32;
		printf("\n");
	#else
		InitGraf((Ptr) &thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
	printf("Welcome to Filter.\n");
	GetPort(&oldPort);

	/* Find device corresponding to the experimental screen. */
	oldDevice = GetGDevice();
	for(i=4;i>=0;i--){
		device = GetScreenDevice(i);
		if(device!=NULL) break;
	}
	clutSize=GDClutSize(device);
	printf("%d colors.\n",clutSize);

	/* Open window once, and keep open for whole experiment. */
	window = GDOpenWindow(device);
	MoveHHi((Handle)window->portPixMap);
	HLock((Handle)window->portPixMap);

	/* Use results of last screen calibration to do gamma correction */
	#include "LuminanceRecord1.h"
	printf("Have you installed an ISR Video Attenuator on that monitor?(No):");
	gets(string);
	i=toupper(string[0]);
	if(i!='Y'){
		printf("No\n");
		LR.r=0.0;
		LR.g=1.0;
		LR.b=0.0;
	}
	else printf("Yes\n");
	if((*device)->gdType!=fixedType) SetLuminances(device,&LR,0,clutSize-1,0.0,LR.LMax);

	contrast = 1.0;
	if(0){
		/* This is how to ask for numbers from the experimenter. */
		/* Supply default value in parentheses. */
		printf("Contrast? (%f) ",contrast);
		gets(string);
		sscanf((char *)string,"%lf",&contrast);
		printf("=%f\n",contrast);
	}

	/* Display text */
	SetPort((WindowPtr)window);
	PmForeColor(0);				/* black in our current clut */
	PmBackColor(clutSize-1);		/* white in our current clut */
	EraseRect(&window->portRect);			/* Fill with background color */
	GetFNum((StringPtr)"\pHelvetica",&FontNum);
	TextFont(FontNum);
	TextFace(bold+italic);
	TextSize(myTextSize);
	j=(unsigned int)((1.0 - contrast)*(clutSize-1));
	PmForeColor(j);
	s = (unsigned char *)"\pHi";
	SetRect(&r,0,0,StringWidth(s),myTextSize);
	r.right+=0.3*myTextSize;	// Allow for slant of italic.
	CenterRectInRect(&r,&window->portRect);
	MoveTo(r.left,r.bottom);
	DrawString(s);

	#if 1
		/* Write a gray wedge, just for debugging, so we can examine clut */
		SetRect(&wedge,0,0,20,256);
		CenterRectInRect(&wedge,&window->portRect);
		OffsetRect(&wedge,2-wedge.left,0);
		for (j=0;j<256;j++){
			row[0]=(long)clutSize*j/256;
			for(i=0;i<20;i++)row[i]=row[0];
			SetPixelsQuickly(wedge.left,j+wedge.top,row,20);
		}
		InsetRect(&wedge,-1,-1);
		FrameRect(&wedge);
	#endif

	/* Filter horizontally */
	SetGDevice(device);				/* Use color table of that screen */
	dstRect=r;						/* dstRect just contains the string ... */
	InsetRect(&dstRect,-dim/2,0);	/* extend dstRect to allow for the blur */
	ConvolveX(f,dim,(BitMap *) *window->portPixMap,(BitMap *) *window->portPixMap,
		&window->portRect,&dstRect);

	/* Filter vertically */
	InsetRect(&dstRect,0,-dim/2);	/* Extend dstRect to allow for the blur */
	ConvolveY(f,dim,(BitMap *) *window->portPixMap,(BitMap *) *window->portPixMap,
		&window->portRect,&dstRect);
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
	while(!GetNextEvent(keyDownMask+mDownMask,&myEvent));
	SetPort(oldPort);
	GDDisposeWindow(device,window);
	abort();
}


/*
FlickeringGrating.c
This demo displays a research-grade visual stimulus. If you just want to know
how to load the clut and display a pattern, then you should start by reading
Grating.c, which is much shorter and simpler.
Copyright (c) 1989-1993 Denis G. Pelli
HISTORY:
11/89 	Lan & Denis wrote it.
23.1.90	dgp		Use second screen only if available.
4/9/90	dgp		Changed WindowPtr to CWindowPtr. Made big arrays static. Reduced
				default memory allocation to 1 megabyte. Use any 8-bit screen,
				preferably not the main screen.
4/23/90	dgp		Added optional timing. Centered the image. Left clut entries 0 and 
				clutSize-1 alone, so background doesn't flash. Asked if ISR Video 
				Attenuator is present.
10/11/90 dgp	Added fpu test.
10/29/90 dgp	Added CenterRectInRect().
10/30/90 dgp	Changed call to SetLuminances() to SetLuminancesAndRange() so that
				the range is now kept fixed throughout all the frames, to avoid
				flashes.
8/24/91	dgp		Made compatible with THINK C 5.0.
				If possible, use ReadLuminanceRecord().
3/10/92	dgp		include mc68881.h
4/26/92	dgp		RestoreCluts().
8/27/92	dgp		replace SysEnvirons() by Gestalt()
12/30/92 dgp	made more like Filter, using a small console so that, if necessary,
				both the console and the grating will fit on the main monitor.
2/7/93	dgp		replaced SetOnePixel by SetPixelsQuickly
7/7/93	dgp		added code for compatibility with Radius PowerView, a SCSI video box.
*/
#include "VideoToolbox.h"
#include "Luminance.h"
#include <math.h>
#if THINK_C
	#include <console.h>
	#include <profile.h>
#endif

#define SIZE 		300			// size of grating, in pixels
#define TMAX		3*67		// duration, in frames, typically at 67 Hz
#define REPETITIONS	5			// Number of times to repeat the animation
#define PROFILE		0			// optionally, report timing

void main(void);
void FlickeringGrating(void);
typedef struct{
	ColorSpec table[256];
}ColorSpecTable;

void main(void)
{
	Require(gestalt8BitQD);
	FlickeringGrating();
}
	
void FlickeringGrating(void)
{
	register int i;
	int j,tmax,pixelSize,clutSize;
	static double fX[SIZE],fY[SIZE];
	double LMid,LMin,LMax,dL,a,contrast;
	WindowPtr window=NULL, oldWindow=NULL;
	GDHandle device=NULL,oldDevice;
	static luminanceRecord LR;
	static ColorSpecTable *tables[TMAX];	// an array of pointers to ColorSpec tables
	static char string[100];
	Rect r;
	Boolean yes;
	RgnHandle rgn;
	
	#if PROFILE
		InitProfile(200,3);	/* only needed if you want timing info */
		_profile=0;
	#endif
	/* INITIALIZE QuickDraw */
	#if THINK_C
		console_options.title="\pFlickeringGrating";
		console_options.top = 20;
		console_options.nrows = 4;
		printf("\n");
	#else
		InitGraf((Ptr) &thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
//	HideMenuBar();
	printf("Welcome to FlickeringGrating.\n");
	GetPort(&oldWindow);
	for(i=8;i>=0;i--){
		// look for a screen with 8-bit pixels.
		device=GetScreenDevice(i);
		if(device == NULL)continue;
		pixelSize=(*(*device)->gdPMap)->pixelSize;
		if(pixelSize==8)break;
	}
	if(pixelSize!=8 && NewPaletteManager())for(i=8;i>=0;i--){
		// try to force a screen to 8-bit pixels.
		device=GetScreenDevice(i);
		if(device == NULL)continue;
		SetDepth(device,8,1L<<gdDevType,1);
		pixelSize=(*(*device)->gdPMap)->pixelSize;
		if(pixelSize==8)break;
	}
	if(device==NULL || pixelSize != 8) PrintfExit("Sorry, I require 8 bits/pixel.\n");
	window = GDOpenWindow1(device);
	sprintf(string,"LuminanceRecord%d.h",i);
	i=ReadLuminanceRecord(string,&LR,0);	/* try to read correct file */
	if(i<=0){
		#include "LuminanceRecord1.h"
	}
	printf("Using luminance calibration for screen %d calibrated %s by %s.\n"
		,LR.screen,LR.date,LR.notes);
	printf("Have you installed an ISR Video Attenuator?");
	yes=YesOrNo(0);
	printf("\n");
	if(!yes){
		LR.r=0.0;
		LR.g=1.0;
		LR.b=0.0;
	}
	#if PROFILE
		_profile=1;
	#endif

	/* load clut with linear gray scale */
	// We'll leave clut entries 0 (white) and clutSize-1 (black) alone,
	// since they are used heavily by Apple's stuff. Window frames
	// will mostly look normal if we leave those two entries alone.
	clutSize=GDClutSize(device);
	SetLuminances(device,&LR,1,clutSize-2,LR.LMin,LR.LMax);
		
	/* Display a sinusoid with a gaussian envelope */
	// Compute the image.
	SetPort(window);
	PmBackColor(1+(long)(0.5+(clutSize-3)*0.5));
	EraseRect(&window->portRect);
	SetRect(&r,0,0,SIZE,SIZE);
	CenterRectInRect(&r,&window->portRect);
	for(i=0;i<SIZE;i++){
		a=(i-SIZE/2)/(SIZE/6.);
		fY[i]=exp(-a*a);
		fX[i]=fY[i]*sin((i-SIZE/2)*(2.0*PI/80.0));
	}
	for(j=0;j<SIZE;j++){
		unsigned long row[SIZE];
		for(i=0;i<SIZE;i++) row[i]=1+(long)(0.5+(clutSize-3)*0.5*(1.0+fY[j]*fX[i]));
		SetPixelsQuickly(r.left,j+r.top,row,SIZE);
	}
	// These bits of conditional code force the Radius PowerView
	// to update the screen from the video buffer in memory.
	#if 0	// No good; causes undesired color translation.
		oldDevice=GetGDevice();
		SetGDevice(device);
		CopyBits((BitMap *)*((CGrafPtr)window)->portPixMap
			,(BitMap *)*((CGrafPtr)window)->portPixMap
			,&r,&r,srcCopy,NULL);
		SetGDevice(oldDevice);
	#endif
	#if 1	// Ok, but I don't actually want to scroll.
		rgn=NewRgn();
		ScrollRect(&r,0,1,rgn);
		DisposeRgn(rgn);
	#endif
	// Allocate ColorSpec tables, one for each frame
	for(i=0;i<TMAX;i++){
		tables[i]= (ColorSpecTable *)NewPtr(sizeof(ColorSpecTable));
		if(tables[i]==NULL){
			printf("Only room for %d lookup tables (one per frame) ... continuing.\n",i);
			break;
		}
	}
	tmax=i;
	// Compute lookup tables.
	LMid=(LR.LMax+LR.LMin)/2.0;
	contrast=(LR.LMax-LR.LMin)/(LR.LMax+LR.LMin);
	LMax=LMid*(1.0+contrast);
	LMin=LMid*(1.0-contrast);
	for(i=0;i<tmax;i++){
		a=6.0*(i-tmax/2)/tmax;
		dL=LMid*contrast*exp(-a*a)*sin((i-tmax/2)*(2.0*PI*3.0*0.015));
		SetLuminancesAndRange(NULL,&LR,1,clutSize-2,LMid-dL,LMid+dL,LMin,LMax);
		*tables[i]= *(ColorSpecTable *)LR.table;
	}
	printf("Now displaying the animation ...\n");
	for(j=0;j<REPETITIONS;j++){
		for(i=0;i<tmax;i++){
			// This is a thinly disguished call to GDSetEntries.
			LoadLuminances(device,(luminanceRecord *) tables[i],1,clutSize-2);
		}
	}
	#if PROFILE
		_profile=0;
	#endif
	for(i=0;i<tmax;i++)DisposPtr((Ptr)tables[i]);
	SetPort(oldWindow);
	GDDisposeWindow1(window);
	RestoreCluts();
//	ShowMenuBar();
	#if !PROFILE
		abort();
	#endif
}

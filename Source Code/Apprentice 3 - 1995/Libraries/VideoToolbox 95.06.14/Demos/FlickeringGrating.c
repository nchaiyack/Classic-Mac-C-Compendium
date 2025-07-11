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
4/26/92	dgp		RestoreCluts().
8/27/92	dgp		replace SysEnvirons() by Gestalt()
12/30/92 dgp	made more like Filter, using a small console so that, if necessary,
				both the console and the grating will fit on the main monitor.
2/7/93	dgp		replaced SetOnePixel by SetPixelsQuickly
7/7/93	dgp		added code for compatibility with Radius PowerView, a SCSI video box.
9/5/94 dgp removed assumption in printf's that int==short.
11/17/94 dgp Josh Solomon reported a stack overflow, so I added a calls to StackGrow
and assert.
1/11/95	dgp	Tidied up various things. Now switch to color mode if using an ISR Attenuator.
4/11/95 dgp Save and restore the display depth and color mode.
6/1/95 dgp added code to support profiling in CodeWarrior.
6/14/95 dgp added call to ShieldCursor().
*/
#include "VideoToolbox.h"
#include "Luminance.h"
#if (THINK_C || THINK_CPLUS)
	#define THINK_C_PROFILE		0			// optionally, report timing
	#include <console.h>
	#include <profile.h>
#endif
#if __MWERKS__
	#include <SIOUX.h>
	#include <profiler.h>
#endif
#if UNIVERSAL_HEADERS
	#include <LowMem.h>
#else
	#define LMGetMBarHeight() (* (short *) 0x0BAA)
	#define LMSetMBarHeight(MBarHeightValue) ((* (short *) 0x0BAA) = (MBarHeightValue))
#endif

#define SIZE 		400			// size of grating, in pixels
#define TMAX		6*67		// duration, in frames, typically at 67 Hz
#define REPETITIONS	1			// Number of times to repeat the animation

void main(void);
void FlickeringGrating(void);
typedef struct{
	ColorSpec table[256];
}ColorSpecTable;

void main(void)
{
	StackGrow(40000L+2*SIZE*sizeof(double)+SIZE*sizeof(long)+sizeof(LuminanceRecord)+TMAX*sizeof(Ptr));
	MaxApplZone();
	Require(gestalt8BitQD);
	FlickeringGrating();
}
	
void FlickeringGrating(void)
{
	register int i;
	int j,tmax,clutSize,error,oldPixelSize,oldIsColor;
	double fX[SIZE],fY[SIZE];
	double LMid,LMin,LMax,dL,a,contrast;
	CWindowPtr window=NULL;
	WindowPtr oldPort=NULL;
	GDHandle device=NULL;
	LuminanceRecord LR,*LP;
	ColorSpecTable *tables[TMAX];	// an array of pointers to ColorSpec tables
	char string[100];
	Rect r;
	Boolean attenuatorInstalled;
	long finalTicks;
	Point pt;
	
	assert(StackSpace()>8000);
	#if THINK_C_PROFILE
		InitProfile(200,3);	/* only needed if you want timing info */
		_profile=0;
	#endif
	#if __MWERKS__ && __profile__
		ProfilerInit(collectDetailed,bestTimeBase,110,20);
		ProfilerSetStatus(0);
	#endif
	/* INITIALIZE QuickDraw */
	MaximizeConsoleHeight();
	#if (THINK_C || THINK_CPLUS)
		console_options.title="\pFlickeringGrating";
		console_options.nrows = 5;
	#elif __MWERKS__
		SIOUXSettings.rows=5;
		SIOUXSettings.autocloseonquit=0;
		SIOUXSettings.showstatusline=0;
		SIOUXSettings.asktosaveonclose=0;
	#else
		InitGraf(&qd.thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
	printf("\n");	// make sure that oldPort is the console
	GetPort(&oldPort);
	printf("Welcome to FlickeringGrating.\n");

	for(i=8;i>=0;i--){
		// look for a screen with 8-bit pixels.
		device=GetScreenDevice(i);
		if(device == NULL)continue;
		if((*(*device)->gdPMap)->pixelSize==8)break;
	}
	do{
		if(GetScreenDevice(1)!=NULL)i=ChooseScreen(i,"Which screen?");
		else i=0;
		device=GetScreenDevice(i);
	}while(device==NULL);
	oldIsColor=TestDeviceAttribute(device,gdDevType);
	oldPixelSize=(*(*device)->gdPMap)->pixelSize;
	if(NewPaletteManager() && (oldPixelSize!=8 || !oldIsColor))
		SetDepth(device,8,1<<gdDevType,1);
	if(device==NULL || (*(*device)->gdPMap)->pixelSize!=8)
		PrintfExit("Sorry, I require 8 bits/pixel.\n");
	sprintf(string,"LuminanceRecord%d.h",i);
	LP=&LR;
	i=ReadLuminanceRecord(string,LP,0);	/* try to read correct file */
	if(i<=0){
		#include "LuminanceRecord1.h"
	}
	attenuatorInstalled=Choose(0,"Have you installed an ISR Video Attenuator?\n",noYes,2);
	printf("Using luminance calibration for screen %d calibrated %s by %s.\n"
		,(int)LP->screen,LP->date,LP->notes);
	if(!attenuatorInstalled){
		LP->r=0.0;
		LP->g=1.0;
		LP->b=0.0;
	}
	window=GDOpenWindow(device);
	#if THINK_C_PROFILE
		_profile=1;
	#endif
	#if __MWERKS__ && __profile__
		ProfilerSetStatus(1);
	#endif

	/* load clut with linear gray scale */
	// We'll leave clut entries 0 (white) and clutSize-1 (black) alone,
	// since they are used heavily by Apple's stuff. Window frames
	// will mostly look normal if we leave those two entries alone.
	clutSize=GDClutSize(device);
	SetLuminances(device,LP,1,clutSize-2,LP->LMin,LP->LMax);
		
	/* Display a sinusoid with a gaussian envelope */
	// Compute the image.
	SetPort((WindowPtr)window);
	PmBackColor(1+(long)(0.5+(clutSize-3)*0.5));
	EraseRect(&window->portRect);
	SetRect(&r,0,0,SIZE,SIZE);
	CenterRectInRect(&r,&window->portRect);
	for(i=0;i<SIZE;i++){
		a=(i-SIZE/2)/(SIZE/6.);
		fY[i]=exp(-a*a);
		fX[i]=fY[i]*sin((i-SIZE/2)*(2.0*PI/80.0));
	}
	assert(StackSpace()>4L*SIZE+4000);
	pt.h=pt.v=0;
	LocalToGlobal(&pt);
	ShieldCursor(&r,pt);
	for(j=0;j<SIZE;j++){
		unsigned long row[SIZE];
		for(i=0;i<SIZE;i++) row[i]=1+(long)(0.5+(clutSize-3)*0.5*(1.0+fY[j]*fX[i]));
		SetPixelsQuickly(r.left,j+r.top,row,SIZE);
	}
	ShowCursor();
	// Allocate ColorSpec tables, one for each frame
	for(i=0;i<TMAX;i++){
		tables[i]=(ColorSpecTable *)NewPtr(sizeof(ColorSpecTable));
		if(tables[i]==NULL){
			printf("Only room for %d lookup tables (one per frame) ... continuing.\n",i);
			break;
		}
	}
	tmax=i;
	// Compute lookup tables.
	LMid=(LP->LMax+LP->LMin)/2.0;
	contrast=(LP->LMax-LP->LMin)/(LP->LMax+LP->LMin);
	LMax=LMid*(1.0+contrast);
	LMin=LMid*(1.0-contrast);
	SetLuminancesAndRange(device,LP,1,clutSize-2,LMid,LMid,LMin,LMax);	// blank screen
	for(i=0;i<tmax;i++){
		a=6.0*(i-tmax/2)/tmax;
		dL=LMid*contrast*exp(-a*a)*sin((i-tmax/2)*(2.0*PI*3.0*0.015));
		SetLuminancesAndRange(NULL,LP,1,clutSize-2,LMid-dL,LMid+dL,LMin,LMax);
		*tables[i]= *(ColorSpecTable *)LP->table;
	}
	printf("Now displaying the animation ...\n");
	Delay(60,&finalTicks);
	for(j=0;j<REPETITIONS;j++){
		for(i=0;i<tmax;i++){
			// This is a thinly disguished call to GDSetEntries.
			LoadLuminances(device,(LuminanceRecord *) tables[i],1,clutSize-2);
		}
	}
	#if THINK_C_PROFILE
		_profile=0;
	#endif
	#if __MWERKS__ && __profile__
		ProfilerSetStatus(0);
	#endif
	for(i=0;i<tmax;i++)DisposPtr((Ptr)tables[i]);
	SetPort(oldPort);
	GDDisposeWindow(window);
	if(NewPaletteManager())
		error=SetDepth(device,oldPixelSize,1<<gdDevType,oldIsColor);	// restore
	else RestoreCluts();
	#if (THINK_C || THINK_CPLUS) && !THINK_C_PROFILE
		if(tmax==TMAX)abort();
	#endif
	#if __MWERKS__ && __profile__
		ProfilerDump("\pFlickeringGrating.Profile");
		ProfilerTerm();
	#endif
	#if __MWERKS__
		if(tmax==TMAX)SIOUXSettings.autocloseonquit=1;
	#endif
}

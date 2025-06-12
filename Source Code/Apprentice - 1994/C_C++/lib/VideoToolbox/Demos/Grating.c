/*
Grating.c
This demo shows how to load the clut and put a vignetted grating onto the
screen. This demo has been kept as simple as possible, to enhance readability,
forsaking accurate control of time and contrast of the stimulus. The
FlickeringGrating demo, on the other hand, is somewhat more elaborate and
presents a research-grade stimulus if the LuminanceRecord.h calibration data are
accurate.

For a real experiment there are some refinements you should consider. This demo
takes a while to create the image on the screen, but you probably want
frame-accurate timing in your experiment. You could either create the image in
an offscreen GWorld and copy it to the screen with CopyBitsQuickly. Or you could
set the clut to uniform gray (every clut entry equal to the background
luminance) while you're computing the image on-screen, and then load a grayscale
ramp into the clut when you want the display to start. Similarly you can make
the image disappear by either loading a new image, or clearing the screen by
calling EraseRect(), or loading a uniform-gray clut.

Another issue that matters for a real experiment is gamma correction. The
numbers loaded into the clut are faithfully transformed into voltages by the
three digital-to-analog converters on your video card, but the resulting
luminance produced on your monitor will be an approximately parabolic function
of the video voltage. Apple provides crude gamma correction by means of a
generic 8-bit gamma table in the video driver, which does make things look
better, but is not accurately matched to the gamma of your particular monitor,
which depends on the current settings of its brightness and contrast knobs.
Furthermore the Apple 8-bit gamma-correction scheme, which simply transforms the
nominal 8-bit clut value into a new 8-bit clut value, necessarily restricts the
range of available values, mapping several onto one output value, and omitting
some output values. In other words your luminances will be 8-bit quantized
twice, both before and after gamma correction. It is preferable to do gamma
correction first, without quantization. Therefore I suggest you eliminate
Apple's gamma correction, by calling GDUncorrectedGamma(), and use the
Luminance.c package to do gamma correction. That package implements the
published algorithm of Pelli and Zhang (1991). The FlickeringGrating demo uses
Luminance.c.

HISTORY:
2/7/93	dgp wrote it, as an answer to questions from Bill Merigan and David Brainard.
2/18/93	dgp	added fpu test.
2/23/93	dgp	use new GDOpenWindow1 and GDDisposeWindow1.
4/18/93	dgp	support directType.
7/7/93	dgp	prefer screen 1. Added ScrollRect for compatibility with Radius PowerView.
10/2/93	dgp	added SAVE_PICT to test PixMapToPICT().
*/
#include "VideoToolbox.h"
#include <math.h>
#if THINK_C
	#include <console.h>
#endif
#define SIZE 300
#define SAVE_PICT	1	// set to zero to disable saving image to disk.

void main(void)
{
	short i,j,error,clutSize,pixelSize,mode;
	short preferredPixelSize[6]={8,32,16,4,2,1};	// Order of preference
	GDHandle device;
	static ColorSpec table[256];
	WindowPtr window,oldPort,w;
	Rect r;
	double a,fX[SIZE],fY[SIZE];
	char string[100];
	Point pt={100,0};
	RgnHandle rgn;
	
	StackGrow(10000);
	#if THINK_C
		console_options.top = 0;
		console_options.left = 0;
		console_options.nrows = 4;
		console_options.ncols = 60;
		printf("\n");
	#else
		InitGraf((Ptr) &thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
	printf("Welcome to Grating.\n");
	Require(gestalt8BitQD);
	GetPort(&oldPort);
	device=GetScreenDevice(1);
	if(device==NULL)device=GetScreenDevice(0);
	
	// Try to get the best possible pixelSize.
	pixelSize=(**(**device).gdPMap).pixelSize;
	if(pixelSize!=preferredPixelSize[i] && NewPaletteManager()){
		for(i=0;i<sizeof(preferredPixelSize)/sizeof(short);i++){
			if(pixelSize==preferredPixelSize[i])break;
			mode=HasDepth(device,preferredPixelSize[i],0,0);
			if(mode!=0){
				printf("Excuse me while I change the pixelSize to %d bits.\n"
					,preferredPixelSize[i]);
				error=SetDepth(device,mode,0,0);
				break;
			}
		}
		pixelSize=(**(**device).gdPMap).pixelSize;
	}

	// Load the clut with a grayscale ramp.
	GDSaveGamma(device);
	GDUncorrectedGamma(device);	// Tell the driver to faithfully copy our colors into 
								// the clut without any transformation.
	clutSize=GDClutSize(device);
	for(i=0;i<clutSize;i++){
		table[i].rgb.red=table[i].rgb.green=table[i].rgb.blue=i*(long)0xffff/(clutSize-1);
	}
	error=GDSetEntriesByType(device,0,clutSize-1,table);

	// Open window and put grating in it.
	SetMouse(pt);
	window=GDOpenWindow1(device);
	SetPort(window);
	PmBackColor((clutSize-1)*0.5);	// This works because GDOpenWindow1 marked
	EraseRect(&window->portRect);	// all the colors as pmExplicit.
	SetRect(&r,0,0,SIZE,SIZE);
	CenterRectInRect(&r,&window->portRect);
	for(i=0;i<SIZE;i++){
		a=(i-SIZE/2)/(SIZE/6.);
		fY[i]=exp(-a*a);
		fX[i]=fY[i]*sin((i-SIZE/2)*(2.0*PI/80.0));
	}
	for(j=0;j<SIZE;j++){
		unsigned long row[SIZE];
		for(i=0;i<SIZE;i++) row[i]=0.5+(clutSize-1)*0.5*(1.0+fY[j]*fX[i]);
		if(pixelSize==16)for(i=0;i<SIZE;i++) row[i]*=1+(1<<5)+(1<<10);
		if(pixelSize==32)for(i=0;i<SIZE;i++) row[i]*=1+(1<<8)+(1UL<<16);
		SetPixelsQuickly(r.left,r.top+j,row,SIZE);
	}
	// This conditional code forces the Radius PowerView
	// to update the screen from the video buffer in memory.
	#if 1	// Ok, but I don't actually want to scroll.
		rgn=NewRgn();
		ScrollRect(&r,0,1,rgn);
		ScrollRect(&r,0,-1,rgn);
		DisposeRgn(rgn);
	#endif
	if(SAVE_PICT)PixMapToPICT("grating.pict",((CWindowPtr)window)->portPixMap
		,&r,2,NULL);
	printf("Done. Hit return to quit.\n");
	gets(string);
	SetPort((WindowPtr)oldPort);
	GDDisposeWindow1(window);
	GDRestoreGamma(device);
	GDRestoreDeviceClut(device);
	abort();
}

/*
TestAttenuator.c
This produced a simple test pattern that allows a quick yes/no evaluation of the
attenuator before shipping. Mainly this is a check to make sure that all the
resistors were soldered correctly.
HISTORY:
11/89 	dgp wrote it
9/9/90	dgp	make the arrays static.
10/17/90	dgp	remove unused variable
2/16/91		dgp	added check for fpu and color quickdraw
8/24/91	dgp	Made compatible with THINK C 5.0.
3/10/92	dgp	include mc68881.h
4/27/92	dgp	call SetDepth to set screen to 8 bits.
8/27/92	dgp	replace SysEnvirons() by Gestalt()
2/7/93	dgp	updated to use SetPixelsQuickly.
*/
#include "VideoToolbox.h"
#include <math.h>
#include "Luminance.h"
#if THINK_C
	#include <console.h>
#endif

#define XMAX 		640		/* Maximum horizontal coordinate */ 
#define YMAX 		480		/* Maxumum vertical coordinate */

void TestAttenuator(void);

void main(void)
{
	Require(gestalt8BitQD);
	TestAttenuator();
}

void TestAttenuator(void)
{
	register short i,j,k;
	double a,A,AA;
	CWindowPtr window=NULL;
	WindowPtr oldWindow=NULL;
	GDHandle device=NULL,oldGDHandle=NULL;
	static luminanceRecord LR;
	short n,mode;
	static double fy[YMAX];
	double c;
	static unsigned long row[XMAX];
	short oldScreen;
	char string[100];
	
	/* parameters of the screen calibration */
	
	console_options.top=YMAX/2-40;
	console_options.nrows=7;
	console_options.txSize=9;
	printf("Welcome to TestAttenuator.\n");
	#include "LuminanceRecord1.h"					// read at compile time
	printf("%s\n",LR.notes);
	oldScreen=LR.screen;
	if(GetScreenDevice(1)!=NULL){
		for(; ;){
			printf("Which screen would you like to calibrate (%d):",LR.screen);
			gets(string);
			sscanf(string,"%d",&LR.screen);
			device=GetScreenDevice(LR.screen);
			if(device!=NULL)break;
			else printf("Sorry, there is no device %d\n",LR.screen);
		}
	}else LR.screen=0;
	sprintf(string,"LuminanceRecord%d.h",LR.screen);
	i=ReadLuminanceRecord(string,&LR,0);		// try to read latest luminanceRecord
	if(i<1)printf("Warning: couldn't find �%s�. Calibrating screen %d.\n"
		,string,LR.screen);
	else oldScreen=LR.screen;

	/* Find device corresponding to the experimental screen. */
	oldGDHandle = GetGDevice();
	device = GetScreenDevice(LR.screen);
	if(GDClutSize(device)!=256 && NewPaletteManager()){
		mode=HasDepth(device,8,1,1);
		if(mode!=0)SetDepth(device,mode,1,1);		// 8-bit pixelSize, color
	}
	if(GDClutSize(device)!=256){
		PrintfExit("Sorry, I require a screen with 256 colors.\n");
	}
	
	GetPort(&oldWindow);
	window = GDOpenWindow(device);
	SetPort((WindowPtr)window);
	
	for(j=0;j<YMAX/2;j++){
		fy[j]=cos(j*2.0*PI/80.0);
	}
	c=1.0;
	SetLuminances(device,&LR,11,90,LR.LMin,LR.LMin+(LR.LMax-LR.LMin)*c);
	c/=6.0;
	SetLuminances(device,&LR,91,170,LR.LMin,LR.LMin+(LR.LMax-LR.LMin)*c);
	c/=6.0;
	SetLuminances(device,&LR,171,250,LR.LMin,LR.LMin+(LR.LMax-LR.LMin)*c);
	/* Display a swept-contrast sinusoid */
	for(j=0;j<YMAX/2;j++){
		k=floor(0.5+79.0*0.5*c*(1.0 + fy[j]));
		k+=11;
		for(i=0;i<1+XMAX/3;i++)row[i]=k;
		SetPixelsQuickly(0,j,row,1+XMAX/3);
	}
	c *=6.0;
	for(j=0;j<YMAX/2;j++){
		k=floor(0.5+79.0*0.5*c*(1.0 + fy[j]));
		k+=91;
		for(i=0;i<1+XMAX/3;i++)row[i]=k;
		SetPixelsQuickly(XMAX/3,j,row,1+XMAX/3);
	}
	c *=6.0;
	for(j=0;j<YMAX/2;j++){
		k=floor(0.5+79.0*0.5*c*(1.0 + fy[j]));
		k+=171;
		for(i=0;i<1+XMAX/3;i++)row[i]=k;
		SetPixelsQuickly(2*XMAX/3,j,row,1+XMAX/3);
	}

	/* compute three clut entries that ought to produce equal luminances */
	AA=LToV(&LR,LR.LMax/4.0);
	printf("%f cd/m^2 equivalent number is %f\n",VToL(&LR,AA),AA);
	i=251;
	A=AA;
	a=A/LR.r;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.red=n<<8;
	A -= n*LR.r;
	a=A/LR.g;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.green=n<<8;
	A -= n*LR.g;
	a=A/LR.b;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.blue=n<<8;
	A -= n*LR.b;
	i++;
	A=AA;
	a=A/LR.g;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.green=n<<8;
	A -= n*LR.g;
	a=A/LR.b;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.blue=n<<8;
	A -= n*LR.b;
	a=A/LR.r;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.red=n<<8;
	A -= n*LR.r;
	i++;
	A=AA;
	a=A/LR.b;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.blue=n<<8;
	A -= n*LR.b;
	a=A/LR.r;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.red=n<<8;
	A -= n*LR.r;
	a=A/LR.g;
	if(a>255.) n=255;
	else n=a;
	LR.table[i].rgb.green=n<<8;
	A -= n*LR.g;
	i++;
	for(i=251;i<254;i++) printf("%4d,%4d,%4d = %6.2f == %7.3f cd/m^2\n",
		LR.table[i].rgb.red/256,LR.table[i].rgb.green/256,LR.table[i].rgb.blue/256,
		LToV(&LR,GetLuminance(NULL,&LR,i)), GetLuminance(NULL,&LR,i));
	LoadLuminances(device,&LR,251,253);
	
	/* Display stripes */
	for(i=0;i<XMAX;) {
		for(k=0;k<10;k++)row[i++]=251;
		for(k=0;k<10;k++)row[i++]=252;
		for(k=0;k<10;k++)row[i++]=253;
	}
	for(j=YMAX/2;j<YMAX;j++)SetPixelsQuickly(0,j,row,XMAX);
	
	printf("Hit return to quit.");
	getchar();
	SetPort(oldWindow);
	SetGDevice(oldGDHandle);
	GDDisposeWindow(device,window);
	RestoreCluts();
	abort();
}

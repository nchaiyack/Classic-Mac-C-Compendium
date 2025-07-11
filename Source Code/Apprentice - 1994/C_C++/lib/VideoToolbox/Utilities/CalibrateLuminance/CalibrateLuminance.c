/*
CalibrateLuminance.c
by Denis Pelli, Lan Zhang, and Preeti Verghese
You will need "Numerical Recipes in C" in order to compile this file; see
limitations below.

USE

You must run this program (or your own equivalent) to calibrate your video card,
ISR Video Attenuator, and monochrome monitor, as described by Pelli & Zhang
(1991).

D.G. Pelli and L. Zhang (1991) Accurate control of contrast on microcomputer
displays. Vision Research, 31:1337-1360.

The program's results are stored in a file called LuminanceRecord?.h that you
can then use in the rest of your C programs. "?" will be a digit indicating
which monitor. LuminanceRecord?.h describes the gains of the three video
pathways of your video card and ISR Video Attenuator, and describes the gamma
function of your monitor, to allow automatic gamma correction later, using
SetLuminance(), etc.

There are two ways of using the LuminanceRecord file. You can use the
preprocessor #include statement, in the midst of your C code, provided you've
got a LuminanceRecord structure called LR. Or you can use the VideoToolbox
routine called ReadLuminanceRecord() to read the data (at runtime) into a
LuminanceRecord of any name.

For historical reasons this program not only measures the gamma function, but
also fits a variety of functions to it. In fact the latest implementation of
Luminance.c, which is the only routine that makes direct use of the gamma
description, would be quite content with just the tabulated gamma function. At
present the gamma function is tabulated very coarsely by CalibrateLuminance, so
it doesn't bother to put this table into the header file LuminanceRecord.h.
However, if you'd rather write your own substitute for this program, then I
would suggest just measuring the luminance at 256 different levels (from 0 to
VMax) and saving the table in the header file. You will also need to measure
the RGB gains, but that can easily be done directly, using an oscilloscope,
rather than trying to be clever, as in here, where we invert the nonlinearity to
infer voltages from the nonlinear luminances that we measure.

CalibrateLuminance also produces a CricketGraph file, CalibrateLuminance%d.data
(where %d is replaced by the monitor number), which is suitable for graphing the
monitor's gamma function. Use the CricketGraph format file CalibrateLuminance.format.

This program measures the luminance of a patch on the screen, using a photometer
and, optionally, a 12-bit Analog to Digital Converter (ADC). If you have a Data
Translation Forerunner Analog-to-Digital card, then GetVoltage.c will
automatically find and use it. You can set the MANUAL flag to force manual
operation. If your photometer has an analog output, it's convenient and more
accurate to have all the voltages read in automatically via an ADC, but you
won't need to run CalibrateLuminance very often, and you can get by with manual
calibration.

You should set the background luminance to approximately the same value as you
will use in your experiment. Note that the channels gains depend on the DACs as
well as the ISR Video Attenuator, and will vary from DAC to DAC by �5%. That's
why you must do this calibration yourself using your own video card and monitor.

This program can calibrate any of your screens, including the main screen. It
now works fine with a single screen, alternating the user dialog with the
measurements.

LIMITATIONS

CalibrateLuminance uses routines from Numerical Recipes in C to do the
polynomial and power law fits to the gamma function. They're copyrighted, so I
can't distribute them. Note: I HAVE included a compiled application
CalibrateLuminance that you can use NOW. You only need to buy the Numerical
Recipes if you want to MODIFY CalibrateLuminance.c. See "Improve Numerical Recipes" 
in the notes folder.

HISTORY

4/25/89	Preeti and Denis
6/18/89	Denis added numerical display of clut index and triple grating for first screen.
8/4/89	Denis replaced SetEntries call by GDSetEntries, and generally updated everything.
8/18/89	Lan Measure the whole routine(R+G+B and R G B gain) 40 times to eliminate the
		effect of screen luminance drift.
9/8/89	Lan & Denis generally tidied it up.
9/8/89	denis: replaced polynomial fit by powerlaw fit
9/10/89 denis: got gain measurement to work with sufficient accuracy. 
		This involved many small changes. I now
		wait for a second after any large luminance change to allow the
		photometer to settle. I also measure the channel gains at many different
		settings of the other DACs, in order to average out the effect of DAC
		inaccuracies.
10/30/89 Lan & Denis: introduced the option of manual readings, and made console smaller.
11/17/89 Lan & Denis: cleaned up for general release.
11/30/89 Denis: added comment to LuminanceRecord.h explaining power[] parameters.
3/29/90	dgp	2.15 Updated to use new GetVoltage that looks for ForeRunner card,
			and new GDOpenWindow() that uses CWindowPtr instead of WindowPtr. Introduced
			conditionals so it compiles without errors under MPW C 3.1. 
4/21/90	dgp	2.20. Fixed the bug in the fixed-power power law fit. Updated whole file
			to be compatible with latest versions of all subroutines. Corrected printout
			of nBackground.
7/28/90	dgp	2.3. Added code (in GetALuminance.c) to find the equivalent number to
			produce any desired background luminance.
			All luminance measurement is now done via the new subroutine GetALuminance. 
			Automatic and manual measurements now use the exactly the same code.
			Calibration of (linear) RGB gains is now optional.
9/18/90	dgp	2.4. Changed all instances of "v" to "V". Pelli
			& Zhang (1991) refer to a nominal voltage v; this file
			now refers to the "equivalent number" V; they are related by V=VMax*v.
9/22/90	dgp	2.5. Fixed cosmetic errors that prevented use of this program on the main
			screen. The trick is appropriate use of BringToFront() and SendBehind()
			referring only to my own window. Trying to do BringToFront() on the console
			often caused a bus error, for no obvious reason. I'll have to ask Mike
			Kahl.
9/24/90	dgp	2.6. Added screen to the LuminanceRecord.h file and appended it to
			the LuminanceRecord.h and .data filenames. This makes it easy to calibrate
			all your monitors and keep the records straight.
			Added LR.date string.
			New default is to retain old gainAccuracy when rgb gains are not remeasured.
10/10/90 dgp Added SetDepth(). Fixed bug that reducing the number of frames sampled
			per a/d measurement by the number of cycles. The number of frames per
			measurement is now fixed.
10/12/90 dgp Added LR.dpi and LR.Hz to the LuminanceRecord?.h file.
10/17/90 dgp Removed unused variables. Added reference to paper to LuminanceRecord?.h.
12/12/90 dgp Measure and subtract off the dark voltage.
			Fixed dumb error of assuming wrong type when printing LP->VMax
			& LP->coefficients, which was resulting in zeroes in the LuminanceRecord.
4/15/91	dgp Check for NewPaletteManager().
8/24/91	dgp	made compatible with THINK C 5. This required introducing several
			casts: (unsigned long *) and (unsigned char *). Hopefully, this won't
			compromise compatibility with old THINK C 4.
3/10/92	dgp	include mc68881.h
6/23/92	dgp	added quick check of photometer gain.
8/27/92	dgp	replace SysEnvirons() by Gestalt()
10/23/92 dgp read latest LuminanceRecord for default values. Mentioned ReadLuminanceRecord()
			in the LuminanceRecord header file.
10/24/92 dgp use GDFrameRate() to measure the frame rate.
11/13/92 dgp advertise ReadLuminanceRecord.c
12/17/92 dgp Enhanced to support arbitrary dacSize. Get dacSize by calling GDGetGamma(). 
			Mostly I just replaced 255 by LP->VMax. Didn't test it after changes.
12/21/92 dgp No longer load unused dac bits.
2/23/93	dgp	Use new GDOpenWindow1 and GDDisposeWindow1.
4/3/93	dgp	Fix bug that causes GetALuminance to fail (i.e. always set entry to black)
			if the luminanceRecord hasn't been initialized.
4/18/93	dgp	Replaced ctSize by clutSize. Now call GDClutSize. Fixed call to SetDepth.
5/4/93	dgp	Leave gray/color mode alone, but warn if in gray mode.
5/12/93	dgp When in gray mode don't bother to offer to calibrate rgb gains.
6/4/93	dgp Conditionally changed the data-saving subroutine to use the
			new WriteLuminanceRecord subroutine. This hasn't been tested yet, and
			we can revert back to the old code by changing the #if 1 to #if 0.
			The virtue of the new code is that it verifies what was written.
*/
#include <VideoToolbox.h>
#include <math.h>
#include <assert.h>
#include "Luminance.h"
#include <Fonts.h>
#include <Packages.h>
#if THINK_C
	#include <console.h>
#endif
#include <nr.h>			/* prototypes of Numerical Recipes and definition of FLOAT */
#include <nrutil.h>
long InitializeLuminanceRecord(luminanceRecord *LP,short flags);

#define AUTOMATIC		1	/* 	=0:input reading by hand from the photometer to the terminal
							   	=1: use A/D converter to read from the photometer directly */
#define NLRGB			16	/* number of times to measure each RGB gain, should be >1*/
#define LUMINANCES		32	/* approx. number of different luminances to measure */
#define	NFRAME			60	/* number of frames per ADC luminance measurement */
#define MAX_WINDOWS		6	/* maximum number of screens that you might have */
#define ROUND_LUMINANCES (2+(256+256/LUMINANCES-1)/(256/LUMINANCES))
							/* round LUMINANCES up to the next divisor of 256, add 2 */

void main(void);
void CalibrateLuminance(void);
void SaveData(luminanceRecord *LP,int nL,int V[],double L[]
	,int nLrgb,int nrgb[3][NLRGB][3],double Lrgb[3][NLRGB],double gain[3]);
void SaveLuminanceRecord(luminanceRecord *LP);
FLOAT PowerRMSError(FLOAT p[]);
double GetALuminance(luminanceRecord *LP,GDHandle device,
	int frames,double LuminancePerVoltage,int entry,int red,int green,int blue);
void LToVHunt(luminanceRecord *LP,GDHandle device,CWindowPtr window,
	double LuminancePerVoltage,int frames,double darkLuminance);

/* These variables are out here, as globals, so that PowerRMSError can access them directly */
static int nL=ROUND_LUMINANCES;
static double L[ROUND_LUMINANCES];	/* luminance at V[i] */
static int V[ROUND_LUMINANCES];
static int variables=4;
static FLOAT *p;

void CalibrateLuminance(void);

void main(void)
{
	long system;
	
	Gestalt(gestaltSystemVersion,&system);
	if(system<0x605)PrintfExit("Sorry, I need at least System 6.05.\n");
	Require(gestalt8BitQD);
	CalibrateLuminance();
}

void CalibrateLuminance(void)
{
	short error,i,j,k,ii,bottom,FontNum,screen;
	long value;
	char string[100];
	Rect myRect,TestRect,labelRect;
	GDHandle device=NULL,oldDevice=NULL;
	WindowPtr window=NULL,oldWindow=NULL,windows[MAX_WINDOWS];
	int nLrgb=NLRGB;
	static double Lrgb[3][NLRGB];
	static int nrgb[3][NLRGB][3];
	double gain[3],LuminancePerVoltage=1000.0;
	static luminanceRecord LR,*LP;
	double *x,*y,*sig;						// for polynomial fit
	FLOAT *a,**u,**v,*w,**cvm,chisq;		// for polynomial fit
	int ma;									// for polynomial fit
	FLOAT **xi,ftol,fret;					// for power law fit
	int iter;								// for power law fit
	double e,f,VV,luminance,darkLuminance;
	short readNumber,readTotal,isGray,testSize,cycles,textSize,frames,clutSize,mode;
	Boolean automatic,skipRGB,yes;
	char colorGray[2][8]={"color","gray"};
	
	assert(StackSpace()>4000);
	automatic=AUTOMATIC && (CardSlot(".ForeRunner")>0);	/* disable if card is missing */
	LP=&LR;
//	#include "LuminanceRecord1.h" 						/* get defaults from old calibration */
	#if THINK_C
		console_options.title="\pCalibrateLuminance";	/* change the console window */
		console_options.nrows=12;
		console_options.top=20;
		console_options.left=0;
		console_options.ncols=105;
		console_options.txSize=9;
		printf("\n");									/* ask THINK C to initialize QuickDraw */
		CopyQuickDrawGlobals();							// Make sure qd is valid.
	#else
		InitGraf(&qd.thePort);
		InitWindows();
		InitFonts();
	#endif

	oldDevice=GetGDevice();
	GetPort(&oldWindow);
	GetFNum((StringPtr) "\pChicago",&FontNum);
	for(i=0;i<MAX_WINDOWS;i++){
		device=GetScreenDevice(i);
		if(device == NULL)break;
		windows[i]=GDOpenWindow1(device);
		SendBehind(windows[i],NULL);
		SetPort(windows[i]);
		TextFont(FontNum);
		textSize=128;
		TextSize(textSize);
		myRect=windows[i]->portRect;
		bottom=myRect.bottom;
		MoveTo(myRect.right/2,myRect.bottom/2);
		sprintf(string,"%1d",i);
		CtoPstr(string);
		Move(-StringWidth((void *)string)/2,textSize/2);
		DrawString((StringPtr) string);
	}
	SetPort(oldWindow);
	SetGDevice(oldDevice);

	printf("Welcome to CalibrateLuminance (version 12/17/92).\n\n"
	"This program helps you to calibrate your ISR Video Attenuator, as well as your video card\n"
	"and monitor. 'Enter' indicates that you should type in a number or letter followed by 'RETURN'.\n"
	"Most questions provide a default answer (in parentheses).\n"
	"Just hit 'RETURN' if you want the default. (Or hit Command-. to quit.)\n"
	"Where appropriate, the default is taken from latest LuminanceRecord.\n");

	screen=0;
	if(GetScreenDevice(1) != NULL)screen=1;
	printf("\nEnter the number of the screen you want to calibrate (%d):",screen);
	gets(string);
	sscanf(string,"%d",&screen);
	printf("%d\n",screen);
	for(i=0;i<MAX_WINDOWS;i++){
		device=GetScreenDevice(i);
		if(device == NULL)break;
		GDDisposeWindow1(windows[i]);
	}
	sprintf(string,"LuminanceRecord%d.h",screen);
	InitializeLuminanceRecord(LP,0);
	i=ReadLuminanceRecord(string,LP,0);		// try to read latest luminanceRecord
	if(i>0)printf("Successfully read old �%s� to set default values.\n",string);
	else{
		printf("Couldn't find an old copy of �%s�; creating new one from scratch.\n",string);
		LR.dpi=76.0;	/* pixels per inch */
		LP->units="cd/m^2";
	}
	LP->screen=screen;
	LP->rangeSet=0;	/* indicate that range parameters have yet to be set */
	LP->L.exists=0;	/* indicate that luminance table has yet to be initialized */
	GetPort(&oldWindow);
	oldDevice=GetGDevice();

	device=GetScreenDevice(LP->screen);		// choose which screen to test
	window=GDOpenWindow1(device);
	SendBehind(window,NULL);
	LP->dacSize=GDDacSize(device);			// Takes 200 �s.
	clutSize=GDClutSize(device);	
	LP->VMin=0;
	LP->VMax=(1L<<LP->dacSize)-1;	/* maximum value that can be loaded into DAC */
	isGray=!TestDeviceAttribute(device,gdDevType);
	printf("Test screen is set to %d colors and %s mode.\n",clutSize,colorGray[isGray]);
	if(clutSize<4){
		for(i=4;i<=32;i*=2){
			mode=HasDepth(device,i,0,0);
			if(mode!=0)break;
		}
		if(mode==0)PrintfExit("\n\007Sorry. This program cannot calibrate a display"
			"with less than 4 colors/grays.");
		printf("Changing screen depth to %d-bit pixels\n",i);
		if(mode==0x100)PrintfExit(BreakLines("Sorry, the HasDepth routine doesn't work properly. "
		"Use the Monitors Control Panel to set the colors/grays and then run "
		"CalibrateLuminance again.\n",80));
		SetDepth(device,mode,0,0);
		clutSize=GDClutSize(device);	
	}
	if(isGray){
		printf("Would you prefer color mode?");
		yes=YesOrNo(0);
		printf("\n");
		if(yes){
			if(HasDepth(device,(**(**device).gdPMap).pixelSize,1,1))
				SetDepth(device,(**device).gdMode,1,1);
			else printf("Sorry, there is no color mode. Continuing in gray mode.\n");
		}
		isGray=!TestDeviceAttribute(device,gdDevType);
	}
	LP->Hz=GDFrameRate(device);
	// printf("The test screen's frame rate is %.2f Hz\n",LP->Hz);
	
	if(isGray)skipRGB=1;
	else{
		printf("\nThere are two kinds of measurement:\n"
		"1. Compulsory calibration of the (nonlinear) monitor's gamma function.\n"
		"This will be affected by your choice of background luminance.\n"
		"2. Optional calibration of the (linear) ISR Video Attenuator 3 channel gains.\n"
		"This is independent of the background. It takes about 15 minutes.\n\n"
		"Do you wish to skip 2., the optional three-channel calibration?");
		skipRGB=YesOrNo(0);
		printf("\n");
	}
	if(skipRGB)printf("Skipping 3 channel calibration.\n");
	else printf("3 channels will be calibrated.\n");
	
	frames=0;
	if(automatic){
		frames=NFRAME;
		printf("Taking photometer readings automatically.\n"
			"Assuming %.3f milliVolts per %s\n",1000.0/LuminancePerVoltage,LP->units);
	}
	else printf("Please enter the photometer readings,in %s, as prompted.\n",LP->units); 
	printf("\n");

	printf("First let's check the photometer's gain setting.\n");
	printf("Point the photometer at the screen and hit return:");
	gets(string);
	clutSize=GDClutSize(device);
	do{
		SetPort(window);
		BringToFront(window);
		FillRect(&window->portRect,qd.black);
		darkLuminance=GetALuminance(LP,device,frames,LuminancePerVoltage,clutSize-1,0,0,0);
		FillRect(&window->portRect,qd.white);
		luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,0,LP->VMax,LP->VMax,LP->VMax);
		luminance-=darkLuminance;
		SendBehind(window,NULL);
		printf("\rWhite seems to be %.1f %s brighter than black.\n",luminance,LP->units);
		printf("Do you want to try that again?");
	}while(YesOrNo(0));
	printf("\n");

	printf("Please occlude the photometer so that I can read the dark level.\n"
	"Hit cr when ready:");
	gets(string);
	darkLuminance=GetALuminance(LP,device,frames,LuminancePerVoltage,0,LP->VMax,LP->VMax,LP->VMax);
	printf("The dark level of %.2f cd/m^2 will be subtracted from all readings\n"
		,darkLuminance);
	
	testSize=160;
	printf("Enter, in pixels, the diameter of the test patch (%d):",testSize);
	gets(string);
	sscanf((char *)string,"%d",&testSize);

	/* draw circle for placement of photocell */
	SetPort(window);
	myRect=window->portRect;
	InsetRect(&myRect,(myRect.right-testSize)/2,(myRect.bottom-testSize)/2);
	PmForeColor(255);
	PenSize(10,10);
	FrameOval(&myRect);
	SetPort(oldWindow);
	
	printf("Please remove the occluder. \n");
	printf("Place your photometer so as to read the luminance of the "
		"center of the test screen.\n"); 

	if(skipRGB)cycles=1;
	else cycles=4;
	printf("Then enter number of times you wish to repeat the whole cycle of measurement. (%d):"
		,cycles);
	gets((char *)string);
	sscanf((char *)string,"%d",&cycles);
	if(cycles<=0)return;

	SetPort(window);
	BringToFront(window);
	FillRect(&window->portRect,qd.gray);
	luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,0,LP->VMax,LP->VMax,LP->VMax);
	luminance-=darkLuminance;
	printf("50%% dithered gray is %.1f %s\n",luminance,LP->units);
	FillRect(&window->portRect,qd.white);
	luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,0,LP->VMax,LP->VMax,LP->VMax);
	luminance-=darkLuminance;
	printf("White is %.1f %s\n",luminance,LP->units);
	FillRect(&window->portRect,qd.black);
	luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,0,LP->VMax,LP->VMax,LP->VMax);
	luminance-=darkLuminance;
	printf("Black is %.1f %s\n",luminance,LP->units);
	SendBehind(window,NULL);

	printf("It is essential that the calibration be made with the same background\n"
	"luminance as will ultimately be used in your experiments. You may specify the\n"
	"background luminance either as a luminance in %s, or as a corresponding \n"
	"DAC value (0..%d).\n",LP->units,LP->VMax);
	printf("Do you wish to enter a luminance?");
	if(YesOrNo(1)){
		printf("\nEnter the desired background luminance in %s (%f):",LP->units,LP->LBackground);
		gets(string);
		sscanf(string,"%lf",&LP->LBackground);
		printf("Now hunting for the DAC value . . .\n");
		LToVHunt(LP,device,(CWindowPtr)window,LuminancePerVoltage,frames,darkLuminance);
		printf("Will use the nearest DAC value, which is %d\n",LP->VBackground);
	}
	else {
		printf("\nEnter the DAC value for the background (%d):",LP->VBackground);
		gets(string);
		sscanf((char *)string,"%d",&LP->VBackground);
	}
	LP->table[2].rgb.red=LP->table[2].rgb.green=LP->table[2].rgb.blue=
		LP->VBackground<<LP->leftShift;
	LoadLuminances(device,LP,2,2);
	SetPort(window);
	BringToFront(window);
	PmBackColor(2);
	EraseRect(&window->portRect);
	luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,0,LP->VMax,LP->VMax,LP->VMax);
	luminance-=darkLuminance;
	SendBehind(window,NULL);
	SetPort(oldWindow);
	printf("Your background luminance is %.3f %s\n",luminance,LP->units);

	/* plan the luminance and gain measurements */
	if(skipRGB)nLrgb=0;
	readTotal=cycles*(ROUND_LUMINANCES+3*nLrgb);
	printf("Now taking %d readings . . .\n",readTotal);
	for (i=0;i<nL;i++) L[i]=0.0;
	k=0;
	for (i=0;i<nL;i++){
		V[i]=k;
		if(k==LP->VMax) {
			if(i+1<nL) V[++i]=k;	/* put two points at VMax, because it's important */
			nL=i+1;
			break;
		}
		k += (LP->VMax+1)/(nL-2);
		if(k>LP->VMax)k=LP->VMax;
	}
	for (k=0;k<3;k++)for(i=0;i<nLrgb;i+=2){
		for(j=0;j<3;j++) nrgb[k][i+1][j]=nrgb[k][i][j]=(LP->VMax+1)*(0.667+0.333*i/nLrgb);
		nrgb[k][i+1][k]=LP->VMax;
		nrgb[k][i][k]=0;
		if(k==2) nrgb[k][i][k]=(LP->VMax+1)/2;	/* take smaller step, because blue gain is highest */
	}
	for(k=0;k<3;k++)for (i=0;i<nLrgb;i++)Lrgb[k][i]=0.0;
	
	/* measure several times to minimize the effect of luminance drift */
	SetPort(window);
	BringToFront(window);
	GetFNum((StringPtr) "\pMonaco",&FontNum);
	TextFont(FontNum);
	TextSize(36);
	myRect=window->portRect;
	bottom=myRect.bottom;
	SetRect(&TestRect, (myRect.right+myRect.left-testSize)/2
		,(myRect.top+myRect.bottom-testSize)/2
		,(myRect.right+myRect.left+testSize)/2
		,(myRect.top+myRect.bottom+testSize)/2 );
	
	/* set up background */
	LP->table[2].rgb.red=LP->table[2].rgb.green=LP->table[2].rgb.blue=
		LP->VBackground<<LP->leftShift;
	LoadLuminances(device,LP,1,2);	/* load background into clut */
	PmBackColor(2);				/* our background */
	EraseRect(&myRect);
	PmBackColor(1);				/* our test luminance */
	EraseOval(&TestRect);
	FlushEvents(everyEvent,0);
	readNumber=0;	/* initialize to count the number of reading */
	for(j=0;j<cycles;j++)	{
		for (k=-1;k<3;k++)	{
			if(k<0) ii=nL;
			else ii=nLrgb;
			for (i=0;i<ii;i++){
				/* set up test */
				SetRect(&labelRect,0,bottom-40,300,bottom);
				SetPort(window);
				PmBackColor(2);
				EraseRect(&labelRect);
				PmForeColor(clutSize-1);				/* black */
				MoveTo(0,bottom);
				if(k<0) DrawPrintf("%3d",V[i]);
				else DrawPrintf("%3d%4d%4d",k,nrgb[k][i][0],nrgb[k][i][1],nrgb[k][i][2]);
				SetPort(oldWindow);
				readNumber++;
				if(!automatic){
					printf("%d out of %d readings:\t",readNumber,readTotal);	
				}
				if(k<0) {
					luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,1
						,V[i],V[i],V[i]);
					luminance-=darkLuminance;
					L[i] += luminance/cycles;
				}
				else {
					luminance=GetALuminance(LP,device,frames,LuminancePerVoltage,1
						,nrgb[k][i][0],nrgb[k][i][1],nrgb[k][i][2]);
					luminance-=darkLuminance;
					Lrgb[k][i] += luminance/cycles;
				}
			}
		}
	}
	GDDisposeWindow1(window);
	SetPort(oldWindow);
	SetGDevice(oldDevice);
	
	/* polynomial and quadratic fits */
	ma=MAX_COEFFICIENTS;
	x=vector(1,nL);
	y=vector(1,nL);
	sig=vector(1,nL);
	a=vector(1,nL);
	u=matrix(1,nL,1,ma);
	v=matrix(1,ma,1,ma);
	w=vector(1,ma);
	cvm=matrix(1,ma,1,ma);
	for(i=0;i<nL;i++){
		x[i+1]=V[i];
		y[i+1]=L[i];
		sig[i+1]=10.0;
	}
	svdfit(x,y,sig,nL,a,ma,u,v,w,&chisq,fpoly);	/* ma-1th order polynomial curve fit */
	svdvar(v,ma,w,cvm);
	if(ma>MAX_COEFFICIENTS)PrintfExit("Error: too many coefficients\007\n");
	LP->coefficients=ma;
	for(i=0;i<ma;i++) LP->p[i]=a[i+1];
	printf("L(V) =");
	for(i=0;i<ma;i++) printf(" + %6g V^%d",a[i+1],i);
	printf(".  chisq %g\n",chisq);
	svdfit(x,y,sig,nL,a,3,u,v,w,&chisq,fpoly);	/* 2nd order polynomial curve fit */
	svdvar(v,3,w,cvm);
	printf("\nquadratic fit:\n\n");
	for(i=0;i<3;i++) LP->q[i]=a[i+1];
	printf("L(V) =");
	for(i=0;i<3;i++) printf(" + %g V^%d",a[i+1],i);
	printf(".  chisq %g\n",chisq);
	free_vector(x,1,nL);
	free_vector(y,1,nL);
	free_vector(sig,1,nL);
	free_vector(a,1,nL);
	free_matrix(u,1,nL,1,ma);
	free_matrix(v,1,ma,1,ma);
	free_vector(w,1,ma);
	free_matrix(cvm,1,ma,1,ma);
	e=0.0;
	for(i=0;i<nL;i++){
		f=0.0;
		VV=1.0;
		for(j=0;j<LP->coefficients;j++){
			f+=LP->p[j]*VV;
			VV*=V[i];
		}
		e+=(L[i]-f)*(L[i]-f);
	}
	LP->polynomialError=sqrt(e/nL);
	e=0.0;
	for(i=0;i<nL;i++){
		f=0.0;
		VV=1.0;
		for(j=0;j<3;j++){
			f+=LP->q[j]*VV;
			VV*=V[i];
		}
		e+=(L[i]-f)*(L[i]-f);
	}
	LP->quadraticError=sqrt(e/nL);
	
	/* power law fit */
	/* L=p[1]+Rectify(p[2]+p[3]*V)^p[4] */
	p=vector(1,4);	/* initial starting point */
	/*
		It is necessary to have a reasonable starting point or the search can get
		stuck out in the boondocks. Since the search is quite slow it is desirable to
		give it as good a starting point as possible. Therefore I make use of the
		quadratic fit to try to give it a pretty good starting point.
	*/
	/* use the quadratic fit as the starting point */
	p[1]=LP->q[0]-0.25*LP->q[1]*LP->q[1]/LP->q[2];
	p[2]=0.5*LP->q[1]/sqrt(LP->q[2]);
	p[3]=sqrt(LP->q[2]);
	p[4]=2.0;
	/*
		On second thought, try starting with a gamma of 2.3.
		Set the luminance offset p[1] to the measured dc level.
		Set the brightness p[2] so as to match the number offset in the quadratic fit.
		Set the contrast p[3] so as to fit the maximum data point.
		Set gamma p[4] to 2.3
	*/
	p[1]=L[1];
	p[2]=0.5*LP->q[1]/sqrt(LP->q[2]);
	p[4]=2.3;
	p[3]=(pow(L[nL-1]-p[1],1.0/p[4])-p[2])/V[nL-1];
	p[2]=p[3]*0.5*LP->q[1]/LP->q[2];
	p[3]=(pow(L[nL-1]-p[1],1.0/p[4])-p[2])/V[nL-1];
	xi=matrix(1,4,1,4);	/* initial set of directions */
	for(i=1;i<=4;i++)for(j=1;j<=4;j++)xi[i][j]=0.0;
	xi[1][1]=0.1;
	xi[2][2]=1.0;
	xi[3][3]=0.1;
	xi[4][4]=0.01;
	ftol=1e-2;	/* fractional tolerance on function value when done */
	printf("\npower law fit:\n\n");
	variables=4;	/* inform PowerRMSError */
	fret=PowerRMSError(p);
	printf("L(V) = %9.4f + Rectify(%9.4f + %9.4f V)^%9.4f � %9.4f\n",p[1],p[2],p[3],p[4],fret);
	powell(p,xi,4,ftol,&iter,&fret,&PowerRMSError);
	printf("L(V) = %9.4f + Rectify(%9.4f + %9.4f V)^%9.4f � %9.4f\n",p[1],p[2],p[3],p[4],fret);
	for(i=0;i<4;i++) LP->power[i]=p[i+1];
	free_vector(p,1,4);
	free_matrix(xi,1,4,1,4);
	e=0.0;
	for(i=0;i<nL;i++){
		f=LP->power[1]+LP->power[2]*V[i];
		if(f>0.0) f=LP->power[0]+pow(f,LP->power[3]);
		else f=LP->power[0];
		e+=(L[i]-f)*(L[i]-f);
	}
	LP->powerError=sqrt(e/nL);
	LP->VMin=0;					/* minimum value that can be loaded into DAC */
	LP->VMax=(1L<<LP->dacSize)-1;	/* maximum value that can be loaded into DAC */
	LP->LMin=VToL(LP,LP->VMin);	/* min luminance */
	LP->LMax=VToL(LP,LP->VMax);	/* max luminance */
	
	/* power law fit with a FIXED gamma */
	/* This is solely for study of the effects of the contrast and brightness knobs */
	/* L=p[1]+Rectify(p[2]+p[3]*V)^p[4] */
	p=vector(1,4);	/* initial starting point */
	/*
		It is necessary to have a reasonable starting point or the search can get
		stuck out in the boondocks. Since the search is quite slow it is desirable to
		give it as good a starting point as possible. 
	*/
	for(i=0;i<4;i++)p[i+1]=LP->power[i];
	p[4]=2.28;
	xi=matrix(1,3,1,3);	/* initial set of directions */
	for(i=1;i<=3;i++)for(j=1;j<=3;j++)xi[i][j]=0.0;
	xi[1][1]=0.1;
	xi[2][2]=1.0;
	xi[3][3]=0.1;
	ftol=1e-2;		/* fractional tolerance on function value when done */
	variables=3;	/* global, to inform PowerRMSError() */
	printf("\npower law fit, with fixed gamma:\n\n");
	fret=PowerRMSError(p);
	printf("L(V) = %9.4f + Rectify(%9.4f + %9.4f V)^%9.4f � %9.4f\n",p[1],p[2],p[3],p[4],fret);
	powell(p,xi,3,ftol,&iter,&fret,&PowerRMSError);
	printf("L(V) = %9.4f + Rectify(%9.4f + %9.4f V)^%9.4f � %9.4f\n",p[1],p[2],p[3],p[4],fret);
	for(i=0;i<4;i++) LP->fixedPower[i]=p[i+1];
	free_vector(p,1,4);
	free_matrix(xi,1,3,1,3);
	e=0.0;
	for(i=0;i<nL;i++){
		f=LP->fixedPower[1]+LP->fixedPower[2]*V[i];
		if(f>0.0) f=LP->fixedPower[0]+pow(f,LP->fixedPower[3]);
		else f=LP->fixedPower[0];
		e += (L[i]-f)*(L[i]-f);
	}
	LP->fixedPowerError=sqrt(e/nL);
	
	if(skipRGB){
		if(isGray){
			LP->r=LP->b=0.0;
			LP->g=1.0;
		}else{
			printf("Please enter gain of red channel   (%6.4f):",LP->r);
			gets(string);
			sscanf((char *)string,"%lf",&LP->r);
			printf("Please enter gain of green channel (%6.4f):",LP->g);
			gets(string);
			sscanf((char *)string,"%lf",&LP->g);
			printf("Please enter gain of blue channel  (%6.4f):",LP->b);
			gets(string);
			sscanf((char *)string,"%lf",&LP->b);
		}
		gain[0]=LP->r;
		gain[1]=LP->g;
		gain[2]=LP->b;
	}
	else {	
		/* calculate R,G,B gains */
		for(k=0;k<3;k++){
			gain[k]=0.0;
			for(i=0;i<nLrgb;i+=2)
				gain[k]+=(LToV(LP,Lrgb[k][i])-LToV(LP,Lrgb[k][i+1]))/(nrgb[k][i][k]-nrgb[k][i+1][k]);
			gain[k] /= nLrgb/2;
			printf("gain[%d]=%6.4f\n",k,gain[k]);
		}
	}
	printf("Sum of gains %6.4f\n",gain[0]+gain[1]+gain[2]);
	LP->r=gain[0]/(gain[0]+gain[1]+gain[2]);		/* Normalize gain */
	LP->g=gain[1]/(gain[0]+gain[1]+gain[2]);
	LP->b=gain[2]/(gain[0]+gain[1]+gain[2]);
	e=1.0-(gain[0]+gain[1]+gain[2]);
	if(skipRGB && fabs(e)<1e-6){
		printf("Please enter gain accuracy   (%6.4f):",LP->gainAccuracy);
		gets(string);
		sscanf((char *)string,"%lf",&LP->gainAccuracy);
	}
	else LP->gainAccuracy=e;
	LP->LBackground=VToL(LP,LP->VBackground);
	LP->gm=(LP->VMax/(2.0*LP->LBackground))
		*(LP->LBackground-VToL(LP,LToV(LP,LP->LBackground)-1.0));
	
	SaveData(LP,nL,V,L,nLrgb,nrgb,Lrgb,gain);
	SaveLuminanceRecord(LP);
	printf(
		"Congratulations! You've finished the luminance calibration of screen %d. The calibration results\n"
		"have been saved as CalibrateLuminance%d.data and LuminanceRecord%d.h.\n\n"
		
		"The CalibrateLuminance%d.data file is a text file suitable for graphing the monitor's gamma function.\n"
		"We recommend using CricketGraph with the CalibrateLuminance.format. The V column is the equivalent\n"
		"number loaded into the clut (V=%d*v), and the L column is the measured luminance.\n\n"
		
		"The program's results have also been stored in a C header file called LuminanceRecord%d.h that all\n"
		"your programs should read by using either #include at compile time, or ReadLuminanceRecord() at\n"
		"runtime. LuminanceRecord%d.h describes the gains of the three video pathways of your video card and\n"
		"ISR Video Attenuator, and describes the gamma function of your monitor, to allow automatic gamma\n"
		"correction later, using SetLuminance(), etc. See Pelli & Zhang (1991) Vision Research 31:1337-1360.\n"
		"Hit RETURN to exit. Good luck.\n"
		,LP->screen,LP->screen,LP->screen,LP->screen,LP->VMax,LP->screen,LP->screen
	);
}

void SaveData(luminanceRecord *LP,int nL,int V[],double L[]
	,int nLrgb,int nrgb[3][NLRGB][3],double Lrgb[3][NLRGB],double gain[3])
/* create Cricket data file, to graph the gamma function */
{
	FILE *myfile;
	int i,j,k;
	double VV,LFit;
	static char fileName[80];
	
	sprintf(fileName,"LuminanceCalibration%d.data",LP->screen);
	myfile=fopen(fileName,"w");
	SetFileInfo(fileName,'TEXT','CGRF');
	fprintf(myfile,"*\n");
	fprintf(myfile,"V\tL \tV(L) ");
	fprintf(myfile,"\tquadratic\tpolynomial\tpower ");
	fprintf(myfile,"\tL-quadratic\tL-polynomial\tL-power ");
	fprintf(myfile,"\tVr \tVg \tVb ");
	fprintf(myfile,"\tLr \tLg \tLb \tV(Lr) \tV(Lg) \tV(Lb) ");
	fprintf(myfile,"\tLR.r\tLR.g \tLR.b \tsum of gains");
	for(i=0;i<nL;i++)	{
		fprintf(myfile,"\n%d\t",V[i]);
		fprintf(myfile,"%9.4g\t",L[i]);
		fprintf(myfile,"%9.4g\t",LToV(LP,L[i]));
		/* quadratic fit */
		LFit=0.0;
		VV=1.0;
		for(j=0;j<3;j++){
			LFit += LP->q[j]*VV;
			VV *= V[i];
		}
		fprintf(myfile,"%9.4g\t",LFit);
		/* polynomial fit */
		LFit=0.0;
		VV=1.0;
		for(j=0;j<LP->coefficients;j++){
			LFit += LP->p[j]*VV;
			VV *= V[i];
		}
		fprintf(myfile,"%9.4g\t",LFit);
		/* power law fit */
		LFit=LP->power[1]+LP->power[2]*V[i];
		if(LFit>0.0) LFit=LP->power[0]+pow(LFit,LP->power[3]);
		else LFit=LP->power[0];
		fprintf(myfile,"%9.4g\t",LFit);

		/* quadratic fit */
		LFit=0.0;
		VV=1.0;
		for(j=0;j<3;j++){
			LFit += LP->q[j]*VV;
			VV *= V[i];
		}
		fprintf(myfile,"%9.4g\t",L[i]-LFit);
		/* polynomial fit */
		LFit=0.0;
		VV=1.0;
		for(j=0;j<LP->coefficients;j++){
			LFit += LP->p[j]*VV;
			VV *= V[i];
		}
		fprintf(myfile,"%9.4g\t",L[i]-LFit);
		/* power law fit */
		LFit=LP->power[1]+LP->power[2]*V[i];
		if(LFit>0.0) LFit=LP->power[0]+pow(LFit,LP->power[3]);
		else LFit=LP->power[0];
		fprintf(myfile,"%9.4g\t",L[i]-LFit);

		if(i<nLrgb) for(k=0;k<3;k++) {
			VV=LP->r*nrgb[k][i][0]+LP->g*nrgb[k][i][1]+LP->b*nrgb[k][i][2];
			fprintf(myfile,"%f\t",VV);
		}
		if(i<nLrgb) for(k=0;k<3;k++) fprintf(myfile,"%f\t",Lrgb[k][i]);
		if(i<nLrgb) for(k=0;k<3;k++) fprintf(myfile,"%f\t",LToV(LP,Lrgb[k][i]));
		if(i==0) fprintf(myfile,"%6g\t%6g\t%6g\t%6g",LP->r,LP->g,LP->b
				,gain[0]+gain[1]+gain[2]);
	}
	fclose(myfile);
}

#if 1
void SaveLuminanceRecord(luminanceRecord *LP)
/* Create C header file "LuminanceRecord?.h", where ? is the screen number. */
{
	FILE *file;
	int i;
	static char string[100],filename[100],dateString[100];
	long seconds;
	
	LP->VMin=0;
	LP->VMax=(1L<<LP->dacSize)-1;	/* maximum value that can be loaded into DAC */
	LP->LMin=VToL(LP,LP->VMin);		/* min luminance */
	LP->LMax=VToL(LP,LP->VMax);		/* max luminance */
	LP->L.exists=0;
	sprintf(filename,"LuminanceRecord%d.h",LP->screen);
	GetDateTime((void *)&seconds);
	printf("Please enter the following descriptive information for the %s file.\n",filename);

	/* time and date */
	IUDateString(seconds,longDate,(unsigned char *)string);
	PtoCstr((unsigned char *)string);
	IUTimeString(seconds,FALSE,(unsigned char *)dateString);
	PtoCstr((unsigned char *)dateString);
	sprintf(dateString,"%s %s",dateString,string);
	LP->date=dateString;
	printf("LR.date=\"%s\";\n",LP->date);	

	printf("Enter LR.id, i.e. monitor model and serial # (%s):",LP->id);
	gets(string);
	if(strlen(string)>0)strcpy(LP->id=malloc(strlen(string)+1),string);
	printf("%s\n",LP->id);

	printf("Enter LR.name, i.e. informal monitor name \n(%s):",LP->name);
	gets(string);
	if(strlen(string)>0)strcpy(LP->name=malloc(strlen(string)+1),string);
	printf("%s\n",LP->name);

	printf("Enter LR.notes, i.e. details of calibration: who & how\n(%s):\n",LP->notes);
	gets(string);
	if(strlen(string)>0) strcpy(LP->notes=malloc(strlen(string)+1),string);
	printf("%s\n",LP->notes);

	printf("Nominally, the Apple High-Res. Monochrome Monitor has 76 pixels/inch.\n");
	printf("Nominally, the Apple High-Res. RGB Monitor has 69 pixels/inch.\n");
	printf("Enter LR.dpi, i.e. pixels per inch (%.1f):",LP->dpi);
	gets(string);
	sscanf(string,"%lf",&LP->dpi);
	printf("%.1f\n",LP->dpi);

	printf("Enter LR.units luminance units (%s):",LP->units);
	gets(string);
	if(strlen(string)>0)strcpy(LP->units=malloc(strlen(string)+1),string);
	printf("%s\n",LP->units);

	// Write comment at top of file.
	file=fopen(filename,"w");
	fprintf(file,"/* This %s file is a description of a monitor produced by the CalibrateLuminance program. */\n",filename);
	fprintf(file,"/* This file may be #included in any C program. It simply fills in a luminanceRecord data structure. */\n");
	fprintf(file,"/* Or use ReadLuminanceRecord.c to read this file at runtime. */\n");
	fprintf(file,"/* The theory is described by Pelli & Zhang (1991). The data structure is defined in Luminance.h.  */\n");
	fprintf(file,"/* CalibrateLuminance and Luminance.h are part of the VideoToolbox software. */\n");
	fprintf(file,"/* Pelli & Zhang (1991) Accurate control of contrast on microcomputer displays. */\n");
	fprintf(file,"/* Vision Research, 31:1337-1360. */\n");
	fprintf(file,"/* The VideoToolbox software is available, free for research purposes, from Denis Pelli. */\n");
	fprintf(file,"/* Institute for Sensory Research, Syracuse University, Syracuse, NY 13244-5290. */\n");
	fprintf(file,"/* Caution: the screen number used here and in GetScreen Device is NOT the same as */\n");
	fprintf(file,"/* displayed by the Monitors cdev in the Control Panel. */\n");
	fclose(file);
	i=WriteLuminanceRecord(filename,LP,0);// assignNoPrintfExit
	// if(i<0)printf("WARNING: WriteLuminanceRecord returned error %d\n",i);
}
#else
{
	FILE *file;
	int i;
	static char string[100],dateString[100];
	long seconds;
	
	LP->VMin=0;
	LP->VMax=(1L<<LP->dacSize)-1;	/* maximum value that can be loaded into DAC */
	LP->LMin=VToL(LP,LP->VMin);		/* min luminance */
	LP->LMax=VToL(LP,LP->VMax);		/* max luminance */
	sprintf(string,"LuminanceRecord%d.h",LP->screen);
	GetDateTime((void *)&seconds);
	file=fopen(string,"w");
	SetFileInfo(string,'TEXT','KAHL');
	printf("Please enter the following descriptive information for the %s file.\n",string);

	fprintf(file,"/* This %s file is a description of a monitor produced by the CalibrateLuminance program. */\n",string);
	fprintf(file,"/* This file may be #included in any C program. It simply fills in a luminanceRecord data structure. */\n");
	fprintf(file,"/* Or use ReadLuminanceRecord.c to read this file at runtime. */\n");
	fprintf(file,"/* The theory is described by Pelli & Zhang (1991). The data structure is defined in Luminance.h.  */\n");
	fprintf(file,"/* CalibrateLuminance and Luminance.h are part of the VideoToolbox software. */\n");
	fprintf(file,"/* Pelli & Zhang (1991) Accurate control of contrast on microcomputer displays. */\n");
	fprintf(file,"/* Vision Research, 31:1337-1360. */\n");
	fprintf(file,"/* The VideoToolbox software is available, free for research purposes, from Denis Pelli. */\n");
	fprintf(file,"/* Institute for Sensory Research, Syracuse University, Syracuse, NY 13244-5290. */\n");
	fprintf(file,"/* Caution: the screen number used here and in GetScreen Device is NOT the same as */\n");
	fprintf(file,"/* displayed by the Monitors cdev in the Control Panel. Sorry. The most obvious difference */\n");
	fprintf(file,"/* is that GetScreenDevice always assigns 0 to the main screen, the one with the menu bar. */\n");
	fprintf(file,"LR.screen=%d;	/* device=GetScreenDevice(LR.screen); */\n"
		,LP->screen);
	/* time and date */
	IUDateString(seconds,longDate,(unsigned char *)string);
	PtoCstr((unsigned char *)string);
	IUTimeString(seconds,FALSE,(unsigned char *)dateString);
	PtoCstr((unsigned char *)dateString);
	sprintf(dateString,"%s %s",dateString,string);
	LP->date=dateString;
	fprintf(file,"LR.date=\"%s\";\n",LP->date);
	printf("LR.date=\"%s\";\n",LP->date);	

	printf("Enter LR.id, i.e. monitor model and serial # (%s):",LP->id);
	gets(string);
	if(strlen(string)>0)strcpy(LP->id=malloc(strlen(string)+1),string);
	printf("%s\n",LP->id);
	fprintf(file,"LR.id=\"%s\";\n",LP->id);

	printf("Enter LR.name, i.e. informal monitor name \n(%s):",LP->name);
	gets(string);
	if(strlen(string)>0)strcpy(LP->name=malloc(strlen(string)+1),string);
	printf("%s\n",LP->name);
	fprintf(file,"LR.name=\"%s\";\n",LP->name);

	printf("Enter LR.notes, i.e. details of calibration: who & how\n(%s):\n",LP->notes);
	gets(string);
	if(strlen(string)>0) strcpy(LP->notes=malloc(strlen(string)+1),string);
	printf("%s\n",LP->notes);
	fprintf(file,"LR.notes=\"%s\";\n",LP->notes);

	printf("Nominally, the Apple High-Res. Monochrome Monitor has 76 pixels/inch.\n");
	printf("Nominally, the Apple High-Res. RGB Monitor has 69 pixels/inch.\n");
	printf("Enter LR.dpi, i.e. pixels per inch (%.1f):",LP->dpi);
	gets(string);
	sscanf(string,"%lf",&LP->dpi);
	printf("%.1f\n",LP->dpi);
	fprintf(file,"LR.dpi=%.1f;\t/* pixels per inch */\n",LP->dpi);

	fprintf(file,"LR.Hz=%.2f;\t/* frames per second */\n",LP->Hz);

	printf("Enter LR.units luminance units (%s):",LP->units);
	gets(string);
	if(strlen(string)>0)strcpy(LP->units=malloc(strlen(string)+1),string);
	printf("%s\n",LP->units);
	fprintf(file,"LR.units=\"%s\";\n",LP->units);

	fprintf(file,"/* coefficients of polynomial fit */\n");
	fprintf(file,"LR.coefficients=%ld;	/* # of coefficients in polynomial fit */\n"
		,LP->coefficients);
	fprintf(file,"/* L(V)=p[0]+p[1]*V+p[2]*V*V+ . . . �polynomialError */\n");
	for(i=0;i<LP->coefficients;i++)	{
		fprintf(file,"LR.p[%d]=%6g;\n",i,LP->p[i]);
	}
	fprintf(file,"LR.polynomialError=%8.4f;	/* RMS error of fit */\n",LP->polynomialError);
	fprintf(file,"/* coefficients of quadratic fit */\n");
	fprintf(file,"/* L(V)=q[0]+q[1]*V+q[2]*V*V�quadraticError */\n");
	for(i=0;i<3;i++)	{
		fprintf(file,"LR.q[%d]=%6g;\n",i,LP->q[i]);
	}
	fprintf(file,"LR.quadraticError=%8.4f;	/* RMS error of fit */\n",LP->quadraticError);
	fprintf(file,"/* coefficients of power law fit */\n");
	fprintf(file,"/* L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]�powerError */\n");
	fprintf(file,"/* where Rectify(x)=x if x�0, and Rectify(x)=0 otherwise */\n");
	fprintf(file,"/* Pelli & Zhang (1991) Eqs.9&10 use symbols v=V/VMax, alpha=power[0], beta=power[1], kappa=power[2]*255, gamma=power[3] */\n");
	for(i=0;i<4;i++)	{
		fprintf(file,"LR.power[%d]=%6g;\n",i,LP->power[i]);
	}
	fprintf(file,"LR.powerError=%8.4f;	/* RMS error of fit */\n",LP->powerError);
	fprintf(file,"/* coefficients of power law fit, with fixed exponent */\n");
	fprintf(file,"/* L(V)=fixedPower[0]+Rectify(fixedPower[1]+fixedPower[2]*V)^fixedPower[3]�fixedPowerError */\n");
	for(i=0;i<4;i++)	{
		fprintf(file,"LR.fixedPower[%d]=%6g;\n",i,LP->fixedPower[i]);
	}
	fprintf(file,"LR.fixedPowerError=%8.4f;	/* RMS error of fit */\n",LP->fixedPowerError);
	fprintf(file,"LR.r=%6g;\n",LP->r);
	fprintf(file,"LR.g=%6g;\n",LP->g);
	fprintf(file,"LR.b=%6g;\n",LP->b);
	fprintf(file,"LR.gainAccuracy=%6g;\n",LP->gainAccuracy);
	fprintf(file,"LR.gm=%6g;	/* The monitor's contrast gain. */\n",LP->gm);
	fprintf(file,"LR.dacSize=%d;	/* bits */\n",LP->dacSize);
	fprintf(file,"LR.VMin=%3d;	/* minimum value that can be loaded into DAC */\n",LP->VMin);
	fprintf(file,"LR.VMax=%3d;	/* maximum value that can be loaded into DAC */\n",LP->VMax);
	fprintf(file,"LR.LMin=%8.2f;	/* luminance at VMin */\n",LP->LMin);
	fprintf(file,"LR.LMax=%8.2f;	/* luminance at VMax */\n",LP->LMax);
	fprintf(file,"LR.LBackground=%8.3f;	/* background luminance during calibration */\n",LP->LBackground);
	fprintf(file,"LR.VBackground=%d;	/* background number used during calibration */\n",LP->VBackground);
	fprintf(file,"LR.rangeSet=0;	/* indicate that range parameters have yet to be set */\n");
	fprintf(file,"LR.L.exists=0;	/* indicate that luminance table has yet to be initialized */\n");
	fclose(file);
}
#endif

FLOAT PowerRMSError(FLOAT pp[])
/*
Returns average squared error of power law fit.
pp[] contains the parameters of a power law fit to the
luminance measurements L[] made at numbers V[].
*/
{
	int i;
	FLOAT f,e,a;
	extern double L[];
	extern int V[];
	extern int nL;
	extern int variables;
	extern FLOAT *p;
	static FLOAT q[5];
	
	for(i=1;i<=variables;i++)q[i]=pp[i];	/* copy the variable parameters */
	for(i=variables+1;i<=4;i++)q[i]=p[i];	/* copy the fixed parameters */
	e=0.0;
	for(i=0;i<nL;i++){
		f=q[2]+q[3]*V[i];
		if(f>0.0) f=q[1]+pow(f,q[4]);
		else f=q[1];
		a=f-L[i];
		e += a*a;
	}
	e=sqrt(e/nL);
	return e;
}


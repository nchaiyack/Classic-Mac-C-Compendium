/*
MeasureMTF.c
Copyright � 1990-1993 Denis G. Pelli
Measures the modulation transfer function of a monitor. Uses drifting gratings,
both horizontal and vertical, of constant temporal frequency. The results
are saved in a KaleidaGraph text file, MTF?.data, where ? stands for the screen
number. The data file includes the dc and second harmonic.
HISTORY:
10/11/90	dgp	wrote it.
10/12/90	dgp	added dc and harmonics.
3/18/91		dgp	updated to work with new PlotXY, but not tested
4/15/91		dgp Check for NewPaletteManager().
8/24/91		dgp	Made compatible with THINK C 5.0.
3/10/92	dgp	include mc68881.h
8/27/92	dgp	replace SysEnvirons() by Gestalt()
10/23/92 dgp try to read latest LuminanceRecord
2/7/93	dgp	updated to use SetPixelsQuickly.
*/
#include "VideoToolbox.h"
#include <math.h>
#include "Luminance.h"
#include <Packages.h>
#if THINK_C
	#include <profile.h>	/* for timing */
	#include <console.h>
	#define PROFILE 1
#endif
#define TWO_PI (2.0*PI)
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

void MeasureMTF(void);
double MeasureContrast(GDHandle device,luminanceRecord *LP,int tPeriod,double c[10],WindowPtr plotWindow);
double saw(double x);

void main(void)
{
	Require(gestalt8BitQD);
	MeasureMTF();
}

void MeasureMTF(void)
{
	GDHandle device,oldGDHandle;
	CWindowPtr window;
	WindowPtr plotWindow,mtfWindow;
	static luminanceRecord LR;
	OSErr error;
	static char string[100],string2[100],outName[100];
	unsigned long seconds;
	FILE *outfile[2];
	int i,tPeriod,width;
	Rect r,srcRect,dstRect;
	double a,b,f,fNominal,c,L,cOut,cH[10],cV[10];
	PlotXYStyle mtfStyle[2];
	static unsigned long row[1048];	// hopefully long enough for any video device
	short oldScreen;

	#if THINK_C
		console_options.ncols=100;
		MaximizeConsoleHeight();
		printf("\n");		   			/* Initialize QuickDraw */
	#else
		InitGraf(&qd.thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
	#if PROFILE
		InitProfile(200,1);
		_profile=0;	/* disable profiling for the moment */
	#endif

	printf("Welcome to MeasureMTF.\n");
	#include "LuminanceRecord1.h"					// read at compile time
	oldScreen=LR.screen;
	if(GetScreenDevice(1)!=NULL){
		printf("Which screen would you like to calibrate (%d):",LR.screen);
		gets(string);
		sscanf(string,"%d",&LR.screen);
	}
	else LR.screen=0;
	sprintf(string,"LuminanceRecord%d.h",LR.screen);
	i=ReadLuminanceRecord(string,&LR,0);		// try to read latest luminanceRecord
	if(i<1)printf("Warning: couldn't find �%s�. Calibrating screen %d.\n"
		,string,LR.screen);
	else oldScreen=LR.screen;

	mtfStyle[0].continuing=0;
	mtfStyle[0].lineWidth=1;
	mtfStyle[0].symbolWidth=0;
	mtfStyle[0].dash[0]=0;
	mtfStyle[0].dashOffset=0;
	mtfStyle[0].color=blackColor;
	mtfStyle[1]=mtfStyle[0];
	mtfStyle[1].color=blueColor;
	
	GetDateTime(&seconds);
	sprintf(outName,"MTF%d.%s",LR.screen,DateString(seconds));
	outfile[0]=stdout;
	outfile[1]=fopen(outName,"w");
	if(outfile[1]==NULL) PrintfExit("Sorry, can't create \"%s\".\007\n",outName);
	SetFileInfo(outName,'TEXT','QKPT');	/* for Kaleidagraph */

	/* Find device corresponding to the experimental screen. */
	oldGDHandle=GetGDevice();
	device = GetScreenDevice(LR.screen);
	if(NewPaletteManager())
		SetDepth(device,8,1,1);	/* 8-bit pixels, color mode */
	window = GDOpenWindow(device);
	
	if(oldScreen==LR.screen){
		printf("Luminance was calibrated on %s\n",LR.date);
		printf("at %.2f %s by %s\n",LR.LBackground,LR.units,LR.notes);
		printf("Range is %.1f to %.1f %s.\n",LR.LMin,LR.LMax,LR.units);
	}
	L=LR.LBackground;	/* desired mean luminance */
	printf("Enter desired display luminance in cd/m^2 (%.2f):",L);
	fgets(string,sizeof(string)-1,stdin);
	sscanf(string,"%lf", &L);

	c=0.9;
	c=MIN(c,MIN((LR.LMax-L)/L,(L-LR.LMin)/L));
	printf("Contrast %.1f\n",c);
	printf("Computing sinusoidal lookup table . . .\n");
	for(i=0;i<256;i++){
		SetLuminance(device,&LR,i,L*(1.0+c*sin(TWO_PI*i/256.0)),L*(1.0-c),L*(1.0+c));
	}

	tPeriod=64;									/* frames in one temporal period */
	width=window->portRect.right;				/* pixels across screen */
	SetRect(&r,0,0,2*tPeriod,2*tPeriod);
	OffsetRect(&r,64,64);
	plotWindow=NewWindow(NULL,&r,"\pL",1,noGrowDocProc,(WindowPtr) -1L,FALSE,0L);
	SetRect(&r,0,0,192,192);
	OffsetRect(&r,640-r.right-64,64);
	mtfWindow=NewWindow(NULL,&r,"\pMTF",1,noGrowDocProc,(WindowPtr) -1L,FALSE,0L);
	MeasureContrast(device,&LR,tPeriod,cH,plotWindow);		/* measure vBlack */
	ffprintf(outfile,
		"notes" "\tcycles/pixel" "\tcycles/screen" 
		"\tHoriz. Grating Gain" "\tVert. Grating Gain" "\tVert./Horiz.");
	fprintf(outfile[0],"\n");
	fprintf(outfile[1],"\tc" "\tcH[0]" "\tcH[1]" "\tcH[2]" "\tcV[0]" "\tcV[1]" "\tcV[2]" 
		"\t(cV[0]-cH[0])/cH[0]" "\tcV[2]/(c*V/H)^2" "\t(cV[0]-cH[0])/cH[0]/(c*V/H)^2"
		"\n");
	ffprintf(outfile,"notes");	/* put some text in notes column */
	for(fNominal=0.0;;fNominal=MAX(fNominal*pow(2.0,1.0/2.0),fNominal+1.0/width)){
		f=floor(fNominal*width+0.5)/width; /* integral periods for zero mean over line */
		if(fNominal>0.5)f=0.5;
		/* horizontal grating */
		srcRect=dstRect=window->portRect;
		dstRect.left=srcRect.right=1;
		for(i=0;i<srcRect.bottom;i++){
			row[0]=128.0+127.5*saw(TWO_PI*f*i);
			SetWindowPixelsQuickly((WindowPtr)window,0,i,row,1);
		}
		/* CopyBits uses the inverse color table of the current GDevice. */
		SetGDevice(device);
		CopyBits((BitMap *)*window->portPixMap,(BitMap *)*window->portPixMap,
			&srcRect,&dstRect,srcCopy,NULL);
		SetGDevice(oldGDHandle);
		cOut=MeasureContrast(device,&LR,tPeriod,cH,plotWindow);
		ffprintf(outfile,"\t%10.5f" "\t%10.2f" "\t%12.6f",f,f*dstRect.right,cOut/c);
		/* draw the MTF */
		PlotXY(mtfWindow
			,log(f/(1./640.))/log(0.5/(1./640.)), log(cOut/c/0.3)/log(1.1/0.3)
			,&mtfStyle[0]);

		/* vertical grating */
		srcRect=dstRect=window->portRect;
		dstRect.bottom=srcRect.top=srcRect.bottom-1;
		for(i=0;i<srcRect.right;i++)row[i]=128.0+127.5*saw(TWO_PI*f*i);
		SetWindowPixelsQuickly((WindowPtr)window,0,srcRect.top,row,srcRect.right);
		SetGDevice(device);
		CopyBits((BitMap *)*window->portPixMap,(BitMap *)*window->portPixMap,
			&srcRect,&dstRect,srcCopy,NULL);
		SetGDevice(oldGDHandle);
		cOut=MeasureContrast(device,&LR,tPeriod,cV,plotWindow);
		ffprintf(outfile,"\t%20.6f" "\t%18.6f",cOut/c,cV[1]/cH[1]);
		fprintf(outfile[0],"\n");
		fprintf(outfile[1],"\t%f" "\t%f" "\t%f" "\t%f" "\t%f" "\t%f" "\t%f"
			,c,cH[0],cH[1],cH[2],cV[0],cV[1],cV[2]);
		a=(cV[0]-cH[0])/cH[0];	/* dc error, expressed as a contrast */
		b=cV[1]/cH[1];			/* contrast gain of video amplifier */
		fprintf(outfile[1],"\t%f" "\t%f" "\t%f" "\n"
			,a,cV[2]/(c*b*c*b),a/(c*b*c*b));
		/* draw the MTF */
		PlotXY(mtfWindow
			,log(f/(1./640.))/log(0.5/(1./640.)), log(cOut/c/0.3)/log(1.1/0.3)
			,&mtfStyle[1]);
		
		if(f==0.5)break;
	}
	
	/* print notes */
	IUDateString(seconds,longDate,(unsigned char *)string);
	PtoCstr((unsigned char *)string);
	IUTimeString(seconds,FALSE,(unsigned char *)string2);
	PtoCstr((unsigned char *)string2);
	ffprintf(outfile,"%s %s\n",string2,string);
	ffprintf(outfile,"%.1f Hz\n",1.0/(tPeriod*0.015));
	ffprintf(outfile,"%.2f cd/m^2\n",L);
	ffprintf(outfile,"%.3f contrast\n",c);
	ffprintf(outfile,"\n" "screen %d\n",LR.screen);
	ffprintf(outfile,"\"%s\" monitor %s\n",LR.name,LR.id);
	ffprintf(outfile,"on %s\n",LR.date);
	ffprintf(outfile,"by %s\n",LR.notes);
	ffprintf(outfile,"at %.2f %s\n",LR.LBackground,LR.units);

	GDDisposeWindow(device,window);
	if(outfile[1] != NULL)fclose(outfile[1]);
	printf("The sum of the times for GetVoltage() and LoadLuminances() should\n"
		"be about one frame, 15 ms. If it's longer, e.g. 30 ms, you should reduce\n"
		"the value of \"frames\" in MeasureContrast().\n");
	DisposeWindow(plotWindow);
	DisposeWindow(mtfWindow);
}


double MeasureContrast(GDHandle device,luminanceRecord *LP
	,int tPeriod,double c[10],WindowPtr plotWindow)
/*
Measures the contrast of a drifting grating of unknown phase.
The temporal period is tPeriod frames. The c[] array is
filled with the amplitude spectrum, where c[i] is the contrast of the i-th harmonic,
except that c[0], which would always be 1, instead is set to the dc level.
*/
{
	double v[256];
	double p[10];
	register int i,j;
	static ColorSpec doubleTable[512];
	static double sinTable[256],cosTable[256];
	static int tablePeriod=0;
	register double a,b;
	double frequency=2000.,gain=100.;
	double frames=0.6;	/* How long the A/D should spend sampling the photometer */
	long n;
	static double vBlack=0.0;
	static int blackSet=0;
	char string[80];
	WindowPtr oldPort;
	int v0,ip;

	if(tPeriod>256){
		printf("Warning. Reducing tPeriod to 256.\n");
		tPeriod=256;
	}
	if(tablePeriod != tPeriod){
		for(i=0;i<tPeriod;i++){
			sinTable[i]=sin(i*TWO_PI/tPeriod);
			cosTable[i]=cos(i*TWO_PI/tPeriod);
		}
		tablePeriod=tPeriod;
	}
	
	n=(long)floor(0.5+frequency*0.015*frames);

	RemeasureContrast:
	if(!blackSet){
		doubleTable[0].rgb.red=doubleTable[0].rgb.green=doubleTable[0].rgb.blue=0;
		for(i=0;i<256;i++)doubleTable[i]=doubleTable[i+256]=doubleTable[0];
		LoadLuminances(device
			,(luminanceRecord *)(doubleTable+(i%tPeriod)*(256/tPeriod)),0,255);
		printf("Please block all light to set black. Hit cr when ready:");
		gets(string);
		vBlack=0.0;
	}
	else for(i=0;i<256;i++)doubleTable[i]=doubleTable[i+256]=LP->table[i];

	/* warm up for one period before collecting data */
	SetPriority(7);
	for(i=0;i<tPeriod;i++){
		LoadLuminances(device
			,(luminanceRecord *)(doubleTable+(i%tPeriod)*(256/tPeriod)),0,255);
		GetVoltage(1,&gain,&frequency,n,NULL);
		v[i]=0.0;
	}
	_profile=1;
	for(i=0;i<tPeriod*4;i++){
		LoadLuminances(device
			,(luminanceRecord *)(doubleTable+(i%tPeriod)*(256/tPeriod)),0,255);
		v[i%tPeriod]+=GetVoltage(1,&gain,&frequency,n,NULL);
	}
	_profile=0;
	SetPriority(0);
	a=0.0;
	for(i=0;i<tPeriod;i++) a+=v[i];
	c[0]=a/tPeriod-vBlack;
	if(!blackSet){
		vBlack=c[0];
		blackSet=1;
		printf("Black %g mV\n",vBlack*1000.0);
		printf("Now please remove light block. Hit cr when ready:");
		gets(string);
		goto RemeasureContrast;
	}
	for(j=1;j<10;j++){
		a=b=0.0;
		for(i=0;i<tPeriod;i++){
			a+=sinTable[i*j%tPeriod]*v[i];
			b+=cosTable[i*j%tPeriod]*v[i];
		}
		c[j]=2.0*sqrt(a*a+b*b)/tPeriod/c[0];
		p[j]=atan2(a,b);
	}
	/* Show one period of raw data, fundamental, raw minus fundamental, 2nd harmonic */
	GetPort(&oldPort);
	SetPort(plotWindow);
	BringToFront(plotWindow);
	EraseRect(&plotWindow->portRect);
	v0=plotWindow->portRect.bottom/4;
	SetOrigin(0,-2*v0);
	MoveTo(0,-v0*(v[0]-vBlack)/c[0]);
	for(i=1;i<tPeriod;i++)LineTo(i,-v0*(v[i]-vBlack)/c[0]);
	SetOrigin(0,-4*v0);
	j=1;
	ip=tPeriod*(1.0-p[j]/TWO_PI);
	a=-v0*(0.0+c[j]*cosTable[(i*j+ip)%tPeriod]);
	MoveTo(0,-v0*(v[0]-vBlack)/c[0]-a);
	for(i=1;i<tPeriod;i++){
		a=-v0*(0.0+c[j]*cosTable[(i*j+ip)%tPeriod]);
		LineTo(i,-v0*(v[i]-vBlack)/c[0]-a);
	}
	ForeColor(blueColor);
	for(j=1;j<3;j++){
		SetOrigin(-tPeriod,-2*v0*j);
		ip=tPeriod*(1.0-p[j]/TWO_PI);
		i=0;
		a=-v0*(1.0+c[j]*cosTable[(i*j+ip)%tPeriod]);
		MoveTo(0,a);
		for(i=1;i<tPeriod;i++){
			a=-v0*(1.0+c[j]*cosTable[(i*j+ip)%tPeriod]);
			LineTo(i,a);
		}
	}
	ForeColor(blackColor);
	SetOrigin(0,0);
	SetPort(oldPort);
	return c[1];
}


double saw(double x)
/* returns sawtooth function with same phase, amplitude, and symmetry as sin() */
{
	x/=TWO_PI;
	x-=0.5;
	x-=floor(x);
	return 2.0*x-1.0;
}

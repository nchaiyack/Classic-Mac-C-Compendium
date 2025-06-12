/*
CheckContrast.c

CheckContrast does a rigorous testing of the contrasts produced on your monitor.
It was developed in order to TEST the theory behind the ISR Video Attenuator during
preparation of the Pelli & Zhang (1991) paper. It requires a 12-bit Analog to 
Digital Converter (ADC). At present the code (in GetVoltage.c) assumes the Data 
Translation Forerunner card in slot e.

The results including both the measured contrast errors and the theoretical error bounds
are stored in a CricketGraph data file, CheckContrast.data, which may be plotted
using the CricketGraph format file CheckContrast.format. The resulting figure
is comparable to one in Pelli and Zhang (1991).

Getting all the errors within the bounds was tough. Everything has to work right, 
including the luminance calibration and the contrast calibration done here. We now
feel that the residual error is due to, in order of decreasing size: 
1. random measurement error in the contrast calibration, which can be reduced by 
averaging more frames
2. drifting of the monitor gamma function between the luminance and contrast 
calibrations.
3. errors of the dac (seem to be within manufacturer spec.)

You must supply a LuminanceRecord?.h, (where ? designates which screen, e.g. 1),
as produced by CalibrateLuminance, for your setup, and recompile CheckContrast 
to #include that file.

D.G. Pelli and L. Zhang (1991) Accurate control of contrast on microcomputer displays. 
Vision Research, 31:1337-1360.

HISTORY

Lan ?		Change from RandomLetter.c to measure the real contrast display in the
			testing screen by setting up a TestRect painted by pmForeColor.
Lan 8/4/89	add GetVoltage from Preeti & Denis to measure the real contrast display by ForeRunner
Lan 9/6/89	change contrast to Michelson contrast(set MICHELSON=1), mean luminance is set to
			the mid point of luminance.
9/11/89 dgp	Increased the delay before each voltage measurement up to 40 ticks, and
			then synched to monitor. This gives the photometer more time to settle, and
			synchs the measurement to the display frame phase. The result is that the
			calibrations are now in spec. Yay!
9/19/89 Lan Check contrast at different region of screen: divide the screen into 12
			regions, namely, row 1-3, column 1-4.
10/30/89 dgp Fix bug in computation of tolerance bounds for CheckContrast.data which
			would occasionally introduce a spurious bound at a discontinuity.
10/30/89 dgp Cleaned up documentation.
2/12/90	dgp	Incorporated old bug fixes 9/25/89 that by mistake were left out of this
			version: Increased number of digits in printouts, to avoid contrasts becoming equal,
			which confuses CricketGraph. Fixed minor bug in code that detects 
			discontinuity in tolerance. Previously, a reduction in contrast ratio which
			wasn't a discontinuity resulted in one of the tolerances never being 
			initialized and coming out as zero. 
10/10/90 dgp Renamed the program CheckContrast from "CalibrateContrast".
10/17/90 dgp Removed unused variables.
2/16/91	 dgp Now check for fpu and color quickdraw.
8/24/91	dgp	Made compatible with THINK C 5.0.
3/10/92	dgp	include mc68881.h
8/27/92	dgp	replace SysEnvirons() by Gestalt()
2/23/93	dgp	use new GDOpenWindow1 and GDDisposeWindow1.
12/19/93 dgp use ReadLuminanceRecord. Check for success.
*/
#include "VideoToolbox.h"
#include <math.h>
#include "Luminance.h"
double contrastRatio(double t, double L1, double L2);

#define MICHELSON	1		/* 1=use Michelson contrast, 0=use Weber contrast */
#define MEASURE	1			/* 1=use ADC, 0=skip measurements to debug */
#define	NCONTRAST	20		/* number of contrasts to test */
#define TRSIZE		160		/* size of test patch, in pixels */
#define LUMINANCE	85.0	/* background luminance in cd/m^2 */
#define NFRAME		200		/* number of total frames used in VoltsDuringFrame */
#define NSAMPLE		2		/* number of repetitions */
							/* NFRAME/NSAMPLE must be int. */
void CheckContrast(void);

void main(void)
{
	Require(gestalt8BitQD);
	CheckContrast();
}

void CheckContrast(void)
{
	register short i;
	int j;
	Rect myRect,TestRect;
	WindowPtr window = NULL,oldWindow=NULL;
	double L,L1,L2;
	double contrast,theContrast[NCONTRAST];
	GDHandle device=NULL,oldGDHandle=NULL;
	luminanceRecord LR;
	static double measuredLTest[NCONTRAST],measuredLBackground[NCONTRAST],measuredContrast[NCONTRAST];
	static double tolerance[NCONTRAST],LTest[NCONTRAST],LBackground[NCONTRAST];
	long int finalTicks;
	FILE *myfile;
	double LuminancePerVolt=1000.0; /* cd/m^2 */
	double e,t;
	double th,tl,ch,cl,r,rh,rl;
	
	SetApplLimit(GetApplLimit()-50000);
	GetPort(&oldWindow);
	printf("\n");	/* init QuickDraw automatically */

	/* parameters of the screen calibration */	
	i=ReadLuminanceRecord("LuminanceRecord1.h",&LR,0);
	if(i<=0)PrintfExit("CheckContrast: couldn't find the LuminanceRecord file.\n");
	printf("Using luminance calibration with notes:\n%s\n",LR.notes);
	
	/* Find device corresponding to the experimental screen. */
	oldGDHandle = GetGDevice();
	device = GetScreenDevice(LR.screen);
	if(device==NULL)PrintfExit("CheckContrast: sorry the calibrated device is not"
		" installed\n");
	
	/* Open window once, and keep open for whole experiment. */
	window = GDOpenWindow1(device);
	
	/* create a testing Rect */
	SetGDevice(device);
	SetPort(window);
	myRect = window->portRect;
	
	SetRect(&TestRect, (myRect.right+myRect.left-TRSIZE)/2, (myRect.top+myRect.bottom-TRSIZE)/2, 
		(myRect.right+myRect.left+TRSIZE)/2, (myRect.top+myRect.bottom+TRSIZE)/2 );	
	/*SetRect(&TestRect, (myRect.right-TRSIZE),(myRect.bottom-TRSIZE),myRect.right, myRect.bottom);	
	SetRect(&TestRect, myRect.left, myRect.top,(myRect.left+TRSIZE), (myRect.top+TRSIZE));
	SetRect(&TestRect, (myRect.right-TRSIZE),myRect.top,myRect.right,(myRect.top+TRSIZE));
	SetRect(&TestRect, myRect.left,(myRect.bottom-TRSIZE),(myRect.left+TRSIZE), myRect.bottom);
	
	SetRect(&TestRect, (myRect.left+(COLUMN-1)*TRSIZE), (myRect.top+(ROW-1)*TRSIZE)
						,(myRect.left+COLUMN*TRSIZE), (myRect.top+ROW*TRSIZE));*/

	contrast=1.0;
	PmBackColor(2);			/* white background */
	EraseRect(&myRect);
	PmBackColor(1);
	EraseRect(&TestRect);	/* test contrast */
	SetPort(oldWindow);
	SetGDevice(oldGDHandle);
	for (i=0; i<NCONTRAST;i++) {
		#if MICHELSON
			L=(LR.LMax+LR.LMin)/2.0;
			L=LR.LBackground;	/* use same background as in CalibrateLuminance */
			L1=L*(1.0-contrast);
			L2=L*(1.0+contrast);
		#else
			L=LUMINANCE;
			if(L>LR.LMax)L=LR.LMax;
			L1=(1.0-contrast)*L;
			L2=L;
		#endif
		SetLuminance(device,&LR,2,L,L1,L2);			/* set background luminance */
		tolerance[i]=SetLuminance(NULL,&LR,1,L1,L1,L2);	/* accuracy+precision */
		tolerance[i]=LR.tolerance;						/* accuracy */
		SetLuminance(NULL,&LR,1,L1,L1,L2);
		LTest[i]=GetLuminance(NULL,&LR,1);
		SetLuminance(NULL,&LR,1,L2,L1,L2);
		LBackground[i]=GetLuminance(NULL,&LR,1);
		#if MICHELSON
			theContrast[i]=(LBackground[i]-LTest[i])/(LBackground[i]+LTest[i]);
		#else
			theContrast[i]=(LBackground[i]-LTest[i])/LBackground[i];
		#endif
		printf("\nContrast %d, requested %.10f, nominally %.10f\n",i,contrast,theContrast[i]);
		measuredLTest[i]=0.0;
		measuredLBackground[i]=0.0;
		
		for(j=0;j<NSAMPLE;j++)	{
			SetLuminance(device,&LR,1,L1,L1,L2);
			#if MEASURE	
				/* measure voltage */
				if(theContrast[i]>0.1)Delay(60L,&finalTicks);
				if(theContrast[i]>0.01)Delay(6L,&finalTicks);
				LoadLuminances(device,&LR,1,2);	/* synch to monitor */
				measuredLTest[i]+=VoltsDuringFrame(NFRAME/NSAMPLE)*LuminancePerVolt/NSAMPLE;
			#endif	
			SetLuminance(device,&LR,1,L2,L1,L2);
			#if MEASURE
				if(theContrast[i]>0.1)Delay(60L,&finalTicks);
				if(theContrast[i]>0.01)Delay(6L,&finalTicks);
				LoadLuminances(device,&LR,1,2);	/* synch to monitor */
				measuredLBackground[i]+=VoltsDuringFrame(NFRAME/NSAMPLE)*LuminancePerVolt/NSAMPLE;
			#endif			
		}
		#if MICHELSON
			measuredContrast[i]=(measuredLBackground[i]-measuredLTest[i])/(measuredLBackground[i]+measuredLTest[i]);
			e=tolerance[i]/(measuredLBackground[i]+measuredLTest[i]);
		#else
			measuredContrast[i]=(measuredLBackground[i]-measuredLTest[i])/measuredLBackground[i];
			e=tolerance[i]/measuredLBackground[i];
		#endif
		printf("LTest %6.2f %6.2f LBackground %6.2f %6.2f tolerance %6.2f\n",
			LTest[i],measuredLTest[i],LBackground[i],measuredLBackground[i],tolerance[i]);
		printf("contrast %5.5f %5.5f",theContrast[i],measuredContrast[i]);
		printf(" error/tolerance %10.5f\n",(measuredContrast[i]-theContrast[i])/e);
		contrast /= sqrt(2.0);
		FlushEvents(everyEvent,0);
	}
	SetPort(oldWindow);
	SetGDevice(oldGDHandle);
	printf("\007Measurements are done. Now saving results to disk\n");
	/* open and write into file */
	myfile=fopen("CheckContrast.data","w");
	SetFileInfo("CheckContrast.data",'TEXT','CGRF');
	fprintf(myfile,"*\n");
	fprintf(myfile,
		"C\tmC\tmC-C"
		"\t+e"
		"\t-e"
		"\t|mC-C|/C"
		"\te/C"
		"\terror/tolerance"
		"\tmC/C"
		"\t(C+e)/C"
		"\t(C-e)/C"
		"\tLTest\tmLTest\tmLTest-LTest"
		"\tLBackground\tmLBackground\tmLBackground-LBackground"
		"\ttolerance\n");
	for(i=0;i<NCONTRAST;i++){
		/* after first iteration, compute contrast ratio errors */
		if(i>0) {
			rl=contrastRatio(tolerance[i-1],LTest[i-1],LBackground[i-1]);
			rh=contrastRatio(tolerance[i],LTest[i],LBackground[i]);
		}
		/* if there's a discontinuity in tolerance, then insert extra pair of records */
		if(i>0 && rh<rl){
			/* find discontinuity */
			cl=theContrast[i-1];
			ch=theContrast[i];
			tl=tolerance[i-1];
			th=tolerance[i];
			for(j=0;j<15;j++){
				if((cl-ch)<0.0001) break;	/* Cricket Graph gets confused if (cl-ch) is too small */
				contrast=(cl+ch)/2.0;
				#if MICHELSON
					L=(LR.LMax+LR.LMin)/2.0;
					L=LR.LBackground;	/* use same background as in CalibrateLuminance */
					L1=L*(1.0-contrast);
					L2=L*(1.0+contrast);
				#else
					L=LUMINANCE;
					if(L>LR.LMax)L=LR.LMax;
					L1=(1.0-contrast)*L;
					L2=L;
				#endif
				t=SetLuminance(NULL,&LR,1,L1,L1,L2);	/* accuracy+precision */
				t=LR.tolerance;							/* accuracy */
				r=contrastRatio(t,L1,L2);
				if(r>=rl){
					rl=r;
					tl=t;
					cl=contrast;
				}
				else {
					rh=r;
					th=t;
					ch=contrast;
				}
			}
			/* make sure there IS a discontinuity */
			if(rh*1.001<rl){
				/* print out one record at lower bound w/o data */
				t=tl;
				contrast=cl;
				#if MICHELSON
					e=t/(L1+L2);
				#else
					e=t/L2;
				#endif
				#if MICHELSON
					L=(LR.LMax+LR.LMin)/2.0;
					L=LR.LBackground;	/* use same background as in CalibrateLuminance */
					L1=L*(1.0-contrast);
					L2=L*(1.0+contrast);
				#else
					L=LUMINANCE;
					if(L>LR.LMax)L=LR.LMax;
					L1=(1.0-contrast)*L;
					L2=L;
				#endif
				fprintf(myfile,"%.10f\t\t\t%.10f\t%.10f\t"
					,contrast
					,e
					,-e
				);
				fprintf(myfile,"\t%.10f\t\t\t%.10f\t%.10f\t%.10f\t\t\t%.10f\t\t\t%.10f\n"
					,e/contrast
					,(contrast+e)/contrast
					,(contrast-e)/contrast
					,L1
					,L2
					,t
				);
				/* print out one record at upper bound w/o data */
				t=th;
				contrast=ch;
				#if MICHELSON
					e=t/(L1+L2);
				#else
					e=t/L2;
				#endif
				#if MICHELSON
					L=(LR.LMax+LR.LMin)/2.0;
					L=LR.LBackground;	/* use same background as in CalibrateLuminance */
					L1=L*(1.0-contrast);
					L2=L*(1.0+contrast);
				#else
					L=LUMINANCE;
					if(L>LR.LMax)L=LR.LMax;
					L1=(1.0-contrast)*L;
					L2=L;
				#endif
				fprintf(myfile,"%.10f\t\t\t%.10f\t%.10f\t"
					,contrast
					,e
					,-e
				);
				fprintf(myfile,"\t%.10f\t\t\t%.10f\t%.10f\t%.10f\t\t\t%.10f\t\t\t%.10f\n"
					,e/contrast
					,(contrast+e)/contrast
					,(contrast-e)/contrast
					,L1
					,L2
					,t
				);
			}
		}
		/* print out one record of data */
		#if MICHELSON
			e=tolerance[i]/(LBackground[i]+LTest[i]);
		#else
			e=tolerance[i]/LBackground[i];
		#endif
		fprintf(myfile,"%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t"
			,theContrast[i]
			,measuredContrast[i]
			,measuredContrast[i]-theContrast[i]
			,e
			,-e
		);
		fprintf(myfile,"%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\n"
			,fabs(measuredContrast[i]-theContrast[i])/theContrast[i]
			,e/theContrast[i]
			,(measuredContrast[i]-theContrast[i])/e
			,measuredContrast[i]/theContrast[i]
			,(theContrast[i]+e)/theContrast[i]
			,(theContrast[i]-e)/theContrast[i]
			,LTest[i]
			,measuredLTest[i]
			,measuredLTest[i]-LTest[i]
			,LBackground[i]
			,measuredLBackground[i]
			,measuredLBackground[i]-LBackground[i]
			,tolerance[i]
		);
	}
	fclose(myfile);
	SetGDevice(oldGDHandle);
	GDDisposeWindow1(window);
}

double contrastRatio(double t, double L1, double L2)
{
double e,contrast;
		#if MICHELSON
			e=t/(L1+L2);
			contrast=fabs(L1-L2)/(L1+L2);
		#else
			e=t/L2;
			contrast=(L2-L1)/L2;
		#endif
		return (contrast+e)/contrast;
}
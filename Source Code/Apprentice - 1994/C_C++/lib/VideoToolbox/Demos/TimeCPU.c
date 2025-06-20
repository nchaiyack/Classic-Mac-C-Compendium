/*
TimeCPU.c
Denis G. Pelli, 1991-1993
This routine uses Timer.c to measure the timing of basic CPU
operations and several random number generators. The Fixed data type is
predefined by Apple as a long (i.e. 32 bits) with an assumed decimal point in
the middle.

The access to video memory overwrites a small part of your main screen, the
first 40 bytes. This will be in the upper left hand corner of your display, and
will usually be barely noticeable. I like seeing that, as it confirms that the
program really is accessing the video memory.

This program requests a memory partition of 900K. It will run in less, but will
then do fewer iterations.

The timing seems to be very accurate. It ought to be as accurate as the
frequency of the oscillator in the VIA chip. However, I haven't checked the
timing against a known standard.

HISTORY:
1/91	dgp	wrote it
2/16/91	dgp	added fpu test, to fail gracefully if compiled with FPU support, but FPU
			is not present.
3/4/91	dgp	added timing of random number generators
8/6/91	dgp	added timing of RandFill.
8/24/91	dgp	Made compatible with THINK C 5.0.
1/25/92	dgp	Calibrate and correct for the slowness of TimeIt(). 
			Measure and subtract off the small loop overhead.
			Identify machine and compiler.
			Automatically append results to TimeCPU.data file.
1/29/92	dgp	Time move from memory to video memory, for showing movies.
			Added transcendental functions since Radius 8881 init and System 7.01
			speed them up dramatically and the Quadra is reputed to
			do them very slowly.
1/30/92	dgp	Access video memory only in 32-bit mode, to avoid crashes.
3/10/92	dgp	include mc68881.h
8/19/92	dgp	time the 68881 instructions _sin, _sqrt, _exp, _log
			Use new Timer.c instead of old TimeIt.c
8/28/92	dgp	updated to use new reentrant Timer.c
11/18/92 dgp renamed output file to �TimeCPU results�
1/11/93	dgp	check for presence of 68020. Put Gestalt tests in main, without
			any fpu usage, since program was crashing when fpu was absent
			before getting to the fpu test. (Supposedly that was fixed
			in THINK C 5. Oh well.)
2/7/93	dgp	added timing of SetPixelsQuickly().
7/9/93	dgp check for 32-bit addressing capability.
3/13/94	dgp	added timing of short arithmetic, and put conditionals around each section.
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <math.h>
#include <SANE.h>		/* for elems68k() & FRANDX */
#include "mc68881.h"

void TimeCPU(void);

void main(void)
{
	long value;

	Require(gestaltOriginalQD);
	Gestalt(gestaltTimeMgrVersion,&value);
	if(value<gestaltRevisedTimeMgr)
		PrintfExit("Sorry, your System is too old; I need at least \n"
			"the Revised Time Manager.\n");
	TimeCPU();
}

void TimeCPU(void)
{
	int error;
	long n,value,quickDraw;
	short i;
	register long *paL,*pbL,iL,jL,kL,mL;
	long nL,iiL;
	double x,y,z;
	Fixed xF,yF,zF;
	void *buffer,*buffer2;
	char Buffer[32];
	FILE *o[2],*dataFile;
	GDHandle device;
	long startTicks,finalTicks;
	double s,s0,f,overhead;
	Timer *timer;
	Boolean can32;
		
	printf("Welcome to TimeCPU.\n");
	assert(StackSpace()>4000);
	value=0;
	error=Gestalt(gestaltAddressingModeAttr,&value);
	can32=!error && (value&(1<<gestalt32BitCapable));
	MaximizeConsoleHeight();
	timer=NewTimer();
	Delay(1,&startTicks);
	StartTimer(timer);
	Delay(60,&finalTicks);
	s=StopTimerSecs(timer);
	printf("The VBL tick frequency is %.2f Hz.\n",(finalTicks-startTicks)/s);
	o[0]=stdout;
	o[1]=dataFile=fopen("TimeCPU results","a");	/* Append to data file */
	if(dataFile!=NULL){
		printf("Key results will be appended to �TimeCPU results� file.\n\n");
		SetFileInfo("TimeCPU results",'TEXT','ttxt');
	}
	else printf("Could not open �TimeCPU results� file\n\n");
	ffprintf(o,"\n%s\n",BreakLines(IdentifyMachine(),80));
	ffprintf(o,"%s\n\n",BreakLines(IdentifyCompiler(),80));
	ffprintf(o,"      Time	Operation\n");
	srand(clock());
	y=sqrt(2.0);
	z=sqrt(3.0);
	kL=y*1000.;
	mL=z*10.;
	RandFill(Buffer,sizeof(Buffer));
		
	n=10000;	/* long arithmetic takes a fraction of a microsecond */
	overhead=0.0;
	StartTimer(timer);
	for(iL=n/10;iL>0;iL--);
	overhead=StopTimerSecs(timer)/n-overhead;	// the loop overhead per operation

	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw>=gestalt8BitQD){
		unsigned long row[100],row2[100];
		int rowLength=100,clutSize;
		
		n/=10;
		device=GetMainDevice();
		clutSize=GDClutSize(device);
		for(i=0;i<rowLength;i++)row[i]=nrand(clutSize);
		StartTimer(timer);
		for(iL=n;iL>0;iL--){
			SetDevicePixelsQuickly(device,0,0,row,rowLength);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.0f	�s	SetPixelsQuickly(,,,,%d);// %d-bit pixels\n",s*1e6
			,rowLength,(**(**device).gdPMap).pixelSize);
		n*=10;

		n/=10;
		StartTimer(timer);
		for(iL=n;iL>0;iL--){
			GetDevicePixelsQuickly(device,0,0,row2,rowLength);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.0f	�s	GetPixelsQuickly(,,,,%d);// %d-bit pixels\n",s*1e6
			,rowLength,(**(**device).gdPMap).pixelSize);
		n*=10;
		for(i=0;i<rowLength;i++)if(row2[i]!=row[i])printf("Pixel wrote %ld != read %ld\n"
			,row[i],row2[i]);
	}
	
	for(;n>0;){
		buffer=NewPtr(sizeof(long)*n);
		if(buffer!=NULL){
			buffer2=NewPtr(sizeof(long)*n);
			if(buffer2!=NULL)break;
			DisposPtr(buffer);
		}
		n/=2;
		printf("Reducing iterations to %ld to fit in available memory.\n",n);
	}
	assert(buffer!=NULL && buffer2!=NULL);
	paL=(long *)buffer;
	pbL=(long *)buffer2;
	StartTimer(timer);
	for(iL=n/10;iL>0;iL--){
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
		*paL++=*pbL++;
	}
	s=StopTimerSecs(timer)/n-overhead;
	ffprintf(o,"%7.2f	�s	*paL++=*pbL++;		// long, memory to memory\n",s*1e6);

	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw>=gestalt8BitQD){
		device=GetMainDevice();
		if(device!=NULL){
			paL=(long *)(**(**device).gdPMap).baseAddr;
			if(paL!=NULL){
				char mode=true32b;
				long t0;
				long *pSave=paL;
				
				StartTimer(timer);
				if(can32)SwapMMUMode(&mode);
				for(iL=n/10;iL>0;iL--) paL-=10;
				if(can32)SwapMMUMode(&mode);
				s0=StopTimerSecs(timer)/n-overhead;
				paL=pSave;
				StartTimer(timer);
				if(can32)SwapMMUMode(&mode);
				for(iL=n/10;iL>0;iL--){
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					*paL++=*pbL++;
					paL-=10;
				}
				if(can32)SwapMMUMode(&mode);
				s=StopTimerSecs(timer)/n-overhead;
				s-=s0;									// remove time for the paL-=10;
				ffprintf(o,"%7.2f	�s	*paL++=*pbL++;		// long, memory to video memory\n",s*1e6);
			}
		}
	}
	DisposPtr(buffer);
	DisposPtr(buffer2);

	if(1){			// time long arithmetic
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
			jL=kL;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jL=kL;			// long, register to register\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
			jL=kL>>1;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jL=kL>>1;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
			jL=kL+mL;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jL=kL+mL;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
			jL=kL-mL;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jL=kL-mL;\n",s*1e6);
	
		n/=10;			/* all other operations take at least several microseconds */
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
			jL=kL*mL;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	jL=kL*mL;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
			jL=kL/mL;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	jL=kL/mL;\n",s*1e6);
	}
	if(1){				// time short arithmetic
		register short jH,kH=1234,mH=5678;
		
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
			jH=kH;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jH=kH;			// short, register to register\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
			jH=kH>>1;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jH=kH>>1;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
			jH=kH+mH;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jH=kH+mH;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
			jH=kH-mH;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.2f	�s	jH=kH-mH;\n",s*1e6);
	
		n/=10;			/* all other operations take at least several microseconds */
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
			jH=kH*mH;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	jH=kH*mH;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
			jH=kH/mH;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	jH=kH/mH;\n",s*1e6);
	}

	if(1){		// time double arithmetic
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=y;
			x=y;
			x=y;
			x=y;
			x=y;
			x=y;
			x=y;
			x=y;
			x=y;
			x=y;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=y;			// double\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
			x=y+z;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=y+z;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
			x=y-z;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=y-z;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
			x=y*z;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=y*z;\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
			x=y/z;
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=y/z;\n",s*1e6);
	}
	
	if(1){				// time transcendental functions
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
			x=sin(y);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=sin(y);\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
			x=sqrt(y);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=sqrt(y);\n",s*1e6);
	
		n/=100;
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
			x=log(y);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=log(y);\n",s*1e6);
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
			x=exp(y);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=exp(y);\n",s*1e6);
		n*=100;
	
		#if mc68881
			StartTimer(timer);
			for(iL=n/10;iL>0;iL--){
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
				x=_sin(y);
			}
			s=StopTimerSecs(timer)/n-overhead;
			ffprintf(o,"%7.1f	�s	x=_sin(y);\n",s*1e6);
		
			StartTimer(timer);
			for(iL=n/10;iL>0;iL--){
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
				x=_sqrt(y);
			}
			s=StopTimerSecs(timer)/n-overhead;
			ffprintf(o,"%7.1f	�s	x=_sqrt(y);\n",s*1e6);
		
			StartTimer(timer);
			for(iL=n/10;iL>0;iL--){
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
				x=_log(y);
			}
			s=StopTimerSecs(timer)/n-overhead;
			ffprintf(o,"%7.1f	�s	x=_log(y);\n",s*1e6);
			
			StartTimer(timer);
			for(iL=n/10;iL>0;iL--){
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
				x=_exp(y);
			}
			s=StopTimerSecs(timer)/n-overhead;
			ffprintf(o,"%7.1f	�s	x=_exp(y);\n",s*1e6);
		#endif
	}

	o[1]=NULL;		/* that's all we want to save in �TimeCPU results� */

	if(1){					// time Fixed
		yF=zF=0x12341234;
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
			xF=FixMul(yF,zF);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	xF=FixMul(yF,zF);	// Fixed\n",s*1e6);
	
		yF=(long)(PI*256);
		zF=(long)(1.1*256);
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
			xF=FixDiv(yF,zF);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	xF=FixDiv(yF,zF);\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
			xF=FixRatio(123,1234);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	xF=FixRatio(123,1234);\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
			xF=DoubleToFix(y);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	xF=DoubleToFix(y);\n",s*1e6);
	
		xF=(long)(PI*256);
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
			x=FixToDouble(xF);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	x=FixToDouble(xF);\n",s*1e6);
	}
	
	if(1){				// time rand()
		n*=10;
		for(;n>0;n/=2){
			buffer=NewPtr(n);
			if(buffer!=NULL)break;
		}
		StartTimer(timer);
		s=StopTimerSecs(timer);
		StartTimer(timer);
		RandFill(buffer,n);
		s=StopTimerSecs(timer) - s;
		DisposPtr(buffer);
		ffprintf(o,"%7.1f	ms	RandFill(,%ld);	// i.e. %4.1f �s/byte\n"
			,s*1e3,n,s*1e6/n);
		n/=10;
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			i=randU();
			i=randU();
			i=randU();
			i=randU();
			i=randU();
			i=randU();
			i=randU();
			i=randU();
			i=randU();
			i=randU();
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	i=randU();		// i.e. %4.1f �s/byte\n",s*1e6, s*1e6/2.);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			i=rand();
			i=rand();
			i=rand();
			i=rand();
			i=rand();
			i=rand();
			i=rand();
			i=rand();
			i=rand();
			i=rand();
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	i=rand(); 		// i.e. %4.1f �s/byte\n",s*1e6, s*1e6/1.);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			i=Random();
			i=Random();
			i=Random();
			i=Random();
			i=Random();
			i=Random();
			i=Random();
			i=Random();
			i=Random();
			i=Random();
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	i=Random();\n",s*1e6);
	
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
			i=nrand(128);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	i=nrand(128);\n",s*1e6);
	
		n/=5;
		x=rand();
		elems68k(&x,FRANDX);
		StartTimer(timer);
		for(iL=n/10;iL>0;iL--){
			elems68k(&x,FRANDX); /* i.e. Randomx() */
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
			elems68k(&x,FRANDX);
		}
		s=StopTimerSecs(timer)/n-overhead;
		ffprintf(o,"%7.1f	�s	elems68k(&x,FRANDX);	// i.e. x=Randomx()\n",s*1e6);
		n*=5;
	}
	DisposeTimer(timer);
	fclose(dataFile);	/* close �TimeCPU results� */
}


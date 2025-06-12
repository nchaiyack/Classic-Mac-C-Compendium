/*
TestLuminance.c
a simple driver to test some of the functionality of Luminance.c
HISTORY:
8/15/89 dgp wrote it.
10/12/90 dgp	updated it.
10/17/90 dgp	tidied up the timing print out.
8/24/91	dgp	Made compatible with THINK C 5.0.
8/27/92	dgp	replace SysEnvirons() by Gestalt()
12/21/92 dgp don't assume 8-bit dacs.
*/
#include "VideoToolbox.h"
#include <math.h>
#include "Luminance.h"
#include <Fonts.h>
#if THINK_C
	#include <console.h>
	#include <profile.h>
#endif

void TestLuminance(void);

void main(void)
{
	Require(gestalt8BitQD);
	TestLuminance();
}

void TestLuminance(void)
{
	register int i;
	static luminanceRecord LR;
	register double L,V;
	register double dL,dV;
	unsigned long t,reps=100;
	double tolerance;
	double L0,L1;
	int n;
	double c,bits;
	double e[512],mean,sd,max,LGot;

	/* Luminance.c includes a recursive routine that requires lots of stack space */
	StackGrow(1000);		/* Increase stack */
	MaxApplZone();			/* Expand heap to the limit. */

	#if THINK_C
		console_options.ncols=100;
		printf("\n");
		InitProfile(200,2);
	#else
	   	/* INITIALIZE QuickDraw */
		InitGraf((Ptr) &thePort);
		InitFonts();
		InitWindows();
		InitCursor();
	#endif
	
	#include "LuminanceRecord1.h"	
	printf("Test SetRange and SetLuminance\n");
	c=1.0;
	L=(LR.LMin+LR.LMax)/2.0;
	for(i=0;i<15;i++){
		L0=L*(1.0-c);
		L1=L*(1.0+c);
		tolerance=SetLuminance(NULL,&LR,0,L,L0,L1);
		printf("c%9.6f, ",c);
		printf("Range%5.1f%5.1f, ",L0,L1);
		printf("fixed %1d/%1d, ",LR.fixed,LR.dacs);
		printf("L(VFixed)=%5.1f cd/m^2, ",VToL(&LR,LR.VFixed));
		bits=log((LR.LMax-LR.LMin)/tolerance)/log(2.0);
		printf("tolerance%6.3f=%4.1f bits\n",tolerance,bits);
		c *= 0.5;
	}

	printf("Test SetLuminancesAndRange & GetLuminance\n");
	L0=LR.LMin;
	L1=LR.LMax;
	tolerance=SetLuminancesAndRange(NULL,&LR,0,LR.VMax,L0,L1,L0,L1);
	printf("Range %5.1f %5.1f, ",L0,L1);
	printf("fixed %1d/%1d, ",LR.fixed,LR.dacs);
	printf("L(VFixed)=%5.1f cd/m^2, ",VToL(&LR,LR.VFixed));
	bits=log((LR.LMax-LR.LMin)/tolerance)/log(2.0);
	printf("tolerance %10.3f = %10.3f bits\n",tolerance,bits);
	dL=(L1-L0)/LR.VMax;
	printf("%10s %10s %10s\n","L requested","L received","Error");
	n=LR.VMax+1;
	if(n>sizeof(e)/sizeof(e[0]))n=sizeof(e)/sizeof(e[0]);
	for(i=0,L=L0;i<n;i++,L+=dL){
		LGot=GetLuminance(NULL,&LR,i);
		e[i]=LGot-L;
		if(i%16==0)printf("%10.3f %10.3f %10.3f\n",L,LGot,LGot-L);
	}
	mean=Mean(e,n,&sd);
	max=0.0;
	for(i=0;i<n;i++)if(fabs(max)<fabs(e[i]))max=e[i];
	printf("Error mean %f sd %f max %f\n\n",mean,sd,max);
	
	printf("Test VToL and LToV\n");
	printf("%10s %10s %10s\n","L requested","V received","L received");
	for(L=-10.0;L<100.0;L+=20.0){
		V=LToV(&LR,L);
		printf("%9.0f %9.1f %12.6f\n",L,V,VToL(&LR,V));
	}

	reps=1000;
	t=TickCount();
	V=LR.VMin;
	dV=(LR.VMax-V)/reps/4;
	for(i=reps/4;i>0;i--) {
		V=VToL(&LR,V);
		V=VToL(&LR,V);
		V=VToL(&LR,V);
		V=VToL(&LR,V);
		V+=dV;
	}
	t=TickCount()-t;
	printf("VToL takes %.1f 탎\n",1e6*t/(60.15*reps));

	reps=1000;
	t=TickCount();
	L=LR.LMin;
	dL=(LR.LMax-L)/reps/4;
	for(i=reps/4;i>0;i--) {
		V=LToV(&LR,L);
		V=LToV(&LR,L);
		V=LToV(&LR,L);
		V=LToV(&LR,L);
		L+=dL;
	}
	t=TickCount()-t;
	printf("LToV takes %.1f 탎\n",1e6*t/(60.15*reps));

	reps=1000;
	L0=(LR.LMin+LR.LMax)/2.0;
	L1=1.01*L0;
	SetLuminance(NULL,&LR,0,50.,L0,L1);
	L=L0;
	dL=(L1-L)/reps/4;
	t=TickCount();
	for(i=reps/4;i>0;i--) {
		SetLuminance(NULL,&LR,0,L,L0,L1);
		SetLuminance(NULL,&LR,0,L,L0,L1);
		SetLuminance(NULL,&LR,0,L,L0,L1);
		SetLuminance(NULL,&LR,0,L,L0,L1);
		L+=dL;
	}
	t=TickCount()-t;
	printf("SetLuminance takes %.1f 탎 with small old range\n",1e6*t/(60.15*reps));

	reps=1000;
	SetLuminance(NULL,&LR,0,50.,LR.LMin,LR.LMax);
	L=LR.LMin;
	dL=(LR.LMax-L)/reps/4;
	t=TickCount();
	for(i=reps/4;i>0;i--) {
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
		L+=dL;
	}
	t=TickCount()-t;
	printf("SetLuminance takes %.1f 탎 with large old range\n",1e6*t/(60.15*reps));

	reps=200;
	SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
	L=LR.LMin;
	dL=(LR.LMax-L)/reps/4;
	t=TickCount();
	for(i=reps/4;i>0;i--) {
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax-1.0);
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax-1.0);
		SetLuminance(NULL,&LR,0,L,LR.LMin,LR.LMax);
		L+=dL;
	}
	t=TickCount()-t;
	printf("SetLuminance takes %.1f 탎 with large new range\n",1e6*t/(60.15*reps));

	reps=200;
	SetLuminance(NULL,&LR,0,L0,L0,L1);
	t=TickCount();
	for(i=0;i<reps/4;i++) {
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1);
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1);
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1);
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1);
	}
	t=TickCount()-t;
	printf("SetLuminancesAndRange(0..255) takes %.1f ms with small old range\n",1000.0*t/(60.15*reps));

#if 1
	reps=48;
	SetLuminance(NULL,&LR,0,L0,L0,L1);
	t=TickCount();
	for(i=0;i<reps/4;i++) {
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1*1.001);
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1);
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1*1.001);
		SetLuminancesAndRange(NULL,&LR,0,255,L0,L1,L0,L1);
	}
	t=TickCount()-t;
	printf("SetLuminancesAndRange(0..255) takes %.1f ms with small new range\n",1000.0*t/(60.15*reps));
#endif

	reps=48;
	SetLuminance(NULL,&LR,0,50.,LR.LMin,LR.LMax);
	t=TickCount();
	for(i=0;i<reps/2;i++) {
		SetLuminancesAndRange(NULL,&LR,0,255,LR.LMin,LR.LMax,LR.LMin,LR.LMax);
		SetLuminancesAndRange(NULL,&LR,0,255,LR.LMin,LR.LMax,LR.LMin,LR.LMax);
	}
	t=TickCount()-t;
	printf("SetLuminancesAndRange(0..255) takes %.1f ms with large old range\n",1000.0*t/(60.15*reps));

#if 1
	reps=48;
	SetLuminance(NULL,&LR,0,50.,LR.LMin,LR.LMax);
	t=TickCount();
	for(i=0;i<reps/2;i++) {
		SetLuminancesAndRange(NULL,&LR,0,255,LR.LMin,LR.LMax-1.0,LR.LMin,LR.LMax-1.0);
		SetLuminancesAndRange(NULL,&LR,0,255,LR.LMin,LR.LMax-1.0,LR.LMin,LR.LMax);
	}
	t=TickCount()-t;
	printf("SetLuminancesAndRange(0..255) takes %.1f ms with large new range\n",1000.0*t/(60.15*reps));
#endif
}	



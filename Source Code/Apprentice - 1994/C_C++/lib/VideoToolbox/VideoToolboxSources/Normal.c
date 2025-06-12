/*
Normal.c
statistical functions related to the normal distribution.
Also see: Binomial.c,ChiSquare.c, Exponential.c, Uniform.c

	f=NormalPdf(x);
	p=Normal(x);
	x=InverseNormal(p);
	x=NormalSample();
	f=Normal2DPdf(double r);
	p=Normal2D(r);
	r=InverseNormal2D(p);
	r=Normal2DSample();
Normal2D is a Gaussian pdf over two dimensions, integrated over all orientations,
0 to 2¹. r is the distance from the origin, [0,Inf]. 

	BoundedNormalIntegers(distribution,n,mean,sd,min,max);
Fills the "distribution" array with n ordered integers so that random samples
from the array,
	i=distribution[nrand(n)];
will have, as nearly as possible, the specified distribution, i.e. they will be
samples (rounded to integer) drawn from the interval [min-.5,max+.5], where min
and max are integers, of a normal distribution with the specified mean and
variance. Once the distribution array has been filled, random samples from it
are a fast way of generating bounded Gaussian noise to be added to each pixel of
an image. Note that "mean" and "sd" apply only to the unclipped underlying
normal distribution. Call this
	mean=MeanW(distribution,n,&sd);
to compute the mean and sd of your integer distribution. The runtime of 
BoundedNormalIntegers will be proportional to max-min+1, and nearly independent
of n. It takes about 0.3 s on a Mac IIci for max-min+1=256.

Copyright (c) 1989,1990,1991,1992 Denis G. Pelli
HISTORY:
1989	dgp wrote it.
4/8/90	dgp	changed the names of the routines. 
			Made sure that domain error produces NAN.
6/90	dgp	added NormalSample()
7/30/91	dgp	now use NAN defined in VideoToolbox.h
12/28/91 dgp sped up NormalPdf() by calculating the scale factor only once
12/29/91 dgp extracted code to create new routine UniformSample.c
1/11/92	dgp	rewrote Normal()'s polynomial evaluation to halve the number of multiplies
			Renamed NormalPDF() to NormalPdf().
1/19/92	dgp	defined the constants LOG2 and LOGPI in VideoToolbox.h
			Added more domain tests, returning NAN if outside. 
			Added more checks to main().
			Wrote Normal2DPdf(),Normal2D(),InverseNormal2D(),and Normal2DSample().
2/1/92	dgp	Redefined Normal2DPdf(r) to now, more sensibly, treat r as the random
			variable, rather than the implicit x and y, where r=sqrt(x^2+y^2). 
			Previously, Normal2D(R)=Integrate[2 Pi r Normal2DPdf(r),{r,0,R}]
			now Normal2D(R)=Integrate[Normal2DPdf(r),{r,0,R}]. There is no change
			in Normal2D(). I suspect that Normal2D is in fact the Raleigh distribution,
			and I will rename it if this is in fact the case.
11/13/92 dgp InverseNormal(0) now returns -INF, and InverseNormal(1) returns INF.
12/15/93 dgp declared some arguments "register".
3/26/94	dgp	added BoundedNormalIntegers().
			Asked compiler to use 68881 instructions for transcendentals.
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <math.h>
#include <mc68881.h>
void BoundedNormalIntegers(short *distribution,long n,double mean,double sd
	,short min,short max);
#if THINK_C && mc68881
	#define exp _exp	/* use fast 68881 instruction instead of SANE */
	#define log _log	/* use fast 68881 instruction instead of SANE */
	#define sqrt _sqrt	/* use fast 68881 instruction instead of SANE */
#endif

double NormalPdf(register double x)
/* Gaussian pdf. Zero mean and unit variance. */
{
	if(IsNan(x))return x;
	return exp(-0.5*(x*x+(LOG2+LOGPI)));
}

double Normal(register double x)
/*
Cumulative normal distribution. From Abramowitz and Stegun Eq. (26.2.17).
Error |e|<7.5 10^-8
*/
{
	register double P,t;
	
	if(x<0.0) return 1.0-Normal(-x);
	t=1.0/(1.0+0.2316419*x);
	P=(0.319381530+(-0.356563782+(1.781477937+(-1.821255978+1.330274429*t)*t)*t)*t)*t;
	return 1.0-NormalPdf(x)*P;
}

double InverseNormal(register double p)
/*
Inverse of Normal(), based on Abramowitz and Stegun Eq. 26.2.23.
Error |e|<4.5 10^-4.
*/
{
	register double t,x;
	
	if(IsNan(p))return p;
	if(p<0.0)return NAN;
	if(p==0.0)return -INF;
	if(p>0.5) return -InverseNormal(1.0-p);
	t=sqrt(-2.0*log(p));
	x=t-(2.515517+(0.802853+0.010328*t)*t)/(1.0+(1.432788+(0.189269+0.001308*t)*t)*t);
	return -x;
}

double NormalSample(void)
{
	return InverseNormal(UniformSample());
}

double Normal2DPdf(double r)
/* Gaussian pdf over two dimensions, integrated over all orientations, 0 to 2¹. */
/* The argument is taken to be the distance from the origin, [0,Inf]. */
/* The rms is 1 */
{
	if(IsNan(r))return r;
	if(r<=0.0)return 0.0;
	return 2*r*exp(-r*r);
}

double Normal2D(double r)
/* Integral of Normal2DPdf() from zero to r. */
{
	if(IsNan(r))return r;
	if(r<=0.0)return 0.0;
	return 1.0-exp(-r*r);
}

double InverseNormal2D(double p)
{
	if(IsNan(p))return p;
	if(p<0.0 || p>1.0)return NAN;
	return sqrt(-log(1.0-p));
}

double Normal2DSample(void)
/* rms is 1 */
{
	return InverseNormal2D(UniformSample());
}

/*
	Integrate[exp(-.5*u^2),{u,a,a+1/sd}]
	=exp(-.5*a^2)*Integrate[exp(-.5*((a+e)^2-a^2)),{e,0,1/sd}]
	=exp(-.5*a^2)*Integrate[exp(-.5*e*e)*exp(-a*e),{e,0,1/sd}]
	=exp(-.5*a^2)*Integrate[(1-.5*e*e)*exp(-a*e),{e,0,1/sd}]
	=exp(-.5*a^2)*((exp(-a/sd) - 1)/(-a)-.5*Integrate[e*e*exp(-a*e),{e,0,1/sd}])
	=exp(-.5*a^2)*(1-exp(-a/sd))/a
*/
void BoundedNormalIntegers(register short *distribution,long n,double mean,double sd
	,short min,short max)
{
	register short i;
	register long j,count;
	double p,p0,p1,x;
	short shortcut;
	
	shortcut=sd*sd>n;	// guarantees count will err by at most ±0.5
	j=0;
	p=p0=Normal((min-.5-mean)/sd);
	p1=Normal((max+.5-mean)/sd)-p0;
	for(i=min;i<max;i++){
		x=(i+.5-mean)/sd;
		if(shortcut)p+=NormalPdf(x)*(exp(x/sd)-1)/x;
		else p=Normal(x);
		count=0.5+n*(p-p0)/p1;
		while(j<count)distribution[j++]=i;
	}
	while(j<n)distribution[j++]=max;
}


#if 0 /* A test program. */
	#include <sane.h>
	extended DoubleToExtended(double x);
	double ExtendedToDouble(extended x80);
	void main()
	{
		double x,y,sum,dx,a,b,mean,sd;
		static double z[1000];
		int i;
		extended e,ee;
		
		Require(0);
		srand(clock());
		printf("%4s%15s%15s%20s%15s\n","x","NormalPdf(x)","Normal(x)","InverseNormal","Error");
		for(x=-4.0;x<=4.0;x+=2.0){
			printf("%4.1f%15.8f%15.8f%20.4f%15.4f\n",
			x,NormalPdf(x),Normal(x),InverseNormal(Normal(x)),InverseNormal(Normal(x))-x);
		}
		sum=0.0;
		dx=0.001;
		for(x=-1.;x<0.;x+=dx)sum+=NormalPdf(x);
		sum*=dx;
		sum-=Normal(0.0)-Normal(-1.0);
		printf("Partial integral of NormalPdf error %.5f\n",sum);
		for(i=0;i<1000;i++)z[i]=NormalSample();
		mean=Mean(z,1000,&sd);
		printf("1000 samples mean %.2f sd %.2f\n",mean,sd);
		printf("\n");
	
		printf("%4s%15s%15s%20s%15s\n","x","Normal2DPdf(x)","Normal2D(x)","InverseNormal2D","Error");
		for(x=-1.;x<=5.0;x+=1.0){
			printf("%4.1f%15.8f%15.8f%20.4f%15.4f\n",
			x,Normal2DPdf(x),Normal2D(x),InverseNormal2D(Normal2D(x)),InverseNormal2D(Normal2D(x))-x);
		}
		sum=0.0;
		dx=0.0001;
		for(x=0;x<1.;x+=dx)sum+=Normal2DPdf(x);
		sum*=dx;
		sum-=Normal2D(1.0);
		printf("Partial integral of Normal2DPdf error %.5f\n",sum);
		for(i=0;i<1000;i++)z[i]=Normal2DSample();
		mean=Mean(z,1000,&sd);
		printf("1000 samples rms %.2f\n",sqrt(mean*mean+sd*sd));
		printf("\n");
		for(i=0;i<1000;i++){
			x=NormalSample();
			y=NormalSample();
			z[i]=sqrt((x*x+y*y)/2.);
		}
		mean=Mean(z,1000,&sd);
		printf("1000 (x,y) normal samples with sd 2^-0.5 have rms hypotenuse of %.2f\n",sqrt(mean*mean+sd*sd));
		printf("\n");
	
		a=4.0*atan(1.0);
		if(a!=PI)printf("4*atan(1)-PI %.19f\n",a-PI);
		a=ExtendedToDouble(pi());
		if(a!=PI)printf("Error: pi %.19f, pi-PI %.19f\n",a,a-PI);
		a=log(a);
		if(a!=LOGPI)printf("Error: log(PI) %.19f, error in LOGPI %.19f\n",a,LOGPI-a);
		a=log(2.0);
		if(a!=LOG2)printf("Error: log(2) %.19f, error in LOG2 %.19f\n",a,LOG2-a);
	}
#endif

/*
Mean.c
	mean=Mean(x,n,&sd);
	mean=MeanF(x,n,&sd);
	mean=MeanB(x,n,&sd);
	mean=MeanUB(x,n,&sd);
	mean=MeanW(x,n,&sd);
	mean=MeanUW(x,n,&sd);
	mean=MeanL(x,n,&sd);
	mean=MeanUL(x,n,&sd);
Quickly compute mean (and optionally the standard deviation) of an array x[] of
n samples. The various routines differ only in the assumed type of x[]: double,
char, short, or long, either signed or unsigned. The standard deviation is computed
only if the last argument is not NULL. The loops have been optimized
to do only one memory access per iteration.

HISTORY:
9/16/90	dgp	wrote it.
1/10/94	dgp added MeanB and MeanUB that operate on arrays of signed char and 
			unsigned char.
			Optimized to minimize memory accesses when compiled by THINK C 6.01.
1/25/94	dgp added MeanW, MeanUW, MeanL, and MeanUL.
*/
#include "VideoToolbox.h"
#include <math.h>
double MeanF(float x[],long n,double *sdPtr);

double Mean(double x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	double *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss;
		register double xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanF(float x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	float *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss;
		register double xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanB(char x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	register char *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss;
		register long xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanUB(unsigned char x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	register unsigned char *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss;
		register long xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanW(short x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	register short *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss;
		register long xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanUW(unsigned short x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	register unsigned short *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss;
		register unsigned long xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanL(long x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	register long *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss,xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

double MeanUL(unsigned long x[],long n,double *sdPtr)
{
	register double s;
	register long i;
	register unsigned long *p;

	p=x;
	if(sdPtr!=NULL){
		register double ss,xx;

		s=ss=0.0;
		for(i=n;i>0;i--){
			s+=xx=*p++;
			ss+=xx*xx;
		}
		s/=n;
		if(n>1)*sdPtr=sqrt((ss-n*s*s)/(n-1));
		else *sdPtr=0;
	}else{
		s=0.0;
		for(i=n;i>0;i--) s+=*p++;
		s/=n;
	}
	return s;
}

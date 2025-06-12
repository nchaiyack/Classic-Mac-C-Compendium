/*
Binomial.c
Copyright (c) 1990,1991,1992 Denis G. Pelli

Various routines that deal with Binomial statistics, including generating random
samples and computing confidence intervals.

This file contains two generations of routines that generate confidence
intervals. The original routines BinomialUpperBound and BinomialLowerBound are
old, but are more robust. The new routines are based on a Numerical Recipes in C
routine to compute the incomplete beta function. The new routines add little to
the old routines, and require the Numerical Recipes in C, so I commented them
out. The new routines do have the advantage of allowing an arbitrary confidence,
whereas the old routines were hard coded to produce a 95% confidence interval.

Also see: ChiSquare.c, Exponential.c, Normal.c, Uniform.c

HISTORY:
1/5/91 Added new binomial routines. These routines may be 
		commented out by setting NEW_BINOMIAL to zero.
1/7/91 Rewrote BinomialUpperBound & BinomialLowerBound to allow specification of the
		desired confidence of the interval.
4/24/92	dgp	Added BinomialSampleQuickly().
4/27/92	dgp	Oops. I just noticed that this file was always including nr.h, which you'd
			only have if you own the Numerical Recipes. I moved the include statement
			down inside the conditional below, where it belongs. 
*/
#include "VideoToolbox.h"
#include <math.h>

#define NEW_BINOMIAL 0

long BinomialSample(double p,long n)
/*
Returns a a sample from a binomial distribution: number of heads in n flips of
coin with a probability p of heads on each flip. This is fine if n is small. However,
if n is large, e.g. n>100, then this routine will be rather slow, and you may prefer
to use the more elaborate Numerical Recipes bnldev() routine.
*/
{
	long k,i;

	k=0;
	for(i=0;i<n;i++) if(p>UniformSample())k++;
	return k;
}

int BinomialSampleQuickly(int n)
// Does n coin flips and returns the number of heads. Very fast.
{
	register int i,k=0;
	register short r;
	
	for(i=n;i>=8;i-=8){
		r=randU();	// Use only the upper 8 bits, which are reputed to be more random.
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
		if(r<0)k++;
		r<<=1;
	}
	if(i>0){
		r=randU();
		for(;i>0;i--){
			if(r<0)k++;
			r<<=1;
		}
	}
	return k;
}

/*
BinomialUpperBound and BinomialLowerBound return a P confidence interval for the
underlying binomial probability assumed to have generated the data.

The formula is based on a Gaussian approximation, solving for the p that will
put the observed result the right number of standard deviations away from the
mean, plus or minus 0.5, as a "continuity correction".

This is the best formula I could find in looking through several statistics
books. However, the result has to be taken with a grain of salt because it is
not possible to produce a binomial confidence interval that will satisfy the
strict definition of a confidence interval, namely one that will have the
specified probability P of containing the unknown but fixed parameter p. That's
because, unlike the Normal distribution, the Binomial distribution is not
translation invariant. E.g. if p is actually zero then the true confidence interval
ALWAYS contains p.

I gave some thought to taking a Bayesian approach, assuming a uniform prior pdf
for p and then computing an a posteriori confidence interval. This can be done,
but it's hard. I concluded that it's pointless because the uniform pdf
assumption is usually unwarranted.

Even though this confidence interval is unsatisfactory (i.e. fundamentally
false) it is useful in practice since it typically behaves similarly to the
Normal case, which is theoretically sound.
*/
double BinomialLowerBound(double P,long k,long n)
/*
Arguments are the confidence P, and the number k of heads in n flips of a coin.
The returned value p is the lower end of a P confidence interval on the
underlying probability of a head on a single trial.
*/
{
	double right,s,ss,p;
	
	if(k>0 && k<n){
		right=k-0.5;
		s=InverseNormal(sqrt(P));
		ss=s*s;
		p=(right+0.5*ss-s*sqrt(right*(1.0-right/n)+0.25*ss))/(n+ss);
		return p;
	}
	if(k==0) return 0.0;
	if(k==n) return pow(1.0-P,1.0/n);
	return sqrt(-1.0);	/* domain error */
}

double BinomialUpperBound(double P,long k,long n)
/*
Arguments are the confidence P, and the number k of heads in n flips of a coin.
The returned value p is the upper end of a P confidence interval on the
underlying probability of a head on a single trial.
*/
{
	double right,s,ss,p;
	
	if(k>0 && k<n){
		right=k+0.5;
		s=InverseNormal(sqrt(P));
		ss=s*s;
		p=(right+0.5*ss+s*sqrt(right*(1.0-right/n)+0.25*ss))/(n+ss);
		return p;
	}
	if(k==0) return 1.0-pow(1.0-P,1.0/n);
	if(k==n) return 1.0;
	return sqrt(-1.0);	/* domain error */
}

#if NEW_BINOMIAL	/* new routines that require Numerical Recipes in C */
	#include <nr.h>				/* prototype for betai() */
	#if 0
		void main(void)
		/* a quick and dirty driver to test some of these routines */
		{
			double p,pUpper,pLower,P,PUpper,PLower;
			int i,n,k;
			
			Require(0);
			n=10;
			p=.2;
			pLower=InverseBinomial(0.05,21,50);
			printf("%f, Abramowitz & Stegun p. 960 Example 18 say this should be 0.3003.\n",pLower);
			P=0.95;
			for(i=0;i<10;i++){
				k=BinomialSample( p, n);
				pLower=BinomialLowerBound(P,k,n);
				pUpper=BinomialUpperBound(P,k,n);
				PLower=1.0-Binomial(pLower,k,n);
				PUpper=Binomial(pUpper,k+1,n);
				printf("n %4d p %6.4f %6.4f²%6.4f²%6.4f  %6.4f< <%6.4f\n"
					,n,p,pLower,k/(double)n,pUpper,PLower,PUpper);
				pLower=InverseBinomial(1.0-sqrt(P),k,n);
				pUpper=InverseBinomial(sqrt(P),k+1,n);
				PLower=1.0-Binomial(pLower,k,n);
				PUpper=Binomial(pUpper,k+1,n);
				printf("  %4d p %6.4f %6.4f²%6.4f²%6.4f  %6.4f< <%6.4f\n"
					,n,p,pLower,k/(double)n,pUpper,PLower,PUpper);
			}
		}
	#endif
	
	double Binomial(double p,long k,long n)
	/*
	Returns the probability of k or more heads in n flips, where probability of
	each heads is p. This identity appears in Numerical Recipes in C, page 182, and
	in Abramowitz and Stegun, p. 945. Eq.26.5.24.
	*/
	{
		if(k>n)return 0.0;
		if(k<=0L)return 1.0;
		if(p==0.0){
			if(k==0)return 1.0;
			else return 0.0;
		}
		if(p==1.0)return 1.0;
		return IncompleteBeta(p,k,n-k+1);
	}
	
	double BinomialPdf(double p,long k,long n)
	/*
	Returns the probability of exactly k heads in n flips, where probability of
	each heads is p. 
	I'm not really sure whether this is more computationally efficient than computing
	it directly, from the definition of the binomial distribution, but it is very easy
	to write, and avoids the difficulty of computing the binomial coefficient
	without overflow.
	*/
	{
		if(k<0L || k>n)return 0.0;
		if(k==n) return pow(p,n);
		return Binomial(p,k,n)-Binomial(p,k+1,n);
	}
	
	double InverseBinomial(double P,long k,long n)
	/* Returns the P-th quantile for the probability p of a heads,
	given k heads in n flips. 
	pUpper=InverseBinomial(0.975,k+1,n) is a 97.5% confidence upper bound on p
	pLower=InverseBinomial(0.025,k,n) is a 97.5% confidence lower bound on p
	Taken together these bounds form a 97.5%*97.5%=95% confidence interval: [pLower,pUpper].
	*/
	{
	#if 0
		return InverseIncompleteBeta(P,k,n-k+1);
	#else
		/*
		This simple-minded bisection routine is slow, but its answer is accurate
		to within ±1e-10. 
		*/
		double low=0.0,high=1.0,mid;
		double f;
		int i;
		
		for(i=0;i<30;i++){
			mid=(low+high)/2.0;
			f=Binomial(mid,k,n);
			if(f>P)high=mid;
			else low=mid;
		}
		return (low+high)/2.0;
	#endif
	}

	double IncompleteBeta(double x,double a,double b)
	/*
	The incomplete beta function Ix(a,b).
	The Numerical Recipes routine assumes a>0. This work around is taken from
	Abramowitze and Stegun page 944, Eq. 26.5.16.
	*/
	{
		if(a>0.0)return betai(a,b,x);	/* Numerical Recipes in C */
		else return IncompleteBeta(x,a+1.0,b)
			+exp(gammln(a+b)-gammln(a+1.0)-gammln(b)+a*log(x)+b*log(1.0-x));
	}
	
	double InverseIncompleteBeta(double p,double a,double b)
	/*
	This simple-minded bisection routine is slow, but its answer is accurate
	to within ±1e-10. 
	*/
	{
		double low=0.0,high=1.0,mid;
		double f;
		int i;
		
		for(i=0;i<30;i++){
			mid=(low+high)/2.0;
			f=IncompleteBeta(mid,a,b);
			if(f>p)high=mid;
			else low=mid;
		}
		return (low+high)/2.0;
	}
	
#endif

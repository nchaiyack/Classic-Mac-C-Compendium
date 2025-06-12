/* PlotXY.c
Copyright © 1991 Denis G. Pelli
PlotXY plots graphs, one point at a time, possibly interleaving multiple graphs. 
The user keeps a PlotXYStyle structure associated with each curve, which allows
interleaving of points for several curves, which is essential for online monitoring.

// defined in VideoToolbox.h
typedef struct{
	Boolean continuing;	// zero to start a new curve
	long color;			// e.g. blackColor, blueColor
	short lineWidth;	// in pixels, zero for none
	short symbolWidth;	// in pixels, zero for none
	short dashOffset;	// in pixels
	short dash[5];		// in pixels. The array is terminated by a zero element
	short h,v;			// reserved for internal use
	Fixed pathLengthF;	// reserved for internal use
} PlotXYStyle;

WARNING: I've discovered that this routine may crash if fed garbage x,y coordinates
while dashing is on, e.g. extremely large numbers. I'm not sure where the problem is. 
Overflows shouldn't cause crashes.

HISTORY:
90 		dgp wrote it.
3/91	dgp	re-rewrote it, adding dashing and symbols, and making it re-entrant.
3/18/91	dgp	replaced floating point math by fixed point math.
			made compatible with original quickdraw.
8/24/91	dgp	Made compatible with THINK C 5.0.
12/27/91 dgp Made dashing faster by measuring path length modulo the dashing period.
1/25/93 dgp removed obsolete support for THINK C 4.
*/
#include "VideoToolbox.h"

Fixed HypotenuseF(long base,long height);

Fixed HypotenuseF(long base,long height)
{
	base*=base;
	height*=height;
	return FracSqrt(base+height)<<1;
}

void PlotXY(WindowPtr window,double x,double y,PlotXYStyle *style)
/*
Draws line to point x,y, where window is mapped as a unit square, with origin
at lower left. 
*/
{
	register PlotXYStyle *s;
	register int i;
	register short h,v,dh,dv;
	Rect r;
	WindowPtr oldPort;
	PenState penState;
	Pattern blackPattern={-1,-1,-1,-1,-1,-1,-1,-1};
	Boolean dashOn;
	int dashElements;
	int dashPeriod;
	int dashDebit;
	int length,delta;
	Fixed lengthF;
	long oldColor;
	int dashingPeriod;
	
	s=style;
	GetPort(&oldPort);
	SetPort(window);
	BringToFront(window);
	GetPenState(&penState);
	oldColor=window->fgColor;
	ForeColor(s->color);
	PenSize(s->lineWidth,s->lineWidth);
	r=window->portRect;
	h=(r.right-r.left)*x;
	v=(r.bottom-r.top)*(1.0-y);
	
	if(!s->continuing){
		/* First point, just go there */
		s->pathLengthF=s->dashOffset<<16;
		s->h=h;
		s->v=v;
		MoveTo(s->h,s->v);
		s->continuing=1;
	}
	else {
		/* Not first point, start from previous point */
		MoveTo(s->h,s->v);
		h-=s->h;
		v-=s->v;
		s->h+=h;
		s->v+=v;
		lengthF=HypotenuseF(h,v);
		length=lengthF>>16;
		/* Figure out where we are in dashing sequence */
		dashDebit=s->pathLengthF>>16;
		dashOn=1;
		dashingPeriod=0;
		for(i=0;s->dash[i]>0;i++)dashingPeriod+=s->dash[i];
		dashElements=i;
		if(dashElements%2==1)dashingPeriod*=2;
		if(dashingPeriod==0)dashingPeriod=1;	/* to avoid divide by zero */
		if(dashElements==0){
			Line(h,v);
		}
		else {
			for(i=0;;i++){
				i%=dashElements;
				dashDebit-=s->dash[i];
				if(dashDebit<0){
					dashDebit+=s->dash[i];
					break;
				}
				dashOn=!dashOn;
			}
			/* Draw dashed line */
			for(;;i++){
				i%=dashElements;
				if(length<=s->dash[i]-dashDebit){
					if(dashOn)Line(h,v);
					else Move(h,v);
					dashDebit+=length;
					break;
				}
				delta=s->dash[i]-dashDebit;
				dh=((long)h*delta+length/2)/length;
				dv=((long)v*delta+length/2)/length;
				if(dashOn)Line(dh,dv);
				else Move(dh,dv);
				h-=dh;
				v-=dv;
				length-=delta;
				dashDebit=0;
				dashOn=!dashOn;
			}
		}
		lengthF%=(long)dashingPeriod<<16;		/* reduce mod period to avoid overflow */
		s->pathLengthF+=lengthF;
		s->pathLengthF%=(long)dashingPeriod<<16;/* reduce mod period for speed */
	}
	
	/* Draw symbol */
	SetRect(&r,0,0,s->symbolWidth,s->symbolWidth);
	OffsetRect(&r,-r.right/2,-r.bottom/2);
	OffsetRect(&r,s->h,s->v);
	FillOval(&r,blackPattern);
	ForeColor(oldColor);
	SetPenState(&penState);
	SetPort(oldPort);
}
	
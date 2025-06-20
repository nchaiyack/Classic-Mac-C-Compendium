/*/
     Project Arashi: FitSped.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, October 28, 1992, 23:38
     Created: Tuesday, February 7, 1989, 22:36

     Copyright � 1989-1992, Juri Munkki
/*/

#include <Math.h>

#define	PI 3.1415926535

#define MAXSEGS	48
#define	ANGLES	120

typedef struct
{
	int		numsegs;
	char	wraps;
	char	seglen;
	char	xoff;
	char	yoff;
	int		filler;
	int		ang[MAXSEGS];
}	shape;

extern	shape		space;

void	CreateFit()
{
	double	x,y,xmin,xmax,ymin,ymax,a;
	double	sx,sy;
	int		i;
	
	xmin=xmax=ymin=xmax=x=y=0.0;
	a=0.0;
	
	for(i=0;i<space.numsegs;i++)
	{	a+=space.ang[i];
		x+= cos(a/ANGLES*PI*2);
		y+=-sin(a/ANGLES*PI*2);
		if(x<xmin)	xmin=x;
		if(x>xmax)	xmax=x;
		if(y<ymin)	ymin=y;
		if(y>ymax)	ymax=y;
	}

	xmin-=1.0;
	ymin-=1.0;
	xmax+=1.0;
	ymax+=1.0;
	x=-(xmin+xmax)/2;
	y=-(ymin+ymax)/2;
	
	sx=640/(xmax-xmin);
	sy=480/(ymax-ymin);
	if(sy<sx)	sx=sy;
	
	x*=sx;
	y*=sx;
	a=0.0;
	MoveTo((int)(x+320.5),(int)(y+220.5));
	SmallCross();
	for(i=0;i<space.numsegs;i++)
	{	a+=space.ang[i];
		x+= cos(a/ANGLES*PI*2)*sx;
		y-= sin(a/ANGLES*PI*2)*sx;
		LineTo((int)(x+320.5),(int)(y+220.5));
		SmallCross();
	}
}
void	DrawPlayer(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	int		x1,y1,x2,y2;
	Point	foo;
	int		slant;
	
	GetMouse(&foo);
	slant = foo.h>>2;
	if(slant<1)slant=1;
	if(slant>7)slant=7;
	x1=cx+dx;
	y1=cy+dy;
	
	MoveTo(cx,cy);
	x2=(dx*slant)/8+dy/4;
	y2=(dy*slant)/8-dx/4;
	LineTo(cx+x2,cy+y2);
	LineTo(x1,y1);
	x2=-(slant*dx)/16-dy/4;
	y2=-(slant*dy)/16+dx/4;
	LineTo(x1+x2,y1+y2);
	x2=-dx/8;
	y2=-dy/8;
	LineTo(x1+x2,y1+y2);
	x2=(dx*slant)/8+dy/8;
	y2=(dy*slant)/8-dx/8;
	LineTo(cx+x2,cy+y2);
	x2=dx/8;
	y2=dy/8;
	LineTo(cx+x2,cy+y2);
	x2=((8-slant)*dx)/16-dy/4;
	y2=((8-slant)*dy)/16+dx/4;
	LineTo(cx+x2,cy+y2);
	LineTo(cx,cy);
}
void	DrawFlipper(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	int	x1,y1,x2,y2;
	int	dx4,dx8,dy4,dy8;
	
	dx4=dx/4;
	dy4=dy/4;
	dx8=dx/8;
	dy8=dy/8;
	
	x1=cx+dx;
	y1=cy+dy;
	MoveTo(cx,cy);
	x2=dx8-dy4;	
	y2=dy8+dx4;
	LineTo(x1-x2,y1-y2);
	x2=dx4-dy8;	
	y2=dy4+dx8;
	LineTo(x1-x2,y1-y2);
	LineTo(x1,y1);
	
	x2=dx8+dy4;	
	y2=dy8-dx4;
	LineTo(cx+x2,cy+y2);
	x2=dx4+dy8;	
	y2=dy4-dx8;
	LineTo(cx+x2,cy+y2);
	LineTo(cx,cy);
}
void	DrawPulsar(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	int	x1,y1,x2,y2;
	int	dx4,dx8,dy4,dy8;
	int	scl,dxf,dyf;
	Point	foo;
	
	GetMouse(&foo);
	scl=(foo.h>>2);
	scl-=20;
	if(scl<-8) scl=-8;
	if(scl>8)  scl=8;
	
	dx4=dx/4;
	dy4=dy/4;
	dxf=(dx*scl)/32;
	dyf=(dy*scl)/32;
	dx8=dx/8;
	dy8=dy/8;

	x1=cx+dx;
	y1=cy+dy;
	MoveTo(cx,cy);
	x2=dx8-dyf;
	y2=dy8+dxf;
	LineTo(cx+x2,cy+y2);
	
	x2=dx4+dyf;
	y2=dy4-dxf;
	LineTo(cx+x2,cy+y2);
	x2=dx/2-dyf;
	y2=dy/2+dxf;
	LineTo(cx+x2,cy+y2);

	x2=-dx4+dyf;
	y2=-dy4-dxf;
	LineTo(x1+x2,y1+y2);
	x2=-dx8-dyf;
	y2=-dy8+dxf;
	LineTo(x1+x2,y1+y2);
	LineTo(x1,y1);
}
void	TestFlip()
{
	double	a;
	Rect	argh;
	long	rest;
	Point	foo;
	
	SetRect(&argh,100,100,300,300);
	
	a=0.0;
	while(!Button())
	{	GetMouse(&foo);
		EraseRect(&argh);
		DrawPlayer(200,200,(int)(100*cos(a/ANGLES*PI*2)),
							(int)(100*-sin(a/ANGLES*PI*2)));
		Delay(3,&rest);
		a=foo.v;
	}
}
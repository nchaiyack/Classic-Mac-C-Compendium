/*/
     Project Arashi: Shapes.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:44
     Created: Monday, July 24, 1989, 22:47

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "STCrack.h"
#include <FixMath.h>

#define	FUSESEGMENTS	3

static	int		fusedivs[]={1500,800,600};
static	int		fuseoff[5][FUSESEGMENTS];
static	Point	BlowCenter;
static	int		RotSpeed;
static	int		BlowSize;

#define	FIXEDPI	(205887L)	/*	pi*65536	*/

void	BlowUpLine(x,y)
int		x,y;
{
	long	unitsize;
	Point	delta;
	Rect	square;
	int		angle;
		
	delta.h = x-VA.CurrentX;
	delta.v = y-VA.CurrentY;
	
	unitsize = FracSqrt(delta.h*(long)delta.h+delta.v*(long)delta.v)>>15;
	
	angle = FixATan2(delta.h,-delta.v) * ANGLES / (2*FIXEDPI);
	angle = (ANGLES+angle) % ANGLES;

	delta.h = x-BlowCenter.h-(delta.h>>1);
	delta.v = y-BlowCenter.v-(delta.v>>1);
	
	
	AddCrack((VA.CurrentX+x)>>1,(VA.CurrentY+y)>>1,
				delta.h,delta.v,VA.color,
				unitsize,FlyingVector,angle,RotSpeed,BlowSize);
	
	RotSpeed= -RotSpeed;
	VAMoveTo(x,y);
}
void	BlowUpHalfLine(x,y)
int		x,y;
{
	BlowUpLine((x+VA.CurrentX)>>1,(y+VA.CurrentY)>>1);
	BlowUpLine(x,y);
}

void	UpdateFuseOff()
{
	register	int		i,j;
	
	for(j=FUSESEGMENTS-1;j>0;j--)
	for(i=0;i<5;i++)
	{	fuseoff[i][j]=fuseoff[i][j-1];
	}		
	for(i=0;i<5;i++)
	{	fuseoff[i][0]=(FastRandom() & 15) - 7;
	}
}


void	DrawFuseBall(cx,cy,size,angle)
register	int		cx,cy,size,angle;
{
	int		dx,dy,i,j,a;
	
	for(i=0;i<5;i++)
	{	VA.color=ThisLevel.fuColor[i];
		VAMoveTo(cx,cy);

		for(j=0;j<FUSESEGMENTS;j++)
		{
			a=angle+fuseoff[i][j];
			if(a<0) a+=ANGLES;
			if(a>=ANGLES)	a-=ANGLES;

			dx=((long)size * Cosins[a])/fusedivs[j];
			dy=((long)size * Sins[a]  )/fusedivs[j];
			VALineTo(cx+dx,cy+dy);
		}

		angle+=(ANGLES/5);
		if(angle>=ANGLES)	angle-=ANGLES;
	}
}
void	BlowFuseBall(cx,cy,size,angle)
register	int		cx,cy,size,angle;
{
	int		dx,dy,i,j,a;
	
	RotSpeed=4;
	BlowSize=10;
	BlowCenter.h=cx;
	BlowCenter.v=cy;

	for(i=0;i<5;i++)
	{	VA.color=ThisLevel.fuColor[i];
		VAMoveTo(cx,cy);

		for(j=0;j<FUSESEGMENTS;j++)
		{
			a=angle+fuseoff[i][j];
			if(a<0) a+=ANGLES;
			if(a>=ANGLES)	a-=ANGLES;

			dx=((long)size * Cosins[a])/fusedivs[j];
			dy=((long)size * Sins[a]  )/fusedivs[j];
			BlowUpLine(cx+dx,cy+dy);
		}

		angle+=(ANGLES/5);
		if(angle>=ANGLES)	angle-=ANGLES;
	}
}
void	DrawPlayer(cx,cy,dx,dy,slant)
int		cx,cy,dx,dy,slant;
{
	register	int		x2,y2,dxs,dys;
				int		x1,y1,x4,y4,x8,y8;

	if(slant<1)slant=1;
	if(slant>7)slant=7;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	dxs=dx*slant;				dys=dy*slant;
	
	VAMoveTo(cx,cy);
	x2=(dxs>>3)+y4;				y2=(dys>>3)-x4;

	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(x1,y1);

	x2=-(dxs>>4)-y4;			y2=-(dys>>4)+x4;

	VASafeLineTo(x1+x2,y1+y2);
	VASafeLineTo(x1-x8,y1-y8);

	x2=(dxs>>3)+y8;				y2=(dys>>3)-x8;

	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(cx+x8,cy+y8);

	x2=(((8-slant)*dx)>>4)-y4;	y2=(((8-slant)*dy)>>4)+x4;

	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(cx,cy);
}

void	BlowUpPlayer(cx,cy,dx,dy,slant)
int		cx,cy,dx,dy,slant;
{
	register	int		x2,y2,dxs,dys;
				int		x1,y1,x4,y4,x8,y8;

	if(slant<1)slant=1;
	if(slant>7)slant=7;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	dxs=dx*slant;				dys=dy*slant;

	BlowCenter.h = ((cx+x1)>>1);
	BlowCenter.v = ((cy+y1)>>1);
	RotSpeed=1;
	BlowSize=40;
	
	VAMoveTo(cx,cy);
	x2=(dxs>>3)+y4;				y2=(dys>>3)-x4;

	BlowUpHalfLine(cx+x2,cy+y2);
	BlowUpHalfLine(x1,y1);

	x2=-(dxs>>4)-y4;			y2=-(dys>>4)+x4;

	BlowUpHalfLine(x1+x2,y1+y2);
	BlowUpHalfLine(x1-x8,y1-y8);

	x2=(dxs>>3)+y8;				y2=(dys>>3)-x8;

	BlowUpHalfLine(cx+x2,cy+y2);
	BlowUpHalfLine(cx+x8,cy+y8);

	x2=(((8-slant)*dx)>>4)-y4;	y2=(((8-slant)*dy)>>4)+x4;

	BlowUpHalfLine(cx+x2,cy+y2);
	BlowUpHalfLine(cx,cy);
}

void	DrawStaticPlayer(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int		x2,y2,dxs,dys;
				int		x1,y1,x4,y4,x8,y8;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	dxs=dx*4;					dys=dy*4;
	
	VAMoveTo(cx,cy);
	x2=(dx>>1)+y4;				y2=(dy>>1)-x4;

	VAStaticLineTo(cx+x2,cy+y2);
	VAStaticLineTo(x1,y1);

	x2=-(dx>>2)-y4;				y2=-(dy>>2)+x4;

	VAStaticLineTo(x1+x2,y1+y2);
	VAStaticLineTo(x1-x8,y1-y8);

	x2=(dx>>1)+y8;				y2=(dy>>1)-x8;

	VAStaticLineTo(cx+x2,cy+y2);
	VAStaticLineTo(cx+x8,cy+y8);

	x2=(dx>>2)-y4;				y2=((dy)>>2)+x4;

	VAStaticLineTo(cx+x2,cy+y2);
	VAStaticLineTo(cx,cy);
}

#ifdef	FUZZYPLAYER
void	VAFractalLineTo(int,int,int,int);

#define	FRACTALFACTOR	(1<<6)
#define	FRACTALDIV		3

void	DrawFuzzyPlayer(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int		x2,y2,dxs,dys;
				int		x1,y1,x4,y4,x8,y8;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	dxs=dx*4;					dys=dy*4;
	
	VAMoveTo(cx,cy);
	x2=(dx>>1)+y4;				y2=(dy>>1)-x4;

	VAFractalLineTo(cx+x2,cy+y2,FRACTALFACTOR,FRACTALDIV);
	VAFractalLineTo(x1,y1,FRACTALFACTOR,FRACTALDIV);

	x2=-(dx>>2)-y4;				y2=-(dy>>2)+x4;

	VAFractalLineTo(x1+x2,y1+y2,FRACTALFACTOR,FRACTALDIV);
	VAFractalLineTo(x1-x8,y1-y8,FRACTALFACTOR,FRACTALDIV);

	x2=(dx>>1)+y8;				y2=(dy>>1)-x8;

	VAFractalLineTo(cx+x2,cy+y2,FRACTALFACTOR,FRACTALDIV);
	VAFractalLineTo(cx+x8,cy+y8,FRACTALFACTOR,FRACTALDIV);

	x2=(dx>>2)-y4;				y2=((dy)>>2)+x4;

	VAFractalLineTo(cx+x2,cy+y2,FRACTALFACTOR,FRACTALDIV);
	VAFractalLineTo(cx,cy,FRACTALFACTOR,FRACTALDIV);
}
#endif
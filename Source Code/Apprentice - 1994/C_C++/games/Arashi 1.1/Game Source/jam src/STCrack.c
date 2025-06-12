/*/
     Project Arashi: STCrack.c
     Major release: Version 1.1, 7/22/92

     Last modification: Saturday, December 19, 1992, 14:39
     Created: Saturday, March 25, 1989, 14:41

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "STCrack.h"

#define	MAXCRACKS	60 /* changed 12-20-92 from 40 */

typedef	struct
{	int		x,y;
	int		xv,yv;
	int		size;
	int		color;
	int		partcode;
	int		rotation;
	int		rotspeed;
	int		count;
}	Crack;

Crack	*Cracks;
int		numcracks;

void	FlipperHalf1(int,int,int,int);
void	FlipperHalf2(int,int,int,int);
void	DrawPulsar(int,int,int,int,int);

void	PlayerHalf1(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int		x2,y2,x1,y1;
				int		x4,y4,x8,y8;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	
	VAMoveTo(cx,cy);
	x2=(dx>>1)+y4;				y2=(dy>>1)-x4;

	VASafeLineTo(cx+x2,cy+y2);

	x2=(dx>>1)+y8;				y2=(dy>>1)-x8;

	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(cx+x8,cy+y8);

	x2=x4-y4;					y2=y4+x4;

	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(cx,cy);
}
void	PlayerHalf2(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int		x2,y2,x1,y1;
				int		x4,y4,x8,y8;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	
	VAMoveTo(cx,cy);
	x2=(dx>>1)+y4;				y2=+(dy>>1)-x4;

	VASafeLineTo(cx-x2,cy+y2);

	x2=(dx>>1)+y8;				y2=(dy>>1)-x8;

	VASafeLineTo(cx-x2,cy+y2);
	VASafeLineTo(cx-x8,cy+y8);

	x2=x4-y4;					y2=y4+x4;

	VASafeLineTo(cx-x2,cy+y2);
	VASafeLineTo(cx,cy);
}
void	OldPlayerHalf2(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int		x2,y2,x1,y1;
				int		x4,y4,x8,y8;
	
	x1=cx+dx;					y1=cy+dy;
	x4=dx>>2;					y4=dy>>2;
	x8=dx>>3;					y8=dy>>3;
	
	VAMoveTo(x1,y1);

	x2=-x4-y4;					y2=-y4+x4;

	VASafeLineTo(x1+x2,y1+y2);
	VASafeLineTo(x1-x8,y1-y8);

	x2=(dx>>1)+y8;				y2=(dy>>1)-x8;

	VASafeLineTo(cx+x2,cy+y2);

	x2=(dx>>1)+y4;				y2=(dy>>1)-x4;

	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(x1,y1);
}

void	FlipperHalf1(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int	x2,y2;
	register	int	dx4,dx8,dy4,dy8;
	
	VA.color=2;
	dx4=dx >> 2;		dy4=dy >> 2;
	dx8=dx >> 3;		dy8=dy >> 3;

	VAMoveTo(cx,cy);
	x2=dx/2+dy8;		y2=dy/2-dx8;
	VASafeLineTo(cx+x2,cy+y2);
	x2=dx8+dy4;			y2=dy8-dx4;
	VASafeLineTo(cx+x2,cy+y2);
	x2=dx4+dy8;			y2=dy4-dx8;
	VASafeLineTo(cx+x2,cy+y2);
	VASafeLineTo(cx,cy);
}
void	FlipperHalf2(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int	x1,y1,x2,y2;
				int	dx4,dx8,dy4,dy8;
	
	VA.color=2;
	dx4=dx >> 2;		dy4=dy >> 2;
	dx8=dx >> 3;		dy8=dy >> 3;
	x1=cx+dx;			y1=cy+dy;

	VAMoveTo(x1,y1);
	x2=dx/2+dy8;		y2=dy/2-dx8;
	VASafeLineTo(cx+x2,cy+y2);
	x2=dx8-dy4;			y2=dy8+dx4;
	VASafeLineTo(x1-x2,y1-y2);
	x2=dx4-dy8;			y2=dy4+dx8;
	VASafeLineTo(x1-x2,y1-y2);
	VASafeLineTo(x1,y1);
}

void	UpdateCracks()
{
				int		i;
	register	Crack	*cp;
	register	int		dx,dy;
	
	cp=Cracks;
	for(i=0;i<numcracks;i++)
	{	cp->x+=cp->xv;
		cp->y+=cp->yv;
		cp->rotation+=cp->rotspeed;
		while(cp->rotation>=ANGLES)	cp->rotation-=ANGLES;
		while(cp->rotation< 0)		cp->rotation+=ANGLES;

		if(!VA.Late)
		{	dx= (cp->size * (long)Cosins[cp->rotation]) >> 8;
			dy= (cp->size * (long)-Sins[cp->rotation]) >> 8;
			VA.color=cp->color;
	
			switch(cp->partcode)
			{	case FlipperLeft:
					FlipperHalf1(cp->x,cp->y,dx,dy);
					break;
				case FlipperRight:
					FlipperHalf1(cp->x,cp->y,dx,dy);
					break;
				case PlayerLeft:
					PlayerHalf1(cp->x,cp->y,dx,dy);
					break;
				case PlayerRight:
					PlayerHalf2(cp->x,cp->y,dx,dy);
					break;
				case PulsarCrack:
					DrawPulsar(cp->x,cp->y,dx,dy,30-(cp->count*2));
					break;
				case FlyingVector:
					dx>>=1;
					dy>>=1;
					VAMoveTo(cp->x-dx,cp->y-dy);
					VASafeLineTo(cp->x+dx,cp->y+dy);
					break;
			}
		}

		if(--cp->count <= 0)
		{	*cp--=Cracks[--numcracks];
			i--;
		}
		cp++;
	}
}
void	AddCrack(x,y,xv,yv,color,unitsize,part,rot,rotspeed,count)
int		x,y,xv,yv,color,unitsize,part,rot,rotspeed,count;
{
	register	Crack	*cp;
	
	if(numcracks < MAXCRACKS)
	{	cp=Cracks+numcracks++;
		cp->x=x;
		cp->y=y;
		cp->xv=xv;
		cp->yv=yv;
		cp->color=color;
		cp->partcode=part;
		cp->rotation=rot;
		cp->rotspeed=rotspeed;
		cp->size=unitsize;
		cp->count=count;
	}
}
void	AllocCracks()
{
	Cracks=(Crack *)NewPtr(sizeof(Crack)*MAXCRACKS);
	numcracks=0;
}

void	InitCracks()
{
	numcracks=0;
}
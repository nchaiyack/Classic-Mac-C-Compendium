/*/
     Project Arashi: STSpikes.c
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, December 14, 1992, 22:04
     Created: Friday, March 24, 1989, 13:07

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"

extern	Shot	shots[];
extern	int		shotcount;
extern	Player	Hero;

#define	SPIKERSPEED			(ThisLevel.spSpeed)
#define	SPIKERTOP			(ThisLevel.spTop)
#define	SPIKERCOLOR			(ThisLevel.spColor[0])
#define	PLASMASPEED			(ThisLevel.spPlSpeed)
#define	PLASMAPRIMECOLOR	(ThisLevel.spColor[1])
#define	PLASMASECCOLOR		(ThisLevel.spColor[2])
#define	PLASMARANGE			5
#define	SPIKEHITDROP		(ThisLevel.shPower)

typedef struct
{	int		top;
	IFixed	ball;
	Fixed	direction;
	IFixed	plasma;
}	Spiker;

			Spiker	*Spikers;
static		int		SpikeRotAngle,SpikeUpdate;


void	DrawSpiker(cx,cy,size,angle)
register	int		cx,cy,size,angle;
{
	register	int		dx,dy,i;
	
	VA.color=SPIKERCOLOR;

	size>>=2;
	dx=(size * Cosins[angle])/256;
	dy=(size * Sins[angle]  )/256;

	for(i=0;i<5;i++)
	{	angle+=(ANGLES/5);
		if(angle>=ANGLES)	angle-=ANGLES;
		VAMoveTo(cx+dx,cy+dy);
		dx=(size * Cosins[angle])/256;
		dy=(size * Sins[angle]  )/256;
		VALineTo(cx+dx,cy+dy);
	}
}


void	DrawPlasma(lane,level)
int		lane,level;
{
	register		int		dx,dy,x,y,angle,i;
					long	size;

	size=ww.unitlen[level];

	x=ww.xc[lane][level];
	y=ww.yc[lane][level];

	angle=(level*21+ANGLES) % ANGLES;

	VAMoveTo(x,y);
	for(i=0;i<3;i++)
	{	angle+=ANGLES/3;
		if(angle>=ANGLES)	angle-=ANGLES;
	
		dx = (Cosins[angle]*size)>>10;
		dy = (Sins[angle]*size)>>10;
		VA.color=PLASMAPRIMECOLOR;
		VASafeLineTo(x+dx,y+dy);
		VA.color=PLASMASECCOLOR;
		VASafeSpot(x-(dx>>1),y-(dy>>1));
	}
}

void	ChangeSpike(lane,level)
int		lane,level;
{
	register	int		x,y,soff,scol,sfield;

	soff=VA.offset;
	sfield=VA.field;
	scol=VA.color;
	VA.field=1;
	VA.offset=0;

	if(Spikers[lane].top<DEPTH)
	{	VA.color=1;
		x=ww.xc[lane][Spikers[lane].top];
		y=ww.yc[lane][Spikers[lane].top];

		VAStaticLine(x,y,ww.xc[lane][DEPTH],ww.yc[lane][DEPTH]);
	}
	Spikers[lane].top=level;

	if(level<DEPTH)
	{	VA.color=0;
		VAStaticLine(	ww.xc[lane][level],
					ww.yc[lane][level],
					ww.xc[lane][DEPTH],
					ww.yc[lane][DEPTH]);
	}
	VA.field=sfield;
	VA.offset=soff;
	VA.color=scol;
}
void	DrawSpike(lane)
int		lane;
{
	register	int		x,y,level,soff,scol,sfield;

	soff=VA.offset;
	sfield=VA.field;
	scol=VA.color;
	VA.field=1;
	VA.offset=0;

	VA.color=0;
	level=Spikers[lane].top;
	if(level<DEPTH)
	{	VAStaticLine(	ww.xc[lane][level],
					ww.yc[lane][level],
					ww.xc[lane][DEPTH],
					ww.yc[lane][DEPTH]);
	}
	VA.field=sfield;
	VA.offset=soff;
	VA.color=scol;
}

void	DropSpike(lane)
register	int		lane;
{
	register	int		x,y;
	register	int		soff,scol,sfield;
	
	soff=VA.offset;
	sfield=VA.field;
	scol=VA.color;
	VA.field=1;
	VA.offset=0;
	
	x=ww.xc[lane][Spikers[lane].top-Hero.flydepth];
	y=ww.yc[lane][Spikers[lane].top-Hero.flydepth];

	VAExplosion(x,y,Spikers[lane].top>DEPTH/2 ? 0 : 1 ,5);
	IncreaseScore(ThisLevel.spSpikePoints);

	if(Hero.flydepth)
	{	Spikers[lane].top+=SPIKEHITDROP;
		
		if(Spikers[lane].top>=DEPTH)
		{	Spikers[lane].top=DEPTH+999;
		}
	}
	else
	{	VA.color=1;
		VAStaticLine(x,y,ww.xc[lane][DEPTH],ww.yc[lane][DEPTH]);
		
		VA.color=0;
	
		Spikers[lane].top+=SPIKEHITDROP;
		if(Spikers[lane].top>=DEPTH)
		{	Spikers[lane].top=DEPTH+999;
		}
		else
		{	VAStaticLine(	ww.xc[lane][Spikers[lane].top],
						ww.yc[lane][Spikers[lane].top],
						ww.xc[lane][DEPTH],
						ww.yc[lane][DEPTH]);
		}
	}

	VA.field=sfield;
	VA.offset=soff;
	VA.color=scol;
}
void	KillSpiker(lane)
register	int		lane;
{
	register	int		x,y;

	if(Spikers[lane].direction && (Spikers[lane].ball.i < DEPTH))
	{	x=ww.xc[lane][Spikers[lane].ball.i];
		y=ww.yc[lane][Spikers[lane].ball.i];
		VAExplosion(x,y,Spikers[lane].ball.i>DEPTH/2 ? 1:2,0);
		PlayB(Dziung,0);
		IncreaseScore(ThisLevel.spPoints);
		Spikers[lane].direction=0;
	}
}

void	BlowPlasma(lane)
register	int		lane;
{
	VAExplosion(ww.xc[lane][Spikers[lane].plasma.i]+1,
				ww.yc[lane][Spikers[lane].plasma.i]+1,
				1,
				PLASMASECCOLOR);
	PlayB(Dziung,0);
	IncreaseScore(ThisLevel.spPlPoints);
	Spikers[lane].plasma.i= DEPTH+1;

}
void	SpikeShotTests()
{
	register	int		i;
	register	Shot	*shp;
	register	int		shotlevel;

	if(Hero.superzapping==1 && Hero.flydepth==0)
	{	for(i=0;i<ww.numsegs;i++)
		{	if(Spikers[i].ball.i<DEPTH)		KillSpiker(i);
			if(Spikers[i].plasma.i<DEPTH)	BlowPlasma(i);
		}
	}

	shp=shots;
	for(i=0;i<shotcount;i++)
	{	shotlevel=shp->level+Hero.flydepth;
		if(shotlevel>Spikers[shp->lane].top)
		{	
			if(shotlevel<=DEPTH)
				DropSpike(shp->lane);	
			
			if(shotlevel>Spikers[shp->lane].ball.i)
				KillSpiker(shp->lane);
			*shp--=shots[--shotcount];
			i--;
		}
		else if(Hero.flydepth==0)
		{	if(shotlevel>Spikers[shp->lane].plasma.i)
			{	ExplodeShot(shp->lane,shotlevel);
				BlowPlasma(shp->lane);
				*shp--=shots[--shotcount];
				i--;
			}
		}
		shp++;
	}
}

void	UpdateSpikes()
{
	register	int		i,j,k;
	
	SpikeRotAngle+=5;
	
	if(SpikeRotAngle < 40)
	{	if(++SpikeUpdate >= ww.numsegs)
			SpikeUpdate=0;
		if(Spikers[SpikeUpdate].top < DEPTH )
			DrawSpike(SpikeUpdate);
	}
	if(SpikeRotAngle>=ANGLES) SpikeRotAngle-=ANGLES;
	
	VA.color=0;
	for(i=0;i<ww.numsegs;i++)
	{	j=Spikers[i].top;
		if ((j == DEPTH) && (Spikers[i].direction > 0)) /* This tells when a Spiker */
		{						/* reaches the bottom.  Generate a Tanker? */
		}
		
		if(Spikers[i].direction)
		{	Spikers[i].ball.f+=Spikers[i].direction;
			k = Spikers[i].ball.i;
			
			if(k<=SPIKERTOP)
			{	Spikers[i].direction = SPIKERSPEED;	/* Spiker at top, turning around */
			}
			else
			{	if(k>=DEPTH) Spikers[i].direction= 0;
			}
			
			if(k<j)
			{	j=k;
				ChangeSpike(i,j);
			}
			
			if(Spikers[i].direction)
			{	DrawSpiker(ww.xc[i][k],ww.yc[i][k],ww.unitlen[k],SpikeRotAngle);
			}
			
			Spikers[i].ball.i = k;
		}
		else
		{	if(VAPosRandom()<ThisLevel.spProb)
			{	Spikers[i].direction= -SPIKERSPEED;
				Spikers[i].ball.i=DEPTH;
				if(Spikers[i].plasma.i>DEPTH && ThisLevel.spPlasma)
				{
					Spikers[i].plasma.i=DEPTH;
				}
			}
		}
		if(Spikers[i].plasma.i<=DEPTH)
		{	Spikers[i].plasma.f-=PLASMASPEED;

			DrawPlasma(i,Spikers[i].plasma.i);
			if(Spikers[i].plasma.i<0)
			{	PlayB(Whiz,0);
				Spikers[i].plasma.i=DEPTH+1;
				Hero.lanestat[i]|=PlasMask;
			}
			else
			{	if(Spikers[i].plasma.i<PLASMARANGE)
					Hero.lanestat[i]|=PlasMask;
			}
		}

		if(j<DEPTH)
		{	VA.color=ThisLevel.spColor[1];
			VAPixel(ww.xc[i][j],ww.yc[i][j]);
		}
	}
	SpikeShotTests();
}
void	InitSpikes()
{
	register	int		i;
	register	int		soff,scol,sfield;

	SpikeRotAngle=0;
	SpikeUpdate=0;

	soff=VA.offset;
	sfield=VA.field;
	scol=VA.color;

	VA.color=0;
	VA.field=1;
	VA.offset=0;
	
	for(i=0;i<ww.numsegs;i++)
	{	Spikers[i].direction=0;
		Spikers[i].plasma.i=DEPTH+1;	/*	No plasma flying	*/

		if(ThisLevel.spStart>=0)
			Spikers[i].top=ThisLevel.spStart;

		if(Spikers[i].top<DEPTH)
			VAStaticLine(	ww.xc[i][DEPTH],ww.yc[i][DEPTH],
							ww.xc[i][Spikers[i].top],ww.yc[i][Spikers[i].top]);
	}
	
	ThisLevel.spStart= -1;

	VA.field=sfield;
	VA.offset=soff;
	VA.color=scol;
}

void	RedrawSpikes()
{
	register	int		i,level;

	VA.field=1;
	VA.offset=0;
	
	for(i=0;i<ww.numsegs;i++)
	{	level=Spikers[i].top;
		if(level<DEPTH)
		{	VAStaticLine(	ww.xc[i][DEPTH],ww.yc[i][DEPTH],
							ww.xc[i][level],ww.yc[i][level]);
		}
		Spikers[i].direction=0;
	}
}

void	FlySpikes(color)
int		color;
{
				Rect	toclip;
	register	int		toplevel,botlevel;
	register	int		i;

	SpikeShotTests();

	botlevel=DEPTH-Hero.flydepth;

	for(i=0;i<ww.numsegs;i++)
	{	toplevel=Spikers[i].top-Hero.flydepth;
		if(toplevel<=ZOOMDEPTH)
			toplevel=ZOOMDEPTH+1;
	
		if(toplevel < botlevel)
		{	if(toplevel <= 0)
			{	Hero.lanestat[i] |= SpikeMask;
			}
			
			toclip.left=ww.xc[i][botlevel];
			toclip.top=ww.yc[i][botlevel];
			toclip.right=ww.xc[i][toplevel];
			toclip.bottom=ww.yc[i][toplevel];
		
			VA.color=ThisLevel.spColor[1];
			VASafePixel(toclip.right,toclip.bottom);

			VA.color=color;
			if(VAClip(&toclip))
			{	VALine(toclip.left,toclip.top,toclip.right,toclip.bottom);
			}
		}
	}
}

void	AllocSpikers()
{
	Spikers=(Spiker *)NewPtr(sizeof(Spiker)*MAXSEGS);	
}
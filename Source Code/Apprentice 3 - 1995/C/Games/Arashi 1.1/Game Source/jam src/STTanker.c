/*/
     Project Arashi: STTanker.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, January 3, 1993, 10:09
     Created: Tuesday, February 5, 1991, 4:25

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"

#define	MAXTANKERS		10
#define	TANKERCOLOR		(ThisLevel.ftColor)
#define	TANKERUPSPEED	(ThisLevel.ftSpeed)

typedef	struct
{
	int		tanktype;	/*	FlipTank, FuseTank, PulsTank		*/
	int		state;		/*	star=0, inactive=1, active=2		*/
	IFixed	level;
	int		lane;
}	Tanker;

enum	{	star, inactive, active	};

extern		Player	Hero;
Tanker		*Tankers;
int			ActiveTankers;
int			ActiveTypes[TANKTYPES];

void	CreateSplitFlippers(int lane,int level);

void	DrawTanker(cx,cy,dx,dy,tanktype)
int		cx,cy,dx,dy,tanktype;
{
	register	int	x1,y1,x2,y2;
	
	VA.color=ThisLevel.tk[tanktype].color;
	x1= dx >> 1;
	y1= dy >> 1;
	x2= dx >> 2;
	y2= dy >> 2;
	VAMoveTo(cx,cy);
	VALineTo(cx+x1+y1	,cy+y1-x1);
	VALineTo(cx+dx		,cy+dy);
	VALineTo(cx+x1-y1	,cy+y1+x1);
	VALineTo(cx			,cy);
	VALineTo(cx+x2		,cy+y2);
	VALineTo(cx+x1-y2	,cy+y1+x2);
	VALineTo(cx+dx-x2	,cy+dy-y2);
	VALineTo(cx+x1+y2	,cy+y1-x2);
	VALineTo(cx+x2		,cy+y2);
	VAMoveTo(cx+dx-x2	,cy+dy-y2);
	VALineTo(cx+dx		,cy+dy);
}
/*
>>	Create a newborn tanker. A tanker appears
>>	as one of the center stars, so the lane or
>>	vertex is decided when the star touches the
>>	playfield. Some of this stuff could just as 
>>	well be done at that time.
*/
void	CreateNewTanker(tanktype)
int		tanktype;
{
	register	Tanker		*thetank;

	if(ActiveTankers<MAXTANKERS)
	{	thetank=Tankers;
		while(thetank->state!=inactive)
		{	thetank++;
		}
		thetank->tanktype=tanktype;
		thetank->state=star;
		thetank->lane=0;
		thetank->level.f=0;
		thetank->level.i=DEPTH;
	
		StarApproach(&(thetank->state),ThisLevel.tk[tanktype].color);
		ActiveTankers++;
		ActiveTypes[tanktype]++;
	}
}

/*
>>	This routine is called for each active tanker.
>>	An active tanker has gone through the star
>>	phase and now rotates and climbs up.
*/
void		UpdateActiveTanker(thetank)
register
Tanker	*thetank;
{
	register	int		dx,dy,x,y;
	register	int		split=0;
	register	int		zap;
	
	thetank->level.f-=ThisLevel.tk[thetank->tanktype].speed;	/*	Move up.	*/
	if(thetank->level.i<=0)				/*	Don't move past the top.			*/
	{	thetank->level.i=0;
		Hero.lanestat[thetank->lane] |= FlipMask;
		split=1;
	}
	else
	{	/*	Calculate the position and width of the tanker.							*/
		x=ww.x[thetank->lane][thetank->level.i];	/*	Find position on screen.	*/
		y=ww.y[thetank->lane][thetank->level.i];
	
		dx=ww.x[thetank->lane+1][thetank->level.i]-x;	/*	Calculate size.			*/
		dy=ww.y[thetank->lane+1][thetank->level.i]-y;
	
		if(!VA.Late)				/*	Is there time to draw something?	*/
		{	DrawTanker(x,y,dx,dy,thetank->tanktype);
		}
		
		/*	Test to see if the player has hit us.							*/
		if(zap = ShotHitTest(thetank->lane,thetank->level.i)) /* mz */
		{	PlayB(Zroom,11);
			IncreaseScore(ThisLevel.tk[thetank->tanktype].points);
			if (zap != -2)
				split=1;			/* mz */
			else
			{
				ThisLevel.totalCount--;
				thetank->state=inactive;
				ActiveTankers--;
				ActiveTypes[thetank->tanktype]--;
				ThisLevel.tk[thetank->tanktype].count--;
			}
		}
	}

	if(split)
	{	
		ThisLevel.tk[thetank->tanktype].count--;
		switch(thetank->tanktype)
		{	case FlipTank:
				ThisLevel.flCount+=2;
				CreateSplitFlippers(thetank->lane,thetank->level.i);
				break;
			case FuseTank:
				ThisLevel.fuCount+=2;
				CreateSplitFuseBalls(thetank->lane,thetank->level.i);
				break;
			case PulsTank:
				ThisLevel.puCount+=2;
				CreateSplitPulsars(thetank->lane,thetank->level.i);
				break;
		}
		ThisLevel.totalCount++;
		thetank->state=inactive;
		ActiveTankers--;
		ActiveTypes[thetank->tanktype]--;
	}
}
/*
>>	Go through the tankers checking their state and
>>	promoting them from stars to fully grown tanker,
>>	when they reach the playfield.
*/
void UpdateTankers()
{
	register	Tanker		*tank;
	register	int			i;
	
	tank=Tankers;
	for(i=0;i<MAXTANKERS;i++)
	{	if(tank->state<0)
		{	tank->lane= -(tank->state + 1)/STARDIVISION;
			tank->state= active;
			if(ww.wraps)
			{	if(tank->lane>=ww.numsegs)
					tank->lane=ww.numsegs-1;
			}
			else
			{	if(tank->lane>ww.numsegs-2)
					tank->lane=ww.numsegs-2;
				else
				if(tank->lane<=0)
					tank->lane=1;
			}
		}
		else
		{	if(tank->state==active)
				UpdateActiveTanker(tank);
		}
		tank++;
	}
	
	/* this code should also be activated by the re-descent of a Spiker (mz) */
	/* add condition TankerPermission? (mz) */
	for(i=0;i<TANKTYPES;i++)
	{	if(ActiveTankers<MAXTANKERS && ThisLevel.tk[i].count>ActiveTypes[i])
		{	if(VAPosRandom() < ThisLevel.tk[i].prob+ThisLevel.probIncrease)
			{	CreateNewTanker(i);
			}
		}
		ThisLevel.starCount += ThisLevel.tk[i].count-ActiveTypes[i];
		ThisLevel.activeCount += ActiveTypes[i];

	}
}

void InitTankers()
{
	register	int		i;
	
	for(i=0;i<MAXTANKERS;i++)
	{	Tankers[i].state=inactive;
	}
	ActiveTankers=0;
	for(i=0;i<TANKTYPES;i++)
	{	ActiveTypes[i]=0;
	}
}

void AllocTankers()
{
	Tankers=(Tanker *)NewPtr(sizeof(Tanker)*MAXTANKERS);
}

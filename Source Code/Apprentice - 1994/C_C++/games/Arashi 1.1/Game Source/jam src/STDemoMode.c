/*/
     Project Arashi: STDemoMode.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:44
     Created: Tuesday, January 15, 1991, 20:56

     Copyright � 1991-1992, Juri Munkki
/*/

#include "VA.h"
#include "Storm.h"

#define	GONSIDES	10

extern	EventRecord	Event;

/*	Demo modes:
*/
enum	{	Entering, Rotating, Exiting, Ended	};

static	int		demomode;
static	int		distance;
static	int		pentalevel;
static	int		centerlevel;
static	int		subgonlevel;

static	Point	penta[GONSIDES+1];
static	Point	subpenta[GONSIDES/2];
static	int		behindflag[GONSIDES];
static	int		baseangle=0;
static	int		centerx,centery;

int		titlescale;

void	CalcGonSides()
{
	int		i,angle;
	int		x,y;
	
	angle=baseangle;
	for(i=0;i<GONSIDES;i++)
	{	x=Cosins[angle];
		y=Sins[angle]+distance;
		
		behindflag[i]=(y>distance);
		penta[i].h=centerx+(x*(long)titlescale)/y;
		penta[i].v=centery+(pentalevel*(long)titlescale)/y;
		angle+=ANGLES/GONSIDES;
		if(angle>=ANGLES) angle-=ANGLES;
	}
	
	penta[GONSIDES].h=centerx;
	penta[GONSIDES].v=centery+(centerlevel*(long)titlescale)/distance;
	
	angle=baseangle+ANGLES/GONSIDES;
	for(i=0;i<GONSIDES/2;i++)
	{	angle+=ANGLES/(GONSIDES/2);
		if(angle>=ANGLES) angle-=ANGLES;

		x=Cosins[angle]/3;
		y=(Sins[angle]/3)+distance;

		subpenta[i].h=centerx+(x*(long)titlescale)/y;
		subpenta[i].v=centery+(subgonlevel*(long)titlescale)/y;
	}

	baseangle+= 1;
	if(baseangle>=ANGLES) baseangle-=ANGLES;
}

void	DrawMainGon()
{
	int		i;
	
	VA.color=5;
	VAMoveTo(penta[GONSIDES-1].h,penta[GONSIDES-1].v);
	for(i=0;i<GONSIDES;i++)
	{	VASafeLineTo(penta[i].h,penta[i].v);
	}
	
	VAMoveTo(penta[0].h,penta[0].v);
	for(i=0;i<GONSIDES*2+2;i+=4)
	{	VASafeLineTo(penta[i%GONSIDES].h,penta[i%GONSIDES].v);
	}
	
	for(i=0;i<GONSIDES;i+=2)
	{	if(behindflag[i])
		{	VA.color=0;
		}
		else
		{	VA.color=5;
		}
		
		if(Event.modifiers & optionKey)
		{	VAFractalLine(penta[GONSIDES].h,penta[GONSIDES].v,penta[i].h,penta[i].v,1<<7,3);
		}
		else
		{	VAMoveTo(penta[GONSIDES].h,penta[GONSIDES].v);
			VASafeLineTo(penta[i].h,penta[i].v);
		}
	}
}

void	DrawSubGon(color)
int		color;
{
	int		i;

	VA.color=color;
	VAMoveTo(subpenta[GONSIDES/2-1].h,subpenta[GONSIDES/2-1].v);
	for(i=0;i<GONSIDES/2;i++)
	{	VASafeLineTo(subpenta[i].h,subpenta[i].v);
	}
}
void	DemoMode()
{
	VAEraseBuffer();
	VAInitFractalLines();

	centerx=(VA.frame.right-VA.frame.left)/2;
	centery=(VA.frame.bottom-VA.frame.top)/2;

	if(centerx>centery)	titlescale=centery;
	else				titlescale=centerx;
		
	demomode=Entering;
	distance=8000;
	pentalevel=150;
	centerlevel=-300;
	subgonlevel=10000;
	
	do
	{	if(!GetNextEvent(everyEvent,&Event))	Event.what=0;
		switch(Event.what)
		{	case keyDown:
				subgonlevel=pentalevel;
				break;
			case mouseDown:
				PlayA(Blast,100);
				PlayB(Blast,100);
				if(demomode==Rotating)
					demomode=Exiting;
				else
					demomode=Ended;
				break;
		}
		
		switch(demomode)
		{	case Entering:
				distance-=75;
				if(distance<500)
				{	distance=500;
					demomode=Rotating;
				}
				break;
			case Rotating:
				break;
			case Exiting:
				pentalevel=pentalevel*3/4;
				centerlevel=centerlevel*3/4;
				if(pentalevel<=centerlevel)
					demomode=Ended;
				break;
		}
				
		CalcGonSides();
		DrawMainGon();
		
		GlobalToLocal(&Event.where);
		VA.color=4;
		VAFractalLine(penta[GONSIDES].h,penta[GONSIDES].v,Event.where.h,Event.where.v,1<<7,5);
		
		if(subgonlevel<=pentalevel)
		{	DrawSubGon(2);
			subgonlevel-=10;
			if(subgonlevel<-400)
				subgonlevel=10000;
		}

		VAStep();
	}	while(demomode!=Ended);
	
	VACloseFractalLines();
}
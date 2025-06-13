/*/
     Project Arashi: STPulsar.c
     Major release: Version 1.1, 7/22/92

     Last modification: Saturday, January 16, 1993, 12:01
     Created: Sunday, February 12, 1989, 22:58

     Copyright � 1989-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "STCrack.h"

#define	MAXPULSARS		20
#define	PULSARCOLOR		(ThisLevel.puColor)
#define	PULSARROTSPEED	(ThisLevel.puRot)
#define	PULSARUPSPEED	(ThisLevel.puSpeed)
#define	PULSINGTIME		(ThisLevel.puTime)
#define	NERVOUSNESS		(PULSINGTIME>>2)

typedef	struct
{
	int		state;		/*	star=0, inactive=1, active=2		*/
	int		pulscounter;/*	While this is >0, pulsar is pulsing.*/
	int		lane;		/*	Actual lane number					*/
	IFixed	level;		/*	Depth level of pulsar				*/
	int		vertex;		/*	vertex currently connected to		*/
	int		mirror;		/*	Draw mirror image?					*/
	int		rotation;	/*	Current rotation of pulsar			*/
	int		direction;	/*	Rotation direction					*/
}	Pulsar,*PulsarPtr;

enum	{	star, inactive, active	};	/*	Pulsar states		*/

Pulsar	*Pulsars;		/*	Pointer to an array of pulsars.		*/
int		ActivePulsars;	/*	The number of active pulsars.		*/
int		PulsarPower;

extern	Player	Hero;	/*	Player status record.				*/
extern
EventRecord		Event;	/*	Event record. DEBUG USE ONLY!		*/

/*
>>	Given a connecting point (cx,cy) and a vector (dx,dy)
>>	a pulsar shape is drawn as quickly as possible.
*/
void	DrawPulsar(cx,cy,dx,dy,power)
int		cx,cy,dx,dy,power;
{
	register	int		normx,normy;
	register	int		ex,ey;
	register	int		bx,by;
	
	dx>>=1;
	dy>>=1;
	ex=cx+dx;
	ey=cy+dy;


	dx -= (power * dx)>>5;
	dy -= (power * dy)>>5;

	bx = dx/3;
	by = dy/3;

	normx=(-dy*power)>>4;
	normy=( dx*power)>>4;
	
	/* if(normy==0 && normx==0) */ /* Tests for when pulsar is most flat */
		

	VA.color=PULSARCOLOR;
	VAMoveTo	(ex+dx,			ey+dy);
	VASafeLineTo(ex+dx-bx+normx,ey+dy-by+normy);
	VASafeLineTo(ex+bx-normx,	ey+by-normy);
	VASafeLineTo(ex+normx,		ey+normy);
	VASafeLineTo(ex-bx-normx,	ey-by-normy);
	VASafeLineTo(ex-dx+bx+normx,ey-dy+by+normy);
	VASafeLineTo(ex-dx,			ey-dy);
}
/*
>>	Create a newborn pulsar. A pulsar appears
>>	as one of the center stars, so the lane or
>>	vertex is decided when the star touches the
>>	playfield. Some of this stuff could just as 
>>	well be done at that time.
*/
void	CreateNewPulsar()
{
	register	Pulsar		*thepulsar;

	if(ActivePulsars<MAXPULSARS)
	{	thepulsar=Pulsars;
		while(thepulsar->state!=inactive)
		{	thepulsar++;
		}
		thepulsar->state=star;
		thepulsar->lane=0;
		thepulsar->level.f=0;
		thepulsar->level.i=DEPTH;
		thepulsar->mirror=0;
		thepulsar->pulscounter=0;
	
		thepulsar->direction= (VARandom()<0)?PULSARROTSPEED:
										-PULSARROTSPEED;
		StarApproach(&(thepulsar->state),PULSARCOLOR);
		ActivePulsars++;
	}
}
/*
>>	This routine creates two pulsars from a destroyed
>>	tanker.
*/
void	CreateSplitPulsars(lane,level)
int		lane,level;
{
	register	int			i,delta,nextseg,prevseg;
	register	Pulsar		*thepulsar;
	
	for(i=0;i<2;i++)
	{	if(ActivePulsars<MAXPULSARS)
		{	thepulsar=Pulsars;
			while(thepulsar->state!=inactive)
			{	thepulsar++;
			}
			thepulsar->vertex= lane+i;
			if(thepulsar->vertex>=ww.numsegs)
				thepulsar->vertex -= ww.numsegs;

			thepulsar->state= active;
			thepulsar->level.f=0;
			thepulsar->level.i=level;
			thepulsar->mirror=0;
			thepulsar->pulscounter=0;
					
			thepulsar->direction= i ? PULSARROTSPEED : -PULSARROTSPEED;

			nextseg=NextSeg[thepulsar->vertex];
			prevseg=PrevSeg[thepulsar->vertex];

			delta=nextseg-prevseg;
			if(delta<0) delta+=ANGLES;
			
			if(thepulsar->direction < 0)
			{	thepulsar->rotation=NextSeg[thepulsar->vertex]-delta/2;
			}
			else
			{	thepulsar->rotation=PrevSeg[thepulsar->vertex]+delta/2;
			}

			thepulsar->lane=LaneSel[thepulsar->vertex][thepulsar->rotation];

			ActivePulsars++;
		}
	}
}

/*
>>	A pulsar moves by rotating from one vertex
>>	to another. It "grabs" onto the vertex while
>>	rotating, so at times it needs to change
>>	the way it touches. The mirror value is
>>	used for properly drawing the pulsar when
>>	this change happens.
*/
void	PulsarSegSkip(thepulsar)
register
PulsarPtr	thepulsar;
{
	thepulsar->rotation+=ANGLES/2;
	if(thepulsar->rotation>=ANGLES)	thepulsar->rotation-=ANGLES;
	thepulsar->mirror= !thepulsar->mirror;

	if(thepulsar->level.i==0)
		Hero.lanestat[LaneSel[thepulsar->vertex][thepulsar->rotation]] |= PulsMask;
}

/*
>>
*/

void	DoPulsing(pulsarp)
PulsarPtr	pulsarp;
{
	pulsarp->pulscounter=PULSINGTIME;
}
/*
>>	This routine is called for each active pulsar.
>>	An active pulsar has gone through the star
>>	phase and now rotates and climbs up.
*/
void		UpdateActivePulsar(pulsarp)
register
PulsarPtr	pulsarp;
{
	register	int		dx,dy,x,y;
				int		sc;
				int		doskip;

	pulsarp->level.f-=PULSARUPSPEED;/*	Move up a bit.							*/
	if(pulsarp->level.i<=0)			/*	Don't move past the top.				*/
	{	pulsarp->level.i=0;
		ThisLevel.edgeCount++;		/*	Pulsar is on the edge.					*/
	}

	y=pulsarp->direction;	/*	I find this ugly, but it needs to be fast.		*/

	if(pulsarp->pulscounter==0)
	{	x=pulsarp->rotation;/*	x is now the current rotation value.			*/
		dx=NextSeg[pulsarp->vertex]-x;	/*	Find next clockwise vertex angle.	*/
		dy=PrevSeg[pulsarp->vertex]-x;	/*	Find next counter-cw vertex angle.	*/
								/*	Note that the above angles are relative!	*/

		if(dx<-ANGLES)	dx=y+y;	/*	A negative angle is used as a flag value	*/
		if(dy<-ANGLES)	dy=y+y;	/*	to indicate that there is no "next vertex".	*/

		if(y>0)						/*	Clockwise rotation?						*/
		{	if(dx<=0)	dx+=ANGLES;	/*	Force angle between 1 and ANGLES		*/
			if(dy<=0)	dy+=ANGLES;	/*	Same thing here.						*/
			if(dx<dy)				/*	Which vertex is next?					*/
			{	if(y>=dx)			/*	Can we reach the next clockwise vertex?	*/
				{	
					y=0;
					DoPulsing(pulsarp);
					pulsarp->rotation+=dx;	/*	Rotate to this vertex.			*/
					pulsarp->vertex++;		/*	Increment vertex value.			*/

					doskip= 1;		/*	We skipped from a vertex to another.	*/
					if(ww.wraps)	/*	Following code depends on field type.	*/
					{	if(pulsarp->vertex>=ww.numsegs)
						{	pulsarp->vertex=0;	/*	Wrap around to other side.	*/
						}
					}
					else	/*	Bounce back, if hit the edge of the playfield.	*/
					{	if(pulsarp->vertex>=ww.numsegs)
						{	pulsarp->vertex=ww.numsegs-1;
							pulsarp->direction= -pulsarp->direction;
							y= -y;
							doskip= 0;
							if(pulsarp->level.i==0)
								Hero.lanestat[pulsarp->vertex] |= PulsMask;
						}
					}
					if(doskip)		/*	Did we grab a new vertex?				*/
						PulsarSegSkip(pulsarp);
				}
				else	/*	Didn't reach a new vertex. No more rotating to do.	*/
				{	pulsarp->rotation+=y;
					y=0;
				}
			}
			else			/*	Basically the same code as above follows,		*/
			{	if(y>=dy)	/*	 but in the other direction.					*/
				{	y=0;
					DoPulsing(pulsarp);
					pulsarp->rotation+=dy;
					pulsarp->vertex--;
					doskip= 1;
					
					if(ww.wraps)
					{	if(pulsarp->vertex<0)
						{	pulsarp->vertex=ww.numsegs-1;
						}
					}
					else
					{	if(pulsarp->vertex==0)
						{	pulsarp->vertex=1;
							pulsarp->direction= -pulsarp->direction;
							y= -y;
							doskip= 0;
							if(pulsarp->level.i==0)
								Hero.lanestat[0] |= PulsMask;
						}
					}
					if(doskip)
						PulsarSegSkip(pulsarp);
				}
				else
				{	pulsarp->rotation+=y;
					y=0;
				}
			}
			/*	Adjust rotation angle to be between 0 and ANGLES-1.				*/
			if(pulsarp->rotation>=ANGLES) pulsarp->rotation-=ANGLES;
		}
		else	/*	Counterclockwise rotation. Otherwise similar to above code.	*/
		{	if(dx>=0)	dx-=ANGLES;
			if(dy>=0)	dy-=ANGLES;
			if(dx>dy)
			{	if(y<=dx)
				{	
					y=0;
					DoPulsing(pulsarp);
					pulsarp->rotation+=dx;
					pulsarp->vertex++;
					
					doskip= 1;
					if(ww.wraps)
					{	if(pulsarp->vertex>=ww.numsegs)
						{	pulsarp->vertex=0;
						}
					}
					else
					{	if(pulsarp->vertex>=ww.numsegs)
						{	pulsarp->vertex=ww.numsegs-1;
							pulsarp->direction= -pulsarp->direction;
							y= -y;
							doskip= 0;
							if(pulsarp->level.i==0)
								Hero.lanestat[pulsarp->vertex] |= PulsMask;
						}
					}
					if(doskip)
						PulsarSegSkip(pulsarp);
				}
				else	
				{	pulsarp->rotation+=y;
					y=0;
				}
			}
			else
			{	if(y<=dy)
				{	
					y=0;
					DoPulsing(pulsarp);
					pulsarp->rotation+=dy;
					pulsarp->vertex--;
					doskip= 1;
					
					if(ww.wraps)
					{	if(pulsarp->vertex<0)
						{	pulsarp->vertex=ww.numsegs-1;
						}
					}
					else
					{	if(pulsarp->vertex==0)
						{	pulsarp->vertex=1;
							pulsarp->direction= -pulsarp->direction;
							y= -y;
							doskip= 0;
							if(pulsarp->level.i==0)
								Hero.lanestat[0] |= PulsMask;
						}
					}
					if(doskip)
						PulsarSegSkip(pulsarp);
				}
				else
				{	pulsarp->rotation+=y;
					y=0;
				}
			}
			if(pulsarp->rotation<0)	pulsarp->rotation+=ANGLES;
		}
	}
	
	/*	Find out the lane that we belong to currently.	This depends
	**	on the vertex we are connected to and the angle at which we
	**	are hanging.
	*/
	pulsarp->lane=LaneSel[pulsarp->vertex][pulsarp->rotation];
	
	if(pulsarp->pulscounter)
	{	if(ThisLevel.puPulsDepth > pulsarp->level.i)
		{	pulsarp->pulscounter--;
			/* change in pulsar behavior so it waits then pulses, then waits */
			/* in each lane. (mz) */
			if( (pulsarp->pulscounter < (int)(PULSINGTIME*.75)) &&
					(pulsarp->pulscounter > (int)(PULSINGTIME*.30)))
				Hero.lanestat[pulsarp->lane] |= PulsMask;
			if(PulsarPower>=13 && pulsarp->pulscounter<NERVOUSNESS)
			{	pulsarp->pulscounter=0;
			}
		}
		else
		{	pulsarp->pulscounter=0;
		}
	}


	/*	Calculate the position and width of the pulsar.				*/
	dx= ww.unitlen[pulsarp->level.i];	/*	Look up length at this level.	*/
	
	dy= (dx*(long) -Sins[pulsarp->rotation])>>8;	/*	Fixed point rotation.	*/
	dx= (dx*(long)Cosins[pulsarp->rotation])>>8;

	x=ww.x[pulsarp->vertex][pulsarp->level.i];/*	Find position on screen.*/
	y=ww.y[pulsarp->vertex][pulsarp->level.i];
	
	if(!VA.Late)				/*	Is there time to draw something?	*/
	{	if(pulsarp->mirror)		/*	Is the pulsar mirrored?			*/
			DrawPulsar(x+dx,y+dy,-dx,-dy,PulsarPower);
		else
			DrawPulsar(x,y,dx,dy,PulsarPower);
	}

	/*	Notify the player record if a pulsar is on the edge.			*/
/*	if(pulsarp->level.i==0)
	{	Hero.segmstat[pulsarp->vertex] |= PulsMask;
	} */ /* Allow player to slip under rotating pulsar */
	
	/*	Test to see if the player has hit us.							*/
	if(ShotHitTest(pulsarp->lane,pulsarp->level.i))
	{	sc=ww.unitlen[pulsarp->level.i];		/*	Explode into two parts.	*/
		AddCrack(	x,y,
					-dx >> 2,-dy >> 2,
					PULSARCOLOR,sc,PulsarCrack,
					pulsarp->rotation,-2*pulsarp->direction,10);

		ThisLevel.puCount--;
		ThisLevel.totalCount--;

		PlayB(Splitter,13);
		IncreaseScore(ThisLevel.puPoints);
		pulsarp->state=inactive;
		ActivePulsars--;
	}
}

/*
>>	Go through the pulsars checking their state and
>>	promoting them from stars to fully grown pulsar,
>>	when they reach the playfield.
*/
void	UpdatePulsars()
{
	register	Pulsar		*psp;
	register	int			i;

	PulsarPower=(VA.FrameCounter) & 31;
	if(ActivePulsars && PulsarPower==30)
	{	PlayB(Phazer2,9);
	}

	if(PulsarPower>15)
		PulsarPower=31-PulsarPower;
	
	psp=Pulsars;
	for(i=0;i<MAXPULSARS;i++)
	{	if(psp->state<0)
		{	psp->vertex= -(psp->state + 1)/STARDIVISION;
			psp->state= active;
			if(!ww.wraps && psp->vertex==0)	psp->vertex=1;
			if(psp->vertex>ww.numsegs)		psp->vertex=ww.numsegs;

			if(psp->direction < 0)
			{	psp->rotation=NextSeg[psp->vertex];
			}
			else
			{	psp->rotation=PrevSeg[psp->vertex];
			}
		}
		else
		{	if(psp->state==active)
				UpdateActivePulsar(psp);
		}
		psp++;
	}

	if(ActivePulsars<MAXPULSARS && ThisLevel.puCount>ActivePulsars)
	{	if(VAPosRandom() < ThisLevel.puProb+ThisLevel.probIncrease)
		{	CreateNewPulsar();
		}
	}
	ThisLevel.starCount += ThisLevel.puCount-ActivePulsars;
	ThisLevel.activeCount += ActivePulsars;
}

void	InitPulsars()
{
	register	int		i;
	
	for(i=0;i<MAXPULSARS;i++)
	{	Pulsars[i].state=inactive;
	}
	
	ActivePulsars=0;
}

void	AllocPulsars()
{
	Pulsars=(Pulsar *)NewPtr(sizeof(Pulsar)*MAXPULSARS);
}

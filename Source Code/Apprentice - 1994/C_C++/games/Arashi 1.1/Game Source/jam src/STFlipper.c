/*/
     Project Arashi: STFlipper.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, December 20, 1992, 19:08
     Created: Sunday, February 12, 1989, 22:58

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "STCrack.h"

#define	MAXFLIPPERS		20
#define	FLIPPERCOLOR	(ThisLevel.flColor)
#define	FLIPPERROTSPEED	(ThisLevel.flRot)
#define	FLIPPERUPSPEED	(ThisLevel.flSpeed)

typedef	struct
{
	int		state;		/*	star=0, inactive=1, active=2		*/
	int		lane;		/*	Actual lane number					*/
	IFixed	level;
	int		vertex;		/*	vertex currently connected to		*/
	int		mirror;		/*	Draw mirror image?					*/
	int		rotation;	/*	Current rotation of flipper			*/
	int		direction;	/*	Rotation direction					*/
}	Flipper,*FlipperPtr;

enum	{	star, inactive, active	};

Flipper	*Flippers;		/*	Pointer to an array of flippers.	*/
int		ActiveFlippers;	/*	The number of active flippers.		*/

extern	Player	Hero;	/*	Player status record.				*/

/*
>>	Given a connecting point (cx,cy) and a vector (dx,dy)
>>	a flipper shape is drawn as quickly as possible.
*/
void	DrawFlipper(cx,cy,dx,dy)
int		cx,cy,dx,dy;
{
	register	int	x1,y1,x2,y2;
				int	dx4,dx8,dy4,dy8;
	
	VA.color=FLIPPERCOLOR;
	dx4=dx >> 2;		dy4=dy >> 2;	/*	Precalculate some values.	*/
	dx8=dx >> 3;		dy8=dy >> 3;
	x1=cx+dx;			y1=cy+dy;
	VAMoveTo(cx,cy);
	x2=dx8-dy4;			y2=dy8+dx4;
	VALineTo(x1-x2,y1-y2);
	x2=dx4-dy8;			y2=dy4+dx8;
	VALineTo(x1-x2,y1-y2);
	VALineTo(x1,y1);
	x2=dx8+dy4;			y2=dy8-dx4;
	VALineTo(cx+x2,cy+y2);
	x2=dx4+dy8;			y2=dy4-dx8;
	VALineTo(cx+x2,cy+y2);
	VALineTo(cx,cy);
}

/*
>>	Create a newborn flipper. A flipper appears
>>	as one of the center stars, so the lane or
>>	vertex is decided when the star touches the
>>	playfield. Some of this stuff could just as 
>>	well be done at that time.
*/
void	CreateNewFlipper()
{
	register	Flipper		*theflip;

	if(ActiveFlippers<MAXFLIPPERS)
	{	theflip=Flippers;
		while(theflip->state!=inactive)
		{	theflip++;
		}
		theflip->state=star;
		theflip->lane=0;
		theflip->level.f=0;
		theflip->level.i=DEPTH;
		theflip->mirror=0;
	
		theflip->direction= (VARandom()<0)?FLIPPERROTSPEED:
										-FLIPPERROTSPEED;
		StarApproach(&(theflip->state),FLIPPERCOLOR);
		ActiveFlippers++;
	}
}
/*
>>	This routine creates two flippers from a destroyed
>>	tanker.
*/
void	CreateSplitFlippers(lane,level)
int		lane,level;
{
	register	int			i,delta,nextseg,prevseg;
	register	Flipper		*theflip;


	for(i=0;i<2;i++)
	{	if(ActiveFlippers<MAXFLIPPERS)
		{	theflip=Flippers;
			while(theflip->state!=inactive)
			{	theflip++;
			}
			theflip->vertex= lane+i;
			if(theflip->vertex>=ww.numsegs)
				theflip->vertex-=ww.numsegs;

			theflip->state= active;
			theflip->level.f=0;
			theflip->level.i=level;
			theflip->mirror=0;
		
			theflip->direction= i ? FLIPPERROTSPEED : -FLIPPERROTSPEED;
			
			nextseg=NextSeg[theflip->vertex];
			prevseg=PrevSeg[theflip->vertex];

			delta=nextseg-prevseg;
			if(delta<0) delta+=ANGLES;
			
			if(theflip->direction < 0)
			{	theflip->rotation=NextSeg[theflip->vertex]-delta/2;
			}
			else
			{	theflip->rotation=PrevSeg[theflip->vertex]+delta/2;
			}

			theflip->lane=LaneSel[theflip->vertex][theflip->rotation];

			ActiveFlippers++;
		}
	}
}
/*
>>	A flipper moves by rotating from one vertex
>>	to another. It "grabs" onto the vertex while
>>	rotating, so at times it needs to change
>>	the way it touches. The mirror value is
>>	used for properly drawing the flipper when
>>	this change happens.
*/
void	SegSkip(theflip)
register
FlipperPtr	theflip;
{
	theflip->rotation+=ANGLES/2;
	if(theflip->rotation>=ANGLES)	theflip->rotation-=ANGLES;
	theflip->mirror= !theflip->mirror;

	if(theflip->level.i==0)
		Hero.lanestat[LaneSel[theflip->vertex][theflip->rotation]] |= FlipMask;
}
/*
>>	This routine is called for each active flipper.
>>	An active flipper has gone through the star
>>	phase and now rotates and climbs up.
*/
void		UpdateActiveFlipper(theflip)
register
FlipperPtr	theflip;
{
	register	int		dx,dy,x,y;
				int		sc;
				int		doskip;

	theflip->level.f-=FLIPPERUPSPEED;	/*	Move up a bit.						*/
	if(theflip->level.i<=0)				/*	Don't move past the top.			*/
	{	theflip->level.i=0;
		ThisLevel.edgeCount++;			/*	Flipper is on the edge.				*/
	}

	y=theflip->direction;	/*	I find this ugly, but it needs to be fast.		*/

	while(y)				/*	While there is rotation to be done.				*/
	{	x=theflip->rotation;/*	x is now the current rotation value.			*/
		dx=NextSeg[theflip->vertex]-x;	/*	Find next clockwise vertex angle.	*/
		dy=PrevSeg[theflip->vertex]-x;	/*	Find next counter-cw vertex angle.	*/
								/*	Note that the above angles are relative!	*/

		if(dx<-ANGLES)	dx=y+y;	/*	A negative angle is used as a flag value	*/
		if(dy<-ANGLES)	dy=y+y;	/*	to indicate that there is no "next vertex".	*/

		if(y>0)						/*	Clockwise rotation?						*/
		{	if(dx<=0)	dx+=ANGLES;	/*	Force angle between 1 and ANGLES		*/
			if(dy<=0)	dy+=ANGLES;	/*	Same thing here.						*/
			if(dx<dy)				/*	Which vertex is next?					*/
			{	if(y>=dx)			/*	Can we reach the next clockwise vertex?	*/
				{	y-=dx;			/*	Yes, adjust the rotation left to do.	*/
					theflip->rotation+=dx;	/*	Rotate to this vertex.			*/
					theflip->vertex++;		/*	Increment vertex value.			*/

					doskip= 1;		/*	We skipped from a vertex to another.	*/
					if(ww.wraps)	/*	Following code depends on field type.	*/
					{	if(theflip->vertex>=ww.numsegs)
						{	theflip->vertex=0;	/*	Wrap around to other side.	*/
						}
					}
					else	/*	Bounce back, if hit the edge of the playfield.	*/
					{	if(theflip->vertex>=ww.numsegs)
						{	theflip->vertex=ww.numsegs-1;
							theflip->direction= -theflip->direction;
							y= -y;
							doskip= 0;
							if(theflip->level.i==0)
								Hero.lanestat[theflip->vertex] |= FlipMask;
						}
					}
					if(doskip)		/*	Did we grab a new vertex?				*/
						SegSkip(theflip);
				}
				else	/*	Didn't reach a new vertex. No more rotating to do.	*/
				{	theflip->rotation+=y;
					y=0;
				}
			}
			else			/*	Basically the same code as above follows,		*/
			{	if(y>=dy)	/*	 but in the other direction.					*/
				{	y-=dy;
					theflip->rotation+=dy;
					theflip->vertex--;
					doskip= 1;
					
					if(ww.wraps)
					{	if(theflip->vertex<0)
						{	theflip->vertex=ww.numsegs-1;
						}
					}
					else
					{	if(theflip->vertex==0)
						{	theflip->vertex=1;
							theflip->direction= -theflip->direction;
							y= -y;
							doskip= 0;
							if(theflip->level.i==0)
								Hero.lanestat[0] |= FlipMask;
						}
					}
					if(doskip)
						SegSkip(theflip);
				}
				else
				{	theflip->rotation+=y;
					y=0;
				}
			}
			/*	Adjust rotation angle to be between 0 and ANGLES-1.				*/
			if(theflip->rotation>=ANGLES) theflip->rotation-=ANGLES;
		}
		else	/*	Counterclockwise rotation. Otherwise similar to above code.	*/
		{	if(dx>=0)	dx-=ANGLES;
			if(dy>=0)	dy-=ANGLES;
			if(dx>dy)
			{	if(y<=dx)
				{	y-=dx;
					theflip->rotation+=dx;
					theflip->vertex++;
					
					doskip= 1;
					if(ww.wraps)
					{	if(theflip->vertex>=ww.numsegs)
						{	theflip->vertex=0;
						}
					}
					else
					{	if(theflip->vertex>=ww.numsegs)
						{	theflip->vertex=ww.numsegs-1;
							theflip->direction= -theflip->direction;
							y= -y;
							doskip= 0;
							if(theflip->level.i==0)
								Hero.lanestat[theflip->vertex] |= FlipMask;
						}
					}
					if(doskip)
						SegSkip(theflip);
				}
				else	
				{	theflip->rotation+=y;
					y=0;
				}
			}
			else
			{	if(y<=dy)
				{	y-=dy;
					theflip->rotation+=dy;
					theflip->vertex--;
					doskip= 1;
					
					if(ww.wraps)
					{	if(theflip->vertex<0)
						{	theflip->vertex=ww.numsegs-1;
						}
					}
					else
					{	if(theflip->vertex==0)
						{	theflip->vertex=1;
							theflip->direction= -theflip->direction;
							y= -y;
							doskip= 0;
							if(theflip->level.i==0)
								Hero.lanestat[0] |= FlipMask;
						}
					}
					if(doskip)
						SegSkip(theflip);
				}
				else
				{	theflip->rotation+=y;
					y=0;
				}
			}
			if(theflip->rotation<0)	theflip->rotation+=ANGLES;
		}
	}

	/*	Find out the lane that we belong to currently.	This depends
	**	on the vertex we are connected to and the angle at which we
	**	are hanging.
	*/
	theflip->lane=LaneSel[theflip->vertex][theflip->rotation];

	/*	Calculate the position and width of the flipper.				*/
	dx= ww.unitlen[theflip->level.i];	/*	Look up length at this level.*/
	
	dy= (dx*(long) -Sins[theflip->rotation])>>8;	/*	Fixed point rotation.	*/
	dx= (dx*(long)Cosins[theflip->rotation])>>8;

	x=ww.x[theflip->vertex][theflip->level.i];/*	Find position on screen.*/
	y=ww.y[theflip->vertex][theflip->level.i];
	
	if(!VA.Late)				/*	Is there time to draw something?	*/
	{	if(theflip->mirror)		/*	Is the flipper mirrored?			*/
			DrawFlipper(x+dx,y+dy,-dx,-dy);
		else
			DrawFlipper(x,y,dx,dy);
	}

	/*	Notify the player record if a flipper is on the edge.			*/
/*	if(theflip->level.i==0)
	{	Hero.segmstat[theflip->vertex] |= FlipMask;
	} */
	
	/*	Test to see if the player has hit us.							*/
	if(ShotHitTest(theflip->lane,theflip->level.i))
	{	sc=ww.unitlen[theflip->level.i];		/*	Explode into two parts.	*/
		AddCrack(	x,y,
					-dx >> 3,-dy >> 3,
					FLIPPERCOLOR,sc,FlipperLeft,
					theflip->rotation,theflip->direction,20);
		AddCrack(	x+dx,y+dy,
					dx >> 3,dy >> 3,
					FLIPPERCOLOR,sc,FlipperRight,
					theflip->rotation+ANGLES/2,theflip->direction,20);
		PlayB(Swoosh,10);

		ThisLevel.flCount--;
		ThisLevel.totalCount--;

		IncreaseScore(ThisLevel.flPoints);
		theflip->state=inactive;
		ActiveFlippers--;
	}
}

/*
>>	Go through the flippers checking their state and
>>	promoting them from stars to fully grown flipper,
>>	when they reach the playfield.
*/
void	UpdateFlippers()
{
	register	Flipper		*flp;
	register	int			i;
	
	flp=Flippers;
	for(i=0;i<MAXFLIPPERS;i++)
	{	if(flp->state<0)
		{	flp->vertex= -(flp->state + 1)/STARDIVISION;
			flp->state= active;
			if(!ww.wraps && flp->vertex==0)	flp->vertex=1;
			if(flp->vertex>ww.numsegs)		flp->vertex=ww.numsegs;

			if(flp->direction < 0)
			{	flp->rotation=NextSeg[flp->vertex];
			}
			else
			{	flp->rotation=PrevSeg[flp->vertex];
			}
		}
		else
		{	if(flp->state==active)
				UpdateActiveFlipper(flp);
		}
		flp++;
	}

	if(ActiveFlippers<MAXFLIPPERS && ThisLevel.flCount>ActiveFlippers)
	{	if(VAPosRandom() < ThisLevel.flProb+ThisLevel.probIncrease)
		{	CreateNewFlipper();
		}
	}
	
	ThisLevel.starCount += ThisLevel.flCount-ActiveFlippers;
	ThisLevel.activeCount += ActiveFlippers;
}

void	InitFlippers()
{
	register	int		i;
	
	for(i=0;i<MAXFLIPPERS;i++)
	{	Flippers[i].state=inactive;
	}
	ActiveFlippers=0;
}

void	AllocFlippers()
{
	Flippers=(Flipper *)NewPtr(sizeof(Flipper)*MAXFLIPPERS);
}

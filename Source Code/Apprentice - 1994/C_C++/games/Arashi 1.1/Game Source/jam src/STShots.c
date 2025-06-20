/*/
     Project Arashi: STShots.c
     Major release: Version 1.1, 7/22/92

     Last modification: Thursday, December 17, 1992, 11:15
     Created: Thursday, March 23, 1989, 20:25

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"

Shot	shots[MAXSHOTS];	/*	Shot data structure is defined in STORM.h	*/
int		shotcount=0;

extern	Player	Hero;

/*
>>	Produce a nice little explosion at (lane,level). These are
>>	just simple pyrotechnics and as such do not really affect
>>	any real shots.
*/
void	ExplodeShot(lane,level)
register	int		lane,level;
{
	register	int		x,y;

	x=ww.xc[lane][level];
	y=ww.yc[lane][level];

	level=3 - (level << 2)/(DEPTH+1);	
	VAExplosion(x,y,level,VA.color);
}
/*
>>	A shot consists of five "spots". One in
>>	the center and four around it placed symmetrically.
>>	This routine draws a shot at (lane,level).
*/
void	DrawShot(lane,level)
register	int		lane,level;
{
	register	int		dx,dy,x,y;
	static		int		shotrotation;

	shotrotation+=7;
	if(shotrotation>=ANGLES) shotrotation-=ANGLES;
	dx= ((ww.unitlen[level]>>3)*Cosins[shotrotation])>>8;
	dy= ((ww.unitlen[level]>>3)*Sins[shotrotation])>>8;

	x=ww.xc[lane][level];
	y=ww.yc[lane][level];

	VA.color=ThisLevel.shColor[4];
	VASpot(x+dx,y+dy);

	if(level<DEPTH/3)
	{
		VA.color=ThisLevel.shColor[3];
		VASpot(x-dx,y-dy);
		VA.color=ThisLevel.shColor[2];
		VASpot(x+dy,y-dx);
		VA.color=ThisLevel.shColor[1];
		VASpot(x-dy,y+dx);
	}
	VA.color=ThisLevel.shColor[0];
	VASpot(x,y);
}
/*
>>	To fire a bullet, call AddShot for that lane.
>>	This routine returns -1, if a shot was available
>>	and false, if none was fired.
*/
int		AddShot(lane)
int		lane;
{
	if(shotcount<MAXSHOTS)
	{	shots[shotcount].lane=lane;
		shots[shotcount].level=0;
		shotcount++;
		return -1;
	}
	else
		return 0;
}
/*
>>	This routine performs a hit test for your object.
>>	When your object is near the center of a lane, call
>>	this routine to determine if you were hit. -1 is
>>	returned, if you were hit. The shot is removed from
>>	the list and an explosion is displayed where the shot
>>	was when it hit.
*/
int		ShotHitTest(lane,level)
register	int		lane,level;
{
	register	int		i;
	register	Shot	*sp;
	
	if(Hero.superzapping)
	{	if(Hero.superzapping==2)	/*	Single enemy zap.	*/
			Hero.superzapping=0;
		return -2;					/* mz */
	}
	
	sp=shots;
	for(i=0;i<shotcount;i++)
	{	if(sp->lane==lane)
		{	if(level <= sp->level)
			if(level+4 >= sp->level)
			{	ExplodeShot(lane,level);
				*sp=shots[--shotcount];
				return -1;
			}
		}
		sp++;
	}
	return 0;
}
/*
>>	UpdateShots moves the shots down one step and then refreshes
>>	the display.
*/
void	UpdateShots()
{
	register	int		i;
	register	int		bottomlevel;
	register	Shot	*shp;
	
	shp=shots;
	
	if(Hero.flydepth)
	{	bottomlevel=DEPTH-Hero.flydepth;
		for(i=0;i<shotcount;i++)
		{	if(shp->level>=bottomlevel)
			{	*shp=shots[--shotcount];
				i--;
			}
			else
			{	if(!VA.Late && shp->level)	DrawShot(shp->lane,shp->level);
				shp->level+= ThisLevel.shSpeed;
				if(shp->level>bottomlevel)	shp->level=bottomlevel;
				shp++;
			}
		}
	
	}
	else
	{	for(i=0;i<shotcount;i++)
		{	if(shp->level>=DEPTH)
			{	*shp=shots[--shotcount];
				i--;
			}
			else
			{	if(!VA.Late && shp->level)	DrawShot(shp->lane,shp->level);
				shp->level+= ThisLevel.shSpeed;
				if(shp->level>DEPTH)	shp->level=DEPTH;
				shp++;
			}
		}
	}
}
/*
>>	Start out with no shots fired.
*/
void	InitShots()
{
	shotcount=0;
}
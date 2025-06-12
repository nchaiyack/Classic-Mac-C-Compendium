/*/
     Project Arashi: STFuseBalls.c
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, June 1, 1993, 22:07
     Created: Sunday, January 13, 1991, 14:28

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "STFloatingScores.h"
#include "PlayOptions.h"

#define	MAXFUSEBALLS		20
#define	FUSESTARCOLOR		0
#define	WARPRANGE			((DEPTH+DEPTH/5)<<4)
#define	MAXWARP				(DEPTH<<3)
#define	WARPSPEED			50
#define	WARPPROB			(ThisLevel.fuWarpP)
#define	WARPANGER			(ThisLevel.fuPlayerPlus)
#define	MAXLEVELSPEED		3
#define	LANEDIVISIONS		256
#define	MAXLANESPEED		(LANEDIVISIONS/8)
#define	ROTATIONSPEED		2	/*(ANGLES/2-1)*/
#define	FUSEDANGERLEVEL		1
/*#define	ZIMMER_WARP*/

extern	Player	Hero;

typedef	struct
{
	int		activeflag;
	int		mode;
	int		lane;
	int		sublane;
	int		level;
	int		warplevel;
	int		lanespeed;
	int		levelspeed;
	int		rotation;
}	FuseBall;

/*	FuseBall movement modes:	*/
enum	{	Waiting=0,Brownian, Warp, Edge	};

FuseBall	*Balls;
int			NumBalls;

void	CreateNewBall()
{
	register	FuseBall	*fbp;
	
	if(NumBalls<MAXFUSEBALLS)
	{	fbp=Balls;
		NumBalls++;
		while(fbp->activeflag)
		{	fbp++;
		}
		fbp->mode=Waiting;
		fbp->level=DEPTH<<4;
		fbp->rotation=0;
		fbp->activeflag=-1;
		StarApproach(&(fbp->mode),FUSESTARCOLOR);
	}
}
/*
>>	This routine creates two fuseballs from a destroyed
>>	tanker.
*/
void	CreateSplitFuseBalls(lane,level)
int		lane,level;
{
	register	int			i;
	register	FuseBall	*theball;
			
	for(i=0;i<2;i++)
	{	if(NumBalls<MAXFUSEBALLS)
		{	theball=Balls;
			while(theball->activeflag)
			{	theball++;
			}
			theball->mode=Brownian;
			theball->lane= lane;
			theball->activeflag= 1;
			theball->level=level<<4;
			theball->sublane= i ? (LANEDIVISIONS-LANEDIVISIONS/4) : (LANEDIVISIONS/4);
			theball->lanespeed= i ? (MAXLANESPEED) : (-MAXLANESPEED);
			theball->levelspeed=0;
			theball->rotation=i ? 0 : (ANGLES/3);

			NumBalls++;
		}
	}
}

void	UpdateFuseBalls()
{
	register	FuseBall	*fbp;
	register	int			i,j;
				int			x,y,l;
				int			rd;	/*	A random number	*/
				int			zap;	/* type of shot hit */
	UpdateFuseOff();
	
	fbp=Balls;
	for(i=0;i<MAXFUSEBALLS;i++)
	{	if(fbp->activeflag)
		{	if(fbp->mode<=0)
			{	if(fbp->mode<0)
				{	fbp->lane=-(fbp->mode + 1)/STARDIVISION;
					fbp->sublane=0;
					fbp->mode=Warp;
					fbp->warplevel=(DEPTH<<4)/3;
					fbp->levelspeed=-WARPSPEED;
					fbp->lanespeed=0;
				}
			}
			else
			{	rd=VARandom();
				if(fbp->mode==Edge)
					ThisLevel.edgeCount++;	/*	Fuseball is on the edge.			*/
				switch(fbp->mode)
				{	case Waiting:	/*	Do nothing.	*/
						break;
					case Warp:
						if(fbp->levelspeed<0)
						{	fbp->level += fbp->levelspeed;
							if(fbp->level < fbp->warplevel)
							{	
								fbp->level= fbp->warplevel;
								fbp->levelspeed=0;
								fbp->mode=Brownian;
							}
						}
						else
						{	fbp->level += fbp->levelspeed;
							if(fbp->level > fbp->warplevel)
							{	fbp->level= fbp->warplevel;
								fbp->levelspeed=0;
								fbp->mode=Brownian;
							}
						}
						break;
					case Brownian:
/* */					if(ThisLevel.fuCount + (ThisLevel.lvNumber>>3) < ThisLevel.totalCount)
							fbp->level += fbp->levelspeed;
						 else 
/* */					 /* keep fuseballs moving up near end (mz)				*/
							fbp->level+=(fbp->levelspeed > 0 )?-fbp->levelspeed:fbp->levelspeed;
						
						if(fbp->level>(DEPTH<<4)) /* if deeper than bottom 1/16 */
						{	fbp->level= (DEPTH<<4);
							if(fbp->levelspeed>0)
							{	fbp->levelspeed= - (fbp->levelspeed>>1);
							}
						}
						else
						if(fbp->level<=0)
						{	fbp->level=0;
							fbp->levelspeed=0;
/* */						if((ThisLevel.fuCount + (int)((ThisLevel.lvNumber)/8) >= ThisLevel.totalCount))
							{
								fbp->mode=Edge; /* only have fuseballs on edge 	*/
												/*	after most else dead (mz)	*/
							}
						}
						if(fbp->level>(DEPTH<<3)) /* if bottom 1/8 */
						{	fbp->levelspeed -= ((rd & 0x3000)>>12)-1;
						}
						else
						{	fbp->levelspeed += ((rd & 0x3000)>>12)-1;
						}
						if(fbp->levelspeed>MAXLEVELSPEED || fbp->levelspeed<=-MAXLEVELSPEED)
						{	fbp->levelspeed /= 3;
						}
						
						j=rd & 0xff;
						if(fbp->lane==Hero.lane) 
							j-= WARPANGER;
#ifdef ZIMMER_WARP
						/* */
/* */					/* when # fuseballs = increasing percent remaining enemies 	*/
						/* make them more angry,so they bounce up and down more (mz)*/
						/* By mid-yellow they should be very active from the start 	*/
						/* */
						if((ThisLevel.fuCount + (int)(ThisLevel.lvNumber) - 16 >= ThisLevel.totalCount))
							{
								/* Need to allow some sub-lane movement too while 	*/
								/* mad so make anger somewhat random so they will	*/
								/* not go into warp, and will fall thru to sublane	*/
								/* code below.										*/
								/* */
								if( (unsigned int)VARandom() > (unsigned int)((7*65536)/10))
									j -= 5*WARPANGER; /* mz */
							}
						if(j<WARPPROB)
						{	
/* */						/* dont let fuseballs go into warp near the end so they	*/
							/* will go to edge. (mz)								*/
							if((ThisLevel.fuCount + (ThisLevel.lvNumber>>3) - 1 < ThisLevel.totalCount))
								fbp->mode=Warp;

							j= (((unsigned int)VARandom()) % WARPRANGE) - WARPRANGE + (DEPTH<<4);
							if(j<0) j=0;
							if(j>(DEPTH<<4)) j=DEPTH<<4;
							if(j>fbp->level+MAXWARP)
							{	j=fbp->level+MAXWARP;
							}
							else
							if(j<fbp->level-MAXWARP)
							{	j=fbp->level-MAXWARP;
							}
							fbp->warplevel=j;
							fbp->levelspeed=(j<fbp->level)?-WARPSPEED:WARPSPEED;
						}
#else
						if(j<WARPPROB)
						{	fbp->mode=Warp;
							j= (((unsigned int)VARandom()) % WARPRANGE) - WARPRANGE + (DEPTH<<4);
							if(j<0) j=0;
							if(j>(DEPTH<<4)) j=DEPTH<<4;
							if(j>fbp->level+MAXWARP)
							{	j=fbp->level+MAXWARP;
							}
							else
							if(j<fbp->level-MAXWARP)
							{	j=fbp->level-MAXWARP;
							}
							fbp->warplevel=j;
							fbp->levelspeed=(j<fbp->level)?-WARPSPEED:WARPSPEED;
						}

#endif
					case Edge:	/*	Fall through from Brownian too.	*/
						fbp->sublane+=fbp->lanespeed;
						while(fbp->sublane<0)
						{	fbp->lane-=1;
							fbp->sublane+=LANEDIVISIONS;
						}

						while(fbp->sublane >= LANEDIVISIONS)
						{	fbp->sublane-=LANEDIVISIONS;
							fbp->lane+=1;
						}
						
						if(fbp->lane<0)
						{	if(ww.wraps)
							{	fbp->lane+=ww.numsegs;
							}
							else
							{	fbp->lane=0;
								fbp->sublane=0;
								fbp->lanespeed= -fbp->lanespeed;
							}
						}
						else if(fbp->lane >= ww.numsegs)
						{	if(ww.wraps)
							{	fbp->lane -= ww.numsegs;
							}
							else
							{	fbp->lane--;
								fbp->sublane+=LANEDIVISIONS-fbp->lanespeed;
								fbp->lanespeed= -fbp->lanespeed;
							}
						}

						fbp->lanespeed+=(rd & 8192)?-1:1;
						if(fbp->lanespeed>MAXLANESPEED || fbp->lanespeed<-MAXLANESPEED)
						{	fbp->lanespeed /=2;
						}
						break;
				}
				fbp->rotation+=ROTATIONSPEED;
				if(fbp->rotation>=ANGLES)	fbp->rotation-=ANGLES;
				
				l=fbp->level>>4;
				x=ww.x[fbp->lane][l];
				y=ww.y[fbp->lane][l];
				x += ((ww.x[fbp->lane+1][l]-x)*fbp->sublane)/LANEDIVISIONS;
				y += ((ww.y[fbp->lane+1][l]-y)*fbp->sublane)/LANEDIVISIONS;
				DrawFuseBall(x,y,ww.unitlen[l],fbp->rotation);
				
				/* Test for fuseball on edge of lane (fbp->lane)				*/
				/* want to add test to allow player to pass thru fuseball 		*/
				/* as it changes lanes.		 									*/
				/* So test to see if the fuseball is in the outer 8 sublanes 	*/
				/* if it is, then let the player pass thru by not adding the  	*/
				/* fusemaskto the lanstat (mz) 									*/
				/* eg 
				sublanes  0..7 8............................247 248....255 
				           ^^^                 ^^^^                    ^^
				           safe					dead				  safe (mz) */
				           
				if(	(l<FUSEDANGERLEVEL) && 
					(fbp->sublane>( (LANEDIVISIONS>>5) )) && 
					(fbp->sublane<(LANEDIVISIONS- (LANEDIVISIONS>>5) )) )
				{	Hero.lanestat[fbp->lane] |= FuseMask;
				} 
				
				/* fuseball can be shot if it is not in the outer 1/4 sublanes 	*/
				/* so it can be shot half of the time (mz) 						*/
				
				if((fbp->sublane>( LANEDIVISIONS>>2) ) && 
					fbp->sublane<(LANEDIVISIONS-(LANEDIVISIONS>>2) ) ||
					Hero.superzapping)
				{	if(zap=ShotHitTest(fbp->lane,l))
					{	fbp->activeflag=0;
						NumBalls--;
						ThisLevel.fuCount--;
						ThisLevel.totalCount--;
								
						/* have different score for Fuseballs depending upon 	*/
						/* how deeply they are killed (mz) 	*/
						/* do not show floating scores if superzap is used */
						
						if ( (DEPTH>l) && (l >= (int)(DEPTH*2/3)) )
						{		/* bottom 1/3 */
							IncreaseScore(ThisLevel.fuBullseye);
							if((PlayOptions->showfscores) && (zap != -2))
								AddFLScore(x,y,ThisLevel.fuBullseye);
						}
						
						else if ( ((DEPTH*2/3)>l) && (l>=(int)(DEPTH/3)) )
						{		/* middle 1/3 */
							IncreaseScore(ThisLevel.fuPoints);
							if((PlayOptions->showfscores) && (zap != -2))
								AddFLScore(x,y,(ThisLevel.fuPoints));
						}
						
						else
						{		/* upper 1/3 */
							IncreaseScore( (ThisLevel.fuPoints)>>1 );
							if((PlayOptions->showfscores) && (zap != -2))
								AddFLScore(x,y,( (ThisLevel.fuPoints>>1) ));
						}
						
						BlowFuseBall(x,y,ww.unitlen[l],fbp->rotation);
						PlayB(Blow,11);
					}
				}
			}
		}
		fbp++;
	}
	if(NumBalls<MAXFUSEBALLS && ThisLevel.fuCount>NumBalls)
	{	if(VAPosRandom() < ThisLevel.fuProb+ThisLevel.probIncrease)
		{	CreateNewBall();
		}
	}
	ThisLevel.starCount += ThisLevel.fuCount-NumBalls;
	ThisLevel.activeCount += NumBalls;
}
void	AllocFuseBalls()
{
	Balls=(FuseBall *)NewPtr(sizeof(FuseBall) * MAXFUSEBALLS);
	NumBalls=0;
}

void	InitFuseBalls()
{
	register	int		i;
	
	for(i=0;i<MAXFUSEBALLS;i++)
	{	Balls[i].activeflag=0;
	}
	NumBalls=0;
}

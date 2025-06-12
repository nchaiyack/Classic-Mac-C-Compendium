/*/
     Project Arashi: STFlyThru.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, March 14, 1993, 20:17
     Created: Sunday, February 24, 1991, 2:05

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "PlayOptions.h"
#include "GamePause.h"
#include <OSUtils.h>	/* mz */

#define	MSGDELAY 120	/* 2 seconds */

extern	Player		Hero;
extern	EventRecord	Event;

static	int			FieldColor;
static	int			SpikeColor;

void	MapFlyColors()
{
	register	int		colorbits;
	register	int		i;
	register	ColorSpec	*colp;
				Handle	FlyColors;
	
	colorbits=0;
	for(i=0;i<5;i++)
	{	colorbits |= 1<<ThisLevel.shColor[i];
	}
	SpikeColor=5;
	FieldColor=6;

	for(i=0;i<6;i++)
	{	if((colorbits & (1<<i))==0)
		{	FieldColor=i;
			colorbits |= 1<<i;
			i=6;
		}
	}

	for(i=0;i<6;i++)
	{	if((colorbits & (1<<i))==0)
		{	SpikeColor=i;
			colorbits |= 1<<i;
			i=6;
		}
	}
	FlyColors=GetResource('CLOT',ThisLevel.lvColor);
	if (ThisLevel.lvColor == 5)
		FlyColors=GetResource('CLOT',ThisLevel.lvColor + 128);
	HandToHand(&FlyColors);
	colp = (ColorSpec *)*FlyColors;
	colp[SpikeColor]=colp[7];
	colp[FieldColor]=colp[8];
	VASetColors(FlyColors);
	DisposHandle(FlyColors);
}

void	RedrawField()
{
	register	int		i,j;
	register	int		oldfield;
	
	oldfield=VA.field;
	VA.field=1;		/*	Field background.		*/
	VA.offset=1;	/*	Background				*/

	for(i=0;i<ww.numsegs;i++)
	{	VAStaticLine(ww.x[i][0],ww.y[i][0],ww.x[i][DEPTH],ww.y[i][DEPTH]);
	}
	if(ww.wraps==0)
		VAStaticLine(ww.x[i][0],ww.y[i][0],ww.x[i][DEPTH],ww.y[i][DEPTH]);

	for(j=0;j<=DEPTH;j+=DEPTH)
	{	for(i=0;i<ww.numsegs;i++)
		{	VAStaticLine(ww.x[i][j],ww.y[i][j],ww.x[i+1][j],ww.y[i+1][j]);
		}
	}
}

void	FlyGrid()
{
	register	int		i,j;
	register	int		upper,depper;
				Rect	toclip;
	
	depper=DEPTH-Hero.flydepth;

	upper=(depper-DEPTH)/3;
	if(upper<ZOOMDEPTH)
		upper=ZOOMDEPTH;


	j=ww.numsegs;	
	if(ww.wraps) j--;
	i=j;
	do
	{	
		toclip.left=ww.x[i][upper];
		toclip.top=ww.y[i][upper];
		toclip.right=ww.x[i][depper];
		toclip.bottom=ww.y[i][depper];

		if(VAClip(&toclip))
		{	if(Hero.lane==i || Hero.lane+1==i)
			{	VA.color=ThisLevel.shColor[0];
			}
			else
			{	if(i==0 && ww.wraps && Hero.lane == ww.numsegs-1)
				{	VA.color=ThisLevel.shColor[0];
				}
				else
				{	VA.color=FieldColor;
				}
			}

			VALine(toclip.left,toclip.top,toclip.right,toclip.bottom);
		}
	}	while(i--);

	VA.color=FieldColor;
	i=j;
	VAMoveTo(ww.x[ww.numsegs][upper],ww.y[ww.numsegs][upper]);
	do
	{	VASafeLineTo(ww.x[i][upper],ww.y[i][upper]);
	}	while(i--);

	i=j;
	VAMoveTo(ww.x[ww.numsegs][depper],ww.y[ww.numsegs][depper]);
	do
	{	VASafeLineTo(ww.x[i][depper],ww.y[i][depper]);
	}	while(i--);
}

void DoWatchSpikesMsg()
{
	/* This next section will add the message 			*/
	/* "Avoid Spikes" to levels 3-8 so the new 			*/
	/* player will know to avoid them. (mz) 			*/
	
	unsigned char	*text=(void *)"\pAvoid Spikes";
	long			targetTicks;
	int				x,y;
	
	VA.segmscale = Getfontscale() * 4;
	if(VA.segmscale <=1) 
		VA.segmscale=2;
	x = (int)( (VA.frame.right - 12*(VA.segmscale*3 + 3))/2 );
	y = (int)(VA.frame.bottom/2 - 2);
	VAMoveTo(x,y);
	VA.color=BG1;
	VADrawText((char *)text,1,12);
	
	targetTicks = Ticks + MSGDELAY;
	/* Now delay for 2 secs.  Update Player and Shots, then erase msg and return */
	while(Ticks <= targetTicks)
	{
		GameEvent();
		UpdatePlayer();
		UpdateShots();
		UpdateSpikes();
		UpdateCracks(); 
		VAStep();
	}
	VAMoveTo(x,y);
	VA.segmscale = Getfontscale() * 4;
	if(VA.segmscale <=1) 
		VA.segmscale=2;
	VA.color = -1;
	VADrawText((char *)text,1,12);
}

void	STFlyThruLoop(Options)
int	Options;
{
	register	int		i;
	
	MapFlyColors();
	VACatchUp();

	ThisLevel.totalCount=0;
	ThisLevel.flCount=0;
	ThisLevel.fuCount=0;
	ThisLevel.puCount=0;

	for(i=0;i<TANKTYPES;i++)
	{	ThisLevel.tk[i].count=0;
	}
	if(PlayOptions->showfscores)
		UpdateFloatingScores();				/* kill off any floating scores remaining */
	if (ThisLevel.ShowSpikesMsg && Options == NormalPlay)
		DoWatchSpikesMsg();
		
#define	MAKENOISE
#ifdef	MAKENOISE
	PlayA(FlyThru,999);
	if(!PlayOptions->noLoudSounds)
		PlayB(FlyThru,999);
#endif
	
	do
	{	GameEvent();
		if(Event.what == keyDown || Event.what == autoKey)
		{	switch((char)Event.message)
			{	case 'p':
				case 'P':
					GamePause(FlythruRunning);
					break;
			}
		}
		if(Hero.flydepth<DEPTH)
			UpdateShots();

		FlyGrid();
		FlySpikes(SpikeColor);

		if(Hero.flydepth<DEPTH)
			UpdatePlayer();
		
		UpdateCracks();
		VAStep();
		if(Hero.flydepth==0)
		{	VA.color=-1;	/*	Black					*/
			RedrawField();
			RedrawSpikes();
		}
		Hero.flydepth+=2;
	}	while(!(Hero.flydepth >= DEPTH-ZOOMDEPTH || Hero.state == HeroDead));

	while(Hero.state != HeroFlying && Hero.state != HeroDead)
	{	GameEvent();
		UpdatePlayer();
		UpdateCracks();
		VAStep();
	}

	if(Hero.state == HeroDead)
	{	VASetColors(GetResource('CLOT',ThisLevel.lvColor));
	}
}


extern	int	FieldDistance;

void	STFlyOutLoop()
{
	short	i;
	
	if (ThisLevel.lvColor != 5)			/* Use alternate grey for invis levels (mz) */
		OpenStars(GetResource('CLOT',ThisLevel.lvColor));
	else
		OpenStars(GetResource('CLOT',ThisLevel.lvColor + 128));
	
	ThisLevel.totalCount=0;
	ThisLevel.flCount=0;
	ThisLevel.fuCount=0;
	ThisLevel.puCount=0;

	for(i=0;i<TANKTYPES;i++)
	{	ThisLevel.tk[i].count=0;
	}
	
	if(PlayOptions->showfscores)
	UpdateFloatingScores();			/* kill off any floating scores remaining 	*/
	
	FieldDistance = 0;
	do
	{	if(FieldDistance == 1)
		{	VA.color=-1;	/*	Black					*/
			RedrawField();
			RedrawSpikes();
			VACatchUp();
		}
		
		FieldRide(&gridOutline,1+(FieldDistance>>2));
		GameEvent();
		VACatchUp();
		VAStep();
	} while(FieldDistance < (DEPTH<<5));
	
}

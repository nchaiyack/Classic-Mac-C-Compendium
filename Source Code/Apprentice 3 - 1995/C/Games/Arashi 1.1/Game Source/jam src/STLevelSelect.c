/*/
     Project Arashi: STLevelSelect.c
     Major release: Version 1.1, 7/22/92

     Last modification: Thursday, May 6, 1993, 22:04
     Created: Wednesday, March 6, 1991, 12:00

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "GamePause.h"
#include <math.h>
#include "PlayOptions.h"
#include "heroflags.h"

#define	MAXSELECTABLES	96
/* #define DEBUGGING_ROUTINES */

extern	EventRecord	Event;
extern 	Player	Hero;

static	Point		BoxSize;

int		FirstLevel=0;
int		LastLevel=300;
int		LastSelect=200;

void	DrawLevelPresentation(offset,resnum)
int		offset,resnum;
{
				shape	**spacehand;
	register	shape	*space;
	register	long	x,y;
				long	xmin,xmax;
				long	ymin,ymax;
	register	int		i,angle;
	register	int		*angles;
				Point	base;


	spacehand=(void *)GetResource('SPCE',resnum);
	HLock(spacehand);
	space=*spacehand;
	
	x=0;		y=0;
	xmin=0;		ymin=0;
	xmax=0;		ymax=0;

	angle=0;
	angles=space->ang;
	for(i=0;i<space->numsegs;i++)
	{	angle+= *angles++;
		while(angle<0) 			angle += ANGLES;
		while(angle>=ANGLES)	angle -= ANGLES;
		
		x += Cosins[angle];		y -= Sins[angle];
		if(x>xmax) xmax=x;		if(y>ymax) ymax=y;
		if(x<xmin) xmin=x;		if(y<ymin) ymin=y;
	}
	xmax += 512;	ymax+= 512;
	xmin -= 512;	ymin-= 512;
	
	x = -(xmin+xmax)/2;
	y = -(ymin+ymax)/2;
	
	xmax -= xmin;
	ymax -= ymin;
	
	if(xmax<ymax)	xmax=ymax;
	
	base.h = BoxSize.h/2 + offset;
	base.v = VA.frame.bottom - BoxSize.v;
	
	VAMoveTo( x * BoxSize.h / xmax + base.h , y * BoxSize.v / xmax + base.v);
	angle=0;
	angles=space->ang;
	for(i=0;i<space->numsegs;i++)
	{	angle+= *angles++;
		while(angle<0) 			angle += ANGLES;
		while(angle>=ANGLES)	angle -= ANGLES;
		x += Cosins[angle];		y -= Sins[angle];
		VASafeLineTo( x * BoxSize.h / xmax + base.h , y * BoxSize.v / xmax + base.v);
	}
	
	HUnlock(spacehand);
	ReleaseResource(spacehand);
}

typedef	struct
{	
	int			levelresource;
	ColorSpec	levelcolor;
	int			levelnumber;
	long		levelbonus;				/* added field (mz) */
}	selectable;


int		CreateStable(stable,highestLastLevel)
selectable	*stable;
int	highestLastLevel;
{
				short		selectFlag;
	register	int			levcount;
	register	int			colorId;
	register	ColorSpec	**LCTable;
				int			SAVEDGAMECOLOR=7;
				int			highlvl=highestLastLevel;
				
	if (highestLastLevel<9)
		highlvl=9;
	
#ifdef DEBUGGING_ROUTINES
	highlvl = 99;
#endif

	LoadGameF();

	LCTable = (void *)GetResource('CLOT',SAVEDGAMECOLOR);

	stable->levelbonus = Hero.score;
	stable->levelresource = ThisLevel.lvField;
	stable->levelnumber = ThisLevel.lvNumber;
	stable->levelcolor = (*LCTable)[8]; 

	ReleaseResource(LCTable);
	
	ThisLevel.lvNext = 1;
	levcount = 1;

	do
	{	
		while(0 == (selectFlag = STLoadForSelect(PlayOptions->restart)));
	
		if(selectFlag == 1)
		{	colorId = ThisLevel.lvColor;
			if (colorId == 5)				/* if invis field color use grey shade 	*/
				colorId += 128;	/* for invisi levels (mz) 	*/

			LCTable = (void *)GetResource('CLOT',colorId);

			stable[levcount].levelresource = ThisLevel.lvField;
			stable[levcount].levelnumber = ThisLevel.lvNumber;
			stable[levcount].levelcolor = (*LCTable)[8];
			stable[levcount].levelbonus = ThisLevel.lvStBonus; 	/* starting bonus (mz) 	*/				
			
			ReleaseResource(LCTable);

			levcount++;
		}
		
	}	while(levcount < MAXSELECTABLES && selectFlag >= 0 &&
			(ThisLevel.lvNumber <= highlvl || PlayOptions->restart));
	
	if(PlayOptions->restart == 0)	/* Arcade mode */
		if(highlvl < stable[levcount-1].levelnumber)
			levcount--;
	
	return	(levcount);
}
/* ================================= */


int		DoLevelSelect(highestLastLevel)
int highestLastLevel;
{
				selectable	*stable;
	register	int			i,j;
	register	int			x,y;
				static	int	thelevel=0;
				int			reallevel;
				int			theoffset=0;
				int			done=0;
				Point		mouse,CenterMouse;
	register	int			direction;
	register	ColorSpec	*colp;
				Handle		SelColors;
				int			OldFrameSpeed;
				int			levelnum; 			  /* temp holding variable (mz) */
				Point		infoBoxSize;
				unsigned char	*infotext=(void *)"\pSaved Game Score";
				
	stable = (selectable *)NewPtr(sizeof(selectable) * MAXSELECTABLES);
		
	SelColors=GetResource('CLOT',128);
	HandToHand(&SelColors);
	HLock(SelColors);
	colp = (ColorSpec *)*SelColors;
	
	LastLevel=LastSelect=CreateStable(stable,highestLastLevel)-1;
	
	CenterMouse.h=(VA.DisplayFrame.right+VA.DisplayFrame.left)/2;
	CenterMouse.v=(VA.DisplayFrame.bottom+VA.DisplayFrame.top)/2;

	mouse=CenterMouse;
	GetSetMouse(&mouse);

	VAEraseBuffer();
	VACatchUp();
	
	BoxSize.h=VA.frame.right/5;
	BoxSize.v=BoxSize.h;
	
	infoBoxSize.h=VA.frame.right/10;
	infoBoxSize.v=infoBoxSize.h;

	OldFrameSpeed = VA.FrameSpeed;
	VA.FrameSpeed = 6;

	do
	{	reallevel=thelevel+(theoffset>BoxSize.h/2 ? 1:0);
		GameEvent();
	
		switch(Event.what)
		{
			case keyDown:
			case autoKey:
				switch((char)Event.message)
				{	case 'p':
					case 'P':
						GamePause(LevelSelectRunning);
						break;
					case 'Q':
					case 'q':
					case 27:
					case 13:
						done=-1;
						break;
					default:
						PlayA(PhazerIn,0);
						PlayB(PhazerIn,0);
						done=stable[reallevel].levelnumber;
						break;
				}
				break;
			case mouseUp:
				PlayA(PhazerIn,0);
				PlayB(PhazerIn,0);
				done=stable[reallevel].levelnumber;
				break;
			case mouseDown:
				break;
		}

		mouse=CenterMouse;
		GetSetMouse(&mouse);
		
		if(mouse.h)
		{	theoffset-=mouse.h;
			direction=(direction/2)+mouse.h;
			if(direction==0) direction=mouse.h;
		}
		else
		{	if(theoffset!=0)
			{	theoffset-=direction;
				if(theoffset<0)	theoffset=0;
				if(theoffset>BoxSize.h) theoffset=BoxSize.h;
			}
		}
		while(theoffset<0)
		{	theoffset+=BoxSize.h;
			 thelevel--; 
		}
		if(thelevel<0)
		{	thelevel=0;
			theoffset=0;
		}

		while(theoffset>=BoxSize.h)
		{	theoffset-=BoxSize.h;
			 thelevel++; 
		}

		if(thelevel>=LastSelect)	
		{	thelevel=LastSelect;
			theoffset=0;
		}

		for(i=0;i<6;i++)
		{	int		lvnum = thelevel+i-2;
		
			if(lvnum>=0 && lvnum<=LastLevel)
			{	VA.color=i+1;
				DrawLevelPresentation(i * BoxSize.h-theoffset,stable[lvnum].levelresource);
				colp[i+1]=stable[lvnum].levelcolor;
			}
		}

		VA.segmscale = BoxSize.v>>4;
		if(VA.segmscale<1) VA.segmscale = 1;
		VA.color = BG1;
		/* print level number below box */
		levelnum = thelevel+(theoffset>BoxSize.h/2 ? 1:0);
		VADrawPadNumber(stable[levelnum].levelnumber
						,2*BoxSize.h+BoxSize.h/2,VA.frame.bottom-4*VA.segmscale,3);
	
		/* show starting level bonus on top of box */
		VADrawPadNumber(stable[levelnum].levelbonus,2*BoxSize.h+BoxSize.h/2 + 4*VA.segmscale,
						VA.frame.bottom-(BoxSize.v)*(1.6),7);
		/*VADrawPadNumber(levelnum,2*BoxSize.h+BoxSize.h/2 + 4*VA.segmscale,
						VA.frame.bottom-(BoxSize.v)*(2),7);*/
		VA.color=0;
		x = BoxSize.h * 2;
		y = VA.frame.bottom - BoxSize.v / 2;

		VAMoveTo    (x,				y);
		VASafeLineTo(x+BoxSize.h,	y);
		VASafeLineTo(x+BoxSize.h,	y-BoxSize.h);
		VASafeLineTo(x,				y-BoxSize.h);
		VASafeLineTo(x,				y);
	
	
		y -= BoxSize.v*1.5;
		x = (VA.frame.right-infoBoxSize.h)>>1;
		
		VAMoveTo    (x,				y);		/* */
		VASafeLineTo(x+infoBoxSize.h,	y);
		VASafeLineTo(x+infoBoxSize.h,	y-infoBoxSize.h);
		VASafeLineTo(x,				y-infoBoxSize.h);
		VASafeLineTo(x,				y);		/* */
		if(levelnum > 0)
			infotext=(void *)"\p Starting Bonus ";
		else
			infotext=(void *)"\pSaved Game Score";
			
		VA.segmscale= VA.segmscale>>2;
		if(VA.segmscale<=1) VA.segmscale=2;
		VA.color=BG1;
		x = ((VA.frame.right - ((VA.segmscale*3+3)<<4) ))>>1;
			
		VAMoveTo(x, y - infoBoxSize.h - 3 ); 
		VADrawText((char *)infotext,1,16);
			
		x = ( (VA.frame.right - (VA.segmscale*3+3)*12 )>>1 );
		VAMoveTo(x, y + VA.segmscale*7 + 2 ); 
		infotext=(void *)"\pLevel number";
		VADrawText((char *)infotext,1,12);
	
	
	
		VASetColors(SelColors);
		VAStep();

#define	ADVERTISE
#ifdef	ADVERTISE			
		{	unsigned

			char	*funtext=(void *)"\p ARASHI *** Copyright 1991 Project STORM Team, All Rights Reserved *** Version 1.1 *** Please select your starting level ***";
			int		rotme=(VA.FrameCounter) % *funtext;
			
			VA.segmscale= VA.segmscale/3;
			if(VA.segmscale<=1) VA.segmscale=2;
			VA.color=BG2;
			VAMoveTo(12,VA.frame.top + VA.segmscale*7 + BoxSize.h/2); 
			
			/*VAMoveTo(12,VA.frame.bottom-BoxSize.v*5/3 - 100); */ /* moved up for lvBonus(mz) */
			VADrawText((char *)funtext,1+rotme,*funtext-rotme);
			VADrawText((char *)funtext,1,rotme);
		}
#endif
	}	while(!done);
	
	VAEraseBuffer();
	VACatchUp();

	HUnlock(SelColors);	
	DisposHandle(SelColors);

	VA.FrameSpeed = OldFrameSpeed;	
	
	if(levelnum == 0)			/* saved game was loaded as level select */
		Hero.Flags |= SavedGameStartMask;

	DisposPtr(stable);
	
	return done;
}

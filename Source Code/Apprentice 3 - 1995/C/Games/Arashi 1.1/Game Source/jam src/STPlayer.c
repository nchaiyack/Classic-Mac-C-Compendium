/*/
     Project Arashi: STPlayer.c
     Major release: Version 1.1, 7/22/92

     Last modification: Thursday, July 8, 1993, 23:04
     Created: Thursday, March 23, 1989, 23:15

     Copyright � 1989-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "STCrack.h"
#include "PlayOptions.h" 
#include "HighScores.h"	/* mz */
#include "heroflags.h"

#define	PLAYERMAXMOVE	(ThisLevel.plMaxMove)
#define	DROPSPEED		(5)
#define	POSTMORTEMTIMER	(60)
#define	NEXTBLOWTIMER	(2)
#define	SENSITIVITY		(PlayOptions->mouseSensitivity)
#define	MAXRECORDSTEPS	(400)
#define freeLifeScore	(20000) /* (mz) */
#define MAXLIVES		(5) /* (mz) */

typedef	struct
{
	long	seed;
	long	frameCounter;
	long	count;
	int		levelNumber;
	char	moveData[MAXRECORDSTEPS];
}	PlayRecordType;

int				PlayKeyCodes[4];	/*	right, left, fire, zap	*/
PlayRecordType	*PlayRecord;
int				RecordStatus;
int				RecordCount;
Handle			RecordedHandle;

int		HeroShifts[]={ 1,2,3,4,4,5,6,7 };

extern	Point			MTemp			: 0x828;
extern	Point			RawMouse		: 0x82c;
extern	Point			Mouse			: 0x830;
extern	int				CrsrNewCouple	: 0x8ce;
extern	EventRecord		Event;

extern	Initials	*HiScoresToBeat;
extern	int 		ScoretoBeatIndex;

		Point			CenterMouse;
static	int			cumulativerr=0;

		Player		Hero;



static	Point		ScorePosition;
static	int			ScoreSegmentScale;

void	DisplayLives()
{
	register	int		i;
	register	int		x,y,dx,sx;
	
	dx = ScoreSegmentScale + ScoreSegmentScale;
	sx = (dx+dx+dx)>>2;	/*	dx*3/4	*/

	x = ScoreSegmentScale;
	y = ScorePosition.v+ScoreSegmentScale;

	VA.field= 2;
	VA.offset=0;
	VA.color= 0;
	for(i=0;i<Hero.lives;i++)
	{	DrawStaticPlayer(x,y,sx,0);
		x+=dx;
	}
}

void	RemoveLife()
{
	register	int		x,y,dx,sx;
	
	Hero.lives--;

	if(Hero.lives < 8 && Hero.lives>=0)
	{	dx = ScoreSegmentScale + ScoreSegmentScale;
		sx = (dx+dx+dx)>>2;	/*	dx*3/4	*/
	
		x = ScoreSegmentScale+dx*Hero.lives;
		y = ScorePosition.v+ScoreSegmentScale;
		
		VA.field=2;
		VA.offset=0;
		VA.color=-1;
		
		DrawStaticPlayer(x,y,sx,0);
	}
}
void	AddLife()
{
	register	int		x,y,dx,sx;
	
	if(Hero.lives < 8 && Hero.lives>=0)
	{	dx = ScoreSegmentScale + ScoreSegmentScale;
		sx = (dx+dx+dx)>>2;	/*	dx*3/4	*/
	
		x = ScoreSegmentScale+dx*Hero.lives;
		y = ScorePosition.v+ScoreSegmentScale;
		
		VA.field=2;
		VA.offset=0;
		VA.color=0;
		
		DrawStaticPlayer(x,y,sx,0);
	}

	Hero.lives++;
}

void	DisplayScore()
{
	VA.color=BG1;
	VA.segmscale=ScoreSegmentScale;
	VADrawNumber(Hero.score,ScorePosition.h,ScorePosition.v);
}

void	ZeroScore()
{
	ScoreSegmentScale=VA.frame.bottom>>5;
	if(ScoreSegmentScale<1)
		ScoreSegmentScale=1;
	do
	{
		ScorePosition.h=(ScoreSegmentScale+7)*8+10;
		ScorePosition.v=ScoreSegmentScale+ScoreSegmentScale+4;
	}	while((ScorePosition.h>VA.frame.right/3) & (ScoreSegmentScale-- > 0));
	
	ScoreSegmentScale++;

	if(!(Hero.Flags & SavedGameStartMask))
	{
		Hero.score=0;
		Hero.lives=2;
		Hero.freeLives = freeLifeScore; /* reset free life score to freeLifeScore (mz) */	
		Hero.Flags = 0;	/* init new game flag */
	}
	Hero.state=HeroPlaying;
}

void	IncreaseScore(points)
long	points;
{

	int			flashcount;
	
	Hero.score+=points;
	Hero.drawscore= 1;
	if (Hero.score > Hero.freeLives) 		/* check for free life (mz) 	*/
	{	
		if(Hero.lives < MAXLIVES || (Hero.lives <= MAXLIVES && Hero.state==HeroFlying))	
											/* no more than MAXLIVES lives 	*/
		{
			if(Hero.Flags & 0x0010)
			{
				PlayB(ZZFreeLife,999);        /* play free life sound (mz) 	*/
				if((!PlayOptions->noLoudSounds)&&(Hero.state!=HeroFlying))
					PlayA(ZZFreeLife,999);
				Hero.Flags |= 0x0001;		 /* set flash flag (mz) 		*/
			}
			AddLife();
		}
		Hero.freeLives = Hero.score - (Hero.score % freeLifeScore) + freeLifeScore;
		/* round score up to next interval */
	}
	
	if (Hero.score > HiScoresToBeat[ScoretoBeatIndex].score)
	{
		unsigned char	*namestr=(void *)"SCORE";
		unsigned char	position[3];
		long	scoretobeat;
		int		hsc=VA.color;
		int		x;
		
		while ((Hero.score > HiScoresToBeat[ScoretoBeatIndex].score) && ScoretoBeatIndex>=1)
			ScoretoBeatIndex--;
		if (ScoretoBeatIndex < 1) /* player now has top score */
		{	scoretobeat = Hero.score;
			ScoretoBeatIndex = 1;
			
			position[0] = 'H';
			position[1] = 'I';
		}
		else
		{	position[1] = ScoretoBeatIndex % 10 + '0';
			if(ScoretoBeatIndex >= 10)
				position[0] = ScoretoBeatIndex / 10 + '0';
			else
				position[0] = ' ';
		
			scoretobeat = HiScoresToBeat[ScoretoBeatIndex].score;
			namestr = HiScoresToBeat[ScoretoBeatIndex].name; 
		}
			
		position[2] = '-';
		
		VA.color = BG2; 
		if (ThisLevel.lvColor == 5)		/* check for invis lvl where BG2 is */
			VA.color=BG1;				/* also invis. (mz) 				*/
		
		VA.segmscale = 1+(ScoreSegmentScale >> 3);
		x = VA.frame.right - 1 - 8*(VA.segmscale*3+3);

		VAMoveTo(x,(VA.segmscale << 2)+4);
		VADrawText((char *) position,0,3);
		VADrawText((char *) namestr,0,5);
		
		VA.segmscale += VA.segmscale;
		x = VA.frame.right - (VA.segmscale + 7) + 1;      /* was - 10 */
		VADrawPadNumber(scoretobeat,x,(VA.segmscale*6)+5,8); 
		
		VA.color = hsc;
	}	/* end if */
}

void	UpdateEdgeLines()
{
	register	int		i;
	register	int		savedfield=VA.field;
	register	int		savedoffset=VA.offset;
	
	VA.field=1;
	VA.offset=1;
	
	for(i=0;i<ww.numsegs;i++)
	{
		if(Hero.lanemissing[i])
		{	if(!(Hero.lanestat[i] & PulsMask))		
			{	VA.color=0;		
				Hero.lanemissing[i]=0;
				VAStaticLine(ww.x[i][0],ww.y[i][0],ww.x[i+1][0],ww.y[i+1][0]);
			}
		}
		else
		{	if((Hero.lanestat[i] & PulsMask))		
			{	VA.color=1;
				Hero.lanemissing[i]=1;
				VAStaticLine(ww.x[i][0],ww.y[i][0],ww.x[i+1][0],ww.y[i+1][0]);
			}
		}
	}
}
void	CrackPlayer()
{
	register	int		angle,delta;
	register	int		x,y,dx,dy;
				int		x2,y2;
	
	x = ww.x[Hero.lane][0];
	y = ww.y[Hero.lane][0];
	
	x2 = ww.x[Hero.lane+1][0];
	y2 = ww.y[Hero.lane+1][0];
	
	dx = x-x2;
	dy = y-y2;

	delta = ww.unitlen[0]*4/3;
	angle = NextSeg[Hero.lane];

	AddCrack(	x,y,
				(dx>>3)-(dy>>2),(dy>>3)+(dx>>2),
				ThisLevel.shColor[0],delta,PlayerLeft,
				angle,4,POSTMORTEMTIMER);
	
	angle = ANGLES-angle;
	if(angle>=ANGLES)	angle-=ANGLES;
	
	AddCrack(	x2,y2,
				-(dx>>3)-(dy>>2),-(dy>>3)+(dx>>2),
				ThisLevel.shColor[0],delta,PlayerRight,
				angle,4,POSTMORTEMTIMER);		
}

void	NukePlayer()
{
	VA.color=ThisLevel.shColor[0];	/*	Shot color 0 is player also color.	*/
	BlowUpPlayer(	ww.x[Hero.lane][Hero.dropdepth],ww.y[Hero.lane][Hero.dropdepth],
					ww.x[Hero.lane+1][Hero.dropdepth]-ww.x[Hero.lane][Hero.dropdepth],
					ww.y[Hero.lane+1][Hero.dropdepth]-ww.y[Hero.lane][Hero.dropdepth],Hero.shifter);

}
void	PlayerCapture(status)
register	int		status;
{
	if(status & FlipMask)
	{	Hero.state = HeroDropping;
		PlayB(FallYell,999);  
		if(!PlayOptions->noLoudSounds)
			PlayA(FallYell,999);
	}
	else
	if(status & PulsMask)
	{	Hero.state=HeroPostMortem;
		Hero.timer=POSTMORTEMTIMER;

		CrackPlayer();
		NukePlayer();

		PlayB(FallZap,999);
		if(!PlayOptions->noLoudSounds)
			PlayB(FallZap,999);
	}
	else
	if(status & FuseMask)
	{	Hero.state=HeroDisintegrating;
		Hero.timer=NEXTBLOWTIMER;
		NukePlayer();
		PlayB(FallZap,999);
		if(!PlayOptions->noLoudSounds)
			PlayB(FallZap,999);
	}
	else if(status & PlasMask)
	{	Hero.state=HeroDisintegrating;
		Hero.timer=NEXTBLOWTIMER;
		NukePlayer();
		PlayB(PhazerOut,999);
		PlayA(PhazerOut,999);
	}
	else if(status & SpikeMask)
	{	Hero.state=HeroPostMortem;
		Hero.timer=POSTMORTEMTIMER;
		CrackPlayer();
		PlayB(FallZap,999);
		if(!PlayOptions->noLoudSounds)
			PlayA(FallZap,999);
	}
	
	cumulativerr=0;
}

void	GetSetMouse(where)
register	Point	*where;
{
	Point	oldwhere;

	oldwhere=Mouse;
	
	RawMouse=*where;
	MTemp=*where;
	CrsrNewCouple=-1;
	
	where->h=oldwhere.h-where->h;
	where->v=oldwhere.v-where->v;
}

void	WannaShoot(enable)
int		enable;
{
	if(enable)
	{	Hero.couldshoot=-1;
		if(!(Event.modifiers & btnState) || Hero.pendingshot)
		{	if(AddShot(Hero.lane))
				PlayA(PhazerIn,2);
			Hero.pendingshot=0;
		}
	}
	else	
	{	Hero.couldshoot=0;
		Hero.pendingshot=(Event.what==mouseDown);
	}
}

void	TextStuff(thecount)
int		thecount;
{
	int		stringid;
	Handle	TheString;
	char	*strp;
	int		totalwidth;
	char	empty[256];
	
	static	int		oldx;
	static	int		oldy;
	static	int		thelen;
	static	int		thescale;
	
	thecount += 64;

	if((thecount & 127)==0)
	{	if(thecount == 128)
		{	thelen = 0;
			oldx = 0;
			oldy = 0;
		}

		thescale = ScoreSegmentScale/4;
		if(thescale < 1) thescale = 1;
		VA.segmscale = thescale;
		
		if(thelen)
		{	VAMoveTo(oldx,oldy);
			for(totalwidth=0;totalwidth<thelen;totalwidth++)
			{	empty[totalwidth]=' ';
			}
			VADrawText(empty,0,thelen);
		}

		stringid = 128+(thecount>>7);
		if(stringid < 132)
		{	TheString = (Handle)GetString(stringid);
			HLock(TheString);
			strp = *TheString;
			thelen = *strp;	

			totalwidth = thelen * (thescale * 3 + 3);

			oldx = (VA.frame.right-VA.frame.left - totalwidth)/2;
			oldy = VA.frame.bottom-(thescale << 3);
			VAMoveTo(oldx,oldy);

			VA.color = BG1;
			VADrawText(strp,1,thelen);
			
			HUnlock(TheString);
			ReleaseResource(TheString);
		}
	}
}

void	UpdatePlayer()
{
				Point	mouse;
	register	int		delta;
	register	int		i,j;
	register	int		rotationtype;
	register	int		steps=ww.numsegs*8;
				int		shootenable;
				int		DrawPlayerFlag;
				long	KeyboardFlags[4];

				Handle		ZapColorsHandle;
				ColorSpec	*ColorTable;
	
	
	if(Hero.state!=HeroFlying && Hero.state!=HeroPostMortem)
		UpdateEdgeLines();

	rotationtype = PlayOptions->rotationType;

	if(PlayOptions->absMoveFlag)
	{
		mouse=CenterMouse;
		GetSetMouse(&mouse);
		
		if(rotationtype < 2)	delta=  mouse.h;
		else					delta= -mouse.v;
		
		if(rotationtype & 1)	delta = -delta;

		delta = ((delta*SENSITIVITY) >> 3) + cumulativerr;
	}
	else
	{
		long	hort,vort;
		int		lane1,lane2;
		
		mouse=CenterMouse;
		GetSetMouse(&mouse);
		
		lane1=Hero.lane-1;
		lane2=Hero.lane+2;
		if(ww.wraps)
		{	if(lane1<0) 			lane1 += ww.numsegs;
			if(lane2>ww.numsegs) 	lane2 -= ww.numsegs;
		}
		else
		{	if(lane1<0)				lane1 = 0;
			if(lane2>ww.numsegs)	lane2 = ww.numsegs;
		}
		hort=ww.x[lane2][0]-ww.x[lane1][0];
		vort=ww.y[lane2][0]-ww.y[lane1][0];
		
		hort *= mouse.h;
		vort *= mouse.v;
		
		delta=(mouse.h<0 ? mouse.h : -mouse.h) +
			  (mouse.v<0 ? mouse.v : -mouse.v);

		if(hort+vort > 0)
		{	delta = -delta;
		}
		
		delta = ((delta*SENSITIVITY) >> 3);
	}
	
	GetKeys(KeyboardFlags);	
	if(BitTst(KeyboardFlags,PlayKeyCodes[0]))
	{	delta = SENSITIVITY;
	}
	else
	if(BitTst(KeyboardFlags,PlayKeyCodes[1]))
	{	delta = -SENSITIVITY;
	}
	
	if(BitTst(KeyboardFlags,PlayKeyCodes[2]))
	{	Hero.pendingshot = 1;
	}

	if(Hero.state == HeroFlying || Hero.state == HeroPlaying)
	{	
		cumulativerr=delta;
		
		if(delta>PLAYERMAXMOVE) delta=PLAYERMAXMOVE;
		if(delta<-PLAYERMAXMOVE) delta=-PLAYERMAXMOVE;
	
		if(RecordStatus != NormalPlay)
		{	TextStuff(RecordCount);
			if(RecordStatus == RecordPlay)
			{	if(RecordCount < MAXRECORDSTEPS)
				{	PlayRecord->moveData[RecordCount] = (delta << 2) +
							((!(Event.modifiers & btnState) || Hero.pendingshot) ? 1 : 0) +
							((Event.what==keyDown && BitTst(KeyboardFlags,PlayKeyCodes[3])) ?
							 2 : 0);
						
					RecordCount++;
					PlayRecord->count = RecordCount;
				}
				else
				{	RecordStatus = NormalPlay;
					Hero.state = HeroDead;
					Hero.lives = 0;
				}
			}
			else
			{	KeyboardFlags[0] = 0;
				KeyboardFlags[1] = 0;
				KeyboardFlags[2] = 0;
				KeyboardFlags[3] = 0;

				if(RecordCount < PlayRecord->count)
				{	delta = PlayRecord->moveData[RecordCount];
					RecordCount++;
					Hero.pendingshot = delta & 1;
					Event.modifiers |=  btnState;
					if(delta & 2)
					{	Event.what=keyDown;
						BitSet(KeyboardFlags,PlayKeyCodes[3]);
					}
					delta >>= 2;
				}
				else
				{	RecordStatus = NormalPlay;
					Hero.state = HeroDead;
					Hero.lives = 0;
				}
			}
		}

		cumulativerr-=delta;
		Hero.rawpos+=delta;

		if(Hero.rawpos<0)
		{	if(ww.wraps)
			{	while(Hero.rawpos<0)
				{	Hero.rawpos+=steps;
				}
			}
			else
				Hero.rawpos=0;
		}
		else
		if(Hero.rawpos>=steps)
		{	if(ww.wraps)
			{	while(Hero.rawpos>=steps)
				{	Hero.rawpos-=steps;
				}
			}
			else
				Hero.rawpos=steps-1;
		}
	
		Hero.shifter=HeroShifts[Hero.rawpos & 7];
		Hero.lane=Hero.rawpos / 8;
	
		shootenable=Hero.oldlane != Hero.lane || (VA.FrameCounter & 1);

		if(Hero.lanestat[Hero.oldlane])
		{	Hero.lane=Hero.oldlane;
			PlayerCapture(Hero.lanestat[Hero.oldlane]);
		}
	}
	

	DrawPlayerFlag = !VA.Late;
	
	switch(Hero.state)
	{	case HeroDropping:
			Hero.dropdepth+=(Hero.dropdepth>>3)+DROPSPEED;
			if(Hero.dropdepth>=DEPTH)
			{	Hero.state=HeroPostMortem;
				Hero.timer=POSTMORTEMTIMER;
				Hero.dropdepth=DEPTH;
				NukePlayer();
				PlayB(Blow,100);
			}
			else
			{	DrawPlayerFlag=1;
			}
			break;
		case HeroDisintegrating:
			DrawPlayerFlag=0;
			Hero.timer--;
			if(Hero.timer<=0)
			{	Hero.state=HeroPostMortem;
				Hero.timer=POSTMORTEMTIMER;
				Hero.shifter = (Hero.shifter+4) & 7;
				NukePlayer();
			}
			break;
		case HeroPostMortem:
			DrawPlayerFlag=0;

			Hero.timer--;
			if(Hero.timer<=0)
			{	Hero.state=HeroDead;
			}
			break;
		default:
			DrawPlayerFlag=1;
			break;
	}

	if(Hero.oldlane!=Hero.lane)
	{	PlayA(Bonk,1);
		i = (delta>0) ? 1 : -1;
		j = (delta>0) ? 1 : 0;
		while(Hero.oldlane!=Hero.lane)
		{	if(Hero.segmstat[Hero.oldlane+j] || Hero.lanestat[Hero.oldlane])
			{	if(Hero.segmstat[Hero.oldlane+j])
				{	Hero.lane=Hero.oldlane+j;
					PlayerCapture(Hero.segmstat[Hero.lane]);
				}
				Hero.lane=Hero.oldlane;
				if(Hero.lanestat[Hero.oldlane])
					PlayerCapture(Hero.lanestat[Hero.oldlane]);
			}
			else
			{	Hero.oldlane+=i;
				if(Hero.oldlane<0)
				{	Hero.oldlane=ww.numsegs-1;
				}
				else
				if(Hero.oldlane>=ww.numsegs)
				{	Hero.oldlane=0;
				}
			}
		}
	}

	if(DrawPlayerFlag)
	{	VA.color=ThisLevel.shColor[0];	/*	Shot color 0 is player also color.	*/
		DrawPlayer(	ww.x[Hero.lane][Hero.dropdepth],ww.y[Hero.lane][Hero.dropdepth],
					ww.x[Hero.lane+1][Hero.dropdepth]-ww.x[Hero.lane][Hero.dropdepth],
					ww.y[Hero.lane+1][Hero.dropdepth]-ww.y[Hero.lane][Hero.dropdepth],Hero.shifter);
		
		if(Hero.flydepth==0)
		{	VALine(	ww.x[Hero.lane][Hero.dropdepth],ww.y[Hero.lane][Hero.dropdepth],
						ww.x[Hero.lane][DEPTH],ww.y[Hero.lane][DEPTH]);
			VALine(	ww.x[Hero.lane+1][Hero.dropdepth],ww.y[Hero.lane+1][Hero.dropdepth],
						ww.x[Hero.lane+1][DEPTH],ww.y[Hero.lane+1][DEPTH]);
		} 
	}
		
	/* if lower 4 bits !=0 then colors are being flashed.  Counts up to 12	*/
	/* so it cycles thru the CLOT's twice modulo 6, then reset to 0. (mz)	*/
		
	if((Hero.superzapping==1 || Hero.Flags & 0x000F) && Hero.state != HeroFlying) 
	{	
		if ( (!(Hero.Flags & 0x000F)) || (Hero.Flags & 0x000C)==0x000C ) 
		{
			/* flashing is done	*/
			ZapColorsHandle=GetResource('CLOT',ThisLevel.lvColor);
			Hero.Flags &= 0x7FF0;
		}
		else{ /* run through all colors to enhance flash (mz) */
			ZapColorsHandle=GetResource('CLOT',((Hero.Flags & 0x000F) % 6 + 1));
			Hero.Flags++;
		}
		VASetColors(ZapColorsHandle);
		ReleaseResource(ZapColorsHandle);
	}

	if(Hero.state == HeroPlaying)
	{	Hero.superzapping=0;
		if(Event.what==keyDown && BitTst(KeyboardFlags,PlayKeyCodes[3]))
		{	if(ThisLevel.plSuperZaps>0)
			{	ThisLevel.plSuperZaps--;
				Hero.superzapping= 1;
				PlayB(ZZSuperZap,998);     /* mz */
				if(!PlayOptions->noLoudSounds)
					PlayA(ZZSuperZap,998);
				ZapColorsHandle=VAGetColors();

				ColorTable=(void *)(*ZapColorsHandle);
				ColorTable[8].rgb.red=65535;
				ColorTable[8].rgb.green=65535;
				ColorTable[8].rgb.blue=65535;
				VASetColors(ZapColorsHandle);
				DisposHandle(ZapColorsHandle);
			}
			else
			if(ThisLevel.plSuperZaps==0)
			{	ThisLevel.plSuperZaps--;
				Hero.superzapping= 2;
			}
		}
	}
	if(Hero.state == HeroPlaying || Hero.state == HeroFlying)
		WannaShoot(shootenable);

	for(i=0;i<ww.numsegs;i++)
	{	Hero.lanestat[i]=0;
		Hero.segmstat[i]=0;
	}
	Hero.segmstat[i]=0;
	
	if(Hero.drawscore)
		DisplayScore();
}

void	InitPlayer()
{
	register	int		i;
				Point	foo;
	
	BlockMove(*GetResource('KEYS',128),PlayKeyCodes,8);
	CenterMouse.h=(VA.DisplayFrame.right+VA.DisplayFrame.left)/2;
	CenterMouse.v=(VA.DisplayFrame.bottom+VA.DisplayFrame.top)/2;
	
	foo=CenterMouse;
	GetSetMouse(&foo);
	cumulativerr=0;
	
	Hero.flydepth=0;
	Hero.dropdepth=0;
	Hero.state=HeroPlaying;
	Hero.timer=0;
	
	for(i=0;i<MAXSEGS;i++)
	{	Hero.lanestat[i]=0;
		Hero.segmstat[i]=0;
		Hero.lanemissing[i]=0;
	}
	Hero.segmstat[MAXSEGS]=0;
	Hero.endtimer=ThisLevel.endTimer;
}

void	StartPlayRecord()
{
	RecordStatus = RecordPlay;
	RecordCount = 0;
	PlayRecord->count = 0;
	PlayRecord->seed = VARandSeed;
	PlayRecord->frameCounter = VA.FrameCounter;
	PlayRecord->levelNumber = ThisLevel.lvNext;

	Hero.lane = 0;
	Hero.shifter = 0;
	Hero.rawpos = 0;
}
void	StartPlayback()
{
	RecordStatus = PlaybackPlay;
	RecordCount = 0;
	VARandSeed = PlayRecord->seed;
	VA.FrameCounter = PlayRecord->frameCounter;
	ThisLevel.lvNext = PlayRecord->levelNumber;
	Hero.lane = 0;
	Hero.shifter = 0;
	Hero.rawpos = 0;
}
void	StartNormalPlay()
{
	RecordStatus = NormalPlay;
}

void	LoadPlayRecord()
{
	RecordedHandle = GetResource('MYGA',128+((ThisLevel.lvNext>>1) & 7));
	if(GetHandleSize(RecordedHandle) != sizeof(PlayRecordType))
	{	SetHandleSize(RecordedHandle,sizeof(PlayRecordType));
	}
	HLock(RecordedHandle);

	PlayRecord = (PlayRecordType *)*RecordedHandle;
}

void	UnloadPlayRecord()
{
	HUnlock(RecordedHandle);
	ReleaseResource(RecordedHandle);
}

void	WriteRecordedPlay()
{
	ChangedResource(RecordedHandle);
	WriteResource(RecordedHandle);
}
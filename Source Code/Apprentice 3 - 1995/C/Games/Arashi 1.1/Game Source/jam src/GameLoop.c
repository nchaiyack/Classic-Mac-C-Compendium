/*/
     Project Arashi: GameLoop.c
     Major release: Version 1.1, 7/22/92

     Last modification: Thursday, July 8, 1993, 23:11
     Created: Sunday, February 5, 1989, 2:21

     Copyright � 1989-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "Shuddup.h"
#include "PlayOptions.h"
#include "GamePause.h"
#include "HighScoresToBeat.h"		/* mz */
#include <GestaltEqu.h>
#include <OSUtils.h>
#include <Types.h>
#include "heroflags.h"

/* #define DEBUGGING_ROUTINES */
#define	UNNATURALDEATH	0
#define NATURALDEATH	1

static 	int	highestLastLevel=9;			/* (mz) */
		int	FinalDeathMode=NATURALDEATH;
		
extern	Player	Hero;
extern	int		OldSound;				/*	Flag for Sound Kit		*/


Initials	*HiScoresToBeat;
int 		ScoretoBeatIndex;

EventRecord		Event;
WindowPtr		BackdropWind;


void	GameLoop(Options)
int		Options;
{
	int			i;
	long int	levelStBonus=0;
	long int	levelBonus=0;
	int			levelComplete=0;
	Handle		ScoreHand;
	int			DoomsdayDeath = 0;
	
	/* read scores from resources  for top right corner */
	if (PlayOptions->restart == 0)
		ScoreHand=GetResource('SCOR',128);
	else 
		ScoreHand=GetResource('SCOR',129);
	HLock(ScoreHand);
	HiScoresToBeat = (Initials *)*ScoreHand;
	ScoretoBeatIndex = 50;		/* start at the bottom */
	
	VAEraseBuffer();

	/*	Flush out some events and let MultiFinder update windows.	*/
	for(i=0;i<10;i++)
		GameEvent();

	ZeroScore();
	
	if(Options == NormalPlay)
	{	StartNormalPlay();
	}
	else
	{	Hero.lives = 0;
		if(Options == PlaybackPlay)
			ThisLevel.lvNext = TickCount() & 7;

		LoadPlayRecord();
		if(Options == RecordPlay)
		{	
			StartPlayRecord();
		}
		else
		if(Options == PlaybackPlay)
		{	
			StartPlayback();
		}
	}

	DisplayLives();

	do
	{	VACatchUp();

		DisplayScore();

		if(Hero.state != HeroDead)
		{	AddLife();
			if (levelComplete == 1)
			{
				/* check to see if this was first level.  If so give STBonus.		*/
				if (!(Hero.Flags & STBonusGivenMask)){
					
					/* set bonus granted and show zap flags 						*/
					
					Hero.Flags |= (ShowSuperZapMsgMask + STBonusGivenMask ); 
					IncreaseScore(levelStBonus); /* give starting bonus				*/
				} 
				
				Hero.Flags |= (ShowSuperZapMsgMask);
				
				/* if  a saved game and start level was already done then turn on	*/
				/* superzap msg flag */
				
				IncreaseScore(levelBonus);
				DisplayScore();			
			}
			else
				if(!(Hero.Flags & SavedgameFirstLVLCompleteMask))
					Hero.Flags &= (FLAGSMASK - ShowSuperZapMsgMask);
			levelComplete = 0;
			levelBonus = 0;
			levelStBonus = 0;
			STLoadLevel(); 
			CreateFit();
		}
		else if(DoomsdayDeath)
		{
			int	numzaps=ThisLevel.plSuperZaps;
			int	lvl = 	ThisLevel.lvNumber;
			
			STLoadLevel(); 
			ThisLevel.plSuperZaps = numzaps;	/* Keep old # of zaps 				*/
			ThisLevel.lvNumber = lvl; 
			ThisLevel.lvNext = lvl + 1;
			Hero.Flags &= (FLAGSMASK - ShowSuperZapMsgMask); /* unset re-zap msg	*/
			CreateFit();
			Hero.Flags |= ShowSuperZapMsgMask;	/* set show re-zap msg 				*/
			DoomsdayDeath = 0;
			
		}
		else
		{	VA.color=0;
			RedrawField();
		}
		
		RemoveLife();
		STInitialize();
	
		do
		{	if(Options != PlaybackPlay)
			{	GameEvent();
				switch(Event.what)
				{	case keyDown:
					case autoKey:
						switch(Event.message & 0xFF)
						{	case 'q':
							case 'Q':
							case 27:
							case 13:	Hero.state=HeroDead;
										Hero.lives=0;				
										FinalDeathMode=UNNATURALDEATH;
										break;
							case ' ':	/* Handled in player routine	*/
																	break;
							case 'p':
							case 'P':	GamePause(GameRunning);	 
										break;
	#ifdef DEBUGGING_ROUTINES
							case 26:	DumpThisLevel();			break;
							case '1':	AddLife();					break;
							case '2':	RemoveLife();				break;

							case 'z':	CreateNewFlipper();			break;
							case 'x':	CreateNewTanker(0);			break;
							case 's':	CreateNewTanker(1);			break;
							case 'd':	CreateNewTanker(2);			break;
							case 'c':	CreateNewPulsar();			break;
							case 'v':	CreateNewBall();			break;
							case 'o':	ThisLevel.totalCount = 0;	break;

						/*	case '@':	Hero.state=HeroFlying;
										DemoMode();					break;
							case 'k':	TestZoomRect();				break;
							case 'l':	DoLevelSelect();			break; */
							default:	Hero.state=HeroFlying;		break;
 	#endif
						}
						break;
				}
			}
			else	/*	Playback active.	*/
			{	if(GamePeekEvent())
				{	Hero.lives = 0;
					Hero.state = HeroDead;
				}
				Event.what = 0;
			}
			if(ThisLevel.activeCount<ThisLevel.boredomCount)
				ThisLevel.probIncrease=ThisLevel.boredProb;
			else
				ThisLevel.probIncrease=0;
			ThisLevel.activeCount=0;

			STUpdate();

			VA.segmscale=5;

			VA.color = BG2; 
			if (!(PlayOptions->restart == 0))	/* only show count if Practice mode */
			{
				if (ThisLevel.lvColor == 5)		/* check for invis lvl where BG2 is */
					VA.color=BG1;				/* also invis. (mz) 				*/
				VADrawPadNumber(ThisLevel.totalCount,VA.frame.right-10,60,3);
				VADrawPadNumber(ThisLevel.edgeCount,VA.frame.right-10,80,3);
				VA.color = BG2;
			}
			VAStep();
			
			if(Hero.state == HeroPlaying)
			{	if(ThisLevel.edgeCount==ThisLevel.totalCount)
				{	Hero.endtimer--;
					if(Hero.endtimer<=0)
					{	Hero.state=HeroFlying;
						Hero.flydepth=0;
						levelComplete=1;
						levelStBonus=ThisLevel.lvStBonus;
						levelBonus=ThisLevel.lvBonus;
					}
				}
			}
		
		} while(Hero.state != HeroDead && Hero.state != HeroFlying);

		if(Hero.state == HeroFlying)
			STFlyThruLoop(Options);
			
		if( (ThisLevel.Doomsday==1) && (Hero.lives > 0)) /* player has died in Doomsday level */
		{	DoomsdayDeath = 1;
			VA.color=-1;	/*	Black					*/
			RedrawField();
			RedrawSpikes();
		}
	
	} while(Hero.lives>0 || Hero.state != HeroDead);
	if(FinalDeathMode  && (Options == NormalPlay))
		STFlyOutLoop();								/* Fly out at end */
	
	FinalDeathMode = NATURALDEATH;
	highestLastLevel = ThisLevel.lvNumber - 1;	/* save highest level *completed*	*/
	
	/* Make sure saved game flag does not carry into next game.			
	Hero.Flags &= (FLAGSMASK - SavedGameStartMask); */
	
	Hero.Flags = NOFLAGSMASK;
	
	if(Options != NormalPlay)
	{	if(Options == RecordPlay)
		{	WriteRecordedPlay();
		}
		UnloadPlayRecord();
	}	
}


int		StormStart()
{
	int				zero=0;
	int				i;
	Handle			TheDevice;
	long			myFeature;
	
	DoInits();
	
	if(installASHI())
		return -1;
		
	DisableSuperClock(); 
	
	/* Now do check for Color Quickdraw. Any version above simple 8 bit will do. */
	if(Gestalt(gestaltQuickdrawVersion, &myFeature))
	{
		ParamText("\pARASHI encountered an unrecoverable _GESTALT error.",nil,nil,nil);
		Alert(131,0);
		ExitToShell();
	}
	if (myFeature < gestalt8BitQD)
	{
		ParamText("\pARASHI requires Color Quickdraw and has not detected it in your System.",nil,nil,nil);
		Alert(131,0);
		ExitToShell();
	}
	
	
	randSeed= Ticks;
	VARandSeed= Ticks;

	TheDevice=ScreenSelect();
	if(TheDevice==0)	return -1;
	
	if(PlayOptions->sys607Sound)
	{	OldSound=0;
	}
	else
	{	OldSound=1;
	}
	if(!PlayOptions->soundOff)
		InitSoundKit();
		
	if(PlayOptions->blankUnused)
		BackdropWind = NewWindow(0,&((*GrayRgn)->rgnBBox),&zero,-1,plainDBox,0,0,0);
	else
	{	Rect	SmallRect = { 0,0,10,10};
		BackdropWind = NewWindow(0,&SmallRect,&zero,-1,plainDBox,0,0,0);
	}	
	VAInit(TheDevice);
					
	if(PlayOptions->verticalGame)
	{	i=VA.frame.bottom*4L/5;
		if(VA.frame.right>i)
			VA.frame.right=i;
	}

	VAInitFractalLines();
	STAllocate();

	/*	Flush out some events and let MultiFinder update windows.	*/
	for(i=0;i<10;i++)	GameEvent();
	
	return 0;
}

long	PlayGame(HighScore,Options)
long	HighScore;
int		Options;
{
	int		i;
	
	Hero.score = -1;
	i=DoLevelSelect(highestLastLevel);
	if(Hero.Flags & SavedGameStartMask)
		Hero.Flags &= (FLAGSMASK - SavedgameFirstLVLCompleteMask);
	if(i>=0)
	{	VA.FrameSpeed=3;
		ThisLevel.lvNext=i;
		GameLoop(Options);
	}
	else
		Hero.score = -1;	
	return	Hero.score;
}

void	PlayDemoGame()
{
	long	seeder;
	
	seeder = VARandSeed;
	GameLoop(PlaybackPlay);
	VARandSeed = seeder;
}

void	RecordDemoGame()
{
	static	demoLevelStart = 10;

	ThisLevel.lvNext = demoLevelStart;
	demoLevelStart+=2;
	GameLoop(RecordPlay);
}
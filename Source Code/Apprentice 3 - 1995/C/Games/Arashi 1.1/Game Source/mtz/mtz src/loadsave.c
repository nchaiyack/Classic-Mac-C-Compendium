#include <VA.h>
#include "STORM.h"
#include "GamePause.h"
#include "PlayOptions.h"

extern	Player			Hero;		/* mz */
extern	int				ScoretoBeatIndex;	/* mz */

typedef struct{
		int		lvnum;
		int		Doomsday;
		long	score;
		int		lives;
		long	freelives;
		int		RestartMode;
		int		Flags;
		int		NumZaps;
		} **GameRecHand, GameRec;
		
void SaveGameF()
{
	GameRecHand	TheGameHand;
	GameRec		TheGame;
								
	TheGameHand = GetResource('SVGM',128 +  PlayOptions ->restart);
	(**TheGameHand).lvnum = ThisLevel.lvNumber; 
	(**TheGameHand).Doomsday = ThisLevel.Doomsday;
	(**TheGameHand).lives = Hero.lives;
	(**TheGameHand).score = Hero.score;
	(**TheGameHand).freelives = Hero.freeLives;
	(**TheGameHand).RestartMode = PlayOptions ->restart;
	(**TheGameHand).Flags = Hero.Flags;
	(**TheGameHand).NumZaps = ThisLevel.plSuperZaps;
	ChangedResource(TheGameHand);
	WriteResource(TheGameHand);
	ReleaseResource(TheGameHand);
	if(PlayOptions ->restart)
		ParamText("\pPractice",nil,nil,nil);
	else
		ParamText("\pArcade",nil,nil,nil);
	Alert(500,0); 
}

void LoadGameF()
{	GameRecHand	TheGameHand;
	GameRec	TheGame;
								
	TheGameHand = GetResource('SVGM',128 +  PlayOptions->restart);
								
	PlayOptions ->restart = (**TheGameHand).RestartMode;
							
	ThisLevel.lvNext = (**TheGameHand).lvnum;
	ThisLevel.Doomsday = (**TheGameHand).Doomsday;

	STLoadLevel();	

	/* if a Doomsday level dont reset # of zaps */
	if(ThisLevel.Doomsday)
		ThisLevel.plSuperZaps = (**TheGameHand).NumZaps;
	Hero.lives = (**TheGameHand).lives;			
	Hero.score = (**TheGameHand).score;			
	Hero.drawscore = 1;		
	Hero.freeLives = (**TheGameHand).freelives;			
	Hero.Flags = (**TheGameHand).Flags;
	ReleaseResource(TheGameHand);
}
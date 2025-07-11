/*/
     Project Arashi: GamePause.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, March 21, 1993, 15:17
     Created: Wednesday, October 9, 1991, 23:45

     Copyright � 1991-1993, Juri Munkki
/*/

#include <VA.h>
#include "STORM.h"
#include "GamePause.h"
#include "PlayOptions.h"
#include "loadsave.h"
#include "heroflags.h"

Rect	VAfullframe;

extern	int				ScoretoBeatIndex;	/* mz */

extern	EventRecord		Event;
extern	WindowPtr		BackdropWind;

extern	Player			Hero;		/* mz */

extern	Point			CenterMouse;
extern	void			GetSetMouse(Point *where);

extern	int				PlayKeyCodes[];		/*	right, left, fire, zap	*/

#define	PAUSEDIALOG	129
enum	{	EndPauseButton = 1, RememberKeysButton,
			RotateRightRadio = 5, RotateLeftRadio,
			FireRadio, SuperZapRadio, SaveGame};

/* typedef struct{
		int		lvnum;
		int		lvType;
		long	score;
		int		lives;
		long	freelives;
		int		ShowFuseScores;
		int		RestartMode;
		int		Flags;
		int		NumZaps;
		} **GameRecHand, GameRec; */
		
void	GamePauseCleanup()
{
	Point		NoWhere;
	long		EndTime;

	NoWhere = CenterMouse;
	GetSetMouse(&NoWhere);
	Delay(5,&EndTime);

	VA.color = 0;
	RedrawField();
	RedrawSpikes();
	DisplayLives();
}

void	GamePause(cleanup)
int		cleanup;
{
	DialogPtr	PauseDialog;
	DialogPtr	DialogHit;
	int			ItemHit;
	int			ColorStatus = -1;
	int			KeyToChange;
	long		KeyRecord[4];
	int			WhichKey;
	int			k; /* mz */
	int			i;	/* mz */
	Rect		tmpRect = VA.frame;
	
	HideWindow(BackdropWind);
	HideWindow(VA.window);
	VARestoreColors();

	PauseDialog = GetNewDialog(PAUSEDIALOG,0,(WindowPtr)-1);
	CenterWindow(PauseDialog);
	ShowWindow(PauseDialog);
	ShowCursor();
	ItemHit = 0;
	KeyToChange = 1;
	SetItemValue(PauseDialog, RotateLeftRadio, 1);
	
	if(cleanup != GameRunning)
		SetItemEnable(PauseDialog, SaveGame, 255);
		
	
	
	do
	{	if(((*(*((GDHandle)VA.device))->gdPMap)->pixelSize == 8) != ColorStatus)
		{	ColorStatus = !ColorStatus;
			SetItemEnable(PauseDialog, EndPauseButton, ColorStatus ? 0 : 255);
		}
		
		
		GetKeys(&KeyRecord);
		if(KeyRecord[0] || KeyRecord[1] || KeyRecord[2] || KeyRecord[3])
		{	
			for(WhichKey=0;WhichKey<128;WhichKey++)
			{	if(BitTst(&KeyRecord,WhichKey))
				{	WhichKey+=1000;
				}
			}
			
			if(WhichKey > 1000)
			{	PlayKeyCodes[KeyToChange] = WhichKey-1001;
				SetItemValue(PauseDialog, RotateRightRadio+KeyToChange, 0);
				SetItemValue(PauseDialog, RotateRightRadio+KeyToChange, 1);
			}
		}
		
		if(GetNextEvent(everyEvent,&Event))
		{	if(IsDialogEvent(&Event) && Event.what != keyDown)
			{	if(!DialogSelect(&Event,&DialogHit,&ItemHit))
					ItemHit=0;
				else
				{	switch(ItemHit)
					{	case RotateRightRadio:
						case RotateLeftRadio:
						case FireRadio:
						case SuperZapRadio:
							SetItemValue(PauseDialog, RotateRightRadio+KeyToChange, 0);
							KeyToChange = ItemHit - RotateRightRadio;
							SetItemValue(PauseDialog, RotateRightRadio+KeyToChange, 1);
							break;
						case RememberKeysButton:
							{	Handle	TheKeys;
							
								TheKeys = GetResource('KEYS',128);
								BlockMove(PlayKeyCodes,*TheKeys,8);
								ChangedResource(TheKeys);
								WriteResource(TheKeys);
								
							}
							break;
						case SaveGame:
								SaveGameF();
							break;
					}
				}
			}
		}
	} while(ItemHit != EndPauseButton);
	
	HideCursor();
	ShowWindow(BackdropWind);

	DisposDialog(PauseDialog);
	ShowWindow(VA.window);
	RectRgn(VA.window->visRgn,&VA.window->portRect);
	SetPort(VA.window);

	{
		int		oldBackTasks = PlayOptions->noBackgroundTasks;
		
		PlayOptions->noBackgroundTasks = FALSE;
		for(i=0;i<50;i++)
		{	GameEvent();
		}
		PlayOptions->noBackgroundTasks = oldBackTasks;
	}
	
	VAEraseBuffer();
	
	if((cleanup != LevelSelectRunning) && (cleanup != TitlesRunning))
	{
		VA.segmscale=(VA.frame.bottom>>6);			/* mz */
		if(VA.segmscale<1) VA.segmscale=1;
		k=VA.color;
		VA.color=BG2;
		VADrawNumber(ThisLevel.lvNumber,VA.frame.right/2-VA.segmscale-3,VA.segmscale*2+4);
		VA.color=k;
	}
	
	
	switch(cleanup)
	{	case GameRunning:
			GamePauseCleanup();
			break;
		case FlythruRunning:
			DisplayLives();					
		case LevelSelectRunning:
			{	Point		NoWhere;
				long		EndTime;
			
				NoWhere = CenterMouse;
				GetSetMouse(&NoWhere);
				Delay(5,&EndTime);
			}
			break;
	}
	
	VACatchUp();
	VAStep();
	
	Event.what = 0;
	Event.modifiers = btnState;
}

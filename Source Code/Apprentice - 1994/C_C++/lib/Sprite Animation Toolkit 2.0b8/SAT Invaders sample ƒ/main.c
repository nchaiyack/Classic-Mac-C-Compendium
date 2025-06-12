//¥ C translation from Pascal source file: main.p

//¥ ================================================.
//¥ =============== SATInvaders main unit ================.
//¥ ================================================.

//¥ Example file for Ingemars Sprite Animation Toolkit.
//¥ © Ingemar Ragnemalm 1992.
//¥ See doc files for legal terms for using this code.

//¥ SATInvaders is a very simple game demonstrating how to use the Sprite Animation.
//¥ Toolkit. It is intended as a minimal demonstration, without many features and options.
//¥ that the other sample program, HeartQuest, has. No high scores or even score, only.
//¥ one life, doesn't save settings, only one kind of enemy, no special effects like explosions.
//¥ etc.

//¥ main SATInvaders.c

#include <TransSkel.h>
#include <SAT.h>
#include "InvadeSAT.h"
//#include "GameGlobals.h"
//¥ SoundConst, sPlayer, sEnemy, sShot, sMissile;

// All the following is now in InvadeSAT.h
//extern void		InitEnemy(void);
//extern pascal void		SetupEnemy(SpritePtr sp);
//extern pascal void		HandleEnemy(SpritePtr me);
//extern void		InitPlayer(void);
//extern pascal void		SetupPlayer(SpritePtr player);
//extern pascal void		HandlePlayer(SpritePtr me);

Boolean soundFlag, plotFastFlag;

//¥ --------------------------------------------------------------------.
//¥ 					Game driver procedures								.
//¥ --------------------------------------------------------------------.

//¥ Setup a new level. This is called when the game starts and at each new level.
void SetUpLevel (short level)
{
	short i, j;
	SpritePtr sp;
	
	//¥ Clear the Sprite list.
	while (gSAT.sRoot) KillSprite(gSAT.sRoot);
	
	missileCount = 0; 	//¥ global count variable

	//¥ Create all the enemy sprites for the level, depending on the level number.
	for (i = 0; i < (level + 1); i++)
		for (j = 0; j <= (level / 2) + 1; j++)
			sp = NewSprite (-3, i * 40 + 2, j * 40 + 0, &HandleEnemy, &SetupEnemy, 0L);

	//¥ Make the player sprite.
	sp = NewSprite (2, gSAT.offSizeH / 2, gSAT.offSizeV - 40, &HandlePlayer, &SetupPlayer, 0L);

	//¥ Copy backScreen to offScreen to erase old sprites.
	CopyBits (&(gSAT.backScreen->portBits), &(gSAT.offScreen->portBits), &(gSAT.offScreen->portRect), &(gSAT.offScreen->portRect), srcCopy, 0L);
	PeekOffscreen ();
} 	//¥ SetUp Level.

//¥ Start a new game. Initialize level, score, number of lives, and call SetUp Level to make the first level.
void StartGame ()
{
	level = 1;
	SetUpLevel (level);
}

void DoFileMenu (short item)
{
	switch (item)
	{
		case run: 
		{
		//¥ Test if we have Color QD, and if so, test bit depth! Alert if ((**features).PlotFast)
			if (!((gSAT.initDepth == 1) || 
				  (gSAT.initDepth == 4) || 
				  (gSAT.initDepth == 8)) && plotFastFlag)
			{
				ReportStr ("\pPlease uncheck 'Fast animation' or set the monitor to b/w, 4-bit or 8-bit mode in the Control Panel.");
				return;
			}
			if (SATDepthChangeTest()) 	//¥ Update if necessary.
				;
			StartGame ();
			ShowWindow (gSAT.wind);
			SelectWindow (gSAT.wind);
			GameWindUpdate ();
			MoveIt ();
		}
		break;
		
		case sound: 
		{
			soundFlag = ! soundFlag;
			CheckItem (fileMenu, sound, soundFlag);
			if (soundFlag) 	//¥ Tell the sound package our settings, so we don't have to bother.
				SATSoundOn();
			else
				SATSoundOff();
		}
		break;

		case fastAnimation: 
		{
			plotFastFlag = ! plotFastFlag;
			CheckItem (fileMenu, fastAnimation, plotFastFlag);
		}
		break;
		
		case quit: 
			SkelWhoa ();
		break;
	}
}

MoveIt ()
{
	long t;
	EventRecord theEvent; 	//¥ fšr att testa musklick.

	stillRunning = true; 	

	//¥ NOTE: No matter how we leave the MoveIt procedure, we should 
	//¥ ShowCursor.
	HideCursor (); 	

	//¥ Main loop! Keep running until the game is paused or ends.
	while (stillRunning == true)
	{
		t = TickCount ();

		//¥ Here is the real heart of the loop: call Animator once per loop. 
		//¥ It will call all the objects.
		RunSAT (plotFastFlag);

		//¥ All the rest of the main loop is game specific, next level, 
		//¥ bonus handling, etc.
		if (globalSpeed.h == 0)
		{
			downCount--;
			if (downCount <= 0)
			{
				globalSpeed.h = - last_H;
				globalSpeed.v = 0;
				turnFlag = false;
			};
		}
		else 
			if (turnFlag)
			{
				downCount = 10;
				last_H = globalSpeed.h;
				globalSpeed.h = 0;
				globalSpeed.v = 3;
			};
		if (! gSAT.anyMonsters)
		{
			SATSoundShutup  ();
			level++;
			SetUpLevel (level);
		} 	//¥ if not anyMonsters.

		//¥ Check for keys being pressed - but don't allow background 
		//¥ processing.
		//¥ If you want background processing, either use 
		//¥ GetNextEvent+SystemTask or WaitNextEvent (the modern call).
		if (GetOSEvent (keyDownMask, &theEvent)) 	//¥ keydown.
			if ((theEvent.modifiers, cmdKey) != 0)
				switch ((char)(theEvent.message, charCodeMask))
				{
					case 'q': 
					{
						SkelWhoa ();
						//¥ Do all the things we have to do when we 
						//¥ leave MoveIt!.
						SATSoundShutup (); 	//¥ Dispose of sound channel.
						FlushEvents (everyEvent, 0); 	//¥ To forget events, like mouse clicks etc.
						ShowCursor ();
						return;
					}
					break;
					case 's': 
					{
						DoFileMenu (sound);
					}
					break;
					
					default:
					break;
				}; 	//¥ switch.
		
		//¥ Delay, using TickCount so it doesn't matter how fast 
		//¥ our Mac is.
		while ((TickCount () - t) < 3);

	} //while stillRunning; (main loop).

	while (! SATSoundDone() )
		SATSoundEvents (); 	//¥ Wait for last sound to complete.

	ShowCursor ();
	FlushEvents (everyEvent, 0); 	//¥ To forget Events, like mouse clicks etc.

	ReportStr ("\pSorry, game over.");

	SATSoundShutup (); 	//¥ Dispose of sound channel.
} 	//¥ MoveIt.

GameWindUpdate ()
{
	Str255 s;
	Rect r;
	CursHandle watch;

	watch = GetCursor (watchCursor); /* WatchCursor */
	SetCursor (*watch);
	if (SATDepthChangeTest() )
	{
		;
	}
	ReleaseResource ((Handle) watch);
//	InitCursor ();

	 PeekOffscreen ();
}

//¥ Process selection from File menu.

void GameWindIdle ()
{
}

GameWindInit ()
{
	Boolean dummy;
	
	//¥ Tell TransSkel to tell us when to update gSAT.wind.
	dummy = SkelWindow (gSAT.wind, 0L, 0L, &GameWindUpdate, 0L, 0L, 0L, &GameWindIdle, false);

	//¥ Set up the two offScreen GrafPorts "offScreen" and "backScreen". SAT has a standard.
	//¥ way to do this. Let SAT draw the background PICT for us, too.

	//¥ Call the Init routines for all the sprite units!.
	InitEnemy ();
	InitPlayer ();
	InitMissile ();
	InitShot ();

	ShowWindow (gSAT.wind);
	SelectWindow (gSAT.wind);
	//¥ Draw the contents of the window (to give the user something to 
	//¥ look at during the rest of startup).
	 PeekOffscreen ();
}

//¥ --------------------------------------------------------------------.
//¥ 			Menu handling procedures						.
//¥ --------------------------------------------------------------------.

//¥ Handle selection of "AboutÉ" item from Apple menu.

void DoAbout ()
{
	short ignore;

	ignore = Alert (aboutAlrt, 0L);
}

//¥ Initialize menus.  Tell TransSkel to process the Apple menu.
//¥ automatically, and associate the proper procedures with the.
//¥ File menu.

void SetUpMenus ()
{
	SkelApple ("\pAbout SAT InvadersÉ", &DoAbout);
	fileMenu = GetMenu (fileMenuRes);
	SkelMenu (fileMenu, &DoFileMenu, 0L, false, true);
	//¥ Set the following flags so they match the menu.
	soundFlag = true;
	plotFastFlag = true;
}

//¥ Hide gamewindow on suspend, so the user can get access to disk icons etc.

void DoSuspendResume (Boolean b)
{
	if (b)
	{
		ShowWindow (gSAT.wind);
		SelectWindow (gSAT.wind);
	}
	else
		HideWindow (gSAT.wind);
}

Boolean DoEvt (EventRecord e)
{
	if (e.what == osEvt)
	{
		if (((e.message, 8), 0xFF) == suspendResumeMessage)
			DoSuspendResume ((e.message, 1) != 0);
		return true;
	}
	else
		return false;
} 	//¥ end DoEvent *.

//¥ --------------------------------------------------------------------.
//¥ 							Main								.
//¥ --------------------------------------------------------------------.
main ()
{
	SkelInit (0L, 6);				//¥ Initialize.

	//¥ Init all the different parts of the game.
	SetUpMenus ();		//¥ install menu handlers.
	InitSAT (129, 128, 512, 322);

	GameWindInit ();	//¥ Init the game window.
	LoadSounds ();		//¥ preload all sound resources.

	//¥ Set the randseed to something that is random enough.
	randSeed = TickCount ();

	SkelEventHook (&DoEvt); //¥ handle MultiFinder-Events.

	SkelMain ();				//¥ loop 'til Quit selected.
	SkelClobber ();				//¥ clean up.
	SATSoundShutup ();			//¥ Terminate sounds.
}


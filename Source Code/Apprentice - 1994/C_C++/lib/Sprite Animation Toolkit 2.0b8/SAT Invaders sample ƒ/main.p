{================================================}
{=============== SATInvaders main unit ================}
{================================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ © Ingemar Ragnemalm 1992 }
{ See doc files for legal terms for using this code. }

{ SATInvaders is a very simple game demonstrating how to use the Sprite Animation}
{ Toolkit. It is intended as a minimal demonstration, without many features and options}
{ that the other sample program, HeartQuest, has. No high scores or even score, only}
{ one life, doesn't save settings, only one kind of enemy, no special effects like explosions}
{ etc. However, it is still a full Mac application with menus and event handling (using}
{TransSkel). There are now some even more "minimal" demos without that.}

program SATInvaders;

	uses
		TransSkel, SAT, GameGlobals, SoundConst, sPlayer, sEnemy, sShot, sMissile;

	var
		soundFlag, plotFastFlag: Boolean;

{ -------------------------------------------------------------------- }
{								Game driver procedures								}
{ -------------------------------------------------------------------- }

{ Setup a new level. This is called when the game starts and at each new level.}
	procedure SetupLevel (level: integer);
		var
			i, j: integer;
			sp: SpritePtr;
	begin { SetupLevel }

{ Clear the Sprite list! Note that this leaves the images "dead" on the screen,}
{ but we will soon erase them. }
		while gSAT.sRoot <> nil do
			KillSprite(gSAT.sRoot);

		missileCount := 0; { count variable in mMissile }

{ Create all the enemy sprites for the level, depending on the level number. }
		for i := 0 to (level + 1) do
			for j := 0 to (level div 2) + 1 do
				sp := NewSprite(-3, i * 40 + 2, j * 40 + 0, @HandleEnemy, @SetupEnemy, nil);

{ Make the player sprite. }
		sp := NewSprite(2, gSAT.offSizeH div 2, gSAT.offSizeV - 40, @HandlePlayer, @SetupPlayer, nil);

{ Copy BackScreen to OffScreen to erase old sprites. }
		CopyBits(gSAT.backScreen^.portbits, gSAT.offScreen^.portbits, gSAT.offScreen^.portrect, gSAT.offScreen^.portrect, srcCopy, nil);
		PeekOffScreen;
	end; { SetupLevel }

{ Start a new game. Initialize level, score, number of lives, and call setuplevel to make the first level. }
	procedure StartGame;
	begin
		Level := 1;
		SetupLevel(level);
	end;

{ Declare forward since we want to call it from MoveIt }
	procedure DoFileMenu (item: integer);
	forward;

{ This routine is the game driver. It calls RunSAT repeatedly until the game ends or is paused. }
{ I also read the keyboard here. This could optionally be moved to the "player object" module. }

	procedure MoveIt;
		var
			t: longint;
			theEvent: EventRecord; { fšr att testa musklick }
	begin
		stillrunning := true; { A flag that tells whether or not to quit this routine. }

		HideCursor; { NOTE: No matter how we leave the MoveIt procedure, we should ShowCursor. }

{ Main loop! Keep running until the game is paused or ends. }
		while stillrunning = true do
			begin
				t := TickCount; {Remember when we started the last turn through the loop.}

{ Here is the real heart of the loop: call Animator once per loop. It will call all the objects,}
{ draw and erase them, sort them etc. }
				RunSAT(plotFastFlag);

{ All the rest of the main loop is game specific, next level, bonus handling, etc. }

{Handle the speed of the invaders. Since all move the same way, this is done globally.}
				if globalspeed.h = 0 then
					begin
						downcount := pred(downcount);
						if downcount <= 0 then
							begin
								globalspeed.h := -lasth;
								globalspeed.v := 0;
								turnflag := false;
							end;
					end
				else if turnflag then
					begin
						downcount := 10;
						lasth := globalspeed.h;
						globalspeed.h := 0;
						globalspeed.v := 3;
					end;

{End of level? If so, set up a new one!}
				if not gSAT.anyMonsters then
					begin
						SATSoundShutUp;
						level := level + 1;
						SetupLevel(level);
					end; {if not anymonsters}

{ Check for keys being pressed - but don't allow background processing.}
{ If you want background processing, either use GetNextEvent+SystemTask or WaitNextEvent (the modern call).}
				if GetOSEvent(keyDownMask, theEvent) then { keydown only }
					if BitAnd(theEvent.modifiers, cmdKey) <> 0 then {Command key pressed?}
						case char(BitAnd(theEvent.message, charCodeMask)) of {With what key?}
							'q': 
								begin {Quit!}
									SkelWhoa;		{Tell TransSkel to quit.}
{ Do all the things we have to do when we leave MoveIt! }
									SATSoundShutUp; { Dispose of sound channel }
									FlushEvents(EveryEvent, 0); { To forget events, like mouse clicks etc. }
									ShowCursor;
									exit(MoveIt);
								end;
							's': 
								begin {Sound on/off}
									DoFileMenu(sound);
								end;
							otherwise
								; {Ignore others}
						end; { case}

{ Delay, using TickCount so it doesn't matter how fast our Mac is. }
				while ((TickCount - t) < 3) do {3/60 per frame = 20 fps if possible}
					;
			end; { while stillrunning (main loop) }

		while not SATSoundDone do
			SATSoundEvents; {Wait for last sound to complete}

		ShowCursor; {Balance HideCursor}
		FlushEvents(EveryEvent - DiskMask, 0); { To forget events, like mouse clicks etc. except disk events }

		ReportStr('Sorry, game over.');

		SATSoundShutUp; { Dispose of sound channel }
	end; { MoveIt }

	procedure GameWindUpdate;
		var
			watch: CursHandle;
	begin
{Set the cursor to wait cursor during screen depth change test. If there's no change,}
{the user won't notice.}
		watch := GetCursor(WatchCursor);
		SetCursor(watch^^);
		if SATDepthChangeTest then
			begin
{Do anything needed after a screen depth change. In this demo, nothing.}
			end;
		ReleaseResource(Handle(watch));
{Set the cursor to arrow again.}
		InitCursor;

{Process the update event by redrawing the window.}
		PeekOffScreen;

{Note: PeekOffScreen can be replaced by drawing with CopyBits, i.e.:}
{SATSetPortScreen;}
{CopyBits(offScreen^.portBits, SATwind^.portBits, offScreen^.portRect, offScreen^.portRect, srcCopy, nil);}
{plus drawing borders. PeekOffScreen draws them black.}
	end;

{	Process selection from File menu.}

	procedure DoFileMenu (item: integer);
	begin
		case item of
			run: 
				begin
{ Test if we have Color QD, and if so, test bit depth! Alert if features^^.PlotFast.}
					if not ((gSAT.initDepth = 1) or (gSAT.initDepth = 4) or (gSAT.initDepth = 8)) and plotFastFlag then
						begin
							ReportStr('Please uncheck ''Fast animation'' or set the monitor to b/w, 4-bit or 8-bit mode in the Control Panel.');
							exit(DoFileMenu);
						end;
					if SATDepthChangeTest then {Update if necessary}
						;
					StartGame;
					ShowWindow(gSAT.wind);
					SelectWindow(gSAT.wind);
					GameWindUpdate;
					MoveIt;
				end;
			sound: 
				begin
					soundFlag := not soundFlag;
					CheckItem(FileMenu, sound, soundFlag);
					if soundFlag then { Tell the sound package our settings, so we don't have to bother. }
						SATSoundOn
					else
						SATSoundOff;
				end;
			fastAnimation: 
				begin
					plotFastFlag := not plotFastFlag;
					CheckItem(fileMenu, fastAnimation, plotFastFlag);
				end;
			quit: 
				SkelWhoa;
		end;
	end;

	procedure GameWindInit;
	begin
{ Tell TransSkel to tell us when to update SATwind. }
		if SkelWindow(gSAT.wind, nil, nil, @GameWindUpdate, nil, nil, nil, nil, false) then
			;

		ShowWindow(gSAT.wind);
		SelectWindow(gSAT.wind);
{ Draw the contents of the window (to give the user something to look at during the rest of startup). }
		PeekOffScreen;
	end;

{ -------------------------------------------------------------------- }
{						Menu handling procedures						}
{ -------------------------------------------------------------------- }

{	Handle selection of "AboutÉ" item from Apple menu}

	procedure DoAbout;
	begin
		if Alert(aboutAlrt, nil) = 1 then
			;
	end;

{	Initialize menus.  Tell TransSkel to process the Apple menu}
{	automatically, and associate the proper procedures with the}
{	File menu.}

	procedure SetUpMenus;
	begin
		SkelApple('About SAT InvadersÉ', @DoAbout);
		fileMenu := GetMenu(fileMenuRes);
		if SkelMenu(fileMenu, @DoFileMenu, nil, true) then
			;
{ Set the following flags so they match the menu }
		soundFlag := true;
		plotFastFlag := true;
	end;

{ Hide gamewindow on suspend, so the user can get access to disk icons etc. }

	procedure DoSuspendResume (b: boolean);
	begin
		if b then
			begin
				ShowWindow(gSAT.wind);
				SelectWindow(gSAT.wind);
			end
		else
			HideWindow(gSAT.wind)
	end;

	function DoEvt (e: eventRecord): boolean;
	begin
		if e.what = OSevt then
			begin
				if BAND(BROTL(e.message, 8), $FF) = SuspendResumeMessage then
					DoSuspendResume(BAnd(e.message, 1) <> 0);
				DoEvt := true;
			end
		else
			DoEvt := false;
	end; (* end DoEvent *)

{ -------------------------------------------------------------------- }
{									Main								}
{ -------------------------------------------------------------------- }

begin
	SkelInit(6, nil);				{ initialize }

{ Init all the different parts of the game. }

	SetUpMenus;						{ install menu handlers }
	InitSAT(129, 128, 512, 322);	{PICTs 129 and 128, width 512, height 322.}
	GameWindInit;	{ Install the game window (SATwind) in TransSkel and show it. }
	Loadsounds;		{ Preload all sound resources }

{ Call the init routines for all the sprite units (generally to preload faces)!}
{ This must be done after InitSAT! }
	InitEnemy;
	InitPlayer;
	InitMissile;
	InitShot;

	randSeed := TickCount;	{ Set the randseed to something that is random enough. }

	SkelEventHook(@DoEvt); { Handle MultiFinder-events }

	SkelMain;					{ Loop 'til Quit selected }
	SkelClobber;				{ Clean up }
	SATSoundShutUp;			{ Terminate sounds, free the sound channel. }
end.
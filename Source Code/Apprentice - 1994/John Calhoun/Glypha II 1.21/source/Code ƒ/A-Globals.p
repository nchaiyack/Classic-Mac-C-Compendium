unit Globals;

interface

	uses
		Palettes, Sound;

	const
		stackSize = 32768;

		kSpaceKey = $31;
		kLeftKey1 = $29;
		kLeftKey2 = $00;
		kRightKey1 = $27;
		kRightKey2 = $01;

		sleep = 10;
		suspendResumeBit = $0001;
		resuming = 1;
		WNETrapNum = $60;
		unimplTrapNum = $9F;

		kRightFace = 0;
		kLeftFace = 1;

		kEnemyModeRange = 30;
		kEnemyModeMinumum = 30;
		kInitEnemyModeRange = 50;
		kInitEnemyModeMinumum = 50;
		kEggDelayRange = 500;
		kEggLevelPenalty = 5;
		kSpaceBar = $20;

		noSound = 0;
		lowPriority = 1;
		highPriority = 2;

		liftAmount = -7;	{originally -8}
		fallAmount = 1;
		maxFall = 8;			{originally 10}
		maxThrust = 16;
		defaultNum = 5;
		maxEnemies = 4;
		newMortalPts = 10000;

		upperEyeHeight = 100;
		lowerEyeHeight = 200;
		handTop = 380;
		handBottom = 470;
		handLeft = 433;

		upperLevel = 0;
		lowerLevel = 1;
		upperEye = 2;
		lowerEye = 3;

		fast = 3;
		slow = 4;

		backPictID = 130;
		objectPictID = 131;
		maskPictID = 132;
		helpPictID = 133;

		alertStrIDs = 128;
		yesNoStrIDs = 129;

		alertID = 128;
		controlDialID = 129;
		hiNameDialID = 130;
		aboutDialID = 131;
		yesNoAlertID = 132;
		configGameDialID = 133;

		mainWndoID = 128;

		mApple = 128;				{Menu list}
		iAbout = 1;
		mGame = 129;					{Menu list}
		iBegin = 1;
		iPause = 2;
		iEnd = 3;
		iQuit = 5;
		mOptions = 130;			{Menu list}
		iConfigure_Game = 1;
		iConfigure_Controls = 2;
		iHelp = 4;
		iClear_HiScores = 5;

	type
		GameObject = record
				oldDest, dest, wholeRect: Rect;
				horiVel, vertVel: Integer;
				facing: Integer;
				mode, otherMode, tempInt: Integer;
				state, otherState: Boolean;
			end;

		Bolt = record
				levelStriking: Integer;
				leftMode, rightMode: Integer;
				leftBolts, rightBolts: array[0..3, 0..3, 0..11] of Point;
			end;

		BitMapPtr = ^BitMap;

	var
		{Here follow all the major global variables							}
		{(yes, there are quite a few)														}
		{Here are then menu handles															}

		theEvent: EventRecord;

		{Integers keep track of numbers, level, modes, etc...		}
		rightOffset, downOffset: Integer;
		numberOfStones, levelOn, mortals, oldDirection: Integer;
		beastsKilled, beastsActive, totalToKill, levelStart, mortalsStart: Integer;
		gameSpeed, startStone, growRate, numberOfEnemies: Integer;

		{Score is LongInt for the 12 yr old kids that top	36727	}
		gameCycle, score, oldScore, nextMortal: LongInt;

		{Keep track of the name last entered for the high score	}
		nameUsing: Str255;

		{Here follow the offscreen bitmap and window variables	}
		mainWndo: WindowPtr;
		mainPalette: PaletteHandle;
		playRgn, obeliskRgn1, obeliskRgn2: RgnHandle;
		smallOffArea, wholeArea: Rect;
		offMaskMap: BitMap;
		offMaskPort: GrafPtr;
		offMaskBits: Ptr;
		virginCPort, loadCPort, objectCPort: CGrafPort;
		virginCPtr, loadCPtr, objectCPtr: CGrafPtr;
		virginCBits, loadCBits, objectCBits: Ptr;
		rgbBlack, rgbWhite, rgbLtBlue, rgbYellow, rgbRed: RGBColor;

		{Here are some of the object definition records					}
		theEye, theHand, thePlayer: GameObject;
		lightning: Bolt;

		{Miscellaneous rects for the score box, platforms, etc..}
		playRect, shortStoneSrc, longStoneSrc: Rect;

		{For the digitized sound																}
		chanPtr: sndchannelptr;
		soundPriority: Integer;

		{Arrays hold hiscores made and read from .RSRC fork			}
		hiScores: array[1..10] of LongInt;
		hiStrings: array[1..10] of Str255;

		{These vars hold info on the enemies										}
		theEnemies: array[0..9] of GameObject;
		enemyLift: array[0..2] of Integer;

		{These vars hold locations of all artwork on offscreens	}
		tombRects: array[-5..6] of Rect;
		playerRects: array[0..1, 0..5] of Rect;

		enemyRects: array[0..2, 0..5, 0..1] of Rect;
		boneRects: array[0..1, 6..7] of Rect;

		absoluteRects: array[0..1, 0..5] of Rect;
		eyeRects: array[0..4] of Rect;

		handRects: array[0..1] of Rect;
		ankRects: array[0..15] of Rect;

		eggRects, gameoverRects: Rect;
		flameRect: array[0..1] of Rect;
		running: array[-16..16, 0..1, 0..1] of Integer;
		idleLanded: array[-16..16] of Integer;
		gliding: array[-16..16, 0..1] of Integer;
		impacted: array[-70..16] of Integer;

{The boolean vars often keep track of the game state		}

		playing, pausing, keyboardControl, soundOn, inBackground: Boolean;
		deadAndGone, scoresChanged, inhibitSound, doneFlag: Boolean;
		onward, keyStillDown, stonesSliding, hasWNE: Boolean;

	procedure DoErrorSound (soundNumber: Integer);
	procedure GenericAlert (whatGives: Integer);

implementation

{=================================}

	procedure DoErrorSound;
		var
			dummyLong: LongInt;
			tempVolume, i: Integer;
	begin
		GetSoundVol(tempVolume);
		if (tempVolume <> 0) then
			for i := 0 to soundNumber do
				begin
					FlashMenuBar(0);
					Delay(8, dummyLong);
					FlashMenuBar(0);
				end;
	end;

{=================================}

	procedure GenericAlert;
		var
			dummyInt: Integer;
			line1, line2: Str255;
			alertHandle: AlertTHndl;
			alertRect: Rect;
	begin
		InitCursor;
		if (whatGives > 0) then
			begin
				GetIndString(line1, alertStrIDs, whatGives);
				line2 := '';
			end
		else
			begin
				GetIndString(line1, alertStrIDs, 1);
				NumToString(whatGives, line2);
				line2 := CONCAT('Error = ', line2);
			end;

		ParamText(line1, line2, '', '');
		alertHandle := AlertTHndl(Get1Resource('ALRT', alertID));
		if (alertHandle <> nil) then
			begin
				HNoPurge(Handle(alertHandle));
				alertRect := alertHandle^^.boundsRect;
				OffsetRect(alertRect, -alertRect.left, -alertRect.top);
				dummyInt := (screenBits.bounds.right - alertRect.right) div 2;
				OffsetRect(alertRect, dummyInt, 0);
				dummyInt := (screenBits.bounds.bottom - alertRect.bottom) div 3;
				OffsetRect(alertRect, 0, dummyInt);
				alertHandle^^.boundsRect := alertRect;
				HPurge(Handle(alertHandle));
			end;
		dummyInt := Alert(alertID, nil);
	end;

{=================================}

end.
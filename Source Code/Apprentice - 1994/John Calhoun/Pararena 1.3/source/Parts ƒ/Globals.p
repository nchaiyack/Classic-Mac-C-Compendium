unit Globals;

interface
	uses
		Sound;

	const
		stackSize = 24000;
		sleep = 2;
		suspendResumeBit = $0001;
		resuming = 1;

		kPauseKey = $30;

		mainWndoID = 128;
		backPictID = 2001;
		yesNoAlertID = 128;
		aboutDialID = 129;
		unlockDialID = 130;
		hiscoreDialID = 131;
		helpPlayDialID = 132;
		helpRulesDialID = 133;
		speedDialID = 134;
		soundDialID = 135;
		physicsDialID = 136;

		mApple = 128;
		iNothing = 0;
		iAbout = 1;

		mGame = 129;
		iBegin = 1;
		iEnd = 2;
		iWhichGame = 4;
		iWhichOpponent = 5;
		iQuit = 7;

		mWhichGame = 130;
		iPractice = 1;
		iFourofFive = 3;
		iFourOfNine = 4;
		iFirstTo13 = 5;
		iDeltaFive = 6;

		mOpponents = 131;
		iMara = 1;
		iOtto = 2;
		iGeorge = 3;
		iClaire = 4;

		mOptions = 132;
		iVisCurs = 1;
		iAutoPickUp = 2;
		iSound = 3;
		iSpeed = 4;
		iPhysics = 5;
		iHelpRules = 7;
		iHelpOther = 8;
		iHiScores = 10;

		noSound = 0;
		lowPriority = 1;
		highPriority = 2;

		south = 0;
		southEast = 1;
		east = 2;
		northEast = 3;
		north = 4;
		northWest = 5;
		west = 6;
		southWest = 7;
		resting = 8;
		notCarrying = 0;
		carrying = 1;
		crouching = 2;

		noone = 0;					{Opponents}
		mara = 1;
		otto = 2;
		george = 3;
		claire = 4;

		practiceSkating = 0;		{Game in session}
		practiceWBall = 1;
		fourOfFive = 2;
		fourOfNine = 3;
		firstToThirteen = 4;
		deltaFive = 5;

		ballNotInArena = 0;		{Ball states}
		nooneHasBall = 1;
		playerHasBall = 2;
		opponentHasBall = 3;

		noFriction = 0;
		weakFriction = 1;
		normalFriction = 2;
		strongFriction = 3;

		centerH = 256;
		centerV = 200;

	type
		dynaMap = record
				dest: Rect;
				oldDest: Rect;
				src: Rect;
				mask: Rect;
				posX: LongInt;
				posZ: LongInt;
				velX: LongInt;
				velZ: LongInt;
				screenH: Integer;
				screenV: Integer;
				mode: Integer;
				carryMode: Integer;
				timeKeeper: LongInt;
			end;

		vertHandle = ^vertPtr;
		vertPtr = ^vert;
		vert = record
				data: array[0..80, -80..80] of Integer;
			end;

		savedScores = record
				greatNames: array[0..11] of Str255;
				deltaNames: array[0..11] of Str255;
				greatScores: array[0..11, 0..1] of Integer;
				deltaScores: array[0..11, 0..1] of Integer;
			end;

		forceHandle = ^forcePtr;
		forcePtr = ^force;
		force = record
				data: array[-24..24, -24..24, 0..1] of Integer;
			end;

	var
		theEvent: EventRecord;

		mainWndo: WindowPtr;
		screenArea, wholeScreen: Rect;
		offVirginMap, offLoadMap, offPlayerMap, offEnemyMap: BitMap;
		offVirginPort, offLoadPort, offPlayerPort, offEnemyPort: GrafPtr;
		offVirginBits, offLoadBits, offPlayerBits, offEnemyBits: Ptr;

		earthPoints, taygetePoints, numerator, denominator, wasMBarHeight, timeToLock: Integer;
		earthFouls, taygeteFouls, lastTouchingBall, frictNum, frictDen, frictState: Integer;
		whichOpponent, whichGame, whichPeriod, mouseConst, delayTime, numberOfStars: Integer;
		smallestDelta, smallestGreatest, rightOffset, downOffset, tileLit, tileToggle: Integer;
		massNumbers: array[0..4, 0..1] of Integer;
		stars: array[0..44, 0..1] of Integer;
		modeOpposites: array[0..20] of Integer;
		nontantTable: array[0..8] of Integer;
		forceTable: array[-24..24, -24..24, 0..1] of Integer;
		mouseForce: array[0..8, 0..1] of Integer;

		periodTime, startTime, currentTime, lastLoopTime: LongInt;

		prefsStr, playerName: Str255;
		ballCatches: array[0..5] of Str255;

		chanPtr: sndChannelPtr;
		soundPriority: Integer;
		theSnd: Handle;

		foulSrc: Rect;
		player, opponent, ball: dynaMap;
		vertTable: vertHandle;
		theForces: forceHandle;
		ballVisRgn, wholeRgn: RgnHandle;
		hiScores: savedScores;

		cursorArray: array[0..8] of CursHandle;
		titleLetters: array[0..7] of dynaMap;
		tileRgns: array[0..5, 0..1] of RgnHandle;
		foulDest: array[0..1, 1..3] of Rect;
		periodDest: array[1..4] of Rect;
		titleSrc, titleMask: array[0..7] of Rect;
		digitSrc: array[0..16] of Rect;
		timeDest, earthScoreDest, taygeteScoreDest: array[1..3] of Rect;
		jetsRects: array[0..2] of Rect;
		teleportMask: array[0..3] of Rect;
		playerSrc, playerMask, opponentSrc: array[0..8, 0..2] of Rect;

		soundOn, playing, pausing, clicked, jetsOut, opponentWants, mad, autoPickUp: Boolean;
		ballJustHit, locked, inhibitSound, lastButton, doneFlag, hasWNE, inColor: Boolean;
		bothJustHit, opponentDropping, cursorVis, scoresChanged, justQuit, inBackground: Boolean;
		soundArray: array[1..5] of Boolean;

	procedure CenterDialog (dialID: Integer);
	procedure FatalError;

{=================================}

implementation

{=================================}

	procedure CenterDialog;
		var
			dialHandle: DialogTHndl;
			dialRect: Rect;
			dummyInt: Integer;
	begin
		dialHandle := DialogTHndl(Get1Resource('DLOG', dialID));
		if dialHandle <> nil then
			begin
				HNoPurge(Handle(dialHandle));
				dialRect := dialHandle^^.boundsRect;
				OffsetRect(dialRect, -dialRect.left, -dialRect.top);
				dummyInt := (screenBits.bounds.right - dialRect.right) div 2;
				OffsetRect(dialRect, dummyInt, 0);
				dummyInt := (screenBits.bounds.bottom - dialRect.bottom + 20) div 2;
				OffsetRect(dialRect, 0, dummyInt);
				dialHandle^^.boundsRect := dialRect;
				HPurge(Handle(dialHandle));
			end;
	end;

{=================================}

	procedure FatalError;
	begin
		ExitToShell;
	end;

{=================================}

end.
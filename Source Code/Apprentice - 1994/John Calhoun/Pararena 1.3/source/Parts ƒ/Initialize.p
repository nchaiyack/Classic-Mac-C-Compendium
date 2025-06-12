unit Initialize;

interface

	uses
		Sound, Globals, Unlock, Utilities, Guts;

	const
		objectPictID = 2000;
		objectPictID2 = 2003;

	procedure InitVariables;

implementation

{=================================}

	procedure InitVariables;
		var
			index: Integer;
			tempRect: Rect;
			tempStr: Str255;

{-----------------------}

		procedure CheckOurEnvirons;
			const
				WNETrapNum = $60;
				unimplTrapNum = $9F;
			var
				err: OSErr;
				thisWorld: SysEnvRec;
		begin
			rightOffset := (ScreenBits.bounds.right - 512) div 2;
			downOffset := (ScreenBits.bounds.bottom - 342) div 2;

			err := SysEnvirons(1, thisWorld);
			with thisWorld do
				begin
					if ((machineType < env512KE) and (machineType <> 0)) then
						FatalError;
					inhibitSound := (systemVersion < $0602);

					inColor := HasColorQD;
				end;

			hasWNE := (NGetTrapAddress(WNETrapNum, ToolTrap) <> NGetTrapAddress(unimplTrapNum, toolTrap));
		end;

{-----------------------}

		procedure GetAllSounds;
			var
				i, howManySounds: Integer;
		begin
			howManySounds := Count1Resources('snd ');		{Get number of sounds}

			for i := 1 to howManySounds do
				begin
					theSnd := Get1IndResource('snd ', i);
					if (theSnd^ <> nil) then
						begin
							MoveHHi(theSnd);
							HLock(theSnd);
						end
					else
						Exit(GetAllSounds);
				end;
		end;

{-----------------------}

		procedure SetUpMenus;                {Initialize the menus}
			var
				theMenu: MenuHandle;
		begin
			ClearMenuBar;				{Clear any old menu bars}
			theMenu := GetMenu(mApple);
			AddResMenu(theMenu, 'DRVR');
			InsertMenu(theMenu, 0);

			theMenu := GetMenu(mGame);
			InsertMenu(theMenu, 0);

			theMenu := GetMenu(mWhichGame);
			InsertMenu(theMenu, -1);

			theMenu := GetMenu(mOpponents);
			InsertMenu(theMenu, -1);

			theMenu := GetMenu(mOptions);
			InsertMenu(theMenu, 0);

			theMenu := NewMenu(133, '(status = LOCKED)');
			InsertMenu(theMenu, 0);

			DisableItem(GetMenu(mGame), iEnd);
			CheckItem(GetMenu(mWhichGame), iDeltaFive, TRUE);
			CheckItem(GetMenu(mOpponents), iGeorge, TRUE);
			CheckItem(GetMenu(mOptions), iVisCurs, cursorVis);
			CheckItem(GetMenu(mOptions), iAutoPickUp, autoPickUp);

			if (inhibitSound) then
				begin
					SetItem(GetMenu(mOptions), iSound, 'Sound need Sys. 6.02 or >');
					DisableItem(GetMenu(mOptions), iSound);
				end;

			if (locked) then
				begin
					DisableItem(GetMenu(mWhichGame), iFourOfNine);
					DisableItem(GetMenu(mWhichGame), iFirstTo13);
					DisableItem(GetMenu(mOpponents), iMara);
					DisableItem(GetMenu(mOpponents), iClaire);
					DisableItem(GetMenu(mOptions), iPhysics);
				end
			else
				begin
					DeleteMenu(133);
				end;

			HideMenuBar;
			ShowMenuBar;

			DrawMenuBar;
		end;

{-----------------------}

		function NewBitMap (var theBitMap: BitMap; theRect: Rect): Ptr;
		begin
			with theBitMap, theRect do
				begin
					rowBytes := ((right - left + 15) div 16) * 2;
					baseAddr := NewPtr(rowBytes * (bottom - top));
					bounds := theRect;
					if (MemError <> noErr) then
						FatalError
					else
						NewBitMap := baseAddr;
				end;
		end;

{-----------------------}

		procedure InitForceTable;
			var
				indexX, indexZ: Integer;
		begin
			vertTable := vertHandle(NewHandle(SIZEOF(vert)));

			Handle(vertTable) := GetResource('vert', 2000);
			if (vertTable = nil) then
				FatalError;

			theForces := forceHandle(NewHandle(SIZEOF(force)));
			Handle(theForces) := GetResource('forc', 2000);
			if (theForces = nil) then
				FatalError;
			MoveHHi(Handle(theForces));
			HLock(Handle(theForces));
			with theForces^^ do
				for indexX := -24 to 24 do
					for indexZ := -24 to 24 do
						begin
							forceTable[indexX, indexZ, 0] := data[indexX, indexZ, 0];
							forceTable[indexX, indexZ, 1] := data[indexX, indexZ, 1];
						end;
			ReleaseResource(Handle(theForces));
			HUnlock(Handle(theForces));
			DisposHandle(Handle(theForces));
		end;

{-----------------------}

		procedure CreateRegions;
		begin
			wholeRgn := NewRgn;
			SetRectRgn(wholeRgn, 0, 0, 512, 342);

			tileRgns[0, 0] := NewRgn;
			MoveTo(219, 213);
			OpenRgn;
			LineTo(183, 198);
			LineTo(199, 185);
			LineTo(226, 175);
			LineTo(240, 200);
			LineTo(226, 205);
			LineTo(219, 213);
			CloseRgn(tileRgns[0, 0]);

			tileRgns[1, 0] := NewRgn;
			MoveTo(219, 213);
			OpenRgn;
			LineTo(183, 198);
			LineTo(147, 183);
			LineTo(172, 161);
			LineTo(211, 148);
			LineTo(226, 175);
			LineTo(240, 200);
			LineTo(226, 205);
			LineTo(219, 213);
			CloseRgn(tileRgns[1, 0]);

			tileRgns[2, 0] := NewRgn;
			MoveTo(183, 198);
			OpenRgn;
			LineTo(147, 183);
			LineTo(109, 163);
			LineTo(144, 135);
			LineTo(195, 116);
			LineTo(211, 148);
			LineTo(226, 175);
			LineTo(199, 185);
			LineTo(183, 198);
			CloseRgn(tileRgns[2, 0]);

			tileRgns[3, 0] := NewRgn;
			MoveTo(147, 183);
			OpenRgn;
			LineTo(109, 163);
			LineTo(74, 142);
			LineTo(118, 104);
			LineTo(180, 81);
			LineTo(195, 116);
			LineTo(211, 148);
			LineTo(172, 161);
			LineTo(147, 183);
			CloseRgn(tileRgns[3, 0]);

			tileRgns[4, 0] := NewRgn;
			MoveTo(109, 163);
			OpenRgn;
			LineTo(74, 142);
			LineTo(35, 115);
			LineTo(86, 72);
			LineTo(165, 44);
			LineTo(180, 81);
			LineTo(195, 116);
			LineTo(144, 135);
			LineTo(109, 163);
			CloseRgn(tileRgns[4, 0]);

			tileRgns[5, 0] := NewRgn;
			MoveTo(74, 142);
			OpenRgn;
			LineTo(35, 115);
			LineTo(86, 72);
			LineTo(165, 44);
			LineTo(180, 81);
			LineTo(118, 104);
			LineTo(74, 142);
			CloseRgn(tileRgns[5, 0]);

			tileRgns[0, 1] := NewRgn;
			MoveTo(512 - 219, 213);
			OpenRgn;
			LineTo(512 - 183, 198);
			LineTo(512 - 199, 185);
			LineTo(512 - 226, 175);
			LineTo(512 - 240, 200);
			LineTo(512 - 226, 205);
			LineTo(512 - 219, 213);
			CloseRgn(tileRgns[0, 1]);

			tileRgns[1, 1] := NewRgn;
			MoveTo(512 - 219, 213);
			OpenRgn;
			LineTo(512 - 183, 198);
			LineTo(512 - 147, 183);
			LineTo(512 - 172, 161);
			LineTo(512 - 211, 148);
			LineTo(512 - 226, 175);
			LineTo(512 - 240, 200);
			LineTo(512 - 226, 205);
			LineTo(512 - 219, 213);
			CloseRgn(tileRgns[1, 1]);

			tileRgns[2, 1] := NewRgn;
			MoveTo(512 - 183, 198);
			OpenRgn;
			LineTo(512 - 147, 183);
			LineTo(512 - 109, 163);
			LineTo(512 - 144, 135);
			LineTo(512 - 195, 116);
			LineTo(512 - 211, 148);
			LineTo(512 - 226, 175);
			LineTo(512 - 199, 185);
			LineTo(512 - 183, 198);
			CloseRgn(tileRgns[2, 1]);

			tileRgns[3, 1] := NewRgn;
			MoveTo(512 - 147, 183);
			OpenRgn;
			LineTo(512 - 109, 163);
			LineTo(512 - 74, 142);
			LineTo(512 - 118, 104);
			LineTo(512 - 180, 81);
			LineTo(512 - 195, 116);
			LineTo(512 - 211, 148);
			LineTo(512 - 172, 161);
			LineTo(512 - 147, 183);
			CloseRgn(tileRgns[3, 1]);

			tileRgns[4, 1] := NewRgn;
			MoveTo(512 - 109, 163);
			OpenRgn;
			LineTo(512 - 74, 142);
			LineTo(512 - 35, 115);
			LineTo(512 - 86, 72);
			LineTo(512 - 165, 44);
			LineTo(512 - 180, 81);
			LineTo(512 - 195, 116);
			LineTo(512 - 144, 135);
			LineTo(512 - 109, 163);
			CloseRgn(tileRgns[4, 1]);

			tileRgns[5, 1] := NewRgn;
			MoveTo(512 - 74, 142);
			OpenRgn;
			LineTo(512 - 35, 115);
			LineTo(512 - 86, 72);
			LineTo(512 - 165, 44);
			LineTo(512 - 180, 81);
			LineTo(512 - 118, 104);
			LineTo(512 - 74, 142);
			CloseRgn(tileRgns[5, 1]);

			ballVisRgn := NewRgn;
			MoveTo(0, 342);
			OpenRgn;
			LineTo(0, 82);
			LineTo(146, 0);
			LineTo(365, 0);
			LineTo(512, 82);
			LineTo(512, 342);
			LineTo(0, 342);
			CloseRgn(ballVisRgn);
		end;

{-----------------------}

		procedure ReadyBitmaps;
			var
				rawPointer: Ptr;
				thePict: PicHandle;
				tempRect: Rect;
		begin
			SetRect(wholeScreen, 0, 0, 512, 342);
			SetRect(screenArea, 0, 0, 512, 322);

			rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen virgin map}
			offVirginPort := GrafPtr(rawPointer);
			OpenPort(offVirginPort);
			offVirginBits := NewBitMap(offVirginMap, wholeScreen);
			SetPortBits(offVirginMap);
			EraseRect(offVirginMap.bounds);
			SetPort(offVirginPort);
			ClipRect(wholeScreen);

			rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
			offLoadPort := GrafPtr(rawPointer);
			OpenPort(offLoadPort);
			offLoadBits := NewBitMap(offLoadMap, wholeScreen);
			SetPortBits(offLoadMap);
			EraseRect(offLoadMap.bounds);
			SetPort(offLoadPort);
			ClipRect(wholeScreen);

			rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
			offPlayerPort := GrafPtr(rawPointer);
			OpenPort(offPlayerPort);
			offPlayerBits := NewBitMap(offPlayerMap, screenArea);
			SetPortBits(offPlayerMap);
			EraseRect(offPlayerMap.bounds);
			SetPort(offPlayerPort);
			thePict := GetPicture(objectPictID);
			if (thePict <> nil) then
				begin
					HLock(Handle(thePict));
					tempRect := thePict^^.picFrame;
					DrawPicture(thePict, tempRect); 	{Draw this picture}
					HUnlock(Handle(thePict));
					ReleaseResource(Handle(thePict));
				end
			else
				FatalError;
			ClipRect(tempRect);

			rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
			offEnemyPort := GrafPtr(rawPointer);
			OpenPort(offEnemyPort);
			offEnemyBits := NewBitMap(offEnemyMap, screenArea);
			SetPortBits(offEnemyMap);
			EraseRect(offEnemyMap.bounds);
			SetPort(offEnemyPort);
			thePict := GetPicture(objectPictID2);
			if (thePict <> nil) then
				begin
					HLock(Handle(thePict));
					tempRect := thePict^^.picFrame;
					DrawPicture(thePict, tempRect); 	{Draw this picture}
					HUnlock(Handle(thePict));
					ReleaseResource(Handle(thePict));
				end
			else
				FatalError;
			ClipRect(tempRect);
		end;

{-----------------------}

		function PrepareSounds: Boolean;
			var
				howManySounds, i: Integer;
		begin
			PrepareSounds := FALSE;

			chanPtr := nil;
			soundPriority := noSound;

			howManySounds := Count1Resources('snd ');		{Get number of sounds}

			for i := 1 to howManySounds do
				begin
					theSnd := Get1IndResource('snd ', i);
					if (theSnd <> nil) then
						begin
							MoveHHi(theSnd);
							HLock(theSnd);
						end
					else
						Exit(PrepareSounds);
				end;
			PrepareSounds := TRUE;
		end;

{-----------------------}

		procedure ReadInScores;
			type
				scoreHandle = ^scorePtr;
				scorePtr = ^score;
				score = record
						data: array[0..47] of Integer;
					end;

				nameHandle = ^namePtr;
				namePtr = ^name;
				name = record
						data: array[0..11, 0..14] of Char;
					end;

				gamePrefs = record
						isLocked, isSoundOn: Boolean;
						isSoundArray: array[1..5] of Boolean;
						whenToLock, isDelayTime: Integer;
					end;
				gamePrefsPtr = ^gamePrefs;
				gamePrefsHand = ^gamePrefsPtr;

				prefHandle = ^prefPtr;
				prefPtr = ^pref;
				pref = record
						data: array[0..24] of Char;
					end;

			var
				index, index2, i: Integer;
				dummyStr: Str255;
				theScores: scoreHandle;
				theNamesGreat, theNamesDelta: nameHandle;
				thePrefs: prefHandle;
				theGamePrefs: gamePrefsHand;
		begin
			theScores := scoreHandle(NewHandle(SIZEOF(score)));
			Handle(theScores) := GetResource('scrs', 128);
			MoveHHi(Handle(theScores));
			HLock(Handle(theScores));
			with hiScores do
				for index := 0 to 11 do
					begin
						greatScores[index, 0] := theScores^^.data[index];
						greatScores[index, 1] := theScores^^.data[index + 12];
						deltaScores[index, 0] := theScores^^.data[index + 24];
						deltaScores[index, 1] := theScores^^.data[index + 36];
					end;
			HUnlock(Handle(theScores));
			ReleaseResource(Handle(theScores));
			DisposHandle(Handle(theScores));

			theNamesGreat := nameHandle(NewHandle(SIZEOF(name)));
			Handle(theNamesGreat) := GetResource('name', 128);
			MoveHHi(Handle(theNamesGreat));
			HLock(Handle(theNamesGreat));
			with hiScores do
				for index := 0 to 11 do
					begin
						dummyStr := '';
						for index2 := 0 to 14 do
							begin
								dummyStr := CONCAT(dummyStr, theNamesGreat^^.data[index, index2]);
							end;
						greatNames[index] := dummyStr;
					end;
			HUnlock(Handle(theNamesGreat));
			ReleaseResource(Handle(theNamesGreat));
			DisposHandle(Handle(theNamesGreat));

			theNamesDelta := nameHandle(NewHandle(SIZEOF(name)));
			Handle(theNamesDelta) := GetResource('name', 129);
			MoveHHi(Handle(theNamesDelta));
			HLock(Handle(theNamesDelta));
			with hiScores do
				for index := 0 to 11 do
					begin
						dummyStr := '';
						for index2 := 0 to 14 do
							begin
								dummyStr := CONCAT(dummyStr, theNamesDelta^^.data[index, index2]);
							end;
						deltaNames[index] := dummyStr;
					end;
			HUnlock(Handle(theNamesDelta));
			ReleaseResource(Handle(theNamesDelta));
			DisposHandle(Handle(theNamesDelta));

			thePrefs := prefHandle(NewHandle(SIZEOF(pref)));
			Handle(thePrefs) := GetResource('pref', 128);
			MoveHHi(Handle(thePrefs));
			HLock(Handle(thePrefs));
			dummyStr := '';
			for index := 0 to 24 do
				begin
					dummyStr := CONCAT(dummyStr, thePrefs^^.data[index]);
				end;
			prefsStr := dummyStr;
			HUnlock(Handle(thePrefs));
			ReleaseResource(Handle(thePrefs));
			DisposHandle(Handle(thePrefs));

			theGamePrefs := gamePrefsHand(NewHandle(SIZEOF(gamePrefs)));
			Handle(theGamePrefs) := GetResource('pref', 129);
			MoveHHi(Handle(theGamePrefs));
			HLock(Handle(theGamePrefs));
			with theGamePrefs^^ do
				begin
					soundOn := isSoundOn;
					if (inhibitSound) then
						soundOn := FALSE;

					for i := 1 to 5 do
						soundArray[i] := isSoundArray[i];

					locked := isLocked;
					timeToLock := whenToLock;
					if (timeToLock < 2) then
						begin
							locked := TRUE;
							timeToLock := 1;
						end
					else
						timeToLock := timeToLock - 1;

					delayTime := isDelayTime;
				end;
			HUnlock(Handle(theGamePrefs));
			ReleaseResource(Handle(theGamePrefs));
			DisposHandle(Handle(theGamePrefs));

			smallestDelta := hiScores.deltaScores[11, 0] - hiScores.deltaScores[11, 1];
			smallestGreatest := hiScores.greatScores[11, 0];
			scoresChanged := FALSE;
		end;

{-----------------------}

		procedure OpenMainWndo;
			var
				thePict: PicHandle;
				tempRect: Rect;
		begin
			mainWndo := nil;
			mainWndo := GetNewWindow(mainWndoID, nil, Pointer(-1));
			SelectWindow(mainWndo);
			SetPort(mainWndo);
			SizeWindow(mainWndo, screenBits.bounds.right, screenBits.bounds.bottom, FALSE);
			SetOrigin(-rightOffset, -downOffset);
			ShowWindow(mainWndo);
			SetPort(offVirginPort);

			thePict := GetPicture(backPictID);
			if (thePict <> nil) then
				begin
					HLock(Handle(thePict));
					tempRect := thePict^^.picFrame;
					DrawPicture(thePict, tempRect);
					HUnlock(Handle(thePict));
					ReleaseResource(Handle(thePict));
				end
			else
				FatalError;

			SetPort(mainWndo);
			CopyBits(offVirginMap, mainWndo^.portBits, tempRect, tempRect, srcCopy, mainWndo^.visRgn);
			CopyBits(offVirginMap, offLoadMap, tempRect, tempRect, srcCopy, nil);

			PenPat(black);
			PenMode(patXOr);
			PenSize(2, 2);
		end;

{-----------------------}

	begin
		SetCursor(GetCursor(WatchCursor)^^);
		CheckOurEnvirons;
		GetAllSounds;
		ReadInScores;
		SetUpMenus;
		ReadyBitmaps;
		InitForceTable;
		CreateRegions;
		if (not PrepareSounds) then
			FatalError;
		SetUpMouseForce;
		OpenMainWndo;

		doneFlag := FALSE;
		playing := FALSE;
		pausing := FALSE;
		jetsOut := FALSE;
		cursorVis := FALSE;
		autoPickUp := FALSE;

		whichOpponent := george;
		whichGame := deltaFive;
		mouseConst := 100;
		tileLit := -1;
		tileToggle := 0;
		numberOfStars := 0;
		frictState := normalFriction;
		playerName := 'Anonymous      ';

		GetDateTime(randSeed);

		ballCatches[0] := 'playerCatch';
		ballCatches[1] := 'maraCatch';
		ballCatches[2] := 'ottoCatch';
		ballCatches[3] := 'georgeCatch';
		ballCatches[4] := 'claireCatch';

		cursorArray[0] := GetCursor(4000);
		cursorArray[1] := GetCursor(4001);
		cursorArray[2] := GetCursor(4002);
		cursorArray[3] := GetCursor(4003);
		cursorArray[4] := GetCursor(4004);
		cursorArray[5] := GetCursor(4005);
		cursorArray[6] := GetCursor(4006);
		cursorArray[7] := GetCursor(4007);
		cursorArray[8] := GetCursor(4008);

		nontantTable[0] := northWest;
		nontantTable[1] := west;
		nontantTable[2] := southWest;
		nontantTable[3] := north;
		nontantTable[4] := resting;
		nontantTable[5] := south;
		nontantTable[6] := northEast;
		nontantTable[7] := east;
		nontantTable[8] := southEast;

		modeOpposites[north] := south;
		modeOpposites[south] := north;
		modeOpposites[east] := west;
		modeOpposites[west] := east;
		modeOpposites[southEast] := northWest;
		modeOpposites[northWest] := southEast;
		modeOpposites[southWest] := northEast;
		modeOpposites[northEast] := southWest;
		modeOpposites[resting] := resting;

		massNumbers[0, 0] := 3;
		massNumbers[0, 1] := 2;
		massNumbers[1, 0] := 1;
		massNumbers[1, 1] := 1;
		massNumbers[2, 0] := 2;
		massNumbers[2, 1] := 1;
		massNumbers[3, 0] := 1;
		massNumbers[3, 1] := 1;
		massNumbers[4, 0] := 1;
		massNumbers[4, 1] := 1;

		with ball do
			begin
				SetRect(src, 298, 125, 310, 137);
				SetRect(mask, 311, 125, 323, 137);
				mode := 0;
			end;

		stars[0, 0] := 26;
		stars[0, 1] := 30;
		stars[1, 0] := 43;
		stars[1, 1] := 24;
		stars[2, 0] := 59;
		stars[2, 1] := 34;
		stars[3, 0] := 48;
		stars[3, 1] := 49;
		stars[4, 0] := 72;
		stars[4, 1] := 79;
		stars[5, 0] := 112;
		stars[5, 1] := 55;
		stars[6, 0] := 41;
		stars[6, 1] := 255;
		stars[7, 0] := 129;
		stars[7, 1] := 303;
		stars[8, 0] := 487;
		stars[8, 1] := 223;
		stars[9, 0] := 495;
		stars[9, 1] := 237;
		stars[10, 0] := 506;
		stars[10, 1] := 266;
		stars[11, 0] := 469;
		stars[11, 1] := 278;
		stars[12, 0] := 200;
		stars[12, 1] := 326;
		stars[13, 0] := 487;
		stars[13, 1] := 57;
		stars[14, 0] := 325;
		stars[14, 1] := 30;
		stars[15, 0] := 17;
		stars[15, 1] := 22;
		stars[16, 0] := 12;
		stars[16, 1] := 15;
		stars[17, 0] := 32;
		stars[17, 1] := 17;
		stars[18, 0] := 61;
		stars[18, 1] := 25;
		stars[19, 0] := 63;
		stars[19, 1] := 37;
		stars[20, 0] := 76;
		stars[20, 1] := 74;
		stars[21, 0] := 96;
		stars[21, 1] := 66;
		stars[22, 0] := 205;
		stars[22, 1] := 11;
		stars[23, 0] := 269;
		stars[23, 1] := 28;
		stars[24, 0] := 255;
		stars[24, 1] := 3;
		stars[25, 0] := 5;
		stars[25, 1] := 225;
		stars[26, 0] := 28;
		stars[26, 1] := 223;
		stars[27, 0] := 12;
		stars[27, 1] := 274;
		stars[28, 0] := 64;
		stars[28, 1] := 276;
		stars[29, 0] := 86;
		stars[29, 1] := 334;
		stars[30, 0] := 437;
		stars[30, 1] := 9;
		stars[31, 0] := 502;
		stars[31, 1] := 58;
		stars[32, 0] := 487;
		stars[32, 1] := 20;
		stars[33, 0] := 381;
		stars[33, 1] := 47;
		stars[34, 0] := 449;
		stars[34, 1] := 83;
		stars[35, 0] := 492;
		stars[35, 1] := 123;
		stars[36, 0] := 489;
		stars[36, 1] := 220;
		stars[37, 0] := 484;
		stars[37, 1] := 232;
		stars[38, 0] := 504;
		stars[38, 1] := 235;
		stars[39, 0] := 508;
		stars[39, 1] := 254;
		stars[40, 0] := 457;
		stars[40, 1] := 258;
		stars[41, 0] := 501;
		stars[41, 1] := 270;
		stars[42, 0] := 320;
		stars[42, 1] := 313;
		stars[43, 0] := 254;
		stars[43, 1] := 328;
		stars[44, 0] := 155;
		stars[44, 1] := 325;

		SetRect(playerSrc[south, notCarrying], 1, 1, 33, 48);
		SetRect(playerSrc[southEast, notCarrying], 34, 1, 66, 48);
		SetRect(playerSrc[east, notCarrying], 67, 1, 99, 48);
		SetRect(playerSrc[northEast, notCarrying], 100, 1, 132, 48);
		SetRect(playerSrc[north, notCarrying], 133, 1, 165, 48);
		SetRect(playerSrc[northWest, notCarrying], 166, 1, 198, 48);
		SetRect(playerSrc[west, notCarrying], 199, 1, 231, 48);
		SetRect(playerSrc[southWest, notCarrying], 232, 1, 264, 48);
		SetRect(playerSrc[resting, notCarrying], 265, 1, 297, 48);

		for index := south to resting do
			begin
				playerSrc[index, carrying] := playerSrc[index, notCarrying];
				OffsetRect(playerSrc[index, carrying], 0, 48);
				playerSrc[index, crouching] := playerSrc[index, notCarrying];
				OffsetRect(playerSrc[index, crouching], 0, 96);
			end;

		for index := south to resting do
			begin
				playerMask[index, carrying] := playerSrc[index, carrying];
				OffsetRect(playerMask[index, carrying], 0, 144);
				playerMask[index, notCarrying] := playerSrc[index, notCarrying];
				OffsetRect(playerMask[index, notCarrying], 0, 144);
				playerMask[index, crouching] := playerSrc[index, crouching];
				OffsetRect(playerMask[index, crouching], 0, 144);
			end;

		for index := south to resting do
			begin
				opponentSrc[index, notCarrying] := playerSrc[index, notCarrying];
				opponentSrc[index, carrying] := playerSrc[index, carrying];
				opponentSrc[index, crouching] := playerSrc[index, crouching];
			end;

		SetRect(digitSrc[0], 301, 76, 312, 96);		{0}
		SetRect(digitSrc[1], 317, 76, 328, 96);		{1}
		SetRect(digitSrc[2], 333, 76, 344, 96);		{2}
		SetRect(digitSrc[3], 349, 76, 360, 96);		{3}
		SetRect(digitSrc[4], 365, 76, 376, 96);		{4}
		SetRect(digitSrc[5], 381, 76, 392, 96);		{5}
		SetRect(digitSrc[6], 397, 76, 408, 96);		{6}
		SetRect(digitSrc[7], 413, 76, 424, 96);		{7}
		SetRect(digitSrc[8], 301, 101, 312, 121);		{8}
		SetRect(digitSrc[9], 317, 101, 328, 121);		{9}
		SetRect(digitSrc[10], 301, 76, 312, 96);	{0}
		SetRect(digitSrc[11], 333, 101, 344, 121);	{solid black}
		SetRect(digitSrc[12], 349, 101, 360, 121);	{solid white}
		SetRect(digitSrc[13], 365, 101, 378, 121);	{-}
		SetRect(digitSrc[14], 381, 101, 392, 121);	{>}
		SetRect(digitSrc[15], 397, 101, 408, 121);	{<}
		SetRect(digitSrc[16], 413, 101, 424, 121);	{E}

		SetRect(timeDest[1], 21, 304, 32, 324);
		SetRect(timeDest[2], 42, 304, 53, 324);
		SetRect(timeDest[3], 58, 304, 69, 324);

		SetRect(earthScoreDest[1], 377, 304, 388, 324);
		SetRect(earthScoreDest[2], 393, 304, 404, 324);
		SetRect(earthScoreDest[3], 409, 304, 420, 324);

		SetRect(taygeteScoreDest[1], 449, 304, 460, 324);
		SetRect(taygeteScoreDest[2], 465, 304, 476, 324);
		SetRect(taygeteScoreDest[3], 481, 304, 492, 324);

		SetRect(titleSrc[0], 299, 2, 327, 35);	{P}
		SetRect(titleSrc[1], 330, 10, 356, 35);	{a}
		SetRect(titleSrc[2], 359, 10, 385, 35);	{r}
		SetRect(titleSrc[3], 330, 10, 356, 35);	{a}
		SetRect(titleSrc[4], 359, 10, 385, 35);	{r}
		SetRect(titleSrc[5], 388, 10, 414, 35);	{e}
		SetRect(titleSrc[6], 417, 10, 443, 35);	{n}
		SetRect(titleSrc[7], 330, 10, 356, 35);	{a}

		SetRect(titleMask[0], 299, 37, 327, 72);	{P}
		SetRect(titleMask[1], 330, 46, 356, 72);	{a}
		SetRect(titleMask[2], 359, 46, 385, 72);	{r}
		SetRect(titleMask[3], 330, 46, 356, 72);	{a}
		SetRect(titleMask[4], 359, 46, 385, 72);	{r}
		SetRect(titleMask[5], 388, 46, 414, 72);	{e}
		SetRect(titleMask[6], 417, 46, 443, 72);	{n}
		SetRect(titleMask[7], 330, 46, 356, 72);	{a}

		SetRect(jetsRects[0], 439, 205, 511, 255);
		SetRect(jetsRects[1], 439, 256, 511, 306);

		SetRect(teleportMask[0], 298, 241, 330, 288);
		SetRect(teleportMask[1], 331, 241, 363, 288);
		SetRect(teleportMask[2], 364, 241, 396, 288);
		SetRect(teleportMask[3], 397, 241, 429, 288);

		SetRect(foulSrc, 330, 38, 335, 43);
		SetRect(foulDest[0, 1], 380, 326, 385, 331);
		SetRect(foulDest[0, 2], 396, 326, 401, 331);
		SetRect(foulDest[0, 3], 412, 326, 417, 331);
		SetRect(foulDest[1, 1], 452, 326, 457, 331);
		SetRect(foulDest[1, 2], 468, 326, 473, 331);
		SetRect(foulDest[1, 3], 484, 326, 489, 331);

		SetRect(periodDest[1], 23, 326, 28, 331);
		SetRect(periodDest[2], 36, 326, 41, 331);
		SetRect(periodDest[3], 49, 326, 54, 331);
		SetRect(periodDest[4], 62, 326, 67, 331);
	end;

{=================================}

end.
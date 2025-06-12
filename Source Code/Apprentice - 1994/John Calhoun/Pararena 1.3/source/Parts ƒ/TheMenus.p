unit TheMenus;

interface

	uses
		Sound, Globals, Unlock, Dialogs, Utilities, Guts, Initialize;

	procedure NewGameVars;
	procedure NewPeriodVars;
	procedure EndOfPeriod;
	procedure Handle_My_Menu (theMenu, theItem: integer); {Handle menu selection}

implementation

{=================================}

	procedure NewGameVars;
		var
			index: Integer;
			dummyLong: LongInt;
			theSnd: Handle;
			signSrcL, signSrcR, signMaskL, signMaskR, signDestL, signDestR, tempRect: Rect;

	begin
		DisableItem(GetMenu(mGame), iBegin);
		EnableItem(GetMenu(mGame), iEnd);
		DisableItem(GetMenu(mGame), iWhichGame);
		DisableItem(GetMenu(mGame), iWhichOpponent);
		DisableItem(GetMenu(mOptions), iPhysics);
		HideMenuBar;

		SetUpMouseForce;

		SetPort(mainWndo);
		tempRect := ScreenBits.bounds;
		if (tempRect.bottom > 342) then
			begin
				tempRect.bottom := tempRect.top + 20;
				OffsetRect(tempRect, -rightOffset, -downOffset);
				ClipRect(tempRect);
				FillRect(tempRect, black);
				SetClip(wholeRgn);
			end;
		CopyBits(offVirginMap, mainWndo^.portBits, wholeScreen, wholeScreen, srcCopy, wholeRgn);

		tileLit := tileLit + 1;
		if (tileLit < 6) then
			repeat
				SetPort(mainWndo);
				InvertRgn(tileRgns[tileLit, tileToggle]);
				SetPort(offVirginPort);
				InvertRgn(tileRgns[tileLit, tileToggle]);
				tileLit := tileLit + 1;
			until (tileLit > 5);
		tileLit := -1;
		if (not cursorVis) then
			HideCursor;

		SetRect(signSrcL, 1, 266, 70, 321);
		SetRect(signMaskL, 71, 266, 140, 321);
		SetRect(signDestL, 70, 0, 139, 55);
		SetRect(signSrcR, 141, 266, 210, 321);
		SetRect(signMaskR, 211, 266, 280, 321);
		SetRect(signDestR, 370, 0, 439, 55);
		for index := 1 to 3 do
			begin
				CopyMask(offEnemyMap, offEnemyMap, mainWndo^.portBits, signSrcL, signMaskL, signDestL);
				CopyMask(offEnemyMap, offEnemyMap, mainWndo^.portBits, signSrcR, signMaskR, signDestR);
				Delay(5, dummyLong);
				CopyMask(offEnemyMap, offEnemyMap, mainWndo^.portBits, signMaskL, signMaskL, signDestL);
				CopyMask(offEnemyMap, offEnemyMap, mainWndo^.portBits, signMaskR, signMaskR, signDestR);
			end;
		CopyMask(offEnemyMap, offEnemyMap, mainWndo^.portBits, signSrcL, signMaskL, signDestL);
		CopyMask(offEnemyMap, offEnemyMap, mainWndo^.portBits, signSrcR, signMaskR, signDestR);

		playing := TRUE;
		pausing := FALSE;
		justQuit := FALSE;
		whichPeriod := 1;
		earthPoints := 0;
		taygetePoints := 0;

		FlushEvents(everyEvent, 0);

		numerator := massNumbers[whichOpponent, 0];
		denominator := massNumbers[whichOpponent, 1];

		case frictState of
			noFriction: 
				begin
					frictNum := 1;
					frictDen := 1;
				end;
			weakFriction: 
				begin
					frictNum := 79;
					frictDen := 80;
				end;
			normalFriction: 
				begin
					frictNum := 39;
					frictDen := 40;
				end;
			strongFriction: 
				begin
					frictNum := 19;
					frictDen := 20;
				end;
			otherwise
				begin
				end;
		end;

		ResetTitleVars;
		Delay(30, dummyLong);
		CopyBits(offVirginMap, mainWndo^.portBits, wholeScreen, wholeScreen, srcCopy, wholeRgn);
	end;

{=================================}

	procedure NewPeriodVars;

		procedure DisplayPeriod;
			var
				index: Integer;
		begin
			for index := 1 to whichPeriod do
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, periodDest[index], srcCopy, nil);
				end;
		end;

		procedure FireUpScoreBoards;
			var
				index: Integer;
				dummyLong: LongInt;
		begin
			for index := 1 to 18 do
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[DoRandom(17)], earthScoreDest[1], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[DoRandom(17)], earthScoreDest[2], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[DoRandom(17)], earthScoreDest[3], srcCopy, nil);
					Delay(2, dummyLong);
				end;
			CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[12], earthScoreDest[1], srcCopy, nil);
			CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[12], earthScoreDest[2], srcCopy, nil);
			CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[12], earthScoreDest[3], srcCopy, nil);
			for index := 1 to 18 do
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[DoRandom(17)], taygeteScoreDest[1], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[DoRandom(17)], taygeteScoreDest[2], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[DoRandom(17)], taygeteScoreDest[3], srcCopy, nil);
					Delay(2, dummyLong);
				end;
		end;

		procedure FireUpClock;
			var
				index: Integer;
				dummyLong: LongInt;
		begin
			for index := 1 to 4 do
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[12], timeDest[1], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[12], timeDest[2], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[12], timeDest[3], srcCopy, nil);
					Delay(10, dummyLong);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[11], timeDest[1], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[11], timeDest[2], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[11], timeDest[3], srcCopy, nil);
					Delay(15, dummyLong);
				end;
			CopyBits(offPlayerMap, offVirginMap, digitSrc[5], timeDest[1], srcCopy, nil);
			CopyBits(offPlayerMap, offVirginMap, digitSrc[0], timeDest[2], srcCopy, nil);
			CopyBits(offPlayerMap, offVirginMap, digitSrc[0], timeDest[3], srcCopy, nil);
		end;

		procedure DoAnnouncer;
			var
				drag, scan: Integer;
				dummyLong: LongInt;
				scanSrc, scanMask, scanSrc2, scanMask2, scanDest, wholeRct: Rect;

{----------------}

			procedure MouthOpen;
			begin
				CopyBits(offVirginMap, offLoadMap, scanDest, scanDest, srcCopy, nil);
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, scanSrc, scanMask, scanDest);
				CopyBits(offLoadMap, mainWndo^.portBits, scanDest, scanDest, srcCopy, nil);
			end;

{----------------}

			procedure MouthShut;
			begin
				CopyBits(offVirginMap, offLoadMap, scanDest, scanDest, srcCopy, nil);
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, scanSrc2, scanMask2, scanDest);
				CopyBits(offLoadMap, mainWndo^.portBits, scanDest, scanDest, srcCopy, nil);
			end;

{----------------}

		begin
			DoTheSound('beamIn', highPriority);
			for drag := 2 to 12 do
				begin
					for scan := 0 to 47 - drag do
						begin
							SetRect(scanSrc, 331, 145 + scan, 363, 145 + drag + scan);
							SetRect(scanMask, 331, 193 + scan, 363, 193 + drag + scan);
							SetRect(scanDest, 240, 193 + scan, 272, 193 + drag + scan);
							wholeRct := scanDest;
							wholeRct.top := wholeRct.top - 1;
							CopyBits(offVirginMap, offLoadMap, wholeRct, wholeRct, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, scanSrc, scanMask, scanDest);
							CopyBits(offLoadMap, mainWndo^.portBits, wholeRct, wholeRct, srcCopy, nil);
						end;
					CopyBits(offVirginMap, mainWndo^.portBits, wholeRct, wholeRct, srcCopy, nil);
				end;
			for scan := 0 to 46 do
				begin
					SetRect(scanSrc, 331, 145 + scan, 363, 146 + scan);
					SetRect(scanMask, 331, 193 + scan, 363, 194 + scan);
					SetRect(scanDest, 240, 193 + scan, 272, 194 + scan);
					CopyBits(offVirginMap, offLoadMap, scanDest, scanDest, srcCopy, nil);
					CopyMask(offPlayerMap, offPlayerMap, offLoadMap, scanSrc, scanMask, scanDest);
					CopyBits(offLoadMap, mainWndo^.portBits, scanDest, scanDest, srcCopy, nil);
				end;
			DoTheSound('pararena', highPriority);
			SetRect(scanSrc, 331, 145, 363, 192);
			SetRect(scanMask, 331, 193, 363, 240);
			SetRect(scanSrc2, 364, 145, 396, 192);
			SetRect(scanMask2, 364, 193, 396, 240);
			SetRect(scanDest, 240, 193, 272, 240);

			MouthOpen;	{"Ladies-"}
			Delay(7, dummyLong);
			MouthShut;	{"-"}
			Delay(2, dummyLong);

			MouthOpen;	{" and "}
			Delay(5, dummyLong);
			MouthShut;	{"-"}
			Delay(2, dummyLong);

			MouthOpen;	{" Gent- "}
			Delay(9, dummyLong);
			MouthShut;	{"-le-"}
			Delay(2, dummyLong);
			MouthOpen;	{"-men "}
			Delay(11, dummyLong);
			MouthShut;	{"-"}
			Delay(20, dummyLong);

			MouthOpen;	{"Par-"}
			Delay(30, dummyLong);
			MouthShut;
			Delay(5, dummyLong);
			MouthOpen;	{"-a-"}
			Delay(25, dummyLong);
			MouthShut;
			Delay(5, dummyLong);
			MouthOpen;	{"rena!!!!"}
			Delay(90, dummyLong);
			for scan := 0 to 46 do
				begin
					SetRect(scanDest, 240, 193 + scan, 272, 194 + scan);
					CopyBits(offVirginMap, offLoadMap, scanDest, scanDest, srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, scanDest, scanDest, srcCopy, nil);
				end;
		end;

{----------------}

		procedure SoloStartUp;
			var
				index: Integer;
		begin
			periodTime := 0;
			DrawEarthScore;
			DrawTaygeteScore;
			DisplayTime(periodTime);
			DisplayPeriod;

			with opponent do
				begin
					velX := 0;
					velZ := 0;
					posX := 52000;
					posZ := 52000;
				end;

			if (soundArray[4]) then
				DoTheSound('beamIn', highPriority);
			for index := 0 to 23 do
				BeamInPlayer(TRUE, player);
			startTime := TickCount div 60;
		end;

{----------------}

		procedure DualStartUp;
			var
				index: Integer;
		begin
			if (whichPeriod = 1) then
				DoAnnouncer;
			if (soundArray[1]) then
				DoTheSound('crowd', highPriority);
			FireUpScoreBoards;
			DrawEarthScore;
			DrawTaygeteScore;
			FireUpClock;
			DisplayTime(periodTime);
			DisplayPeriod;
			if (soundArray[4]) then
				DoTheSound('beamIn', highPriority);
			for index := 0 to 23 do
				BeamInPlayer(FALSE, opponent);
			if (soundArray[4]) then
				DoTheSound('beamIn', highPriority);
			for index := 0 to 23 do
				BeamInPlayer(TRUE, player);
			DrawPlayerOpponent;
			bothJustHit := FALSE;
			startTime := TickCount div 60;
			FireBall;
		end;

{----------------}

	begin
		earthFouls := 0;
		taygeteFouls := 0;
		DrawFouls(earthFouls, 0);
		DrawFouls(taygeteFouls, 1);

		with ball do
			begin
				SetRect(oldDest, 0, 0, 0, 0);
				SetRect(dest, 0, 0, 0, 0);
			end;

		with opponent do
			begin
				velX := 0;
				velZ := 0;
				posX := 2000;
				posZ := 0;
				mode := -1;
				timeKeeper := 0;
				carryMode := notCarrying;
				screenH := centerH + posX div 100;
				HLock(Handle(vertTable));
				screenV := vertTable^^.data[ABS(posX div 300), posZ div 300] + 20;
				HUnlock(Handle(vertTable));
				SetRect(dest, screenH - 13, screenV - 45, screenH + 14, screenV);
				oldDest := dest;
			end;

		with player do
			begin
				velX := 0;
				velZ := 0;
				posX := -2000;
				posZ := 0;
				mode := -1;
				timeKeeper := 0;
				carryMode := notCarrying;
				screenH := centerH + posX div 100;
				HLock(Handle(vertTable));
				screenV := vertTable^^.data[ABS(posX div 300), posZ div 300] + 20;
				HUnlock(Handle(vertTable));
				SetRect(dest, screenH - 13, screenV - 45, screenH + 14, screenV);
				oldDest := dest;
			end;

		case whichGame of
			practiceSkating: 
				begin
					SoloStartUp;
					DrawPlayer;
				end;
			practiceWBall: 
				begin
					SoloStartUp;
					DrawPlayer;
					FireBall;
				end;
			fourOfFive: 
				begin
					periodTime := 300;
					DualStartUp;
				end;
			fourOfNine: 
				begin
					periodTime := 540;
					DualStartUp;
				end;
			firstToThirteen: 
				begin
					periodTime := 0;
					DualStartUp;
				end;
			deltaFive: 
				begin
					periodTime := 0;
					DualStartUp;
				end;
			otherwise
				begin
				end;
		end;

		mad := FALSE;

		FlushEvents(everyEvent, 0);
		lastLoopTime := TickCount + delayTime;
	end;

{=================================}

	procedure DoEnd;
	begin
		InitCursor;
		EnableItem(GetMenu(mGame), iBegin);
		DisableItem(GetMenu(mGame), iEnd);
		EnableItem(GetMenu(mGame), iWhichGame);
		EnableItem(GetMenu(mGame), iWhichOpponent);
		if (not locked) then
			EnableItem(GetMenu(mOptions), iPhysics);
		CopyBits(offVirginMap, mainWndo^.portBits, screenArea, screenArea, srcCopy, nil);
		ShowMenuBar;
		DrawMenuBar;
		FlushEvents(everyEvent, 0);
		justQuit := TRUE;
		playing := FALSE;
		pausing := FALSE;
	end;

{=================================}

	procedure EndOfPeriod;
		var
			index: Integer;
	begin
		whichPeriod := whichPeriod + 1;
		OpponentOutOfArena;
		PlayerOutOfArena;
		for index := 1 to 22 do
			begin
				OpponentInTransit;
				PlayerInTransit;
			end;
		opponent.timeKeeper := 0;
		player.timeKeeper := 0;
		CopyBits(offVirginMap, mainWndo^.portBits, ball.oldDest, ball.oldDest, srcCopy, wholeRgn);
		CopyBits(offVirginMap, mainWndo^.portBits, ball.dest, ball.dest, srcCopy, wholeRgn);

		case whichGame of
			fourOfFive, fourOfNine: 
				begin
					if (whichPeriod > 4) then
						begin
							DoGameOver;
						end
					else
						begin
							NewPeriodVars;
						end;
				end;
			firstToThirteen, deltaFive: 
				DoGameOver;
			otherwise
		end;
	end;

{=================================}

	procedure Handle_My_Menu;               {Handle menu selections realtime}
		var
			DNA: integer;
			BoolHolder: boolean;
			DAName: Str255;
			SavePort: GrafPtr;

{-----------------}

		procedure DoApple;
		begin
			case theItem of
				iNothing: 
					;
				iAbout: 
					begin
						CopyBits(mainWndo^.portBits, offLoadMap, wholeScreen, wholeScreen, srcCopy, wholeRgn);
						if ShowAboutDialog then
							DoUnlock;
						CopyBits(offLoadMap, mainWndo^.portBits, wholeScreen, wholeScreen, srcCopy, wholeRgn);
					end;
				otherwise
					begin
						GetPort(SavePort);
						GetItem(GetMenu(mApple), theItem, DAName);
						DNA := OpenDeskAcc(DAName);
						SetPort(SavePort);
					end;
			end;
		end;

{-----------------}

		procedure DoGame;
		begin
			case theItem of                   {Handle all commands in this menu list}
				iBegin: 
					begin
						NewGameVars;
						NewPeriodVars;
					end;
				iEnd: 
					DoEnd;
				iQuit: 
					begin
						doneFlag := TRUE;
						playing := FALSE;
						pausing := FALSE;
						InitCursor;
					end;
				otherwise
			end;
		end;

{-----------------}

		procedure DoWhichGame;
			var
				index: Integer;
		begin
			for index := iPractice to iDeltaFive do
				CheckItem(GetMenu(mWhichGame), index, FALSE);
			CheckItem(GetMenu(mWhichGame), theItem, TRUE);
			whichGame := theItem - 1;
		end;

{-----------------}

		procedure DoOpponents;
			var
				index: Integer;
		begin
			for index := iMara to iClaire do
				CheckItem(GetMenu(mOpponents), index, FALSE);
			CheckItem(GetMenu(mOpponents), theItem, TRUE);
			whichOpponent := theItem;
		end;

{-----------------}

		procedure DoOptions;
			var
				index: Integer;
		begin
			case theItem of
				iVisCurs: 
					begin
						cursorVis := not cursorVis;
						CheckItem(GetMenu(mOptions), iVisCurs, cursorVis);
					end;
				iAutoPickUp: 
					begin
						autoPickUp := not autoPickUp;
						CheckItem(GetMenu(mOptions), iAutoPickUp, autoPickUp);
					end;
				iSound: 
					DoSound;
				iSpeed: 
					DoSpeed;
				iPhysics: 
					begin
						DoPhysics;
						SetUpMouseForce;
					end;
				iHelpRules: 
					DoHelpRules;
				iHelpOther: 
					DoHelpPlay;

				iHiScores: 
					begin
						ShowHiScores;
					end;
				otherwise
			end;
		end;

{-----------------}

	begin
		case theMenu of
			mApple: 
				DoApple;
			mGame: 
				DoGame;
			mWhichGame: 
				DoWhichGame;
			mOpponents: 
				DoOpponents;
			mOptions: 
				DoOptions;
			otherwise
		end;

		HiliteMenu(0);
	end;

{=================================}

end.                                    {End of unit}
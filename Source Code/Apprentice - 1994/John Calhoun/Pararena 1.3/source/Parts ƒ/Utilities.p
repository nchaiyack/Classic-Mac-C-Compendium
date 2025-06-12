unit Utilities;

interface

	uses
		Sound, Globals, Unlock;

	procedure ResetTitleVars;
	procedure DoErrorSound (soundNumber: Integer);
	procedure DoJets;
	procedure BeamInPlayer (us: Boolean; var who: dynaMap);
	function DoRandom (range: Integer): Integer;
	procedure DoTheSound (whichOne: Str255; priority: Integer);
	procedure FireBall;
	procedure PlayerOutOfArena;
	procedure PlayerInTransit;
	procedure OpponentOutOfArena;
	procedure OpponentInTransit;
	function LapsedTime: LongInt;
	procedure BallOutOfArena;
	procedure DoHolding;
	procedure DrawEarthScore;
	procedure DrawTaygeteScore;
	procedure DoAScore (whoseGoal: Integer);
	procedure DrawPlayerOpponent;
	procedure DrawOpponentPlayer;
	procedure DrawPlayer;
	procedure DrawPlayerAndBall;
	procedure DisplayTime (timeIs: LongInt);
	procedure DrawFouls (fouls, who: Integer);
	procedure DoGameOver;
	procedure SetUpMouseForce;
	procedure HideMenuBar;
	procedure ShowMenuBar;

implementation

{=================================}

	procedure ResetTitleVars;
		const
			leftEdge = -12000;
			topEdge = -12000;
		var
			index: Integer;
	begin
		jetsOut := FALSE;
		for index := 0 to 7 do
			with titleLetters[index] do
				begin
					posX := leftEdge + 3000 * index;
					posZ := topEdge;
					velX := 10 * (DoRandom(100) - 50);
					velZ := DoRandom(100) - 50;
					screenH := centerH + posX div 100;
					HLock(Handle(vertTable));
					screenV := vertTable^^.data[ABS(posX div 300), posZ div 300];
					HUnlock(Handle(vertTable));
					if index = 0 then
						SetRect(dest, screenH - 14, screenV - 33, screenH + 14, screenV)
					else
						SetRect(dest, screenH - 13, screenV - 25, screenH + 13, screenV);
					oldDest := dest;
				end;
	end;

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

	procedure DoJets;
		var
			tempRect: Rect;
	begin
		OffsetRect(jetsRects[2], 4, 0);
		tempRect := jetsRects[2];
		tempRect.left := tempRect.left - 4;
		CopyBits(offVirginMap, offLoadMap, tempRect, tempRect, srcCopy, nil);
		CopyMask(offEnemyMap, offEnemyMap, offLoadMap, jetsRects[0], jetsRects[1], jetsRects[2]);
		CopyBits(offLoadMap, mainWndo^.portBits, tempRect, tempRect, srcCopy, wholeRgn);
		if (tempRect.left > 512) then
			jetsOut := FALSE;
	end;

{=================================}

	procedure BeamInPlayer;
		var
			howRezzed: Integer;
	begin
		mad := FALSE;
		with who do
			begin
				mode := mode + 1;
				howRezzed := mode div 6;
				CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
				if (us) then
					CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[howRezzed], dest)
				else
					CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[howRezzed], dest);
				CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
				if (mode > 22) then
					mode := resting;
			end;
	end;

{=================================}

	function DoRandom;
		var
			rawResult: LongInt;
	begin
		rawResult := ABS(Random);
		DoRandom := (rawResult * range) div 32768;
	end;

{=================================}

	function GetA5: LONGINT;
	inline
		$2E8D; {MOVE.L A5,(A7)}

{=================================}

	function LoadA5 (newA5: LONGINT): LONGINT;
	inline
		$2F4D, $0004, $2A5F;

{=================================}

	procedure ChanCallBack (chan: SndChannelPtr; cmd: SndCommand);
		var
			oldA5: LongInt;
	begin
		if cmd.param1 <> 12345 then
			Exit(ChanCallBack); 			{ Skip err callBackCmd w/ System 6.0.4 }
		oldA5 := LoadA5(cmd.param2);		{ get the application's A5 and set it }
		soundPriority := noSound;
		oldA5 := LoadA5(oldA5);			{ restore old A5 }
	end;

{=================================}

	procedure DoTheSound;
		var
			aCommand: SndCommand;
			err: OSErr;
	begin
		if (not soundOn) then
			Exit(DoTheSound);

		if (priority < soundPriority) then
			Exit(DoTheSound);

		soundPriority := priority;

		theSnd := GetNamedResource('snd ', whichOne);
		if (theSnd^ = nil) then
			Exit(DoTheSound);

		if (chanPtr <> nil) then
			begin
				with aCommand do
					begin
						cmd := quietCmd;
						param1 := 0;
						param2 := 0;
					end;
				err := SndDoImmediate(chanPtr, aCommand);
				err := SndDisposeChannel(chanPtr, TRUE);
				chanPtr := nil;
			end;

		err := SndNewChannel(chanPtr, 0, 0, @ChanCallBack);
		if (err = noErr) then
			err := SndPlay(chanPtr, theSnd, TRUE);
		with aCommand do
			begin
				cmd := callBackCmd;
				param1 := 12345;
				param2 := GetA5;
			end;
		if (err = noErr) then
			err := SndDoCommand(chanPtr, aCommand, FALSE);

	end;

{=================================}

	procedure FireBall;
	begin
		ballJustHit := FALSE;
		clicked := FALSE;
		lastTouchingBall := nooneHasBall;
		with ball do
			begin
				CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, wholeRgn);
				CopyBits(offVirginMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
				mode := nooneHasBall;
				timeKeeper := -40;
				SetRect(dest, 1, 30, 1, 30);
				oldDest := dest;
			end;
		player.carryMode := notCarrying;
	end;

{=================================}

	procedure PlayerOutOfArena;
		var
			lostBall: Boolean;
	begin
		mad := FALSE;
		lostBall := FALSE;
		if (ball.mode = playerHasBall) then
			lostBall := TRUE;
		if (lostBall) then
			BallOutOfArena;
		with player do
			begin
				timeKeeper := -100;
				CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
				CopyBits(offVirginMap, offLoadMap, oldDest, oldDest, srcCopy, nil);
			end;
	end;

{=================================}

	procedure PlayerInTransit;
	begin
		with player do
			begin
				mode := resting;
				case timeKeeper of
					-100: 
						begin
							if (soundArray[4]) then
								DoTheSound('beamOut', highPriority);
							CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, wholeRgn);
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyBits(offVirginMap, offLoadMap, oldDest, oldDest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[3], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-99..-95: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[3], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-94..-90: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[2], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-89..-85: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[1], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-84..-80: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[0], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-79: 
						begin
							CopyBits(offVirginMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
						end;
					-78..-22: 
						begin
						end;
					-21: 
						begin
							if (soundArray[4]) then
								DoTheSound('beamIn', highPriority);
							velX := 0;
							velZ := 0;
							posX := -2000;
							posZ := 0;
							mode := resting;
							carryMode := notCarrying;
							screenH := centerH + posX div 100;
							HLock(Handle(vertTable));
							screenV := vertTable^^.data[ABS(posX div 300), posZ div 300] + 20;
							HUnlock(Handle(vertTable));
							SetRect(dest, screenH - 13, screenV - 45, screenH + 14, screenV);
							oldDest := dest;
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[0], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-20..-16: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[0], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-15..-11: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[1], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-10..-6: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[2], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-5..0: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[3], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					otherwise
				end;
				timeKeeper := timeKeeper + 1;
			end;
	end;

{=================================}

	procedure OpponentOutOfArena;
		var
			lostBall: Boolean;
	begin
		lostBall := FALSE;
		if (ball.mode = opponentHasBall) then
			lostBall := TRUE;
		if (lostBall) then
			BallOutOfArena;
		with opponent do
			begin
				timeKeeper := -100;
				CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
				CopyBits(offVirginMap, offLoadMap, oldDest, oldDest, srcCopy, nil);
			end;
	end;

{=================================}

	procedure OpponentInTransit;
	begin
		with opponent do
			begin
				mode := resting;
				case timeKeeper of
					-100: 
						begin
							if (soundArray[4]) then
								DoTheSound('beamOut', highPriority);
							CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, wholeRgn);
							CopyBits(offVirginMap, offLoadMap, oldDest, oldDest, srcCopy, nil);
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[3], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-99..-95: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[3], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-94..-90: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[2], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-89..-85: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[1], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-84..-80: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[0], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-79: 
						begin
							CopyBits(offVirginMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
						end;
					-78..-22: 
						begin
						end;
					-21: 
						begin
							if (soundArray[4]) then
								DoTheSound('beamIn', highPriority);
							velX := 0;
							velZ := 0;
							posX := 2000;
							posZ := 0;
							mode := resting;
							carryMode := notCarrying;
							screenH := centerH + posX div 100;
							HLock(Handle(vertTable));
							screenV := vertTable^^.data[ABS(posX div 300), posZ div 300] + 20;
							HUnlock(Handle(vertTable));
							SetRect(dest, screenH - 13, screenV - 45, screenH + 14, screenV);
							oldDest := dest;
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, playerSrc[resting, 0], teleportMask[0], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-20..-16: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[0], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-15..-11: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[1], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-10..-6: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[2], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					-5..0: 
						begin
							CopyBits(offVirginMap, offLoadMap, dest, dest, srcCopy, nil);
							CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[resting, 0], teleportMask[3], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
						end;
					otherwise
				end;
				timeKeeper := timeKeeper + 1;
			end;
	end;

{=================================}

	function LapsedTime;
	begin
		currentTime := TickCount div 60;
		LapsedTime := currentTime - startTime;
	end;

{=================================}

	procedure DoAFoul;
	begin
		if (lastTouchingBall = playerHasBall) then
			begin
				if (soundArray[2]) then
					DoTheSound('foul', highPriority);
				earthFouls := earthFouls + 1;
				if (earthFouls > 3) then
					begin
						earthFouls := 0;
						taygetePoints := taygetePoints + 1;
						if (soundArray[1]) then
							DoTheSound('crowd', highPriority);
						DrawTaygeteScore;
					end;
				DrawFouls(earthFouls, 0);
			end
		else if (lastTouchingBall = opponentHasBall) then
			begin
				if (soundArray[2]) then
					DoTheSound('foul', highPriority);
				taygeteFouls := taygeteFouls + 1;
				if (taygeteFouls > 3) then
					begin
						taygeteFouls := 0;
						earthPoints := earthPoints + 1;
						if (soundArray[1]) then
							DoTheSound('crowd', highPriority);
						DrawEarthScore;
					end;
				DrawFouls(taygeteFouls, 1);
			end;
	end;

{=================================}

	procedure BallOutOfArena;
	begin
		CopyBits(offVirginMap, mainWndo^.portBits, ball.oldDest, ball.oldDest, srcCopy, wholeRgn);
		DoAFoul;
		FireBall;
	end;

{=================================}

	procedure DoHolding;
	begin
		lastTouchingBall := ball.mode;
		opponent.carryMode := notCarrying;
		player.carryMode := notCarrying;
		DoAFoul;
		FireBall;
	end;

{=================================}

	procedure DrawEarthScore;
		var
			digit1, digit2, digit3: Integer;
	begin
		digit1 := earthPoints div 100;
		digit2 := (earthPoints - (digit1 * 100)) div 10;
		digit3 := earthPoints - ((digit1 * 100) + (digit2 * 10));
		if (digit1 = 0) then
			digit1 := 11;
		if ((digit1 = 11) and (digit2 = 0)) then
			digit2 := 11;
		CopyBits(offPlayerMap, offVirginMap, digitSrc[digit1], earthScoreDest[1], srcCopy, nil);
		CopyBits(offPlayerMap, offVirginMap, digitSrc[digit2], earthScoreDest[2], srcCopy, nil);
		CopyBits(offPlayerMap, offVirginMap, digitSrc[digit3], earthScoreDest[3], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit1], earthScoreDest[1], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit2], earthScoreDest[2], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit3], earthScoreDest[3], srcCopy, nil);
	end;

{=================================}

	procedure DrawTaygeteScore;
		var
			digit1, digit2, digit3: Integer;
	begin
		digit1 := taygetePoints div 100;
		digit2 := (taygetePoints - (digit1 * 100)) div 10;
		digit3 := taygetePoints - ((digit1 * 100) + (digit2 * 10));
		if (digit1 = 0) then
			digit1 := 11;
		if ((digit1 = 11) and (digit2 = 0)) then
			digit2 := 11;
		CopyBits(offPlayerMap, offVirginMap, digitSrc[digit1], taygeteScoreDest[1], srcCopy, nil);
		CopyBits(offPlayerMap, offVirginMap, digitSrc[digit2], taygeteScoreDest[2], srcCopy, nil);
		CopyBits(offPlayerMap, offVirginMap, digitSrc[digit3], taygeteScoreDest[3], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit1], taygeteScoreDest[1], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit2], taygeteScoreDest[2], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit3], taygeteScoreDest[3], srcCopy, nil);
	end;

{=================================}

	procedure DoAScore;
		var
			tempWhole: Rect;
	begin
		with ball do
			begin
				CopyBits(offVirginMap, mainWndo^.portBits, dest, dest, srcCopy, wholeRgn);
				CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, wholeRgn);
				timeKeeper := -2;
			end;

		if (whoseGoal = 20000) then
			begin
				with player do
					begin
						UnionRect(oldDest, dest, tempWhole);
						CopyBits(offVirginMap, offLoadMap, tempWhole, tempWhole, srcCopy, nil);
						if (timeKeeper > -1) then
							CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[mode, carryMode], playerMask[mode, carryMode], dest);
						CopyBits(offLoadMap, mainWndo^.portBits, tempWhole, tempWhole, srcCopy, wholeRgn);
					end;
				earthPoints := earthPoints + 1;
				if (soundArray[1]) then
					DoTheSound('crowd', lowPriority);
				earthFouls := 0;
				DrawFouls(0, 0);
				DrawEarthScore;
			end
		else
			begin
				if (whichGame > practiceWBall) then
					with opponent do
						begin
							UnionRect(oldDest, dest, tempWhole);
							CopyBits(offVirginMap, offLoadMap, tempWhole, tempWhole, srcCopy, nil);
							if (timeKeeper > -1) then
								CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[mode, carryMode], playerMask[mode, carryMode], dest);
							CopyBits(offLoadMap, mainWndo^.portBits, tempWhole, tempWhole, srcCopy, wholeRgn);
						end;
				taygetePoints := taygetePoints + 1;
				if (soundArray[1]) then
					DoTheSound('crowd', lowPriority);
				taygeteFouls := 0;
				DrawFouls(0, 1);
				DrawTaygeteScore;
			end;
		FireBall;
	end;

{=================================}

	procedure DrawPlayerOpponent;
		var
			ourMode: Integer;
			wholePlayer, wholeOpponent, wholeBall: Rect;
	begin
		if (tileLit > -1) then
			begin
				SetPort(offVirginPort);
				InvertRgn(tileRgns[tileLit, tileToggle]);
			end;

		with ball do
			if (timeKeeper = 0) then
				begin
					UnionRect(oldDest, dest, wholeBall);
					CopyBits(offVirginMap, offLoadMap, wholeBall, wholeBall, srcCopy, nil);
				end;
		with player do
			begin
				UnionRect(oldDest, dest, wholePlayer);
				CopyBits(offVirginMap, offLoadMap, wholePlayer, wholePlayer, srcCopy, nil);
			end;
		with opponent do
			begin
				UnionRect(oldDest, dest, wholeOpponent);
				CopyBits(offVirginMap, offLoadMap, wholeOpponent, wholeOpponent, srcCopy, nil);
			end;

		with ball do
			if (timeKeeper = 0) then
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, src, mask, dest);
		with player do
			if (timeKeeper > -1) then
				begin
					ourMode := carryMode;
					if ((Button) and (carryMode = notCarrying)) then
						ourMode := crouching;
					CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[mode, ourMode], playerMask[mode, ourMode], dest);
				end;
		with opponent do
			if (timeKeeper > -1) then
				begin
					CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[mode, carryMode], playerMask[mode, carryMode], dest);
				end;

		if (tileLit > -1) then
			begin
				SetPort(mainWndo);
				InvertRgn(tileRgns[tileLit, tileToggle]);
				tileLit := tileLit + 1;
				if (tileLit > 5) then
					tileLit := -1;
				SetPort(offVirginPort);
			end;

		with ball do
			if (timeKeeper = 0) then
				begin
					CopyBits(offLoadMap, mainWndo^.portBits, wholeBall, wholeBall, srcCopy, ballVisRgn);
					oldDest := dest;
				end;
		with player do
			begin
				CopyBits(offLoadMap, mainWndo^.portBits, wholePlayer, wholePlayer, srcCopy, wholeRgn);
				oldDest := dest;
			end;
		with opponent do
			begin
				CopyBits(offLoadMap, mainWndo^.portBits, wholeOpponent, wholeOpponent, srcCopy, wholeRgn);
				oldDest := dest;
			end;
	end;


{=================================}

	procedure DrawOpponentPlayer;
		var
			ourMode: Integer;
			wholePlayer, wholeOpponent, wholeBall: Rect;
	begin
		if (tileLit > -1) then
			begin
				SetPort(offVirginPort);
				InvertRgn(tileRgns[tileLit, tileToggle]);
			end;

		with ball do
			if (timeKeeper = 0) then
				begin
					UnionRect(oldDest, dest, wholeBall);
					CopyBits(offVirginMap, offLoadMap, wholeBall, wholeBall, srcCopy, nil);
				end;
		with opponent do
			begin
				UnionRect(oldDest, dest, wholeOpponent);
				CopyBits(offVirginMap, offLoadMap, wholeOpponent, wholeOpponent, srcCopy, nil);
			end;
		with player do
			begin
				UnionRect(oldDest, dest, wholePlayer);
				CopyBits(offVirginMap, offLoadMap, wholePlayer, wholePlayer, srcCopy, nil);
			end;

		with ball do
			if (timeKeeper = 0) then
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, src, mask, dest);
		with opponent do
			if (timeKeeper > -1) then
				begin
					CopyMask(offEnemyMap, offPlayerMap, offLoadMap, opponentSrc[mode, carryMode], playerMask[mode, carryMode], dest);
				end;
		with player do
			if (timeKeeper > -1) then
				begin
					ourMode := carryMode;
					if ((Button) and (carryMode = notCarrying)) then
						ourMode := crouching;
					CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[mode, ourMode], playerMask[mode, ourMode], dest);
				end;

		if (tileLit > -1) then
			begin
				SetPort(mainWndo);
				InvertRgn(tileRgns[tileLit, tileToggle]);
				tileLit := tileLit + 1;
				if (tileLit > 5) then
					tileLit := -1;
				SetPort(offVirginPort);
			end;

		with ball do
			if (timeKeeper = 0) then
				begin
					CopyBits(offLoadMap, mainWndo^.portBits, wholeBall, wholeBall, srcCopy, ballVisRgn);
					oldDest := dest;
				end;
		with opponent do
			begin
				CopyBits(offLoadMap, mainWndo^.portBits, wholeOpponent, wholeOpponent, srcCopy, wholeRgn);
				oldDest := dest;
			end;
		with player do
			begin
				CopyBits(offLoadMap, mainWndo^.portBits, wholePlayer, wholePlayer, srcCopy, wholeRgn);
				oldDest := dest;
			end;
	end;

{=================================}

	procedure DrawPlayer;
		var
			ourMode: Integer;
			wholePlayer: Rect;
	begin
		with player do
			begin
				UnionRect(oldDest, dest, wholePlayer);
				CopyBits(offVirginMap, offLoadMap, wholePlayer, wholePlayer, srcCopy, nil);
				if (timeKeeper > -1) then
					begin
						ourMode := carryMode;
						if ((Button) and (carryMode = notCarrying)) then
							ourMode := crouching;
						CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[mode, ourMode], playerMask[mode, ourMode], dest);
					end;
				CopyBits(offLoadMap, mainWndo^.portBits, wholePlayer, wholePlayer, srcCopy, wholeRgn);
				oldDest := dest;
			end;
	end;

{=================================}

	procedure DrawPlayerAndBall;
		var
			ourMode: Integer;
			wholePlayer, wholeBall: Rect;
	begin
		with ball do
			if (timeKeeper = 0) then
				begin
					UnionRect(oldDest, dest, wholeBall);
					CopyBits(offVirginMap, offLoadMap, wholeBall, wholeBall, srcCopy, nil);
				end;
		with player do
			begin
				UnionRect(oldDest, dest, wholePlayer);
				CopyBits(offVirginMap, offLoadMap, wholePlayer, wholePlayer, srcCopy, nil);
			end;

		with ball do
			if (timeKeeper = 0) then
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, src, mask, dest);
		with player do
			if (timeKeeper > -1) then
				begin
					ourMode := carryMode;
					if ((Button) and (carryMode = notCarrying)) then
						ourMode := crouching;
					CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerSrc[mode, ourMode], playerMask[mode, ourMode], dest);
				end;

		with ball do
			if (timeKeeper = 0) then
				begin
					CopyBits(offLoadMap, mainWndo^.portBits, wholeBall, wholeBall, srcCopy, ballVisRgn);
					oldDest := dest;
				end;
		with player do
			begin
				CopyBits(offLoadMap, mainWndo^.portBits, wholePlayer, wholePlayer, srcCopy, wholeRgn);
				oldDest := dest;
			end;
	end;

{=================================}

	procedure DisplayTime;
		var
			digit1, digit2, digit3: Integer;
	begin
		digit1 := timeIs div 60;
		timeIs := (timeIs mod 60);
		digit2 := timeIs div 10;
		digit3 := timeIs mod 10;
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit1], timeDest[1], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit2], timeDest[2], srcCopy, nil);
		CopyBits(offPlayerMap, mainWndo^.portBits, digitSrc[digit3], timeDest[3], srcCopy, nil);
	end;

{=================================}

	procedure DrawFouls;
	begin
		case fouls of
			0: 
				begin
					CopyBits(offVirginMap, mainWndo^.portBits, foulDest[who, 1], foulDest[who, 1], srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, foulDest[who, 2], foulDest[who, 2], srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, foulDest[who, 3], foulDest[who, 3], srcCopy, nil);
				end;
			1: 
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, foulDest[who, 1], srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, foulDest[who, 2], foulDest[who, 2], srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, foulDest[who, 3], foulDest[who, 3], srcCopy, nil);
				end;
			2: 
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, foulDest[who, 1], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, foulDest[who, 2], srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, foulDest[who, 3], foulDest[who, 3], srcCopy, nil);
				end;
			3: 
				begin
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, foulDest[who, 1], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, foulDest[who, 2], srcCopy, nil);
					CopyBits(offPlayerMap, mainWndo^.portBits, foulSrc, foulDest[who, 3], srcCopy, nil);
				end;
			otherwise
				begin
				end;
		end;
	end;

{=================================}

	procedure DoGameOver;

{-------------------}

		procedure DoHoopla;
			var
				index: Integer;
				astroSrc, astroMask, astroDest, wholeAstro, oldAstro: Rect;
		begin
			repeat
			until (soundPriority = noSound);
			DoTheSound('crowd', lowPriority);
			SetRect(astroSrc, 365, 91, 511, 147);
			SetRect(astroMask, 365, 148, 511, 204);
			SetRect(astroDest, 512, 201, 658, 257);
			oldAstro := astroDest;
			for index := 1 to 350 do
				begin
					UnionRect(astroDest, oldAstro, wholeAstro);
					CopyBits(offVirginMap, offLoadMap, wholeAstro, wholeAstro, srcCopy, nil);
					CopyMask(offEnemyMap, offEnemyMap, offLoadMap, astroSrc, astroMask, astroDest);
					CopyBits(offLoadMap, mainWndo^.portBits, wholeAstro, wholeAstro, srcCopy, wholeRgn);
					oldAstro := astroDest;
					OffsetRect(astroDest, -2, 0);
				end;

			if ((earthPoints > smallestGreatest) and (not locked)) then
				begin
					GetName(playerName);
					CopyBits(offVirginMap, mainWndo^.portBits, screenArea, screenArea, srcCopy, wholeRgn);
					DoGreatestScore;
					if ((earthPoints - taygetePoints) > smallestDelta) then
						DoDeltaScore;
					ShowHiScores;
				end
			else if (((earthPoints - taygetePoints) > smallestDelta) and (not locked)) then
				begin
					GetName(playerName);
					CopyBits(offVirginMap, mainWndo^.portBits, screenArea, screenArea, srcCopy, wholeRgn);
					DoDeltaScore;
					ShowHiScores;
				end;
			CopyBits(offVirginMap, mainWndo^.portBits, screenArea, screenArea, srcCopy, wholeRgn);
		end;

{-------------------}

		procedure DoBummer;
		begin
			DoTheSound('crowd', lowPriority);

			if (taygetePoints > smallestGreatest) then
				DoGreatestScore;
			if ((taygetePoints - earthPoints) > smallestDelta) then
				DoDeltaScore;
		end;

{--------vvv--------}

	begin
		InitCursor;
		FlushEvents(everyEvent, 0);

		if (earthPoints > taygetePoints) then
			DoHoopla
		else if ((earthPoints = taygetePoints) and (earthFouls < taygeteFouls)) then
			DoHoopla
		else
			DoBummer;

		EnableItem(GetMenu(mGame), iBegin);
		DisableItem(GetMenu(mGame), iEnd);
		EnableItem(GetMenu(mGame), iWhichGame);
		EnableItem(GetMenu(mGame), iWhichOpponent);

		if (not locked) then
			EnableItem(getMenu(mOptions), iPhysics);

		CopyBits(offVirginMap, mainWndo^.portBits, wholeScreen, wholeScreen, srcCopy, wholeRgn);
		ShowMenuBar;
		playing := FALSE;
		pausing := FALSE;
		justQuit := TRUE;
	end;

{=================================}

	procedure SetUpMouseForce;
	begin
		mouseForce[south, 0] := 0;
		mouseForce[south, 1] := TRUNC(1 * mouseConst);
		mouseForce[southEast, 0] := TRUNC(0.707 * mouseConst);
		mouseForce[southEast, 1] := TRUNC(0.707 * mouseConst);
		mouseForce[east, 0] := TRUNC(1 * mouseConst);
		mouseForce[east, 1] := 0;
		mouseForce[northEast, 0] := TRUNC(0.707 * mouseConst);
		mouseForce[northEast, 1] := TRUNC(-0.707 * mouseConst);
		mouseForce[north, 0] := 0;
		mouseForce[north, 1] := TRUNC(-1 * mouseConst);
		mouseForce[northWest, 0] := TRUNC(-0.707 * mouseConst);
		mouseForce[northWest, 1] := TRUNC(-0.707 * mouseConst);
		mouseForce[west, 0] := TRUNC(-1 * mouseConst);
		mouseForce[west, 1] := 0;
		mouseForce[southWest, 0] := TRUNC(-0.707 * mouseConst);
		mouseForce[southWest, 1] := TRUNC(0.707 * mouseConst);
		mouseForce[resting, 0] := 0;
		mouseForce[resting, 1] := 0;
	end;

{=================================}

	function GetGrayRgn: RgnHandle;
	inline
		$2EB8, $09EE;

{=================================}

	function GetMBarHeight: Integer;
	inline
		$3EB8, $0BAA;

{=================================}

	procedure SetMBarHeight (newHeight: Integer);
	inline
		$31DF, $0BAA;

{=================================}

	function GetWindowList: WindowPtr;
	inline
		$2EB8, $9D6;

{=================================}

	function GetMBarRgn: RgnHandle;
		var
			theRect: Rect;
			worldRgn, mBarRgn: RgnHandle;
	begin
		theRect := GetGrayRgn^^.rgnBBox;
		UnionRect(theRect, screenBits.bounds, theRect);
		worldRgn := NewRgn;
		OpenRgn;
		FrameRoundRect(theRect, 16, 16);
		CloseRgn(worldRgn);

		theRect := screenBits.bounds;
		theRect.bottom := theRect.top + wasMBarHeight;
		mBarRgn := NewRgn;
		RectRgn(mBarRgn, theRect);
		SectRgn(worldRgn, mBarRgn, mBarRgn);

		DisposeRgn(worldRgn);
		GetMBarRgn := mBarRgn;
	end;

{=================================}

	procedure HideMenuBar;
		var
			theRect: Rect;
			mBarHeight: Integer;
			grayRgn, menuBarRgn: RgnHandle;
			startWindow: WindowPeek;
	begin
		mBarHeight := GetMBarHeight;
		if (mBarHeight <> 0) then
			begin
				grayRgn := GetGrayRgn;
				wasMBarHeight := mBarHeight;
				menuBarRgn := GetMBarRgn;
				SetMBarHeight(0);
				UnionRgn(grayRgn, menuBarRgn, grayRgn);

				startWindow := WindowPeek(GetWindowList);
				PaintBehind(startWindow, menuBarRgn);
				CalcVisBehind(startWindow, menuBarRgn);
				DisposeRgn(menuBarRgn);
			end;
	end;

{=================================}

	procedure ShowMenuBar;
		var
			grayRgn, menuBarRgn: RgnHandle;
	begin
		if (GetMBarHeight = 0) then
			begin
				grayRgn := GetGrayRgn;
				menuBarRgn := GetMBarRgn;
				SetMBarHeight(wasMBarHeight);

				DiffRgn(grayRgn, menuBarRgn, grayRgn);
				CalcVisBehind(WindowPeek(GetWindowList), menuBarRgn);
				DisposeRgn(menuBarRgn);
				DrawMenuBar;
			end;
	end;

{=================================}

end.
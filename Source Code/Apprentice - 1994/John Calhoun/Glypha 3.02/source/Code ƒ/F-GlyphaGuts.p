unit GlyphaGuts;

interface
	uses
		Dialogs, Sound, Enemies, GameUtils;

	procedure DoAStoneHit (whichStone: Rect);
	procedure MoveThePlayer;
	procedure CheckTombStones;
	procedure AnimateTheAnk;
	procedure AdvanceALevel;
	procedure EnterANewMortal;
	procedure DoEnd;
	procedure ExitAMortal;
	procedure SlideTheStones;

implementation

{=================================}

	procedure DoOnTheGround;
		var
			onLand: Boolean;
			index: Integer;
			tempRect, dummyRect: Rect;
	begin
		with thePlayer do
			begin
				if (state) then		{taking off from the ground}
					begin
						DoTheSound('flap.snd', TRUE);
						state := FALSE;
						mode := 5;
						ChangeRect;
						vertVel := liftAmount;
					end
				else							{but if not flapping...}
					begin
						onLand := FALSE;
						tempRect := dest;
						InsetRect(tempRect, 17, 0);
						OffsetRect(tempRect, 0, 2);
						for index := startStone to numberOfStones do
							if (SectRect(tombRects[index], tempRect, dummyRect)) then
								onLand := TRUE;
						if not onLand then
							begin
								mode := 4;
								ChangeRect;
								vertVel := fallAmount;
							end
						else							{on solid ground}
							begin
								if (keyStillDown) then	{and running}
									begin
										horiVel := running[horiVel, facing, 0];
										mode := running[horiVel, facing, 1];
										if ((mode = 0) or (mode = 2)) then
											DoTheSound('walk.snd', TRUE);
									end
								else							{not running, we're braking - screeeech}
									begin
										mode := 1;
										horiVel := idleLanded[horiVel];
										if (horiVel <> 0) then
											DoTheSound('screech.snd', TRUE);
									end;						{end - else, key still down}
							end;							{end - else, not on solid ground}
					end;								{end - else, not flapping to take off}
				dest.left := dest.left + horiVel;
				dest.right := dest.right + horiVel;
				dest.top := dest.top + vertVel;
				dest.bottom := dest.bottom + vertVel;
			end;
	end;

{=================================}

	procedure DoAStoneHit;
	begin
		with thePlayer do
			begin
				if (horiVel > 0) then
					begin
						if (dest.left < whichStone.left) then	{hit the edge}
							begin
								dest.left := dest.left + whichStone.left - whichStone.right;
								dest.right := dest.right + whichStone.left - whichStone.right;
								horiVel := impacted[horiVel];
							end
						else
							begin
								if (vertVel < 0) then							{hit coming up}
									begin
										vertVel := impacted[vertVel];
										OffsetRect(dest, 0, whichStone.bottom - dest.top);
									end
								else
									begin
										if (mode = 6) then
											begin
												DoTheSound('boom1.snd', TRUE);
												OffsetRect(dest, 0, whichStone.top - dest.bottom);
												vertVel := 0;
												horiVel := 0;
												mode := 50;
												ChangeRect;
												Exit(DoAStoneHit);
											end;
										if (vertVel < 4) or (dest.right > whichStone.right) then
											begin
												DoTheSound('screech.snd', TRUE);
												vertVel := -3;
												OffSetRect(dest, 0, whichStone.top - dest.bottom);
											end
										else
											begin
												DoTheSound('walk.snd', TRUE);
												vertVel := 0;
												OffSetRect(dest, 0, whichStone.top - dest.bottom - 10);
												mode := 1;
												ChangeRect;
											end;
									end;
							end;
					end
				else
					begin
						if (dest.right > whichStone.right) then	{hit the edge}
							begin
								OffsetRect(dest, whichStone.right - whichStone.left, 0);
								horiVel := impacted[horiVel];
							end
						else
							begin
								if (vertVel < 0) then		{hit coming up}
									begin
										vertVel := impacted[vertVel];
										OffsetRect(dest, 0, whichStone.bottom - dest.top);
									end
								else
									begin
										if (mode = 6) then
											begin
												DoTheSound('boom1.snd', TRUE);
												OffsetRect(dest, 0, whichStone.top - dest.bottom);
												vertVel := 0;
												horiVel := 0;
												if (mortals <= 1) then
													mode := 7
												else
													mode := 50;
												ChangeRect;
												Exit(DoAStoneHit);
											end;
										if (vertVel < 4) or (dest.left < whichStone.left) then
											begin
												DoTheSound('screech.snd', TRUE);
												vertVel := -3;
												OffSetRect(dest, 0, whichStone.top - dest.bottom);
											end
										else
											begin
												DoTheSound('walk.snd', TRUE);
												vertVel := 0;
												OffSetRect(dest, 0, whichStone.top - dest.bottom - 10);
												mode := 1;
												ChangeRect;
											end;
									end;
							end;
					end;
			end;
	end;

{=================================}

	procedure DoInTheAir;
		var
			index: Integer;
			tempRect, dummyRect: Rect;
	begin
		with thePlayer do
			begin
				if (keyStillDown) then
					horiVel := gliding[horiVel, facing];
				if (state) then
					begin
						DoTheSound('flap.snd', TRUE);
						mode := 5;
						state := FALSE;
						vertVel := vertVel + liftAmount;
					end
				else
					begin
						if (vertVel < maxFall) then
							vertVel := vertVel + fallAmount;
						if (vertVel > 0) then
							mode := 4
						else
							mode := 5;
					end;
				dest.left := dest.left + horiVel;
				dest.right := dest.right + horiVel;
				dest.top := dest.top + vertVel;
				dest.bottom := dest.bottom + vertVel;
				for index := startStone to numberOfStones do
					begin
						if SectRect(dest, tombRects[index], dummyRect) then
							DoAStoneHit(dummyRect);
					end;
			end;
	end;

{=================================}

	procedure DoFallingSkeleton;
		var
			index: Integer;
			dummyRect: Rect;
	begin
		with thePlayer do
			begin
				if (vertVel < maxFall) then
					vertVel := vertVel + fallAmount;
				dest.left := dest.left + horiVel;
				dest.right := dest.right + horiVel;
				dest.top := dest.top + vertVel;
				dest.bottom := dest.bottom + vertVel;
				for index := startStone to numberOfStones do
					begin
						if SectRect(dest, tombRects[index], dummyRect) then
							DoAStoneHit(dummyRect);
					end;
			end;
	end;

{=================================}

	procedure DoDeadBones;
	begin
		with thePlayer do
			begin
				if (mode < 8) then
					begin
						OffsetRect(dest, 0, 1);
						dest.bottom := dest.bottom - 1;
						if (dest.bottom = dest.top) then
							begin
								deadAndGone := TRUE;
								CopyBits(offVirginMap, mainWndo^.portBits, dest, dest, srcCopy, playRgn);
							end;
					end
				else
					mode := mode - 1;
			end;
	end;

{=================================}

	procedure MoveThePlayer;

	begin
		case thePlayer.mode of
			0..3: 
				begin
					DoOnTheGround;
					CheckTombStones;
				end;
			4..5: 
				begin
					DoInTheAir;
					CheckTombStones;
				end;
			6: 
				begin
					DoFallingSkeleton;
					CheckTombStones;
				end;
			7..100: 
				DoDeadBones;
		end;
	end;

{=================================}

	procedure CheckTombStones;
		var
			hori, vert: Integer;
			dummyRect: Rect;
	begin
		with thePlayer do
			begin
				if (not SectRect(dest, playRect, dummyRect)) then	{Player has left the screen	}
					begin
						if (dest.top > playRect.bottom) then
							begin
								if (otherState) then
									begin
										OffsetRect(dest, 0, 1);
										if (dest.top > playRect.bottom + 10) then
											deadAndGone := TRUE;
										Exit(CheckTombStones);
									end
								else
									begin
										DoTheSound('bird.snd', TRUE);
										OffsetRect(dest, 0, playRect.bottom - dest.top + 1);
										otherState := TRUE;
										Exit(CheckTombStones);
									end;
							end
						else
							begin
								if (dest.bottom <= playRect.top) then
									begin
										vertVel := impacted[vertVel];
										OffSetRect(dest, 0, playRect.top - dest.bottom + 1);
									end
								else
									begin
										if (dest.right < playRect.left) then
											begin
												CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, playRgn);
												OffsetRect(dest, 511, 0);
												oldDest := dest;
											end
										else
											begin
												if (dest.left > playRect.right) then
													begin
														CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, playRgn);
														OffsetRect(dest, -511, 0);
														oldDest := dest;
													end;
											end;
									end;
							end;
					end;
			end;
	end;

{=================================}

	procedure AnimateTheAnk;
		var
			index, index2, howHigh: Integer;
			dummyLong: LongInt;
			newRect, tempRect: Rect;
			theEvent: EventRecord;
	begin
		DoTheSound('lightning.snd', TRUE);
		if ((numberOfStones > 4) and (DoRandom(2) = 0) and (not stonesSliding)) then
			begin
				SetRect(newRect, 247, 121, 263, 145);
				howHigh := upperLevel;
				OffsetRect(thePlayer.dest, 0, -172);
				thePlayer.oldDest := thePlayer.dest;
			end
		else
			begin
				SetRect(newRect, 247, 292, 263, 316);
				howHigh := lowerLevel;
			end;
		CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, ankRects[0, source], ankRects[0, mask], newRect);
		StrikeLightning(howHigh);
		StrikeLightning(howHigh);
		StrikeLightning(howHigh);

		tempRect := thePlayer.dest;
		FlushEvents(everyEvent, 0);
		lastLoopTime := TickCount;

		for index := 0 to 60 do		{Here's where the player flashes in get-ready mode	}
			begin
				DoTheSound('rez.snd', TRUE);
				CopyBits(offVirginMap, offLoadMap, tempRect, tempRect, srcCopy, nil);
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, ankRects[0, source], ankRects[0, mask], newRect);
				CopyBits(offLoadMap, mainWndo^.portBits, tempRect, tempRect, srcCopy, nil);
				if (stonesSliding) then
					SlideTheStones;
				UpdateEye;
				HandleTheEnemies;
				DrawBeasts;
				CopyBits(offVirginMap, offLoadMap, newRect, newRect, srcCopy, nil);
				CopyMask(offPlayerMap, offPlayerMap, offLoadMap, playerRects[0, 1], playerRects[2, 1], tempRect);
				CopyBits(offLoadMap, mainWndo^.portBits, tempRect, tempRect, srcCopy, nil);
				if (EventAvail(playMask, theEvent) or Button) then
					begin
						if ((theEvent.what = MouseDown) or (theEvent.what = keyDown) or Button) then
							begin
								thePlayer.otherState := FALSE;
								Exit(AnimateTheAnk);
							end;
					end;
				if (stonesSliding) then
					SlideTheStones;
				UpdateEye;
				HandleTheEnemies;
				if (theHand.state) then
					RetractTheHand;
				DrawBeasts;
			end;
	end;

{=================================}

	procedure AdvanceALevel;
		var
			index, aNumber, anotherNumber: Integer;
	begin
		if (score > 0) then						{Compute the time bonus (if any) to the score.	}
			aNumber := 2000 - gameCycle		{If they cleared it in less than 2000 cycles		}
		else
			aNumber := 0;
		if (aNumber < 0) then					{But don't SUBTRACT from their score, even	}
			aNumber := 0;							{if they took forever.  Just add zero.}
		score := score + aNumber;				{Add it...												}
		CheckExtraMortal;
		oldScore := Score;							{Keep a hold on this score.				}
		levelOn := levelOn + 1;

		if (levelOn = 3) then
			stonesSliding := TRUE
		else
			stonesSliding := FALSE;
		DoTheHoopla;

		with theEye do
			begin
				otherMode := 0;
				mode := (levelOn * 10) - 1500;
				if (mode > -100) then
					mode := -100;
				dest := eyeRects[4];
				OffsetRect(dest, -43, 70);
				if (DoRandom(2) = 0) then
					OffsetRect(dest, 0, 100);
				oldDest := dest;
			end;

		beastsActive := 0;
		beastsKilled := 0;

		if ((levelOn div 5) = (levelOn / 5)) then
			begin								{Every fifth wave is an egg wave!!}
				if (onFastMachine) then
					begin
						totalToKill := 8;				{In "slow mode" we assume they are on a MacII	}
						numberOfEnemies := 8;		{so we give them nine eggs to clear.					}
						beastsActive := 8;
					end
				else
					begin
						totalToKill := 6;				{In fast mode we only give them 6.  Nine would	}
						numberOfEnemies := 6;		{be slow the machine down too much.				}
						beastsActive := 6;
					end;
				for index := 1 to numberOfEnemies do
					with theEnemies[index] do
						begin
							mode := -DoRandom(300) + levelOn;
							if (mode > -100) then
								mode := DoRandom(50) - 150;
							otherState := TRUE;
							facing := DoRandom(2);
							aNumber := DoRandom(2);					{Now we figure out where the sphinx is	}
							anotherNumber := DoRandom(100) + 10;	{to appear.  What platform, what side.	}
							if (facing = 0) then
								begin
									if aNumber = 0 then
										SetRect(dest, anotherNumber, 74, 22 + anotherNumber, 96)
									else
										SetRect(dest, anotherNumber, 190, 22 + anotherNumber, 212);
								end
							else
								begin
									if aNumber = 0 then
										SetRect(dest, 400 + anotherNumber, 74, 422 + anotherNumber, 96)
									else
										SetRect(dest, 400 + anotherNumber, 190, 422 + anotherNumber, 212);
								end;
							oldDest := dest;
						end;
			end
		else
			begin			{a normal 'non-egg' wave}
				totalToKill := levelOn div 5 + 4;
				numberOfEnemies := levelOn div 5 + 1;
				if ((numberOfEnemies > 3) and (not onFastMachine)) then
					numberOfEnemies := 3
				else if (numberOfEnemies > 5) then
					numberOfEnemies := 5;
				for index := 1 to numberOfEnemies do
					with theEnemies[index] do
						begin
							mode := 150 + DoRandom(100);
							DoEnemyPlacement(index);
						end;
			end;

		for index := 1 to numberOfEnemies do
			with theEnemies[index] do
				begin
					state := FALSE;
					aNumber := DoRandom(levelOn);	{What type of sphinx?  O, 1 or 2?  Well, on level 1,	}
					if (aNumber > 20) then				{there is a 0% chance that a random # between 0 &	}
						otherMode := 2							{0 will be greater than 20...so, no tough sphinx.		}
					else													{But on level 30, aNumber will be a random number	}
						begin												{between 0 and 29.  That gives us almost a 33%		}
							if (aNumber > 10) then		{chance that it will be a sphinx 2, if not a sphinx 2,	}
								otherMode := 1					{(not >20) then a good 50% chance that aNumber>10	}
							else											{(11 to 20) and thus a sphinx 1.  Note: there is		}
								otherMode := 0;					{ALWAYS a CHANCE of the easiest sphinx 0, but it	}
						end;												{gets increasingly unlikely as the player goes on.		}
					vertVel := 0;
					horiVel := 0;
				end;

		with theHand do
			begin
				dest := handRects[source, 0];
				OffsetRect(dest, 100, 270);
				oldDest := dest;
				state := FALSE;
				onward := FALSE;
				mode := 0;
			end;

		growRate := levelOn div 3 + 1;
		if (growRate > 8) then
			growRate := 8;
		numberOfStones := 2;
		startStone := 0;
		if (levelOn < 3) then
			tombRects[0] := tombRects[-1]
		else
			tombRects[0] := tombRects[-2];

		if (levelOn = 3) then
			begin
				tombRects[5] := tombRects[-4];
				numberOfStones := numberOfStones + 2;
			end
		else
			tombRects[5] := tombRects[-3];

		if (((levelOn + 4) / 5) = ((levelOn + 4) div 5)) then
			numberOfStones := numberOfStones + 3;
		if ((levelOn / 5) = (levelOn div 5)) then
			numberOfStones := numberOfStones + 3;
		if (((levelOn + 3) / 5) = ((levelOn + 3) div 5)) then
			numberOfStones := numberOfStones + 2;
		if (((levelOn + 2) / 5) = ((levelOn + 2) div 5)) then
			numberOfStones := numberOfStones + 2;
		RedoTheBackground;
		gameCycle := 0;
		FlushEvents(everyEvent, 0);
	end;

{=================================}

	procedure EnterANewMortal;
	begin
		with thePlayer do
			begin
				facing := 0;
				mode := 0;
				horiVel := 0;
				vertVel := 0;
				dest := absoluteRects[facing, mode];
				OffSetRect(dest, 238, 267);
				oldDest := dest;
				if (theEye.mode > 0) then
					theEye.mode := 272;
				state := FALSE;
			end;
		keyStillDown := FALSE;
		playing := TRUE;
		pausing := FALSE;
		deadAndGone := FALSE;
		AnimateTheAnk;
	end;

{===================================}

	procedure DoEnd;
		var
			index: Integer;
			dummyLong: LongInt;
			tempRect: Rect;
	begin
		DoTheSound('music.snd', FALSE);					{Play the theme song, but wait for it to finish	}
		playing := FALSE;
		pausing := FALSE;
		RedoTheBackground;
		ShowScore;
		tempRect := gameoverRects[0];
		OffsetRect(tempRect, -105, -100);
		for index := 1 to 5 do
			begin
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, gameoverRects[1], gameoverRects[1], tempRect);
				Delay(4, dummyLong);
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, gameoverRects[0], gameoverRects[1], tempRect);
				Delay(4, dummyLong);
			end;

		with theEye do
			begin
				dest := eyeRects[4];
				OffsetRect(dest, -43, 70);
				oldDest := dest;
			end;

		Delay(120, dummyLong);

		InitCursor;
		FinalScore;
		RedoTheBackground;
		SetEventMask(EveryEvent);
		RedrawHiScores;
		if (downOffset < 20) then
			begin
				InsertMenu(AppleMenu, 0);		{Insert this menu into the menu bar}
				InsertMenu(GameMenu, 0);		{Insert this menu into the menu bar}
				InsertMenu(OptionsMenu, 0);	{Insert this menu into the menu bar}
				DrawMenuBar;					{Draw the menu bar}
				FlashMenuBar(0);
			end;
	end;

{=================================}

	procedure ExitAMortal;
		var
			aNumber: Integer;
	begin
		mortals := mortals - 1;
		UnionRect(thePlayer.oldDest, thePlayer.dest, thePlayer.wholeRect);
		CopyBits(offVirginMap, mainWndo^.portBits, thePlayer.wholeRect, thePlayer.wholeRect, srcCopy, playRgn);
		if (mortals <= 0) then							{Game over, dude}
			begin
				DoEnd;
			end
		else
			begin
				EnterANewMortal;
			end;
	end;

{=================================}

	procedure SlideTheStones;
		var
			oldStone1, oldStone2, newStone1, newStone2, stoneSrc: Rect;
	begin
		SetRect(stoneSrc, 166, 316, 346, 332);
		oldStone1 := stoneSrc;
		oldStone2 := stoneSrc;
		OffsetRect(tombRects[5], 1, 0);
		OffsetRect(tombRects[6], -1, 0);
		OffsetRect(oldStone1, tombRects[5].left - stoneSrc.left, 0);
		OffsetRect(oldStone2, tombRects[6].right - stoneSrc.right, 0);
		newStone1 := oldStone1;
		newStone2 := oldStone2;
		oldStone1.left := oldStone1.left - 1;
		oldStone2.right := oldStone2.right + 1;

		CopyBits(offVirginMap, offLoadMap, oldStone1, oldStone1, srcCopy, nil);
		CopyBits(offVirginMap, offLoadMap, oldStone2, oldStone2, srcCopy, nil);
		CopyBits(offVirginMap, offLoadMap, stoneSrc, newStone1, srcCopy, nil);
		CopyBits(offVirginMap, offLoadMap, stoneSrc, newStone2, srcCopy, nil);
		CopyBits(offLoadMap, mainWndo^.portBits, oldStone1, oldStone1, srcCopy, playRgn);
		CopyBits(offLoadMap, mainWndo^.portBits, oldStone2, oldStone2, srcCopy, playRgn);

		if (oldStone2.right < 0) then
			begin
				tombRects[5] := tombRects[-3];
				tombRects[6] := tombRects[-5];
				numberOfStones := numberOfStones - 2;
				stonesSliding := FALSE;
			end;
	end;

{=================================}

end.
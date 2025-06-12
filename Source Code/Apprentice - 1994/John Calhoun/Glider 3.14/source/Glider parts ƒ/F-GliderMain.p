unit GliderMain;

interface
	uses
		Dialogs, Sound, GameUtilities;

	procedure HandleCandle;
	procedure ComputeGliderPos;
	procedure DrawGlider;
	procedure DoGliderBitesIt;
	procedure DotheHoopla;
	procedure GetSet;
	procedure WrapUpaGame;
	procedure DoFinalScore;
	procedure WelcomeToNextLevel;
	procedure DetermineObstacle (whasUp: Integer);
	procedure ComputeStatus;
	procedure DoDyingDisplay;
	procedure DoCopter;
	procedure DoDrip;
	procedure DoCat;

implementation

	var
		tempRect: Rect;				{Temporary rectangle	}
		Index: Integer;				{For looping				}
		CtrlHandle: controlhandle;	{Control handle			}

{=================================}

	procedure HandleCandle;
		var
			index: Integer;
	begin
		for index := 1 to numberOfCandles do
			begin
				candleInfo[index, 2] := candleInfo[index, 2] + 1;	{Increment the "mode"}
				case candleInfo[index, 2] of							{And depending on the mode, we'll		}
					0..2, 4: 													{either idle this pass through the loop...	}
						begin
						end;
					3: 															{or we'll draw a new flame state!		}
						begin
							DrawAnObject(7, candleInfo[index, 0], candleInfo[index, 1])
						end;
					5: 
						begin
							DrawAnObject(14, candleInfo[index, 0], candleInfo[index, 1])
						end;
					otherwise
						candleInfo[index, 2] := -1;
				end;	{case candleInfo[index, 2] of}
			end;
	end;

{=================================}

	procedure ComputeGliderPos;
	begin
		if (stalling) then
			begin
				moveHori := stall;		{Simple. }
			end
		else
			begin
				moveHori := thrust;
			end;
		moveVert := gravity;
	end;

{=================================}

	procedure DrawGlider;
		var
			mode, shadoMode: Integer;
			tempRect, temp2Rect: Rect;
	begin
		if (gliderCraft) then			{Here is where we do the flicker-free animation.	}
			begin							{First we go through some bogus conditionals to	}
				shadoMode := 53;				{get the proper objects from the objectMap...		}
				if (stalling) then
					mode := 1
				else
					mode := 0;
				if (burning) then
					mode := 37 + burnMode;
				if (crushed) then
					mode := 35;
			end
		else
			begin
				mode := 2;
				shadoMode := 54;
				if (burning) then
					mode := 56 + burnMode;
				if (crushed) then
					mode := 55;
			end;
{Step 1, find the smallest rect that encloses both the new position of the shadow and the old.	}
		UnionRect(shadowDropRect, oldShadowRect, tempRect);
{Step 2, Copy that swatch of background from the virginMap to the loadMap for assembly.	}
		CopyBits(virginMap, loadMap, tempRect, tempRect, srcCopy, nil);
{But before we do, let's do Step 1 and 2 for the glider as well.  This way there's no way we	}
		UnionRect(playerDropRect, oldPlayerRect, temp2Rect);
{will cover up the shadow with the background behind the glider (during those LOW glides)		}
		CopyBits(virginMap, loadMap, temp2Rect, temp2Rect, srcCopy, nil);
{Step 3, Mask the shadow from the objectMap to it's swatch of background on the loadMap	}
		CopyMask(objectsMap, objectsMap, loadMap, objectsRects[shadoMode, 0], objectsRects[shadoMode, 1], shadowDropRect);
{Ditto for the glider.  Now both shadow & glider are assembled and ready for visibility.		}
		CopyMask(objectsMap, objectsMap, loadMap, objectsRects[mode, 0], objectsRects[mode, 1], playerDropRect);
{Step 4, grab the whole assembled shadow (with surrounding bckgrnd) and dump to on screen.}
		CopyBits(loadMap, mainWindow^.portBits, tempRect, tempRect, srcCopy, mainWindow^.visRgn);
{Ditto for the glider.  Only these last 2 steps are seen by the player.  Flickerless movement!	}
		CopyBits(loadMap, mainWindow^.portBits, temp2Rect, temp2Rect, srcCopy, mainWindow^.visRgn);
	end;

{=================================}

	procedure DoGliderBitesIt;
		var
			Index: Integer;
			tempRect: Rect;
	begin
		DoTheSound('boom.snd', highPriority);
		gliderNumber := gliderNumber - 1;			{Subtract 1 from number of gliders player has.	}
		SetPort(mainWindow);
		for Index := 1 to 30 do
			begin
				SetRect(tempRect, 24 + playerDropRect.left - Index * 2, 11 + playerDropRect.top - Index * 2, 24 + playerDropRect.left + Index * 2, 11 + playerDropRect.top + Index * 2);
				InvertOval(tempRect);
			end;
		for Index := 1 to 30 do
			begin
				SetRect(tempRect, 24 + playerDropRect.left - Index * 2, 11 + playerDropRect.top - Index * 2, 24 + playerDropRect.left + Index * 2, 11 + playerDropRect.top + Index * 2);
				InvertOval(tempRect);
			end;
		if (gliderNumber < 1) then						{i.e. "If the player is out of gliders then...."		}
			begin
				DoFinalScore;									{Show the player the final score, etc.				}
			end
		else												{Otherwise... we're dead but not finished...the		}
			begin											{player has more gliders still.						}
				Stalling := FALSE;
				ResetGliderPos;
				FoldTheGlider;
				GetSet;											{Now call on GetSet to do the rest (not once-only)	}
			end;
	end;

{=================================}

	procedure DotheHoopla;						{If the player makes it through the last room,	}
		var											{we call this little animation sequence.			}
			index, index2, mode: Integer;				{You can examine it if you like, but it's just	}
			dummyRect: Rect;							{more of the same tedious rect manipulation.	}
			gliderRects: array[1..30, 0..1] of Rect;
	begin
		for index := 1 to 30 do
			begin
				if (gliderCraft) then
					begin
						SetRect(gliderRects[index, 0], 0, 0, 46, 19);
						mode := 0;
					end
				else
					begin
						SetRect(gliderRects[index, 0], 0, 0, 65, 22);
						mode := 2;
					end;
				OffsetRect(gliderRects[index, 0], DoRandom(1000) - 1000, DoRandom(260) + 20);
				gliderRects[index, 1] := gliderRects[index, 0];
			end;
		PenNormal;
		PenMode(patXOr);
		SetPort(virginPort);
		FillRect(virginPort^.portRect, black);
		for index := 1 to 1000 do
			begin
				MoveTo(DoRandom(512), DoRandom(320));
				Line(0, 0);
			end;
		SetPort(mainWindow);
		FillRect(mainWindow^.portRect, black);
		TextFont(0);						{Set font to Chicago (everyone has this font)		}
		TextSize(12);					{Make it 12 point in size so that it isn't cruddy	}
		TextMode(srcXOr);
		MoveTo(300, 293);
		DrawString('At last you are free!');
		for index2 := 1 to 160 do
			begin
				for index := 1 to 30 do
					begin
						OffsetRect(gliderRects[index, 0], 10, -1);
						UnionRect(gliderRects[index, 0], gliderRects[index, 1], dummyRect);
						CopyBits(virginMap, loadMap, dummyRect, dummyRect, srcCopy, nil);
						CopyMask(objectsMap, objectsMap, loadMap, objectsRects[mode, 0], objectsRects[mode, 1], gliderRects[index, 0]);
					end;
				for index := 1 to 30 do
					begin
						UnionRect(gliderRects[index, 0], gliderRects[index, 1], dummyRect);
						CopyBits(loadMap, mainWindow^.portBits, dummyRect, dummyRect, srcCopy, mainWindow^.visRgn);
						gliderRects[index, 1] := gliderRects[index, 0];
					end;
			end;
	end;

{=================================}

	procedure GetSet;
		var
			index: Integer;
			dummyLong: LongInt;
	begin
		extraGlider := FALSE;
		extraTime := FALSE;
		darkOn := FALSE;
		switchUntouched := TRUE;
		Stalling := FALSE;
		burning := FALSE;
		crushed := FALSE;
		moveHori := 0;
		moveVert := 0;
		gameTime := 0;
		dripMode := 0;
		dripCycle := 23;
		catMode := 0;
		numberOfCopters := (levelOn div 10) + 1;
		if (levelOn > 13) then
			numberOfCopters := 1;
		for index := 1 to numberOfCopters do
			copterMode[index, 0] := DoRandom((18 - levelOn) * 8) + (18 - levelOn) * 6;
		SetRect(vitalArea, playerDropRect.left + 5, playerDropRect.top + 10, playerDropRect.left + 43, playerDropRect.top + 15);
		UpDate_GliderMain;
		DrawGlider;
		Delay(90, dummyLong);
	end;

{=================================}

	procedure WrapUpaGame;
		var
			index, ranking: Integer;
			theOverflow: LongInt;
			tempString, fixedScore, padding: Str255;
	begin
		FlushEvents(everyEvent, 0);									{Clear all the keystrokes	}
		if (theScore > hiScores[10]) then		{Is it a high score?			}
			begin
				scoresChanged := TRUE;										{Mark scores as changed	}
				DoTheSound('bonus.snd', highPriority);							{Play the bonus sound.		}
				ranking := 10;												{And sort through the high	}
				for index := 9 downto 1 do								{scores to find the players	}
					begin														{ranking.						}
						if (theScore > hiScores[index]) then
							ranking := index;
					end;
				for index := 10 downto ranking + 1 do
					begin
						hiScores[index] := hiScores[index - 1];				{Move everyone's score		}
						hiStrings[index] := hiStrings[index - 1];				{down the list.				}
					end;
				hiScores[ranking] := theScore;
				WhosHiScore(nameUsing);
				hiStrings[ranking] := nameUsing;
				if (theScore > 999999) then
					begin
						theOverflow := (theScore div 1000000) * 1000000;
						theScore := theScore - theOverflow;
					end;
			end;
		playing := FALSE;
		switchUntouched := TRUE;
		darkOn := FALSE;
		burning := FALSE;
		crushed := FALSE;
		stalling := FALSE;
		for index := 1 to numberOfCopters do
			SetRect(coptersRects[index, 0], 0, 0, 0, 0);
		ResetGliderPos;
		levelOn := levelBegin;
		Update_GliderMain;
		EnableItem(GetMenu(L_Game), 1);
		EnableItem(GetMenu(L_Game), 2);
		DisableItem(GetMenu(L_Game), 4);
		DisableItem(GetMenu(L_Game), 5);
		EnableItem(GetMenu(L_Options), 0);
		SetItem(GetMenu(l_Game), 4, 'Pause');
		SetPort(oldPort);
		RedrawHiScores;
	end;

{=================================}

	procedure DoFinalScore;
		var
			index: Integer;
			dummyLong: LongInt;
			dummyRect, dummyRect2, dummyRect3: Rect;
			dummyString, dummyString2: Str255;
	begin
		SetPort(loadPort);
		EraseRect(loadMap.bounds);
		SetRect(dummyRect, 0, 0, 0, 0);
		ResizeARect(dummyRect, 34);
		CopyBits(objectsMap, loadMap, objectsRects[34, 0], dummyRect, srcCopy, nil);
		PenNormal;
		TextFont(0);
		TextSize(12);
		MoveTo(60, 15);
		if levelOn > maxLevel then
			begin
				DoTheSound('bonus.snd', highPriority);
				dummyString := 'You made it!  Final score ';
				theScore := theScore + 1000 * gliderNumber;
			end
		else
			begin
				dummyString := 'Final score ';
			end;
		NumToString(theScore, dummyString2);
		dummyString := CONCAT(dummyString, dummyString2);
		DrawString(dummyString);
		dummyRect2 := dummyRect;
		OffsetRect(dummyRect2, 512, 290);
		dummyRect3 := dummyRect2;
		dummyRect2.right := dummyRect2.right + 10;
		for index := 135 downto 0 do
			begin
				OffsetRect(dummyRect2, -3, 0);
				OffsetRect(dummyRect3, -3, 0);
				CopyBits(virginMap, loadMap, dummyRect2, dummyRect2, srcCopy, nil);
				CopyMask(loadMap, objectsMap, loadMap, dummyRect, objectsRects[34, 1], dummyRect3);
				CopyBits(loadMap, mainWindow^.portBits, dummyRect2, dummyRect2, srcCopy, mainWindow^.visRgn);
			end;
		PenNormal;
		Delay(120, dummyLong);
		WrapUpaGame;
	end;

{=================================}

	procedure WelcomeToNextLevel;
		var
			dummyRect, dummyRect2, dummyRect3: Rect;
			index: Integer;
			dummyLong, now: LongInt;
			dummyString, dummyString2: Str255;
	begin
		if ((levelOn = 6) or (levelOn = 11)) then		{Hang on to score in these rooms in case	}
			oldScore := theScore;							{player gets sucked back to these rooms.	}
		theScore := theScore + (1000 * levelOn) + (300000 div gameTime);		{Here's the score formula	}
		if extraGlider then
			gliderNumber := gliderNumber + 1;
		levelOn := levelOn + 1;
		if LevelOn > maxLevel then
			begin
				DotheHoopla;
				DoFinalScore;
			end
		else
			begin
				SetPort(loadPort);
				EraseRect(loadMap.bounds);
				SetRect(dummyRect, 0, 0, 0, 0);
				ResizeARect(dummyRect, 34);
				CopyBits(objectsMap, loadMap, objectsRects[34, 0], dummyRect, srcCopy, nil);
				PenNormal;
				TextFont(0);
				TextSize(12);
				MoveTo(60, 15);
				dummyString := 'Room ';
				NumToString(levelOn, dummyString2);
				dummyString := CONCAT(dummyString, dummyString2, '   Gliders= ');
				NumToString(gliderNumber, dummyString2);
				dummyString := CONCAT(dummyString, dummyString2, '   Score= ');
				NumToString(theScore, dummyString2);
				dummyString := CONCAT(dummyString, dummyString2);
				DrawString(dummyString);
				dummyRect2 := dummyRect;
				OffsetRect(dummyRect2, 512, 290);
				dummyRect3 := dummyRect2;
				dummyRect2.right := dummyRect2.right + 10;
				now := TickCount + 1;
				for index := 340 downto 0 do
					begin
						now := TickCount + 1;
						OffsetRect(dummyRect2, -3, 0);
						OffsetRect(dummyRect3, -3, 0);
						CopyBits(virginMap, loadMap, dummyRect2, dummyRect2, srcCopy, nil);
						CopyMask(loadMap, objectsMap, loadMap, dummyRect, objectsRects[34, 1], dummyRect3);
						CopyBits(loadMap, mainWindow^.portBits, dummyRect2, dummyRect2, srcCopy, mainWindow^.visRgn);
						if ((index mod 2) = 0) then
							begin
								repeat
								until (TickCount >= now);
								now := TickCount + 1;
							end;
					end;
				OffsetRect(playerDropRect, -playerDropRect.left + startHori, 0);
				OffsetRect(shadowDropRect, -shadowDropRect.left + startHori, 0);
				oldPlayerRect := playerDropRect;
				oldShadowRect := shadowDropRect;
				GetSet;
			end;   {end of else}
	end;

{=================================}

	procedure DetermineObstacle;
		var
			Index: Integer;
			dummyLong: LongInt;
	begin
		case objectArray[whasUp] of
			1:                                                {lift the glider <floor vent>}
				begin
					moveVert := moveVert + ventLift;
				end;
			2:                                                {death to the glider <flames>}
				begin
					if (not burning) then
						begin
							DoTheSound('flash.snd', highPriority);
							burning := TRUE;
							stalling := TRUE;
							timeOfDeath := gameTime;
							if (gliderCraft) then
								ResizeARect(playerDropRect, 35)
							else
								ResizeARect(playerDropRect, 55);
						end;
				end;
			6:                                                 {air blows the glider down to the floor}
				begin
					moveVert := moveVert + ductGravity;
				end;
			7:                                                 {the glider has been sucked back to another room}
				begin
					stalling := TRUE;
					case levelOn of
						1..5: 
							begin
								levelOn := 1;
								theScore := 0;
							end;
						6..10: 
							begin
								levelOn := 6;
								theScore := oldScore;
							end;
						11..15: 
							begin
								levelOn := 11;
								theScore := oldScore;
							end;
						otherwise
							begin
							end;
					end;
					DoTheSound('ductsuck.snd', highPriority);
					Delay(40, dummyLong);
					ResetGliderPos;
					GetSet;
					DoTheSound('ductdump.snd', highPriority);
				end;
			8:                                                 {the glider has succeeded in a time bonus <clock>}
				begin
					if (not extraTime) and (not burning) and (not crushed) then
						begin
							DoTheSound('bonus.snd', highPriority);
							extraTime := TRUE;
							gameTime := 0;
							SetPort(MainWindow);
							for Index := 1 to 12 do
								begin
									InvertRect(eventArray[whasUp]);
									Delay(2, dummyLong);
								end;
						end;
				end;
			9:                                                  {the glider gets another 'ship' <folded piece of paper>}
				begin
					if (not extraGlider) then
						begin
							DoTheSound('bonus.snd', highPriority);
							extraGlider := TRUE;
							SetPort(MainWindow);
							for Index := 1 to 12 do
								begin
									InvertRect(eventArray[whasUp]);
									Delay(2, dummyLong);
								end;
						end;
				end;
			10:                                               {excellent - the air is turned on <thermostat>}
				begin
					if (ventLift = 0) then
						begin
							DoTheSound('bonus.snd', highPriority);
							ventLift := floorFast;
							SetPort(MainWindow);
							for Index := 1 to 12 do
								begin
									InvertRect(eventArray[whasUp]);
									Delay(2, dummyLong);
								end;
						end;
				end;
			11: 
				begin
					if (darkOn) and (switchUntouched) then
						begin
							DoTheSound('bonus.snd', highPriority);
							darkOn := FALSE;
							switchUntouched := FALSE;
							SetPort(MainWindow);
							for Index := 1 to 12 do
								begin
									InvertRect(eventArray[whasUp]);
									Delay(2, dummyLong);
								end;
							Update_GliderMain;
						end;
				end;
			12: 		{glider has landed upon a surface like a shelf or table - now display crushed glider	}
				begin
					if (not burning) and (not crushed) then
						begin
							DoTheSound('crunch.snd', highPriority);
							crushed := TRUE;
							stalling := TRUE;
							timeOfDeath := gameTime;
							if (gliderCraft) then
								ResizeARect(playerDropRect, 35)
							else
								ResizeARect(playerDropRect, 55);
						end;
				end;
			otherwise
				begin
				end;
		end;  {case}
	end;

{=================================}

	procedure ComputeStatus;
		var
			Index: Integer;
			dummy: Rect;
	begin
		for Index := 1 to totalObjects do
			begin
				if SectRect(vitalArea, eventArray[Index], dummy) then
					DetermineObstacle(Index);
			end;
		if (not SectRect(vitalArea, playArea, dummy)) then        {heading out-of bounds}
			begin
				if (playerDropRect.top > 280) and (not crushed) and (not burning) then
					begin
						DoTheSound('crunch.snd', highPriority);
						crushed := TRUE;
						stalling := TRUE;
						timeOfDeath := gameTime;
						if (gliderCraft) then
							ResizeARect(playerDropRect, 35)
						else
							ResizeARect(playerDropRect, 55);
					end;
				if (playerDropRect.top < 10) then
					begin
						moveVert := -ventLift;
					end;
				if (playerDropRect.left < -17) then
					begin
						moveHori := thrust;
					end;
				if (playerDropRect.left > 510) then
					begin
						WelcomeToNextLevel;
					end;
			end;  {end of if (not SetRect...}
	end;

{=================================}

	procedure DoDyingDisplay;
	begin
		if (playerDropRect.bottom > 295) then
			DoGliderBitesIt
		else
			begin
				if (burning) then
					begin
						burnMode := DoRandom(2);
						if (gliderCraft) then
							ResizeARect(playerDropRect, burnMode + 37)
						else
							ResizeARect(playerDropRect, burnMode + 56);
						if (playerDropRect.top > 275) then
							begin
								OffsetRect(playerDropRect, 0, 276 - playerDropRect.top);
							end;
					end		{end-if burning}
				else
					begin
						if (playerDropRect.bottom < 300) then
							OffsetRect(playerDropRect, 0, gravity * 2);
					end;
			end;
	end;

{=================================}

	procedure DoCopter;
		var
			overlapping: Boolean;
			index, index2, hori: Integer;
			dummyRect: Rect;
	begin
		for index := 1 to numberOfCopters do
			begin
				case copterMode[index, 0] of
					0: 
						begin
						end;
					1: 		{in mode=1 the helicopter twirls down until it's off screen or hit the player}
						begin
							copterMode[index, 1] := copterMode[index, 1] + 1;
							if (copterMode[index, 1] > 8) then
								copterMode[index, 1] := 1;
							OffsetRect(coptersRects[index, 0], -2 * advanceRate, gravity);
							UnionRect(coptersRects[index, 0], coptersRects[index, 1], dummyRect);
							CopyBits(virginMap, loadMap, dummyRect, dummyRect, srcCopy, nil);
							CopyMask(objectsMap, objectsMap, loadMap, objectsRects[copterMode[index, 1] + 14, 0], objectsRects[copterMode[index, 1] + 14, 1], coptersRects[index, 0]);
							CopyBits(loadMap, mainWindow^.portBits, dummyRect, dummyRect, srcCopy, mainWindow^.visRgn);
							coptersRects[index, 1] := coptersRects[index, 0];	{set "old rect" to current rect}
							if (SectRect(coptersRects[index, 0], vitalArea, dummyRect) and (not crushed) and (not burning)) then
								begin
									DoTheSound('boom.snd', highPriority);
									crushed := TRUE;
									stalling := TRUE;
									timeOfDeath := gameTime;
									if (gliderCraft) then
										ResizeARect(playerDropRect, 35)
									else
										ResizeARect(playerDropRect, 55);
								end;
							if (coptersRects[index, 0].right < 0) then	{i.e. if the copter has gone off the left..	}
								copterMode[index, 0] := DoRandom((18 - levelOn) * 4) + 10;
						end;
					2: 		{when the copter reaches mode=2, then it has burst on the scene from above}
						begin
							SetRect(coptersRects[index, 0], 0, 0, 0, 0);
							ResizeARect(coptersRects[index, 0], 15);
							repeat
								overlapping := FALSE;
								hori := DoRandom(300) + 220;
								OffsetRect(coptersRects[index, 0], hori, -36);
								for index2 := 1 to numberOfCopters do
									begin
										if (index2 <> index) then
											if SectRect(coptersRects[index, 0], coptersRects[index2, 0], dummyRect) then
												overlapping := TRUE;
									end;
							until (not overlapping);
							coptersRects[index, 1] := coptersRects[index, 0];	{set "old rect" to current rect}
							copterMode[index, 0] := 1;		{put copter in mode 1 (see above)}
							copterMode[index, 1] := 1;		{the actual cycle in it's spin is 1}
						end;
					otherwise	{the copter begins at an arbitrary and high mode number, each game cycle}
						begin		{this gets decremented until it reaches mode=2, then it appears}
							copterMode[index, 0] := copterMode[index, 0] - 1;
						end;
				end;		{case copterMode[index, 0] of}
			end;
	end;

{=================================}

	procedure DoDrip;
		var
			dummyRect: Rect;
	begin
		case dripMode of
			0:				{In this mode, the drip has changed appearance and needs to be drawn				}
				begin
					SetRect(dripDropRect, 0, 0, 0, 0);		{First, reset the rectangle for the drip				}
					ResizeARect(dripDropRect, dripCycle);	{Resize it according to which drip is to be drawn	}
					OffsetRect(dripDropRect, 350, 5);		{Move it so that it resides on the stained ceiling	}
					CopyBits(virginMap, loadMap, dripDropRect, dripDropRect, srcCopy, nil);	{Draw it		}
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[dripCycle, 0], objectsRects[dripCycle, 1], dripDropRect);
					CopyBits(loadMap, mainWindow^.portBits, dripDropRect, dripDropRect, srcCopy, nil);
					dripMode := 1;								{Advance the drip mode to 1 for next game pass	}
				end;
			1..18:		{The drip is idling while drop builds up.  It provides a time delay					}
				begin
					dripMode := dripMode + advanceRate;	{All we do is advance the mode until it reaches	}
				end;											{19 or 20 (advanceRate is 2 for fast mode).		}
			19..20: 											{We're through idling.  Time to advance the drip.	}
				begin
					dripCycle := dripCycle + 1;				{Increment the cycle of the drip (it's swelling).	}
					if (dripCycle = 25) then					{If the cycle is 25, then the drop has formed		}
						begin										{We need to retract the drip first and draw it.		}
							CopyBits(virginMap, loadMap, dripDropRect, dripDropRect, srcCopy, nil);
							CopyMask(objectsMap, objectsMap, loadMap, objectsRects[25, 0], objectsRects[25, 1], dripDropRect);
							CopyBits(loadMap, mainWindow^.portBits, dripDropRect, dripDropRect, srcCopy, nil);
							dripMode := 21;							{And then advance the drip mode to 21.				}
						end
					else											{If the drop has not fallen, then we simply have a	}
						dripMode := 0;								{drip that has swelled.  We need to draw it.			}
				end;											{Going back to mode=0 will draw the new drip		}
			21: 		{In this mode, the drop has freed itself and needs to be set up.							}
				begin
					CopyBits(virginMap, loadMap, dripDropRect, dripDropRect, srcCopy, nil);
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[26, 0], objectsRects[26, 1], dripDropRect);
					CopyBits(loadMap, mainWindow^.portBits, dripDropRect, dripDropRect, srcCopy, nil);
					SetRect(dripDropRect, 0, 0, 0, 0);		{We reset the bounding rectangle for the newly	}
					ResizeARect(dripDropRect, 27);			{freed water drop.										}
					OffsetRect(dripDropRect, 358, 20);		{Then we move it a little down from the ceiling.	}
					oldDripRect := dripDropRect;				{The old Rect initially where the current drip is.	}
					CopyBits(virginMap, loadMap, dripDropRect, dripDropRect, srcCopy, nil);	{And draw	}
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[27, 0], objectsRects[27, 1], dripDropRect);
					CopyBits(loadMap, mainWindow^.portBits, dripDropRect, dripDropRect, srcCopy, nil);
					dripCycle := 26;							{The water still on the ceiling is the drip corres-	}
					dripMode := 22;								{sponding to rectangle#26.  Bump the mode to 22.	}
					DoTheSound('drip.snd', highPriority);
				end;
			22: 		{In this mode we track the falling drop of water, checking for the floor or glider.	}
				begin
					UnionRect(dripDropRect, oldDripRect, dummyRect);	{dummy = union of the old and new	}
					CopyBits(virginMap, loadMap, dummyRect, dummyRect, srcCopy, nil);	{get background	}
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[27, 0], objectsRects[27, 1], dripDropRect);
					CopyBits(loadMap, mainWindow^.portBits, dummyRect, dummyRect, srcCopy, nil);
					if (SectRect(dummyRect, vitalArea, dummyRect)) then	{Has it hit the glider?			}
						begin
							DoGliderBitesIt;
						end;
					oldDripRect := dripDropRect;
					OffsetRect(dripDropRect, 0, gravity * 13);	{This drop falls pretty tootin' fast!  13		}
					if (dripDropRect.bottom > 300) then		{times faster than the glider falls.  If it has  	}
						dripMode := 23;							{fallen past 300 in pixels, it has hit the ground.	}
				end;
			23: 		{This is for the hit-the-ground drop.  All we do is cover the old drop on the floor	}
				begin	{and set the mode and cycle back to the start for another drip. What we do is grab	}
					UnionRect(dripDropRect, oldDripRect, dummyRect);	{a patch of the background to cover	}
					CopyBits(virginMap, mainWindow^.portBits, dummyRect, dummyRect, srcCopy, nil);	{it.	}
					dripMode := 0;
					dripCycle := 23;
				end;
			otherwise			{And that's it.  One case statement to run the drip through it's various	}
				begin				{dripping stages of animation.  Easy, organized, difficult to understand.	}
				end;				{Walk through it though.  See what happens as the mode and cycle change.	}
		end;		{end case dripMode of}
	end;

{=================================}

	procedure DoCat;
		var
			switch, tempSide: Integer;
			dummyRect: Rect;

		procedure RightSideARect (cycle: Integer);
			var
				tempRight: Integer;
				dummyRect: Rect;
		begin
			oldPawRect := pawRect;
			tempRight := pawRect.right;
			pawRect := objectsRects[cycle, 0];
			OffsetRect(pawRect, tempRight - pawRect.right, -16);
			UnionRect(pawRect, oldPawRect, dummyRect);
			CopyBits(virginMap, loadMap, dummyRect, dummyRect, srcCopy, nil);
			CopyMask(objectsMap, objectsMap, loadMap, objectsRects[cycle, 0], objectsRects[cycle, 1], pawRect);
			CopyBits(loadMap, mainWindow^.portBits, dummyRect, dummyRect, srcCopy, nil);
		end;

	begin
		case catMode of
			0: 	{In this mode the cat has moved it's tail down and it needs to be redrawn	}
				begin
					CopyBits(virginMap, loadMap, catBodyRect, catBodyRect, srcCopy, nil);
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[28, 0], objectsRects[28, 1], catBodyRect);
					CopyBits(loadMap, mainWindow^.portBits, catBodyRect, catBodyRect, srcCopy, nil);
					catMode := 1;
				end;
			1: 	{This is the tail down idle mode	}
				begin
					switch := DoRandom(20 div advanceRate);
					if (switch = 0) then
						catMode := 2;
					if (SectRect(vitalArea, dangerRect, dummyRect)) then
						catMode := 4;
				end;
			2: 	{In this mode the cat has swung it's tail up and it needs to be drawn	}
				begin
					CopyBits(virginMap, loadMap, catBodyRect, catBodyRect, srcCopy, nil);
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[29, 0], objectsRects[29, 1], catBodyRect);
					CopyBits(loadMap, mainWindow^.portBits, catBodyRect, catBodyRect, srcCopy, nil);
					catMode := 3;
				end;
			3: 	{This is the tail up idle mode		}
				begin
					switch := DoRandom(20 div advanceRate);
					if (switch = 0) then
						catMode := 0;
					if (SectRect(vitalArea, dangerRect, dummyRect)) then
						catMode := 4;
				end;
			4: 	{This is the mode where the cat is going to swat with its paw		}
				begin
					RightSideARect(30);
					catMode := 5;
				end;
			5..6: 	{In this mode the cat is swatting his paw	}
				begin
					catMode := catMode + advanceRate;
				end;
			7..8: 	{In this mode we resize the pawRect for the next cycle of swatting	}
				begin
					RightSideARect(31);
					catMode := 9;
				end;
			9..10: 	{In this mode the cat is swatting his paw	}
				begin
					catMode := catMode + advanceRate;
				end;
			11..12: 	{In this mode we resize the pawRect for the next cycle of swatting	}
				begin
					RightSideARect(32);
					catMode := 13;
				end;
			13..14: 	{In this mode the cat is swatting his paw - it is fully extended	}
				begin
					catMode := catMode + advanceRate;
					if (SectRect(vitalArea, escapeRect, dummyRect)) then
						begin
							crushed := TRUE;
							stalling := TRUE;
							timeOfDeath := gameTime;
							if (gliderCraft) then
								ResizeARect(playerDropRect, 35)
							else
								ResizeARect(playerDropRect, 55);
						end;
				end;
			15..16: 	{In this mode we resize the pawRect for the next cycle of swatting	}
				begin
					RightSideARect(33);
					catMode := 17;
				end;
			17..18: 	{In this mode the cat is swatting his paw - it is being retracted	}
				begin
					catMode := catMode + advanceRate;
				end;
			19..20: 	{In this mode the cat is going to go into refractory mode	}
				begin
					dummyRect := catBodyRect;
					dummyRect.left := dummyRect.left - 30;
					CopyBits(virginMap, loadMap, dummyRect, dummyRect, srcCopy, nil);
					CopyMask(objectsMap, objectsMap, loadMap, objectsRects[28, 0], objectsRects[28, 1], catBodyRect);
					CopyBits(loadMap, mainWindow^.portBits, dummyRect, dummyRect, srcCopy, nil);
					catMode := 21;
				end;
			21..50: 	{In this mode the cat is in it's refractory period and the only safe time to escape	}
				begin
					catMode := catMode + advanceRate;
					if ((SectRect(vitalArea, escapeRect, dummyRect)) and (not crushed)) then
						WelcomeToNextLevel;
				end;
			otherwise	{We reset the cat to a mode=0}
				begin
					catMode := 0;
				end;
		end;	{case catMode of}
		if (playerDropRect.right > 480) and (not crushed) then
			begin
				crushed := TRUE;
				stalling := TRUE;
				timeOfDeath := gameTime;
				if (gliderCraft) then
					ResizeARect(playerDropRect, 35)
				else
					ResizeARect(playerDropRect, 55);
			end;
	end;

{=================================}

end.
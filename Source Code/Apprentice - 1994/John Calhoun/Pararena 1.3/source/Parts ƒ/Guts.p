unit Guts;

interface
	uses
		Sound, Globals, Unlock, Utilities;

	procedure ResetArrows;
	procedure WheresMouse;
	procedure NewPlayerPosition;
	function FollowDiagonally (hereX, hereZ, thereX, thereZ: LongInt): Integer;
	function RunTheGoal (hereX, hereZ, speedX, speedZ: LongInt; direction: Integer): Integer;
	procedure NewOpponentPosition;
	procedure NewBallPosition;
	procedure BallInTransit;

implementation

{=================================}

	procedure TogglePlayerArrow;
		var
			srcRect, dstRect: Rect;
	begin
		SetRect(srcRect, 422, 169, 446, 204);
		SetRect(dstRect, 57, 0, 91, 35);
		CopyBits(offPlayerMap, mainWndo^.portBits, srcRect, dstRect, srcXOr, nil);
	end;

{=================================}

	procedure ToggleOpponentArrow;
		var
			srcRect, dstRect: Rect;
	begin
		SetRect(srcRect, 422, 205, 446, 240);
		SetRect(dstRect, 425, 0, 449, 35);
		CopyBits(offPlayerMap, mainWndo^.portBits, srcRect, dstRect, srcXOr, nil);
	end;

{=================================}

	procedure ResetArrows;
		var
			dstRect: Rect;
	begin
		SetRect(dstRect, 57, 0, 91, 35);
		CopyBits(offVirginMap, mainWndo^.portBits, dstRect, dstRect, srcCopy, nil);
		SetRect(dstRect, 425, 0, 449, 35);
		CopyBits(offVirginMap, mainWndo^.portBits, dstRect, dstRect, srcCopy, nil);
	end;

{=================================}

	procedure WheresMouse;
		var
			nontant, which: Integer;
			mousePt: Point;
	begin
		if ((Button) and (not lastButton)) then
			clicked := TRUE;
		lastButton := Button;

		nontant := 0;
		GetMouse(mousePt);
		if (mousePt.h > (340 + rightOffset)) then
			nontant := 6
		else if (mousePt.h < (170 + rightOffset)) then
			nontant := 0
		else
			nontant := 3;
		if (mousePt.v > (228 + downOffset)) then
			nontant := nontant + 2
		else if (mousePt.v < (114 + downOffset)) then
			nontant := nontant + 0
		else
			nontant := nontant + 1;
		which := nontantTable[nontant];
		player.mode := which;
		SetCursor(cursorArray[which]^^);
	end;

{=================================}

	procedure NewPlayerPosition;
		var
			field1, field2, forceX, forceZ: Integer;

{-------------------------}

		procedure DoGoalSlam;
		begin
			with player do
				begin
					if forceX = 10000 then
						begin
							if ((velZ < 0) and (velX < 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := -velZ div 4;
									velZ := -forceX;
									Exit(DoGoalSlam);
								end;
							if ((-velZ > velX) and (velZ < 0) and (velX > 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := -velZ div 4;
									velZ := -forceX;
									Exit(DoGoalSlam);
								end;
							if ((-velX > velZ) and (velZ > 0) and (velX < 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := -velZ div 4;
									velZ := -forceX;
								end;
						end
					else
						begin
							if ((velZ < 0) and (velX > 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := velZ div 4;
									velZ := forceX;
									Exit(DoGoalSlam);
								end;
							if ((velZ < velX) and (velZ < 0) and (velX < 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := velZ div 4;
									velZ := forceX;
									Exit(DoGoalSlam);
								end;
							if ((velX > velZ) and (velZ > 0) and (velX > 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := velZ div 4;
									velZ := forceX;
								end;
						end;
				end;
		end;

{-------------------------}

	begin
		with player do
			begin
				forceX := forceTable[posX div 1000, posZ div 1000, 0];
				forceZ := forceTable[posX div 1000, posZ div 1000, 1];
				if ((forceX = 10000) or (forceX = 11000)) then
					DoGoalSlam
				else if (forceX < 10000) then
					begin
						velX := velX * frictNum div frictDen + forceX + mouseForce[mode, 0];
						velZ := velZ * frictNum div frictDen + forceZ + mouseForce[mode, 1];
					end;
				posX := posX + velX;
				posZ := posZ + velZ;
				screenH := centerH + posX div 100;
				field1 := ABS(posX div 300);
				field2 := posZ div 300;
				if ((field1 < 81) and (ABS(field2) < 81)) then
					begin
						HLock(Handle(vertTable));
						screenV := vertTable^^.data[field1, field2] + 20;
						HUnlock(Handle(vertTable));
					end
				else
					begin
						if (ball.mode = playerHasBall) then
							ResetArrows;
						PlayerOutOfArena;
					end;
				if (screenV < 0) then
					begin
						if (ball.mode = playerHasBall) then
							ResetArrows;
						PlayerOutOfArena;
					end;
				SetRect(dest, screenH - 16, screenV - 47, screenH + 16, screenV);
			end;
	end;

{=================================}

	function FollowDiagonally;
		const
			edge = 12000;
		var
			displaceX, displaceZ: LongInt;

{-------------------------}

		procedure FleeTheEdge;
		begin
			if (hereX > edge) then
				begin
					if (hereZ > 0) then
						FollowDiagonally := northWest
					else
						FollowDiagonally := southWest;
					Exit(FleeTheEdge);
				end;
			if (hereX < -edge) then
				begin
					if (hereZ > 0) then
						FollowDiagonally := northEast
					else
						FollowDiagonally := southEast;
					Exit(FleeTheEdge);
				end;
			if (hereZ > edge) then
				begin
					if (hereX > 0) then
						FollowDiagonally := northWest
					else
						FollowDiagonally := northEast;
					Exit(FleeTheEdge);
				end;
			if (hereZ < -edge) then
				begin
					if (hereX > 0) then
						FollowDiagonally := southWest
					else
						FollowDiagonally := southEast;
				end;
		end;

{-------------------------}

	begin
		displaceX := thereX - hereX;
		displaceZ := thereZ - hereZ;
		if (displaceX > 0) then
			begin
				if (displaceZ > 0) then
					FollowDiagonally := southEast
				else
					FollowDiagonally := northEast;
			end
		else
			begin
				if (displaceZ > 0) then
					FollowDiagonally := southWest
				else
					FollowDiagonally := northWest;
			end;
		if ((ABS(hereX) > edge) or (ABS(hereZ) > edge)) then		{Getting near the edge}
			FleeTheEdge;
	end;

{=================================}

	function RunTheGoal;
	begin
		RunTheGoal := direction;
		if (direction = northEast) then
			begin
				if ((hereZ < -16000) and (hereX > 16000)) then
					begin
						opponentDropping := TRUE;
						RunTheGoal := west;
					end
				else
					begin
						if (speedZ > 0) or (hereZ < -16000) then	{was 12000}
							RunTheGoal := southWest;
					end;
			end
		else if (direction = southWest) then
			begin
				if ((speedZ < 0) or (hereZ > 6000)) then		{was 8000}
					RunTheGoal := northEast;
			end
		else		{Not facing along diagonal}
			begin
				RunTheGoal := northEast;
			end;
	end;

{=================================}

	procedure MaraDecides;
		var
			newDir: Integer;
	begin
		with opponent do
			begin
				newDir := mode;
				case ball.mode of
					nooneHasBall: 
						begin
							if (mad) then
								newDir := FollowDiagonally(posX, posZ, player.posX, player.posZ)
							else
								begin
									if (DoRandom(100) = 0) then
										mad := TRUE;
									newDir := FollowDiagonally(posX, posZ, ball.posX, ball.posZ);
									mode := newDir;
								end;
							opponentWants := TRUE;
							opponentDropping := FALSE;
						end;
					playerHasBall: 
						begin
							newDir := FollowDiagonally(posX, posZ, ball.posX, ball.posZ);
						end;
					opponentHasBall: 
						begin
							newDir := RunTheGoal(posX, posZ, velX, velZ, mode);
						end;
					otherwise
						begin
						end;
				end;	{case}
				mode := newDir;
			end;		{with opponent}
	end;

{=================================}

	procedure OttoDecides;
		var
			newDir: Integer;
	begin
		with opponent do
			begin
				newDir := mode;
				case ball.mode of
					nooneHasBall: 
						begin
							newDir := FollowDiagonally(posX, posZ, ball.posX, ball.posZ);
							opponentWants := TRUE;
							opponentDropping := FALSE;
						end;
					playerHasBall: 
						begin
							newDir := FollowDiagonally(posX, posZ, ball.posX - 5000, ball.posZ - 5000);
						end;
					opponentHasBall: 
						begin
							newDir := RunTheGoal(posX, posZ, velX, velZ, mode);
						end;
					otherwise
						begin
						end;
				end;	{case}
				mode := newDir;
			end;		{with opponent}
	end;

{=================================}

	procedure GeorgeDecides;
		var
			newDir: Integer;
	begin
		with opponent do
			begin
				newDir := mode;
				case ball.mode of
					nooneHasBall: 
						begin
							newDir := FollowDiagonally(posX, posZ, ball.posX, ball.posZ);
							opponentWants := TRUE;
							if (DoRandom(5) = 0) then
								opponentWants := FALSE;
							opponentDropping := FALSE;
						end;
					playerHasBall: 
						begin
							newDir := FollowDiagonally(posX, posZ, ball.posX - 9000, ball.posZ - 9000);
						end;
					opponentHasBall: 
						begin
							newDir := RunTheGoal(posX, posZ, velX, velZ, mode);
						end;
					otherwise
						begin
						end;
				end;	{case}
				mode := newDir;
			end;		{with opponent}
	end;

{=================================}

	procedure ClaireDecides;
		var
			newDir: Integer;
	begin
		with opponent do
			begin
				newDir := mode;
				case ball.mode of
					nooneHasBall: 
						begin
							if (mad) then
								newDir := FollowDiagonally(posX, posZ, player.posX, player.posZ)
							else
								begin
									if (DoRandom(400) = 0) then
										mad := TRUE;
									newDir := FollowDiagonally(posX, posZ, ball.posX, ball.posZ);
								end;
							opponentWants := TRUE;
							opponentDropping := FALSE;
						end;
					playerHasBall: 
						begin
							newDir := player.mode;
						end;
					opponentHasBall: 
						begin
							newDir := RunTheGoal(posX, posZ, velX, velZ, mode);
						end;
					otherwise
						begin
						end;
				end;	{case}
				mode := newDir;
			end;		{with opponent}
	end;

{=================================}

	procedure NewOpponentPosition;
		var
			field1, field2, forceX, forceZ: Integer;

{-------------------------}

		procedure HumansCollide;
			var
				tempX, tempZ: LongInt;
		begin
			if (soundArray[5]) then
				DoTheSound('crash', lowPriority);
			mad := FALSE;
			with opponent do
				begin
					tempX := velX;
					tempZ := velZ;
					velX := player.velX * denominator div numerator;
					velZ := player.velZ * denominator div numerator;
				end;
			with player do
				begin
					velX := tempX * numerator div denominator;
					velZ := tempZ * numerator div denominator;
				end;
			with opponent do
				begin
					if (velX = 0) then
						velX := -10;
					repeat
						posX := posX + velX;
						posZ := posZ + velZ;
						player.posX := player.posX + player.velX;
						player.posZ := player.posZ + player.velZ;
					until ((ABS(posX - player.posX) > 2500) or (ABS(posZ - player.posZ) > 2500));
				end;
		end;

{-------------------------}

		procedure OpponentGoalSlams;
		begin
			with opponent do
				begin
					if forceX = 10000 then
						begin
							if ((velZ < 0) and (velX < 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := -velZ div 4;
									velZ := -forceX;
									Exit(OpponentGoalSlams);
								end;
							if ((-velZ > velX) and (velZ < 0) and (velX > 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := -velZ div 4;
									velZ := -forceX;
									Exit(OpponentGoalSlams);
								end;
							if ((-velX > velZ) and (velZ > 0) and (velX < 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := -velZ div 4;
									velZ := -forceX;
								end;
						end
					else
						begin
							if ((velZ < 0) and (velX > 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := velZ div 4;
									velZ := forceX;
									Exit(OpponentGoalSlams);
								end;
							if ((velZ < velX) and (velZ < 0) and (velX < 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := velZ div 4;
									velZ := forceX;
									Exit(OpponentGoalSlams);
								end;
							if ((velX > velZ) and (velZ > 0) and (velX > 0)) then
								begin
									if (soundArray[5]) then
										DoTheSound('ricochet', lowPriority);
									forceX := velX div 4;
									velX := velZ div 4;
									velZ := forceX;
								end;
						end;
				end;
		end;

{-------------------------}

	begin
		case whichOpponent of
			mara: 
				MaraDecides;
			otto: 
				OttoDecides;
			george: 
				GeorgeDecides;
			claire: 
				ClaireDecides;
			otherwise
				begin
				end;
		end;		{case}

		with opponent do
			begin
				forceX := forceTable[posX div 1000, posZ div 1000, 0];
				forceZ := forceTable[posX div 1000, posZ div 1000, 1];
				if ((ABS(posX - player.posX) < 2500) and (ABS(posZ - player.posZ) < 2500) and (player.timeKeeper > -1)) then
					HumansCollide;
				if ((forceX = 10000) or (forceX = 11000)) then
					OpponentGoalSlams
				else if (forceX < 10000) then
					begin
						velX := velX * frictNum div frictDen + forceX + mouseForce[mode, 0];
						velZ := velZ * frictNum div frictDen + forceZ + mouseForce[mode, 1];
					end;
				posX := posX + velX;
				posZ := posZ + velZ;
				screenH := centerH + posX div 100;
				field1 := ABS(posX div 300);
				field2 := posZ div 300;
				if ((field1 < 81) and (ABS(field2) < 81)) then
					begin
						HLock(Handle(vertTable));
						screenV := vertTable^^.data[field1, field2] + 20;
						HUnlock(Handle(vertTable));
					end
				else
					begin
						if (ball.mode = opponentHasBall) then
							ResetArrows;
						OpponentOutOfArena;
					end;
				if (screenV < 0) then
					begin
						if (ball.mode = opponentHasBall) then
							ResetArrows;
						OpponentOutOfArena;
					end;
				SetRect(dest, screenH - 16, screenV - 47, screenH + 16, screenV);
			end;
		if (ball.timeKeeper > 290) then
			opponentDropping := TRUE;
	end;


{=================================}

	procedure NewBallPosition;

{-------------------------}

		procedure PlayerCapture;
		begin
			tileToggle := 0;
			tileLit := 0;
			TogglePlayerArrow;
			with ball do
				begin
					player.velX := player.velX + velX * 2 div 3;
					player.velZ := player.velZ + velZ * 2 div 3;
					velX := player.velX;
					velZ := player.velZ;
					ballJustHit := TRUE;
					mode := playerHasBall;
					timeKeeper := 1;
					player.carryMode := carrying;
					CopyBits(offVirginMap, offLoadMap, oldDest, oldDest, srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, wholeRgn);
					if (soundArray[5]) then
						DoTheSound(ballCatches[0], highPriority);
				end;
		end;

{-------------------------}

		procedure OpponentCapture;
		begin
			ToggleOpponentArrow;
			tileToggle := 1;
			tileLit := 0;
			with ball do
				begin
					opponent.velX := opponent.velX + velX * 2 div 3;
					opponent.velZ := opponent.velZ + velZ * 2 div 3;
					velX := opponent.velX;
					velZ := opponent.velZ;
					ballJustHit := TRUE;
					mode := opponentHasBall;
					timeKeeper := 1;
					opponent.carryMode := carrying;
					CopyBits(offVirginMap, offLoadMap, oldDest, oldDest, srcCopy, nil);
					CopyBits(offVirginMap, mainWndo^.portBits, oldDest, oldDest, srcCopy, wholeRgn);
					if (soundArray[5]) then
						DoTheSound(ballCatches[whichOpponent], highPriority);
				end;
		end;

{-------------------------}

		procedure PlayerCollides;
			var
				tempX, tempZ: LongInt;
		begin
			if (player.timeKeeper < 0) then
				Exit(PlayerCollides);
			lastTouchingBall := playerHasBall;
			if (ballJustHit) then
				Exit(PlayerCollides);
			if ((button) or (autoPickUp)) then
				PlayerCapture
			else
				with ball do
					begin
						if (soundArray[5]) then
							DoTheSound('ricochet', lowPriority);
						tempX := velX;
						tempZ := velZ;
						velX := player.velX * 3 div 2;
						velZ := player.velZ * 3 div 2;
						player.velX := tempX * 2 div 3;
						player.velZ := tempZ * 2 div 3;
						ballJustHit := TRUE;
					end;
		end;

{-------------------------}

		procedure OpponentCollides;
			var
				tempX, tempZ: LongInt;
		begin
			if ((whichGame <= practiceWBall) or (opponent.timeKeeper < 0)) then
				Exit(OpponentCollides);
			lastTouchingBall := opponentHasBall;
			if (ballJustHit) then
				Exit(OpponentCollides);
			if (opponentWants) then
				OpponentCapture
			else
				with ball do
					begin
						if (soundArray[5]) then
							DoTheSound('ricochet', lowPriority);
						tempX := velX;
						tempZ := velZ;
						velX := opponent.velX * 3 div 2;
						velZ := opponent.velZ * 3 div 2;
						opponent.velX := tempX * 2 div 3;
						opponent.velZ := tempZ * 2 div 3;
						ballJustHit := TRUE;
					end;
		end;

{-------------------------}

		procedure PlayerCarriesBall;
			var
				field1, field2: Integer;
				tempX, tempZ: LongInt;
		begin
			with ball do
				if (clicked) then
					begin
						ResetArrows;
						ballJustHit := TRUE;
						mode := nooneHasBall;
						player.carryMode := notCarrying;
						timeKeeper := 0;
						tempX := velX;
						tempZ := velZ;
						velX := player.velX * 3 div 2;
						velZ := player.velZ * 3 div 2;
						player.velX := player.velX div 2;
						player.velZ := player.velZ div 2;
						posX := posX + velX;
						posZ := posZ + velZ;
						screenH := centerH + posX div 100;
						field1 := ABS(posX div 300);
						field2 := posZ div 300;
						if ((field1 < 81) and (ABS(field2) < 81)) then
							begin
								HLock(Handle(vertTable));
								screenV := vertTable^^.data[field1, field2] + 20;
								HUnlock(Handle(vertTable));
							end
						else
							BallOutOfArena;
						if (screenV < 0) then
							BallOutOfArena;
						SetRect(dest, screenH - 6, screenV - 12, screenH + 6, screenV);
						oldDest := dest;
						if (soundArray[5]) then
							DoTheSound('ballDrop', lowPriority);
					end
				else
					begin
						velX := player.velX;
						velZ := player.velZ;
						posX := player.posX;
						posZ := player.posZ;
						SetRect(dest, 1, 30, 1, 30);
						oldDest := dest;
						timeKeeper := timeKeeper + 1;
					end;
		end;

{-------------------------}

		procedure OpponentCarriesBall;
			var
				field1, field2: Integer;
				tempX, tempZ: LongInt;
		begin
			with ball do
				if (opponentDropping) then
					begin
						ResetArrows;
						opponentDropping := FALSE;
						ballJustHit := TRUE;
						mode := nooneHasBall;
						opponent.carryMode := notCarrying;
						timeKeeper := 0;
						tempX := velX;
						tempZ := velZ;
						velX := opponent.velX * 3 div 2;
						velZ := opponent.velZ * 3 div 2;
						opponent.velX := opponent.velX div 2;
						opponent.velZ := opponent.velZ div 2;
						posX := posX + velX;
						posZ := posZ + velZ;
						screenH := centerH + posX div 100;
						field1 := ABS(posX div 300);
						field2 := posZ div 300;
						if ((field1 < 81) and (ABS(field2) < 81)) then
							begin
								HLock(Handle(vertTable));
								screenV := vertTable^^.data[field1, field2] + 20;
								HUnlock(Handle(vertTable));
							end
						else
							BallOutOfArena;
						if (screenV < 0) then
							BallOutOfArena;
						SetRect(dest, screenH - 6, screenV - 12, screenH + 6, screenV);
						oldDest := dest;
						if (soundArray[5]) then
							DoTheSound('ballDrop', lowPriority);
					end
				else
					begin
						velX := opponent.velX;
						velZ := opponent.velZ;
						posX := opponent.posX;
						posZ := opponent.posZ;
						SetRect(dest, 1, 30, 1, 30);
						oldDest := dest;
						timeKeeper := timeKeeper + 1;
					end;
		end;

{-------------------------}

		procedure BallRolling;
			var
				field1, field2, forceX, forceZ: Integer;
		begin
			with ball do
				begin
					if ((ABS(posX - player.posX) < 2000) and (ABS(posZ - player.posZ) < 2000)) then
						PlayerCollides
					else
						begin
							if ((ABS(posX - opponent.posX) < 2000) and (ABS(posZ - opponent.posZ) < 2000)) then
								OpponentCollides
							else
								ballJustHit := FALSE;
						end;

					if ((ABS(posX) <= 24000) and (ABS(posZ) <= 24000)) then
						begin
							forceX := forceTable[posX div 1000, posZ div 1000, 0];
							forceZ := forceTable[posX div 1000, posZ div 1000, 1];
							if ((forceX = 10000) or (forceX = 11000)) then
								begin
									if ((forceZ = 20000) or (forceZ = 30000)) then
										begin
											ResetArrows;
											DoAScore(forceZ);
										end
									else
										begin
											if forceX = 10000 then
												begin
													if ((velZ < 0) and (velX < 0)) then
														begin
															if (soundArray[5]) then
																DoTheSound('ricochet', lowPriority);
															forceX := velX * 2 div 3;
															velX := -velZ * 2 div 3;
															velZ := -forceX;
														end;
													if ((-velZ > velX) and (velZ < 0) and (velX > 0)) then
														begin
															if (soundArray[5]) then
																DoTheSound('ricochet', lowPriority);
															forceX := velX * 2 div 3;
															velX := -velZ * 2 div 3;
															velZ := -forceX;
														end;
													if ((-velX > velZ) and (velZ > 0) and (velX < 0)) then
														begin
															if (soundArray[5]) then
																DoTheSound('ricochet', lowPriority);
															forceX := velX * 2 div 3;
															velX := -velZ * 2 div 3;
															velZ := -forceX;
														end;
												end
											else
												begin
													if ((velZ < 0) and (velX > 0)) then
														begin
															if (soundArray[5]) then
																DoTheSound('ricochet', lowPriority);
															forceX := velX * 2 div 3;
															velX := velZ * 2 div 3;
															velZ := forceX;
														end;
													if ((velZ < velX) and (velZ < 0) and (velX < 0)) then
														begin
															if (soundArray[5]) then
																DoTheSound('ricochet', lowPriority);
															forceX := velX * 2 div 3;
															velX := velZ * 2 div 3;
															velZ := forceX;
														end;
													if ((velX > velZ) and (velZ > 0) and (velX > 0)) then
														begin
															if (soundArray[5]) then
																DoTheSound('ricochet', lowPriority);
															forceX := velX * 2 div 3;
															velX := velZ * 2 div 3;
															velZ := forceX;
														end;
												end;
										end;
								end
							else if (forceX < 10000) then
								begin
									velX := velX * 49 div 50 + forceX;
									velZ := velZ * 49 div 50 + forceZ;
								end
						end
					else
						begin
						end;
					posX := posX + velX;
					posZ := posZ + velZ;
					screenH := centerH + posX div 100;
					field1 := ABS(posX div 300);
					field2 := posZ div 300;
					if ((field1 < 81) and (ABS(field2) < 81)) then
						begin
							HLock(Handle(vertTable));
							screenV := vertTable^^.data[field1, field2] + 20;
							HUnlock(Handle(vertTable));
						end
					else
						BallOutOfArena;
					if (screenV < 0) then
						BallOutOfArena;
					SetRect(dest, screenH - 6, screenV - 12, screenH + 6, screenV);
				end;
		end;

{-------------------------}

	begin
		with ball do
			begin
				if (mode = playerHasBall) then
					PlayerCarriesBall
				else if (mode = opponentHasBall) then
					OpponentCarriesBall
				else
					BallRolling;
				clicked := FALSE;
				if (timeKeeper = 250) then
					DoTheSound('holding', highPriority);
				if (timeKeeper > 300) then
					begin
						ResetArrows;
						DoHolding;
					end;
			end;
	end;

{=================================}

	procedure BallInTransit;
	begin
		with ball do
			begin
				timeKeeper := timeKeeper + 1;
				if ((timeKeeper = -10) and (soundArray[3])) then
					DoTheSound('ballShot', highPriority);
				if (timeKeeper = 0) then
					begin
						with ball do
							begin
								velX := -2500;
								velZ := 0;
								posX := 300;
								posZ := -20500;
								screenH := centerH + posX div 100;
								HLock(Handle(vertTable));
								screenV := vertTable^^.data[ABS(posX div 300), posZ div 300] + 20;
								HUnlock(Handle(vertTable));
								SetRect(dest, screenH - 6, screenV - 12, screenH + 6, screenV);
								oldDest := dest;
							end;
						NewBallPosition;
					end;
			end;
	end;

{=================================}

end.
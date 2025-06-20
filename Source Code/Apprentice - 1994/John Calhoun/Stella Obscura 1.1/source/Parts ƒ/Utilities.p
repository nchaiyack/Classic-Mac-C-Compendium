unit Utilities;

interface

	uses
		Sound, SomeGlobals;

	var
		soundOn, slowOn, playing, pausing, gameOver, forgetSound: Boolean;
		chanPtr: sndchannelptr;
		playerShot: array[1..numberOfEnemies] of Boolean;
		levelOn, elevationMode, shields, fireMode: Integer;
		theScore: LongInt;
		shieldString, scoreString: Str255;
		mousePt: Point;
		leftShipDst, rightShipDst, oldLeftShip, oldRightShip: Rect;
		shipSrc, shipMaskSrc: array[0..4] of Rect;
		crossHairSrc, crossHairMask: array[0..2] of Rect;
		leftCrossHairDst, rightCrossHairDst, oldLeftCrossHair, oldRightCrossHair: Rect;
		hoopOut: Boolean;
		heightRatio: array[1..36] of Real;
		hoopAngle, hoopDistance: Integer;
		hoopSrc, hoopMask, leftHoopDst, rightHoopDst: Rect;
		shieldSrc, leftShieldDst, rightShieldDst, bigClip: Rect;

	procedure DoTheSound (whichOne: Str255; asynch: Boolean);
	procedure PredefineEnemyRects;
	procedure SetShipsPos;
	procedure ResetEnemy (whichOne, specifically: Integer);
	procedure UpDateEnemy;
	procedure ComputeHoops;
	procedure PlayerFiredUpon (whoShot: Integer);
	procedure DrawScene;
	procedure HyperSpace3D;
	procedure WrapItUp;

implementation

{=================================}

	procedure DoTheSound;
		var
			theSnd: Handle;
			err: OSErr;
	begin
		if (soundOn) then
			begin
				theSnd := GetNamedResource('snd ', whichOne);
				if (theSnd <> nil) and (ResError = noErr) then
					begin
						if (chanPtr <> nil) then
							begin
								err := SndDisposeChannel(chanPtr, TRUE);
								chanPtr := nil;
							end;
						if (asynch = true) and (SndNewChannel(chanPtr, 0, initMono, nil) = noErr) then
							err := SndPlay(chanPtr, theSnd, TRUE)
						else
							err := SndPlay(nil, theSnd, FALSE);
					end;
			end;
	end;

{=================================}

	procedure PredefineEnemyRects;
		var
			index, index2, enemyXPos, enemyYPos, enemyDistance, horiOff, vertOff, enemyDepth: Integer;
			scale: Real;
			rightEnemyDst, leftEnemyDst: Rect;
	begin
		enemyMaxCycles := (farthest - nearest) div enemySpeed + 1;
		for index := 1 to 10 do
			begin
				enemyXPos := DoRandom(2 * farthestStray) - farthestStray;
				enemyYPos := DoRandom(2 * farthestStray) - farthestStray;
				enemyDistance := farthest;
				index2 := 1;
				enemyDistance := enemyDistance - enemySpeed;
				enemySize[index, index2] := (enemyDistance - 10) div 32;
				if (enemySize[index, index2] > 8) then
					enemySize[index, index2] := 8;
				SetRect(leftEnemyDst, -14, -16, 14, 16);
				rightEnemyDst := leftEnemyDst;
				scale := nearest / enemyDistance * farthestStray;
				horiOff := TRUNC(enemyXPos * scale);
				vertOff := TRUNC(enemyYPos * scale);
				enemyDepth := ((10 - enemySize[index, index2]) div 2);
				OffsetRect(leftEnemyDst, horiOff + 63 + enemyDepth, vertOff + 120);
				OffsetRect(rightEnemyDst, horiOff + 63 - enemyDepth, vertOff + 120);
				theEnemies[index, leftWhole, index2] := leftEnemyDst;
				theEnemies[index, leftNew, index2] := leftEnemyDst;
				theEnemies[index, rightWhole, index2] := rightEnemyDst;
				theEnemies[index, rightNew, index2] := rightEnemyDst;
				for index2 := 2 to enemyMaxCycles do
					begin
						enemyDistance := enemyDistance - enemySpeed;
						enemySize[index, index2] := (enemyDistance - 10) div 32;
						if (enemySize[index, index2] > 8) then
							enemySize[index, index2] := 8;
						SetRect(leftEnemyDst, -14, -16, 14, 16);
						rightEnemyDst := leftEnemyDst;
						scale := nearest / enemyDistance * farthestStray;
						horiOff := TRUNC(enemyXPos * scale);
						vertOff := TRUNC(enemyYPos * scale);
						enemyDepth := ((10 - enemySize[index, index2]) div 2);
						OffsetRect(leftEnemyDst, horiOff + 63 + enemyDepth, vertOff + 120);
						OffsetRect(rightEnemyDst, horiOff + 63 - enemyDepth, vertOff + 120);
						UnionRect(leftEnemyDst, theEnemies[index, leftNew, index2 - 1], theEnemies[index, leftWhole, index2]);
						theEnemies[index, leftNew, index2] := leftEnemyDst;
						UnionRect(rightEnemyDst, theEnemies[index, rightNew, index2 - 1], theEnemies[index, rightWhole, index2]);
						theEnemies[index, rightNew, index2] := rightEnemyDst;
					end;
			end;
		enemyMaxCycles := enemyMaxCycles - 1;
	end;

{=================================}

	procedure SetShipsPos;
		var
			horiOff, vertOff: Integer;
	begin
		GetMouse(mousePt);
		SetRect(leftShipDst, -17, -11, 18, 12);
		rightShipDst := leftShipDst;
		horiOff := ((mousePt.h - 256) div 4);
		vertOff := TRUNC((mousePt.v - 171) * 0.7);
		OffsetRect(leftShipDst, horiOff + 63 + depth, vertOff + 120);
		OffsetRect(rightShipDst, horiOff + 63 - depth, vertOff + 120);
		elevationMode := (mousePt.v div 69);
		if (elevationMode > 4) then
			elevationMode := 4;

		SetRect(leftCrossHairDst, -6, -6, 5, 5);
		rightCrossHairDst := leftCrossHairDst;
		horiOff := (horiOff div 2);
		vertOff := (vertOff div 2);
		OffsetRect(leftCrossHairDst, horiOff + 63 + sightDepth, vertOff + 120);
		OffsetRect(rightCrossHairDst, horiOff + 63 - sightDepth, vertOff + 120);

		if button and (fireMode = 0) then
			begin
				fireMode := 2;
				DoTheSound('laserFire', TRUE);
			end
		else if (fireMode > 0) then
			fireMode := fireMode - 1;
	end;

{=================================}

	procedure ResetEnemy;
		var
			thePntr, dimen: Integer;
			tempRect: Rect;
	begin
		enemiesPassed := enemiesPassed + 1;
		thePntr := specifically;
		playerShot[whichOne] := FALSE;
		dimen := currentEnemies[whichOne, dataDimension];
		tempRect := theEnemies[dimen, leftWhole, thePntr];
		OffsetRect(tempRect, 129, 50);
		CopyBits(offLeftVirginMap, mainWndo^.portBits, theEnemies[dimen, leftWhole, thePntr], tempRect, srcCopy, leftScreenRgn);
		tempRect := theEnemies[dimen, rightWhole, thePntr];
		OffsetRect(tempRect, 256, 50);
		CopyBits(offRightVirginMap, mainWndo^.portBits, theEnemies[dimen, rightWhole, thePntr], tempRect, srcCopy, rightScreenRgn);
		if (enemiesPassed > enemyMax) then
			begin
				hoopOut := TRUE;
				CopyBits(offLeftVirginPort^.portBits, offLeftPort^.portBits, offLeftVirginMap.bounds, offLeftVirginMap.bounds, srcCopy, nil);
				CopyBits(offRightVirginPort^.portBits, offRightPort^.portBits, offRightVirginMap.bounds, offRightVirginMap.bounds, srcCopy, nil);
				CopyBits(offLeftPort^.portBits, mainWndo^.portBits, offLeftMap.bounds, leftBckgrndDst, srcCopy, nil);
				CopyBits(offRightPort^.portBits, mainWndo^.portBits, offRightMap.bounds, rightBckgrndDst, srcCopy, nil);
			end
		else
			begin
				currentEnemies[whichOne, currentPointer] := 1;
				enemyShot[whichOne] := FALSE;
				currentEnemies[whichOne, dataDimension] := DoRandom(10) + 1;
			end;
	end;

{=================================}

	procedure UpDateEnemy;
		var
			index, thePntr, dimen: Integer;
			dummyRect: Rect;
	begin
		for index := 1 to numberOfEnemies do
			begin
				currentEnemies[index, currentPointer] := currentEnemies[index, currentPointer] + 1;
				thePntr := currentEnemies[index, currentPointer];
				dimen := currentEnemies[index, dataDimension];
				if (thePntr > enemyMaxCycles) then
					ResetEnemy(index, thePntr - 1);
				if (fireMode <> 0) and (enemySize[dimen, thePntr] = 5) and SectRect(oldLeftCrossHair, theEnemies[dimen, leftNew, thePntr], dummyRect) then
					begin
						enemyShot[index] := TRUE;
						ResetEnemy(index, thePntr - 1);
						theScore := theScore + 50;
						NumToString(theScore, scoreString);
						DoTheSound('enemyBoom', TRUE);
					end
				else if (enemySize[dimen, thePntr] < 5) then
					begin
						if (DoRandom(50) < levelOn) then
							playerShot[index] := TRUE;
					end;
			end;
	end;

{=================================}

	procedure ComputeHoops;
		var
			hoopWidth, hoopHeight, index: Integer;
			center: Point;
	begin
		SetPort(offPlayerPort);
		PenNormal;
		hoopAngle := hoopAngle + flipSpeed;
		if (hoopAngle > 36) then
			hoopAngle := hoopAngle - 36;
		hoopDistance := hoopDistance - travelSpeed;
		if (hoopDistance < nearest) then
			begin
				CopyBits(offLeftVirginPort^.portBits, offLeftPort^.portBits, offLeftVirginMap.bounds, offLeftVirginMap.bounds, srcCopy, nil);
				CopyBits(offRightVirginPort^.portBits, offRightPort^.portBits, offRightVirginMap.bounds, offRightVirginMap.bounds, srcCopy, nil);
				CopyBits(offLeftPort^.portBits, mainWndo^.portBits, offLeftMap.bounds, leftBckgrndDst, srcCopy, nil);
				CopyBits(offRightPort^.portBits, mainWndo^.portBits, offRightMap.bounds, rightBckgrndDst, srcCopy, nil);
				SetPt(center, 63, 120);
				if PtInRect(center, leftShipDst) then
					HyperSpace3D
				else
					begin
						enemiesPassed := 0;
						for index := 1 to numberOfEnemies do
							begin
								playerShot[index] := FALSE;
								enemyShot[index] := FALSE;
								currentEnemies[index, currentPointer] := index;
							end;
						enemyMax := enemyMax + 10;
						hoopOut := FALSE;
						hoopAngle := 1;
						hoopDistance := farthest;
						ComputeHoops;
					end;
			end
		else
			begin
				hoopWidth := TRUNC(nearest / hoopDistance * hoopSize) + 1;
				hoopHeight := TRUNC(hoopWidth * heightRatio[hoopAngle]) + 1;
				SetRect(hoopMask, 0, 0, hoopWidth, hoopHeight);
				hoopSrc := hoopMask;
				OffsetRect(hoopSrc, 328, 1);
				leftHoopDst := hoopMask;
				rightHoopDst := hoopMask;
				OffsetRect(hoopMask, 328, 103);
				EraseRect(hoopMask);
				OffsetRect(leftHoopDst, 64 - (hoopWidth div 2) + (hoopWidth div 10), 120 - (hoopHeight div 2));
				OffsetRect(rightHoopDst, 64 - (hoopWidth div 2) - (hoopWidth div 10), 120 - (hoopHeight div 2));
				PenSize((hoopWidth div 20) + 1, (hoopWidth div 20) + 1);
				FrameOval(hoopMask);
			end;
	end;

{=================================}

	procedure DrawHoopScene;
		var
			wholeLeftShip, wholeRightShip, wholeLeftCrossHair, wholeRightCrosshair: Rect;
			tempRect, tempRect2, wholeLeftHoop, wholeRightHoop: Rect;
	begin
		wholeLeftHoop := leftHoopDst;
		wholeLeftHoop.top := wholeLeftHoop.top - 5;
		wholeLeftHoop.bottom := wholeLeftHoop.bottom + 5;
		wholeRightHoop := rightHoopDst;
		wholeRightHoop.top := wholeLeftHoop.top;
		wholeRightHoop.bottom := wholeLeftHoop.bottom;
		CopyBits(offLeftVirginMap, offLeftMap, wholeLeftHoop, wholeLeftHoop, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, wholeRightHoop, wholeRightHoop, srcCopy, nil);
{Get swatch for crosshair}
		UnionRect(oldLeftCrossHair, leftCrossHairDst, wholeLeftCrossHair);
		UnionRect(oldRightCrossHair, rightCrossHairDst, wholeRightCrosshair);
		CopyBits(offLeftVirginMap, offLeftMap, wholeLeftCrossHair, wholeLeftCrossHair, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, wholeRightCrosshair, wholeRightCrosshair, srcCopy, nil);
{Get swatch for player's ship}
		UnionRect(oldLeftShip, leftShipDst, wholeLeftShip);
		UnionRect(oldRightShip, rightShipDst, wholeRightShip);
		CopyBits(offLeftVirginMap, offLeftMap, wholeLeftShip, wholeLeftShip, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, wholeRightShip, wholeRightShip, srcCopy, nil);

		CopyMask(offPlayerMap, offPlayerMap, offLeftMap, hoopSrc, hoopMask, leftHoopDst);
		CopyMask(offPlayerMap, offPlayerMap, offRightMap, hoopSrc, hoopMask, rightHoopDst);
		CopyMask(offPlayerMap, offPlayerMap, offLeftMap, crossHairSrc[fireMode], crossHairMask[fireMode], leftCrossHairDst);
		CopyMask(offPlayerMap, offPlayerMap, offRightMap, crossHairSrc[fireMode], crossHairMask[fireMode], rightCrossHairDst);
		CopyMask(offPlayerMap, offPlayerMap, offLeftMap, shipSrc[elevationMode], shipMaskSrc[elevationMode], leftShipDst);
		CopyMask(offPlayerMap, offPlayerMap, offRightMap, shipSrc[elevationMode], shipMaskSrc[elevationMode], rightShipDst);

		tempRect := wholeLeftHoop;
		OffsetRect(tempRect, 129, 50);
		tempRect2 := wholeRightHoop;
		OffsetRect(tempRect2, 256, 50);
		CopyBits(offLeftMap, mainWndo^.portBits, wholeLeftHoop, tempRect, srcCopy, leftScreenRgn);
		CopyBits(offRightMap, mainWndo^.portBits, wholeRightHoop, tempRect2, srcCopy, rightScreenRgn);
		tempRect := wholeLeftCrossHair;
		OffsetRect(tempRect, 129, 50);
		tempRect2 := wholeRightCrossHair;
		OffsetRect(tempRect2, 256, 50);
		CopyBits(offLeftMap, mainWndo^.portBits, wholeLeftCrossHair, tempRect, srcCopy, nil);
		CopyBits(offRightMap, mainWndo^.portBits, wholeRightCrossHair, tempRect2, srcCopy, nil);
		tempRect := wholeLeftShip;
		OffsetRect(tempRect, 129, 50);
		tempRect2 := wholeRightShip;
		OffsetRect(tempRect2, 256, 50);
		CopyBits(offLeftMap, mainWndo^.portBits, wholeLeftShip, tempRect, srcCopy, leftScreenRgn);
		CopyBits(offRightMap, mainWndo^.portBits, wholeRightShip, tempRect2, srcCopy, rightScreenRgn);

		SetPort(mainWndo);
		PenNormal;
		TextFont(0);
		TextSize(12);
		TextMode(NotSrcCopy);
		MoveTo(145, 59);
		DrawString('stargate ahead');
		MoveTo(268, 59);
		DrawString('stargate ahead');
		TextFont(1);
		TextSize(9);
		MoveTo(152, 74);
		DrawString(shieldString);
		MoveTo(152, 280);
		DrawString(scoreString);
		MoveTo(273, 74);
		DrawString(shieldString);
		MoveTo(273, 280);
		DrawString(scoreString);

		oldLeftShip := leftShipDst;
		oldRightShip := rightShipDst;
		oldLeftCrossHair := leftCrossHairDst;
		oldRightCrossHair := rightCrossHairDst;
	end;

{=================================}

	procedure PlayerFiredUpon;
		var
			thePntr, dimen: Integer;
			tempRect, tempRect2: Rect;
	begin
		thePntr := currentEnemies[whoShot, currentPointer];
		dimen := currentEnemies[whoShot, dataDimension];
		playerShot[whoShot] := FALSE;
		SetPort(mainWndo);
		PenNormal;
		PenMode(patXOr);
		ClipRect(leftBckgrndDst);
		MoveTo(leftShipDst.left + 147, leftShipDst.top + 72);
		LineTo((theEnemies[dimen, leftNew, thePntr].left + theEnemies[dimen, leftNew, thePntr].right) div 2 + 129, (theEnemies[dimen, leftNew, thePntr].top + theEnemies[dimen, leftNew, thePntr].bottom) div 2 + 50);
		tempRect := leftShipDst;
		OffsetRect(tempRect, 129, 50);
		InvertOval(tempRect);
		ClipRect(rightBckgrndDst);
		MoveTo(rightShipDst.left + 274, rightShipDst.top + 72);
		LineTo((theEnemies[dimen, rightNew, thePntr].left + theEnemies[dimen, rightNew, thePntr].right) div 2 + 256, (theEnemies[dimen, rightNew, thePntr].top + theEnemies[dimen, rightNew, thePntr].bottom) div 2 + 50);
		tempRect2 := rightShipDst;
		OffsetRect(tempRect2, 256, 50);
		InvertOval(tempRect2);
		shields := shields - 1;
		if (shields < 0) then
			gameOver := TRUE;
		NumToString(shields, shieldString);
		DoTheSound('enemyFire', TRUE);
		ClipRect(leftBckgrndDst);
		MoveTo(leftShipDst.left + 147, leftShipDst.top + 72);
		LineTo((theEnemies[dimen, leftNew, thePntr].left + theEnemies[dimen, leftNew, thePntr].right) div 2 + 129, (theEnemies[dimen, leftNew, thePntr].top + theEnemies[dimen, leftNew, thePntr].bottom) div 2 + 50);
		InvertOval(tempRect);
		ClipRect(rightBckgrndDst);
		MoveTo(rightShipDst.left + 274, rightShipDst.top + 72);
		LineTo((theEnemies[dimen, rightNew, thePntr].left + theEnemies[dimen, rightNew, thePntr].right) div 2 + 256, (theEnemies[dimen, rightNew, thePntr].top + theEnemies[dimen, rightNew, thePntr].bottom) div 2 + 50);
		InvertOval(tempRect2);
		ClipRect(bigClip);
		CopyBits(offLeftVirginMap, mainWndo^.portBits, shieldSrc, leftShieldDst, srcCopy, nil);
		CopyBits(offRightVirginMap, mainWndo^.portBits, shieldSrc, rightShieldDst, srcCopy, nil);
	end;

{=================================}

	procedure DrawScene;
		var
			index, thePntr, dimen, theSize: Integer;
			wholeLeftShip, wholeRightShip, wholeLeftCrossHair, wholeRightCrosshair: Rect;
			tempRect, tempRect2: Rect;
	begin
		if (hoopOut) then
			begin
				DrawHoopScene;
				Exit(DrawScene);
			end;
		for index := 1 to numberOfEnemies do
			begin
				thePntr := currentEnemies[index, currentPointer];
				dimen := currentEnemies[index, dataDimension];
				CopyBits(offLeftVirginMap, offLeftMap, theEnemies[dimen, leftWhole, thePntr], theEnemies[dimen, leftWhole, thePntr], srcCopy, nil);
				CopyBits(offRightVirginMap, offRightMap, theEnemies[dimen, rightWhole, thePntr], theEnemies[dimen, rightWhole, thePntr], srcCopy, nil);
			end;
{Get swatch for crosshair}
		UnionRect(oldLeftCrossHair, leftCrossHairDst, wholeLeftCrossHair);
		UnionRect(oldRightCrossHair, rightCrossHairDst, wholeRightCrosshair);
		CopyBits(offLeftVirginMap, offLeftMap, wholeLeftCrossHair, wholeLeftCrossHair, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, wholeRightCrosshair, wholeRightCrosshair, srcCopy, nil);
{Get swatch for player's ship}
		UnionRect(oldLeftShip, leftShipDst, wholeLeftShip);
		UnionRect(oldRightShip, rightShipDst, wholeRightShip);
		CopyBits(offLeftVirginMap, offLeftMap, wholeLeftShip, wholeLeftShip, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, wholeRightShip, wholeRightShip, srcCopy, nil);

		for index := 1 to numberOfEnemies do
			begin
				thePntr := currentEnemies[index, currentPointer];
				dimen := currentEnemies[index, dataDimension];
				theSize := enemySize[dimen, thePntr];
				CopyMask(offPlayerMap, offPlayerMap, offLeftMap, enemySrc[theSize], enemyMaskSrc[theSize], theEnemies[dimen, leftNew, thePntr]);
				CopyMask(offPlayerMap, offPlayerMap, offRightMap, enemySrc[theSize], enemyMaskSrc[theSize], theEnemies[dimen, rightNew, thePntr]);
			end;
		CopyMask(offPlayerMap, offPlayerMap, offLeftMap, crossHairSrc[fireMode], crossHairMask[fireMode], leftCrossHairDst);
		CopyMask(offPlayerMap, offPlayerMap, offRightMap, crossHairSrc[fireMode], crossHairMask[fireMode], rightCrossHairDst);
		CopyMask(offPlayerMap, offPlayerMap, offLeftMap, shipSrc[elevationMode], shipMaskSrc[elevationMode], leftShipDst);
		CopyMask(offPlayerMap, offPlayerMap, offRightMap, shipSrc[elevationMode], shipMaskSrc[elevationMode], rightShipDst);

{Drop enemies to screen}
		for index := 1 to numberOfEnemies do
			begin
				thePntr := currentEnemies[index, currentPointer];
				dimen := currentEnemies[index, dataDimension];
				tempRect := theEnemies[dimen, leftWhole, thePntr];
				OffsetRect(tempRect, 129, 50);
				CopyBits(offLeftMap, mainWndo^.portBits, theEnemies[dimen, leftWhole, thePntr], tempRect, srcCopy, leftScreenRgn);
				tempRect := theEnemies[dimen, rightWhole, thePntr];
				OffsetRect(tempRect, 256, 50);
				CopyBits(offRightMap, mainWndo^.portBits, theEnemies[dimen, rightWhole, thePntr], tempRect, srcCopy, rightScreenRgn);
			end;
{Drop crosshair to screen}
		tempRect := wholeLeftCrossHair;
		OffsetRect(tempRect, 129, 50);
		CopyBits(offLeftMap, mainWndo^.portBits, wholeLeftCrossHair, tempRect, srcCopy, nil);
		tempRect := wholeRightCrosshair;
		OffsetRect(tempRect, 256, 50);
		CopyBits(offRightMap, mainWndo^.portBits, wholeRightCrosshair, tempRect, srcCopy, nil);
{Drop player's ship to screen}
		tempRect := wholeLeftShip;
		OffsetRect(tempRect, 129, 50);
		CopyBits(offLeftMap, mainWndo^.portBits, wholeLeftShip, tempRect, srcCopy, leftScreenRgn);
		tempRect := wholeRightShip;
		OffsetRect(tempRect, 256, 50);
		CopyBits(offRightMap, mainWndo^.portBits, wholeRightShip, tempRect, srcCopy, rightScreenRgn);

		SetPort(mainWndo);
		PenNormal;
		TextFont(1);
		TextSize(9);
		TextMode(NotSrcCopy);
		MoveTo(153, 74);
		DrawString(shieldString);
		MoveTo(153, 280);
		DrawString(scoreString);
		MoveTo(273, 74);
		DrawString(shieldString);
		MoveTo(273, 280);
		DrawString(scoreString);

		for index := 1 to numberOfEnemies do
			if playerShot[index] then
				PlayerFiredUpon(index);
		oldLeftShip := leftShipDst;
		oldRightShip := rightShipDst;
		oldLeftCrossHair := leftCrossHairDst;
		oldRightCrossHair := rightCrossHairDst;
	end;

{=================================}

	procedure HyperSpace3D;
		var
			index, index2: Integer;
			blackHole, tempRect, tempRect2: Rect;
			starPoints: array[1..30] of Point;
			Trails: array[1..10, 1..30, 0..5] of Integer;
	begin
		SetPort(mainWndo);
		tempRect := leftShipDst;
		OffsetRect(tempRect, 129, 50);
		tempRect2 := rightShipDst;
		OffsetRect(tempRect2, 256, 50);

		DoTheSound('enterHole', TRUE);

		for index := 1 to 30 do
			begin
				repeat
					SetPt(starPoints[index], (DoRandom(8) - 4) * 3, (DoRandom(8) - 4) * 3);
				until ((starPoints[index].h <> 0) and (starPoints[index].v <> 0));
				for index2 := 1 to 10 do
					begin
						Trails[index2, index, 0] := (192 + starPoints[index].h);
						Trails[index2, index, 1] := (170 + starPoints[index].v);
						Trails[index2, index, 2] := (starPoints[index].h * (index2 - 1) + index2);
						Trails[index2, index, 3] := (starPoints[index].v * (index2 - 1));
						Trails[index2, index, 4] := (starPoints[index].h * (index2 - 1) - index2);
						Trails[index2, index, 5] := (starPoints[index].v * (index2 - 1));
					end;
				SetRect(blackHole, 63 - index * 4, 120 - index * 4, 63 + index * 4, 120 + index * 4);
				OffsetRect(blackHole, 129 + index div 6, 50);
				ClipRect(leftBckgrndDst);
				FillOval(blackHole, black);
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, shipSrc[elevationMode], shipMaskSrc[elevationMode], tempRect);
				OffsetRect(blackHole, 127 - index div 3, 0);
				ClipRect(rightBckgrndDst);
				FillOval(blackHole, black);
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, shipSrc[elevationMode], shipMaskSrc[elevationMode], tempRect2);
			end;

		levelOn := levelOn + 1;
		enemiesPassed := 0;
		enemyMax := levelOn * 5 + 5;
		enemySpeed := (enemyMax - 10) div 4 + 5;
		if enemyMax > 30 then
			enemyMax := 30;
		PredefineEnemyRects;
		for index := 1 to numberOfEnemies do
			begin
				playerShot[index] := FALSE;
				enemyShot[index] := FALSE;
				currentEnemies[index, currentPointer] := index;
				currentEnemies[index, dataDimension] := index;
			end;
		fireMode := 0;
		shields := 10;
		NumToString(shields, shieldString);
		hoopOut := FALSE;
		hoopAngle := 1;
		hoopDistance := farthest;
		ComputeHoops;

		DoTheSound('hyperSpace', TRUE);
		SetPort(mainWndo);
		PenNormal;
		PenPat(white);
		for index2 := 1 to 10 do
			begin
				for index := 1 to 30 do
					begin
						ClipRect(leftBckgrndDst);
						MoveTo(Trails[index2, index, 0], Trails[index2, index, 1]);
						Line(Trails[index2, index, 2], Trails[index2, index, 3]);
						ClipRect(rightBckgrndDst);
						MoveTo(Trails[index2, index, 0] + 127, Trails[index2, index, 1]);
						Line(Trails[index2, index, 4], Trails[index2, index, 5]);
					end;
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, shipSrc[elevationMode], shipMaskSrc[elevationMode], tempRect);
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, shipSrc[elevationMode], shipMaskSrc[elevationMode], tempRect2);
			end;

		PenPat(black);
		PenSize(2, 2);
		DoTheSound('hyperSpace', TRUE);
		for index2 := 1 to 10 do
			begin
				for index := 1 to 30 do
					begin
						ClipRect(leftBckgrndDst);
						MoveTo(Trails[index2, index, 0], Trails[index2, index, 1]);
						Line(Trails[index2, index, 2], Trails[index2, index, 3]);
						ClipRect(rightBckgrndDst);
						MoveTo(Trails[index2, index, 0] + 127, Trails[index2, index, 1]);
						Line(Trails[index2, index, 4], Trails[index2, index, 5]);
					end;
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, shipSrc[elevationMode], shipMaskSrc[elevationMode], tempRect);
				CopyMask(offPlayerMap, offPlayerMap, mainWndo^.portBits, shipSrc[elevationMode], shipMaskSrc[elevationMode], tempRect2);
			end;
		ClipRect(bigClip);
		DoTheVirgin;
	end;

{=================================}

	procedure WrapItUp;
		var
			index: Integer;
			dummyLong: LongInt;
			tempSrc, tempMask, tempLeft, tempRight, wholeLeft, wholeRight, tempRect: Rect;
	begin
		DoTheSound('enemyBoom', TRUE);
		SetPort(mainWndo);
		PenNormal;
		PenPat(white);
		tempLeft := leftShipDst;
		OffsetRect(tempLeft, 129, 50);
		tempRight := rightShipDst;
		OffsetRect(tempRight, 256, 50);
		for index := 1 to 50 do
			begin
				ClipRect(leftBckgrndDst);
				FrameOval(tempLeft);
				ClipRect(rightBckgrndDst);
				FrameOval(tempRight);
				InsetRect(tempLeft, -2, -2);
				InsetRect(tempRight, -2, -2);
			end;
		ClipRect(bigClip);
		SetPort(offPlayerPort);
		SetRect(tempSrc, 328, 103, 328 + 91, 103 + 43);
		EraseRect(tempSrc);
		SetRect(tempMask, 226, 279, 317, 322);
		tempLeft := tempMask;
		OffsetRect(tempLeft, -226, -279);
		tempRight := tempLeft;
		OffsetRect(tempLeft, 18, 241);
		OffsetRect(tempRight, 10, 241);
		for index := 1 to 332 do
			begin
				wholeLeft := tempLeft;
				wholeLeft.bottom := wholeLeft.bottom + 1;
				wholeRight := tempRight;
				wholeRight.bottom := wholeRight.bottom + 1;
				CopyBits(offLeftVirginMap, offLeftMap, wholeLeft, wholeLeft, srcCopy, nil);
				CopyBits(offRightVirginMap, offRightMap, wholeRight, wholeRight, srcCopy, nil);
				CopyMask(offPlayerMap, offPlayerMap, offLeftMap, tempSrc, tempMask, tempLeft);
				CopyMask(offPlayerMap, offPlayerMap, offRightMap, tempSrc, tempMask, tempRight);
				tempRect := wholeLeft;
				OffsetRect(tempRect, 129, 50);
				CopyBits(offLeftMap, mainWndo^.portBits, wholeLeft, tempRect, srcCopy, leftScreenRgn);
				tempRect := wholeRight;
				OffsetRect(tempRect, 256, 50);
				CopyBits(offRightMap, mainWndo^.portBits, wholeRight, tempRect, srcCopy, rightScreenRgn);
				OffsetRect(tempLeft, 0, -1);
				OffsetRect(tempRight, 0, -1);
			end;

		Delay(150, dummyLong);

		DoTheVirgin;
		ShowCursor;
		EnableItem(GetMenu(AppleM), 0);
		EnableItem(GetMenu(GameM), 1);
		DisableItem(GetMenu(GameM), 2);
		DisableItem(GetMenu(GameM), 3);
		EnableItem(GetMenu(OptionsM), 0);
		playing := FALSE;
		pausing := FALSE;
		SetEventMask(idleMask);
		FlushEvents(everyEvent, 0);
	end;

{=================================}

end.
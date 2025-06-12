unit SomeGlobals;

interface

	uses
		Sound;

	const
		AppleM = 201;
		GameM = 202;
		OptionsM = 203;

		depth = 4;
		sightDepth = 2;
		flipSpeed = 2;
		travelSpeed = 6;
		nearest = 10;
		farthest = 300;
		hoopSize = 100;
		farthestStray = 25;
		numberOfEnemies = 3;

		leftWhole = 0;
		rightWhole = 2;
		leftNew = 1;
		rightNew = 3;
		currentPointer = 1;
		dataDimension = 2;

	var
		mainWndo: WindowPtr;

		offLeftVirginArea, offRightVirginArea, offLeftArea, offRightArea, offPlayerArea: Rect;
		offLeftVirginMap, offRightVirginMap, offLeftMap, offRightMap, offPlayerMap: BitMap;
		offLeftVirginPort, offRightVirginPort, offLeftPort, offRightPort, offPlayerPort: GrafPtr;
		offLeftVirginBits, offRightVirginBits, offLeftBits, offRightBits, offPlayerBits: Ptr;

		playMask, idleMask: Integer;
		leftScreenRgn, rightScreenRgn: RgnHandle;
		bckgrndDst, bckgrndSrc, smallBckgrndSrc, leftBckgrndDst, rightBckgrndDst: Rect;

		enemiesPassed, enemyMax, enemyMaxCycles, enemySpeed: Integer;
		enemyShot: array[1..numberOfEnemies] of Boolean;
		enemySize: array[1..10, 1..49] of Integer;
		enemySrc, enemyMaskSrc: array[0..8] of Rect;
		theEnemies: array[1..10, 0..3, 1..49] of Rect;
		enemyData: array[1..10, 1..49] of Integer;
		currentEnemies: array[1..3, 1..2] of Integer;

	function DoRandom (range: Integer): Integer;
	procedure IntroEffects;
	procedure DoTheVirgin;

implementation

{=================================}

	function DoRandom;
		var
			rawResult: LongInt;
	begin
		rawResult := Abs(Random);
		DoRandom := (rawResult * range) div 32768
	end;

{=================================}

	procedure IntroEffects;
		var
			index: Integer;
			dummyLong: LongInt;
			tempSrc, tempMask, tempLeft, tempRight, wholeLeft, wholeRight, tempRect: Rect;
	begin
		CopyBits(offLeftVirginPort^.portBits, offLeftPort^.portBits, offLeftVirginMap.bounds, offLeftVirginMap.bounds, srcCopy, nil);
		CopyBits(offRightVirginPort^.portBits, offRightPort^.portBits, offRightVirginMap.bounds, offRightVirginMap.bounds, srcCopy, nil);
		CopyBits(offLeftPort^.portBits, mainWndo^.portBits, offLeftMap.bounds, leftBckgrndDst, srcCopy, nil);
		CopyBits(offRightPort^.portBits, mainWndo^.portBits, offRightMap.bounds, rightBckgrndDst, srcCopy, nil);

		SetRect(tempSrc, 328, 103, 328 + 63, 103 + 35);
		SetPort(offPlayerPort);
		EraseRect(tempSrc);
		SetRect(tempMask, 1, 264, 64, 299);
		tempLeft := tempMask;
		OffsetRect(tempLeft, -1, -264);
		tempRight := tempLeft;
		OffsetRect(tempLeft, 33, -36);
		OffsetRect(tempRight, 31, -36);
		for index := 1 to 100 do
			begin
				wholeLeft := tempLeft;
				wholeLeft.top := wholeLeft.top - 1;
				wholeRight := tempRight;
				wholeRight.top := wholeRight.top - 1;
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
				OffsetRect(tempLeft, 0, 1);
				OffsetRect(tempRight, 0, 1);
			end;

		SetRect(tempSrc, 328, 103, 328 + 122, 103 + 67);
		EraseRect(tempSrc);
		SetRect(tempMask, 103, 255, 225, 322);
		tempLeft := tempMask;
		OffsetRect(tempLeft, -103, -255);
		tempRight := tempLeft;
		OffsetRect(tempLeft, 4, 241);
		OffsetRect(tempRight, 0, 241);
		for index := 1 to 130 do
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

		SetRect(tempSrc, 328, 103, 328 + 101, 103 + 22);
		EraseRect(tempSrc);
		SetRect(tempMask, 1, 300, 102, 322);
		tempLeft := tempMask;
		OffsetRect(tempLeft, -1, -300);
		tempRight := tempLeft;
		OffsetRect(tempLeft, 18, 241);
		OffsetRect(tempRight, 8, 241);
		for index := 1 to 40 do
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

		CopyBits(offLeftVirginMap, offLeftMap, offLeftVirginMap.bounds, offLeftVirginMap.bounds, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, offRightVirginMap.bounds, offRightVirginMap.bounds, srcCopy, nil);
		CopyBits(offLeftMap, mainWndo^.portBits, offLeftMap.bounds, leftBckgrndDst, srcCopy, nil);
		CopyBits(offRightMap, mainWndo^.portBits, offRightMap.bounds, rightBckgrndDst, srcCopy, nil);
	end;

{=================================}

	procedure DoTheVirgin;
		var
			tempRect, planetSrc, planetMask: Rect;
	begin
		CopyBits(offPlayerMap, offLeftVirginMap, smallBckGrndSrc, smallBckGrndSrc, srcCopy, nil);
		tempRect := smallBckGrndSrc;
		OffsetRect(tempRect, 127, 0);
		CopyBits(offPlayerMap, offRightVirginMap, tempRect, smallBckGrndSrc, srcCopy, nil);

		SetRect(planetSrc, 256, 121, 287, 152);
		SetRect(planetMask, 256, 153, 287, 184);
		tempRect := planetSrc;
		OffsetRect(tempRect, -256 + DoRandom(80), 60 + DoRandom(30));
		CopyMask(offPlayerMap, offPlayerMap, offLeftVirginMap, planetSrc, planetMask, tempRect);
		OffsetRect(tempRect, -2, 0);
		CopyMask(offPlayerMap, offPlayerMap, offRightVirginMap, planetSrc, planetMask, tempRect);
		SetRect(planetSrc, 288, 121, 298, 131);
		SetRect(planetMask, 299, 121, 309, 131);
		tempRect.right := tempRect.left + 10;
		tempRect.bottom := tempRect.top + 10;
		OffsetRect(tempRect, DoRandom(60) - 30, DoRandom(15));
		CopyMask(offPlayerMap, offPlayerMap, offLeftVirginMap, planetSrc, planetMask, tempRect);
		OffsetRect(tempRect, -4, 0);
		CopyMask(offPlayerMap, offPlayerMap, offRightVirginMap, planetSrc, planetMask, tempRect);

		CopyBits(offLeftVirginMap, offLeftMap, offLeftVirginMap.bounds, offLeftVirginMap.bounds, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, offRightVirginMap.bounds, offRightVirginMap.bounds, srcCopy, nil);
		CopyBits(offLeftMap, mainWndo^.portBits, offLeftMap.bounds, leftBckgrndDst, srcCopy, nil);
		CopyBits(offRightMap, mainWndo^.portBits, offRightMap.bounds, rightBckgrndDst, srcCopy, nil);
	end;

{=================================}

end.
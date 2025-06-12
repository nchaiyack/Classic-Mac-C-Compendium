unit GameUtils;

interface

	uses
		Sound, Palettes, Globals, Dialogs;

	procedure CenterZeroRect (var theRect: Rect);
	procedure NewLightning;
	procedure StrikeLightning (whichLevel: Integer);
	procedure DoTheSound (whichOne: Str255; priority: Integer);
	function DoRandom (range: Integer): Integer;
	procedure ChangeRect;
	procedure DrawPlayer (newRect, oldRect: Rect);
	procedure DrawBeasts;
	procedure DrawHand;
	procedure RedoTheBackground;
	procedure ShowScore;
	procedure ShowMortals;
	procedure ShowLevel;
	procedure CheckExtraMortal;
	procedure ReDrawHiScores;
	procedure ReadInScores;
	procedure WriteOutScores;
	procedure FinalScore;
	procedure DoHelpScreen;
	procedure FlushTheScores;

{=================================}

implementation

{=================================}

	procedure CenterZeroRect;
	begin
		OffsetRect(theRect, -theRect.left, -theRect.top);
		OffsetRect(theRect, (theRect.left - theRect.right) div 2, (theRect.top - theRect.bottom) div 2);
	end;
{=================================}

	procedure NewLightning;

		const
			obeliskHeight = 250;
			obel1left = 172;
			obel2left = 468;
			pylonToCenter = 148;

			upperHeight = 180;
			lowerHeight = 425;

			upperDelta = upperHeight - obeliskHeight;
			lowerDelta = lowerHeight - obeliskHeight;

			upperEyeDelta = upperEyeHeight - obeliskHeight;
			lowerEyeDelta = lowerEyeHeight - obeliskHeight;

		var
			index, index2, travel, hori, vert: Integer;

	begin

		with lightning do
			begin
				levelStriking := upperLevel;
				leftMode := DoRandom(4);
				rightMode := DoRandom(4);
				for index := 0 to 3 do
					begin
						SetPt(leftBolts[upperLevel, index, 0], obel1left, obeliskHeight);
						SetPt(rightBolts[upperLevel, index, 0], obel2left, obeliskHeight);
						SetPt(leftBolts[lowerLevel, index, 0], obel1left, obeliskHeight);
						SetPt(rightBolts[lowerLevel, index, 0], obel2left, obeliskHeight);
						SetPt(leftBolts[upperEye, index, 0], obel1left, obeliskHeight);
						SetPt(rightBolts[upperEye, index, 0], obel2left, obeliskHeight);
						SetPt(leftBolts[lowerEye, index, 0], obel1left, obeliskHeight);
						SetPt(rightBolts[lowerEye, index, 0], obel2left, obeliskHeight);
					end;
				for index := 0 to 3 do
					for index2 := 1 to 10 do
						begin
							travel := (6 - ABS(index2 - 6)) * 5;

							hori := obel1left + (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (upperDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(leftBolts[upperLevel, index, index2], hori, vert);

							hori := obel2left - (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (upperDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(rightBolts[upperLevel, index, index2], hori, vert);

							hori := obel1left + (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (lowerDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(leftBolts[lowerLevel, index, index2], hori, vert);

							hori := obel2left - (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (lowerDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(rightBolts[lowerLevel, index, index2], hori, vert);

							hori := obel1left + (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (upperEyeDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(leftBolts[upperEye, index, index2], hori, vert);

							hori := obel2left - (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (upperEyeDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(rightBolts[upperEye, index, index2], hori, vert);

							hori := obel1left + (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (lowerEyeDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(leftBolts[lowerEye, index, index2], hori, vert);

							hori := obel2left - (index2 * (pylonToCenter div 12));
							vert := obeliskHeight + (index2 * (lowerEyeDelta div 12));
							vert := vert + DoRandom(travel * 2) - travel;
							SetPt(rightBolts[lowerEye, index, index2], hori, vert);
						end;
				for index := 0 to 3 do
					begin
						SetPt(leftBolts[upperLevel, index, 11], 320, upperHeight);
						SetPt(rightBolts[upperLevel, index, 11], 318, upperHeight);
						SetPt(leftBolts[lowerLevel, index, 11], 320, lowerHeight);
						SetPt(rightBolts[lowerLevel, index, 11], 318, lowerHeight);
						SetPt(leftBolts[upperEye, index, 11], 320, upperEyeHeight);
						SetPt(rightBolts[upperEye, index, 11], 318, upperEyeHeight);
						SetPt(leftBolts[lowerEye, index, 11], 320, lowerEyeHeight);
						SetPt(rightBolts[lowerEye, index, 11], 318, lowerEyeHeight);
					end;
			end;
	end;

{=================================}

	procedure StrikeLightning;
		var
			index: Integer;
			isPoint, wasPoint: Point;
			savedPort: GrafPtr;
	begin
		GetPort(savedPort);
		SetPort(GrafPtr(mainWndo));

		InvertRgn(obeliskRgn1);
		InvertRgn(obeliskRgn2);
		PenNormal;
		PenSize(2, 2);
		PenMode(patXOr);
		with lightning do
			begin
				leftMode := DoRandom(4);
				rightMode := DoRandom(4);
				for index := 1 to 11 do
					begin
						isPoint := leftBolts[whichLevel, leftMode, index];
						wasPoint := leftBolts[whichLevel, leftMode, index - 1];
						MoveTo(wasPoint.h, wasPoint.v);
						LineTo(isPoint.h - 2, isPoint.v);
						isPoint := rightBolts[whichLevel, leftMode, index];
						wasPoint := rightBolts[whichLevel, leftMode, index - 1];
						MoveTo(wasPoint.h, wasPoint.v);
						LineTo(isPoint.h + 2, isPoint.v);
					end;
				for index := 1 to 11 do
					begin
						isPoint := leftBolts[whichLevel, leftMode, index];
						wasPoint := leftBolts[whichLevel, leftMode, index - 1];
						MoveTo(wasPoint.h, wasPoint.v);
						LineTo(isPoint.h - 2, isPoint.v);
						isPoint := rightBolts[whichLevel, leftMode, index];
						wasPoint := rightBolts[whichLevel, leftMode, index - 1];
						MoveTo(wasPoint.h, wasPoint.v);
						LineTo(isPoint.h + 2, isPoint.v);
					end;
			end;
		InvertRgn(obeliskRgn1);
		InvertRgn(obeliskRgn2);
		SetPort(GrafPtr(savedPort));
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
			theSnd: Handle;
			err: OSErr;
	begin
		if (not soundOn) then
			Exit(DoTheSound);

		if (priority < soundPriority) then
			Exit(DoTheSound);

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

		err := SndNewChannel(chanPtr, 0, initMono + $0004, @ChanCallBack);
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

		if (err = noErr) then
			soundPriority := priority
		else
			soundPriority := noSound;
	end;

{=================================}

	function DoRandom;
		var
			rawResult: LongInt;
	begin
		rawResult := Abs(Random);
		DoRandom := (rawResult * range) div 32768
	end;

{=================================}

	procedure ChangeRect;
		var
			hori, vert: Integer;
	begin
		with thePlayer do
			begin
				hori := dest.left;
				vert := dest.top;
				if (mode < 6) then
					begin
						dest := playerRects[facing, mode];
						OffsetRect(dest, -dest.left, -dest.top);
					end
				else
					begin
						SetRect(dest, 0, 0, 45, 22);		{pile of bones rect}
						dest.top := dest.top + 17;			{ h diff from skeleton to bone pile}
						dest.bottom := dest.bottom + 17;
					end;
				dest.left := dest.left + hori;
				dest.right := dest.right + hori;
				dest.top := dest.top + vert;
				dest.bottom := dest.bottom + vert;
			end;
	end;

{=================================}

	procedure DrawPlayer;
		var
			index, hori, vert: integer;
			dummyLong: LongInt;
			standInRect, standInRect2, tempRect: Rect;
	begin
		with thePlayer do
			case mode of
				0..5: 
					begin
						UnionRect(oldRect, newRect, wholeRect);
						CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, wholeRect, wholeRect, srcCopy, nil);
						CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, playerRects[facing, mode], playerRects[facing, mode], newRect);
						CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);
						if (otherState) then		{brief flash as player dies and turns to a falling skeleton}
							begin
								mode := 6;
								CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);
								hori := newRect.left;
								vert := newRect.top;
								newRect := boneRects[0, 6];
								OffsetRect(newRect, -newRect.left, -newRect.top);
								tempRect := newRect;
								OffsetRect(newRect, hori, vert);
								dest := newRect;
								if (dest.bottom < 200) then
									OffsetRect(tempRect, 0, 200);
								SetPort(GrafPtr(loadCPtr));
								for index := 1 to 10 do
									begin
										EraseRect(tempRect);
										CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, newRect, tempRect, srcCopy, nil);
										CopyMask(BitMapPtr(loadCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, tempRect, boneRects[facing, mode], newRect);
										CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, newRect, newRect, srcCopy, playRgn);
										Delay(1, dummyLong);
										CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, newRect, newRect, srcCopy, nil);
										CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, boneRects[facing, mode], boneRects[facing, mode], newRect);
										CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, newRect, newRect, srcCopy, playRgn);
									end;
								CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, newRect, newRect, srcCopy, playRgn);
							end;
					end;
				6: 					{falling skeleton}
					begin
						UnionRect(oldRect, newRect, wholeRect);
						CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, wholeRect, wholeRect, srcCopy, nil);
						CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, boneRects[facing, mode], boneRects[facing, mode], newRect);
						CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);
					end;
				7..100: 		{sinking bones}
					begin
						UnionRect(oldRect, newRect, wholeRect);
						CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, wholeRect, wholeRect, srcCopy, nil);
						standInRect := boneRects[facing, 7];
						standInRect2 := boneRects[facing, 7];
						vert := dest.bottom - dest.top;
						standInRect.bottom := standInRect.top + vert;
						standInRect2.bottom := standInRect2.top + vert;
						CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, standInRect, standInRect2, newRect);
						CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);
					end;
				otherwise
					begin
					end;
			end;
	end;

{=================================}

	procedure DrawHand;
	begin
		with theHand do
			begin
				UnionRect(oldDest, dest, wholeRect);
				CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, wholeRect, wholeRect, srcCopy, nil);
				CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, handRects[mode], handRects[mode], dest);
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);
			end;
	end;

{=================================}

	procedure DrawBeasts;
		var
			index: Integer;
			otherRect, otherMask, tempSrc, tempMask: Rect;
	begin

		if (levelOn < 11) then	{swap flames on background}
			begin
				CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, flameRect[0], flameRect[0], srcCopy, nil);
				CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, flameRect[1], flameRect[0], srcCopy, nil);
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, flameRect[0], flameRect[1], srcCopy, nil);
			end;

{copy all neccesary background rects to the load map}
		with theEye do
			if (mode > 0) then
				begin
					UnionRect(oldDest, dest, wholeRect);
					CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, wholeRect, wholeRect, srcCopy, nil);
				end;

		for index := 1 to numberOfEnemies do
			with theEnemies[index] do
				begin
					UnionRect(oldDest, dest, wholeRect);
					CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, BitMapPtr(loadCPtr^.portPixMap^)^, wholeRect, wholeRect, srcCopy, nil);
					oldDest := dest;
				end;

{copy the beasts through their masks onto the load map}
		for index := 1 to numberOfEnemies do
			with theEnemies[index] do
				case mode of
					-1000..-1: 
						begin
							tempSrc := eggRects;
							tempMask := eggRects;
							tempSrc.bottom := tempSrc.top + (dest.bottom - dest.top);
							tempMask.bottom := tempMask.top + (dest.bottom - dest.top);
							CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, tempSrc, tempMask, dest);
						end;
					0..5: 
						CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, enemyRects[otherMode, mode, facing], enemyRects[otherMode, mode, facing], dest);
					6: 
						begin
							otherRect := enemyRects[otherMode, 1, facing];
							otherRect.bottom := otherRect.top + (dest.bottom - dest.top);
							otherMask := enemyRects[otherMode, 1, facing];
							otherMask.bottom := otherMask.top + (dest.bottom - dest.top);
							CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, otherRect, otherMask, dest);
						end;
					otherwise
						;
				end;	{case}

		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, flameRect[0], flameRect[0], srcCopy, nil);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, flameRect[1], flameRect[1], srcCopy, nil);
		{copy the sum bitmaps to the screen}
		for index := 1 to numberOfEnemies do
			with theEnemies[index] do
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);

		with theEye do
			if (mode > 0) then
				begin
					CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, BitMapPtr(loadCPtr^.portPixMap^)^, eyeRects[otherMode], eyeRects[otherMode], dest);
					CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, wholeRect, wholeRect, srcCopy, playRgn);
					oldDest := dest;
				end;
	end;

{=================================}

	procedure RedoTheBackground;
		var
			tempRect, anotherRect: Rect;
			thePict: PicHandle;

	begin
		SetPort(GrafPtr(virginCPtr));						{Set the port to my window}
		thePict := GetPicture(backPictID);			{Get Picture into memory}
		if (thePict <> nil) then								{Only use handle if it is valid}
			begin
				ClipRect(wholeArea);								{Clip picture to this rectangle}
				HLock(Handle(thePict));							{Lock the handle before using it}
				tempRect := thePict^^.picFrame;
				DrawPicture(thePict, tempRect);
				HUnLock(Handle(thePict));						{Unlock the picture again}
				ReleaseResource(Handle(thePict));
			end;

		if (numberOfStones > 2) then
			begin			{draw top floor end stones}
				tempRect := longStoneSrc;
				OffsetRect(tempRect, -tempRect.left, -tempRect.top);	{Get it in upper left}
				OffsetRect(tempRect, tombRects[3].left, tombRects[3].top);
				CopyBits(BitMapPtr(objectCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, longStoneSrc, tempRect, srcCopy, nil);

				tempRect := longStoneSrc;
				OffsetRect(tempRect, -tempRect.left, -tempRect.top);	{Get it in upper left}
				OffsetRect(tempRect, tombRects[4].left, tombRects[4].top);
				CopyBits(BitMapPtr(objectCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, longStoneSrc, tempRect, srcCopy, nil);
			end;

		if ((numberOfStones > 4) and (levelOn <> 3)) then
			begin
				CopyBits(BitMapPtr(objectCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, shortStoneSrc, tombRects[5], srcCopy, nil);
			end;

		if (levelOn < 3) then
			begin							{put covers over acid pools}
				tempRect := longStoneSrc;
				OffsetRect(tempRect, -tempRect.left, -tempRect.top);	{Get it in upper left}
				OffsetRect(tempRect, tombRects[-5].left, tombRects[-5].top);
				CopyBits(BitMapPtr(objectCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, longStoneSrc, tempRect, srcCopy, playRgn);

				tempRect := longStoneSrc;
				OffsetRect(tempRect, -tempRect.left, -tempRect.top);	{Get it in upper left}
				OffsetRect(tempRect, tombRects[-4].left, tombRects[-4].top);
				CopyBits(BitMapPtr(objectCPtr^.portPixMap^)^, BitMapPtr(virginCPtr^.portPixMap^)^, longStoneSrc, tempRect, srcCopy, playRgn);
			end;

		if (levelOn > 10) then
			begin
				SetPort(GrafPtr(virginCPtr));
				FillRect(flameRect[0], black);
				FillRect(flameRect[1], black);
			end;

		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, wholeArea, wholeArea, srcCopy, mainWndo^.visRgn);
		ShowScore;
		ShowMortals;
		ShowLevel;
	end;

{=================================}

	procedure ShowScore;
		var
			wasPort: GrafPtr;
			tempStr: Str255;
			tempRect: Rect;
	begin
		GetPort(wasPort);
		SetPort(GrafPtr(mainWndo));
		SetRect(tempRect, 268, 438, 364, 458);
		TextFont(0);
		TextSize(12);
		NumToString(score, tempStr);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, tempRect, tempRect, srcCopy, mainWndo^.visRgn);
		RGBForeColor(rgbWhite);
		PenMode(srcOr);
		MoveTo(270, 456);
		DrawString(tempStr);
		RGBForeColor(rgbBlack);
		SetPort(GrafPtr(wasPort));
	end;

{=================================}

	procedure ShowMortals;
		var
			wasPort: GrafPtr;
			tempStr: Str255;
			tempRect: Rect;
	begin
		GetPort(wasPort);
		SetPort(GrafPtr(mainWndo));
		SetRect(tempRect, 218, 438, 266, 458);
		TextFont(0);
		TextSize(12);
		NumToString(mortals, tempStr);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, tempRect, tempRect, srcCopy, mainWndo^.visRgn);
		RGBForeColor(rgbWhite);
		PenMode(srcOr);
		MoveTo(220, 456);
		DrawString(tempStr);
		RGBForeColor(rgbBlack);
		SetPort(GrafPtr(wasPort));
	end;

{=================================}

	procedure ShowLevel;
		var
			wasPort: GrafPtr;
			tempStr: Str255;
			tempRect: Rect;
	begin
		GetPort(wasPort);
		SetPort(GrafPtr(mainWndo));
		SetRect(tempRect, 378, 438, 426, 458);
		TextFont(0);
		TextSize(12);
		NumToString(levelOn, tempStr);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, tempRect, tempRect, srcCopy, mainWndo^.visRgn);
		RGBForeColor(rgbWhite);
		PenMode(srcOr);
		MoveTo(380, 456);
		DrawString(tempStr);
		RGBForeColor(rgbBlack);
		SetPort(GrafPtr(wasPort));
	end;

{=================================}

	procedure CheckExtraMortal;
	begin
		if (score > nextMortal) then
			begin
				DoTheSound('bonus.snd', highPriority);	{You got an extra player!}
				mortals := mortals + 1;
				ShowMortals;
				nextMortal := nextMortal + newMortalPts;
			end;
	end;

{=================================}

	procedure ReDrawHiScores;
		var
			index, heightIs: Integer;
			dummyLong: LongInt;
			tempRect: Rect;
			dummyString: Str255;
	begin
		SetPort(GrafPtr(loadCPtr));
		PenNormal;
		SetRect(tempRect, 207, 168, 434, 388);
		FillRect(tempRect, black);
		TextFont(1);
		TextSize(9);
		for index := 1 to 10 do	{Now we're going to loop through all the scores	}
			begin
				RGBForeColor(rgbYellow);
				MoveTo(tempRect.left + 20, index * 14 + tempRect.top + 50);
				DrawString(hiStrings[index]);							{Draw the name of person}
				RGBForeColor(rgbRed);
				MoveTo(370, index * 14 + tempRect.top + 50);
				NumToString(hiScores[index], dummyString);
				DrawString(dummyString);									{And draw their score}
			end;																				{Copy screen to virgin for updates}
		RGBForeColor(rgbBlack);
		heightIs := tempRect.bottom - tempRect.top;
		tempRect.top := tempRect.bottom;
		for index := 1 to heightIs div 4 do
			begin
				tempRect.top := tempRect.top - 4;
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, tempRect, tempRect, srcCopy, mainWndo^.visRgn);
			end;

		SetPort(GrafPtr(mainWndo));
		TextFont(0);
		TextSize(12);
		PenMode(srcOr);
		RGBForeColor(rgbLtBlue);
		MoveTo(tempRect.left + 20, tempRect.top + 30);
		DrawString('Glypha II 1.2 High Scores');
		RGBForeColor(rgbBlack);

		CopyBits(mainWndo^.portBits, BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portRect, mainWndo^.portRect, srcCopy, nil);
	end;

{=================================}

	procedure ReadInScores;
		type
			scoreHandle = ^scorePtr;
			scorePtr = ^score;
			score = record
					data: array[0..9] of LongInt;
				end;

			nameHandle = ^namePtr;
			namePtr = ^name;
			name = record
					data: array[0..9, 0..14] of Char;
				end;

			prefHandle = ^prefPtr;
			prefPtr = ^pref;
			pref = record
					data: array[0..31] of Char;
				end;

		var
			index, index2, tempVolume: Integer;
			dummyStr: Str255;
			theScores: scoreHandle;
			theNames: nameHandle;
			thePrefs: prefHandle;

	begin
		theScores := scoreHandle(NewHandle(SIZEOF(score)));
		if (theScores = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		MoveHHi(Handle(theScores));
		HLock(Handle(theScores));
		Handle(theScores) := GetResource('scrs', 128);
		for index := 0 to 9 do
			hiScores[index + 1] := theScores^^.data[index];
		HUnlock(Handle(theScores));
		ReleaseResource(Handle(theScores));
		DisposHandle(Handle(theScores));

		theNames := nameHandle(NewHandle(SIZEOF(name)));
		if (theNames = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		MoveHHi(Handle(theNames));
		HLock(Handle(theNames));
		Handle(theNames) := GetResource('name', 128);
		for index := 0 to 9 do
			begin
				dummyStr := '';
				for index2 := 0 to 14 do
					dummyStr := CONCAT(dummyStr, theNames^^.data[index, index2]);
				hiStrings[index + 1] := dummyStr;
			end;
		HUnlock(Handle(theNames));
		ReleaseResource(Handle(theNames));
		DisposHandle(Handle(theNames));

		thePrefs := prefHandle(NewHandle(SIZEOF(pref)));
		if (thePrefs = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		MoveHHi(Handle(thePrefs));
		HLock(Handle(thePrefs));
		Handle(thePrefs) := GetResource('pref', 128);

		if (thePrefs^^.data[0] = 'M') then
			keyboardControl := FALSE
		else
			keyboardControl := TRUE;
		if (thePrefs^^.data[1] = 'S') then
			soundOn := TRUE
		else
			soundOn := FALSE;
		if (inhibitSound) then
			soundOn := FALSE;
		GetSoundVol(tempVolume);
		if (tempVolume = 0) then
			soundOn := FALSE;

		case thePrefs^^.data[2] of
			'S': 
				gameSpeed := slow;
			'F': 
				gameSpeed := fast;
			otherwise
				gameSpeed := fast;
		end;

		nameUsing := '';
		for index := 1 to 15 do
			nameUsing := CONCAT(nameUsing, thePrefs^^.data[index + 16]);

		HUnlock(Handle(thePrefs));
		ReleaseResource(Handle(thePrefs));
		DisposHandle(Handle(thePrefs));

		scoresChanged := FALSE;
	end;

{=================================}

	procedure WriteOutScores;
		type
			scoreHandle = ^scorePtr;
			scorePtr = ^score;
			score = record
					data: array[0..9] of LongInt;
				end;

			nameHandle = ^namePtr;
			namePtr = ^name;
			name = record
					data: array[0..9, 0..14] of Char;
				end;

			prefHandle = ^prefPtr;
			prefPtr = ^pref;
			pref = record
					data: array[0..31] of Char;
				end;

		var
			index, index2: Integer;
			dummyStr: Str255;
			theScores: scoreHandle;
			theNames: nameHandle;
			thePrefs: prefHandle;

	begin
		thePrefs := prefHandle(NewHandle(SIZEOF(pref)));
		if (thePrefs = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		HLock(Handle(thePrefs));
		Handle(thePrefs) := GetResource('pref', 128);
		if (keyboardControl) then
			thePrefs^^.data[0] := 'K'
		else
			thePrefs^^.data[0] := 'M';
		if (soundOn) then
			thePrefs^^.data[1] := 'S'
		else
			thePrefs^^.data[1] := 'N';

		if (gameSpeed = slow) then
			thePrefs^^.data[2] := 'S'
		else
			thePrefs^^.data[2] := 'F';

		for index := 1 to 15 do
			thePrefs^^.data[index + 16] := COPY(nameUsing, index, 1);
		ChangedResource(Handle(thePrefs));
		WriteResource(Handle(thePrefs));
		HUnlock(Handle(thePrefs));
		ReleaseResource(Handle(thePrefs));
		DisposHandle(Handle(thePrefs));

		if (scoresChanged) then
			begin
				theScores := scoreHandle(NewHandle(SIZEOF(score)));
				if (theScores = nil) then
					begin
						GenericAlert(4);
						ExitToShell;
					end;
				MoveHHi(Handle(theScores));
				HLock(Handle(theScores));
				Handle(theScores) := GetResource('scrs', 128);
				for index := 0 to 9 do
					theScores^^.data[index] := hiScores[index + 1];
				ChangedResource(Handle(theScores));
				WriteResource(Handle(theScores));
				HUnlock(Handle(theScores));
				ReleaseResource(Handle(theScores));
				DisposHandle(Handle(theScores));

				theNames := nameHandle(NewHandle(SIZEOF(name)));
				if (theNames = nil) then
					begin
						GenericAlert(4);
						ExitToShell;
					end;
				HLock(Handle(theNames));
				Handle(theNames) := GetResource('name', 128);
				for index := 0 to 9 do
					for index2 := 0 to 14 do
						theNames^^.data[index, index2] := COPY(hiStrings[index + 1], index2 + 1, 1);
				ChangedResource(Handle(theNames));
				WriteResource(Handle(theNames));
				HUnlock(Handle(theNames));
				ReleaseResource(Handle(theNames));
				DisposHandle(Handle(theNames));
			end;
	end;

{=================================}

	procedure FinalScore;
		var
			index, ranking: Integer;
	begin
		FlushEvents(everyEvent, 0);									{Clear all the keystrokes	}
		if (score > hiScores[10]) and (mortalsStart = defaultNum) then		{Is it a high score?			}
			begin
				scoresChanged := TRUE;						{Mark scores as changed			}
				DoTheSound('bonus.snd', highPriority);		{Play the bonus sound.			}
				ranking := 10;										{And sort through the high	}
				for index := 9 downto 1 do				{scores to find the players	}
					begin														{ranking.										}
						if (score > hiScores[index]) then
							ranking := index;
					end;
				for index := 10 downto ranking + 1 do
					begin
						hiScores[index] := hiScores[index - 1];		{Move everyone's score}
						hiStrings[index] := hiStrings[index - 1];	{down the list.				}
					end;
				hiScores[ranking] := score;
				SetPort(GrafPtr(mainWndo));
				WhosHiScore(nameUsing);
				hiStrings[ranking] := nameUsing;
			end;
	end;

{=================================}

	procedure DoHelpScreen;
		const
			helpLeft = 208;
			helpTop = 170;
		var
			index: Integer;
			srcRect, dropRect, tempRect: Rect;
			thePict: PicHandle;
	begin
		SetPort(GrafPtr(loadCPtr));
		thePict := GetPicture(helpPictID);
		SetRect(dropRect, 0, 0, 452, 440);
		if (thePict <> nil) then
			begin
				HLock(Handle(thePict));
				tempRect := thePict^^.picFrame;
				DrawPicture(thePict, tempRect);
				HUnLock(Handle(thePict));
				ReleaseResource(Handle(thePict));
			end
		else
			Exit(DoHelpScreen);

		SetRect(dropRect, 0 + helpLeft, helpTop + 220, 226 + helpLeft, helpTop + 220);
		SetRect(srcRect, 0, 0, 226, 0);
		for index := 1 to 110 do
			begin
				dropRect.top := dropRect.top - 2;
				srcRect.bottom := srcRect.bottom + 2;
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, srcRect, dropRect, srcCopy, mainWndo^.visRgn);
			end;
		repeat
		until not Button;
		repeat
		until Button;

		SetRect(dropRect, 0 + helpLeft, helpTop + 220, 226 + helpLeft, helpTop + 220);
		SetRect(srcRect, 226, 0, 452, 0);
		for index := 1 to 110 do
			begin
				dropRect.top := dropRect.top - 2;
				srcRect.bottom := srcRect.bottom + 2;
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, srcRect, dropRect, srcCopy, mainWndo^.visRgn);
			end;
		repeat
		until not Button;
		repeat
		until Button;

		SetRect(dropRect, 0 + helpLeft, helpTop + 220, 226 + helpLeft, helpTop + 220);
		SetRect(srcRect, 226, 220, 452, 220);
		for index := 1 to 110 do
			begin
				dropRect.top := dropRect.top - 2;
				srcRect.bottom := srcRect.bottom + 2;
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, srcRect, dropRect, srcCopy, mainWndo^.visRgn);
			end;
		repeat
		until not Button;
		repeat
		until Button;

		SetRect(dropRect, 0 + helpLeft, helpTop + 220, 226 + helpLeft, helpTop + 220);
		SetRect(srcRect, 0, 220, 226, 220);
		for index := 1 to 110 do
			begin
				dropRect.top := dropRect.top - 2;
				srcRect.bottom := srcRect.bottom + 2;
				CopyBits(BitMapPtr(loadCPtr^.portPixMap^)^, mainWndo^.portBits, srcRect, dropRect, srcCopy, mainWndo^.visRgn);
			end;
		repeat
		until not Button;
		repeat
		until Button;

		FlushEvents(everyEvent, 0);
		RedrawHiScores;
	end;

{=================================}

	procedure FlushTheScores;
		var
			index: Integer;
	begin
		for index := 1 to 10 do
			begin
				hiScores[index] := 0;
				hiStrings[index] := 'Play Me .......';
			end;
		scoresChanged := TRUE;
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, wholeArea, wholeArea, srcCopy, mainWndo^.visRgn);
		RedrawHiScores;
	end;

{=================================}

end.
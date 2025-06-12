unit Initialize;

interface

	uses
		Sound, SomeGlobals, Utilities;

	const
		objectPictID = 2001;

	var
		AppleMenu: MenuHandle;

	procedure Init_My_Menus;
	function NewBitMap (var theBitMap: BitMap; theRect: Rect): Ptr;
	procedure InitVariables;

implementation

	procedure Init_My_Menus;                {Initialize the menus}
		var
			tempMenu: MenuHandle;                  {Throw away all other menu handles}

	begin                                   {Start of Init_My_Menus}
		ClearMenuBar;                         {Clear any old menu bars}
		tempMenu := GetMenu(AppleM);
		AddResMenu(tempMenu, 'DRVR');
		InsertMenu(tempMenu, 0);
		AppleMenu := tempMenu;
		tempMenu := GetMenu(GameM);
		InsertMenu(tempMenu, 0);
		tempMenu := GetMenu(OptionsM);
		InsertMenu(tempMenu, 0);

		DisableItem(GetMenu(GameM), 2);
		DisableItem(GetMenu(GameM), 3);

		DrawMenuBar;
	end;

{=================================}

	function NewBitMap;
	begin
		with theBitMap, theRect do
			begin
				rowBytes := ((right - left + 15) div 16) * 2;
				baseAddr := NewPtr(rowBytes * (bottom - top));
				bounds := theRect;
				if MemError <> noErr then
					begin
						NewBitMap := nil
					end
				else
					NewBitMap := baseAddr;
			end;
	end;

{=================================}

	procedure InitVariables;
		var
			index: Integer;
			rawPointer: Ptr;
			Pic_Handle: PicHandle;
			tempRect: Rect;
			theSnd: Handle;
	begin
		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offLeftPort := GrafPtr(rawPointer);
		OpenPort(offLeftPort);
		SetRect(offLeftArea, 0, 0, 127, 241);
		offLeftBits := NewBitMap(offLeftMap, offLeftArea);
		SetPortBits(offLeftMap);
		EraseRect(offLeftMap.bounds);
		ClipRect(offLeftArea);

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offRightPort := GrafPtr(rawPointer);
		OpenPort(offRightPort);
		SetRect(offRightArea, 0, 0, 127, 241);
		offRightBits := NewBitMap(offRightMap, offRightArea);
		SetPortBits(offRightMap);
		EraseRect(offRightMap.bounds);
		ClipRect(offRightArea);

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offLeftVirginPort := GrafPtr(rawPointer);
		OpenPort(offLeftVirginPort);
		SetRect(offLeftVirginArea, 0, 0, 127, 241);
		offLeftVirginBits := NewBitMap(offLeftVirginMap, offLeftVirginArea);
		SetPortBits(offLeftVirginMap);
		EraseRect(offLeftVirginMap.bounds);
		ClipRect(offLeftVirginArea);

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offRightVirginPort := GrafPtr(rawPointer);
		OpenPort(offRightVirginPort);
		SetRect(offRightVirginArea, 0, 0, 127, 241);
		offRightVirginBits := NewBitMap(offRightVirginMap, offRightVirginArea);
		SetPortBits(offRightVirginMap);
		EraseRect(offRightVirginMap.bounds);
		ClipRect(offRightVirginArea);

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offPlayerPort := GrafPtr(rawPointer);
		OpenPort(offPlayerPort);
		SetRect(offPlayerArea, 0, 0, 512, 322);
		offPlayerBits := NewBitMap(offPlayerMap, offPlayerArea);
		SetPortBits(offPlayerMap);
		EraseRect(offPlayerMap.bounds);
		SetPort(offPlayerPort);
		Pic_Handle := GetPicture(objectPictID);
		SetRect(tempRect, 0, 0, 512, 322);
		if (Pic_Handle <> nil) then
			begin
				ClipRect(tempRect);
				HLock(Handle(Pic_Handle));
				tempRect.Right := tempRect.Left + (Pic_Handle^^.picFrame.Right - Pic_Handle^^.picFrame.Left);
				tempRect.Bottom := tempRect.Top + (Pic_Handle^^.picFrame.Bottom - Pic_Handle^^.picFrame.Top);
				HUnLock(Handle(Pic_Handle));
			end;
		if (Pic_Handle <> nil) then
			DrawPicture(Pic_Handle, tempRect);
		ReleaseResource(Handle(Pic_Handle));
		SetRect(tempRect, 0, 0, 1023, 1023);
		ClipRect(tempRect);

		if forgetSound then
			soundOn := FALSE
		else
			soundOn := TRUE;
		slowOn := FALSE;
		playing := FALSE;
		pausing := FALSE;
		chanPtr := nil;
		GetDateTime(RandSeed);

		playMask := EveryEvent - MDownMask - MUpMask - KeyUpMask - AutoKeyMask - UpdateMask - ActivMask;
		idleMask := EveryEvent - KeyUpMask - AutoKeyMask;

		for index := 1 to 36 do
			begin
				heightRatio[index] := ABS(SIN(((index - 1) * 10) * (6.28318 / 360)));
			end;

		SetRect(bckgrndSrc, 0, 0, 254, 241);
		bckgrndDst := bckgrndSrc;
		OffsetRect(bckgrndDst, 129, 50);
		SetRect(leftBckgrndDst, 0, 0, 127, 241);
		rightBckgrndDst := leftBckgrndDst;
		smallBckGrndSrc := leftBckgrndDst;
		OffsetRect(leftBckgrndDst, 129, 50);
		OffsetRect(rightBckgrndDst, 256, 50);

		leftScreenRgn := NewRgn;
		MoveTo(129, 50);
		OpenRgn;
		Line(127, 0);
		Line(0, 241);
		Line(-127, 0);
		Line(0, -241);
		CloseRgn(leftScreenRgn);
		MoveHHi(Handle(leftScreenRgn));
		HLock(Handle(leftScreenRgn));

		rightScreenRgn := NewRgn;
		MoveTo(256, 50);
		OpenRgn;
		Line(127, 0);
		Line(0, 241);
		Line(-127, 0);
		Line(0, -241);
		CloseRgn(rightScreenRgn);
		MoveHHi(Handle(rightScreenRgn));
		HLock(Handle(rightScreenRgn));

		SetRect(bigClip, -1000, -1000, 1512, 1342);

		SetRect(shieldSrc, 20, 14, 40, 24);
		leftShieldDst := shieldSrc;
		OffsetRect(leftShieldDst, 129, 50);
		rightShieldDst := shieldSrc;
		OffsetRect(rightShieldDst, 256, 50);

		SetRect(crossHairSrc[0], 442, 1, 453, 12);
		SetRect(crossHairMask[0], 430, 1, 441, 12);
		SetRect(crossHairSrc[2], 492, 1, 503, 12);
		SetRect(crossHairMask[2], 456, 1, 467, 12);
		SetRect(crossHairSrc[1], 492, 1, 503, 12);
		SetRect(crossHairMask[1], 468, 1, 479, 12);

		SetRect(shipSrc[0], 256, 1, 291, 24);
		SetRect(shipSrc[1], 256, 25, 291, 48);
		SetRect(shipSrc[2], 256, 49, 291, 72);
		SetRect(shipSrc[3], 256, 73, 291, 96);
		SetRect(shipSrc[4], 256, 97, 291, 120);

		SetRect(shipMaskSrc[0], 292, 1, 327, 24);
		SetRect(shipMaskSrc[1], 292, 25, 327, 48);
		SetRect(shipMaskSrc[2], 292, 49, 327, 72);
		SetRect(shipMaskSrc[3], 292, 73, 327, 96);
		SetRect(shipMaskSrc[4], 292, 97, 327, 120);

		SetRect(enemySrc[0], 453, 26, 481, 58);
		SetRect(enemySrc[1], 453, 59, 481, 91);
		SetRect(enemySrc[2], 453, 92, 481, 124);
		SetRect(enemySrc[3], 453, 125, 481, 157);
		SetRect(enemySrc[4], 453, 158, 481, 190);
		SetRect(enemySrc[5], 453, 191, 481, 223);
		SetRect(enemySrc[6], 453, 224, 481, 256);
		SetRect(enemySrc[7], 453, 257, 481, 289);
		SetRect(enemySrc[8], 453, 290, 481, 322);

		SetRect(enemyMaskSrc[0], 484, 26, 512, 58);
		SetRect(enemyMaskSrc[1], 484, 59, 512, 91);
		SetRect(enemyMaskSrc[2], 484, 92, 512, 124);
		SetRect(enemyMaskSrc[3], 484, 125, 512, 157);
		SetRect(enemyMaskSrc[4], 484, 158, 512, 190);
		SetRect(enemyMaskSrc[5], 484, 191, 512, 223);
		SetRect(enemyMaskSrc[6], 484, 224, 512, 256);
		SetRect(enemyMaskSrc[7], 484, 257, 512, 289);
		SetRect(enemyMaskSrc[8], 484, 290, 512, 322);

		theSnd := GetNamedResource('snd ', 'laserFire');
		MoveHHi(theSnd);
		theSnd := GetNamedResource('snd ', 'enemyFire');
		MoveHHi(theSnd);
		theSnd := GetNamedResource('snd ', 'hyperSpace');
		MoveHHi(theSnd);
		theSnd := GetNamedResource('snd ', 'enemyBoom');
		MoveHHi(theSnd);
		theSnd := GetNamedResource('snd ', 'enterHole');
		MoveHHi(theSnd);
	end;

{=================================}

end.
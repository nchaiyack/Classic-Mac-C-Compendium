unit Initialize;

interface
	uses
		Sound, Palettes, Globals, AboutWndo, Dialogs, GameUtils, Enemies, GlyphaGuts;

	procedure InitVariables;

implementation

{=================================}

	procedure InitVariables;
		var
			sizeOfOff, offRowBytes, dummyLong: LongInt;
			i, i2, theDepth, howManySounds, vertOrigin: Integer;
			rawPointer: Ptr;
			tempByte: SignedByte;
			thePict: PicHandle;
			tempRect: Rect;
			theSnd: Handle;
			ignore: Boolean;

{------------------}

		procedure CheckOurEnvirons;
			var
				err: OSErr;
				thisWorld: SysEnvRec;
				theDevice: GDHandle;
		begin
			rightOffset := (ScreenBits.bounds.right - 640) div 2;
			downOffset := (ScreenBits.bounds.bottom - 480) div 2;

			err := SysEnvirons(1, thisWorld);	{Check set up the Mac game is on	}
			with thisWorld do
				begin
					if (not HasColorQD) then
						begin
							GenericAlert(1);
							ExitToShell;
						end;
					if (systemVersion < $0602) then
						inhibitSound := TRUE		{Global to prevent sound on older	}
					else											{versions of the System.					}
						inhibitSound := FALSE;	{Or, we allow sound.							}
				end;
			if ((screenBits.bounds.right < 640) or (screenBits.bounds.bottom < 400)) then
				begin
					GenericAlert(2);
					ExitToShell;
				end;
			theDevice := GetMainDevice;
			HLock(Handle(theDevice));
			if (theDevice^^.gdPMap^^.pixelSize <> 4) then
				begin
					HUnlock(Handle(theDevice));
					GenericAlert(3);
					ExitToShell;
				end;
			HUnlock(Handle(theDevice));

			hasWNE := (NGetTrapAddress(WNETrapNum, ToolTrap) <> NGetTrapAddress(unimplTrapNum, toolTrap));
		end;

{------------------}

		procedure Init_My_Menus;
			var
				aMenu: MenuHandle;
		begin
			ClearMenuBar;									{Clear any old menu bars}
			aMenu := GetMenu(mApple);			{Get the menu from the resource file}
			if (aMenu <> nil) then
				begin
					AddResMenu(aMenu, 'DRVR');{Add in DAs}
					InsertMenu(aMenu, 0);			{Insert this menu into the menu bar}
				end
			else
				begin
					GenericAlert(4);
					ExitToShell;
				end;
			aMenu := GetMenu(mGame);			{Get the menu from the resource file}
			if (aMenu <> nil) then
				InsertMenu(aMenu, 0)				{Insert this menu into the menu bar}
			else
				begin
					GenericAlert(4);
					ExitToShell;
				end;
			DisableItem(aMenu, iPause);
			DisableItem(aMenu, iEnd);
			aMenu := GetMenu(mOptions);		{Get the menu from the resource file}
			if (aMenu <> nil) then
				InsertMenu(aMenu, 0)				{Insert this menu into the menu bar}
			else
				begin
					GenericAlert(4);
					ExitToShell;
				end;
			DrawMenuBar;
		end;

{------------------}

		function NewBitMap (var theBitMap: BitMap; theRect: Rect): Ptr;
		begin
			with theBitMap, theRect do
				begin
					rowBytes := ((right - left + 15) div 16) * 2;
					baseAddr := NewPtr(rowBytes * (bottom - top));
					bounds := theRect;
					if MemError <> noErr then
						begin
							GenericAlert(4);
							ExitToShell;
						end
					else
						NewBitMap := baseAddr;
				end;
		end;

{------------------}

	begin
		SetApplLimit(Ptr(LongInt(GetApplLimit) - StackSize));
		MaxApplZone;
		for i := 1 to 12 do
			MoreMasters;
		InitGraf(@thePort);
		InitFonts;
		FlushEvents(everyEvent, 0);
		InitWindows;
		InitMenus;
		TEInit;
		InitDialogs(nil);
		InitCursor;

		ErrorSound(@DoErrorSound);

		for i := 1 to 3 do
			ignore := EventAvail(EveryEvent, theEvent);

		inBackground := FALSE;

		SetCursor(GetCursor(WatchCursor)^^);
		CheckOurEnvirons;

		if (screenBits.bounds.bottom - screenBits.bounds.top < 480) then
			vertOrigin := (screenBits.bounds.bottom - screenBits.bounds.top - 460) div 2
		else
			vertOrigin := 20;

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
					begin
						GenericAlert(4);
						ExitToShell;
					end;
			end;

		FlushEvents(everyEvent, 0);	{Clear out all events}
		Init_My_Menus;							{Initialize menu bar}

{if (screenBits.bounds.bottom < 480) then}
{SetRect(wholeArea, 0, 0, 640, screenBits.bounds.bottom)}
{else}
{SetRect(wholeArea, 0, 0, 640, 460);}

		SetRect(wholeArea, 0, 0, 640, 460);
		SetRect(smallOffArea, 0, 0, 512, 270);

		rgbBlack.red := 0;
		rgbBlack.green := 0;
		rgbBlack.blue := 0;
		rgbWhite.red := -1;
		rgbWhite.green := -1;
		rgbWhite.blue := -1;
		rgbYellow.red := -1;
		rgbYellow.green := -1;
		rgbYellow.blue := 0;
		rgbRed.red := -1;
		rgbRed.green := 0;
		rgbRed.blue := 0;
		rgbLtBlue.red := 0;
		rgbLtBlue.green := -1;
		rgbLtBlue.blue := -1;

		mainWndo := GetNewCWindow(mainWndoID, nil, Pointer(-1));
		if (mainWndo <> nil) then
			begin
				ShowWindow(mainWndo);
				SelectWindow(GrafPtr(mainWndo));
				SetPort(GrafPtr(mainWndo));
				RGBForeColor(rgbBlack);
				RGBBackColor(rgbWhite);
				MoveWindow(mainWndo, 0, vertOrigin, TRUE);
				ClipRect(screenBits.bounds);
			end
		else
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		mainPalette := GetNewPalette(mainWndoID);
		if (mainPalette <> nil) then
			SetPalette(mainWndo, mainPalette, TRUE)
		else
			begin
				GenericAlert(4);
				ExitToShell;
			end;

		MoveTo(0, 0);
		playRgn := NewRgn;
		OpenRgn;
		LineTo(0, 450);
		LineTo(161, 450);
		LineTo(161, 269);
		LineTo(172, 250);
		LineTo(182, 269);
		LineTo(182, 450);
		LineTo(457, 450);
		LineTo(457, 269);
		LineTo(468, 250);
		LineTo(478, 269);
		LineTo(478, 450);
		LineTo(640, 450);
		LineTo(640, 0);
		LineTo(0, 0);
		CloseRgn(playRgn);
		MoveHHi(Handle(playRgn));
		HLock(Handle(playRgn));

		MoveTo(161, 460);
		obeliskRgn1 := NewRgn;
		OpenRgn;
		LineTo(161, 269);
		LineTo(172, 250);
		LineTo(182, 269);
		LineTo(182, 460);
		LineTo(161, 460);
		CloseRgn(obeliskRgn1);
		MoveHHi(Handle(obeliskRgn1));
		HLock(Handle(obeliskRgn1));

		MoveTo(457, 460);
		obeliskRgn2 := NewRgn;
		OpenRgn;
		LineTo(457, 269);
		LineTo(468, 250);
		LineTo(478, 269);
		LineTo(478, 460);
		LineTo(457, 460);
		CloseRgn(obeliskRgn2);
		MoveHHi(Handle(obeliskRgn2));
		HLock(Handle(obeliskRgn2));


		OpenLogoWindo(rightOffset, downOffset);

		virginCPtr := @virginCPort;
		OpenCPort(virginCPtr);
		theDepth := 4;
		offRowBytes := ((((theDepth * (wholeArea.right - wholeArea.left)) + 15)) div 16) * 2;
		sizeOfOff := LONGINT(wholeArea.bottom - wholeArea.top) * offRowBytes;
		virginCBits := NewPtr(sizeOfOff);
		if (virginCPtr = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		with virginCPtr^.portPixMap^^ do
			begin
				baseAddr := virginCBits;
				rowBytes := offRowBytes + $8000;
				bounds := wholeArea;
			end;
		RGBForeColor(rgbBlack);
		RGBBackColor(rgbWhite);
		EraseRect(thePort^.portRect);
		ClipRect(wholeArea);
		CopyRgn(virginCPtr^.clipRgn, virginCPtr^.visRgn);

		thePict := GetPicture(backPictID);
		if (thePict <> nil) then
			begin
				tempByte := HGetState(Handle(thePict));
				MoveHHi(Handle(thePict));
				HLock(Handle(thePict));
				tempRect := thePict^^.picFrame;
				DrawPicture(thePict, tempRect);
				HSetState(Handle(thePict), tempByte);
			end
		else
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		ReleaseResource(Handle(thePict));

		loadCPtr := @loadCPort;
		OpenCPort(loadCPtr);
		loadCBits := NewPtr(sizeOfOff);
		if (loadCBits = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		with loadCPtr^.portPixMap^^ do
			begin
				baseAddr := loadCBits;
				rowBytes := offRowBytes + $8000;
				bounds := wholeArea;
			end;
		RGBForeColor(rgbBlack);
		RGBBackColor(rgbWhite);
		EraseRect(thePort^.portRect);
		ClipRect(wholeArea);
		CopyRgn(loadCPtr^.clipRgn, loadCPtr^.visRgn);

{Set up object port}
		offRowBytes := ((((theDepth * (smallOffArea.right - smallOffArea.left)) + 15)) div 16) * 2;
		sizeOfOff := LONGINT(smallOffArea.bottom - smallOffArea.top) * offRowBytes;
		objectCPtr := @objectCPort;
		OpenCPort(objectCPtr);
		objectCBits := NewPtr(sizeOfOff);
		if (objectCBits = nil) then
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		with objectCPtr^.portPixMap^^ do
			begin
				baseAddr := objectCBits;
				rowBytes := offRowBytes + $8000;
				bounds := smallOffArea;
			end;
		RGBForeColor(rgbBlack);
		RGBBackColor(rgbWhite);
		EraseRect(thePort^.portRect);
		ClipRect(smallOffArea);
		CopyRgn(objectCPtr^.clipRgn, objectCPtr^.visRgn);

		thePict := GetPicture(objectPictID);
		if (thePict <> nil) then
			begin
				tempByte := HGetState(Handle(thePict));
				MoveHHi(Handle(thePict));
				HLock(Handle(thePict));
				tempRect := thePict^^.picFrame;
				DrawPicture(thePict, tempRect);
				HSetState(Handle(thePict), tempByte);
			end
		else
			begin
				GenericAlert(4);
				ExitToShell;
			end;
		ReleaseResource(Handle(thePict));

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offMaskPort := GrafPtr(rawPointer);
		OpenPort(offMaskPort);
		offMaskBits := NewBitMap(offMaskMap, smallOffArea);
		SetPortBits(offMaskMap);
		EraseRect(offMaskMap.bounds);
		SetPort(offMaskPort);
		ClipRect(smallOffArea);
		CopyRgn(offMaskPort^.clipRgn, offMaskPort^.visRgn);

		thePict := GetPicture(maskPictID);	{Get Picture into memory}
		if (thePict <> nil) then			{Only use handle if it is valid}
			begin
				HLock(Handle(thePict));			{Lock the handle before using it}
				tempRect := thePict^^.picFrame;
				DrawPicture(thePict, tempRect);
				ReleaseResource(Handle(thePict));
				HUnLock(Handle(thePict));		{Unlock the picture again}
			end
		else
			begin
				GenericAlert(4);
				ExitToShell;
			end;

		numberOfStones := 4;
		levelStart := 1;
		levelOn := 0;
		mortalsStart := defaultNum;
		mortals := mortalsStart;
		score := 0;
		GetDateTime(RandSeed);
		playing := FALSE;
		pausing := FALSE;

		chanPtr := nil;
		soundPriority := noSound;

		SetRect(playerRects[0, 0], 260, 38, 308, 86);		{Rects for player facing right	}
		SetRect(playerRects[0, 1], 309, 38, 357, 86);		{walking}
		SetRect(playerRects[0, 2], 260, 38, 308, 86);		{walking}
		SetRect(playerRects[0, 3], 309, 38, 357, 86);		{walking}
		SetRect(playerRects[0, 4], 260, 0, 308, 37);		{flapping}
		SetRect(playerRects[0, 5], 309, 0, 357, 37);		{flapping}
		SetRect(playerRects[1, 0], 407, 38, 455, 86);		{Rects for player facing left	}
		SetRect(playerRects[1, 1], 358, 38, 406, 86);		{walking}
		SetRect(playerRects[1, 2], 407, 38, 455, 86);		{walking}
		SetRect(playerRects[1, 3], 358, 38, 406, 86);		{walking}
		SetRect(playerRects[1, 4], 407, 0, 455, 37);		{flapping}
		SetRect(playerRects[1, 5], 358, 0, 406, 37);		{flapping}

		SetRect(boneRects[0, 6], 260, 87, 308, 124);		{Falling skeleton left	}
		SetRect(boneRects[1, 6], 309, 87, 357, 124);		{Falling skeleton right	}
		SetRect(boneRects[0, 7], 456, 25, 501, 47);			{Pile of bones - left		}
		SetRect(boneRects[1, 7], 456, 25, 501, 47);			{Same bones for right	}

		SetRect(enemyRects[0, 0, 0], 49, 123, 97, 171);	{enemy 1 walking right}
		SetRect(enemyRects[0, 1, 0], 0, 123, 48, 171);	{enemy 1 walking right}
		SetRect(enemyRects[0, 2, 0], 49, 123, 97, 171);	{enemy 1 walking right}
		SetRect(enemyRects[0, 3, 0], 0, 123, 48, 171);	{enemy 1 walking right}
		SetRect(enemyRects[0, 4, 0], 65, 0, 129, 40);		{enemy 1 flapping left}
		SetRect(enemyRects[0, 5, 0], 0, 0, 64, 40);			{enemy 1 flapping left}
		SetRect(enemyRects[1, 0, 0], 49, 172, 97, 220);	{enemy 2 walking right}
		SetRect(enemyRects[1, 1, 0], 0, 172, 48, 220);	{enemy 2 walking right}
		SetRect(enemyRects[1, 2, 0], 49, 172, 97, 220);	{enemy 2 walking right}
		SetRect(enemyRects[1, 3, 0], 0, 172, 48, 220);	{enemy 2 walking right}
		SetRect(enemyRects[1, 4, 0], 65, 41, 129, 81);	{enemy 2 flapping left}
		SetRect(enemyRects[1, 5, 0], 0, 41, 64, 81);		{enemy 2 flapping left}
		SetRect(enemyRects[2, 0, 0], 49, 221, 97, 269);	{enemy 3 walking right}
		SetRect(enemyRects[2, 1, 0], 0, 221, 48, 269);	{enemy 3 walking right}
		SetRect(enemyRects[2, 2, 0], 49, 221, 97, 269);	{enemy 3 walking right}
		SetRect(enemyRects[2, 3, 0], 0, 221, 48, 269);	{enemy 3 walking right}
		SetRect(enemyRects[2, 4, 0], 65, 82, 129, 122);	{enemy 3 flapping left}
		SetRect(enemyRects[2, 5, 0], 0, 82, 64, 122);		{enemy 3 flapping left}

		SetRect(enemyRects[0, 0, 1], 98, 123, 146, 171);	{enemy 1 walking left}
		SetRect(enemyRects[0, 1, 1], 147, 123, 195, 171);	{enemy 1 walking left}
		SetRect(enemyRects[0, 2, 1], 98, 123, 146, 171);	{enemy 1 walking left}
		SetRect(enemyRects[0, 3, 1], 147, 123, 195, 171);	{enemy 1 walking left}
		SetRect(enemyRects[0, 4, 1], 130, 0, 194, 40);		{enemy 1 flapping right}
		SetRect(enemyRects[0, 5, 1], 195, 0, 259, 40);		{enemy 1 flapping right}
		SetRect(enemyRects[1, 0, 1], 98, 172, 146, 220);	{enemy 2 walking left}
		SetRect(enemyRects[1, 1, 1], 147, 172, 195, 220);	{enemy 2 walking left}
		SetRect(enemyRects[1, 2, 1], 98, 172, 146, 220);	{enemy 2 walking left}
		SetRect(enemyRects[1, 3, 1], 147, 172, 195, 220);	{enemy 2 walking left}
		SetRect(enemyRects[1, 4, 1], 130, 41, 194, 81);		{enemy 2 flapping right}
		SetRect(enemyRects[1, 5, 1], 195, 41, 259, 81);		{enemy 2 flapping right}
		SetRect(enemyRects[2, 0, 1], 98, 221, 146, 269);	{enemy 3 walking left}
		SetRect(enemyRects[2, 1, 1], 147, 221, 195, 269);	{enemy 3 walking left}
		SetRect(enemyRects[2, 2, 1], 98, 221, 146, 269);	{enemy 3 walking left}
		SetRect(enemyRects[2, 3, 1], 147, 221, 195, 269);	{enemy 3 walking left}
		SetRect(enemyRects[2, 4, 1], 130, 82, 194, 122);	{enemy 3 flapping right}
		SetRect(enemyRects[2, 5, 1], 195, 82, 259, 122);	{enemy 3 flapping right}

		enemyLift[0] := -1;
		enemyLift[1] := -2;
		enemyLift[2] := -4;

		SetRect(handRects[0], 358, 87, 422, 144);
		SetRect(handRects[1], 424, 87, 489, 144);

		SetRect(ankRects[0], 245, 180, 261, 203);
		SetRect(ankRects[1], 262, 180, 278, 203);
		SetRect(ankRects[2], 279, 180, 295, 203);
		SetRect(ankRects[3], 296, 180, 312, 203);
		SetRect(ankRects[4], 313, 180, 329, 203);
		SetRect(ankRects[5], 330, 180, 346, 203);
		SetRect(ankRects[6], 347, 180, 363, 203);
		SetRect(ankRects[7], 364, 180, 380, 203);
		SetRect(ankRects[8], 381, 180, 397, 203);
		SetRect(ankRects[9], 364, 180, 380, 203);
		SetRect(ankRects[10], 347, 180, 363, 203);
		SetRect(ankRects[11], 330, 180, 346, 203);
		SetRect(ankRects[12], 313, 180, 329, 203);
		SetRect(ankRects[13], 296, 180, 312, 203);
		SetRect(ankRects[14], 279, 180, 295, 203);
		SetRect(ankRects[15], 262, 180, 278, 203);

		for i := 0 to 1 do
			for i2 := 0 to 5 do
				SetRect(absoluteRects[i, i2], 0, 0, playerRects[i, i2].right - playerRects[i, i2].left, playerRects[i, i2].bottom - playerRects[i, i2].top);

		running[-16, 0, 0] := -14;	{new hori velocity}
		running[-16, 0, 1] := 1;		{bird leg mode}
		running[-15, 0, 0] := -13;
		running[-15, 0, 1] := 1;
		running[-14, 0, 0] := -12;
		running[-14, 0, 1] := 1;
		running[-13, 0, 0] := -11;
		running[-13, 0, 1] := 1;
		running[-12, 0, 0] := -10;
		running[-12, 0, 1] := 1;
		running[-11, 0, 0] := -9;
		running[-11, 0, 1] := 1;
		running[-10, 0, 0] := -8;
		running[-10, 0, 1] := 1;
		running[-9, 0, 0] := -7;
		running[-9, 0, 1] := 1;
		running[-8, 0, 0] := -6;
		running[-8, 0, 1] := 1;
		running[-7, 0, 0] := -5;
		running[-7, 0, 1] := 1;
		running[-6, 0, 0] := -4;
		running[-6, 0, 1] := 1;
		running[-5, 0, 0] := -3;
		running[-5, 0, 1] := 1;
		running[-4, 0, 0] := -2;
		running[-4, 0, 1] := 1;
		running[-3, 0, 0] := -1;
		running[-3, 0, 1] := 1;
		running[-2, 0, 0] := 0;
		running[-2, 0, 1] := 1;
		running[-1, 0, 0] := 1;
		running[-1, 0, 1] := 1;
		running[0, 0, 0] := 4;
		running[0, 0, 1] := 0;
		running[1, 0, 0] := 4;
		running[1, 0, 1] := 0;
		running[2, 0, 0] := 4;
		running[2, 0, 1] := 0;
		running[3, 0, 0] := 12;
		running[3, 0, 1] := 3;
		running[4, 0, 0] := 11;
		running[4, 0, 1] := 1;
		running[5, 0, 0] := 4;
		running[5, 0, 1] := 0;
		running[6, 0, 0] := 4;
		running[6, 0, 1] := 0;
		running[7, 0, 0] := 4;
		running[7, 0, 1] := 0;
		running[8, 0, 0] := 12;
		running[8, 0, 1] := 3;
		running[9, 0, 0] := 12;
		running[9, 0, 1] := 3;
		running[10, 0, 0] := 12;
		running[10, 0, 1] := 3;
		running[11, 0, 0] := 3;
		running[11, 0, 1] := 2;
		running[12, 0, 0] := 4;
		running[12, 0, 1] := 0;
		running[13, 0, 0] := 11;
		running[13, 0, 1] := 1;
		running[14, 0, 0] := 12;
		running[14, 0, 1] := 1;
		running[15, 0, 0] := 13;
		running[15, 0, 1] := 1;
		running[16, 0, 0] := 14;
		running[16, 0, 1] := 1;

		for i := -16 to 16 do
			begin
				running[i, 1, 0] := -running[-i, 0, 0];
				running[i, 1, 1] := running[-i, 0, 1];
			end;

		for i := 2 to 16 do
			begin
				idleLanded[i] := i - 2;
				idleLanded[-i] := -i + 2;
			end;
		idleLanded[-1] := 0;
		idleLanded[0] := 0;
		idleLanded[1] := 0;

		for i := -16 to -1 do
			begin
				gliding[i, 0] := i + 3;
				gliding[-i, 1] := -i - 3;
			end;

		for i := 0 to 14 do
			begin
				gliding[i, 0] := i + 2;
				gliding[-i, 1] := -i - 2;
			end;

		gliding[16, 0] := 16;
		gliding[-16, 1] := -16;
		gliding[15, 0] := 16;
		gliding[-15, 1] := -16;

		for i := -70 to 16 do
			begin
				impacted[i] := (-2 * i) div 3;
			end;
		SetRect(tombRects[-5], -61, 424, 206, 440);	{acid cover left}
		SetRect(tombRects[-4], 433, 424, 700, 440);	{acid cover right}
		SetRect(tombRects[-3], 232, 180, 407, 197);	{top floor center}
		SetRect(tombRects[-2], 206, 424, 433, 460);	{bottom floor center}
		SetRect(tombRects[-1], -61, 424, 700, 440);	{whole floor bottom}
		SetRect(tombRects[0], 206, 424, 433, 460);	{bottom floor center}
		SetRect(tombRects[1], -117, 283, 150, 300);	{bottom floor left}
		SetRect(tombRects[2], 490, 283, 757, 300);	{bottom floor right}
		SetRect(tombRects[3], -107, 104, 160, 121);	{top floor left}
		SetRect(tombRects[4], 480, 104, 747, 121);	{top floor right}
		SetRect(tombRects[5], 232, 180, 407, 197);	{top floor center}
		SetRect(tombRects[6], -61, 424, 206, 440);	{acid cover left}

		SetRect(eyeRects[0], 196, 219, 244, 250);
		SetRect(eyeRects[1], 196, 187, 244, 218);
		SetRect(eyeRects[2], 196, 155, 244, 186);
		SetRect(eyeRects[3], 196, 123, 244, 154);
		SetRect(eyeRects[4], 196, 123, 244, 154);

		with theEye do
			begin
				dest := eyeRects[4];
				CenterZeroRect(dest);
				OffsetRect(dest, 320, upperEyeHeight);
				oldDest := dest;
			end;

		SetRect(eggRects, 456, 0, 480, 24);
		SetRect(flameRect[0], 80, 307, 112, 342);
		SetRect(flameRect[1], 528, 307, 560, 342);
		SetRect(shortStoneSrc, 337, 236, 512, 253);
		SetRect(longStoneSrc, 245, 253, 512, 270);
		SetRect(playRect, 15, 37, 625, 440);
		SetRect(gameoverRects, 245, 145, 457, 179);

		doneFlag := FALSE;

		NewLightning;
		ReadInScores;
		CloseLogoWindo;
		RedoTheBackground;
		DoTheSound('music.snd', highPriority);
		ReDrawHiScores;
		FlushEvents(everyEvent, 0);
		repeat
		until (soundPriority = noSound);
		InitCursor;
		CopyMask(BitMapPtr(objectCPtr^.portPixMap^)^, offMaskMap, mainWndo^.portBits, eyeRects[4], eyeRects[4], theEye.dest);
		DoTheSound('lightning.snd', highPriority);
		SetPort(GrafPtr(mainWndo));
		for i := 1 to 3 do
			StrikeLightning(upperEye);
		CopyBits(BitMapPtr(virginCPtr^.portPixMap^)^, mainWndo^.portBits, theEye.dest, theEye.dest, srcCopy, playRgn);
		FlushEvents(everyEvent, 0);
	end;

{=================================}

end.
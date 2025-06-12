unit Initialize;

interface
	uses
		Sound, GameUtils, Enemies, GlyphaGuts;

	procedure Init_My_Menus;
	function NewBitMap (var theBitMap: BitMap; theRect: Rect): Ptr;
	procedure InitVariables;

implementation

{=================================}

	procedure Init_My_Menus;
		const
			Menu1 = 201;				{Menu resource ID}
			Menu2 = 202;				{Menu resource ID}
			Menu3 = 203;				{Menu resource ID}
	begin
		ClearMenuBar;							{Clear any old menu bars}
		AppleMenu := GetMenu(Menu1);		{Get the menu from the resource file}
		AddResMenu(AppleMenu, 'DRVR');	{Add in DAs}
		InsertMenu(AppleMenu, 0);			{Insert this menu into the menu bar}
		GameMenu := GetMenu(Menu2);		{Get the menu from the resource file}
		InsertMenu(GameMenu, 0);				{Insert this menu into the menu bar}
		OptionsMenu := GetMenu(Menu3);		{Get the menu from the resource file}
		InsertMenu(OptionsMenu, 0);			{Insert this menu into the menu bar}
		DisableItem(GameMenu, 2);
		DisableItem(GameMenu, 3);
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
			index, index2: Integer;
			rawPointer: Ptr;
			Pic_Handle: PicHandle;
			theSnd: Handle;
			tempRect: Rect;
	begin
		numberOfStones := 4;
		levelStart := 1;
		levelOn := 0;
		mortalsStart := defaultNum;
		chanPtr := nil;
		ahnkCursor := GetCursor(5000);
		playMask := MDownMask + MUpMask + KeyDownMask;
		GetDateTime(RandSeed);
		playing := FALSE;
		pausing := FALSE;

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen virgin console}
		offVirginPort := GrafPtr(rawPointer);
		OpenPort(offVirginPort);
		SetRect(offVirginArea, 0, 0, 512, 342);
		offVirginBits := NewBitMap(offVirginMap, offVirginArea);
		SetPortBits(offVirginMap);
		EraseRect(offVirginMap.bounds);
		Pic_Handle := GetPicture(1);				{Get Picture into memory}
		if (Pic_Handle <> nil) then				{Only use handle if it is valid}
			DrawPicture(Pic_Handle, offVirginArea);{Draw this picture}
		ReleaseResource(Handle(Pic_Handle));

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offLoadPort := GrafPtr(rawPointer);
		OpenPort(offLoadPort);
		SetRect(offLoadArea, 0, 0, 512, 342);
		offLoadBits := NewBitMap(offLoadMap, offLoadArea);
		SetPortBits(offLoadMap);
		EraseRect(offLoadMap.bounds);

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offPlayerPort := GrafPtr(rawPointer);
		OpenPort(offPlayerPort);
		SetRect(offPlayerArea, 0, 0, 512, 282);
		offPlayerBits := NewBitMap(offPlayerMap, offPlayerArea);
		SetPortBits(offPlayerMap);
		EraseRect(offPlayerMap.bounds);
		Pic_Handle := GetPicture(2);			{Get Picture into memory}
		SetRect(tempRect, 0, 0, 512, 282);	{left,top,right,bottom}
		if (Pic_Handle <> nil) then			{Only use handle if it is valid}
			begin
				ClipRect(tempRect);					{Clip picture to this rectangle}
				HLock(Handle(Pic_Handle));			{Lock the handle before using it}
				tempRect.Right := tempRect.Left + (Pic_Handle^^.picFrame.Right - Pic_Handle^^.picFrame.Left);
				tempRect.Bottom := tempRect.Top + (Pic_Handle^^.picFrame.Bottom - Pic_Handle^^.picFrame.Top);
				HUnLock(Handle(Pic_Handle));		{Unlock the picture again}
			end;
		if (Pic_Handle <> nil) then				{Only use handle if it is valid}
			DrawPicture(Pic_Handle, tempRect); 	{Draw this picture}
		ReleaseResource(Handle(Pic_Handle));
		SetRect(tempRect, 0, 0, 1023, 1023); 	{Widen the clip area again}
		ClipRect(tempRect);

		rawPointer := NewPtr(SizeOf(GrafPort));	{Initialize and setup offscreen}
		offEnemyPort := GrafPtr(rawPointer);
		OpenPort(offEnemyPort);
		SetRect(offEnemyArea, 0, 0, 512, 285);
		offEnemyBits := NewBitMap(offEnemyMap, offEnemyArea);
		SetPortBits(offEnemyMap);
		EraseRect(offEnemyMap.bounds);
		SetPort(offEnemyPort);				{Set the port to my window}
		Pic_Handle := GetPicture(3);			{Get Picture into memory}
		SetRect(tempRect, 0, 0, 512, 285);		{left,top,right,bottom}
		if (Pic_Handle <> nil) then			{Only use handle if it is valid}
			begin
				ClipRect(tempRect);				{Clip picture to this rectangle}
				HLock(Handle(Pic_Handle));			{Lock the handle before using it}
				tempRect.Right := tempRect.Left + (Pic_Handle^^.picFrame.Right - Pic_Handle^^.picFrame.Left);
				tempRect.Bottom := tempRect.Top + (Pic_Handle^^.picFrame.Bottom - Pic_Handle^^.picFrame.Top);
				HUnLock(Handle(Pic_Handle));		{Unlock the picture again}
			end;
		if (Pic_Handle <> nil) then			{Only use handle if it is valid}
			DrawPicture(Pic_Handle, tempRect); 	{Draw this picture}
		ReleaseResource(Handle(Pic_Handle));

		SetRect(playerRects[0, 0], 0, 0, 54, 51);				{Rects for player facing right	}
		SetRect(playerRects[0, 1], 108, 0, 162, 51);
		SetRect(playerRects[0, 2], 54, 0, 108, 51);
		SetRect(playerRects[0, 3], 108, 0, 162, 51);
		SetRect(playerRects[0, 4], 162, 10, 217, 49);
		SetRect(playerRects[0, 5], 217, 10, 272, 49);
		SetRect(playerRects[1, 0], 218, 104, 272, 155);	{Rects for player facing left	}
		SetRect(playerRects[1, 1], 110, 104, 164, 155);
		SetRect(playerRects[1, 2], 164, 104, 216, 155);
		SetRect(playerRects[1, 3], 110, 104, 164, 155);
		SetRect(playerRects[1, 4], 55, 114, 110, 153);
		SetRect(playerRects[1, 5], 0, 114, 55, 153);

		SetRect(playerRects[2, 0], 0, 52, 54, 103);			{These are masks for above	}
		SetRect(playerRects[2, 1], 108, 52, 162, 103);
		SetRect(playerRects[2, 2], 54, 52, 108, 103);
		SetRect(playerRects[2, 3], 108, 52, 162, 103);
		SetRect(playerRects[2, 4], 162, 62, 217, 101);
		SetRect(playerRects[2, 5], 217, 62, 272, 101);

		SetRect(playerRects[3, 0], 218, 156, 272, 207);
		SetRect(playerRects[3, 1], 110, 156, 164, 207);
		SetRect(playerRects[3, 2], 164, 156, 218, 207);
		SetRect(playerRects[3, 3], 110, 156, 164, 207);
		SetRect(playerRects[3, 4], 55, 166, 110, 205);
		SetRect(playerRects[3, 5], 0, 166, 55, 205);

		SetRect(boneRects[0, 6, 0], 411, 108, 459, 146);	{Falling skeleton left	}
		SetRect(boneRects[0, 6, 1], 462, 108, 510, 146);	{It's mask				}
		SetRect(boneRects[1, 6, 0], 410, 69, 458, 107);		{Falling skeleton right	}
		SetRect(boneRects[1, 6, 1], 461, 69, 509, 107);		{It's mask				}
		SetRect(boneRects[0, 7, 0], 462, 191, 506, 214);	{Pile of bones - left		}
		SetRect(boneRects[0, 7, 1], 462, 222, 506, 245);	{It's mask				}
		SetRect(boneRects[1, 7, 0], 462, 191, 506, 214);	{Same bones for right	}
		SetRect(boneRects[1, 7, 1], 462, 222, 506, 245);	{It's mask				}

		SetRect(enemyRects[0, 0, 0, 0], 289, 88, 331, 131);
		SetRect(enemyRects[0, 1, 0, 0], 334, 88, 376, 131);
		SetRect(enemyRects[0, 2, 0, 0], 379, 88, 421, 131);
		SetRect(enemyRects[0, 3, 0, 0], 334, 88, 376, 131);
		SetRect(enemyRects[0, 4, 0, 0], 0, 79, 68, 116);
		SetRect(enemyRects[0, 5, 0, 0], 69, 79, 137, 116);
		SetRect(enemyRects[1, 0, 0, 0], 289, 0, 331, 43);
		SetRect(enemyRects[1, 1, 0, 0], 334, 0, 376, 43);
		SetRect(enemyRects[1, 2, 0, 0], 379, 0, 421, 43);
		SetRect(enemyRects[1, 3, 0, 0], 334, 0, 376, 43);
		SetRect(enemyRects[1, 4, 0, 0], 0, 0, 68, 37);
		SetRect(enemyRects[1, 5, 0, 0], 69, 0, 137, 37);
		SetRect(enemyRects[2, 0, 0, 0], 289, 176, 331, 219);
		SetRect(enemyRects[2, 1, 0, 0], 334, 176, 376, 219);
		SetRect(enemyRects[2, 2, 0, 0], 379, 176, 421, 219);
		SetRect(enemyRects[2, 3, 0, 0], 334, 176, 376, 219);
		SetRect(enemyRects[2, 4, 0, 0], 0, 158, 68, 195);
		SetRect(enemyRects[2, 5, 0, 0], 69, 158, 137, 195);
		SetRect(enemyRects[0, 0, 1, 0], 0, 241, 42, 284);
		SetRect(enemyRects[0, 1, 1, 0], 468, 88, 510, 131);
		SetRect(enemyRects[0, 2, 1, 0], 423, 88, 465, 131);
		SetRect(enemyRects[0, 3, 1, 0], 468, 88, 510, 131);
		SetRect(enemyRects[0, 4, 1, 0], 207, 79, 275, 116);
		SetRect(enemyRects[0, 5, 1, 0], 138, 79, 206, 116);
		SetRect(enemyRects[1, 0, 1, 0], 43, 241, 85, 284);
		SetRect(enemyRects[1, 1, 1, 0], 468, 0, 510, 43);
		SetRect(enemyRects[1, 2, 1, 0], 423, 0, 465, 43);
		SetRect(enemyRects[1, 3, 1, 0], 468, 0, 510, 43);
		SetRect(enemyRects[1, 4, 1, 0], 207, 0, 275, 37);
		SetRect(enemyRects[1, 5, 1, 0], 138, 0, 206, 37);
		SetRect(enemyRects[2, 0, 1, 0], 86, 241, 128, 284);
		SetRect(enemyRects[2, 1, 1, 0], 468, 176, 510, 219);
		SetRect(enemyRects[2, 2, 1, 0], 423, 176, 465, 219);
		SetRect(enemyRects[2, 3, 1, 0], 468, 176, 510, 219);
		SetRect(enemyRects[2, 4, 1, 0], 207, 158, 275, 195);
		SetRect(enemyRects[2, 5, 1, 0], 138, 158, 206, 195);

		SetRect(enemyRects[0, 0, 0, 1], 289, 132, 331, 175);
		SetRect(enemyRects[0, 1, 0, 1], 334, 132, 376, 175);
		SetRect(enemyRects[0, 2, 0, 1], 379, 132, 421, 175);
		SetRect(enemyRects[0, 3, 0, 1], 334, 132, 376, 175);
		SetRect(enemyRects[0, 4, 0, 1], 0, 117, 68, 154);
		SetRect(enemyRects[0, 5, 0, 1], 69, 117, 137, 154);
		SetRect(enemyRects[1, 0, 0, 1], 289, 44, 331, 87);
		SetRect(enemyRects[1, 1, 0, 1], 334, 44, 376, 87);
		SetRect(enemyRects[1, 2, 0, 1], 379, 44, 421, 87);
		SetRect(enemyRects[1, 3, 0, 1], 334, 44, 376, 87);
		SetRect(enemyRects[1, 4, 0, 1], 0, 42, 68, 79);
		SetRect(enemyRects[1, 5, 0, 1], 69, 42, 137, 79);
		SetRect(enemyRects[2, 0, 0, 1], 289, 220, 331, 263);
		SetRect(enemyRects[2, 1, 0, 1], 334, 220, 376, 263);
		SetRect(enemyRects[2, 2, 0, 1], 379, 220, 421, 263);
		SetRect(enemyRects[2, 3, 0, 1], 334, 220, 376, 263);
		SetRect(enemyRects[2, 4, 0, 1], 0, 201, 68, 238);
		SetRect(enemyRects[2, 5, 0, 1], 69, 200, 137, 237);
		SetRect(enemyRects[0, 0, 1, 1], 129, 241, 171, 285);
		SetRect(enemyRects[0, 1, 1, 1], 468, 132, 510, 175);
		SetRect(enemyRects[0, 2, 1, 1], 423, 132, 465, 175);
		SetRect(enemyRects[0, 3, 1, 1], 468, 132, 510, 175);
		SetRect(enemyRects[0, 4, 1, 1], 207, 117, 275, 154);
		SetRect(enemyRects[0, 5, 1, 1], 138, 117, 206, 154);
		SetRect(enemyRects[1, 0, 1, 1], 172, 241, 214, 284);
		SetRect(enemyRects[1, 1, 1, 1], 468, 44, 510, 87);
		SetRect(enemyRects[1, 2, 1, 1], 423, 44, 465, 87);
		SetRect(enemyRects[1, 3, 1, 1], 468, 44, 510, 87);
		SetRect(enemyRects[1, 4, 1, 1], 207, 42, 275, 79);
		SetRect(enemyRects[1, 5, 1, 1], 138, 42, 206, 79);
		SetRect(enemyRects[2, 0, 1, 1], 215, 241, 257, 284);
		SetRect(enemyRects[2, 1, 1, 1], 468, 220, 510, 263);
		SetRect(enemyRects[2, 2, 1, 1], 423, 220, 465, 263);
		SetRect(enemyRects[2, 3, 1, 1], 468, 220, 510, 263);
		SetRect(enemyRects[2, 4, 1, 1], 207, 200, 275, 237);
		SetRect(enemyRects[2, 5, 1, 1], 138, 200, 206, 237);

		enemyLift[0] := -1;
		enemyLift[1] := -2;
		enemyLift[2] := -4;

		SetRect(handRects[0, 0], 274, 69, 340, 126);
		SetRect(handRects[1, 0], 341, 77, 406, 126);
		SetRect(handRects[0, 1], 274, 127, 340, 186);	{location of masks}
		SetRect(handRects[1, 1], 341, 136, 406, 186);

		for index := 0 to 15 do
			begin
				SetRect(ankRects[index, 0], index * 17 + 1, 209, index * 17 + 17, 234);
				SetRect(ankRects[index, 1], index * 17 + 1, 236, index * 17 + 17, 261);
			end;

		for index := 0 to 1 do
			for index2 := 0 to 5 do
				SetRect(absoluteRects[index, index2], 0, 0, playerRects[index, index2].right - playerRects[index, index2].left, playerRects[index, index2].bottom - playerRects[index, index2].top);

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

		for index := -16 to 16 do
			begin
				running[index, 1, 0] := -running[-index, 0, 0];
				running[index, 1, 1] := running[-index, 0, 1];
			end;

		for index := 2 to 16 do
			begin
				idleLanded[index] := index - 2;
				idleLanded[-index] := -index + 2;
			end;
		idleLanded[-1] := 0;
		idleLanded[0] := 0;
		idleLanded[1] := 0;

		for index := -16 to -1 do
			begin
				gliding[index, 0] := index + 3;
				gliding[-index, 1] := -index - 3;
			end;

		for index := 0 to 14 do
			begin
				gliding[index, 0] := index + 2;
				gliding[-index, 1] := -index - 2;
			end;

		gliding[16, 0] := 16;
		gliding[-16, 1] := -16;
		gliding[15, 0] := 16;
		gliding[-15, 1] := -16;

		for index := -70 to 16 do
			begin
				impacted[index] := (-2 * index) div 3;
			end;
		SetRect(tombRects[-5], -70, 317, 167, 343);
		SetRect(tombRects[-4], 346, 317, 583, 343);
		SetRect(tombRects[-3], 201, 145, 312, 162);
		SetRect(tombRects[-2], 167, 317, 346, 343);
		SetRect(tombRects[-1], -70, 317, 583, 343);
		SetRect(tombRects[0], 167, 317, 346, 343);
		SetRect(tombRects[1], -70, 211, 110, 226);
		SetRect(tombRects[2], 403, 211, 583, 226);
		SetRect(tombRects[3], -70, 95, 134, 110);
		SetRect(tombRects[4], 379, 95, 583, 110);
		SetRect(tombRects[5], 201, 145, 312, 162);
		SetRect(tombRects[6], -70, 317, 167, 343);

		SetRect(eyeRects[0], 465, 0, 512, 33);
		SetRect(eyeRects[1], 417, 0, 464, 33);
		SetRect(eyeRects[2], 370, 0, 417, 33);
		SetRect(eyeRects[3], 323, 0, 370, 33);
		SetRect(eyeRects[4], 276, 0, 323, 33);
		SetRect(eyeRects[5], 465, 33, 512, 66);	{eye mask Rects follow}
		SetRect(eyeRects[6], 417, 33, 464, 66);
		SetRect(eyeRects[7], 370, 33, 417, 66);
		SetRect(eyeRects[8], 323, 33, 370, 66);
		SetRect(eyeRects[9], 276, 33, 323, 66);

		with theEye do
			begin
				dest := eyeRects[4];
				OffsetRect(dest, -43, 70);
				oldDest := dest;
			end;

		SetRect(eggRects[0], 410, 164, 432, 186);
		SetRect(eggRects[1], 434, 164, 456, 186);

		SetRect(flameRect[0], 44, 226, 71, 275);
		SetRect(flameRect[1], 445, 226, 472, 275);

		SetRect(littleTombRect, 260, 266, 371, 283);
		SetRect(upperTombRect1, 0, 265, 135, 281);
		SetRect(upperTombRect2, 136, 265, 271, 281);

		SetRect(scoreGrabRect, 373, 266, 509, 283);
		SetRect(scoreDropRect, 189, 166, 325, 183);

		SetRect(gameoverRects[0], 274, 189, 456, 224);
		SetRect(gameoverRects[1], 274, 225, 456, 261);

		SetRect(playRect, 12, 37, 500, 322);

		MoveTo(0, 0);
		playRgn := NewRgn;
		OpenRgn;
		LineTo(0, 339);
		LineTo(122, 339);
		LineTo(122, 161);
		LineTo(133, 141);
		LineTo(144, 161);
		LineTo(144, 339);
		LineTo(369, 339);
		LineTo(369, 161);
		LineTo(380, 141);
		LineTo(391, 161);
		LineTo(391, 339);
		LineTo(512, 339);
		LineTo(512, 0);
		LineTo(0, 0);
		CloseRgn(playRgn);
		HLock(Handle(playRgn));

		MoveTo(122, 339);
		obeliskRgn1 := NewRgn;
		OpenRgn;
		LineTo(122, 161);
		LineTo(133, 141);
		LineTo(144, 161);
		LineTo(144, 339);
		LineTo(122, 339);
		CloseRgn(obeliskRgn1);
		HLock(Handle(obeliskRgn1));

		MoveTo(369, 339);
		obeliskRgn2 := NewRgn;
		OpenRgn;
		LineTo(369, 161);
		LineTo(380, 141);
		LineTo(391, 161);
		LineTo(391, 339);
		LineTo(369, 339);
		CloseRgn(obeliskRgn2);
		HLock(Handle(obeliskRgn2));

		theSnd := GetNamedResource('snd ', 'ahnk.snd');
		theSnd := GetNamedResource('snd ', 'bird.snd');
		theSnd := GetNamedResource('snd ', 'bonus.snd');
		theSnd := GetNamedResource('snd ', 'boom1.snd');
		theSnd := GetNamedResource('snd ', 'boom2.snd');
		theSnd := GetNamedResource('snd ', 'drip.snd');
		theSnd := GetNamedResource('snd ', 'flap.snd');
		theSnd := GetNamedResource('snd ', 'flip.snd');
		theSnd := GetNamedResource('snd ', 'music.snd');
		theSnd := GetNamedResource('snd ', 'rez.snd');
		theSnd := GetNamedResource('snd ', 'screech.snd');
		theSnd := GetNamedResource('snd ', 'spawn.snd');
		theSnd := GetNamedResource('snd ', 'walk.snd');
		theSnd := GetNamedResource('snd ', 'lightning.snd');
	end;

{=================================}

end.
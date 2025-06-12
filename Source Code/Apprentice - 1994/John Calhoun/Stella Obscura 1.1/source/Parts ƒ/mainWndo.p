unit mainWndo;

interface
	uses
		Sound, SomeGlobals, Utilities;

	procedure Init_mainWndo;
	procedure Close_mainWndo (whichWindow: WindowPtr; var theInput: TEHandle);
	procedure Update_mainWndo (whichWindow: WindowPtr);
	procedure BringUpStartUp;

implementation

	var
		tempRect: Rect;                      {Temporary rectangle}
		Index: Integer;                      {For looping}
		CtrlHandle: controlhandle;           {Control handle}
		sTemp: Str255;                       {Get text entered, temp holding}

{=================================}

	procedure Init_mainWndo;
	begin
		mainWndo := nil;
	end;

{=================================}

	procedure Close_mainWndo;
	begin
		if (mainWndo <> nil) and (mainWndo = whichWindow) then
			begin
				DisposeWindow(mainWndo);
				mainWndo := nil;
			end;
	end;

{=================================}

	procedure UpDate_mainWndo;
	begin
		SetPort(mainWndo);
		FillRect(mainWndo^.portRect, black);
		CopyBits(offLeftVirginMap, offLeftMap, offLeftVirginMap.bounds, offLeftVirginMap.bounds, srcCopy, nil);
		CopyBits(offRightVirginMap, offRightMap, offRightVirginMap.bounds, offRightVirginMap.bounds, srcCopy, nil);
		CopyBits(offLeftMap, mainWndo^.portBits, offLeftMap.bounds, leftBckgrndDst, srcCopy, nil);
		CopyBits(offRightMap, mainWndo^.portBits, offRightMap.bounds, rightBckgrndDst, srcCopy, nil);
	end;

{=================================}

	procedure BringUpStartUp;
		var
			tempRect: Rect;
			Pic_Handle: PicHandle;
	begin
		mainWndo := GetNewWindow(1, nil, Pointer(-1));
		SelectWindow(mainWndo);
		SetPort(mainWndo);
		Pic_Handle := GetPicture(2002);
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

		repeat
		until (not button);
		repeat
		until (button);
	end;

{=================================}

end.                                    {End of unit}
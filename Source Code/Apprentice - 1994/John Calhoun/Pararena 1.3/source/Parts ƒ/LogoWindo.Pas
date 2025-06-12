unit LogoWindo;

interface

	procedure OpenLogo;
	procedure CloseLogo;

implementation

 {________________________________}

	const
		logoWindID = 1999;
		logoPictID = 1999;

	var
		logoWndo: WindowPtr;

 {________________________________}

	procedure OpenLogo;
		var
			rightOff, downOff: Integer;
			tempRect: Rect;
			thePict: PicHandle;
	begin
		logoWndo := nil;

		logoWndo := GetNewWindow(logoWindID, nil, Pointer(-1));
		rightOff := logoWndo^.portRect.right - logoWndo^.portRect.left;
		rightOff := (ScreenBits.bounds.right - rightOff) div 2;
		downOff := logoWndo^.portRect.bottom - logoWndo^.portRect.top;
		downOff := (ScreenBits.bounds.bottom - downOff) div 2;
		MoveWindow(logoWndo, rightOff, downOff, FALSE);
		SelectWindow(logoWndo);
		SetPort(logoWndo);
		ShowWindow(logoWndo);

		thePict := GetPicture(logoPictID);
		if (thePict <> nil) then
			begin
				HLock(Handle(thePict));
				tempRect := thePict^^.picFrame;
				ClipRect(tempRect);
				DrawPicture(thePict, tempRect);
				HUnlock(Handle(thePict));
				ReleaseResource(Handle(thePict));
			end;
	end;

{________________________________}

	procedure CloseLogo;
	begin
		if (logoWndo <> nil) then
			begin
				DisposeWindow(logoWndo);
				logoWndo := nil;
			end;
	end;

{________________________________}

end.                                    {End of unit}
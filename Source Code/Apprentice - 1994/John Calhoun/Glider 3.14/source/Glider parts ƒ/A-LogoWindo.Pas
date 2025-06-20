unit LogoWindo;

interface

	procedure Init_LogoWindo;
	procedure Close_LogoWindo;
	procedure Open_LogoWindo;
	procedure Update_LogoWindo;

implementation

	var
		logoWindo: WindowPtr;
		tempRect: Rect;
		Pic_Handle: PicHandle;

{=================================}

	procedure Init_LogoWindo;
	begin
		logoWindo := nil;
	end;

{=================================}

	procedure Close_LogoWindo;
	begin
		if (logoWindo <> nil) then
			begin
				DisposeWindow(logoWindo);
				logoWindo := nil;
			end;
	end;

{=================================}

	procedure UpDate_LogoWindo;
		var
			SavePort: WindowPtr;
	begin
		if (logoWindo <> nil) then
			begin
				GetPort(SavePort);
				SetPort(logoWindo);
				Pic_Handle := GetPicture(1999);
				SetRect(tempRect, 0, 0, 321, 308);
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
				SetPort(SavePort);
			end;
	end;

{=================================}

	procedure Open_LogoWindo;
	begin
		if (logoWindo = nil) then
			begin
				logoWindo := GetNewWindow(1999, nil, Pointer(-1));
				MoveWindow(logoWindo, (screenBits.bounds.right - 321) div 2, (screenBits.bounds.bottom - 308) div 2, FALSE);
				ShowWindow(logoWindo);
				SelectWindow(logoWindo);
				SetPort(logoWindo);
				UpDate_LogoWindo;
			end
		else
			SelectWindow(logoWindo);
	end;

{=================================}

end.                                    {End of unit}
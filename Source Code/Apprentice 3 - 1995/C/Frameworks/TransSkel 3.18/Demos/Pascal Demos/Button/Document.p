unit Document;

interface

	uses
		TransSkel, ButtonGlobals;


	procedure SetupDocument;

implementation

	const

		returnKey = chr(13);
		enterKey = chr(3);
		escapeKey = chr(27);

	var

		wind: WindowPtr;
		okBtn: ControlHandle;
		cancelBtn: ControlHandle;

{--------------------------------------------------------------------}
{ Window handling procedures }
{--------------------------------------------------------------------}


	procedure Mouse (pt: Point;
									t: LongInt;
									mods: Integer);
		var
			ctrl: ControlHandle;
			partNo: Integer;
	begin
		partNo := FindControl(pt, wind, ctrl);
		if (partNo <> 0) then
			begin
				if (partNo = inButton) then
					begin
						if (TrackControl(ctrl, pt, nil) <> 0) then
							begin
								{ nothing done }
							end;
					end;
			end;
	end;


	procedure Key (c: char;
									code: Integer;
									mods: Integer);
	begin
		if ((c = returnKey) or (c = enterKey)) then
			begin
				if (okBtn^^.contrlHilite = normalHilite) then
					SkelFlashButton(okBtn);
			end
		else
			begin
				if ((c = escapeKey) or SkelCmdPeriod(SkelGetCurrentEvent^)) then
					begin
						if (cancelBtn^^.contrlHilite = normalHilite) then
							SkelFlashButton(cancelBtn);
					end;
			end;

	end;


	procedure Update (resized: Boolean);
		var
			wind: WindowPtr;
			r: Rect;
			h: Integer;
	begin
		GetPort(wind);

		r := wind^.portRect;
		EraseRect(r);
		DrawControls(wind);
		SkelDrawButtonOutline(okBtn);
	end;


	procedure Activate (active: Boolean);
		var
			hilite: Integer;
	begin
		if (active) then
			hilite := normalHilite
		else
			hilite := dimHilite;
		HiliteControl(okBtn, hilite);
		SkelDrawButtonOutline(okBtn);
		HiliteControl(cancelBtn, hilite);
	end;


	procedure Clobber;
		var
			wind: WindowPtr;
	begin
		GetPort(wind);
		HideWindow(wind);
		DisposeWindow(wind);
	end;


	procedure SetupDocument;
		var
			r: Rect;
			ignore: Boolean;
	begin

		if (SkelQuery(skelQHasColorQD) <> 0) then
			wind := GetNewCWindow(docWindRes, nil, WindowPtr(-1))
		else
			wind := GetNewWindow(docWindRes, nil, WindowPtr(-1));
		ignore := SkelWindow(wind, @Mouse, @Key, @Update, @Activate, nil, @Clobber, nil, false);
		SetRect(r, 10, 20, 80, 40);
		cancelBtn := NewControl(wind, r, 'Cancel', true, 0, 0, 1, pushButProc, 0);
		OffsetRect(r, 80, 0);
		okBtn := NewControl(wind, r, 'OK', true, 0, 0, 1, pushButProc, 0);

		ShowWindow(wind);
		SkelDoEvents(activMask + updateMask);
	end;

end.
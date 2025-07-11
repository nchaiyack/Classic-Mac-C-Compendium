unit MultiSkelHelp;

interface

	uses
		TransSkel, MultiSkelGlobals;

	procedure HelpWindInit;

implementation

	var

		teHelp: TEHandle;
		helpScroll: ControlHandle;
		helpLine: Integer;
		halfPage: Integer;


	procedure DoScroll (lDelta: Integer);
		var
			newLine: Integer;
	begin
		newLine := helpLine + lDelta;
		if (newLine < 0) then
			newLine := 0;
		if (newLine > GetCtlMax(helpScroll)) then
			newLine := GetCtlMax(helpScroll);
		SetCtlValue(helpScroll, newLine);
		lDelta := (helpLine - newLine) * teHelp^^.lineHeight;
		TEScroll(0, lDelta, teHelp);
		helpLine := newLine;
	end;


	procedure TrackScroll (theScroll: ControlHandle;
									partCode: Integer);
		var
			lDelta: Integer;
	begin
		if (partCode = GetCRefCon(theScroll)) then
			begin
				case partCode of
					inUpButton: 
						lDelta := -1;
					inDownButton: 
						lDelta := 1;
					inPageUp: 
						lDelta := -halfPage;
					inPageDown: 
						lDelta := halfPage;
				end;
				DoScroll(lDelta);
			end;
	end;


	procedure Mouse (pt: Point;
									t: LongInt;
									mods: Integer);
		var
			thePart: Integer;
			ignore: Integer;
	begin
		thePart := TestControl(helpScroll, pt);
		if (thePart = inThumb) then
			begin
				ignore := TrackControl(helpScroll, pt, nil);
				DoScroll(GetCtlValue(helpScroll) - helpLine);
			end
		else if (thePart <> 0) then
			begin
				SetCRefCon(helpScroll, thePart);
				ignore := TrackControl(helpScroll, pt, @TrackScroll);
			end;
	end;


	procedure Update (resized: Boolean);
		var
			r: Rect;
			visLines: Integer;
			lHeight: Integer;
			topLines: Integer;
			nLines: Integer;
			scrollLines: Integer;
	begin
		r := helpWind^.portRect;
		EraseRect(r);
		if (resized) then
			begin
				r.left := r.left + 4;
				r.bottom := r.bottom - 2;
				r.top := r.top + 2;
				r.right := r.right - 19;
				teHelp^^.destRect.right := r.right;
				teHelp^^.viewRect := r;
				TECalText(teHelp);
				lHeight := teHelp^^.lineHeight;
				nLines := teHelp^^.nLines;
				visLines := (r.bottom - r.top) div lHeight;
				halfPage := visLines div 2;
				topLines := (r.top - teHelp^^.destRect.top) div lHeight;
				scrollLines := visLines - (nLines - topLines);
				if ((scrollLines > 0) and (topLines > 0)) then
					begin
						if (scrollLines > topLines) then
							scrollLines := topLines;
						TEScroll(0, scrollLines * lHeight, teHelp);
					end;
				scrollLines := nLines - visLines;
				helpLine := (r.top - teHelp^^.destRect.top) div lHeight;
				HideControl(helpScroll);
				r := helpWind^.portRect;
				r.left := r.right - 15;
				r.bottom := r.bottom - 14;
				r.top := r.top - 1;
				r.right := r.right + 1;
				SizeControl(helpScroll, r.right - r.left, r.bottom - r.top);
				MoveControl(helpScroll, r.left, r.top);
				if ((nLines - visLines) < 0) then
					SetCtlMax(helpScroll, 0)
				else
					SetCtlMax(helpScroll, nLines - visLines);
				SetCtlValue(helpScroll, helpLine);
				ShowControl(helpScroll);
			end;
		DrawGrowBox(helpWind);
		DrawControls(helpWind);
		r := teHelp^^.viewRect;
		TEUpdate(r, teHelp);
		ValidRect(helpWind^.portRect);
	end;


	procedure Activate (active: Boolean);
	begin
		DrawGrowBox(helpWind);
		if (active) then
			begin
				DisableItem(editMenu, 0);
				if (GetCtlMax(helpScroll) > 0) then
					HiliteControl(helpScroll, normalHilite)
				else
					HiliteControl(helpScroll, dimHilite);
			end
		else
			begin
				EnableItem(editMenu, 0);
				if (GetCtlMax(helpScroll) > 0) then
					HiliteControl(helpScroll, dimHilite);
			end;
		DrawMenuBar;
	end;


	procedure Clobber;
	begin
		TEDispose(teHelp);
		DisposeControl(helpScroll);
		DisposeWindow(helpWind);
	end;


	procedure HelpWindInit;
		var
			r: Rect;
			textHandle: Handle;
			visLines: Integer;
			scrollLines: Integer;
			ignore: Boolean;
	begin
		if (SkelQuery(skelQHasColorQD) <> 0) then
			helpWind := GetNewCWindow(helpWindRes, nil, WindowPtr(-1))
		else
			helpWind := GetNewWindow(helpWindRes, nil, WindowPtr(-1));
		if (helpWind = nil) then
			exit(HelpWindInit);
		ignore := SkelWindow(helpWind, @Mouse, nil, @Update, @Activate, nil, @Clobber, nil, false);

		TextFont(0);
		TextSize(0);

		r := helpWind^.portRect;
		r.left := r.left + 4;
		r.bottom := r.bottom - 2;
		r.top := r.top + 2;
		r.right := r.right - 19;
		teHelp := TENew(r, r);
		textHandle := GetResource('TEXT', helpTextRes);
		HLock(textHandle);
		TEInsert(textHandle^, GetHandleSize(textHandle), teHelp);
		HUnlock(textHandle);
		ReleaseResource(textHandle);

		visLines := (r.bottom - r.top) div teHelp^^.lineHeight;
		scrollLines := teHelp^^.nLines - visLines;
		halfPage := visLines div 2;
		helpLine := 0;
		r := helpWind^.portRect;
		r.left := r.right - 15;
		r.bottom := r.bottom - 14;
		r.top := r.top - 1;
		r.right := r.right + 1;

		helpScroll := NewControl(helpWind, r, '', true, helpLine, 0, scrollLines, scrollBarProc, 0);

		ValidRect(helpWind^.portRect);
	end;

end.
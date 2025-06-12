program ManyWind;

	uses
		TransSkel;

	const

		maxWind = 20;

{ menu numbers }
		aMenuNum = skelAppleMenuID;
		fMenuNum = aMenuNum + 1;
		wMenuNum = fMenuNum + 1;
		cMenuNum = wMenuNum + 1;

{ File menu item numbers }
		newWind = 1;
		closeWind = 2;
		quitApp = 4;

{ Color menu item numbers }
		cWhite = 1;
		cLtGray = 2;
		cGray = 3;
		cDkGray = 4;
		cBlack = 5;

	var
		fileMenu: MenuHandle;
		windowMenu: MenuHandle;
		colorMenu: MenuHandle;

		windCount: Integer;		{ number of currently existing windows }
		windNum: Integer;		{ id of last window created }

	procedure MakeWindow;
	forward;

	procedure DoFileMenu (item: Integer);
		var
			w: WindowPtr;
	begin
		case item of
			newWind: 
				MakeWindow;
			closeWind: 
				SkelClose(FrontWindow);
			quitApp: 
				SkelStopEventLoop;
		end;
	end;


	procedure DoWindowMenu (item: Integer);
		var
			iTitle: Str255;
			wTitle: Str255;
			w: WindowPtr;
	begin
		GetItem(windowMenu, item, iTitle);		{ get window name }
		w := FrontWindow;
		while (w <> nil) do
			begin
				GetWTitle(w, wTitle);
				if (EqualString(iTitle, wTitle, false, true)) then
					begin
						SelectWindow(w);
						w := nil;
					end
				else
					w := WindowPtr(WindowPeek(w)^.nextWindow);
			end;
	end;


	procedure DoColorMenu (item: Integer);
		var
			w: WindowPtr;
	begin
		w := FrontWindow;
		if (WindowPeek(w)^.windowKind < 0) then	{ front is DA window }
			exit(DoColorMenu);
		case item of
			cWhite: 
				BackPat(white);
			cLtGray: 
				BackPat(ltGray);
			cGray: 
				BackPat(gray);
			cDkGray: 
				BackPat(dkGray);
			cBlack: 
				BackPat(black);
		end;
		EraseRect(w^.portRect);
		SetWRefCon(w, item);
	end;


	procedure DoMClobber (m: MenuHandle);
	begin
		DisposeMenu(m);
	end;


	procedure SetItemEnableState (m: MenuHandle;
									item: Integer;
									state: Boolean);
	begin
		if (state) then
			EnableItem(m, item)
		else
			DisableItem(m, item);
	end;


	procedure AdjustMenus;
		var
			nItems: Integer;
			i: Integer;
			iTitle: Str255;
			wTitle: Str255;
			mark: Byte;
	begin
		SetItemEnableState(fileMenu, newWind, windCount < maxWind);
		SetItemEnableState(fileMenu, closeWind, FrontWindow <> nil);
		if (windCount > 0) then
			begin
				for i := 1 to 5 do
					begin
						if (GetWRefCon(FrontWindow) = i) then
							mark := checkMark
						else
							mark := noMark;
						SetItemMark(colorMenu, i, char(mark));
					end;
				GetWTitle(FrontWindow, wTitle);
				nItems := CountMItems(windowMenu);
				for i := 1 to nItems do
					begin
						GetItem(windowMenu, i, iTitle);
						if (EqualString(iTitle, wTitle, false, true)) then
							mark := checkMark
						else
							mark := noMark;
						SetItemMark(windowMenu, i, char(mark));
					end;
			end;
	end;


	procedure DoWUpdate (resized: Boolean);
		var
			w: WindowPtr;
	begin
		GetPort(w);
		EraseRect(thePort^.portRect);	{ repaint w/background pattern }
	end;


	procedure DoWClose;
		var
			w: WindowPtr;
	begin
		GetPort(w);						{ window to be closed }
		SkelRmveWind(w);
	end;


	procedure DoWClobber;
		var
			w: WindowPtr;
			i: Integer;
			mItems: Integer;
			iTitle: Str255;
			wTitle: Str255;
	begin
		GetPort(w);			{ window to be close }
		GetWTitle(w, wTitle);
		DisposeWindow(w);
		windCount := windCount - 1;
		if (windCount = 0) then
			begin
				SkelRmveMenu(windowMenu);	{ last window - clobber menus }
				SkelRmveMenu(colorMenu);
			end
		else
			begin
				mItems := CountMItems(windowMenu);
				for i := 1 to mItems do
					begin
						GetItem(windowMenu, i, iTitle);
						if (EqualString(iTitle, wTitle, false, true)) then
							DelMenuItem(windowMenu, i);
					end;
			end;
	end;


	procedure MakeWindow;
		var
			w: WindowPtr;
			r: Rect;
			s: Str255;
			ignore: Boolean;
	begin
		w := FrontWindow;
		if (w = nil) then
			SetRect(r, 100, 100, 300, 250)
		else
			begin
				SkelGetWindContentRect(w, r);
				OffsetRect(r, 20, 20);
				if ((r.left > 480) or (r.top > 300)) then	{ keep on screen }
					OffsetRect(r, 40 - r.left, 40 - r.top);
			end;
		windNum := windNum + 1;
		NumToString(windNum, s);
		if (SkelQuery(skelQHasColorQD) <> 0) then
			w := NewCWindow(nil, r, s, true, noGrowDocProc, WindowPtr(-1), true, 0)
		else
			w := NewWindow(nil, r, s, true, noGrowDocProc, WindowPtr(-1), true, 0);
		ignore := SkelWindow(w, nil, nil, @DoWUpdate, nil, @DoWClose, @DoWClobber, nil, false);
		windCount := windCount + 1;
		if (windCount = 1) then		{ if first window, create new menus }
			begin
				colorMenu := NewMenu(cMenuNum, 'Color');
				AppendMenu(colorMenu, 'White;Light Gray;Gray;Dark Gray;Black');
				ignore := SkelMenu(colorMenu, @DoColorMenu, @DoMClobber, false, false);
				windowMenu := NewMenu(wMenuNum, 'Window');
				ignore := SkelMenu(windowMenu, @DoWindowMenu, @DoMClobber, false, true);
			end;
		AppendMenu(windowMenu, s);
		SetWRefCon(w, cWhite);
	end;


	procedure SetupMenus;
		var
			ignore: Boolean;
	begin
		SkelApple('', nil);
		fileMenu := NewMenu(fMenuNum, 'File');
		AppendMenu(fileMenu, 'New/N;Close/W;(-;Quit/Q');
		ignore := SkelMenu(fileMenu, @DoFileMenu, @DoMClobber, false, true);
	end;


begin
	windCount := 0;
	windNum := 0;
	SkelInit(nil);
	SetupMenus;
	SkelEventLoop;
	SkelCleanup;
end.
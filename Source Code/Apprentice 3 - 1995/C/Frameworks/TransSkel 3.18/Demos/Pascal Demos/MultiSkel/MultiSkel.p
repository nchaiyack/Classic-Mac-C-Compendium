program MultiSkel;

	uses
		TransSkel, MultiSkelRgn, MultiSkelZoom, MultiSkelEdit, MultiSkelHelp, MultiSkelGlobals;

	const

		open = 1;
		close = 2;
		quit = 4;

	var

		fileMenu: MenuHandle;


	procedure MyShowWindow (wind: WindowPtr);
	begin
		if (WindowPeek(wind)^.visible = false) then
			begin
				SelectWindow(wind);
				ShowWindow(wind);
			end;
	end;


	procedure DoAppleMenu (item: Integer);
		var
			ignore: Integer;
	begin
		ignore := SkelAlert(aboutAlrtRes, SkelDlogFilter(nil, true), skelPositionOnMainDevice);
		SkelRmveDlogFilter;
	end;


	procedure DoFileMenu (item: Integer);
	begin
		case item of
			open: 
				begin
					MyShowWindow(rgnWind);
					MyShowWindow(zoomWind);
					MyShowWindow(editWind);
					MyShowWindow(helpWind);
				end;
			close: 
				SkelClose(FrontWindow);
			quit: 
				SkelStopEventLoop;
		end;
	end;


	procedure SetupMenus;
		var
			ignore: Boolean;
	begin
		SkelApple('About MultiSkel…', @DoAppleMenu);
		fileMenu := GetMenu(fileMenuRes);
		ignore := SkelMenu(fileMenu, @DoFileMenu, nil, false, false);
		editMenu := GetMenu(editMenuRes);
		ignore := SkelMenu(editMenu, @EditWindEditMenu, nil, false, true);
	end;


	procedure SetWaitTimes;
		var
			f: LongInt;
			b: LongInt;
	begin
		SkelGetWaitTimes(f, b);
		b := f;
		SkelSetWaitTimes(f, b);
	end;

begin
	SkelInit(nil);
	SetWaitTimes;
	SetupMenus;
	RgnWindInit;
	ZoomWindInit;
	EditWindInit;
	HelpWindInit;
	SkelEventLoop;
	SkelCleanup;
end.
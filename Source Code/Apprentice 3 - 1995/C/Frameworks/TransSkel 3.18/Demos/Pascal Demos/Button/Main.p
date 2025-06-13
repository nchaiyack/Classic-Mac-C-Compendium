{ TransSkel button application in Pascal }

{ 10 Feb 94 Version 1.00, Paul DuBois }

program Button;

	uses
		TransSkel, ButtonGlobals, Modal1, Modal2, Modal3, Document, Modeless, Movable;

	const

		doModal1Item = 1;
		doModal2Item = 2;
		doModal3Item = 3;
		doMovableItem = 4;
	{ sepLine }
		quitAppItem = 6;


{--------------------------------------------------------------------}
{ Menu handling procedures }
{--------------------------------------------------------------------}


{ Handle selection of "About Button..." item from Apple menu }

	procedure DoAppleMenu (item: Integer);
		var
			ignore: Integer;
	begin
		ignore := SkelAlert(aboutAlrtRes, SkelDlogFilter(nil, true), skelPositionOnParentDevice);
		SkelRmveDlogFilter;
	end;


{ Process selection from File menu }

	procedure DoFileMenu (item: Integer);
	begin
		case item of
			doModal1Item: 
				DoModal1;
			doModal2Item: 
				DoModal2;
			doModal3Item: 
				DoModal3;
			doMovableItem: 
				DoMovableModal;
			quitAppItem: 
				SkelStopEventLoop;
		end;
	end;


	procedure AdjustMenus;
		var
			m: MenuHandle;
	begin
		m := GetMHandle(skelAppleMenuID);
		if (SkelIsMMDlog(FrontWindow)) then
			DisableItem(m, 1)
		else
			EnableItem(m, 1);
		m := GetMHandle(fileMenuRes);
		if (SkelIsMMDlog(FrontWindow)) then
			begin
				DisableItem(m, doModal1Item);
				DisableItem(m, doModal2Item);
				DisableItem(m, doModal3Item);
				DisableItem(m, doMovableItem);
			end
		else
			begin
				EnableItem(m, doModal1Item);
				EnableItem(m, doModal2Item);
				EnableItem(m, doModal3Item);
				EnableItem(m, doMovableItem);
			end;
	end;


{ Initialize menus.  Tell TransSkel to process the Apple menu }
{ automatically, and associate the proper procedures with the }
{ File menu. }

	procedure SetupMenus;
		var
			m: MenuHandle;
			ignore: Boolean;
	begin
		SkelApple('About Button…', @DoAppleMenu);
		m := GetMenu(fileMenuRes);
		ignore := SkelMenu(m, @DoFileMenu, nil, false, true);
		if (SkelQuery(skelQSysVersion) < $00000700) then
			DisableItem(m, doMovableItem);
	end;


begin
	SkelInit(nil);
	horizRatio := FixRatio(1, 2);
	vertRatio := FixRatio(1, 5);
	SetupMenus;
	SetupDocument;
	SetupModeless;
	InitMovableModal;
	SkelEventLoop;
	SkelCleanup;
end.
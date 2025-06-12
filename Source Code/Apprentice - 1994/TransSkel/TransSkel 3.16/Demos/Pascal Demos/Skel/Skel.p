program Skel;

	uses
		TransSkel;

	const
		fileMenuRes = 129;		{ File menu }
		aboutAlrtRes = 1000;	{ About box }
		theWindRes = 260;		{ Window }
		reportDlog = 257;		{ message dialog box }
		aboutStr = 128;			{ message strings }
		rattleStr = 129;
		frightStr = 130;

		{ dialog item numbers }

		okayItem = 1;
		messageItem = 2;
		titleItem = 3;
		outlineItem = 4;

		{ File menu item numbers }

		rattle = 1;
		frighten = 2;
		{ gray line }
		quit = 4;

	var

		theWind: WindowPtr;
		fileMenu: MenuHandle;


	{ Read a string resource and put into the Alert/Dialog paramtext values }

	procedure SetParamText (strNum: Integer);
		var
			h: StringHandle;
			flags: SignedByte;
	begin
		h := GetString(strNum);
		flags := HGetState(Handle(h));
		HLock(Handle(h));
		ParamText(h^^, '', '', '');
		HSetState(Handle(h), flags);
	end;


	{ Put up a dialog box with a message and an OK button.  The message }
	{ is stored in the 'STR ' resource whose number is passed as strNum. }

	procedure Report (strNum: Integer);
		var
			theDialog: DialogPtr;
			savePort: GrafPtr;
			itemHit: Integer;
	begin
		SetParamText(strNum);
		theDialog := GetNewDialog(reportDlog, nil, WindowPtr(-1));
		SkelPositionWindow(theDialog, skelPositionOnParentDevice, FixRatio(1, 2), FixRatio(1, 5));

		GetPort(savePort);
		SetPort(theDialog);
		SkelSetDlogButtonOutliner(theDialog, outlineItem);
		ShowWindow(theDialog);
		ModalDialog(SkelDlogFilter(nil, true), itemHit);
		SkelRmveDlogFilter;
		DisposeDialog(theDialog);
		SetPort(savePort);
	end;


	{ Handle selection of "About Skel..." item from Apple menu }

	procedure DoAppleMenu (item: Integer);
		var
			ignore: Integer;
	begin
		SetParamText(aboutStr);
		ignore := SkelAlert(aboutAlrtRes, SkelDlogFilter(nil, true), skelPositionOnParentDevice);
		SkelRmveDlogFilter;
	end;


	{ Process selection from File menu. }
	{ Rattle, Frighten	A dialog box with message }
	{ Quit				Request a halt by calling SkelStopEventLoop().  This }
	{					makes SkelMain return. }

	procedure DoFileMenu (item: Integer);
	begin
		case item of
			rattle: 
				Report(rattleStr);
			frighten: 
				Report(frightStr);
			quit: 
				SkelStopEventLoop;	{ request halt }
		end;
	end;


	{ Initialize menus . Tell TransSkel to process the Apple menu }
	{ automatically, and associate the proper procedures with the }
	{ File and Edit menus . }
	{ \311 is the ellipsis character. }

	procedure SetupMenus;
		var
			ignore: Boolean;
	begin
		SkelApple('About Skel…', @DoAppleMenu);
		fileMenu := GetMenu(fileMenuRes);
		ignore := SkelMenu(fileMenu, @DoFileMenu, nil, false, true);
	end;


{ Window Handling Routines }

{ On update event, can ignore the resizing information, since the whole }
{ window is always redrawn in terms of the current size, anyway. }
{ Content area is dark gray except scroll bar areas, which are white. }
{ Draw grow box as well. }

{ Note that we can assume the current port is set to theWind. }

	procedure Update (resized: Boolean);
		var
			r: Rect;
	begin
		r := theWind^.portRect;			{ paint window dark gray }
		r.bottom := r.bottom - 15;		{ don't bother painting the }
		r.right := r.right - 15;			{ scroll bar areas }
		FillRect(r, dkGray);
		r := theWind^.portRect;			{ paint scroll bar areas white }
		r.left := r.right - 15;
		FillRect(r, white);
		r := theWind^.portRect;
		r.top := r.bottom - 15;
		FillRect(r, white);
		DrawGrowIcon(theWind);
	end;


	procedure Activate (active: Boolean);
	begin
		DrawGrowIcon(theWind);		{ make grow box reflect new window state }
	end;


	procedure Clobber;
	begin
		DisposeWindow(theWind);
	end;


{ Read window from resource file and install handler for it.  Mouse }
{ and key clicks are ignored.  There is no close proc since the window }
{ doesn't have a close box.  There is no idle proc since nothing is }
{ done while the window is in front (all the things that are done are }
{ handled by TransSkel). }

	procedure WindInit;
		var
			ignore: Boolean;
	begin
		if (SkelQuery(skelQHasColorQD) <> 0) then
			theWind := GetNewCWindow(theWindRes, nil, WindowPtr(-1))
		else
			theWind := GetNewWindow(theWindRes, nil, WindowPtr(-1));
		ignore := SkelWindow(theWind, nil, nil, @Update, @Activate, nil, @Clobber, nil, false);
	end;


begin
	SkelInit(nil);
	SetupMenus;
	WindInit;
	SkelEventLoop;
	SkelCleanup;
end.
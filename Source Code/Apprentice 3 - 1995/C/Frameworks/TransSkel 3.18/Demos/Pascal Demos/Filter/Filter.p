program Filter;

	uses
		TransSkel, Sicn;

	const

		aboutAlrtRes = 1000;		{ About box }
		jumpyDlogRes = 1001;		{ jumping dialog box }
		dirSicnRes = 1001;			{ direction SICNs }

{ Apple menu item numbers }

		aboutFilter = 1;
		aboutNoFilter = 2;

{ File menu and items numbers }

		fileMenuNum = 1001;

		jumpyFilter = 1;
		jumpyNoFilter = 2;
		quit = 4;

	var

		theWind: WindowPtr;
		fileMenu: MenuHandle;

		hSicn: SicnHandle;
		warpBox: ControlHandle;
		warpVal: Integer;


	procedure DrawDlogUserItem (dlog: DialogPtr;
									item: Integer);
		var
			r: Rect;
	begin
		SkelGetDlogRect(dlog, item, r);
		PlotSicn(r, hSicn, item - 2);
	end;


	function TrackRect (r: Rect): Boolean;
		var
			i: Boolean;
			inRect: Boolean;
			pt: Point;
	begin
		InvertRect(r);
		inRect := true;
		while (StillDown) do
			begin
				GetMouse(pt);
				i := PtInRect(pt, r);
				if (inRect and not i) then		{ was in, now out }
					begin
						InvertRect(r);
						inRect := false;
					end
				else if (not inRect and i) then	{ was out, now in}
					begin
						InvertRect(r);
						inRect := true;
					end;
			end;
		if (inRect) then
			InvertRect(r);
		TrackRect := inRect;
	end;


	function DlogFilter (d: DialogPtr;
									e: EventRecord;
									var item: Integer): Boolean;
		var
			pt: Point;
			r: Rect;
			i: Integer;
	begin
		DlogFilter := false;
		if (e.what = mouseDown) then
			begin
				pt := e.where;
				GlobalToLocal(pt);
				for i := 2 to 18 do
					begin
						SkelGetDlogRect(d, i, r);
						if (PtInRect(pt, r)) then
							begin
								if (TrackRect(r)) then
									begin
										item := i;
										DlogFilter := true;
									end;
							end;
					end;
			end;
	end;


	function DlogFilter2 (d: DialogPtr;
									e: EventRecord;
									var item: Integer): Boolean;
	begin
		if (SkelDlogMapKeyToButton(d, e, item, 1, 0)) then
			DlogFilter2 := true
		else
			DlogFilter2 := DlogFilter(d, e, item);
	end;


	procedure JumpyDialog (useSkelFilter: Boolean);
		const
			delta = 20;
		type
			IntegerPtr = ^Integer;
			PointPtr = ^Point;
		var
			dlog: DialogPtr;
			savePort: GrafPtr;
			itemHit: Integer;
			pt: Point;
			mousePt: Point;
			scrnRect: Rect;
			dlogRect: Rect;
			dlogHt: Integer;
			dlogWid: Integer;
			culh: Integer;
			culv: Integer;
			i: Integer;
			intPtr: IntegerPtr;
			ptPtr: PointPtr;
			loop: Boolean;
			ignore: Boolean;
			fakeGD: GDHandle;
	begin
		hSicn := SicnHandle(GetResource('SICN', dirSicnRes));
		if (hSicn = nil) then
			begin
				SysBeep(1);
				exit(JumpyDialog);
			end;
		LoadResource(Handle(hSicn));
		GetPort(savePort);
		dlog := GetNewDialog(jumpyDlogRes, nil, WindowPtr(-1));

		SkelPositionWindow(dlog, skelPositionOnMainDevice, FixRatio(1, 2), FixRatio(1, 5));
		SkelSetDlogButtonOutliner(dlog, 19);

		warpBox := SkelGetDlogCtl(dlog, 20);
		SetCtlValue(warpBox, warpVal);

		for i := 2 to 18 do
			begin
				SkelSetDlogProc(dlog, i, @DrawDlogUserItem);
			end;

		dlogRect := dlog^.portRect;
		dlogWid := dlogRect.right - dlogRect.left;
		dlogHt := dlogRect.bottom - dlogRect.top;

		ignore := SkelGetWindowDevice(dlog, fakeGD, scrnRect);
		InsetRect(scrnRect, 8, 8);

		culh := scrnRect.left + (scrnRect.right - scrnRect.left - dlogWid) div 2;
		culv := scrnRect.top + (scrnRect.bottom - scrnRect.top - dlogHt) div 2;

		SetPort(dlog);
		ShowWindow(dlog);

		loop := true;
		while (loop) do
			begin
				if (useSkelFilter) then
					begin
						ModalDialog(SkelDlogFilter(@DlogFilter, true), itemHit);
						SkelRmveDlogFilter;
					end
				else
					ModalDialog(@DlogFilter2, itemHit);

				if (itemHit = 1) then
					loop := false
				else if (itemHit = 20) then
					warpVal := SkelToggleDlogCtlValue(dlog, 20)
				else
					begin
						pt.h := dlog^.portRect.left;
						pt.v := dlog^.portRect.top;
						LocalToGlobal(pt);
						GetMouse(mousePt);
						LocalToGlobal(mousePt);
						mousePt.h := mousePt.h - pt.h;
						mousePt.v := mousePt.v - pt.v;
						case itemHit of
							2: 
								begin
									pt.h := culh;
									pt.v := culv;
								end;
							3: 
								begin
									pt.v := pt.v - delta;
								end;
							4: 
								begin
									pt.h := pt.h + delta;
									pt.v := pt.v - delta;
								end;
							5: 
								begin
									pt.h := pt.h + delta;
								end;
							6: 
								begin
									pt.h := pt.h + delta;
									pt.v := pt.v + delta;
								end;
							7: 
								begin
									pt.v := pt.v + delta;
								end;
							8: 
								begin
									pt.h := pt.h - delta;
									pt.v := pt.v + delta;
								end;
							9: 
								begin
									pt.h := pt.h - delta;
								end;
							10: 
								begin
									pt.h := pt.h - delta;
									pt.v := pt.v - delta;
								end;
							11: 
								begin
									pt.h := culh;
									pt.v := scrnRect.top;
								end;
							12: 
								begin
									pt.h := scrnRect.right - dlogWid;
									pt.v := scrnRect.top;
								end;
							13: 
								begin
									pt.h := scrnRect.right - dlogWid;
									pt.v := culv;
								end;
							14: 
								begin
									pt.h := scrnRect.right - dlogWid;
									pt.v := scrnRect.bottom - dlogHt;
								end;
							15: 
								begin
									pt.h := culh;
									pt.v := scrnRect.bottom - dlogHt;
								end;
							16: 
								begin
									pt.h := scrnRect.left;
									pt.v := scrnRect.bottom - dlogHt;
								end;
							17: 
								begin
									pt.h := scrnRect.left;
									pt.v := culv;
								end;
							18: 
								begin
									pt.h := scrnRect.left;
									pt.v := scrnRect.top;
								end;
						end;
						if (pt.h < scrnRect.left) then
							pt.h := scrnRect.left;
						if (pt.h > scrnRect.right - dlogWid) then
							pt.h := scrnRect.right - dlogWid;
						if (pt.v < scrnRect.top) then
							pt.v := scrnRect.top;
						if (pt.v > scrnRect.bottom - dlogHt) then
							pt.v := scrnRect.bottom - dlogHt;
						if (warpVal <> 0) then
							begin
								HideCursor;
								mousePt.h := mousePt.h + pt.h;
								mousePt.v := mousePt.v + pt.v;
								{ warp the mouse pointer }
								ptPtr := PointPtr($082c);
								ptPtr^ := mousePt;
								ptPtr := PointPtr($0828);
								ptPtr^ := mousePt;
								intPtr := IntegerPtr($08ce);
								intPtr^ := $ffff;
							end;
						MoveWindow(dlog, pt.h, pt.v, false);
						if (warpVal <> 0) then
							ShowCursor;
					end;
			end;
		DisposeDialog(dlog);

		ReleaseResource(Handle(hSicn));
		SetPort(savePort);
	end;


	procedure DoAppleMenu (item: Integer);
		var
			ignore: Integer;
	begin
		case item of
			aboutFilter: 
				begin
					ignore := SkelAlert(aboutAlrtRes, SkelDlogFilter(nil, true), skelPositionOnParentDevice);
					SkelRmveDlogFilter;
				end;
			aboutNoFilter: 
				ignore := SkelAlert(aboutAlrtRes, nil, skelPositionOnParentDevice);
		end;
	end;


	procedure DoFileMenu (item: Integer);
	begin
		case item of
			jumpyFilter: 
				JumpyDialog(true);
			jumpyNoFilter: 
				JumpyDialog(false);
			quit: 
				SkelStopEventLoop;
		end;
	end;


	procedure SetUpMenus;
		var
			ignore: Boolean;
	begin
		SkelApple('Filtered “About Filter”…;Unfiltered “About Filter”…', @DoAppleMenu);
		fileMenu := NewMenu(fileMenuNum, 'File');
		AppendMenu(fileMenu, 'Filtered Jump Dialog…/F');
		AppendMenu(fileMenu, 'Unfiltered Jump Dialog…/U');
		AppendMenu(fileMenu, '(-;Quit/Q');
		ignore := SkelMenu(fileMenu, @DoFileMenu, nil, false, true);
	end;


	procedure Activate (active: Boolean);
	begin
		DrawGrowIcon(theWind);
	end;


	procedure Update (resized: Boolean);
		var
			r: Rect;
	begin
		r := theWind^.portRect;			{ paint window dark gray }
		r.bottom := r.bottom - 15;		{ don't bother painting the }
		r.right := r.right - 15;		{ scroll bar areas white }
		FillRect(r, ltGray);
		r := theWind^.portRect;			{ paint scroll bar areas white }
		r.left := r.right - 15;
		FillRect(r, white);
		r := theWind^.portRect;
		r.top := r.bottom - 15;
		FillRect(r, white);
		DrawGrowIcon(theWind);
	end;


	procedure Clobber;
	begin
		DisposeWindow(theWind);
	end;


	procedure WindInit;
		var
			bounds: Rect;
			ignore: Boolean;
	begin
		SetRect(bounds, 0, 0, 500, 282);
		OffsetRect(bounds, 4, 40);
		if (SkelQuery(skelQHasColorQD) <> 0) then
			theWind := NewCWindow(nil, bounds, 'The Dialog’s Accomplice', true, documentProc + 8, WindowPtr(-1), false, 0)
		else
			theWind := NewWindow(nil, bounds, 'The Dialog’s Accomplice', true, documentProc + 8, WindowPtr(-1), false, 0);
		ignore := SkelWindow(theWind, nil, nil, @Update, @Activate, nil, @Clobber, nil, false);
	end;


begin
	warpVal := 0;			{ mouse warp is off initially }
	SkelInit(nil);
	SetUpMenus;
	WindInit;
	SkelEventLoop;
	SkelCleanup;
end.
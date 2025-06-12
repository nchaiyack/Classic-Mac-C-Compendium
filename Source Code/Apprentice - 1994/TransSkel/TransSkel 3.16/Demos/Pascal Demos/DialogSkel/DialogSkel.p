{ TransSkel DialogSkel application in Pascal }

{ 11 Feb 94 Version 1.00, Paul DuBois }

program DialogSkel;

	uses
		TransSkel;

	const

		normalHilite = 0;
		dimHilite = 255;

		fileMenuID = skelAppleMenuID + 1;
		editMenuID = fileMenuID + 1;

		mDlogRes = 1000;
		aboutAlrtRes = 1001;
		docWindRes = 1000;

		showDlog1 = 1;
		showDlog2 = 2;
		showDoc = 3;
		closeWind = 4;
{ separator line }
		quit = 6;

		undo = 1;
{ separator line }
		cut = 3;
		copy = 4;
		paste = 5;
		clear = 6;

		button1 = 1;
		edit1 = 2;
		static1 = 3;
		radio1 = 4;
		radio2 = 5;
		radio3 = 6;
		check1 = 7;
		check2 = 8;
		user1 = 9;

	var

		mDlog1: DialogPtr;
		mDlog2: DialogPtr;
		docWind: WindowPtr;
		iconNum1: Integer;
		iconNum2: Integer;

		fileMenu: MenuHandle;
		editMenu: MenuHandle;

		hidden1: Boolean;			{ flags for keeping track of dialog }
		hidden2: Boolean;			{ visibility when a suspend occurs }

{--------------------------------------------------------------------}
{ Miscellaneous stuff }
{--------------------------------------------------------------------}

	procedure DrawIcon (dlog: DialogPtr;
									item: Integer);
		var
			h: Handle;
			r: Rect;
	begin
		SkelGetDlogRect(dlog, item, r);
		if (dlog = mDlog1) then
			h := GetIcon(iconNum1)
		else
			h := GetIcon(iconNum2);
		PlotIcon(r, h);
	end;


	procedure SetDlogRadio (dlog: DialogPtr;
									item: Integer);
		var
			partner: DialogPtr;
			tmpPort: GrafPtr;
			r: Rect;
	begin
		partner := DialogPtr(GetWRefCon(dlog));
		SkelSetDlogRadioButtonSet(dlog, radio1, radio3, item);

		if (partner = mDlog1) then
			iconNum1 := item - radio1
		else
			iconNum2 := item - radio1;

		SkelGetDlogRect(partner, user1, r);
		GetPort(tmpPort);
		SetPort(partner);
		InvalRect(r);		{ invalidate item rect to generate update rect }
		SetPort(tmpPort);
	end;


{--------------------------------------------------------------------}
{ Dialog window setup and handler routines }
{--------------------------------------------------------------------}


	function DlogFilter (dlog: DialogPtr;
									evt: EventRecord;
									var item: Integer): Boolean;
		var
			hilite: Integer;
			c: char;
			str: Str255;
			ignore: Boolean;
	begin
		DlogFilter := false;
		SkelGetDlogStr(dlog, edit1, str);
		if (length(str) = 0) then
			hilite := dimHilite
		else
			hilite := normalHilite;
		if ((evt.what = activateEvt) and (BitAnd(evt.modifiers, activeFlag) = 0)) then
			hilite := dimHilite;
		if (dlog <> FrontWindow) then
			hilite := dimHilite;
		if (SkelSetDlogCtlHilite(dlog, button1, hilite)) then
			SkelDrawButtonOutline(SkelGetDlogCtl(dlog, button1));
		case evt.what of
			nullEvent: 
				SkelSetDlogCursor(dlog);
			keyDown: 
				DlogFilter := SkelDlogMapKeyToButton(dlog, evt, item, button1, 0);
			updateEvt: 
				SkelDrawButtonOutline(SkelGetDlogCtl(dlog, button1));
			activateEvt:
{ Accept button and outline are set above.  Set other controls here. }
				begin
					if (BitAnd(evt.modifiers, activeFlag) <> 0) then
						hilite := normalHilite
					else
						hilite := dimHilite;
					ignore := SkelSetDlogCtlHilite(dlog, radio1, hilite);
					ignore := SkelSetDlogCtlHilite(dlog, radio2, hilite);
					ignore := SkelSetDlogCtlHilite(dlog, radio3, hilite);
					ignore := SkelSetDlogCtlHilite(dlog, check1, hilite);
					ignore := SkelSetDlogCtlHilite(dlog, check2, hilite);
				end;
		end;
	end;


	procedure DlogSelect (actor: DialogPtr;
									item: Integer);
		var
			partner: DialogPtr;
			title: Str255;
	begin
		partner := DialogPtr(GetWRefCon(actor));
		case item of
			button1: 
				begin
					SkelGetDlogStr(actor, edit1, title);
					SetWTitle(partner, title);
				end;
			radio1, radio2, radio3: 
				SetDlogRadio(actor, item);
			check1: 
				if (SkelToggleDlogCtlValue(actor, item) <> 0) then
					ShowWindow(partner)
				else
					HideWindow(partner);
			check2: 
				if (SkelToggleDlogCtlValue(actor, item) <> 0) then
					WindowPeek(partner)^.goAwayFlag := Boolean(255)
				else
					WindowPeek(partner)^.goAwayFlag := Boolean(0);
		end;
	end;


	procedure DlogClose;
		var
			actor: DialogPtr;
			partner: DialogPtr;
	begin
		GetPort(actor);
		partner := DialogPtr(GetWRefCon(actor));
		HideWindow(actor);
		SkelSetDlogCtlValue(partner, check1, 0);
	end;


	procedure DlogClobber;
		var
			dlog: DialogPtr;
	begin
		GetPort(dlog);
		DisposeDialog(dlog);
	end;


	function DemoDialog (title: Str255;
									h: Integer;
									v: Integer): DialogPtr;
		var
			dlog: DialogPtr;
			ignore: Boolean;
	begin
		dlog := GetNewDialog(mDlogRes, nil, WindowPtr(-1));
		MoveWindow(dlog, h, v, false);
		SetWTitle(dlog, title);
		ignore := SkelDialog(dlog, @DlogFilter, @DlogSelect, @DlogClose, @DlogClobber);
		DemoDialog := dlog;
	end;


{--------------------------------------------------------------------}
{ Document window setup and handler routines }
{--------------------------------------------------------------------}


	procedure DocUpdate (resized: Boolean);
	begin
	end;


	procedure DocActivate (active: Boolean);
	begin
	end;


	procedure DocClobber;
	begin
		HideWindow(docWind);
		DisposeWindow(docWind);
	end;


	procedure DocWindow (h: Integer;
									v: Integer);
		var
			ignore: Boolean;
	begin
		if (SkelQuery(skelQHasColorQD) <> 0) then
			docWind := GetNewCWindow(docWindRes, nil, WindowPtr(-1))
		else
			docWind := GetNewWindow(docWindRes, nil, WindowPtr(-1));
		ignore := SkelWindow(docWind, nil, nil, @DocUpdate, @DocActivate, nil, @DocClobber, nil, false);
		MoveWindow(docWind, h, v, false);
	end;


{--------------------------------------------------------------------}
{ Menu handlers }
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
			showDlog1: 
				begin
					SelectWindow(mDlog1);
					ShowWindow(mDlog1);
					SkelSetDlogCtlValue(mDlog2, check1, 1);
				end;
			showDlog2: 
				begin
					SelectWindow(mDlog2);
					ShowWindow(mDlog2);
					SkelSetDlogCtlValue(mDlog1, check1, 1);
				end;
			showDoc: 
				begin
					SelectWindow(docWind);
					ShowWindow(docWind);
				end;
			closeWind: 
				SkelClose(FrontWindow);
			quit: 
				SkelStopEventLoop;
		end;
	end;


	procedure DoEditMenu (item: Integer);
		var
			dlog: DialogPtr;
			ignore: Integer;
	begin
		if (SystemEdit(item - 1)) then	{ if DA handled operation, return }
			exit(DoEditMenu);

{ if front window is document window, do nothing }
		dlog := DialogPtr(FrontWindow);
		if (WindowPeek(dlog)^.windowKind <> dialogKind) then
			exit(DoEditMenu);
		case item of
			cut: 
				begin
					DlgCut(dlog);
					ignore := ZeroScrap;
					ignore := TEToScrap;
				end;
			copy: 
				begin
					DlgCopy(dlog);
					ignore := ZeroScrap;
					ignore := TEToScrap;
				end;
			paste: 
				begin
					ignore := TEFromScrap;
					DlgPaste(dlog);
				end;
			clear: 
				DlgDelete(dlog);
		end;
	end;


{ Adjust menus when mouse click occurs in menu bar, }
{ before menus are shown. }

	procedure AdjustMenus;
		var
			w: WindowPtr;
	begin
		w := FrontWindow;
		if (w = nil) then
			DisableItem(fileMenu, closeWind)
		else
			EnableItem(fileMenu, closeWind);
		if (w = docWind) then
			begin
				DisableItem(editMenu, undo);
				DisableItem(editMenu, cut);
				DisableItem(editMenu, copy);
				DisableItem(editMenu, paste);
				DisableItem(editMenu, clear);
			end
		else
			begin
				{ modeless dialog or DA -- dim undo for dialogs }
				if (WindowPeek(w)^.windowKind = dialogKind) then
					DisableItem(editMenu, undo)
				else
					EnableItem(editMenu, undo);

				EnableItem(editMenu, cut);
				EnableItem(editMenu, copy);
				EnableItem(editMenu, paste);
				EnableItem(editMenu, clear);
			end;
	end;


{--------------------------------------------------------------------}
{ Suspend/resume handler }
{--------------------------------------------------------------------}


	procedure SuspendResume (inForeground: Boolean);
		var
			w: WindowPtr;
			event: EventRecord;
			ignore: Boolean;
	begin
		if (inForeground = false) then
			begin
				hidden1 := false;
				hidden2 := false;
				if (WindowPeek(mDlog1)^.visible) then
					begin
						ShowHide(mDlog1, false);
						hidden1 := true;
					end;
				if (WindowPeek(mDlog2)^.visible) then
					begin
						ShowHide(mDlog2, false);
						hidden2 := true;
					end;
				w := FrontWindow;
				if (w <> nil) then
					begin
						HiliteWindow(w, false);
						SkelActivate(w, false);
					end;
			end
		else
			begin
				w := FrontWindow;
				if (w <> nil) then
					HiliteWindow(w, false);
				if (hidden1) then
					ShowHide(mDlog1, true);
				if (hidden2) then
					ShowHide(mDlog2, true);
				w := FrontWindow;
				if (w <> nil) then
					begin
						HiliteWindow(w, true);
						SkelActivate(w, true);
					end;
				if (EventAvail(mDownMask, event)) then
					ignore := GetNextEvent(mDownMask, event);
			end;
	end;


{--------------------------------------------------------------------}
{ Main program }
{--------------------------------------------------------------------}


	procedure Initialize;
		var
			ignore: Boolean;
	begin
		iconNum1 := 0;
		iconNum2 := 0;

		SkelInit(nil);

		SkelSetSuspendResume(@SuspendResume);

		SkelApple('About DialogSkel…', @DoAppleMenu);

		fileMenu := NewMenu(fileMenuID, 'File');
		AppendMenu(fileMenu, 'Show Dialog 1;Show Dialog 2;Show Doc Window');
		AppendMenu(fileMenu, 'Close/W;(-;Quit/Q');
		ignore := SkelMenu(fileMenu, @DoFileMenu, nil, false, false);

		editMenu := NewMenu(editMenuID, 'Edit');
		AppendMenu(editMenu, '(Undo/Z;(-;Cut/X;Copy/C;Paste/V;Clear');
		ignore := SkelMenu(editMenu, @DoEditMenu, nil, false, false);

		DrawMenuBar;
		SkelSetMenuHook(@AdjustMenus);

		DocWindow(100, 125);

		mDlog1 := DemoDialog('Modeless Dialog 1', 50, 50);
		mDlog2 := DemoDialog('Modeless Dialog 2', 150, 200);
		SetWRefCon(mDlog1, LongInt(mDlog2));
		SetWRefCon(mDlog2, LongInt(mDlog1));
		SkelSetDlogStr(mDlog1, edit1, 'Modeless Dialog 2');
		SkelSetDlogStr(mDlog2, edit1, 'Modeless Dialog 1');
		SkelSetDlogProc(mDlog1, user1, @DrawIcon);
		SkelSetDlogProc(mDlog2, user1, @DrawIcon);
		SkelSetDlogRadioButtonSet(mDlog1, radio1, radio3, radio1);
		SkelSetDlogRadioButtonSet(mDlog2, radio1, radio3, radio1);
		SkelSetDlogCtlValue(mDlog1, check1, 1);
		SkelSetDlogCtlValue(mDlog2, check1, 1);
		SkelSetDlogCtlValue(mDlog1, check2, 1);
		SkelSetDlogCtlValue(mDlog2, check2, 1);

		SelectWindow(docWind);
		ShowWindow(docWind);
		SkelDoEvents(activMask + updateMask);
		SelectWindow(mDlog1);
		ShowWindow(mDlog1);
		SkelDoEvents(activMask + updateMask);
		SelectWindow(mDlog2);
		ShowWindow(mDlog2);
		SkelDoEvents(activMask + updateMask);
	end;


begin
	Initialize;
	SkelEventLoop;
	SkelCleanup;
end.
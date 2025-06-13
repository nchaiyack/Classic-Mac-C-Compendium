unit ICWindows;

interface

	uses
		ICWindowGlobals;

	function InitICWindows: OSErr;

	function WindowsOpen (wt: WindowType): OSErr;
	function WindowsClose (wp: WindowPtr): OSErr;
	function WindowsFlushAll: OSErr;	(* close all information windows *)
	function WindowsCloseAll: OSErr;	(* close all information windows *)

	procedure WindowsSetTitle (wt: WindowType; title: Str255);
	procedure WindowsIdle;
	procedure WindowActivateDeactivate (window: WindowPtr; activate: boolean);
	procedure WindowItemWhere (window: WindowPtr; er: EventRecord; item: integer);
	procedure WindowsDoKey (er: EventRecord);
	function WindowsEarlyHandleEvent (er: EventRecord): boolean;
	function WindowsEarlyHandleKey (er: EventRecord): boolean;
	procedure WindowsAdjustMenus;
	procedure WindowsDoEditMenu (item: integer);

	procedure WindowsRestorePositions;
	procedure WindowsSavePositions;
	procedure WindowsResetPositions;

	function GetWindowPtr (wt: WindowType): WindowPtr;

	function EditCurrentPreference (var key: Str255): OSErr;

implementation

	uses
		IconFamilies, 

		ICMiscSubs, ICSubs, ICTypes, ICAPI, ICDialogs, ICText, ICGlobals, 

		ICTextWhats, ICPopupWhats, ICFontWhats, ICFSSpecWhats, ICFileMapWhat, ICHelperWhat, ICButtonWhat, 

		ICDocUtils;


	function CallWhatOpen (wt: WindowType; item: integer; open: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatClose (wt: WindowType; item: integer; close: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatFlush (wt: WindowType; item: integer; close: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatClick (wt: WindowType; item: integer; er: eventRecord; close: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatKey (wt: WindowType; item: integer; er: eventRecord; key: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatActivate (wt: WindowType; item: integer; activate: boolean; actproc: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatIdle (wt: WindowType; item: integer; idleproc: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function CallWhatCursor (wt: WindowType; item: integer; pt: Point; cursorid: integer; cursorproc: ProcPtr): OSErr;
	inline
		$205F, $4E90;

	function TypeToWhat (typ: OSType): integer;
		var
			i: integer;
	begin
		TypeToWhat := 1;
		for i := 1 to whats_max do begin
			if whatinfo[i].typ = typ then begin
				TypeToWhat := i;
				leave;
			end;
		end;
	end;

	function GetWhatType (wt: WindowType; item: integer): OSType;
		var
			wrp: WhatRecordPtr;
	begin
		wrp := windowinfo[wt].items[item];
		if wrp = nil then begin
			GetWhatType := 'NULL';
		end
		else begin
			GetWhatType := wrp^.typ;
		end;
	end;

	function GetWindowPtr (wt: WindowType): WindowPtr;
	begin
		GetWindowPtr := windowinfo[wt].window;
	end;

	function OurWindow (wp: WindowPtr): boolean;
	begin
		OurWindow := GetWindowType(wp) <> WT_None;
	end;

	function WhatIdleText (wt: WindowType; item: integer; var cursor: integer): OSErr;
		var
			r: rect;
			pt: Point;
	begin
		GetDItemRect(windowinfo[wt].window, item, r);
		GetMouse(pt);
		if PtInRect(pt, r) then begin
			cursor := iBeamCursor;
		end;
		WhatIdleText := noErr;
	end;

	procedure DoWindowIdle (window: WindowPtr);
		var
			selected_item, i: integer;
			what: WhatRecordPtr;
			idleproc: ProcPtr;
			wt: WindowType;
			err: OSErr;
			cursorid: integer;
			r: rect;
			pt: Point;
			cursor: ProcPtr;
			junk: OSErr;
			cursor_set: boolean;
	begin
		SetPort(window);
		wt := GetWindowType(window);
		selected_item := GetSelectedItem(GetWindowType(window));
		if selected_item > 0 then begin
			TextIdle(windowinfo[GetWindowType(window)].items[selected_item]^.data);
		end;
		if (window = FrontWindow) & InForeground then begin
			GetMouse(pt);
			cursor_set := false;
			for i := 1 to item_max do begin
				what := windowinfo[wt].items[i];
				if what <> nil then begin
					GetDItemRect(window, i, r);
					if PtInRect(pt, r) then begin
						cursor_set := true;
						cursorid := whatinfo[TypeToWhat(what^.typ)].cursorid;
						cursor := whatinfo[TypeToWhat(what^.typ)].cursor;
						if cursor = nil then begin
							if cursorid = 0 then begin
								InitCursor;
							end
							else begin
								SetCursor(GetCursor(cursorid)^^);
							end;
						end
						else begin
							junk := CallWhatCursor(wt, i, pt, cursorid, cursor);
						end; (* if *)
					end;
				end; (* if *)
			end; (* for *)
			if not cursor_set then begin
				InitCursor;
			end; (* if *)
		end;
	end;

	procedure WindowDoKey (window: WindowPtr; er: EventRecord);
		var
			selected_item: integer;
			key: ProcPtr;
	begin
		selected_item := windowinfo[GetWindowType(window)].selected_item;
		if selected_item > 0 then begin
			key := whatinfo[TypeToWhat(GetWhatType(GetWindowType(window), selected_item))].key;
			if key <> nil then begin
				SetPort(window);
				DisplayError(acDoThis, CallWhatKey(GetWindowType(window), selected_item, er, key));
			end; (* if *)
		end;
	end;

	procedure WindowActivateDeactivate (window: WindowPtr; activate: boolean);
		var
			wt: WindowType;
			item: integer;
			i: integer;
			actproc: ProcPtr;
			what: WhatRecordPtr;
			err: OSErr;
	begin
		wt := GetWindowType(window);
		if wt <> WT_None then begin
			SetPort(window);
			item := GetSelectedItem(wt);
			if item > 0 then begin
				TextActivate(windowinfo[wt].items[item]^.data, activate);
			end; (* if *)
			for i := 1 to item_max do begin
				what := windowinfo[wt].items[i];
				if what <> nil then begin
					actproc := whatinfo[TypeToWhat(what^.typ)].activate;
					if actproc <> nil then begin
						err := CallWhatActivate(wt, i, activate, actproc);
						if (err <> noErr) and (err <> userCanceledErr) then begin
							SysBeep(10);
						end; (* if *)
					end;
				end; (* if *)
			end; (* for *)
		end; (* if *)
	end;

	procedure WindowItemWhere (window: WindowPtr; er: EventRecord; item: integer);
		var
			click: ProcPtr;
	begin
		case GetWindowType(window) of
			WT_About: 
				;
			otherwise begin
				click := whatinfo[TypeToWhat(GetWhatType(GetWindowType(window), item))].click;
				if click <> nil then begin
					SetPort(window);
					DisplayError(acDoThis, CallWhatClick(GetWindowType(window), item, er, click));
				end; (* if *)
			end;
		end;
	end;

	function WindowEarlyHandleEvent (window: WindowPtr; er: EventRecord): boolean;
	begin
		WindowEarlyHandleEvent := false;
	end;

	procedure WindowTab (window: WindowPtr; shift: boolean);
		var
			orgitem, rorgitem, i: integer;
			k: integer;
			dirn: integer;
			h: handle;
			r: rect;
			selitem: integer;
			t: OSType;
	begin
		selitem := -1;
		orgitem := GetSelectedItem(GetWindowType(window));
		rorgitem := orgitem;
		if orgitem <= 0 then begin
			if shift then begin
				orgitem := 1;
			end
			else begin
				orgitem := item_max;
			end;
		end;
		dirn := item_max - 2 * ord(shift);
		i := orgitem;
		repeat
			i := (i + dirn) mod item_max + 1;
			t := GetWhatType(GetWindowType(window), i);
			if t = 'TEXT' then begin
				selitem := i;
				leave;
			end;
		until (i = orgitem);
		if selitem > 0 then begin
			SelectTextItem(GetWindowType(window), selitem);
		end;
	end;

	function WindowEarlyHandleKey (window: WindowPtr; er: EventRecord): boolean;
		var
			b: boolean;
			ch: integer;
	begin
		b := false;
		ch := BAND(er.message, $FF);
		if ch = 9 then begin
			WindowTab(window, BAND(er.modifiers, shiftKey) <> 0);
			b := true;
		end;
		if not b then begin
		end;
		WindowEarlyHandleKey := b;
	end;

	function FlushWindowType (wp: WindowPtr; wt: WindowType): OSErr;
		var
			i: integer;
			first_err, err: OSErr;
			flush: ProcPtr;
			what: WhatRecordPtr;
			portrect: Rect;
	begin
		first_err := noErr;
		SetPort(wp);
		GetWindowRect(wp, portrect);
		windowinfo[wt].position := portrect.topLeft;
		for i := 1 to item_max do begin
			what := windowinfo[wt].items[i];
			if what <> nil then begin
				flush := whatinfo[TypeToWhat(what^.typ)].flush;
				if flush <> nil then begin
					err := CallWhatFlush(wt, i, flush);
					if first_err = noErr then begin
						first_err := err;
					end; (* if *)
				end;
			end; (* if *)
		end;
		FlushWindowType := first_err;
	end; (* FlushWindowType *)

	function DisposeWindowType (wp: WindowPtr; wt: WindowType): OSErr;
		var
			i: integer;
			first_err, err: OSErr;
			close: ProcPtr;
			what: WhatRecordPtr;
	begin
		first_err := noErr;
		SetPort(wp);
		for i := 1 to item_max do begin
			what := windowinfo[wt].items[i];
			if what <> nil then begin
				close := whatinfo[TypeToWhat(what^.typ)].close;
				if close <> nil then begin
					err := CallWhatClose(wt, i, close);
					if first_err = noErr then begin
						first_err := err;
					end; (* if *)
				end;
				windowinfo[wt].items[i] := nil;
				DisposePtr(Ptr(what));
			end; (* if *)
		end;
		windowinfo[wt].window := nil;
		DisposeDialog(wp);
		DisposeWindowType := first_err;
	end; (* DisposeWindowType *)

	function CloseWindowType (wp: WindowPtr; wt: WindowType): OSErr;
		var
			err: OSErr;
			err2: OSErr;
			opened: boolean;
	begin
		opened := false;
		err := ICMapErr(ICBegin(GetInstance, icReadWritePerm));
		if err = noErr then begin
			opened := true;
			err := FlushWindowType(wp, wt);
		end; (* if *)
		err2 := DisposeWindowType(wp, wt);
		if err = noErr then begin
			err := err2;
		end; (* if *)
		if opened then begin
			err2 := ICMapErr(ICEnd(GetInstance));
			if err = noErr then begin
				err := err2;
			end; (* if *)
		end; (* if *)
		CloseWindowType := err;
	end; (* CloseWindowType *)

	function WindowsEarlyHandleEvent (er: EventRecord): boolean;
	begin
		WindowsEarlyHandleEvent := false;
		if GetWindowType(FrontWindow) <> WT_None then begin
			WindowsEarlyHandleEvent := WindowEarlyHandleEvent(FrontWindow, er);
		end;
	end;

	function WindowsEarlyHandleKey (er: EventRecord): boolean;
	begin
		WindowsEarlyHandleKey := false;
		if GetWindowType(FrontWindow) <> WT_None then begin
			WindowsEarlyHandleKey := WindowEarlyHandleKey(FrontWindow, er);
		end;
	end;

	procedure WindowsDoKey (er: EventRecord);
	begin
		if GetWindowType(FrontWindow) <> WT_None then begin
			WindowDoKey(FrontWindow, er);
		end;
	end;

	procedure WindowsIdle;
		var
			wt: WindowType;
	begin
		for wt := WT_None to WT_Last do begin
			if windowinfo[wt].window <> nil then begin
				DoWindowIdle(windowinfo[wt].window);
			end;
		end;
	end;

	procedure WindowsSetTitle (wt: WindowType; title: Str255);
	begin
		if windowinfo[wt].window <> nil then begin
			SetWTitle(windowinfo[wt].window, title);
		end; (* if *)
	end; (* WindowsSetTitle *)

	procedure WindowsAdjustMenus;
		var
			wt: WindowType;
	begin
		wt := GetWindowType(FrontWindow);
		AdjustTextMenu(wt);
	end;

	procedure WindowsDoEditMenu (item: integer);
		var
			wt: WindowType;
	begin
		wt := GetWindowType(FrontWindow);
		if wt <> WT_None then begin
			DoTextMenu(wt, item);
		end; (* if *)
	end;

	type
		WhatTemplateRecord = record
				key: str31;
				typ: OSType;
				flags: longInt;
			end;
		WhatTemplateArray = array[1..item_max] of WhatTemplateRecord;
		WhatTemplateArrayPtr = ^WhatTemplateArray;
		WhatTemplateArrayHandle = ^WhatTemplateArrayPtr;

	function ParseWhat (wt: WindowType): OSErr;
		var
			what: WhatTemplateArrayHandle;
			i: integer;
			err: OSErr;
	begin
		err := noErr;
		for i := 1 to item_max do begin
			windowinfo[wt].items[i] := nil;
		end;
		what := WhatTemplateArrayHandle(GetResource('WHAT', windowinfo[wt].id));
		if what <> nil then begin
			HLock(handle(what));
			for i := 1 to GetHandleSize(handle(what)) div SizeOf(WhatTemplateRecord) do begin
				if what^^[i].typ <> 'NULL' then begin
					windowinfo[wt].items[i] := WhatRecordPtr(NewPtr(SizeOf(WhatRecord)));
					err := MemError;
					if err <> noErr then begin
						leave;
					end; (* if *)
					with windowinfo[wt].items[i]^ do begin
						key := what^^[i].key;
						typ := what^^[i].typ;
						flags := what^^[i].flags;
					end; (* with *)
				end; (* if *)
			end; (* for *)
			HUnlock(handle(what));
		end; (* if *)
		ParseWhat := err;
	end; (* ParseWhat *)

	function GetWindowID (wt: WindowType): integer;
	begin
		if wt = WT_About then begin
			GetWindowID := 128;
		end
		else begin
			GetWindowID := 200 + ord(wt) - ord(WT_Main);
		end;
	end;

	function EditCurrentPreference (var key: Str255): OSErr;
		var
			wt: WindowType;
			what: WhatTemplateArrayHandle;
			err: OSErr;
			i: integer;
			id: integer;
	begin
		err := -1;
		for wt := WT_None to WT_Last do begin
			if (WT_Personal <= wt) & (wt <= WT_Last) then begin
				id := GetWindowID(wt);
				what := WhatTemplateArrayHandle(GetResource('WHAT', id));
				if what <> nil then begin
					HLock(handle(what));
					for i := 1 to GetHandleSize(handle(what)) div SizeOf(WhatTemplateRecord) do begin
						if IUEqualString(key, what^^[i].key) = 0 then begin
							err := WindowsOpen(wt);
							if (what^^[i].typ = 'TEXT') then begin
								SelectTextItem(wt, i);
							end;
						end;
					end;
					HUnlock(handle(what));
				end;
			end;
		end;
		EditCurrentPreference := err;
	end;

	function PrepWindow (wt: WindowType; id: integer; wp: WindowPtr): OSErr;
		var
			what, i: integer;
			err: OSErr;
			first_err: OSErr;
	begin
		SetPort(wp);
		windowinfo[wt].window := wp;
		windowinfo[wt].id := id;
		windowinfo[wt].selected_item := -1;
		first_err := ParseWhat(wt);
		if first_err = noErr then begin
			for i := 1 to item_max do begin
				if windowinfo[wt].items[i] <> nil then begin
					with windowinfo[wt].items[i]^ do begin
						what := TypeToWhat(typ);
						if whatinfo[what].open <> nil then begin
							err := CallWhatOpen(wt, i, whatinfo[what].open);
							if first_err = noErr then begin
								first_err := err;
							end; (* if *)
						end;
					end; (* with *)
				end;
			end; (* for *)
		end; (* if *)
		if first_err = noErr then begin
			if windowinfo[wt].selected_item = -1 then begin
				WindowTab(windowinfo[wt].window, false);
			end; (* if *)
		end; (* if *)
		PrepWindow := first_err;
	end; (* PrepWindow *)

	function NewWindow (wt: WindowType): OSErr;
		var
			wp: WindowPtr;
			junk: OSErr;
			position: Point;
			original_position: Rect;
			err: OSErr;
			err2: OSErr;
			id, dlg_id: integer;
	begin
		id := GetWindowID(wt);
		wp := nil;
		err := ICMapErr(ICBegin(GetInstance, icReadWritePerm));
		if err = noErr then begin
			dlg_id := id;
			if (wt = WT_Font) & not system7 then begin
				dlg_id := 290;
			end;
			wp := GetNewDialog(dlg_id, nil, POINTER(-1));
			if wp = nil then begin
				err := memFullErr;
			end; (* if *)
			if err = noErr then begin
				err := PrepWindow(wt, id, wp);
			end; (* if *)
			if err = noErr then begin
				position := windowinfo[wt].position;
				if (position.h <> 0) or (position.v <> 0) then begin
					GetWindowRect(wp, original_position);
					MoveWindow(wp, position.h, position.v, false);
					ShowWindow(wp);			(* because TitleBarOnScreen requires window to be shown *)
					if not TitleBarOnScreen(wp) then begin
						MoveWindow(wp, original_position.left, original_position.top, false);
					end; (* if *)
				end; (* if *)
				ShowWindow(wp);
			end;
			err2 := ICMapErr(ICEnd(GetInstance));
			if err = noErr then begin
				err := err2;
			end; (* if *)
		end; (* if *)
		(* tidy up code *)
		if err <> noErr then begin
			if wp <> nil then begin
				junk := DisposeWindowType(wp, wt);
			end; (* if *)
		end; (* if *)
		NewWindow := err;
	end; (* NewWindow *)

	function WindowsOpen (wt: WindowType): OSErr;
	begin
		if windowinfo[wt].window <> nil then begin
			ShowWindow(windowinfo[wt].window);
			SelectWindow(windowinfo[wt].window);
			WindowsOpen := noErr;
		end
		else begin
			WindowsOpen := NewWindow(wt);
		end;
	end; (* WindowsOpen *)

	function WindowsClose (wp: WindowPtr): OSErr;
		var
			wt: WindowType;
	begin
		WindowsClose := noErr;
		wt := GetWindowType(wp);
		if wt <> WT_None then begin
			WindowsClose := CloseWindowType(wp, wt);
		end; (* if *)
	end; (* WindowsClose *)

	type
		pointArray = array[WT_Main..WT_Last] of Point;

	procedure WindowsResetPositions;
		var
			wt: WindowType;
			pos: Point;
	begin
		pos.h := 2;
		pos.v := 42;
		for wt := WT_Main to pred(WT_Last) do begin
			windowinfo[wt].position := pos;
			pos.h := pos.h + 20 * ord(screenbits.bounds.right > 512);
			pos.v := pos.v + 10 * ord(screenbits.bounds.bottom >= 400) + 8 * ord(screenbits.bounds.bottom >= 480);
		end; (* for *)
	end;

	procedure WindowsRestorePositions;
		var
			err, err2: OSErr;
			key: Str255;
			attr: longint;
			window_positions: pointArray;
			size: longint;
			wt: WindowType;
	begin
		err := ICMapErr(ICBegin(GetInstance, icReadOnlyPerm));
		if err = noErr then begin
			key := StringOf(Ptr(longint(ICcreator)), '¥WindowPositions');
			size := sizeof(window_positions);
			err := ICGetPref(GetInstance, key, attr, @window_positions, size);
			if (err = noErr) and (size <> sizeof(window_positions)) then begin
				err := -1;
			end; (* if *)
			err2 := ICMapErr(ICEnd(GetInstance));
			if err = noErr then begin
				err := err2;
			end; (* if *)
		end; (* if *)
		if err = noErr then begin
			for wt := WT_Main to WT_Last do begin
				windowinfo[wt].position := window_positions[wt];
			end; (* for *)
		end
		else begin
			WindowsResetPositions;
		end; (* if *)
	end; (* WindowsRestorePositions *)

	procedure WindowsSavePositions;
		var
			err, err2: OSErr;
			wt: WindowType;
			key: Str255;
			window_positions: pointArray;
	begin
		for wt := WT_Main to WT_Last do begin
			window_positions[wt] := windowinfo[wt].position;
		end; (* for *)
		err := ICMapErr(ICBegin(GetInstance, icReadWritePerm));
		if err = noErr then begin
			key := StringOf(Ptr(longint(ICcreator)), '¥WindowPositions');
			err := ICSetPref(GetInstance, key, ICattr_no_change, @window_positions, sizeof(window_positions));
			err2 := ICMapErr(ICEnd(GetInstance));
			if err = noErr then begin
				err := err2;
			end; (* if *)
		end; (* if *)
	end; (* WindowsSavePositions *)

	function WindowsFlushAll: OSErr;	(* flush all information windows *)
		var
			wt: WindowType;
			first_err, err: OSErr;
	begin
		first_err := ICMapErr(ICBegin(GetInstance, icReadWritePerm));
		if first_err = noErr then begin
			for wt := WT_Main to WT_Last do begin
				if windowinfo[wt].window <> nil then begin
					SetPort(windowinfo[wt].window);
					err := FlushWindowType(windowinfo[wt].window, wt);
					if first_err = noErr then begin
						first_err := err;
					end; (* if *)
				end; (* if *)
			end; (* for *)
			err := ICMapErr(ICEnd(GetInstance));
			if first_err = noErr then begin
				first_err := err;
			end; (* if *)
		end; (* if *)
		WindowsFlushAll := first_err;
	end; (* WindowsFlushAll *)

	function WindowsCloseAll: OSErr;	(* close all information windows *)
		var
			wt: WindowType;
			err, first_err: OSErr;
	begin
		first_err := noErr;
		for wt := WT_Personal to WT_Last do begin
			if windowinfo[wt].window <> nil then begin
				err := CloseWindowType(windowinfo[wt].window, wt);
				if first_err = noErr then begin
					first_err := err;
				end; (* if *)
			end;
		end; (* for *)
		WindowsCloseAll := first_err;
	end; (* WindowsCloseAll *)

	procedure InitWhats;
		procedure W (what: integer; xtyp: OSType; xopen, xkey, xclick, xidle, xflush, xclose, xactivate, xcursor: ProcPtr; xcursorid: integer);
		begin
			with whatinfo[what] do begin
				typ := xtyp;
				open := xopen;
				key := xkey;
				click := xclick;
				flush := xflush;
				close := xclose;
				activate := xactivate;
				idle := xidle;
				cursor := xcursor;
				cursorid := xcursorid;
			end;
		end;
	begin
		W(1, 'NULL', nil, nil, nil, nil, nil, nil, nil, nil, 0);
		W(2, 'TEXT', @WhatOpenText, @WhatKeyText, @WhatClickText, nil, @WhatFlushText, @WhatCloseText, nil, nil, iBeamCursor);
		W(3, 'SPOP', @WhatOpenPopup, nil, @WhatClickPopup, nil, @WhatFlushPopup, @WhatClosePopup, nil, nil, 0);
		W(4, 'FFSP', @WhatOpenFSSpec, nil, @WhatClickFSSpec, nil, @WhatFlushFSSpec, nil, nil, nil, 0);
		W(5, 'FPOP', @WhatOpenFont, nil, @WhatClickFont, nil, @WhatFlushFont, nil, nil, nil, 0);
		W(6, 'FMAP', @WhatOpenFileMap, @WhatKeyFileMap, @WhatClickFileMap, nil, @WhatFlushFileMap, @WhatCloseFileMap, @WhatActivateFileMap, @WhatCursorFileMap, plusCursor);
		W(7, 'FBUT', nil, nil, @WhatClickFileMap, nil, nil, nil, nil, nil, 0);
		W(8, 'HMAP', @WhatOpenHelper, @WhatKeyHelper, @WhatClickHelper, nil, @WhatFlushHelper, @WhatCloseHelper, @WhatActivateHelper, @WhatCursorHelper, plusCursor);
		W(9, 'HBUT', nil, nil, @WhatClickHelper, nil, nil, nil, nil, nil, 0);
		W(10, 'BUTN', @WhatOpenButton, nil, @WhatClickButton, nil, nil, nil, nil, nil, 0);
		W(11, 'FSIZ', nil, nil, @WhatClickFontSize, nil, nil, nil, nil, nil, 0);
	end;

	procedure AboutBoxUpdate (dlg: DialogPtr; item: integer);
		var
			r: Rect;
	begin
		GetDItemRect(dlg, item, r);
		case item of
			1: 
				DrawIcon(128, r, false);
			3: 
				DisplayStyledString(dlg, item, concat(GetAString(129, item), app_version.shortVersion));
			otherwise
				DisplayStyledString(dlg, item, GetAString(129, item));
		end; (* case *)
	end; (* AboutBoxUpdate *)

	function InitICWindows: OSErr;
		var
			wt: WindowType;
			i: integer;
			pos: Point;
			wp: WindowPtr;
			kind: integer;
			err: OSErr;
	begin
		InitICWindowGlobals;
		InitWhats;
		for wt := WT_None to WT_Last do begin
			windowinfo[wt].window := nil;
			windowinfo[wt].position.h := 0;
			windowinfo[wt].position.v := 0;
		end;
		WindowsResetPositions;
		(* bring the about box up hidden and leave it there *)
		err := noErr;
		wp := GetNewDialog(128, nil, WindowPtr(-1));
		windowinfo[WT_About].window := wp;
		if wp = nil then begin
			err := memFullErr;
		end; (* if *)
		if err = noErr then begin
			err := PrepWindow(WT_About, 128, wp);
		end; (* if *)
		if err = noErr then begin
			for i := 1 to CountDItems(wp) do begin
				GetDItemKind(wp, i, kind);
				if band(kind, $7f) = userItem then begin
					SetDItemHandle(wp, i, @AboutBoxUpdate);
				end; (* if *)
			end; (* for *)
		end; (* if *)
		InitICWindows := err;
	end; (* InitICWindows *)

end.
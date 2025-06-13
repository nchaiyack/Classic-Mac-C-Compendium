unit ICHelperWhat;

interface

	uses
		ICWindowGlobals;

	function WhatOpenHelper (wt: WindowType; item: integer): OSErr;
	function WhatActivateHelper (wt: WindowType; item: integer; activate: boolean): OSErr;
	function WhatFlushHelper (wt: WindowType; item: integer): OSErr;
	function WhatCloseHelper (wt: WindowType; item: integer): OSErr;
	function WhatClickHelper (wt: WindowType; item: integer; er: eventRecord): OSErr;
	function WhatKeyHelper (wt: WindowType; item: integer; er: EventRecord): OSErr;
	function WhatCursorHelper (wt: WindowType; item: integer; pt: Point; cursorid: integer): OSErr;

implementation

	uses
		IconFamilies, ICStrH, 

		ICKeys, ICAPI, ICSubs, ICDialogs, ICMiscSubs, ICDocUtils, ICGlobals, {}
		ICIConSuites, ICStandardFile, ICMovableModal;

	const
		ditChange = 1;
		ditMainUserItem = 2;
		ditAdd = 3;
		ditDelete = 4;
		ditList = 5;

	const
		ditOK = 1;
		ditCancel = 2;
		ditAddUserItem = 3;
		ditHelper = 4;
		ditAppName = 5;
		ditChooseApplication = 6;

	const
		kCellHeight = 36;

	type
		InternalAppSpec = record
				key: Str255;
				locked: boolean;
				appspec: ICAppSpec;
			end;

	procedure GetNthElement (entries: Handle; index: integer; var res: InternalAppSpec);
	begin
		BlockMove(Ptr(longint(entries^) + (index - 1) * sizeof(InternalAppSpec)), @res, sizeof(InternalAppSpec));
	end; (* GetNthElement *)

	procedure SetNthElement (entries: Handle; index: integer; var res: InternalAppSpec);
	begin
		BlockMove(@res, Ptr(longint(entries^) + (index - 1) * sizeof(InternalAppSpec)), sizeof(InternalAppSpec));
	end; (* SetNthElement *)

	function EntryLocked (entries: Handle; index: integer): boolean;
		var
			appspec: InternalAppSpec;
	begin
		GetNthElement(entries, index, appspec);
		EntryLocked := appspec.locked;
	end; (* EntryLocked *)

	procedure DimButtons (wt: WindowType);
		var
			have_selection: boolean;
			dlg: DialogPtr;
			selection: integer;
			entries: Handle;
			lh: ListHandle;
	begin
		dlg := windowinfo[wt].window;
		lh := ListHandle(windowinfo[wt].items[ditList]^.spare_data);
		entries := Handle(windowinfo[wt].items[ditList]^.data);
		selection := SelectedLine(lh);
		have_selection := (selection <> -1);
		SetDCtlEnable(dlg, ditAdd, not IsDocLocked);
		SetDCtlEnable(dlg, ditDelete, have_selection and not EntryLocked(entries, selection + 1));
		SetDCtlEnable(dlg, ditChange, have_selection and not EntryLocked(entries, selection + 1));
		OutlineDefault1(dlg, ditMainUserItem);
	end; (* DimButtons *)

	procedure UserItemUpdate (dlg: DialogPtr; item: integer);
		var
			list_rect: Rect;
	begin
		GetDItemRect(dlg, item, list_rect);
		PenNormal;
		InsetRect(list_rect, -1, -1);
		FrameRect(list_rect);
		LUpdate(dlg^.visRgn, ListHandle(windowinfo[GetWindowType(dlg)].items[item]^.spare_data));
	end; (* UserItemUpdate *)

	procedure MyLDEF (message: integer; select: boolean; var r: Rect; the_cell: Cell; offset: integer; datalen: integer; lh: ListHandle);
		var
			entries: Handle;

		procedure LDDraw;
			const
				HiliteMode = $938;
			var
				appspec: InternalAppSpec;
				pos: longint;
				err: OSErr;
				junkpos: longint;
				suite: Handle;
				cur_x: integer;
				junk: OSErr;
				tmprect: Rect;
				sys_font_info: FontInfo;
				top: integer;
				transfer: integer;
				rgn: RgnHandle;
				rgn2: RgnHandle;
		begin
			if datalen = 0 then begin
				GetNthElement(entries, the_cell.v + 1, appspec);
				EraseRect(r);
				cur_x := 0;
				rgn := NewRgn;
				if system7 then begin
					tmprect := r;
					tmprect.bottom := tmprect.top + 32;
					tmprect.right := tmprect.left + 32;
					OffsetRect(tmprect, (kCellHeight - 32) div 2, (kCellHeight - 32) div 2);
					junk := GetDTDBIconSuiteCached('APPL', appspec.appspec.fCreator, suite);
					if suite <> nil then begin
						if select then begin
							transfer := ttSelected;
						end
						else begin
							transfer := ttNone;
						end; (* if *)
						err := PlotIconSuite(tmprect, atNone, transfer, suite);
						junk := IconSuiteToRgn(rgn, tmprect, atNone, suite);
					end; (* if *)
					cur_x := cur_x + kCellHeight;
				end;
				cur_x := cur_x + 2;

				TextFont(systemFont);
				GetFontInfo(sys_font_info);

				top := (kCellHeight - (sys_font_info.ascent + sys_font_info.descent)) div 2;
				TextFont(systemFont);
				MoveTo(r.left + cur_x, r.top + top + sys_font_info.ascent);
				DrawString(concat(copy(appspec.key, length(kICHelper) + 1, 255), GetAString(128, 5), appspec.appspec.name));

				if select then begin
					rgn2 := NewRgn;
					RectRgn(rgn2, r);
					if has_colorQD then begin
						BitClr(Ptr(HiliteMode), pHiliteBit);
					end; (* if *)
					XorRgn(rgn, rgn2, rgn2);
					InvertRgn(rgn2);
					DisposeRgn(rgn2);
				end; (* if *)
				if rgn <> nil then begin
					DisposeRgn(rgn);
				end; (* if *)
			end; (* if *)
		end;

	begin
		SetPort(lh^^.port);
		entries := Handle(windowinfo[GetWindowType(lh^^.port)].items[ditList]^.data);
		case message of
			lInitMsg: 
				;
			lDrawMsg: 
				LDDraw;
			lHiliteMsg: 
				LDDraw;
			lCloseMsg: 
				;
			otherwise
				;
		end;
	end; (* MyLDEF *)

	function WhatOpenHelper (wt: WindowType; item: integer): OSErr;
		var
			dlg: DialogPtr;
			entries: Handle;
			attr: longint;
			err: OSErr;
			count: longint;
			list_rect: Rect;
			data_rect: Rect;
			cell_size: Point;
			lh: ListHandle;
			tmpapspec: InternalAppSpec;
			ndx: longint;
			size: longint;
	begin
		err := noErr;
		windowinfo[wt].items[item]^.spare_data := nil;
		windowinfo[wt].items[item]^.data := nil;
		dlg := windowinfo[wt].window;
		SetDItemHandle(dlg, item, @UserItemUpdate);
		entries := NewHandle(0);
		if entries = nil then begin
			err := memFullErr;
		end; (* if *)
		if err = noErr then begin
			ndx := 1;
			repeat
				err := ICMapErr(ICGetIndPref(GetInstance, ndx, tmpapspec.key));
				if err = noErr then begin
					if IUEqualString(copy(tmpapspec.key, 1, length(kICHelper)), kICHelper) = 0 then begin
						size := sizeof(tmpapspec.appspec);
						err := ICMapErr(ICGetPref(GetInstance, tmpapspec.key, attr, @tmpapspec.appspec, size));
						if err = noErr then begin
							tmpapspec.locked := btst(attr, ICattr_locked_bit) | IsDocLocked;
							err := PtrAndHand(@tmpapspec, entries, sizeof(InternalAppSpec));
						end; (* if *)
					end; (* if *)
				end; (* if *)
				ndx := ndx + 1;
			until err <> noErr;
		end; (* if *)
		if err = icPrefNotFoundErr then begin
			err := noErr;
		end; (* if *)
		if err = noErr then begin
			count := GetHandleSize(entries) div sizeof(InternalAppSpec);
			windowinfo[wt].items[item]^.data := Ptr(entries);
			GetDItemRect(dlg, item, list_rect);
			list_rect.right := list_rect.right - 15;
			SetRect(data_rect, 0, 0, 1, count);
			SetPt(cell_size, list_rect.right - list_rect.left, kCellHeight);
			lh := LNew(list_rect, data_rect, cell_size, 128, dlg, true, false, false, true);
			if lh = nil then begin
				err := memFullErr;
			end; (* if *)
		end; (* if *)
		if err = noErr then begin
			lh^^.refCon := ord(@MyLDEF);
			lh^^.selFlags := lOnlyOne;
			windowinfo[wt].items[item]^.spare_data := Ptr(lh);
			DimButtons(wt);
			SetUpDefaultOutline(dlg, ditChange, ditMainUserItem);
			windowinfo[wt].selected_item := ditList;
		end; (* if *)
		WhatOpenHelper := err;
	end; (* WhatOpenHelper *)

	function ChooseApplication (var spec: ICAppSpec): boolean;
		var
			valid: boolean;
			info: FInfo;
			fs: FSSpec;
			err: OSErr;
	begin
		err := ICStandardGetFile('APPL', fs, info);
		DisplayError(acChooseApplication, err);
		if err = noErr then begin
			spec.fCreator := info.fdCreator;
			spec.name := fs.name;
		end; (* if *)
		ChooseApplication := err = noErr;
	end; (* ChooseApplication *)

	procedure FixButton (dlg: DialogPtr);
		var
			t1, t2: Str255;
	begin
		GetItemText(dlg, ditAppName, t1);
		GetItemText(dlg, ditHelper, t2);
		SetDCtlEnable(dlg, ditOK, (t1 <> '') and (t2 <> ''));
		OutlineDefault1(dlg, ditAddUserItem);
	end; (* FixButton *)

	function DoAddFilter (dlg: DialogPtr; var event: EventRecord; var item: integer): boolean;
		var
			res: boolean;
	begin
		res := CancelModalFilter(dlg, event, item);
		FixButton(dlg);
		DoAddFilter := res;
	end;(* DoAddFilter *)

	function DoEdit (entries: Handle; lh: ListHandle; var choosen_app: InternalAppSpec): OSErr;
		var
			dlg: DialogPtr;
			item: integer;
			junk: integer;
			err: OSErr;
			saved_state: Ptr;
	begin
		err := noErr;
		dlg := GetNewDialog(800, nil, WindowPtr(-1));
		if dlg = nil then begin
			err := memFullErr;
		end; (* if *)
		if err = noErr then begin
			SetUpDefaultOutline(dlg, ditOK, ditAddUserItem);
			SetItemText(dlg, ditHelper, choosen_app.key);
			SetItemText(dlg, ditAppName, choosen_app.appspec.name);
			SelIText(dlg, ditHelper, 0, 255);
			FixButton(dlg);
			ShowWindow(dlg);
			DisableMenuBar(saved_state, -1);
			InitCursor;
			repeat
				MovableModalDialog(@DoAddFilter, item);
				case item of
					ditChooseApplication:  begin
						if ChooseApplication(choosen_app.appspec) then begin
							SetItemText(dlg, ditAppName, choosen_app.appspec.name);
							FixButton(dlg);
						end; (* if *)
					end;
					otherwise
						;
				end; (* case *)
			until item in [ditOK, ditCancel];
			if item = ditOK then begin
				GetItemText(dlg, ditHelper, choosen_app.key);
				choosen_app.key := concat(kICHelper, choosen_app.key);
			end
			else begin
				err := userCanceledErr;
			end; (* if *)
			ReEnableMenuBar(saved_state);
			DisposeDialog(dlg);
		end; (* if *)
		DoEdit := err;
	end;

	function DoAdd (entries: Handle; lh: ListHandle): OSErr;
		var
			choosen_app: InternalAppSpec;
			dlg: DialogPtr;
			item: integer;
			junk: integer;
			err: OSErr;
	begin
		choosen_app.appspec.name := '';
		choosen_app.key := '';
		err := DoEdit(entries, lh, choosen_app);
		if err = noErr then begin
			err := PtrAndHand(@choosen_app, entries, sizeof(choosen_app));
			if err = noErr then begin
				junk := LAddRow(1, 32767, lh);
			end; (* if *)
		end;
		DoAdd := err;
	end; (* DoAdd *)

	function DoChange (selection: integer; entries: Handle; lh: ListHandle): OSErr;
		var
			err: OSErr;
			tmpspec: InternalAppSpec;
	begin
		GetNthElement(entries, selection + 1, tmpspec);
		Delete(tmpspec.key, 1, length(kICHelper));
		err := DoEdit(entries, lh, tmpspec);
		if err = noErr then begin
			SetNthElement(entries, selection + 1, tmpspec);
			err := noErr;
		end
		else begin
			err := userCanceledErr;
		end; (* if *)
		DoChange := err;
	end; (* DoChange *)

	function DoDelete (selection: integer; entries: Handle; lh: ListHandle): OSErr;
		var
			err: OSErr;
			apspec: InternalAppSpec;
			pos: longint;
			junk: OSErr;
	begin
		err := ICMapErr(ICBegin(GetInstance, icReadWritePerm));
		if err = noErr then begin
			GetNthElement(entries, selection + 1, apspec);
			err := ICDeletePref(GetInstance, apspec.key);
			junk := ICMapErr(ICEnd(GetInstance));
		end; (* if *)
		if err = noErr then begin
			pos := Munger(entries, selection * sizeof(InternalAppSpec), nil, sizeof(InternalAppSpec), Ptr(-1), 0);
			err := MemError;
			if err = noErr then begin
				LDelRow(1, selection, lh);
			end; (* if *)
		end; (* if *)
		DoDelete := err;
	end; (* DoDelete *)

	function WhatClickHelper (wt: WindowType; item: integer; er: eventRecord): OSErr;
		var
			lh: ListHandle;
			entries: Handle;
			selection: integer;
			err: OSErr;
			pos: longint;
			r: Rect;
	begin
		entries := Handle(windowinfo[wt].items[ditList]^.data);
		lh := ListHandle(windowinfo[wt].items[ditList]^.spare_data);
		GlobalToLocal(er.where);
		selection := SelectedLine(lh);
		err := 1;
		case item of
			ditAdd: 
				err := DoAdd(entries, lh);
			ditChange: 
				err := DoChange(selection, entries, lh);
			ditDelete: 
				if selection <> -1 then begin
					err := DoDelete(selection, entries, lh);
				end; (* if *)
			ditList:  begin
				if LClick(er.where, 0, lh) then begin
					selection := SelectedLine(lh);			(* selection may have been changed by LClick *)
					if EntryLocked(entries, selection + 1) then begin
						LockedAlert(wt, item);
						err := userCanceledErr;
					end
					else begin
						FlashItem(windowinfo[wt].window, ditChange);
						err := DoChange(selection, entries, lh);
					end; (* if *)
				end; (* if *)
			end;
		end; (* case *)
		if err = noErr then begin
			DirtyDocument;
		end; (* if *)
		if err = 1 then begin
			err := noErr;
		end; (* if *)
		DimButtons(wt);
		WhatClickHelper := err;
	end; (* WhatClickHelper *)

	function WhatFlushHelper (wt: WindowType; item: integer): OSErr;
		var
			err: OSerr;
			entries: Handle;
			i: integer;
			appspec: InternalAppSpec;
			junk: OSErr;
			first_err: OSErr;
	begin
		err := noErr;
		entries := Handle(windowinfo[wt].items[ditList]^.data);
		first_err := noErr;
		for i := 1 to GetHandleSize(entries) div sizeof(InternalAppSpec) do begin
			GetNthElement(entries, i, appspec);
			err := ICMapErr(ICSetPref(GetInstance, appspec.key, ICattr_no_change, @appspec.appspec, sizeof(appspec.appspec)));
			if first_err = noErr then begin
				first_err := err;
			end; (* if *)
		end; (* for *)
		err := first_err;
		WhatFlushHelper := err;
	end; (* WhatFlushHelper *)

	function WhatCloseHelper (wt: WindowType; item: integer): OSErr;
	begin
		LDispose(ListHandle(windowinfo[wt].items[item]^.spare_data));
		DisposeHandle(Handle(windowinfo[wt].items[ditList]^.data));
		WhatCloseHelper := noErr;
	end; (* WhatCloseHelper *)

	function WhatActivateHelper (wt: WindowType; item: integer; activate: boolean): OSErr;
	begin
		LActivate(activate, ListHandle(windowinfo[wt].items[item]^.spare_data));
		WhatActivateHelper := noErr;
	end; (* WhatActivateHelper *)

	function GetEntryName (list: ListHandle; c: cell): str255;
		var
			appspec: InternalAppSpec;
			entries: handle;
	begin
		GetEntryName := '';
		entries := Handle(windowinfo[GetWindowType(list^^.port)].items[ditList]^.data);
		GetNthElement(entries, c.v + 1, appspec);
		GetEntryName := concat(copy(appspec.key, length(kICHelper) + 1, 255), GetAString(128, 5), appspec.appspec.name);
	end;

	function WhatKeyHelper (wt: WindowType; item: integer; er: EventRecord): OSErr;
		var
			ch: integer;
			lh: ListHandle;
			err: OSErr;
			entries: Handle;
			selection: integer;
	begin
		err := noErr;
		if (er.what = keyDown) or (er.what = autoKey) then begin
			ch := BAND(er.message, $FF);
			case ch of
				crChar, enterChar:  begin
					if GetDCtlEnable(windowinfo[wt].window, ditChange) then begin
						entries := Handle(windowinfo[wt].items[ditList]^.data);
						lh := ListHandle(windowinfo[wt].items[item]^.spare_data);
						selection := SelectedLine(lh);
						FlashItem(windowinfo[wt].window, ditChange);
						err := DoChange(selection, entries, lh);
						if err = noErr then begin
							DirtyDocument;
						end;
					end;
				end;
				otherwise begin
					DoListKey(ListHandle(windowinfo[wt].items[item]^.spare_data), er.modifiers, chr(BAND(er.message, $FF)), @GetEntryName);
					DimButtons(wt);
				end;
			end;
		end;
		WhatKeyHelper := err;
	end; (* WhatKeyHelper *)

	function WhatCursorHelper (wt: WindowType; item: integer; pt: Point; cursorid: integer): OSErr;
		var
			r: Rect;
	begin
		GetDItemRect(windowinfo[wt].window, item, r);
		InsetRect(r, 15, 0);
		if PtInRect(pt, r) then begin
			SetCursor(GetCursor(cursorid)^^);
		end
		else begin
			InitCursor;
		end; (* if *)
	end; (* WhatCursorHelper *)

end. (* ICHelperWhat *)
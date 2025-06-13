unit ICFileMapWhat;

interface

	uses
		ICWindowGlobals;

	function WhatOpenFileMap (wt: WindowType; item: integer): OSErr;
	function WhatActivateFileMap (wt: WindowType; item: integer; activate: boolean): OSErr;
	function WhatFlushFileMap (wt: WindowType; item: integer): OSErr;
	function WhatCloseFileMap (wt: WindowType; item: integer): OSErr;
	function WhatClickFileMap (wt: WindowType; item: integer; er: eventRecord): OSErr;
	function WhatKeyFileMap (wt: WindowType; item: integer; er: EventRecord): OSErr;
	function WhatCursorFileMap (wt: WindowType; item: integer; pt: Point; cursorid: integer): OSErr;

implementation

	uses
		IconFamilies, ICStrH, 

		ICKeys, ICAPI, ICSubs, ICDialogs, ICMiscSubs, ICSubs, ICDocUtils, ICGlobals, {}
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
		ditEditorUserItem = 3;
		ditEntryName = 4;
		ditExtension = 5;
		ditMIMEType = 6;
		ditAsciiRadio = 7;
		ditBinaryRadio = 8;
		ditMacintoshRadio = 9;
		ditType = 10;
		ditCreator = 11;
		ditChooseExample = 12;
		ditApplicationName = 13;
		ditPost = 14;
		ditPostCreator = 15;
		ditPostButton = 16;
		ditNotForIncoming = 17;
		ditNotForOutgoing = 18;

	const
(* ditOK = 1; *)
(* ditCancel = 2; *)
		ditManualUserItem = 3;
		ditManualType = 4;
		ditManualCreator = 5;
		ditManualApplicationName = 6;

	const
		kCellHeight = 36;

	procedure DimButtons (wt: WindowType);
		var
			have_selection: boolean;
			dlg: DialogPtr;
			lh: ListHandle;
	begin
		lh := ListHandle(windowinfo[wt].items[ditList]^.spare_data);
		dlg := windowinfo[wt].window;
		have_selection := (SelectedLine(lh) <> -1);
		SetDCtlEnable(dlg, ditAdd, not IsLocked(wt, ditList));
		SetDCtlEnable(dlg, ditDelete, have_selection and not IsLocked(wt, ditList));
		SetDCtlEnable(dlg, ditChange, have_selection and not IsLocked(wt, ditList));
		OutlineDefault1(dlg, ditMainUserItem);
	end; (* DimButtons *)

	procedure MyLDEF (message: integer; select: boolean; var r: Rect; the_cell: Cell; offset: integer; datalen: integer; lh: ListHandle);
		var
			entries: Handle;

		procedure LDClose;
		begin
			(* do nothing *)
		end;

		procedure LDDraw;
			const
				HiliteMode = $938;
			var
				entry: ICMapEntry;
				pos: longint;
				err: OSErr;
				junkpos: longint;
				suite: Handle;
				cur_x: integer;
				junk: OSErr;
				tmprect: Rect;
				sys_font_info, appl_font_info: FontInfo;
				top: integer;
				transfer: integer;
				rgn: RgnHandle;
				rgn2: RgnHandle;
		begin
			if datalen = 0 then begin
				err := ICMapErr(ICGetIndMapEntry(GetInstance, entries, the_cell.v + 1, junkpos, entry));
				if err = noErr then begin
					EraseRect(r);
					cur_x := 0;
					rgn := NewRgn;
					if system7 then begin
						tmprect := r;
						tmprect.bottom := tmprect.top + 32;
						tmprect.right := tmprect.left + 32;
						OffsetRect(tmprect, (kCellHeight - 32) div 2, (kCellHeight - 32) div 2);
						junk := GetDTDBIconSuiteCached(entry.file_type, entry.file_creator, suite);
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

					TextFont(applFont);
					GetFontInfo(appl_font_info);

					top := (kCellHeight - (sys_font_info.ascent + sys_font_info.descent + 0 + appl_font_info.ascent + appl_font_info.descent)) div 2;
					TextFont(systemFont);
					MoveTo(r.left + cur_x, r.top + top + sys_font_info.ascent);
					DrawString(concat(entry.creator_app_name, GetAString(128, 5), entry.entry_name));

					TextFont(applFont);
					MoveTo(r.left + cur_x, r.top + top + sys_font_info.ascent + sys_font_info.descent + 0 + appl_font_info.ascent);
					DrawString(entry.extension);

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
				LDClose;
		end;
	end; (* MyLDEF *)

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

	function WhatOpenFileMap (wt: WindowType; item: integer): OSErr;
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
	begin
		windowinfo[wt].items[item]^.spare_data := nil;
		windowinfo[wt].items[item]^.data := nil;
		err := noErr;
		dlg := windowinfo[wt].window;
		SetDItemHandle(dlg, item, @UserItemUpdate);
		if ICMapErr(ICGetPrefHandle(GetInstance, windowinfo[wt].items[item]^.key, attr, entries)) <> noErr then begin
			entries := NewHandle(0);
			if entries = nil then begin
				err := memFullErr;
			end; (* if *)
		end; (* if *)
		ProcessAttributes(wt, item, attr);
		if err = noErr then begin
			if ICCountMapEntries(GetInstance, entries, count) <> noErr then begin
				count := 0;
			end; (* if *)
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
		WhatOpenFileMap := err;
	end; (* WhatOpenFileMap *)

	procedure SetRadio (modal: DialogPtr; value: integer);
	begin
		SetDCtlBoolean(modal, ditAsciiRadio, value = ditAsciiRadio);
		SetDCtlBoolean(modal, ditBinaryRadio, value = ditBinaryRadio);
		SetDCtlBoolean(modal, ditMacintoshRadio, value = ditMacintoshRadio);
	end; (* SetRadio *)

	function AddChangeFilter (dlg: DialogPtr; var event: EventRecord; var item: integer): boolean;
		var
			res: boolean;
			ch: char;
	begin
		res := CancelModalFilter(dlg, event, item);
		if not res then begin
			if (event.what = keyDown) and (band(event.modifiers, cmdKey) <> 0) then begin
				ch := char(band(event.message, charCodeMask));
				if ch in ['1'..'3'] then begin
					SetRadio(dlg, ord(ch) - ord('1') + ditAsciiRadio);
				end; (* if *)
			end; (* if *)
		end; (* if *)
		if InForeground then begin
			if band(event.modifiers, optionKey) = 0 then begin
				SetDCtlTitle(dlg, ditChooseExample, GetAString(128, 6));
			end
			else begin
				SetDCtlTitle(dlg, ditChooseExample, GetAString(128, 7));
			end; (* if *)
		end; (* if *)
		AddChangeFilter := res;
	end; (* AddChangeFilter *)

	function DoDialog (var entry: ICMapEntry): OSErr;

		function MapOSType (ot: OSType): Str15;
		begin
			if ot = OSType(0) then begin
				MapOSType := '';
			end
			else begin
				MapOSType := ot;
			end; (* if *)
		end; (* MapOSType *)

		function MapStr (s: Str255): OSType;
		begin
			s := copy(concat(s, '    '), 1, 4);
			MapStr := s;
		end; (* MapStr *)

		var
			modal: DialogPtr;

		procedure DoChooseExample;
			var
				info: FInfo;
				err: OSErr;
				fs, app_fss: FSSpec;
		begin
			err := ICStandardGetFile(OSType(0), fs, info);
			DisplayError(acGetExample, err);
			if err = noErr then begin
				SetItemText(modal, ditType, MapOSType(info.fdType));
				SetItemText(modal, ditCreator, MapOSType(info.fdCreator));
				err := MyGetAPPL(info.fdCreator, app_fss);
				if err = noErr then begin
					SetItemText(modal, ditApplicationName, app_fss.name);
				end
				else begin
					SetItemText(modal, ditApplicationName, '');
				end; (* if *)
			end; (* if *)
		end; (* DoChooseExample *)

		function ChoosePost: boolean;
			var
				info: FInfo;
				err: OSErr;
				fs, app_fss: FSSpec;
		begin
			err := ICStandardGetFile('APPL', fs, info);
			DisplayError(acGetExample, err);
			if err = noErr then begin
				entry.post_creator := info.fdCreator;
				entry.post_app_name := fs.name;
			end; (* if *)
			ChoosePost := err = noErr;
		end;

		procedure DoEditTypeCreator;
			var
				edit_dlg: DialogPtr;
				item: integer;
				tmpstr: Str255;
				saved_state: Ptr;
		begin
			edit_dlg := GetNewDialog(701, nil, WindowPtr(-1));
			if edit_dlg <> nil then begin
				GetItemText(modal, ditType, tmpstr);
				SetItemText(edit_dlg, ditManualType, tmpstr);
				GetItemText(modal, ditCreator, tmpstr);
				SetItemText(edit_dlg, ditManualCreator, tmpstr);
				GetItemText(modal, ditApplicationName, tmpstr);
				SetItemText(edit_dlg, ditManualApplicationName, tmpstr);
				SetUpDefaultOutline(edit_dlg, ditOK, ditManualUserItem);

				SelIText(edit_dlg, ditManualType, 0, 32767);
				ShowWindow(edit_dlg);
				DisableMenuBar(saved_state, -1);
				InitCursor;
				repeat
					MovableModalDialog(@CancelModalFilter, item);
				until item in [ditOK, ditCancel];
				GetItemText(edit_dlg, ditManualType, tmpstr);
				SetItemText(modal, ditType, tmpstr);
				GetItemText(edit_dlg, ditManualCreator, tmpstr);
				SetItemText(modal, ditCreator, tmpstr);
				GetItemText(edit_dlg, ditManualApplicationName, tmpstr);
				SetItemText(modal, ditApplicationName, tmpstr);
				ReEnableMenuBar(saved_state);
				DisposeDialog(edit_dlg);
			end; (* if *)
		end; (* DoEditTypeCreator *)

		procedure SetPostInfo;
		begin
			if entry.post_creator <> OSType(0) then begin
				SetDCtlTitle(modal, ditPostButton, concat(GetIndStr(128, 24), entry.post_app_name, GetIndStr(128, 25)));
				SetItemText(modal, ditPostCreator, MapOSType(entry.post_creator));
			end
			else begin
				SetDCtlTitle(modal, ditPostButton, GetIndStr(128, 23));
				SetItemText(modal, ditPostCreator, '');
			end;
			SetDCtlBoolean(modal, ditPost, btst(entry.flags, ICmap_post_bit));
		end;

		var
			err: OSErr;
			item: integer;
			flags: longint;
			tmpstr: Str255;
			dummy: boolean;
			saved_state: Ptr;
	begin
		err := noErr;
		modal := GetNewDialog(700, nil, WindowPtr(-1));
		if modal = nil then begin
			err := memFullErr;
		end; (* if *)
		if err = noErr then begin
			SetUpDefaultOutline(modal, ditOK, ditEditorUserItem);
			SetItemText(modal, ditEntryName, entry.entry_name);
			SetItemText(modal, ditExtension, entry.extension);
			SetItemText(modal, ditMIMEType, entry.MIME_type);
			SetDCtlBoolean(modal, ditAsciiRadio, not btst(entry.flags, ICmap_binary_bit));
			SetDCtlBoolean(modal, ditBinaryRadio, btst(entry.flags, ICmap_binary_bit) and not (btst(entry.flags, ICmap_resource_fork_bit)));
			SetDCtlBoolean(modal, ditMacintoshRadio, btst(entry.flags, ICmap_binary_bit) and (btst(entry.flags, ICmap_resource_fork_bit)));
			SetDCtlBoolean(modal, ditNotForIncoming, btst(entry.flags, ICmap_not_incoming_bit));
			SetDCtlBoolean(modal, ditNotForOutgoing, btst(entry.flags, ICmap_not_outgoing_bit));
			SetItemText(modal, ditType, MapOSType(entry.file_type));
			SetItemText(modal, ditCreator, MapOSType(entry.file_creator));
			SetItemText(modal, ditApplicationName, entry.creator_app_name);
			SetPostInfo;
			SelIText(modal, ditEntryName, 0, 32767);
			ShowWindow(modal);
			DisableMenuBar(saved_state, -1);
			InitCursor;
			repeat
				MovableModalDialog(@AddChangeFilter, item);
				case item of
					ditAsciiRadio..ditMacintoshRadio:  begin
						SetRadio(modal, item);
					end;
					ditNotForIncoming..ditNotForOutgoing:  begin
						ToggleDCtlBoolean(modal, item);
					end;
					ditChooseExample:  begin
						if IsKeyDown(kOptionKeyCode) then begin
							DoEditTypeCreator;
						end
						else begin
							DoChooseExample;
						end; (* if *)
					end;
					ditPost:  begin
						if not btst(entry.flags, ICmap_post_bit) then begin
							if (entry.post_creator = OSType(0)) then begin
								if entry.file_creator <> OSType(0) then begin
									entry.post_creator := entry.file_creator;
									entry.post_app_name := entry.creator_app_name;
								end
								else begin
									dummy := ChoosePost;
								end;
							end;
							if entry.post_creator <> OSType(0) then begin
								bset(entry.flags, ICmap_post_bit);
							end;
						end
						else begin
							bclr(entry.flags, ICmap_post_bit);
						end;
						SetPostInfo;
					end;
					ditPostButton:  begin
						if ChoosePost then begin
							bset(entry.flags, ICmap_post_bit);
						end;
						SetPostInfo;
					end;
					otherwise
						;
				end; (* case *)
			until item in [ditOK, ditCancel];
			if item = ditOK then begin
				GetItemText(modal, ditEntryName, entry.entry_name);
				GetItemText(modal, ditExtension, entry.extension);
				GetItemText(modal, ditMIMEType, entry.MIME_type);

				flags := band(entry.flags, BNOT(ICmap_binary_mask + ICmap_data_fork_mask + ICmap_resource_fork_mask + ICmap_not_incoming_mask + ICmap_not_outgoing_mask));
				flags := flags + ICmap_data_fork_mask;
				if not GetDCtlBoolean(modal, ditAsciiRadio) then begin
					flags := flags + ICmap_binary_mask;
				end; (* if *)
				if GetDCtlBoolean(modal, ditMacintoshRadio) then begin
					flags := flags + ICmap_resource_fork_mask;
				end; (* if *)
				if GetDCtlBoolean(modal, ditNotForIncoming) then begin
					flags := flags + ICmap_not_incoming_mask;
				end; (* if *)
				if GetDCtlBoolean(modal, ditNotForOutgoing) then begin
					flags := flags + ICmap_not_outgoing_mask;
				end; (* if *)
				entry.flags := flags;

				GetItemText(modal, ditType, tmpstr);
				entry.file_type := MapStr(tmpstr);
				GetItemText(modal, ditCreator, tmpstr);
				entry.file_creator := MapStr(tmpstr);
				GetItemText(modal, ditApplicationName, entry.creator_app_name);
			end
			else begin
				err := userCanceledErr;
			end; (* if *)
		end; (* if *)
		if modal <> nil then begin
			ReEnableMenuBar(saved_state);
			DisposeDialog(modal);
		end; (* if *)
		DoDialog := err;
	end; (* DoDialog *)

	function DoAddChange (selection: integer; var entry: ICMapEntry; entries: Handle; lh: ListHandle): OSErr;
		var
			junkint: integer;
			err: OSErr;
			pos: longint;
			app_fss: FSSpec;
	begin
		if selection = -1 then begin
			with entry do begin
				version := 0;
				file_type := OSType(0);
				file_creator := OSType(0);
				post_creator := OSType(0);
				flags := 0;
				extension := '';
				creator_app_name := '';
				post_app_name := '';
				MIME_type := '';
				entry_name := '';
			end; (* with *)
			err := noErr;
		end
		else begin
			err := ICMapErr(ICGetIndMapEntry(GetInstance, entries, selection + 1, pos, entry));
		end; (* if *)

		if err = noErr then begin
			err := DoDialog(entry);
		end; (* if *)

		if err = noErr then begin
			if selection = -1 then begin
				err := ICMapErr(ICAddMapEntry(GetInstance, entries, entry));
				if err = noErr then begin
					junkint := LAddRow(1, 32767, lh);
				end; (* if *)
			end
			else begin
				err := ICMapErr(ICSetMapEntry(GetInstance, entries, pos, entry));
			end; (* if *)
		end; (* if *)
		DoAddChange := err;
	end; (* DoAddChange *)

	function WhatClickFileMap (wt: WindowType; item: integer; er: eventRecord): OSErr;
		var
			lh: ListHandle;
			entry: ICMapEntry;
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
				err := DoAddChange(-1, entry, entries, lh);
			ditChange: 
				err := DoAddChange(selection, entry, entries, lh);
			ditDelete: 
				if selection <> -1 then begin
					err := ICMapErr(ICGetIndMapEntry(GetInstance, entries, selection + 1, pos, entry));
					if err = noErr then begin
						err := ICMapErr(ICDeleteMapEntry(GetInstance, entries, pos));
					end; (* if *)
					if err = noErr then begin
						LDelRow(1, selection, lh);
					end; (* if *)
				end; (* if *)
			ditList:  begin
				if LClick(er.where, 0, lh) then begin
					if IsLocked(wt, item) then begin
						LockedAlert(wt, item);
						err := userCanceledErr;
					end
					else begin
						FlashItem(windowinfo[wt].window, ditChange);
						err := DoAddChange(SelectedLine(lh), entry, entries, lh);			(* selection may have been changed by LClick *)
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
		WhatClickFileMap := err;
	end; (* WhatClickFileMap *)

	function WhatFlushFileMap (wt: WindowType; item: integer): OSErr;
	begin
		WhatFlushFileMap := ICMapErr(ICSetPrefHandle(GetInstance, windowinfo[wt].items[item]^.key, ICattr_no_change, Handle(windowinfo[wt].items[item]^.data)));
	end; (* WhatFlushFileMap *)

	function WhatCloseFileMap (wt: WindowType; item: integer): OSErr;
	begin
		LDispose(ListHandle(windowinfo[wt].items[item]^.spare_data));
		DisposeHandle(Handle(windowinfo[wt].items[item]^.data));
		WhatCloseFileMap := noErr;
	end; (* WhatCloseFileMap *)

	function WhatActivateFileMap (wt: WindowType; item: integer; activate: boolean): OSErr;
	begin
		LActivate(activate, ListHandle(windowinfo[wt].items[item]^.spare_data));
		WhatActivateFileMap := noErr;
	end; (* WhatActivateFileMap *)

	function GetEntryName (list: ListHandle; c: cell): str255;
		var
			entry: ICMapEntry;
			err: OSErr;
			junkpos: longint;
			entries: handle;
	begin
		GetEntryName := '';
		entries := Handle(windowinfo[GetWindowType(list^^.port)].items[ditList]^.data);
		err := ICMapErr(ICGetIndMapEntry(GetInstance, entries, c.v + 1, junkpos, entry));
		if err = noErr then begin
			GetEntryName := concat(entry.creator_app_name, GetAString(128, 5), entry.entry_name);
		end;
	end;

	function WhatKeyFileMap (wt: WindowType; item: integer; er: EventRecord): OSErr;
		var
			ch: integer;
			changed: boolean;
			entry: ICMapEntry;
			lh: ListHandle;
			err: OSErr;
	begin
		lh := ListHandle(windowinfo[wt].items[ditList]^.spare_data);
		err := noErr;
		if (er.what = keyDown) or (er.what = autoKey) then begin
			ch := BAND(er.message, $FF);
			case ch of
				crChar, enterChar:  begin
					if GetDCtlEnable(windowinfo[wt].window, ditChange) then begin
						FlashItem(windowinfo[wt].window, ditChange);
						err := DoAddChange(SelectedLine(lh), entry, Handle(windowinfo[wt].items[ditList]^.data), lh);
						if err = noErr then begin
							DirtyDocument;
						end;
					end; (* if *)
				end;
				otherwise begin
					DoListKey(lh, er.modifiers, chr(BAND(er.message, $FF)), @GetEntryName);
					DimButtons(wt);
				end;
			end;
		end;
		WhatKeyFileMap := err;
	end; (* WhatKeyFileMap *)

	function WhatCursorFileMap (wt: WindowType; item: integer; pt: Point; cursorid: integer): OSErr;
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
	end; (* WhatCursorFileMap *)

end. (* ICFileMapWhat *)
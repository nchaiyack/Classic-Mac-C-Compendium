unit ICDialogs;

interface

	const
		i_ok = 1;
		i_cancel = 2;
		i_discard = 3;

	type
		SavedWindowInfo = record
				oldport: GrafPtr;
				thisport: GrafPtr;
				font: integer;
				size: integer;
				face: Style;
			end;

	procedure EnterWindow (window: WindowPtr; font, size: integer; face: Style; var saved: SavedWindowInfo);
	procedure ExitWindow (saved: SavedWindowInfo);
	procedure SetItemText (dlg: dialogPtr; item: integer; text: str255);
	procedure GetItemText (dlg: dialogPtr; item: integer; var text: str255);
	function GetItemTextF (dlg: dialogPtr; item: integer): str255;
	procedure OutlineDefault1 (dp: dialogPtr; item: integer);
	procedure SetUpDefaultOutline (dp: dialogPtr; def_item, user_item: integer);
	procedure FlashItem (dlg: dialogPtr; item: integer);
	procedure SetDItemRect (dp: dialogPtr; item: integer; rr: rect);
	procedure GetDItemRect (dp: dialogPtr; item: integer; var rr: rect);
	procedure SetDItemKind (dp: dialogPtr; item: integer; k: integer);
	procedure GetDItemKind (dp: dialogPtr; item: integer; var k: integer);
	function GetDControlHandle (dp: dialogPtr; item: integer): controlHandle;
	function GetDItemHandle (dp: dialogPtr; item: integer): handle;
	procedure SetDItemHandle (dp: dialogPtr; item: integer; h: univ handle);
	function GetDCtlEnable (dlg: dialogPtr; item: integer): boolean;
	procedure SetDCtlEnable (dp: dialogPtr; item: integer; on: boolean);
	function GetDCtlTitle (dp: dialogPtr; item: integer): str255;
	procedure SetDCtlTitle (dp: dialogPtr; item: integer; s: str255);
	function GetDCtlBoolean (dp: dialogPtr; item: integer): boolean;
	procedure SetDCtlBoolean (dp: dialogPtr; item: integer; value: boolean);
	procedure ToggleDCtlBoolean (dp: dialogPtr; item: integer);
	function GetDCtlValue (dp: dialogPtr; item: integer): integer;
	procedure SetDCtlValue (dp: dialogPtr; item: integer; value: integer);
	procedure DrawDItem (dp: dialogPtr; item: integer);
	function GetPopupMHandle (dlg: dialogPtr; item: integer): menuHandle;
	procedure SetPopUpMenuOnMouseDown (dlg: dialogPtr; item: integer; text: str255);
	procedure GetPopUpItemText (dlg: dialogPtr; item: integer; var text: str255);
	procedure GetDAFont (var font: integer);
	procedure SetWindowTitle (window: windowPtr; title: str255);
	function SelectedTextItem (dlg: DialogPtr): integer;
	procedure DrawTheFriggingGrowIcon (window: windowPtr; bounds: rect);
	procedure DisplayStyledString (dlg: dialogPtr; item: integer; s: str255);
{ s= "font:size:style:just:text" }
	procedure ShiftTab (dlg: DialogPtr);
	function CountDItems (dlg: DialogPtr): integer;
	function OKModalFilter (dlg: DialogPtr; var er: EventRecord; var item: integer): boolean;
	function CancelModalFilter (dlg: DialogPtr; var er: EventRecord; var item: integer): boolean;
	function CancelDiscardModalFilter (dlg: DialogPtr; var er: EventRecord; var item: integer): boolean;
	procedure DrawGrayRect (dlg: DialogPtr; item: integer; title: str255);

implementation

	procedure SetItemText (dlg: dialogPtr; item: integer; text: str255);
		var
			it: integer;
			ih: handle;
			box: rect;
			oldtext: str255;
	begin
		GetDItem(dlg, item, it, ih, box);
		GetIText(ih, oldtext);
		if oldtext <> text then
			SetIText(ih, text);
	end;

	procedure GetItemText (dlg: dialogPtr; item: integer; var text: str255);
		var
			it: integer;
			ih: handle;
			box: rect;
	begin
		GetDItem(dlg, item, it, ih, box);
		GetIText(ih, text);
	end;

	function GetItemTextF (dlg: dialogPtr; item: integer): str255;
		var
			text: str255;
	begin
		GetItemText(dlg, item, text);
		GetItemTextF := text;
	end;

	procedure OutlineDefault1 (dp: dialogPtr; item: integer);
		var
			kind: integer;
			h: handle;
			r: rect;
	begin
		SetPort(dp);
		GetDItem(dp, 1, kind, h, r);
		PenSize(3, 3);
		InsetRect(r, -4, -4);
		if controlHandle(h)^^.contrlHilite = 255 then
			PenPat(gray);
		FrameRoundRect(r, 16, 16);
		if controlHandle(h)^^.contrlHilite = 255 then
			PenPat(black);
		PenNormal;
	end;

	procedure SetUpDefaultOutline (dp: dialogPtr; def_item, user_item: integer);
		var
			kind: integer;
			h: handle;
			r: rect;
	begin
		if def_item <> 1 then
			DebugStr('SetUpDefaultOutline:Cant handle anything except 1 yet');
		GetDItem(dp, user_item, kind, h, r);
		InsetRect(r, -10, -10);
		SetDItem(dp, user_item, userItem, handle(@OutlineDefault1), r);
	end;

	procedure FlashItem (dlg: dialogPtr; item: integer);
		var
			kind: integer;
			h: handle;
			r: rect;
			f: longInt;
	begin
		GetDItem(dlg, item, kind, h, r);
		HiliteControl(controlHandle(h), inButton);
		Delay(2, f);
		HiliteControl(controlHandle(h), 0);
	end;

	procedure SetDItemRect (dp: dialogPtr; item: integer; rr: rect);
		var
			kind: integer;
			h: handle;
			r: rect;
	begin
		GetDItem(dp, item, kind, h, r);
		SetDItem(dp, item, kind, h, rr);
	end;

	procedure GetDItemRect (dp: dialogPtr; item: integer; var rr: rect);
		var
			kind: integer;
			h: handle;
	begin
		GetDItem(dp, item, kind, h, rr);
	end;

	procedure SetDItemKind (dp: dialogPtr; item: integer; k: integer);
		var
			kk: integer;
			h: handle;
			r: rect;
	begin
		GetDItem(dp, item, kk, h, r);
		SetDItem(dp, item, k, h, r);
	end;

	procedure GetDItemKind (dp: dialogPtr; item: integer; var k: integer);
		var
			r: rect;
			h: handle;
	begin
		GetDItem(dp, item, k, h, r);
	end;

	function GetDControlHandle (dp: dialogPtr; item: integer): controlHandle;
	begin
		GetDControlHandle := ControlHandle(GetDItemHandle(dp, item));
	end;

	function GetDItemhandle (dp: dialogPtr; item: integer): handle;
		var
			kind: integer;
			h: handle;
			r: rect;
	begin
		GetDItem(dp, item, kind, h, r);
		GetDItemhandle := h;
	end;

	procedure SetDItemHandle (dp: dialogPtr; item: integer; h: univ handle);
		var
			kind: integer;
			hh: handle;
			r: rect;
	begin
		GetDItem(dp, item, kind, hh, r);
		SetDItem(dp, item, kind, h, r);
	end;

	function GetDCtlEnable (dlg: dialogPtr; item: integer): boolean;
		var
			k: integer;
			h: handle;
			r: rect;
	begin
		GetDItem(dlg, item, k, h, r);
		GetDCtlEnable := controlHandle(h)^^.contrlHilite <> 255;
	end;

	procedure SetDCtlEnable (dp: dialogPtr; item: integer; on: boolean);
		var
			ch: ControlHandle;
			hilite: integer;
	begin
		ch := GetDControlHandle(dp, item);
		hilite := 255 * ord(not on);
		if ch^^.contrlHilite <> hilite then begin
			HiliteControl(ch, hilite);
		end;
	end;

	function GetDCtlTitle (dp: dialogPtr; item: integer): str255;
		var
			s: str255;
	begin
		GetCTitle(GetDControlHandle(dp, item), s);
		GetDCtlTitle := s;
	end;

	procedure SetDCtlTitle (dp: dialogPtr; item: integer; s: str255);
		var
			ch: ControlHandle;
			old: str255;
	begin
		ch := GetDControlHandle(dp, item);
		GetCTitle(ch, old);
		if old <> s then begin
			SetCTitle(ch, s);
		end;
	end;

	function GetDCtlBoolean (dp: dialogPtr; item: integer): boolean;
	begin
		GetDCtlBoolean := GetCtlValue(GetDControlHandle(dp, item)) <> 0;
	end;

	procedure SetDCtlBoolean (dp: dialogPtr; item: integer; value: boolean);
	begin
		SetCtlValue(GetDControlHandle(dp, item), ord(value));
	end;

	procedure ToggleDCtlBoolean (dp: dialogPtr; item: integer);
	begin
		SetDCtlBoolean(dp, item, not GetDCtlBoolean(dp, item));
	end;

	function GetDCtlValue (dp: dialogPtr; item: integer): integer;
	begin
		GetDCtlValue := GetCtlValue(GetDControlHandle(dp, item));
	end;

	procedure SetDCtlValue (dp: dialogPtr; item: integer; value: integer);
	begin
		SetCtlValue(GetDControlHandle(dp, item), value);
	end;

	procedure DrawDItem (dp: dialogPtr; item: integer);
	begin
		Draw1Control(GetDControlHandle(dp, item));
	end;

	function GetPopupMHandle (dlg: dialogPtr; item: integer): menuHandle;
		type
			MenuHandlePtr = ^MenuHandle;
			MenuHandleHandle = ^MenuHandlePtr;
	begin
		GetPopupMHandle := MenuHandleHandle(ControlHandle(GetDItemHandle(dlg, item))^^.contrlData)^^;
	end;

	procedure SetPopUpMenuOnMouseDown (dlg: dialogPtr; item: integer; text: str255);
		var
			mh: MenuHandle;
			i, index, start: integer;
			s: str255;
			added: boolean;
	begin
		mh := GetPopupMHandle(dlg, item);
		if text = '' then begin
			GetItem(mh, 1, text);
		end;
		GetItem(mh, 2, s);
		if s = '-' then begin
			DelMenuItem(mh, 2);
			DelMenuItem(mh, 1);
		end;
		index := 0;
		for i := 1 to CountMItems(mh) do begin
			GetItem(mh, i, s);
			if (IUEqualString(s, text) = 0) then begin
				index := i;
				leave;
			end;
		end;
		if index = 0 then begin
			InsMenuItem(mh, '(-;fred', 0);
			SetItem(mh, 1, text);
			index := 1;
		end;
		SetDCtlValue(dlg, item, index);
	end;

	procedure GetPopUpItemText (dlg: dialogPtr; item: integer; var text: str255);
		var
			mh: MenuHandle;
	begin
		mh := GetPopupMHandle(dlg, item);
		GetItem(GetPopupMHandle(dlg, item), GetDCtlValue(dlg, item), text);
	end;

	procedure GetDAFont (var font: integer);
		type
			intPtr = ^integer;
		const
			DlgFont = $AFA;
	begin
		font := intPtr(DlgFont)^;
	end;

	procedure SetWindowTitle (window: windowPtr; title: str255);
		var
			s: str255;
	begin
		GetWTitle(window, s);
		if s <> title then
			SetWTitle(window, title);
	end;

	function SelectedTextItem (dlg: DialogPtr): integer;
	begin
		SelectedTextItem := DialogPeek(dlg)^.editField + 1;
	end;

	function CountDItems (dlg: DialogPtr): integer;
		type
			IntegerPtr = ^Integer;
			IntegerHandle = ^IntegerPtr;
	begin
		CountDItems := IntegerHandle(DialogPeek(dlg)^.items)^^ + 1;
	end;

	procedure ShiftTab (dlg: DialogPtr);
		var
			gv: longInt;
			orgitem, i, count: integer;
			k: integer;
	begin
		orgitem := SelectedTextItem(dlg);
		count := CountDItems(dlg);
		if (orgitem > 0) & (count > 1) then begin
			i := orgitem;
			repeat
				i := i - 1;
				if i = 0 then begin
					i := count;
				end;
				GetDItemKind(dlg, i, k);
			until (i = orgitem) | (k = editText);
		end;
		GetDItemKind(dlg, i, k);
		if k = editText then begin
			SelIText(dlg, i, 0, 255);
		end;
	end;

	procedure DrawTheFriggingGrowIcon (window: windowPtr; bounds: rect);
		var
			clip: RgnHandle;
	begin
		SetPort(window);
		PenNormal;
		clip := NewRgn;
		GetClip(clip);
		ClipRect(bounds);
		DrawGrowIcon(window);
		SetClip(clip);
		DisposeRgn(clip);
	end;

	function DoButtonKey (dlg: DialogPtr; item: integer; var er: EventRecord; var result_item: integer): boolean;
	begin
		if GetDCtlEnable(dlg, item) then begin
			result_item := item;
			FlashItem(dlg, item);
			DoButtonKey := true;
		end
		else begin
			SysBeep(10);
			er.what := nullEvent;
			DoButtonKey := false;
		end;
	end;

	function OKModalFilter (dlg: DialogPtr; var er: EventRecord; var item: integer): boolean;
		var
			ch: integer;
	begin
		OKModalFilter := false;
		if (er.what = keyDown) or (er.what = autoKey) then begin
			ch := BAND(er.message, $FF);
			if (ch = 13) or (ch = 3) then begin
				OKModalFilter := DoButtonKey(dlg, i_ok, er, item);
			end;
		end;
	end;

	function CancelModalFilter (dlg: DialogPtr; var er: EventRecord; var item: integer): boolean;
		var
			ch: integer;
	begin
		CancelModalFilter := false;
		if (er.what = keyDown) or (er.what = autoKey) then begin
			ch := BAND(er.message, $FF);
			if (ch = 13) or (ch = 3) then begin
				CancelModalFilter := DoButtonKey(dlg, i_ok, er, item);
			end
			else if ((ch = ord('.')) and (BAND(er.modifiers, cmdKey) <> 0)) or (ch = 27) then begin
				CancelModalFilter := DoButtonKey(dlg, i_cancel, er, item);
			end;
		end;
	end;

	function CancelDiscardModalFilter (dlg: DialogPtr; var er: EventRecord; var item: integer): boolean;
		var
			ch: integer;
			result: boolean;
	begin
		CancelDiscardModalFilter := false;
		if CancelModalFilter(dlg, er, item) then begin
			CancelDiscardModalFilter := true;
		end
		else if (er.what = keyDown) or (er.what = autoKey) then begin
			ch := BAND(er.message, $FF);
			if (ch = ord('d')) and (BAND(er.modifiers, cmdKey) <> 0) then begin
				CancelDiscardModalFilter := DoButtonKey(dlg, i_discard, er, item);
			end;
		end;
	end;

	procedure EnterWindow (window: WindowPtr; font, size: integer; face: Style; var saved: SavedWindowInfo);
	begin
		GetPort(saved.oldport);
		SetPort(window);
		saved.thisport := window;
		saved.font := window^.txFont;
		saved.size := window^.txSize;
		saved.face := window^.txFace;
		TextFont(font);
		TextSize(size);
		TextFace(face);
	end;

	procedure ExitWindow (saved: SavedWindowInfo);
	begin
		SetPort(saved.thisport);
		TextFont(saved.font);
		TextSize(saved.size);
		TextFace(saved.face);
		SetPort(saved.oldport);
	end;

	procedure DrawGrayRect (dlg: DialogPtr; item: integer; title: str255);
		const
			left_indent = 20;
			gap = 2;
		var
			r, er: rect;
			fi: FontInfo;
			sw: integer;
	begin
		GetDItemRect(dlg, item, r);
		GetFontInfo(fi);
		MoveTo(r.left + left_indent, r.top + fi.ascent);
		sw := StringWidth(title);
		er.top := r.top;
		er.bottom := er.top + fi.ascent + fi.descent;
		er.left := r.left + left_indent;
		er.right := er.left + sw;
		EraseRect(er);
		DrawString(title);
		PenPat(gray);
		r.top := r.top + (fi.ascent) div 2;
		MoveTo(er.left - gap, r.top);
		LineTo(r.left, r.top);
		LineTo(r.left, r.bottom);
		LineTo(r.right, r.bottom);
		LineTo(r.right, r.top);
		LineTo(er.right + gap, r.top);
		PenNormal;
	end;

	function Split (sub, s: str255; var s1, s2: str255): boolean;
		var
			p: integer;
	begin
		p := Pos(sub, s);
		if p > 0 then begin
			s1 := copy(s, 1, p - 1);
			s2 := copy(s, p + length(sub), 255);
		end;
		Split := p > 0;
	end;

	procedure DisplayStyledString (dlg: dialogPtr; item: integer; s: str255);
		var
			t: str255;
			box: rect;
			just: integer;
			this: str255;
			font, size, i, index: integer;
			st: Style;
			fi: FontInfo;
			fixsize: boolean;
			oldfont, oldsize: integer;
			oldface: Style;
	begin
		SetPort(dlg);
		oldfont := dlg^.txFont;
		oldsize := dlg^.txSize;
		oldface := dlg^.txFace;
		GetDItemRect(dlg, item, box);
		if Split(':', s, this, s) then begin
			fixsize := false;
			if this = '' then begin
				font := geneva;
			end
			else begin
				GetFNum(this, font);
				if font = 0 then begin
					fixsize := true;
					font := geneva;
				end;
			end;
			if Split(':', s, this, s) then begin
				if this = '' then begin
					size := 9;
				end
				else begin
					ReadString(this, size);
				end;
				if Split(':', s, this, s) then begin
					st := [];
					for i := 1 to length(this) do begin
						st := st + [StyleItem(ord(this[i]) - 48)]
					end;
					if Split(':', s, this, s) then begin
						if this = '' then begin
							just := teJustLeft;
						end
						else begin
							ReadString(this, just);
						end;
						TextFont(font);
						TextSize(size);
						TextFace(st);
						if fixsize then begin
							GetFontInfo(fi);
							while (fi.ascent + fi.descent > box.bottom - box.top) do begin
								if size > 48 then begin
									size := 48;
								end
								else if size > 36 then begin
									size := 36;
								end
								else if size > 27 then begin
									size := 27;
								end
								else if size > 24 then begin
									size := 24;
								end
								else if size > 18 then begin
									size := 18;
								end
								else if size > 14 then begin
									size := 14;
								end
								else if size > 12 then begin
									size := 12;
								end
								else begin
									size := 9;
									TextSize(size);
									leave;
								end;
								TextSize(size);
								GetFontInfo(fi);
							end;
						end;
						TextBox(@s[1], length(s), box, just);
					end;
				end;
			end;
		end;
		TextFont(oldfont);
		TextSize(oldsize);
		TextFace(oldface);
	end;

end.
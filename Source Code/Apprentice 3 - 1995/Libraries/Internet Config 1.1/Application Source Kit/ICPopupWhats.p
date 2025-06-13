unit ICPopupWhats;

interface

	uses
		ICWindowGlobals;

	function WhatOpenPopup (wt: WindowType; item: integer): OSErr;
	function WhatFlushPopup (wt: WindowType; item: integer): OSErr;
	function WhatClosePopup (wt: WindowType; item: integer): OSErr;
	function WhatClickPopup (wt: WindowType; item: integer; er: eventRecord): OSErr;

implementation

	uses
		ICStrH, 

		ICDialogs, ICMiscSubs, ICSubs, ICAPI, ICDocUtils, ICMovableModal;

	procedure InternalSetItem (mh: MenuHandle; item_num: integer; text: Str255);
	begin
		Delete(text, Pos(':', text), 255);
		SetItem(mh, item_num, text);
	end; (* InternalSetItem *)

	procedure SafeAppendMenu (mh: MenuHandle; text: str255);
	begin
		AppendMenu(mh, 'fred');
		InternalSetItem(mh, CountMItems(mh), text);
	end;

	procedure SafeInsertMenuItem (mh: MenuHandle; after: integer; text: str255);
	begin
		InsMenuItem(mh, 'fred', after);
		InternalSetItem(mh, after, text);
	end;

	procedure AdjustPopupMenu (dlg: DialogPtr; item: integer);
		var
			control: ControlHandle;
			menuh: MenuHandle;
			servers: handle;
			server: str255;
			servers_count, menu_count, i, found: integer;
			longmenu: boolean;
	begin
		control := GetDControlHandle(dlg, item);
		menuh := GetPopupMHandle(dlg, item);
		servers := handle(control^^.contrlRfCon);
		menu_count := CountMItems(menuh);
		servers_count := CountStrsH(servers);

		if menu_count = 0 then begin
			for i := 2 to servers_count do begin
				SafeAppendMenu(menuh, GetIndStrH(servers, i));
			end;
			AppendMenu(menuh, '(-');
			AppendMenu(menuh, GetAString(128, 2));
			menu_count := CountMItems(menuh);
		end;
		found := -1;
		server := GetIndStrH(servers, 1);
		for i := 2 to servers_count do begin
			if server = GetIndStrH(servers, i) then begin
				found := i;
				leave;
			end;
		end;
		longmenu := menu_count = servers_count + 3;
		if found > 0 then begin
			if longmenu then begin
				DelMenuItem(menuh, 1);
				DelMenuItem(menuh, 1);
				SetDCtlValue(dlg, item, GetDCtlValue(dlg, item) - 2);
			end;
			SetCtlMax(GetDControlHandle(dlg, item), CountMItems(menuh));
			SetDCtlValue(dlg, item, found - 1);
		end
		else begin
			if not longmenu then begin
				InsMenuItem(menuh, '(-;fred', 0);
				SetDCtlValue(dlg, item, GetDCtlValue(dlg, item) + 2);
			end;
			InternalSetItem(menuh, 1, GetIndStrH(servers, 1));
			SetCtlMax(GetDControlHandle(dlg, item), CountMItems(menuh));
			SetDCtlValue(dlg, item, 1);
		end;
		EnableItem(menuh, GetDCtlValue(dlg, item));
	end;

	function UsingName (servers: Handle; name: Str255): boolean;
		var
			tmpstr: Str255;
			i: integer;
	begin
		UsingName := false;
		for i := 2 to CountStrsH(servers) do begin
			tmpstr := GetIndStrH(servers, i);
			Delete(tmpstr, Pos(':', tmpstr), 255);
			if IUEqualString(tmpstr, name) = 0 then begin
				UsingName := true;
				leave;
			end; (* if *)
		end; (* for *)
	end; (* UsingName *)

	var
		ask_filter_globals: Handle;

	function AskServerFilter (dlog: DialogPtr; var event: EventRecord; var item: integer): boolean;
		var
			dim_ok: boolean;
			name_string, host_string: Str255;
	begin
		GetItemText(dlog, 4, name_string);
		GetItemText(dlog, 5, host_string);
		dim_ok := (name_string = '') | (host_string = '') | UsingName(ask_filter_globals, name_string);
		SetDCtlEnable(dlog, ok, not dim_ok);
		OutlineDefault1(dlog, 3);
		AskServerFilter := CancelModalFilter(dlog, event, item);
	end; (* AskServerFilter *)

	function AskOtherServer (archie_popup: boolean; servers: Handle; var server: Str255): boolean;
		var
			modal: DialogPtr;
			result: boolean;
			item: integer;
			tmpstr: Str255;
			saved_state: Ptr;
	begin
		result := false;
		modal := GetNewDialog(400, nil, WindowPtr(-1));
		if modal <> nil then begin
			ParamText(GetAString(128, 3 + ord(archie_popup)), '', '', '');
			SelIText(modal, 4, 0, 32767);
			SetUpDefaultOutline(modal, ok, 3);
			if archie_popup then begin
				HideDItem(modal, 6);
				HideDItem(modal, 10);
			end; (* if *)
			ShowWindow(modal);
			DisableMenuBar(saved_state, -1);
			InitCursor;
			ask_filter_globals := servers;
			repeat
				MovableModalDialog(@AskServerFilter, item);
			until item in [ok, cancel];
			result := (item = ok);
			if result then begin
				GetItemText(modal, 4, server);
				GetItemText(modal, 5, tmpstr);
				server := concat(server, ':', tmpstr);
				GetItemText(modal, 6, tmpstr);
				server := concat(server, ':', tmpstr);
			end; (* if *)
			ReEnableMenuBar(saved_state);
			DisposeWindow(modal);
		end; (* if *)
		AskOtherServer := result;
	end; (* AskOtherServer *)

	const
		pf_archie = 0;
		pf_archie_mask = $0001;

	function WhatClickPopup (wt: WindowType; item: integer; er: eventRecord): OSErr;
		var
			dlg: WindowPtr;
			value: integer;
			control: ControlHandle;
			menuh: MenuHandle;
			servers: handle;
			server: str255;
			servers_count, menu_count, i, found: integer;
			longmenu: boolean;
			archie_popup: boolean;
	begin
		dlg := windowinfo[wt].window;
		control := GetDControlHandle(dlg, item);
		menuh := GetPopupMHandle(dlg, item);
		servers := handle(control^^.contrlRfCon);
		menu_count := CountMItems(menuh);
		servers_count := CountStrsH(servers);
		longmenu := (menu_count = servers_count + 3);

		value := GetDCtlValue(dlg, item);
		if value = menu_count then begin
			archie_popup := btst(windowinfo[wt].items[item]^.flags, pf_archie);
			if AskOtherServer(archie_popup, servers, server) then begin
				SetIndStrH(servers, 1, server);
			end; (* if *)
			AdjustPopupMenu(dlg, item);
		end
		else begin
			value := value - 2 * ord(longmenu);
			if value > 0 then begin
				server := GetIndStrH(servers, value + 1);
				SetIndStrH(servers, 1, server);
				AdjustPopupMenu(dlg, item);
			end;
		end;
		WhatClickPopup := noErr;
	end; (* WhatClickPopup *)

	function WhatOpenPopup (wt: WindowType; item: integer): OSErr;
		var
			dlg: WindowPtr;
			server: str255;
			servers: handle;
			keybase: Str255;
			attr: longint;
			err: OSErr;
	begin
		err := noErr;
		keybase := windowinfo[wt].items[item]^.key;
		servers := nil;
		if ICGetPrefHandle(GetInstance, concat(keybase, 'All'), attr, servers) <> noErr then begin
			servers := NewStrH;
			if servers = nil then begin
				err := memFullErr;
			end; (* if *)
		end; (* if *)
		if ICGetPrefStr(GetInstance, concat(keybase, 'Preferred'), attr, server) <> noErr then begin
			server := GetIndStrH(servers, 1);
		end; (* if *)
		ProcessAttributes(wt, item, attr);
		dlg := windowinfo[wt].window;
		GetDControlHandle(dlg, item)^^.contrlRfCon := longint(servers);
		InsIndStrH(servers, 1, server);
		AdjustPopupMenu(dlg, item);
		SetDCtlEnable(dlg, item, not IsLocked(wt, item));
		WhatOpenPopup := err;
	end; (* WhatOpenPopup *)

	function WhatFlushPopup (wt: WindowType; item: integer): OSErr;
		var
			dlg: WindowPtr;
			server: Str255;
			old_server: Str255;
			keybase: Str255;
			err: OSErr;
			attr: longint;
	begin
		dlg := windowinfo[wt].window;
		server := GetIndStrH(Handle(GetDControlHandle(dlg, item)^^.contrlRfCon), 1);
		keybase := windowinfo[wt].items[item]^.key;
		if ICMapErr(ICGetPrefStr(GetInstance, concat(keybase, 'Preferred'), attr, old_server)) <> noErr then begin
			old_server := '::';
		end; (* if *)
		err := noErr;
		if server <> old_server then begin
			err := ICMapErr(ICSetPrefStr(GetInstance, concat(keybase, 'Preferred'), ICattr_no_change, server));
			if err = noErr then begin
				DirtyDocument;
			end; (* if *)
		end; (* if *)
		WhatFlushPopup := err;
	end; (* WhatFlushPopup *)

	function WhatClosePopup (wt: WindowType; item: integer): OSErr;
	begin
		DisposeHandle(Handle(GetDControlHandle(windowinfo[wt].window, item)^^.contrlRfCon));
		WhatClosePopup := noErr;
	end; (* WhatClosePopup *)

end. (* ICPopupWhats *)
unit ICFSSpecWhats;

interface

	uses
		ICWindowGlobals;

	function WhatOpenFSSpec (wt: WindowType; item: integer): OSErr;
	function WhatFlushFSSpec (wt: WindowType; item: integer): OSErr;
	function WhatClickFSSpec (wt: WindowType; item: integer; er: eventRecord): OSErr;

implementation

	uses
		Folders, 

		ICDialogs, ICMiscSubs, ICSubs, ICAPI, ICDocUtils, ICKeys, ICGlobals, ICStandardFile;

	function GetVolumeStuff (name: str31; date: longInt; var vrn: integer): OSErr;
		var
			err: OSErr;
			pb: HParamBlockRec;
			index: integer;
			s: str255;
			pass, i: integer;
	begin
		for pass := 1 to 2 do begin
			i := 1;
			while true do begin
				pb.ioVolIndex := i;
				i := i + 1;
				pb.ioNamePtr := @s;
				s := '';
				err := PBGetVInfoSync(@pb);
				if err <> noErr then begin
					leave;
				end;
				if IUEqualString(name, s) = 0 then begin
					if (pass = 2) or (pb.ioVCrDate = date) then begin
						leave;
					end;
				end;
			end;
			if err = noErr then begin
				leave;
			end;
		end;
		if err = noErr then begin
			vrn := pb.ioVRefNum;
		end;
		GetVolumeStuff := err;
	end;

	function WhatOpenFSSpec (wt: WindowType; item: integer): OSErr;
		var
			spec: handle;
			valid: boolean;
			loe: longInt;
			changed: boolean;
			attr: longint;
			pb: CInfoPBRec;
			dlg: DialogPtr;
			junk: OSErr;
			modified: boolean;
			fs: FSSpec;
	begin
		valid := false;
		spec := nil;
		modified := false;
		if ICGetPrefHandle(GetInstance, windowinfo[wt].items[item]^.key, attr, spec) = noErr then begin
			valid := true;
		end;
		ProcessAttributes(wt, item, attr);
		if valid & (GetHandleSize(spec) < SizeOf(ICFileSpec)) then begin
			valid := false;
		end;
		if valid then begin
			if (ICFileSpecHandle(spec)^^.alias.aliasSize = 0) or not has_aliasMgr then begin
				if GetVolumeStuff(ICFileSpecHandle(spec)^^.vol_name, ICFileSpecHandle(spec)^^.vol_creation_date, fs.vRefNum) = noErr then begin
					fs.parID := ICFileSpecHandle(spec)^^.fss.parID;
					fs.name := ICFileSpecHandle(spec)^^.fss.name;
					if FSpGetCatInfo(fs, 0, pb) <> noErr then begin
						modified := true;
						valid := false;
					end;
				end
				else begin
					valid := false;
				end;
			end
			else begin
				loe := Munger(spec, 0, nil, SizeOf(ICFileSpec) - SizeOf(AliasRecord), @loe, 0);
				if ResolveAlias(nil, AliasHandle(spec), fs, changed) <> noErr then begin
					modified := true;
					valid := false;
				end;
			end;
		end;
		if not valid then begin
			if (FindFolder(kOnSystemDisk, kDesktopFolderType, true, fs.vRefNum, fs.parID) = noErr) & (FSpGetCatInfo(fs, -1, pb) = noErr) then begin
				fs.vRefNum := pb.ioVRefNum;
				fs.parID := pb.ioDrParID;
			end
			else begin
				fs.vRefNum := 0;
				fs.parID := 0;
				fs.name := '';
			end; (* if *)
		end;
		DisposeHandle(spec);
		windowinfo[wt].items[item]^.fss := fs;
		windowinfo[wt].items[item]^.modified := modified;
		SetDCtlTitle(windowinfo[wt].window, item, fs.name);
		SetDCtlEnable(windowinfo[wt].window, item, not IsLocked(wt, item));
		WhatOpenFSSpec := noErr;
	end; (* WhatOpenFSSpec *)

	function WhatClickFSSpec (wt: WindowType; item: integer; er: eventRecord): OSErr;
		var
			err: OSErr;
			fs: FSSpec;
			dirID: longInt;
	begin
		err := ICStandardGetFolder(fs, dirID);
		if err = noErr then begin
			DirtyDocument;
			windowinfo[wt].items[item]^.fss.vRefNum := fs.vRefNum;
			windowinfo[wt].items[item]^.fss.parID := dirID;
			windowinfo[wt].items[item]^.fss.name := '';
			SetDCtlTitle(windowinfo[wt].window, item, fs.name);
		end; (* if *)
		WhatClickFSSpec := err;
	end; (* WhatClickFSSpec *)

	function WhatFlushFSSpec (wt: WindowType; item: integer): OSErr;
		var
			alias: AliasHandle;
			ifs: ICFileSpec;
			pb: HParamBlockRec;
			err: OSErr;
			pos: longInt;
	begin
		ifs.fss := windowinfo[wt].items[item]^.fss;
		alias := nil;
		err := -1;
		if has_aliasMgr then begin
			err := NewAlias(nil, ifs.fss, alias);
		end;
		if err <> noErr then begin
			alias := AliasHandle(NewHandle(sizeof(AliasRecord)));
			err := MemError;
			if err = noErr then begin
				alias^^.aliasSize := 0;
				alias^^.userType := OSType(0);
			end; (* if *)
		end; (* if *)

		if err = noErr then begin
			pb.ioVRefNum := ifs.fss.vRefNum;
			pb.ioVolIndex := 0;
			pb.ioNamePtr := @ifs.vol_name;
			err := PBGetVInfoSync(@pb);
			if err = noErr then begin
				ifs.vol_creation_date := pb.ioVCrDate;
			end; (* if *)
		end; (* if *)

		if err = noErr then begin
			pos := Munger(Handle(alias), 0, nil, 0, @ifs, SizeOf(ICFileSpec) - SizeOf(AliasRecord));
			err := MemError;
		end; (* if *)

		if err = noErr then begin
			err := ICMapErr(ICSetPrefHandle(GetInstance, windowinfo[wt].items[item]^.key, ICattr_no_change, Handle(alias)));
		end; (* if *)

		if alias <> nil then begin
			DisposeHandle(Handle(alias));
		end; (* if *)
		WhatFlushFSSpec := err;
	end; (* WhatFlushFSSpec *)

end. (* ICFSSpecWhats *)
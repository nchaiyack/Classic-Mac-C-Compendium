unit ICDocument;

interface

	uses
		ICTypes;

	function DoNewDoc: OSErr;
	function DoOpenDoc (fss: FSSpec): OSErr;
	function DoCloseDocWindow (wp: WindowPtr): OSErr;
	function DoCloseDoc: OSErr;
	function DoQuit: OSErr;

	function DoSFOpen: OSErr;
	function DoSave: OSErr;
	function DoSaveAs: OSErr;
	function DoOpenInternetPreferences: OSErr;

	function IsDocOpen: boolean;
	function IsDocDirty: boolean;
	function IsDocNew: boolean;

	function GetDocumentName: Str255;
	function GetInstance: ICInstance;
	procedure DirtyDocument;
	function IsDocLocked: boolean;

	function DoOpenApp: OSErr;

	function InitICDocument: OSErr;
	procedure TermICDocument;

	function EditPreference (var key: Str255; var fs: FSSpec; usefss, usecurrent: boolean): OSErr;

implementation

	uses
		Folders, 

		ICStrH, 

		ICTypes, ICAPI, ICKeys, 

		ICGlobals, ICMiscSubs, ICSubs, ICDialogs, ICWindowGlobals, ICWindows, ICStandardFile;

	var
		instance: ICInstance;
		current_file: FSSpec;
		current_seed: longInt;
		scratch_file: FSSpec;			(* open if name <> '' *)
		new_document: boolean;
		dirty_document: boolean;
		locked_document: boolean;

	function CopyFileSafe (source, dest: FSSpec): OSErr;
		var
			temp: FSSpec;
			err, junk: OSErr;
			count: integer;
	begin
		temp := dest;
		count := 0;
		repeat
			temp.name := concat('Internet Config Temp ', DecStr(count));
			err := HCreate(temp.vRefNum, temp.parID, temp.name, ICcreator, ICfiletype);
			count := count + 1;
		until err <> dupFNErr;
		if err = noErr then begin
			err := CopyFile(source, temp);
			if err = noErr then begin
				junk := HDelete(dest.vRefNum, dest.parID, dest.name);
				err := HRename(dest.vRefNum, dest.parID, temp.name, dest.name);
			end;
		end;
		CopyFileSafe := err;
	end;

	function GetInstance: ICInstance;
	begin
		GetInstance := instance;
	end; (* GetInstance *)

	function IsDocOpen: boolean;
	begin
		IsDocOpen := (scratch_file.name <> '');
	end; (* IsDocOpen *)

	function IsDocDirty: boolean;
	begin
		IsDocDirty := IsDocOpen and dirty_document;
	end; (* IsDocDirty *)

	function IsDocNew: boolean;
	begin
		IsDocNew := IsDocOpen and new_document;
	end; (* IsDocNew *)

	procedure DirtyDocument;
	begin
		dirty_document := true;
	end; (* DirtyDocument *)

	function IsDocLocked: boolean;
	begin
		IsDocLocked := IsDocOpen and locked_document;
	end; (* IsDocLocked *)

	function GetDocumentName: Str255;
	begin
		if current_file.name = '' then begin
			GetDocumentName := GetAString(128, 1);
		end
		else begin
			GetDocumentName := current_file.name;
		end; (* if *)
	end; (* GetDocumentName *)

	procedure AddDefaultPrefs;

		function PrefExists (key: str255): boolean;
			var
				attr: ICAttr;
				count: longInt;
		begin
			PrefExists := ICGetPref(instance, key, attr, nil, count) = noErr;
		end;

		procedure SetPrefHandle (key: str255; h: handle);
			var
				junk: ICError;
		begin
			if not PrefExists(key) then begin
				junk := ICSetPrefHandle(instance, key, ICattr_no_change, h);
			end;
		end;

		procedure SetPrefStr (key: str255; data: str255);
			var
				junk: ICError;
		begin
			if not PrefExists(key) then begin
				junk := ICSetPrefStr(instance, key, ICattr_no_change, data);
			end;
		end;

		procedure SetPref (key: str255; data: ptr; size: longInt);
			var
				junk: ICError;
		begin
			if not PrefExists(key) then begin
				junk := ICSetPref(instance, key, ICattr_no_change, data, size);
			end;
		end;

		procedure CopyServerPref (id: integer; key: Str255);
			var
				tmph: Handle;
		begin
			tmph := GetResource('STR#', id);
			if tmph <> nil then begin
				SetPrefHandle(concat(key, 'All'), tmph);
				SetPrefStr(concat(key, 'Preferred'), GetIndStrH(tmph, 1));
			end; (* if *)
		end; (* CopyServerPref *)

		procedure CopyPrefH (source_type: ResType; source_id: integer; dest_key: Str255);
			var
				tmph: Handle;
		begin
			tmph := GetResource(source_type, source_id);
			if tmph <> nil then begin
				SetPrefHandle(dest_key, tmph);
			end; (* if *)
		end; (* CopyPrefH *)

		procedure CopyHelpers;
			var
				x: integer;
				key: Str255;
				appspec: ICAppSpec;
				i: integer;
		begin
			x := 1;
			for i := 1 to CountStrs(666) div 3 do begin
				key := GetIndStr(666, x);
				x := x + 1;
				appspec.fCreator := copy(concat(GetIndStr(666, x), '    '), 1, 4);
				x := x + 1;
				appspec.name := GetIndStr(666, x);
				x := x + 1;
				SetPref(concat(kICHelper, key), @appspec, sizeof(appspec));
			end; (* for *)
		end; (* CopyHelpers *)

		var
			err: OSErr;
			junk: OSErr;
	begin
		CopyServerPref(200, 'Archie');
		CopyServerPref(201, 'InfoMac');
		CopyServerPref(202, 'UMich');
		CopyPrefH('TaBl', 128, kICCharacterSet);
		CopyPrefH('MAP ', 128, kICMapping);
		CopyPrefH('STR ', 128, kICQuotingString);
		CopyPrefH('SERV', 128, kICServices);
		SetPrefStr(kICRealName, GetOwnerName);
		CopyHelpers;
	end;

	function CreateScratchFile (protofile: FSSpecPtr): OSErr;
		var
			err: OSErr;
			count: integer;
	begin
		err := FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, scratch_file.vRefNum, scratch_file.parID);
		if err = noErr then begin
(*    scratch_file.parID := 2; *)
			count := 0;
			repeat
				scratch_file.name := concat('Internet Config Temp ', DecStr(count));
				err := HCreate(scratch_file.vRefNum, scratch_file.parID, scratch_file.name, ICcreator, ICfiletype);
				count := count + 1;
			until err <> dupFNErr;
		end; (* if *)
		if err = noErr then begin
			if protofile = nil then begin
				HCreateResFile(scratch_file.vRefNum, scratch_file.parID, scratch_file.name);
				(* temporary workaround bug in ICAPI *)
			end
			else begin
				err := CopyFile(protofile^, scratch_file);
				if err = noErr then begin
					err := HRstFLock(scratch_file.vRefNum, scratch_file.parID, scratch_file.name);
				end; (* if *)
			end;
		end;
		if err = noErr then begin
			err := ICMapErr(ICSpecifyConfigFile(instance, scratch_file));
		end; (* if *)
		if err = noErr then begin
			err := ICMapErr(ICBegin(instance, icReadWritePerm));
			if err = noErr then begin
				AddDefaultPrefs;
				err := ICMapErr(ICEnd(instance));
			end; (* if *)
		end; (* if *)
		CreateScratchFile := err;
	end; (* CreateScratchFile *)

	function DoCloseDoc: OSErr;
		var
			but: integer;
			err: OSErr;
			junk: OSErr;
	begin
		err := WindowsCloseAll;
		if (err = noErr) & IsDocDirty then begin
			ParamText(GetDocumentName, '', '', '');
			InitCursor;
			but := CautionAlert(135, @CancelDiscardModalFilter);
			case but of
				ok:  begin
					err := DoSave;
				end;
				cancel: 
					err := userCanceledErr;
				otherwise
			end; (* case *)
		end; (* if *)
		if err = noErr then begin
			err := WindowsClose(windowinfo[WT_Main].window);
		end; (* if *)
		if err = noErr then begin
			junk := HDelete(scratch_file.vRefNum, scratch_file.parID, scratch_file.name);
			scratch_file.name := '';
		end; (* if *)
		DoCloseDoc := err;
	end; (* DoCloseDoc *)

	function DoCloseDocWindow (wp: WindowPtr): OSErr;
		var
			wt: WindowType;
			but: integer;
			confirmed: boolean;
			err: OSErr;
	begin
		err := noErr;
		wt := GetWindowType(wp);
		case wt of
			WT_None: 
				;
			WT_About: 
				HideWindow(wp);
			WT_Main: 
				err := DoCloseDoc;
			otherwise
				err := WindowsClose(wp);
		end; (* case *)
		DoCloseDocWindow := err;
	end; (* DoCloseDocWindow *)

	function DoNewDoc: OSErr;
		var
			err: OSErr;
			junk: OSErr;
	begin
		err := DoCloseDoc;
		if err = noErr then begin
			WindowsResetPositions;
			new_document := true;
			dirty_document := false;
			locked_document := false;
			err := CreateScratchFile(nil);
			if err = noErr then begin
				err := WindowsOpen(WT_Main);
			end;
			if err = noErr then begin
				WindowsSetTitle(WT_Main, GetAString(128, 1));
				current_file.name := '';						(* make it untitled *)
			end; (* if *)
		end; (* if *)
		DoNewDoc := err;
	end; (* DoNewDoc *)

	procedure CurrentSeed (var seed: longInt);
		var
			err: OSErr;
	begin
		seed := 0;
		if current_file.name <> '' then begin
			err := ICSpecifyConfigFile(instance, current_file);
			if err = noErr then begin
				err := ICGetSeed(instance, seed);
			end;
			err := ICSpecifyConfigFile(instance, scratch_file);
		end;
	end;

	function SameSeed (seed1, seed2: longInt): boolean;
	begin
		SameSeed := (seed1 = seed2) or (seed1 = 0) or (seed2 = 0);
	end;

	function DoOpenDoc (fss: FSSpec): OSErr;
		var
			count: integer;
			err: OSErr;
			junk: OSErr;
	begin
		err := DoCloseDoc;
		if err = noErr then begin
			new_document := false;
			dirty_document := false;
			locked_document := FileLocked(fss);
			err := CreateScratchFile(@fss);
			if err = noErr then begin
				WindowsRestorePositions;
				err := WindowsOpen(WT_Main);
			end;
			if err = noErr then begin
				WindowsSetTitle(WT_Main, fss.name);
				current_file := fss;
			end; (* if *)
			CurrentSeed(current_seed);
		end; (* if *)
		DoOpenDoc := err;
	end; (* DoOpenDoc *)

	function DoQuit: OSErr;
		var
			err: OSErr;
	begin
		err := DoCloseDoc;
		if err = noErr then begin
			quitNow := true;
		end; (* if *)
		DoQuit := err;
	end; (* DoQuit *)

	function DoSFOpen: OSErr;
		var
			err: OSErr;
			fss: FSSpec;
			info: FInfo;
	begin
		err := ICStandardGetFile(ICfiletype, fss, info);
		if err = noErr then begin
			err := DoOpenDoc(fss);
		end; (* if *)
		DoSFOpen := err;
	end; (* DoSFOpen *)

	function FindInternetPreferences (var default_config: FSSpec): OSErr;
		var
			err: OSErr;
			isfolder, wasalias: boolean;
	begin
		err := FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, default_config.vRefNum, default_config.parID);
		if err = noErr then begin
			err := ICMapErr(ICDefaultFileName(instance, default_config.name));
		end; (* if *)
		if err = noErr then begin
			if has_aliasMgr then begin
				err := ResolveAliasFile(default_config, true, isfolder, wasalias);
			end;
		end;
		FindInternetPreferences := err;
	end;

	function EditPreference (var key: Str255; var fs: FSSpec; usefss, usecurrent: boolean): OSErr;
		var
			err: OSErr;
	begin
		err := noErr;
		if not usefss then begin
			if usecurrent then begin
				fs := current_file;
				usecurrent := IsDocOpen;
			end;
			if not usecurrent then begin
				err := FindInternetPreferences(fs);
			end;
		end;
		if err = noErr then begin
			if not IsDocOpen | (fs.vrefnum <> current_file.vrefnum) | (fs.parID <> current_file.parID) | (IUEqualString(fs.name, current_file.name) <> 0) then begin
				err := DoOpenDoc(fs);
			end;
		end;
		if err = noErr then begin
			if key <> '' then begin
				err := EditCurrentPreference(key);
			end;
		end;
		EditPreference := err;
	end;

	function DoOpenInternetPreferences: OSErr;
		var
			default_config: FSSpec;
			err: OSErr;
	begin
		err := FindInternetPreferences(default_config);
		if err = noErr then begin
			err := DoOpenDoc(default_config);
		end; (* if *)
		DoOpenInternetPreferences := err;
	end;

	function InternalDoSave (fss: FSSpec): OSErr;
		var
			err: OSErr;
	begin
		err := noErr;
		err := WindowsFlushAll;
		if err = noErr then begin
			WindowsSavePositions;
			err := CopyFileSafe(scratch_file, fss);
		end; (* if *)
		if err = noErr then begin
			WindowsSetTitle(WT_Main, fss.name);
			new_document := false;
			dirty_document := false;
			current_file := fss;
		end; (* if *)
		CurrentSeed(current_seed);
		InternalDoSave := err;
	end; (* InternalDoSave *)

	function DoSave: OSErr;
		var
			err: OSErr;
			seed: longInt;
			a: integer;
	begin
		if current_file.name = '' then begin
			err := DoSaveAs;
		end
		else begin
			CurrentSeed(seed);
			a := ok;
			if not SameSeed(seed, current_seed) then begin
				a := CautionAlert(160, @CancelModalFilter);
			end;
			if a = ok then begin
				err := InternalDoSave(current_file);
			end;
		end; (* if *)
		DoSave := err;
	end; (* DoSave *)

	function DoSaveAs: OSErr;
		var
			err: OSErr;
			fss: FSSpec;
	begin
		err := ICStandardPutFile('', GetDocumentName, fss);
		if err = noErr then begin
			err := InternalDoSave(fss);
		end; (* if *)
		DoSaveAs := err;
	end; (* DoSaveAs *)

	function DoOpenApp: OSErr;
		var
			default_config: FSSpec;
			err: OSErr;
	begin
		err := FindInternetPreferences(default_config);
		if err = noErr then begin
			err := DoOpenDoc(default_config);
		end; (* if *)
		if err = fnfErr then begin
			err := DoNewDoc;
			if err = noErr then begin
				err := InternalDoSave(default_config);
			end; (* if *)
		end; (* if *)
		DoOpenApp := err;
	end; (* DoOpenApp *)

	function InitICDocument: OSErr;
		var
			inst: ICInstance;
			err: OSErr;
	begin
		instance := nil;
		current_file.name := '';
		scratch_file.name := '';
		err := ICMapErr(ICStart(inst, ICcreator));
		if err = noErr then begin
			instance := inst;
		end; (* if *)
		InitICDocument := err;
	end; (* InitICDocument *)

	procedure TermICDocument;
		var
			junk: ICError;
	begin
		if instance <> nil then begin
			junk := ICStop(instance);
		end; (* if *)
	end; (* TermICDocument *)

end. (* ICDocument *)
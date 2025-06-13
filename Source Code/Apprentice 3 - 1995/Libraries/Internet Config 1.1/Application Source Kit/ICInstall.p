unit ICInstall;

interface

	procedure AdjustInstalMenu (menu: integer);
	procedure DoInstallMenu (menu, item: integer);
	function InitializeComponentInstallation: OSErr;

implementation

	uses
		Folders, Components, 

		ICTypes, ICCAPI, 

		ICMiscSubs, ICGlobals, ICStandardFile, ICDialogs, ICSubs, ICAPI;

	const			(* why aren't these in Components.p??? *)
		registerCmpGlobal = 1;
		registerCmpNoDuplicates = 2;
		registerCompAfter = 4;

	var
		application_version: longInt;
		installed_version: longInt;

	function GetVersionFromResFile: longInt;
		var
			versh: VersRecHndl;
	begin
		GetVersionFromResFile := 0;
		versh := VersRecHndl(Get1Resource('vers', 1));
		if versh <> nil then begin
			GetVersionFromResFile := versh^^.numericVersion.version;
		end; (* if *)
	end;

	function GetRegisteredVersion: longInt;
		var
			inst: ComponentInstance;
			junk: OSErr;
			refnum: integer;
	begin
		GetRegisteredVersion := 0;
		if has_components then begin
			inst := OpenDefaultComponent(internetConfigurationComponentType, internetConfigurationComponentSubType);
			if inst <> nil then begin
				refnum := OpenComponentResFile(Component(inst));
				if refnum <> -1 then begin
					GetRegisteredVersion := GetVersionFromResFile;
					CloseResFile(refnum);
				end;
				junk := CloseComponent(inst);
			end; (* if *)
		end; (* if *)
	end;

	function IsInstalled (var where: FSSpec): boolean;
		var
			err: OSErr;
			ndx: integer;
			found: boolean;
			cpb: CInfoPBRec;
			info: FInfo;
	begin
		err := FindFolder(kOnSystemDisk, kExtensionFolderType, true, where.vRefNum, where.parID);
		where.name := GetAString(128, 12);
		if err = noErr then begin
			err := HGetFInfo(where.vRefNum, where.parID, where.name, info);
			if err <> noErr then begin
				found := false;
				ndx := 1;
				repeat
					with cpb do begin
						ioNamePtr := @where.name;
						ioVRefNum := where.vRefNum;
						ioDirID := where.parID;
						ioFDirIndex := ndx;
					end; (* with *)
					err := PBGetCatInfoSync(@cpb);
					if err = noErr then begin
						found := (cpb.ioFlFndrInfo.fdType = 'thng') and (cpb.ioFlFndrInfo.fdCreator = ICcreator);
					end; (* if *)
					ndx := ndx + 1;
				until found or (err <> noErr);
				if not found then begin
					where.name := GetAString(128, 12);
					err := fnfErr;
				end; (* if *)
			end; (* if *)
		end; (* if *)
		IsInstalled := (err = noErr);
	end; (* IsInstalled *)

	procedure UpdateInstalledVersion (var fss: FSSpec);
		var
			ref: integer;
			err: OSErr;
	begin
		installed_version := 0;
		if IsInstalled(fss) then begin
			ref := HOpenResFile(fss.vRefNum, fss.parID, fss.name, fsRdPerm);
			err := ResError;
			if err = noErr then begin
				installed_version := GetVersionFromResFile;
				CloseResFile(ref);
			end; (* if *)
		end;
	end;

	function SaveComponentToFile (fss: FSSpec): OSErr;
		var
			err: OSErr;
			thng: Handle;
			rref: integer;
			junk: OSErr;
	begin
		err := noErr;
		thng := GetResource('ThNg', 128);
		if thng = nil then begin
			err := resNotFound;
		end; (* if *)
		if err = noErr then begin
			HNoPurge(thng);
			junk := HCreate(fss.vRefNum, fss.parID, fss.name, ICcreator, 'thng');
			err := HOpenRF(fss.vRefNum, fss.parID, fss.name, fsRdWrPerm, rref);
			if err = noErr then begin
				err := SetEOF(rref, GetHandleSize(thng));
				if err = noErr then begin
					err := FSWriteQ(rref, GetHandleSize(thng), thng^);
				end; (* if *)
				junk := FSClose(rref);
			end; (* if *)
			HPurge(thng);
		end; (* if *)
		UpdateInstalledVersion(fss); { in case we are installing or saving to the init }
		SaveComponentToFile := err;
	end; (* SaveComponentToFile *)

	function RegisterFile (fss: FSSpec): OSErr;
		var
			ref: integer;
			err, err2, junk: OSErr;
			result: longint;
	begin
		ref := HOpenResFile(fss.vRefNum, fss.parID, fss.name, fsRdPerm);
		err := ResError;
		if err = noErr then begin
			result := RegisterComponentResourceFile(ref, registerCmpGlobal);
			if result > 0 then begin
				err := noErr;
			end
			else begin
				err := result;
			end; (* if *)
			CloseResFile(ref);
			err2 := ResError;
			if err = noErr then begin
				err := err2;
			end; (* if *)
		end; (* if *)
		RegisterFile := err;
	end; (* RegisterFile *)

	function SaveICComponent: OSErr;
		var
			junklong: longint;
			err: OSErr;
			fss: FSSpec;
	begin
		err := ICStandardPutFile(GetAString(128, 13), GetAString(128, 12), fss);
		if err = noErr then begin
			err := SaveComponentToFile(fss);
		end;
		SaveICComponent := err;
	end; (* SaveICComponent *)

	function JustInstallICComponent: OSErr;
		var
			err: OSErr;
			where: FSSpec;
	begin
		UpdateInstalledVersion(where);
		err := SaveComponentToFile(where);
		if err = noErr then begin
			if has_components then begin
				err := RegisterFile(where);
			end;
		end; (* if *)
		JustInstallICComponent := err;
	end;

	function InstallICComponent: OSErr;
		var
			err: OSErr;
			where: FSSpec;
			prompt: Str255;
			desc: Str255;
			ndx: integer;
			junk: integer;
			registered_version: longInt;
	begin
		err := noErr;
		registered_version := 0;
		UpdateInstalledVersion(where);
		if (installed_version > 0) then begin
			if installed_version < application_version then begin
				prompt := GetAString(128, 17);
			end
			else if installed_version < application_version then begin
				prompt := GetAString(128, 18);
			end
			else if installed_version = application_version then begin
				prompt := GetAString(128, 19);
			end; (* if *)
			ParamText(prompt, '', '', '');
			InitCursor;
			if CautionAlert(145, @CancelModalFilter) <> ok then begin
				err := userCanceledErr;
			end; (* if *)
		end; (* if *)
		if err = noErr then begin
			err := JustInstallICComponent;
		end; (* if *)
		if err = noErr then begin
			InitCursor;
			ParamText(GetAString(128, 20), '', '', '');
			junk := NoteAlert(142, nil);
		end; (* if *)
		InstallICComponent := err;
	end; (* InstallICComponent *)

	function RemoveICComponent: OSErr;
		var
			err: OSErr;
			fss: FSSpec;
			junk: integer;
	begin
		err := noErr;
		UpdateInstalledVersion(fss);
		if installed_version > 0 then begin
			err := HDelete(fss.vRefNum, fss.parID, fss.name);
		end;
		if err = noErr then begin
			if GetRegisteredVersion > 0 then begin
				junk := NoteAlert(144, nil);
			end;
		end;
		UpdateInstalledVersion(fss);
		RemoveICComponent := err;
	end;

	procedure AdjustInstalMenu (menu: integer);
		var
			fss: FSSpec;
	begin
		if has_components then begin
			SetItemEnable(GetMHandle(menu), IM_Install, installed_version <> application_version);
		end
		else begin
			SetItemEnable(GetMHandle(menu), IM_Install, false);
		end;
		SetItemEnable(GetMHandle(menu), IM_Remove, installed_version > 0);
	end;

	procedure DoInstallMenu (menu, item: integer);
	begin
		case item of
			IM_Install: 
				DisplayError(acInstallComponent, InstallICComponent);
			IM_Save: 
				DisplayError(acInstallComponent, SaveICComponent);
			IM_Remove: 
				DisplayError(acRemoveComponent, RemoveICComponent);
			otherwise
				;
		end; (* case *)
	end;

	function CheckICUsageVersion: OSErr;
		var
			err, err2: OSErr;
			inst: ICInstance;
			component_instance: ComponentInstance;
	begin
		err := ICMapErr(ICStart(inst, ICcreator));
		if err = noErr then begin
			err := ICMapErr(ICGetComponentInstance(inst, component_instance));
			if err = noErr then begin
				if BAND(GetComponentVersion(component_instance), $FFFF0000) < BAND(internetConfigurationComponentInterfaceVersion, $FFFF0000) then begin
					err := unimpErr;
				end;
			end
			else begin
				err := noErr; { we work fine without a component, we just can't deal with an old component }
			end;
			err2 := ICMapErr(ICStop(inst));
			if err = noErr then begin
				err := err2;
			end;
		end;
		CheckICUsageVersion := err;
	end;

	function InitializeComponentInstallation: OSErr;
		var
			fss: FSSpec;
			a: integer;
			err, err2: OSErr;
			inst: ICInstance;
			component_instance: ComponentInstance;
	begin
		err := noErr;
		application_version := app_version.numericVersion.version;
		UpdateInstalledVersion(fss);
		if has_components then begin
			if installed_version < application_version then begin
				InitCursor;
				if installed_version > 0 then begin
					a := NoteAlert(146, @CancelModalFilter);
					if a <> ok then begin
						err := userCanceledErr;
					end;
				end
				else begin
					ParamText(GetAString(128, 8), '', '', '');
					a := NoteAlert(141, @CancelModalFilter);
				end;
				if a = ok then begin
					err := JustInstallICComponent;
					DisplayError(acInstallComponent, err);
					if err = noErr then begin
						err := CheckICUsageVersion;
						if err = noErr then begin
							InitCursor;
							ParamText(GetAString(128, 20), '', '', '');
							a := NoteAlert(142, nil);
						end
						else begin
							InitCursor;
							ParamText(GetAString(128, 26), '', '', '');
							a := NoteAlert(142, nil);
							err := userCanceledErr;
						end;
					end;
				end;
			end;
		end;
{ Ensure invariant that we are using the glue, or we are using the current version of the component }
		if err = noErr then begin
			err := CheckICUsageVersion;
		end;
		InitializeComponentInstallation := err;
	end;

end. (* ICInstall *)
temp_instance: ComponentInstance;
temp_instance := OpenDefaultComponent(internetConfigurationComponentType, internetConfigurationComponentSubType);
if temp_instance <> nil then begin
	junk := CloseComponent(temp_instance);
end;
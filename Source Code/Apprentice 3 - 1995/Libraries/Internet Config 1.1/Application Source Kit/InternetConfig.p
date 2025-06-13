program InternetConfig;

	uses
		RequiredEventSupport,{}
		ICGlobals, ICDocument, ICEvents, ICWindows, ICInstall, ICIconSuites, ICMiscSubs, ICEditPrefAppleEvents;

	procedure HandleAppParams;
		var
			ap_file: AppFile;
			fss: FSSpec;
			msg, count: integer;
			junk: OSErr;
			err: OSErr;
			junklong: longint;
	begin
		if has_AppleEvents then begin
			junk := InitAppleEvents(@DoOpenApp, @DoOpenDoc, nil, @DoQuit);
		end
		else begin
			CountAppFiles(msg, count);
			if count <= 0 then begin
				DisplayError(acNewDocument, DoOpenApp);
			end
			else begin
				GetAppFiles(1, ap_file);
				ClrAppFiles(1);
				if GetWDInfo(ap_file.vRefNum, fss.vRefNum, fss.parID, junklong) = noErr then begin
					fss.name := ap_file.fName;
					DisplayError(acOpenDocument, DoOpenDoc(fss));
				end; (* if *)
			end; (* if *)
		end; (* if *)
	end; (* HandleAppParams *)

	var
		mbar: Handle;
		err: OSErr;
		junk: OSErr;
begin
	mbar := GetNewMBar(128);
	if mbar = nil then begin
		ExitToShell;
	end; (* if *)
	SetMenuBar(mbar);
	AddResMenu(GetMHandle(M_Apple), 'DRVR');
	DrawMenuBar;
	InitGlobals;
	InitMiscSubs;
	InitICIconSuites;
	err := InitICWindows;
	if err = noErr then begin
		err := InitializeComponentInstallation;
	end; (* if *)
	if err = noErr then begin
		err := InitICDocument;
	end; (* if *)
	if err <> noErr then begin
		DisplayError(acStartApplication, err);
		ExitToShell;
	end; (* if *)
	HandleAppParams;
	SetupEditPrefAppleEvent;
	InitCursor;
	while not quitnow do begin
		HandleEvents;
	end; (* while *)
	TermICDocument;
end. (* InternetConfig *)
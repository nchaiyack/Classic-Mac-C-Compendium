unit ICEditPrefAppleEvents;

interface

	const
		EPSuite = 'ICAp';
		EPEvent = 'ICAp';
{ Direct parameter is key }
		EPDestination = 'dest'; { pass true if you want the default preference file.  defaults to current pref file (or default pref file if none)  }

	procedure SetupEditPrefAppleEvent;

implementation

	uses
		AppleEvents,{}
		ICGlobals, ICDocument;

{$PUSH}
{$R-}
	function HandleEditPrefAE (event, reply: AppleEvent; ignored: longInt): OSErr;
		var
			key: Str255;
			typ: DescType;
			actualSize: Size;
			fs: FSSpec;
			usefss, usecurrent: boolean;
			err: OSErr;
	begin
		usefss := true;
		usecurrent := false;
		err := AEGetParamPtr(event, EPDestination, typeFSS, typ, @fs, sizeof(fs), actualSize);
		if err <> noErr then begin
			usefss := false;
			usecurrent := true;
			if AEGetParamPtr(event, EPDestination, typeBoolean, typ, @fs, sizeof(fs), actualSize) = noErr then begin
				usecurrent := false;
			end;
		end;
		err := AEGetParamPtr(event, keyDirectObject, typeChar, typ, @key[1], 255, actualSize);
		if err <> noErr then begin
			actualSize := 0;
		end;
		key[0] := chr(actualSize);
		err := EditPreference(key, fs, usefss, usecurrent);
		HandleEditPrefAE := err;
	end;
{$POP}

	procedure SetupEditPrefAppleEvent;
		var
			junk: OSErr;
	begin
		if has_AppleEvents then begin
			junk := AEInstallEventHandler(EPSuite, EPEvent, @HandleEditPrefAE, 0, false);
		end;
	end;

end.
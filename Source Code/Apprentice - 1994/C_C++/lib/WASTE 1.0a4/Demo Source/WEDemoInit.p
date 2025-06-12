unit DemoInit;

{ WASTE DEMO PROJECT: }
{ Initialization & Finalization Routines }

{ Copyright © 1993-1994 Merzwaren }
{ All Rights Reserved }

interface
	uses
		DemoIntf;

	function Initialize: OSErr;
	procedure Finalize;

implementation
	uses
		DemoEvents, DemoMenus, DemoWindows;

	function Initialize: OSErr;

		const
			kMinSystemVersion = $700;
			kScrapThreshold = 4 * 1024;

		var
			response: LongInt;
			scrapResult: LongInt;

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					Initialize := err;
					ErrorAlert(err);
					Exit(Initialize);
				end;
		end;  { CheckErr }

	begin
		Initialize := noErr;

{ expand the zone to its maximum size }
		MaxApplZone;

{ allocate some extra master pointer blocks }
		MoreMasters;
		MoreMasters;
		MoreMasters;
		MoreMasters;

{ initialize the Toolbox }
		InitGraf(@thePort);
		InitFonts;
		InitWindows;
		InitMenus;
		TEInit;
		InitDialogs(nil);
		InitCursor;
		FlushEvents(everyEvent, 0);

{ if desk scrap is too large, unload it }
		if (InfoScrap^.scrapSize > kScrapThreshold) then
			scrapResult := UnloadScrap;

{ make sure system software version is 7.0 or newer }
		if (Gestalt(gestaltSystemVersion, response) <> noErr) | (response < kMinSystemVersion) then
			begin
				response := Alert(kAlertNeedSys7, nil);
				Initialize := -1;
				Exit(Initialize);
			end;

{ determine whether Color QuickDraw is available }
		gHasColorQD := (Gestalt(gestaltQuickDrawVersion, response) = noErr) & (response >= gestalt8BitQD);

{ determine whether the Text Services Manager is available }
		gHasTextServices := (Gestalt(gestaltTSMgrVersion, response) = noErr);

{ register this application with the TSM }
		if (gHasTextServices) then
			CheckErr(InitTSMAwareApplication);

{ perform other initialization chores }
		CheckErr(InitializeEvents);
		CheckErr(InitializeMenus);

	end;  { Initialize }

	procedure Finalize;
		var
			window: WindowPtr;
			err: OSErr;
	begin

{ close all windows }
		repeat
			window := FrontWindow;
			if (window <> nil) then
				DestroyWindow(window);
		until (window = nil);

{ notify text services that we're closing down }
		if (gHasTextServices) then
			err := CloseTSMAwareApplication;

	end;  { Finalize }

end.
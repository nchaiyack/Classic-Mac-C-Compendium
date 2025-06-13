unit WEDemoInit;

{ WASTE DEMO PROJECT: }
{ Initialization & Finalization Routines }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WEDemoIntf;

	function Initialize: OSErr;
	procedure Finalize;

implementation
	uses
		Dialogs, Fonts, GestaltEqu, Scrap, TextServices, WEDemoEvents, WEDemoMenus, WEDemoWindows, WEDemoDrags, WEDemoPictures, WEDemoSounds;

	var

{ static variables }

		sMyTrackingHandler: DragTrackingHandlerUPP;
		sMyReceiveHandler: DragReceiveHandlerUPP;

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
		MoreMasters;
		MoreMasters;

{ initialize the Toolbox }
{$IFC NOT UNDEFINED THINK_PASCAL}
		InitGraf(@thePort);
{$ELSEC}
		InitGraf(@qd.thePort);
{$ENDC}
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
{$IFC NOT UNDEFINED THINK_PASCAL}
				SetCursor(arrow);
{$ELSEC}
				SetCursor(qd.arrow);
{$ENDC}
				response := Alert(kAlertNeedSys7, nil);
				Initialize := -1;
				Exit(Initialize);
			end;

{ determine whether Color QuickDraw is available }
		gHasColorQD := (Gestalt(gestaltQuickDrawVersion, response) = noErr) & (response >= gestalt8BitQD);

{ determine whether the Drag Manager is available }
		gHasDragAndDrop := (Gestalt(gestaltDragMgrAttr, response) = noErr) & BTST(response, gestaltDragMgrPresent);

{ determine whether the Text Services Manager is available }
		gHasTextServices := (Gestalt(gestaltTSMgrVersion, response) = noErr);

{ register this application with the TSM }
		if (gHasTextServices) then
			CheckErr(InitTSMAwareApplication);

{ install default drag handlers }
		if (gHasDragAndDrop) then
			begin
				sMyTrackingHandler := NewDragTrackingHandlerProc(@MyTrackingHandler);
				sMyReceiveHandler := NewDragReceiveHandlerProc(@MyReceiveHandler);
				CheckErr(InstallTrackingHandler(sMyTrackingHandler, nil, nil));
				CheckErr(InstallReceiveHandler(sMyReceiveHandler, nil, nil));
			end;

{ install WASTE handlers for PICT objects }
		CheckErr(WEInstallObjectHandler(kTypePicture, weNewHandler, NewWENewObjectProc(@HandleNewPicture), nil));
		CheckErr(WEInstallObjectHandler(kTypePicture, weDisposeHandler, NewWEDisposeObjectProc(@HandleDisposePicture), nil));
		CheckErr(WEInstallObjectHandler(kTypePicture, weDrawHandler, NewWEDrawObjectProc(@HandleDrawPicture), nil));

{ install WASTE handlers for snd objects }
		CheckErr(WEInstallObjectHandler(kTypeSound, weNewHandler, NewWENewObjectProc(@HandleNewSound), nil));
		CheckErr(WEInstallObjectHandler(kTypeSound, weDrawHandler, NewWEDrawObjectProc(@HandleDrawSound), nil));
		CheckErr(WEInstallObjectHandler(kTypeSound, weClickHandler, NewWEClickObjectProc(@HandleClickSound), nil));

{ perform other initialization chores }
		CheckErr(InitializeEvents);
		CheckErr(InitializeMenus);

	end;  { Initialize }

	procedure Finalize;
	begin

{ remove drag handlers }
		if (gHasDragAndDrop) then
			begin
				if (RemoveTrackingHandler(sMyTrackingHandler, nil) <> noErr) then
					;
				if (RemoveReceiveHandler(sMyReceiveHandler, nil) <> noErr) then
					;
			end;

{ notify text services that we're closing down }
		if (gHasTextServices) then
			if (CloseTSMAwareApplication <> noErr) then
				;
	end;  { Finalize }

end.
unit WEDemoEvents;

{ WASTE DEMO PROJECT: }
{ Events Handling }

{ Copyright � 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WEDemoIntf;

	function InitializeEvents: OSErr;
	procedure DoWindowEvent ({const} var event: EventRecord);
	procedure ProcessEvent;

implementation
	uses
		AppleEvents, AERegistry, DiskInit, TextServices, WEDemoMenus, WEDemoScripting, WEDemoWindows, Segments;

	const

{ possible values for HandleOpenDocument refCon parameter }

		kDoOpen = 0;
		kDoPrint = 1;

	var

		gSleepTime: LongInt;					{ sleep time for WaitNextEvent }
		gMouseRgn: RgnHandle;				{ mouse region for WaitNextEvent }

	procedure AdjustCursor (mouseLoc: Point;
									mouseRgn: RgnHandle);
		var
			window: WindowPtr;
	begin

{ by default, set mouseRgn to the whole QuickDraw coordinate plane, }
{ so that we never get mouse moved events }
		SetRectRgn(mouseRgn, -maxint, -maxint, maxint, maxint);

{ give text services a chance to set the cursor shape }
		if (gHasTextServices) then
			if (SetTSMCursor(mouseLoc)) then
				Exit(AdjustCursor);

{ if there is a window open, give WEAdjustCursor an opportunity to set the cursor }
{ WEAdjustCursor intersects mouseRgn (if supplied) with a region within which }
{ the cursor is to retain its shape. }
{ (if the cursor is outside the view region, this is subtracted from mouseRgn) }

		window := FrontWindow;
		if (window <> nil) then
			if (WEAdjustCursor(mouseLoc, mouseRgn, DocumentPeek(window)^.hWE)) then
				Exit(AdjustCursor);

{ set the cursor to the arrow cursor }
{$IFC NOT UNDEFINED THINK_PASCAL}
		SetCursor(arrow);
{$ELSEC}
		SetCursor(qd.arrow);
{$ENDC}

	end;  { AdjustCursor }

	procedure DoMouseDown ({const} var event: EventRecord);
		var
			window: WindowPtr;
			partCode: Integer;
	begin

{ find out where this click went down in }
		partCode := FindWindow(event.where, window);

{ dispatch on partCode }
		case partCode of

			inMenuBar: 
				begin
					PrepareMenus;
					DoMenuChoice(MenuSelect(event.where));
				end;

			inSysWindow: 
				SystemClick(event, window);

			inContent: 
				if (DoContent(event.where, event, window)) then
					SelectWindow(window);

			inDrag: 
				DoDrag(event.where, window);

			inGrow: 
				DoGrow(event.where, window);

			inGoAway: 
				if (TrackGoAway(window, event.where)) then
					if (DoClose(closingWindow, savingAsk, window) <> noErr) then
						;

			inZoomIn, inZoomOut: 
				if (TrackBox(window, event.where, partCode)) then
					DoZoom(partCode, window);

			otherwise
				;
		end;  { case partCode }
	end;  { DoMouseDown }

	procedure DoKeyDown ({const} var event: EventRecord);
		const

{ virtual key codes generated by some function keys }
			keyF1 = $7A;
			keyF2 = $78;
			keyF3 = $63;
			keyF4 = $76;
		var
			key: Char;
	begin

{ extract character code from event message }
		key := Char(BAND(event.message, charCodeMask));

{ map function keys to the equivalent command+key combos }
{ note that all function keys generate the same character code, i.e. $10 }
		if (key = CHR($10)) then
			begin
				event.modifiers := BOR(event.modifiers, cmdKey);
				case BSR(BAND(event.message, keyCodeMask), 8) of

					keyF1: 
						key := 'z';

					keyF2: 
						key := 'x';

					keyF3: 
						key := 'c';

					keyF4: 
						key := 'v';

					otherwise
						key := CHR(0);
				end;  { case }
			end;

{ command + printable character combos are routed to MenuKey }
		if (BAND(event.modifiers, cmdKey) <> 0) and (key >= CHR(32)) then
			begin
				PrepareMenus;
				DoMenuChoice(MenuKey(key));
			end
		else
			DoKey(key, event);
	end;  { DoKeyDown }

	procedure DoDiskEvent ({const} var event: EventRecord);
		var
			dialogCorner: Point;
			err: OSErr;
	begin
		if (BSR(event.message, 16) <> noErr) then
			begin
				SetPt(dialogCorner, 112, 80);
				err := DIBadMount(dialogCorner, event.message);
			end;
	end;  { DoDiskEvent }

	procedure DoOSEvent ({const} var event: EventRecord);
		var
			osMessage: Integer;
			window: WindowPtr;
	begin

{ extract the OS message field from the event record }
		osMessage := BSR(BAND(event.message, osEvtMessageMask), 24);

{ dispatch on osMessage }
		case osMessage of

			suspendResumeMessage: 
				begin
					window := FrontWindow;
					if (window <> nil) then
						DoActivate(BAND(event.message, resumeFlag) <> 0, window);
				end;

			mouseMovedMessage: 
				;

			otherwise
				;
		end;  { case }
	end;  { DoOSEvent }

	procedure DoHighLevelEvent ({const} var event: EventRecord);
	begin
		if (AEProcessAppleEvent(event) <> noErr) then
			;
	end;  { DoHighLevelEvent }

	procedure DoNullEvent ({const} var event: EventRecord);
		var
			window: WindowPtr;
	begin
		window := FrontWindow;
		if (window <> nil) then
			WEIdle(gSleepTime, DocumentPeek(window)^.hWE)
		else
			gSleepTime := maxLongInt;

{ unload (= unlock & mark as purgeable) non-persistent code segments }
		UnloadNonPersistentSegments;

	end;  { DoNullEvent }

	procedure DoWindowEvent ({const} var event: EventRecord);
		var
			window: WindowPtr;
	begin

{ the message field of the event record contains the window pointer }
		window := WindowPtr(event.message);

{ make sure this window is an application window; check the windowKind field }
		if (WindowPeek(window)^.windowKind <> userKind) then
			Exit(DoWindowEvent);

		case event.what of

			updateEvt: 
				DoUpdate(window);

			activateEvt: 
				DoActivate(BAND(event.modifiers, activeFlag) <> 0, window);

		end;  { case }
	end;  { DoWindowEvent }

	procedure ProcessEvent;
		const
			kHighLevelEvent = 23;		{ copied from "EPPC.p" }
		var
			event: EventRecord;
			gotEvent: Boolean;
	begin

		gotEvent := WaitNextEvent(everyEvent, event, gSleepTime, gMouseRgn);

{ give text services a chance to intercept this event }
		if (gHasTextServices) then
			if TSMEvent(event) then
				;

{ adjust cursor shape and set mouse region }
{ (we assume event.where is the current mouse position in global coordinates }
{ if event.what <= osEvt; high-level events store the event ID there) }

		if (event.what <= osEvt) then
			AdjustCursor(event.where, gMouseRgn);

{ dispatch on event.what }
		case event.what of

			nullEvent: 
				DoNullEvent(event);

			mouseDown: 
				DoMouseDown(event);

			keyDown, autoKey: 
				DoKeyDown(event);

			updateEvt, activateEvt: 
				DoWindowEvent(event);

			diskEvt: 
				DoDiskEvent(event);

			osEvt: 
				DoOSEvent(event);

			kHighLevelEvent: 
				DoHighLevelEvent(event);

			otherwise
				;											{ ignore other events }
		end;  { case }

		if (gotEvent) then
			gSleepTime := 0;						{ force early idle after non-idle event }

	end;  { ProcessEvent }

	function GotRequiredParams ({const} var ae: AppleEvent): OSErr;
		var
			returnedType: DescType;
			actualSize: Size;
			err: OSErr;
	begin
		err := AEGetAttributePtr(ae, keyMissedKeywordAttr, typeWildCard, returnedType, nil, 0, actualSize);
		if (err = errAEDescNotFound) then
			GotRequiredParams := noErr
		else if (err = noErr) then
			GotRequiredParams := errAEParamMissed
		else
			GotRequiredParams := err;
	end;  { GotRequiredParams }

	function HandleOpenDocument ({const} var ae, reply: AppleEvent;
									refCon: LongInt): OSErr;
		var
			docList: AEDescList;
			keyword: AEKeyword;
			returnedType: DescType;
			actualSize: Size;
			numberOfDocuments, i: LongInt;
			fileSpec: FSSpec;

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					HandleOpenDocument := err;
					err := AEDisposeDesc(docList);
					Exit(HandleOpenDocument);
				end;
		end;  { CheckErr }

	begin
		HandleOpenDocument := noErr;

{ extract direct parameter from the Apple Event }
		CheckErr(AEGetParamDesc(ae, keyDirectObject, typeAEList, docList));

{ perform the recommended check for additional required parameters }
		CheckErr(GotRequiredParams(ae));

{ count the items in the list of aliases }
		CheckErr(AECountItems(docList, numberOfDocuments));

		for i := 1 to numberOfDocuments do
			begin

{ coerce the nth alias to a file system specification record }
				CheckErr(AEGetNthPtr(docList, i, typeFSS, keyword, returnedType, @fileSpec, SizeOf(fileSpec), actualSize));

{ open the specified file }
				CheckErr(CreateWindow(@fileSpec));
			end;  { for }

{ dispose of the alias list }
		CheckErr(AEDisposeDesc(docList));

	end;  { HandleOpenDocument }

	function HandleOpenApplication ({const} var ae, reply: AppleEvent;
									refCon: LongInt): OSErr;
		var
			err: OSErr;
	begin

{ perform the recommended check for additional required parameters }
		err := GotRequiredParams(ae);
		if (err <> noErr) then
			begin
				HandleOpenApplication := err;
				Exit(HandleOpenApplication);
			end;

{ create a new window from scratch }
		HandleOpenApplication := CreateWindow(nil);

	end;  { HandleOpenApplication }

	function HandleQuitApplication ({const} var ae, reply: AppleEvent;
									refCon: LongInt): OSErr;
		var
			optKey: AEKeyword;
			actualType: DescType;
			actualSize: Size;
			saving: SavingOption;
			err: OSErr;
	begin

{ default saving option is savingAsk }
		saving := savingAsk;

{ extract the optional save options }
		err := AEGetParamPtr(ae, keyAESaveOptions, typeEnumerated, actualType, @optKey, SizeOf(optKey), actualSize);
		if (err = noErr) then
			begin
				if (optKey = kAEYes) then
					saving := savingYes
				else if (optKey = kAENo) then
					saving := savingNo
				else if (optKey <> kAEAsk) then
					begin
						HandleQuitApplication := paramErr;		{ for want of a better code }
						Exit(HandleQuitApplication);
					end;
			end;

{ perform the recommended check for additional required parameters }
		err := GotRequiredParams(ae);
		if (err <> noErr) then
			begin
				HandleQuitApplication := err;
				Exit(HandleQuitApplication);
			end;

		HandleQuitApplication := DoQuit(saving);
	end;  { HandleQuitApplication }

{ THINK Pascal compiler directive: put the following code in the "Init" segment }
{$S Init}

	function InitializeEvents: OSErr;

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					InitializeEvents := err;
					Exit(InitializeEvents);
				end;
		end;  { CheckErr }

	begin
		InitializeEvents := noErr;

{ allocate space for the mouse region }
		gMouseRgn := NewRgn;

{ install Apple event handlers for the Required Suite }
		CheckErr(AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerProc(@HandleOpenApplication), 0, false));
		CheckErr(AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerProc(@HandleOpenDocument), kDoOpen, false));
		CheckErr(AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerProc(@HandleOpenDocument), kDoPrint, false));
		CheckErr(AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerProc(@HandleQuitApplication), 0, false));

{ install Apple event handlers for a subset of the Core Suite }
		CheckErr(InstallCoreHandlers);

{ install Apple event handlers for inline input }
		CheckErr(WEInstallTSMHandlers);

	end;  { InitializeEvents }

end.
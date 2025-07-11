unit DemoWindows;

{ WASTE DEMO PROJECT: }
{ Window Handling }

{ Copyright � 1993-1994 Merzwaren }
{ All Rights Reserved }

interface
	uses
		DemoIntf;

	procedure DoDrag (hitPt: Point;
									window: WindowPtr);
	procedure DoGrow (hitPt: Point;
									window: WindowPtr);
	procedure DoZoom (partCode: Integer;
									window: WindowPtr);
	procedure DoContent (hitPt: Point;
									var event: EventRecord;
									window: WindowPtr);
	procedure DoKey (key: Char;
									var event: EventRecord);
	procedure DoUpdate (window: WindowPtr);
	procedure DoActivate (activFlag: Boolean;
									window: WindowPtr);
	function CreateWindow (pFileSpec: FSSpecPtr): OSErr;
	procedure DestroyWindow (window: WindowPtr);

implementation
	uses
		LongControls, DemoFiles;

	var

{ static variables }

		sScrollStep: Integer;					{ how many pixels to scroll (used by ScrollProc) }

	procedure MyDrawGrowIcon (wind: GrafPtr;
									validate: Boolean);
{ the standard Toolbox trap _DrawGrowIcon draws two lines from the grow icon }
{ to the left and top margins of the window's content area }
{ these additional lines may create ugly dirt, so we use this routine to temporarily }
{ set the clip region to the grow icon rect }
{ in addition, if validate is TRUE, we call _ValidRect on the icon rect }
		var
			savePort: GrafPtr;
			saveClip: RgnHandle;
			r: Rect;
	begin

{ save port and set thePort to wind }
		GetPort(savePort);
		SetPort(wind);

{ save the clip region }
		saveClip := NewRgn;
		GetClip(saveClip);

{ calculate grow icon rect }
		r.botRight := wind^.portRect.botRight;
		r.top := r.bottom - (kBarWidth - 2);
		r.left := r.right - (kBarWidth - 2);

{ set clip region to grow icon rect }
		ClipRect(r);

{ call _DrawGrowIcon }
		DrawGrowIcon(wind);

{ if validate is TRUE, remove the grow icon rect from the update region }
		if (validate) then
			ValidRect(r);

{ restore old clip region }
		SetClip(saveClip);
		DisposeRgn(saveClip);

{ restore old port }
		SetPort(savePort);

	end;  { MyDrawGrowIcon }

	procedure ScrollBarChanged (window: WindowPtr);
{ scroll text to reflect new scroll bar setting }
		var
			hWE: WEHandle;
			viewRect, destRect: LongRect;
	begin
		hWE := DocumentPeek(window)^.hWE;
		WEGetViewRect(viewRect, hWE);
		WEGetDestRect(destRect, hWE);
		WEScroll(viewRect.left - destRect.left - LCGetValue(DocumentPeek(window)^.scrollBars.h), viewRect.top - destRect.top - LCGetValue(DocumentPeek(window)^.scrollBars.v), hWE);
	end;  { ScrollBarChanged }

	procedure AdjustBars (window: WindowPtr);
{ recalculate scroll bar settings based on the text total height and destination rectangle }
		var
			hWE: WEHandle;
			savePort: GrafPtr;
			viewRect, destRect: LongRect;
			value: LongInt;
			max: LongInt;
			bar: ControlHandle;
			hilite: Integer;
			axis: VHSelect;
	begin

		GetPort(savePort);
		SetPort(window);

{ get view and destination rectangle }
		hWE := DocumentPeek(window)^.hWE;
		WEGetViewRect(viewRect, hWE);
		WEGetDestRect(destRect, hWE);

{ do for each axis }
		for axis := v to h do
			begin

{ get scroll bar handle }
				bar := DocumentPeek(window)^.scrollBars.vh[axis];

{ calculate new scroll bar settings }

{ NOTE: (destRect.bottom - destRect.top) always equals the total text height because }
{ WASTE automatically updates destRect.bottom whenever line breaks are recalculated }

				value := viewRect.topLeft.vh[axis] - destRect.topLeft.vh[axis];
				max := value + (destRect.botRight.vh[axis] - viewRect.botRight.vh[axis]);

{ calculate new highlight state }
				if (max <= 0) then
					begin
						max := 0;
						hilite := kCtlInactive;
					end
				else
					hilite := kCtlActive;

{ reset the scroll bar }
				LCSetMax(bar, max);
				LCSetValue(bar, value);
				HiliteControl(bar, hilite);

{ if value exceeds max then the bottom of the destination rectangle is above }
{ the bottom of the view rectangle: we need to scroll the text downward }
{ (this happens only for the vertical scroll bar, if you think about it) }
				if (value > max) then
					ScrollBarChanged(window);

			end;  { for axis }

		SetPort(savePort);

	end;  { AdjustBars }

	procedure ViewChanged (window: WindowPtr);
{ Fix its scroll bars and WE view rect when the window is created, }
{ or after it is resized or zoomed, or when the page is adjusted }
		var
			savePort: GrafPtr;
			bar: ControlHandle;
			barRects: array[VHSelect] of Rect;
			r: Rect;
			viewRect: LongRect;
			axis: VHSelect;
	begin

		GetPort(savePort);
		SetPort(window);

{ recalculate the correct rectangles for the text area and the scroll bars, }
{ based on the window's port rect }
		with window^.portRect do
			begin
				SetRect(barRects[v], right - (kBarWidth - 1), -1, right + 1, bottom - (kBarWidth - 2));
				SetRect(barRects[h], -1, bottom - (kBarWidth - 1), right - (kBarWidth - 2), bottom + 1);
				SetRect(r, 0, 0, right - (kBarWidth - 1), bottom - (kBarWidth - 1));
				InsetRect(r, kTextMargin, kTextMargin);
				WERectToLongRect(r, viewRect);
			end;

{ resize the text area }
		WESetViewRect(viewRect, DocumentPeek(window)^.hWE);

{ move and resize the control bars }
		for axis := v to h do
			begin
				bar := DocumentPeek(window)^.scrollBars.vh[axis];
				r := barRects[axis];
				MoveControl(bar, r.left, r.top);
				SizeControl(bar, r.right - r.left, r.bottom - r.top);
				ValidRect(r);
			end;  { for axis }

{ reset thumb positions and the max values of the control bars }
		AdjustBars(window);

{ redraw the control bars }
		ShowControl(DocumentPeek(window)^.scrollBars.v);
		ShowControl(DocumentPeek(window)^.scrollBars.h);

		SetPort(savePort);

	end;  { ViewChanged }

	procedure DoDrag (hitPt: Point;
									window: WindowPtr);
		var
			dragRect: Rect;
	begin
		dragRect := GetGrayRgn^^.rgnBBox;
		DragWindow(window, hitPt, dragRect);
	end;  { DoDrag }

	procedure Resize (newSize: Point;
									window: WindowPtr);
		var
			savePort: GrafPtr;
			viewRect: LongRect;
			r: Rect;
			tempRgn, dirtyRgn: RgnHandle;
	begin

		GetPort(savePort);
		SetPort(window);

{ create temporary regions for calculations }
		tempRgn := NewRgn;
		dirtyRgn := NewRgn;

{ save old text region }
		WEGetViewRect(viewRect, DocumentPeek(window)^.hWE);
		WELongRectToRect(viewRect, r);
		RectRgn(tempRgn, r);

{ erase the old grow icon rect }
		r.botRight := window^.portRect.botRight;
		r.top := r.bottom - (kBarWidth - 2);
		r.left := r.right - (kBarWidth - 2);
		EraseRect(r);

{ hide the scroll bars }
		HideControl(DocumentPeek(window)^.scrollBars.v);
		HideControl(DocumentPeek(window)^.scrollBars.h);

{ perform the actual resizing of window, redraw scroll bars and grow icon}
		SizeWindow(window, newSize.h, newSize.v, false);
		ViewChanged(window);
		MyDrawGrowIcon(window, true);

{ calculate the dirty region (to be updated) }
		WEGetViewRect(viewRect, DocumentPeek(window)^.hWE);
		WELongRectToRect(viewRect, r);
		RectRgn(dirtyRgn, r);
		XOrRgn(dirtyRgn, tempRgn, dirtyRgn);
		with window^.portRect do
			SetRectRgn(tempRgn, left, top, right - (kBarWidth - 1), bottom - (kBarWidth - 1));
		SectRgn(dirtyRgn, tempRgn, dirtyRgn);

{ mark the dirty region as invalid }
		InvalRgn(dirtyRgn);

{ throw away temporary regions }
		DisposeRgn(tempRgn);
		DisposeRgn(dirtyRgn);

		SetPort(savePort);

	end;  { Resize }

	procedure DoGrow (hitPt: Point;
									window: WindowPtr);
		const
			kMinWindowWidth = 200;
			kMinWindowHeight = 80;
		var
			sizeRect: Rect;
			newSize: LongInt;
	begin
		SetRect(sizeRect, kMinWindowWidth, kMinWindowHeight, maxint, maxint);
		newSize := GrowWindow(window, hitPt, sizeRect);
		if (newSize <> 0) then
			Resize(Point(newSize), window);
	end;  { DoGrow }

	procedure DoZoom (partCode: Integer;
									window: WindowPtr);
		var
			savePort: GrafPtr;
			viewRect: LongRect;
			textRect: Rect;
	begin

		GetPort(savePort);
		SetPort(window);

		EraseRect(window^.portRect);
		HideControl(DocumentPeek(window)^.scrollBars.v);
		HideControl(DocumentPeek(window)^.scrollBars.h);
		ZoomWindow(window, partCode, false);
		ViewChanged(window);
		WEGetViewRect(viewRect, DocumentPeek(window)^.hWE);
		WELongRectToRect(viewRect, textRect);
		InvalRect(textRect);

		SetPort(savePort);

	end;  { DoZoom }

	procedure ScrollProc (bar: ControlHandle;
									partCode: Integer);

{ this is a callback routine called by the Toolbox Control Manager }
{ move the scroll bar thumb and scroll the text accordingly }

		var
			value, step: LongInt;
	begin

		if (partCode = 0) then
			Exit(ScrollProc);

		value := LCGetValue(bar);
		step := sScrollStep;

		if ((value < LCGetMax(bar)) and (step > 0)) or ((value > 0) and (step < 0)) then
			begin
				LCSetValue(bar, value + step);
				ScrollBarChanged(FrontWindow);
			end;
	end;  { ScrollProc }

	procedure DoScrollBar (hitPt: Point;
									modifiers: Integer;
									window: WindowPtr);
		var
			bar: ControlHandle;
			viewRect: LongRect;
			axis: VHSelect;
			partCode, step: Integer;
	begin

{ find out which scroll bar was hit (if any) and in which part }
		partCode := FindControl(hitPt, window, bar);

		if (bar <> nil) then

{ dispatch on partCode }
			if (partCode = inThumb) then
				begin

{ click in thumb: call TrackControl with no actionProc and adjust text }
					partCode := TrackControl(bar, hitPt, nil);
					LCSynch(bar);
					ScrollBarChanged(window);

				end
			else
				begin

{ click in a bar, but not in thumb }
{ which scroll bar was hit? }
					if (bar = DocumentPeek(window)^.scrollBars.v) then
						axis := v
					else
						axis := h;

{ get text view rectangle }
					WEGetViewRect(viewRect, DocumentPeek(window)^.hWE);

{ dispatch on partCode }
					case partCode of

						inUpButton: 
							if (BitAnd(modifiers, optionKey) = 0) then
								step := -kScrollDelta
							else
								step := -1;

						inDownButton: 
							if (BitAnd(modifiers, optionKey) = 0) then
								step := +kScrollDelta
							else
								step := +1;

						inPageUp: 
							step := -(viewRect.botRight.vh[axis] - viewRect.topLeft.vh[axis]) + kScrollDelta;

						inPageDown: 
							step := (viewRect.botRight.vh[axis] - viewRect.topLeft.vh[axis]) - kScrollDelta;

						otherwise
							step := 0;
					end;  { case }

{ save step in a static variable for our ScrollProc callback }
					sScrollStep := step;

{ track the mouse }
					partCode := TrackControl(bar, hitPt, @ScrollProc);

				end;
	end;  { DoScrollBar }

	function ClickLoop (hWE: WEHandle): Boolean;

{ this is a callback routine repeatedly called by WEClick while tracking the mouse }
{ if the mouse goes out of the text rect, scroll the text and adjust the scroll bars }

		var
			window: WindowPtr;
			textRect: LongRect;
			mouseLoc: Point;
			bar: ControlHandle;
			oldValue: LongInt;
			delta: Integer;
			axis: VHSelect;
	begin

{ return TRUE so WEClick keeps tracking the mouse }
		ClickLoop := true;

{ retrieve the window pointer stored in the WE instance as a "reference constant" }
		if (WEGetInfo(weRefCon, @window, hWE) <> noErr) then
			Exit(ClickLoop);

{ get text rect and mouse location (in local coords) }
		WEGetViewRect(textRect, hWE);
		GetMouse(mouseLoc);

		for axis := v to h do
			begin
				bar := DocumentPeek(window)^.scrollBars.vh[axis];
				oldValue := LCGetValue(bar);
				delta := mouseLoc.vh[axis] - textRect.botRight.vh[axis];
				if (delta > 0) then
					begin
						if (oldValue < LCGetMax(bar)) then
							LCSetValue(bar, oldValue + kScrollDelta);
					end
				else
					begin
						delta := textRect.topLeft.vh[axis] - mouseLoc.vh[axis];
						if (delta > 0) then
							begin
								if (oldValue > 0) then
									LCSetValue(bar, oldValue - kScrollDelta);
							end;
					end;
			end;  { for axis }

{ scroll the text to match current scroll bar settings }
		ScrollBarChanged(window);

	end;  { ClickLoop }

	procedure TextScrolled (hWE: WEHandle);

{ This is a callback routine called whenever the text is scrolled automatically. }
{ Since auto-scrolling is enabled, WEScroll may be invoked internally by WASTE }
{ in many different circumstances, and we want to be notified when this happens }
{ so we can adjust the scroll bars }

		var
			window: WindowPtr;
	begin

{ retrieve the window pointer stored in the WE instance as a "reference constant" }
		if (WEGetInfo(weRefCon, @window, hWE) <> noErr) then
			Exit(TextScrolled);

{ make sure scroll bars are in synch with the destination rectangle }
		AdjustBars(window);

	end;  { TextScrolled }

	procedure DoContent (hitPt: Point;
									var event: EventRecord;
									window: WindowPtr);
		var
			viewRect: LongRect;
			textRect: Rect;
			savePort: GrafPtr;
	begin

{ set the port to our window's port }
		GetPort(savePort);
		SetPort(window);

{ convert the point to local coordinates }
		GlobalToLocal(hitPt);

{ get view rectangle in short coordinates }
		WEGetViewRect(viewRect, DocumentPeek(window)^.hWE);
		WELongRectToRect(viewRect, textRect);

		if PtInRect(hitPt, textRect) then
			WEClick(hitPt, event.modifiers, event.when, DocumentPeek(window)^.hWE)
		else
			DoScrollBar(hitPt, event.modifiers, window);

{ restore the port }
		SetPort(savePort);

	end;  { DoContent }

	procedure DoScrollKey (keyCode: SignedByte;
									window: WindowPtr);
		var
			bar: ControlHandle;
			v: LongInt;
			viewRect: LongRect;
	begin
		bar := DocumentPeek(window)^.scrollBars.v;

{ do nothing if the scroll bar isn't active }
		if (bar^^.contrlHilite <> kCtlActive) then
			Exit(DoScrollKey);

{ get current scroll bar value }
		v := LCGetValue(bar);

{ get text view rect }
		WEGetViewRect(viewRect, DocumentPeek(window)^.hWE);

		case keyCode of

			keyPgUp: 
				v := v - (viewRect.bottom - viewRect.top) + kScrollDelta;

			keyPgDn: 
				v := v + (viewRect.bottom - viewRect.top) - kScrollDelta;

			keyHome: 
				v := 0;

			keyEnd: 
				v := maxLongInt;

			otherwise
				;
		end;  { case }

{ set the new scroll bar value and scroll the text pane accordingly }
		LCSetValue(bar, v);
		ScrollBarChanged(window);

	end;  { DoScrollKey }

	procedure DoKey (key: Char;
									var event: EventRecord);
		var
			window: WindowPtr;
			keyCode: SignedByte;
	begin
		window := FrontWindow;

{ do nothing if no window is active }
		if (window = nil) then
			Exit(DoKey);

{ extract virtual key code from event record }
		keyCode := BSR(BAND(event.message, keyCodeMask), 8);

{ page movement keys are handled by DoScrollKey }
		if (keyCode in [keyPgUp, keyPgDn, keyHome, keyEnd]) then
			DoScrollKey(keyCode, window)
		else
			WEKey(key, event.modifiers, DocumentPeek(window)^.hWE);

	end;  { DoKey }

	procedure DoUpdate (window: WindowPtr);
		var
			savePort: GrafPtr;
			updateRgn: RgnHandle;
	begin
		GetPort(savePort);
		SetPort(window);
		BeginUpdate(window);

{ BeginUpdate sets the window visRgn to the region to update }
		updateRgn := window^.visRgn;

{ erase update region }
		EraseRgn(updateRgn);

{ draw scroll bars }
		UpdateControls(window, updateRgn);

{ draw grow icon }
		MyDrawGrowIcon(window, false);

{ draw text }
		WEUpdate(updateRgn, DocumentPeek(window)^.hWE);

		EndUpdate(window);
		SetPort(savePort);

	end;  { DoUpdate }

	procedure DoActivate (activFlag: Boolean;
									window: WindowPtr);
		var
			menuID: Integer;
	begin
		MyDrawGrowIcon(window, true);
		if (activFlag) then
			begin
				SetPort(window);
				WEActivate(DocumentPeek(window)^.hWE);
				AdjustBars(window);
				for menuID := kMenuEdit to kMenuAlignment do
					EnableItem(GetMHandle(menuID), 0);
			end
		else
			begin
				WEDeactivate(DocumentPeek(window)^.hWE);
				HiliteControl(DocumentPeek(window)^.scrollBars.v, kCtlBackground);
				HiliteControl(DocumentPeek(window)^.scrollBars.h, kCtlBackground);
				for menuID := kMenuEdit to kMenuAlignment do
					DisableItem(GetMHandle(menuID), 0);
			end;
		InvalMenuBar;
	end;  { DoActivate }

	function CreateWindow (pFileSpec: FSSpecPtr): OSErr;
		var
			pWRecord: Ptr;
			window: WindowPtr;
			hWE: WEHandle;
			bar: ControlHandle;
			axis: VHSelect;
			fileInfo: FInfo;
			fileRefNum: Integer;
			textRect: Rect;
			longTextRect: LongRect;
			callback: ProcPtr;

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					CreateWindow := err;
					ErrorAlert(err);

{ here we should destroy partially allocated data structures to avoid memory leaks }
{ but, hey, this is just a demo, not a real-world application }
					Exit(CreateWindow);
				end;
		end;  { CheckErr }

	begin
		CreateWindow := noErr;

{ allocate a non-relocatable block to hold a document record }
		pWRecord := NewPtrClear(SizeOf(DocumentRecord));
		CheckErr(MemError);

{ create the window from a 'WIND' template; the window is initially invisible }
{ if Color QuickDraw is available, create a color window }
		if (gHasColorQD) then
			window := GetNewCWindow(kWindowTemplateID, pWRecord, WindowPtr(-1))
		else
			window := GetNewWindow(kWindowTemplateID, pWRecord, WindowPtr(-1));

{ make sure we got a window }
		if (window = nil) then
			begin
				DisposPtr(pWRecord);
				CheckErr(memFullErr);
			end;

{ calculate the text rectangle }
		textRect := window^.portRect;
		textRect.right := textRect.right - (kBarWidth - 1);
		textRect.bottom := textRect.bottom - (kBarWidth - 1);
		InsetRect(textRect, kTextMargin, kTextMargin);
		WERectToLongRect(textRect, longTextRect);

{ before calling WENew, set the port to gWindow, }
{ so the WE instance knows where to draw }
		SetPort(window);

{ create the WE instance, enabling certain features }
		CheckErr(WENew(longTextRect, longTextRect, weDoAutoScroll + weDoOutlineHilite + weDoUseTempMem + weDoDrawOffscreen, hWE));

{ save a reference to the window in the WE instance }
		CheckErr(WESetInfo(weRefCon, @window, hWE));

{ now the other way around: save the WE handle in the document record }
		DocumentPeek(window)^.hWE := hWE;

{ set up our callbacks }
		callback := @ClickLoop;
		CheckErr(WESetInfo(weClickLoop, @callback, hWE));
		callback := @TextScrolled;
		CheckErr(WESetInfo(weScrollProc, @callback, hWE));

		for axis := v to h do
			begin

{ create a scroll bar from a 'CNTL' template }
				bar := GetNewControl(kScrollBarTemplateID, window);
				if (bar = nil) then
					CheckErr(-1);

{ attach a LongControl record to the scroll bar; this allows us to use long }
{ settings and thus to scroll text taller than 32,767 pixels }
				CheckErr(LCAttach(bar));

{ save control handle in the document record }
				DocumentPeek(window)^.scrollBars.vh[axis] := bar;

			end;  { for axis }

{ ViewChanged adjusts the scroll bars rectangles to the window frame }
		ViewChanged(window);

{ if pFileSpec is not NIL, it points to a file to read }
		if (pFileSpec <> nil) then
			begin

{ turn the cursor into a wristwatch because this can be a lengthy operation }
				SetCursor(GetCursor(watchCursor)^^);

{ retrieve file information }
				CheckErr(FSpGetFInfo(pFileSpec^, fileInfo));

{ make sure we recognize the file type }
				if (fileInfo.fdType <> kTypeText) then
					CheckErr(-1);

{ read in the file }
				CheckErr(ReadTextFile(pFileSpec, hWE));

{ calculate line breaks }
				CheckErr(WECalText(hWE));

{ set the window title to the file name }
				SetWTitle(window, pFileSpec^.name);

			end;  { if pFileSpec <> NIL }

{ adjust scroll bar settings based on the total text height }
		AdjustBars(window);

{ show the document window }
		ShowWindow(window);

	end;  { CreateWindow }

	procedure DestroyWindow (window: WindowPtr);
		var
			menuID: Integer;
	begin

{ destroy the WE record }
		WEDispose(DocumentPeek(window)^.hWE);

{ destroy the LongControl records attached to the scroll bars }
		LCDetach(DocumentPeek(window)^.scrollBars.v);
		LCDetach(DocumentPeek(window)^.scrollBars.h);

{ destroy the window record and all associated data structures }
		DisposeWindow(window);

{ disable the text-editing menus }
		for menuID := kMenuFont to kMenuAlignment do
			DisableItem(GetMHandle(menuID), 0);
		InvalMenuBar;

	end;  { DestroyWindow }

end.
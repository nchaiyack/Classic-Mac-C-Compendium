unit ICButtonWhat;

interface

	uses
		ICWindowGlobals;

	function WhatOpenButton (wt: WindowType; item: integer): OSErr;
	function WhatClickButton (wt: WindowType; item: integer; er: eventRecord): OSErr;

implementation

	uses
		Traps, IconFamilies, ICStrH, OSSubs, 

		ICKeys, ICAPI, ICSubs, ICDialogs, ICMiscSubs, ICSubs, ICDocUtils, ICGlobals, ICWindowUtils;

	procedure HackDeviceLoop (drawingRgn: RgnHandle; drawingProc: DeviceLoopDrawingProcPtr; userData: LONGINT; flags: longint);
	inline
		$ABCA;

	procedure GetIconRect (dlg: DialogPtr; item: integer; var r: Rect);
		var
			width: integer;
	begin
		GetDItemRect(dlg, item, r);
		width := r.right - r.left;
		r.right := r.left + 32;
		r.bottom := r.top + 32;
		OffsetRect(r, (width - 32) div 2, 8);
	end; (* GetIconRect *)

	var
		button_highlighted: boolean;

	procedure ButtonDeviceLoopProc (depth: integer; deviceFlags: integer; targetDevice: GDHandle; item: longint);
		var
			dlg: DialogPtr;
			r: Rect;
			width: integer;
			colour: RGBColor;
			font_info: FontInfo;
			boundary_rect: Rect;
			icon_rect: Rect;
	begin
		GetPort(dlg);
		PenNormal;
		GetDItemRect(dlg, item, r);
		if button_highlighted then begin
			if depth = 1 then begin
				PaintRect(r);
			end
			else begin
				colour.red := lowrd(16000);     (* 8520 *)
				colour.green := lowrd(16000);
				colour.blue := lowrd(16000);
				RGBBackColor(colour);
				EraseRect(r);
				BackColor(whiteColor);
			end; (* if *)
		end
		else begin
			if depth = 1 then begin
				EraseRect(r);
			end
			else begin
				colour.red := lowrd(57015);
				colour.green := lowrd(57015);
				colour.blue := lowrd(57015);
				RGBBackColor(colour);
				EraseRect(r);
				BackColor(whiteColor);
			end; (* if *)
		end; (* if *)
		FrameRect(r);
		GetIconRect(dlg, item, icon_rect);
		boundary_rect := icon_rect;
		InsetRect(boundary_rect, -4, -4);
		EraseRect(boundary_rect);
		FrameRect(boundary_rect);
		DrawIcon(200 + item, icon_rect, button_highlighted);
		TextFont(applFont);
		TextSize(9);
		GetFontInfo(font_info);
		width := StringWidth(GetAString(130, item));
		MoveTo((r.left + (r.right - r.left) div 2) - (width div 2), r.bottom - 2 - font_info.descent);
		if button_highlighted then begin
			TextMode(srcBic);
		end; (* if *)
		DrawString(GetAString(130, item));
		TextMode(srcOr);
	end; (* ButtonDeviceLoopProc *)

	procedure DrawButton (dlg: DialogPtr; item: integer; highlighted: boolean);
		var
			r: Rect;
			rgn: RgnHandle;
	begin
		button_highlighted := highlighted;
		SetPort(dlg);
		if TrapAvailable(_DeviceLoop) then begin
			GetDItemRect(dlg, item, r);
			rgn := NewRgn;
			RectRgn(rgn, r);
			HackDeviceLoop(rgn, @ButtonDeviceLoopProc, item, 0);
			DisposeRgn(rgn);
		end
		else begin
			ButtonDeviceLoopProc(1, 0, nil, item);
		end; (* if *)
	end; (* DrawButton *)

	procedure UserItemUpdate (dlg: DialogPtr; item: integer);
	begin
		DrawButton(dlg, item, false);
	end; (* UserItemUpdate *)

	function WhatOpenButton (wt: WindowType; item: integer): OSErr;
	begin
		SetDItemHandle(windowinfo[wt].window, item, @UserItemUpdate);
		WhatOpenButton := noErr;
	end; (* WhatOpenButton *)

	function WhatClickButton (wt: WindowType; item: integer; er: eventRecord): OSErr;
		var
			dlg: DialogPtr;
			highlighted: boolean;
			junk: OSErr;
			mouse_pos: Point;
			r: Rect;

		procedure ToggleHighlight;
		begin
			highlighted := not highlighted;
			DrawButton(dlg, item, highlighted);
		end; (* ToggleHighlight *)

		var
			err: OSErr;
	begin
		dlg := windowinfo[wt].window;
		GetDItemRect(dlg, item, r);
		highlighted := false;
		repeat
			GetMouse(mouse_pos);
			if PtInRect(mouse_pos, r) <> highlighted then begin
				ToggleHighlight;
			end; (* if *)
		until not StillDown;
		err := noErr;
		if highlighted then begin
			ToggleHighlight;
			err := WindowsOpen(WindowType(ord(WT_Personal) + item - 1));
		end; (* if *)
		WhatClickButton := err;
	end; (* WhatClickButton *)

end. (* ICButtonWhat *)






function WhatClickButton (wt: WindowType; item: integer; er: eventRecord): OSErr;
	const
		HiliteMode = $938;
	var
		dlg: DialogPtr;
		r: Rect;
		highlighted: boolean;
		suite: Handle;
		junk: OSErr;
		transfer: integer;
		highlight_rgn: RgnHandle;
		tmp_rgn: RgnHandle;
		icon_rect: Rect;
		mouse_pos: Point;

	procedure ToggleHighlight;
	begin
		highlighted := not highlighted;
		if system7 then begin
			if highlighted then begin
				transfer := ttSelected;
			end
			else begin
				transfer := ttNone;
			end; (* if *)
			junk := PlotIconSuite(icon_rect, atNone, transfer, suite);
		end; (* if *)
		if has_colorQD then begin
			BitClr(Ptr(HiliteMode), pHiliteBit);
		end; (* if *)
		InvertRgn(highlight_rgn);
	end; (* ToggleHighlight *)

	var
		err: OSErr;
begin
	dlg := windowinfo[wt].window;
	GetIconRect(dlg, item, icon_rect);
	GetDItemRect(dlg, item, r);
	suite := nil;
	highlight_rgn := NewRgn;
	InsetRect(r, 1, 1);
	RectRgn(highlight_rgn, r);
	if system7 then begin
		junk := GetIconSuite(suite, 200 + item, svAllLargeData);
		tmp_rgn := NewRgn;
		junk := IconSuiteToRgn(tmp_rgn, icon_rect, atNone, suite);
		XorRgn(tmp_rgn, highlight_rgn, highlight_rgn);
		DisposeRgn(tmp_rgn);
	end; (* if *)
	highlighted := false;
	while StillDown do begin
		GetMouse(mouse_pos);
		if PtInRect(mouse_pos, r) <> highlighted then begin
			ToggleHighlight;
		end; (* if *)
	end; (* while *)
	err := noErr;
	if highlighted then begin
		ToggleHighlight;
		err := WindowsOpen(WindowType(ord(WT_Personal) + item - 1), 200 + item);
	end; (* if *)
	if suite <> nil then begin
		junk := DisposeIconSuite(suite, false);
	end; (* if *)
	if highlight_rgn <> nil then begin
		DisposeRgn(highlight_rgn);
	end; (* if *)
	WhatClickButton := err;
end; (* WhatClickButton *)
unit WASTE1;

{ WASTE PROJECT: }
{ Unit One: Low-level routines; Drawing }

{ Copyright © 1993-1994 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		Script, WASTEIntf;

	function _WEOffsetToLine (offset: LongInt;
									hWE: WEHandle): LongInt;
	function _WEPixelToLine (vOffset: LongInt;
									hWE: WEHandle): LongInt;
	function _WEOffsetToRun (offset: LongInt;
									hWE: WEHandle): LongInt;
	procedure _WEGetIndStyle (runIndex: LongInt;
									var info: WERunInfo;
									hWE: WEHandle);
	procedure WEGetRunInfo (offset: LongInt;
									var info: WERunInfo;
									hWE: WEHandle);
	procedure _WEContinuousStyleRange (rangeStart, rangeEnd: LongInt;
									var mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle);
	procedure _WESynchNullStyle (hWE: WEHandle);
	function WEContinuousStyle (var mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle): Boolean;
	procedure _WESegmentLoop (firstLine, lastLine: LongInt;
									function Callback (pLine: LinePtr;
																	pAttrs: WERunAttributesPtr;
																	pSegment: Ptr;
																	segmentStart, segmentLength: LongInt;
																	styleRunPosition: JustStyleCode): Boolean;
									hWE: WEHandle);
	procedure _WEDrawLines (firstLine, lastLine: LongInt;
									doErase: Boolean;
									hWE: WEHandle);
	function _WEAllocate (blockSize: Size;
									allocFlags: Integer;
									var h: univ Handle): OSErr;
	function _WECalcPenIndent (slop: Integer;
									alignment: Integer): Integer;
	procedure _WESaveQDEnvironment (port: GrafPtr;
									saveColor: Boolean;
									var theEnvironment: QDEnvironment);
	procedure _WERestoreQDEnvironment (var theEnvironment: QDEnvironment);
	procedure _WEFillFontInfo (port: GrafPtr;
									var targetStyle: WERunAttributes);
	procedure _WECopyStyle (var sourceStyle, targetStyle: WETextStyle;
									offStyles: Integer;
									mode: Integer);
	function _WEOffsetInRange (offset: LongInt;
									edge: SignedByte;
									rangeStart, rangeEnd: LongInt): Boolean;

implementation
	uses
		ExternQD, Palettes, QDOffscreen;

	function _WEOffsetToLine (offset: LongInt;
									hWE: WEHandle): LongInt;

{ given a byte offset into the text, find the corresponding line index }

		var
			pWE: WEPtr;
			pLines: LineArrayPtr;
			minIndex, maxIndex, index: LongInt;
	begin
		pWE := hWE^;

{ get a pointer to the line array }
		pLines := pWE^.hLines^;

{ do a fast binary search through the style run array }
		minIndex := 0;
		maxIndex := pWE^.nLines;

		while (minIndex < maxIndex) do
			begin
				index := BSR(minIndex + maxIndex, 1);
				if (offset >= pLines^[index].lineStart) then
					if (offset < pLines^[index + 1].lineStart) then
						Leave
					else
						minIndex := index + 1
				else
					maxIndex := index;
			end;  { while }

		_WEOffsetToLine := index;

	end;  { _WEOffsetToLine }

	function _WEPixelToLine (vOffset: LongInt;
									hWE: WEHandle): LongInt;

{ given a vertical pixel offset in local coordinates, }
{ find the corresponding line index }

		var
			pWE: WEPtr;
			pLines: LineArrayPtr;
			minIndex, maxIndex, index: LongInt;
	begin
		pWE := hWE^;

{ get a pointer to the line array }
		pLines := pWE^.hLines^;

{ do a fast binary search through the style run array }
		minIndex := 0;
		maxIndex := pWE^.nLines;

		while (minIndex < maxIndex) do
			begin
				index := BSR(minIndex + maxIndex, 1);
				if (vOffset >= pLines^[index].lineOrigin) then
					if (vOffset < pLines^[index + 1].lineOrigin) then
						Leave
					else
						minIndex := index + 1
				else
					maxIndex := index;
			end;  { while }

		_WEPixelToLine := index;

	end;  { _WEPixelToLine }

	function _WEOffsetToRun (offset: LongInt;
									hWE: WEHandle): LongInt;
		var
			pWE: WEPtr;
			pRuns: RunArrayPtr;
			minIndex, maxIndex, index: LongInt;
	begin
		_WEOffsetToRun := 0;
		pWE := hWE^;

{ get a pointer to the style run array }
		pRuns := pWE^.hRuns^;

{ do a fast binary search through the style run array }
		minIndex := 0;
		maxIndex := pWE^.nRuns;

		while (minIndex < maxIndex) do
			begin
				index := BSR(minIndex + maxIndex, 1);
				if (offset >= pRuns^[index].runStart) then
					if (offset < pRuns^[index + 1].runStart) then
						Leave
					else
						minIndex := index + 1
				else
					maxIndex := index;
			end;  { while }

		_WEOffsetToRun := index;

	end;  { _WEOffsetToRun }

	procedure _WEGetIndStyle (runIndex: LongInt;
									var info: WERunInfo;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			pTheRun: RunArrayPeek;
	begin
		pWE := hWE^;

{ get a pointer to the specified run array element }
		pTheRun := @pWE^.hRuns^^[runIndex];

{ fill in the runStart and runEnd fields from the style run array }
		info.runStart := pTheRun^.first.runStart;
		info.runEnd := pTheRun^.second.runStart;

{ copy the style information from the appropriate entry in the style table }
		info.runAttrs := pWE^.hStyles^^[pTheRun^.first.styleIndex].info;

	end;  { _WEGetIndStyle }

	procedure WEGetRunInfo (offset: LongInt;
									var info: WERunInfo;
									hWE: WEHandle);
	begin
		_WEGetIndStyle(_WEOffsetToRun(offset, hWE), info, hWE);
	end;  { WEGetRunInfo }

	procedure _WEContinuousStyleRange (rangeStart, rangeEnd: LongInt;
									var mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle);

{ find out which style attributes are continous over the specified text range }
{ on entry, the mode bitmap specifies which attributes are to be checked }
{ on exit, the mode bitmap specifies the continuous attributes, also copied to ts }

		var
			pWE: WEPtr;
			bitmap: LongInt;
			runIndex: LongInt;
			runInfo: WERunInfo;
	begin
		pWE := hWE^;

{ get bitmap of style attributes to check (valid bits are kModeFont..kModeColor) }
		bitmap := BitAnd(mode, weDoAll);

{ get style info at the beginning of the specified range }
		runIndex := _WEOffsetToRun(rangeStart, hWE);
		_WEGetIndStyle(runIndex, runInfo, hWE);

{ copy the specified fields to ts }
		_WECopyStyle(runInfo.runAttrs.runStyle, ts, 0, BitOr(mode, weDoReplaceFace));

{ loop through style runs across the current selection range }
{ if we determine that all specified attributes are discontinuous, we exit prematurely }
		repeat
			_WEGetIndStyle(runIndex, runInfo, hWE);

{ determine which attributes have changed, if any }
			if BTST(bitmap, kModeFont) then
				if (runInfo.runAttrs.tsFont <> ts.tsFont) then
					BCLR(bitmap, kModeFont);

			if BTST(bitmap, kModeFace) then
				if (runInfo.runAttrs.tsFace <> ts.tsFace) then
					begin
						ts.tsFace := BitAnd(ts.tsFace, runInfo.runAttrs.tsFace);
						if (ts.tsFace = 0) then
							BCLR(bitmap, kModeFace);
					end;

			if BTST(bitmap, kModeSize) then
				if (runInfo.runAttrs.tsSize <> ts.tsSize) then
					BCLR(bitmap, kModeSize);

			if BTST(bitmap, kModeColor) then
				if (not _WEBlockCmp(@runInfo.runAttrs.tsColor, @ts.tsColor, SizeOf(RGBColor))) then
					BCLR(bitmap, kModeColor);

			runIndex := runIndex + 1;
		until (bitmap = 0) | (runInfo.runEnd >= rangeEnd);

		mode := bitmap;
	end;  { _WEContinuousStyleRange }

	procedure _WESynchNullStyle (hWE: WEHandle);

{ This routine fills the nullStyle field of the WE record with valid information }
{ and makes sure that the null style font belongs to the keyboard script. }

		var
			pWE: WEPtr;
			runIndex: LongInt;
			keyboardScript: ScriptCode;
			fontID: Integer;
			runInfo: WERunInfo;
	begin
		pWE := hWE^;

{ find the run index of the style run preceding the insertion point }
		runIndex := _WEOffsetToRun(pWE^.selStart - 1, hWE);

{ if the nullStyle record is marked as invalid, fill it with the style attributes }
{ associated with the character preceding the insertion point, and mark it as valid }
		if (not BTST(pWE^.flags, weFUseNullStyle)) then
			begin
				_WEGetIndStyle(runIndex, runInfo, hWE);
				pWE^.nullStyle := runInfo.runAttrs;
				BSET(pWE^.flags, weFUseNullStyle);
			end;

{ if only the Roman script is installed, we're finished }
		if (not BTST(pWE^.flags, weFNonRoman)) then
			Exit(_WESynchNullStyle);

{ *** FONT / KEYBOARD SYNCHRONIZATION *** }
{ get the keyboard script }
		keyboardScript := GetEnvirons(smKeyScript);

{ find out what font will be used for the next character typed }
		fontID := pWE^.nullStyle.tsFont;

{ do nothing if the font script is the same as the keyboard script }
		if (Font2Script(fontID) = keyboardScript) then
			Exit(_WESynchNullStyle);

{ scan style runs starting from the insertion point backwards,}
{ looking for the first font belonging to the keyboard script }
		repeat
			_WEGetIndStyle(runIndex, runInfo, hWE);
			fontID := runInfo.runAttrs.tsFont;
			if (Font2Script(fontID) = keyboardScript) then
				Leave;
			runIndex := runIndex - 1;
		until (runIndex < 0);

{ if no font was ever used for the keyboard script, default to the }
{ application font for the script }
		if (runIndex < 0) then
			fontID := GetScript(keyboardScript, smScriptAppFond);

{ change the font in the null style record }
		pWE^.nullStyle.tsFont := fontID;

	end;  { _WESynchNullStyle }

	function WEContinuousStyle (var mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle): Boolean;

{ find out which style attributes are continous over the selection range }
{ on entry, the mode bitmap specifies which attributes are to be checked }
{ on exit, the mode bitmap specifies the continuous attributes, also copied to ts }
{ return TRUE if all specified attributes are continuous }

		var
			pWE: WEPtr;
			oldMode: Integer;
	begin
		pWE := hWE^;

{ two rather different paths are taken depending on whether }
{ the selection range is empty or not }
		if (pWE^.selStart = pWE^.selEnd) then
			begin

{ if the selection range is empty, always return TRUE and set ts }
{ from the nullStyle record, after having validated it }
				WEContinuousStyle := true;
				_WESynchNullStyle(hWE);
				_WECopyStyle(pWE^.nullStyle.runStyle, ts, 0, BitOr(mode, weDoReplaceFace));
			end
		else
			begin

{ otherwise get the continuous style attributes over the selection range }
				oldMode := mode;
				_WEContinuousStyleRange(pWE^.selStart, pWE^.selEnd, mode, ts, hWE);

{ return TRUE if mode hasn't changed }
				WEContinuousStyle := (oldMode = mode);

			end;
	end;  { WEContinuousStyle }

	procedure _WESegmentLoop (firstLine, lastLine: LongInt;
									function Callback (pLine: LinePtr;
																	pAttrs: WERunAttributesPtr;
																	pSegment: Ptr;
																	segmentStart, segmentLength: LongInt;
																	styleRunPosition: JustStyleCode): Boolean;
									hWE: WEHandle);

{ For each style segment on every line in the specified range, set up }
{ text attributes in the port and call the callback. }
{ the WE record must be already locked }

		var
			pWE: WEPtr;
			pLines: LineArrayPtr;
			pText: LongInt;
			lineIndex: LongInt;
			runIndex, previousRunIndex: LongInt;
			lineStart, lineEnd, segmentStart, segmentEnd: LongInt;
			styleRunPosition: JustStyleCode;
			runInfo: WERunInfo;
			saveLineLock: Boolean;
			saveTextLock: Boolean;
			saveEnvironment: QDEnvironment;
	begin
		pWE := hWE^;

{ save the QuickDraw environment }
		_WESaveQDEnvironment(pWE^.port, BTST(pWE^.flags, weFHasColorQD), saveEnvironment);

{ make sure firstLine and lastLine are within the allowed range }
		lineIndex := pWE^.nLines - 1;
		firstLine := _WEPinInRange(firstLine, 0, lineIndex);
		lastLine := _WEPinInRange(lastLine, 0, lineIndex);

{ lock the line array }
		saveLineLock := _WESetHandleLock(pWE^.hLines, true);
		pLines := pWE^.hLines^;

{ lock the text }
		saveTextLock := _WESetHandleLock(pWE^.hText, true);
		pText := LongInt(pWE^.hText^);

{ find the style run index corresponding to the beginning of the first line }
		runIndex := _WEOffsetToRun(pLines^[firstLine].lineStart, hWE);
		previousRunIndex := -1;

{ loop thru the specified lines }
		for lineIndex := firstLine to lastLine do
			begin

{ get line start and line end }
				lineStart := pLines^[lineIndex].lineStart;
				lineEnd := pLines^[lineIndex + 1].lineStart;

{ loop thru each style run on this line }
				repeat

{ get style run information for the current style run }
					_WEGetIndStyle(runIndex, runInfo, hWE);

					if (previousRunIndex <> runIndex) then
						begin

{ set new text attributes }
							TextFont(runInfo.runAttrs.tsFont);
							%_TextFace(runInfo.runAttrs.tsFace);
							TextSize(runInfo.runAttrs.tsSize);

{ remember previous run index }
							previousRunIndex := runIndex;
						end;

{ determine the relative position of this style run on the line }
					styleRunPosition := 0;										{ onlyStyleRun }

					if (runInfo.runStart <= lineStart) then
						segmentStart := lineStart
					else
						begin
							styleRunPosition := styleRunPosition + 2;	{ rightStyleRun or middleStyleRun }
							segmentStart := runInfo.runStart;
						end;

					if (runInfo.runEnd >= lineEnd) then
						segmentEnd := lineEnd
					else
						begin
							styleRunPosition := styleRunPosition + 1;	{ leftStyleRun or middleStyleRun }
							segmentEnd := runInfo.runEnd;
						end;

{ do the callback }
					if Callback(@pLines^[lineIndex], @runInfo.runAttrs, Ptr(pText + segmentStart), segmentStart, segmentEnd - segmentStart, styleRunPosition) then
						Leave;

{ advance style run index, unless this style run goes on to the next line }
					if (runInfo.runEnd <= lineEnd) then
						runIndex := runIndex + 1;

				until (runInfo.runEnd >= lineEnd);

			end;  { for }

{ unlock the text }
		IgnoreBoolean(_WESetHandleLock(pWE^.hText, saveTextLock));

{ unlock the line array }
		IgnoreBoolean(_WESetHandleLock(pWE^.hLines, saveLineLock));

{ restore the QuickDraw environment }
		_WERestoreQDEnvironment(saveEnvironment);

	end;  { _WESegmentLoop }

{$IFC WASTE_SEGMENT}
{$S WASTE_TSM_SUPPORT}
{$ENDC}

	procedure _WEDrawTSMHilite (var segmentRect: Rect;
									tsFlags: SignedByte);
		var
			qd: QDPtr;
			flags: LongInt;
			underlineHeight: Integer;
			background, foreground, saveForeground: RGBColor;
			isColorPort: Boolean;
			usingTrueGray: Boolean;
	begin
		flags := tsFlags;
		qd := GetQDGlobals;
		isColorPort := CGrafPtr(qd^.thePort)^.portVersion < 0;
		usingTrueGray := false;

{ by default, the pen pattern is solid }
		PenPat(qd^.black);

{ if we're drawing in color, set the foreground color }
		if (isColorPort) then
			begin

{ save foreground color }
				GetForeColor(saveForeground);

{ by default, the foreground color is black }
				foreground.red := 0;
				foreground.green := 0;
				foreground.blue := 0;

{ if we're underlining raw (unconverted) text, see if a "true gray" is available }
				if (not BTST(flags, tsTSMConverted)) then
					begin
						GetBackColor(background);
						usingTrueGray := GetGray(GetGDevice, background, foreground);
					end;  { if raw text }

{ set the foreground color }
				RGBForeColor(foreground);

			end;  { if color graf port }

{ if we're underlining raw (unconverted) text and no true gray is available, }
{ simulate gray with a 50% pattern }
		if (not BTST(flags, tsTSMConverted)) then
			if (usingTrueGray = false) then
				PenPat(qd^.gray);

{ use a 2-pixel tall underline if text is "selected", else use a 1-pixel tall underline }
		if BTST(flags, tsTSMSelected) then
			underlineHeight := 2
		else
			underlineHeight := 1;

{ segmentRect becomes the rectangle to paint }
		InsetRect(segmentRect, 1, 0);
		segmentRect.top := segmentRect.bottom - underlineHeight;

{ draw the underline }
		PaintRect(segmentRect);

{ restore the foreground color }
		if (isColorPort) then
			RGBForeColor(saveForeground);

	end;  { _WEDrawTSMHilite }

{$IFC WASTE_SEGMENT}
{$S}
{$ENDC}

	procedure _WEDrawLines (firstLine, lastLine: LongInt;
									doErase: Boolean;
									hWE: WEHandle);

{ draw the specified range of lines }
{ we can safely assume that the WE record is already locked }
{ and the port is already set the pWE^.port }

		var
			pWE: WEPtr;
			lineRect: Rect;							{ rectangle enclosing the current line }
			drawRect: Rect;						{ visible portion of the line rectangle }
			bounds: Rect;							{ bounds of the offscreen buffer, in global coordinates }
			screenPort: GrafPtr;
			screenDevice: GDHandle;
			offscreenPixels: PixMapHandle;
			usingColor: Boolean;				{ TRUE if we're drawing in color }
			usingOffscreen: Boolean;			{ TRUE if we're using an offscreen port }
			drawingOffscreen: Boolean;		{ TRUE if actually drawing to an offscreen buffer }

		function SLDraw (pLine: LinePtr;
										pAttrs: WERunAttributesPtr;
										pSegment: Ptr;
										segmentStart, segmentLength: LongInt;
										styleRunPosition: JustStyleCode): Boolean;
			type
				BitMapPtr = ^BitMap;
			var
				slop: Fixed;
				segmentRect: Rect;
				theColor: RGBColor;
		begin
			SLDraw := false;							{ keep looping }

{ is this the first segment on this line? }
			if (styleRunPosition <= smLeftStyleRun) then
				begin

{ calculate the line rectangle (the rectangle which completely encloses the current line) }
					lineRect.left := pWE^.destRect.left;
					lineRect.right := pWE^.destRect.right;
					lineRect.top := pWE^.destRect.top + pLine^.lineOrigin;
					lineRect.bottom := pWE^.destRect.top + LinePeek(pLine)^.second.lineOrigin;

{ calculate the visible portion of this rectangle }
{ we do this by intersecting the line rectangle with the view rectangle }
					drawRect := pWE^.viewRgn^^.rgnBBox;
					if SectRect(lineRect, drawRect, drawRect) then
						;

					if (usingOffscreen) then
						begin

{ calculate the boundary rectangle for the offscreen buffer }
{ this is simply drawRect converted to global coordinates }
							bounds := drawRect;
							LocalToGlobal(bounds.topLeft);
							LocalToGlobal(bounds.botRight);

{ update the offscreen graphics world for the new bounds (this could fail) }
							drawingOffscreen := false;
							if (UpdateGWorld(GWorldPtr(pWE^.offscreenPort), 0, bounds, nil, nil, 0) >= 0) then
								begin

{ get the pixel map associated with the offscreen graphics world }
									offscreenPixels := GetGWorldPixMap(GWorldPtr(pWE^.offscreenPort));

{ lock it down }
									if LockPixels(offscreenPixels) then
										begin

{ offscreen pixel buffer allocation was successful }
											drawingOffscreen := true;

{ switch graphics world }
											GetGWorld(GWorldPtr(screenPort), screenDevice);
											SetGWorld(GWorldPtr(pWE^.offscreenPort), nil);

{ synchronize the coordinate system of the offscreen port with that of the screen port }
											SetOrigin(drawRect.left, drawRect.top);

{ reset the offscreen clip region }
											ClipRect(drawRect);

										end;
								end;  { if pixel buffer allocation was successful }
						end;  { if usingOffscreen }

{ if doErase is TRUE, erase the drawable area before drawing text }
					if (doErase) then
						EraseRect(drawRect);

{ position the pen }
					MoveTo(lineRect.left + _WECalcPenIndent(pLine^.lineSlop, pWE^.alignment), lineRect.top + pLine^.lineAscent);

				end;  { if first segment on line }

{ if drawingOffscreen, switch thePort to the offscreen port }
{ and synchronize text attributes }
			if (drawingOffscreen) then
				begin
					SetPort(pWE^.offscreenPort);
					TextFont(pAttrs^.tsFont);
					%_TextFace(pAttrs^.tsFace);
					TextSize(pAttrs^.tsSize);
				end;  { if drawingOffscreen }

{ get horizontal coordinate of the pen before drawing the segment }
			GetPen(segmentRect.topLeft);

{ set the foreground color }
			if (usingColor) then
				RGBForeColor(pAttrs^.tsColor);

			slop := 0;

{ calculate the "slop" (extra space) for this text segment (justified text only) }
			if (pWE^.alignment = weJustify) then
				begin

{ if this is the last segment on the line, strip trailing spaces }
					if (not Odd(styleRunPosition)) then
						segmentLength := VisibleLength(pSegment, segmentLength);

{ calculate how much extra space is to be applied to this text segment }
					slop := FixMul(NPortionText(pSegment, segmentLength, styleRunPosition, Point(kOneToOneScaling), Point(kOneToOneScaling)), pLine^.lineJustAmount);

				end;  { if alignment = weJustify }

{ draw the segment }
			NDrawJust(pSegment, segmentLength, slop, styleRunPosition, Point(kOneToOneScaling), Point(kOneToOneScaling));

{ get horizontal coordinate of the pen after drawing the segment }
			GetPen(segmentRect.botRight);
			segmentRect.bottom := lineRect.bottom;

{ if this segment is in the TSM area, underline it in the appropriate way }
			if BTST(pAttrs^.tsFlags, tsTSMHilite) then
				_WEDrawTSMHilite(segmentRect, pAttrs^.tsFlags);

			if (drawingOffscreen) then
				begin
					if (not Odd(styleRunPosition)) then
						begin

{ after drawing offscreen the last segment, }
{ prepare to copy the offscreen buffer to video RAM }

{ first set the graphics world to the screen port }
							SetGWorld(GWorldPtr(screenPort), screenDevice);

{ before calling CopyBits, set the foreground color to black to avoid colorization (color only) }
							if (usingColor) then
								begin
									theColor.red := 0;
									theColor.green := 0;
									theColor.blue := 0;
									RGBForeColor(theColor);
								end;

{ copy the offscreen image of the [visible portion of the] line to the screen }
							CopyBits(BitMapPtr(offscreenPixels^)^, screenPort^.portBits, drawRect, drawRect, srcCopy, nil);

{ restore the original offscreen coordinate system and unlock the pixel image }
							SetPort(pWE^.offscreenPort);
							SetOrigin(0, 0);
							UnlockPixels(offscreenPixels);

						end;  { if last segment }

{ restore the screen port for _WESegmentLoop }
					SetPort(screenPort);

				end;  { if drawingOffscreen }
		end;  { SLDraw }

	begin  { _WEDrawLines }
		pWE := hWE^;
		usingOffscreen := false;
		drawingOffscreen := false;

{ do nothing if our graphics port is not visible }
		if EmptyRgn(pWE^.port^.visRgn) then
			Exit(_WEDrawLines);

{ If doErase is TRUE, we're drawing over old text, so we must erase each line }
{ before redrawing it.  But if the weFDrawOffscreen feature is enabled, we draw }
{ the entire line offscreen and then we copy the image right over the old line, }
{ without erasing it, thus achieving a very smooth drawing effect. }

		if ((doErase) and BTST(pWE^.flags, weFDrawOffscreen)) then
			begin

{ has an offscreen world already been allocated? }
				if (pWE^.offscreenPort = nil) then
					begin

{ nope, then create one; its bounds are set initially to an arbitrary rectangle }
						SetRect(bounds, 0, 0, 1, 1);
						IgnoreShort(NewGWorld(GWorldPtr(pWE^.offscreenPort), 0, bounds, nil, nil, gwPixPurge + gwNoNewDevice + gwUseTempMem));
					end;

				usingOffscreen := (pWE^.offscreenPort <> nil);
			end;

		usingColor := BTST(pWE^.flags, weFHasColorQD);
		_WESegmentLoop(firstLine, lastLine, SLDraw, hWE);

	end;  { _WEDrawLines }

	function _WEAllocate (blockSize: Size;
									allocFlags: Integer;
									var h: univ Handle): OSErr;

{ Allocate a new relocatable block. }
{ AllocFlags may specify whether the block should be cleared and whether }
{ temporary memory should be used. }

		var
			theHandle: Handle;
	begin
		theHandle := nil;

{ if kAllocTemp is specified, try tapping temporary memory }
		if (BitAnd(allocFlags, kAllocTemp) <> 0) then
			theHandle := TempNewHandle(blockSize, _WEAllocate);

{ if kAllocTemp isn't specified, or TempNewHandle failed, try with current heap }
		if (theHandle = nil) then
			_WEAllocate := %_NewHandle(blockSize, theHandle);

{ if kAllocClear is specified, zero the block }
		if (BitAnd(allocFlags, kAllocClear) <> 0) then
			if (theHandle <> nil) then
				_WEBlockClr(theHandle^, blockSize);

{ return handle through VAR parameter }
		h := theHandle;

	end;  { _WEAllocate }

	function _WECalcPenIndent (slop: Integer;
									alignment: Integer): Integer;
	begin

{ if alignment is weFlushDefault, use the system global SysDirection }
		if (alignment = weFlushDefault) then
			if (GetSysJust = 0) then
				alignment := weFlushLeft
			else
				alignment := weFlushRight;

		if (alignment = weFlushRight) then
			_WECalcPenIndent := slop								{ right aligned }
		else if (alignment = weCenter) then
			_WECalcPenIndent := slop div 2						{ centered }
		else
			_WECalcPenIndent := 0;									{ left aligned or justified }
	end;  { _WECalcPenIndent }

	procedure _WESaveQDEnvironment (port: GrafPtr;
									saveColor: Boolean;
									var theEnvironment: QDEnvironment);
	begin
		with theEnvironment do
			begin
				GetPort(envPort);
				SetPort(port);
				GetPenState(envPen);
				PenNormal;
				envStyle.tsFont := port^.txFont;
				envStyle.tsFace := GrafPtr1(port)^.txFace;
				Boolean(envStyle.tsFlags) := saveColor;		{ remember if color was saved }
				envStyle.tsSize := port^.txSize;
				if (saveColor) then
					GetForeColor(envStyle.tsColor);
				envMode := port^.txMode;
				TextMode(srcOr);
			end;  { with }
	end;  { _WESaveQDEnvironment }

	procedure _WERestoreQDEnvironment (var theEnvironment: QDEnvironment);
	begin
		with theEnvironment do
			begin
				SetPenState(envPen);
				TextFont(envStyle.tsFont);
				%_TextFace(envStyle.tsFace);
				TextSize(envStyle.tsSize);
				TextMode(envMode);
				if Boolean(envStyle.tsFlags) then
					RGBForeColor(envStyle.tsColor);
				SetPort(envPort);
			end;  { with }
	end;  { _WERestoreQDEnvironment }

	procedure _WEFillFontInfo (port: GrafPtr;
									var targetStyle: WERunAttributes);

{ given a WERunAttributes record, fill in the runHeight, runAscent fields etc. }
		var
			fInfo: FontInfo;
			saveEnvironment: QDEnvironment;
	begin
		_WESaveQDEnvironment(port, false, saveEnvironment);
		with targetStyle do
			begin

{ we don't want a zero font size; although QuickDraw accepts zero to mean }
{ the default font size, it can cause trouble to us when we do calculations }
				if (tsSize = 0) then
					tsSize := 12;

{ set the text attributes }
				TextFont(tsFont);
				TextSize(tsSize);
				%_TextFace(tsFace);
				GetFontInfo(fInfo);
				runHeight := fInfo.ascent + fInfo.descent + fInfo.leading;
				runAscent := fInfo.ascent;
			end;  { with }
		_WERestoreQDEnvironment(saveEnvironment);
	end;  { _WEFillFontInfo }

	procedure _WECopyStyle (var sourceStyle, targetStyle: WETextStyle;
									offStyles: Integer;
									mode: Integer);

{ Copy some or all of the attributes composing sourceStyle to targetStyle. }
{ The mode parameter determines which attributes are to be copied and how. }
{ If mode contains weDoToggleFace, then offStyles indicates which }
{ QuickDraw styles are to be turned off. }

		var
			longMode: LongInt;
			longSize: LongInt;
			sourceFace, targetFace: LongInt;

	begin
		longMode := mode;	{ this allows the compiler to generate tighter code }

{ if the kModeFont bit is set, copy the font family number }
		if BTST(longMode, kModeFont) then
			targetStyle.tsFont := sourceStyle.tsFont;

{ if the kModeSize or the kModeAddSize bit is set, alter the font size }
		if (BitAnd(longMode, weDoSize + weDoAddSize) <> 0) then
			begin
				longSize := sourceStyle.tsSize;

{ if kModeAddSize is set, the source size is added to the target size, }
{ otherwise the source size replaces the target size outright }
				if BTST(longMode, kModeAddSize) then
					longSize := longSize + targetStyle.tsSize;

{ range-check the resulting size }
				longSize := _WEPinInRange(longSize, kMinFontSize, kMaxFontSize);
				targetStyle.tsSize := longSize;

			end;  { if alter size }

{ if kModeFace is set, copy the QuickDraw styles (tsFace field); }
{ the (rather complex) rules for copying the styles are explained below in detail }
		if BTST(longMode, kModeFace) then
			begin
				sourceFace := sourceStyle.tsFace;
				targetFace := targetStyle.tsFace;

{ sourceFace replaces targetFace outright if one or both of these conditions hold: }
{ 1. sourceFace is zero (= empty set = plain text) }
{ 2. the kModeReplaceFace bit is set }

				if ((sourceFace = 0) or BTST(longMode, kModeReplaceFace)) then
					targetFace := sourceFace
				else
					begin

{ Otherwise sourceFace is interpreted as a bitmap indicating }
{ which styles are to be altered -- all other styles are left intact. }
{ What exactly happens to the styles indicated in sourceFace }
{ depends on whether the kModeToggleFace bit is set or clear. }

{ if kModeToggleFace is set, turn a style off if it's set in offStyles, else turn it on }
						if BTST(longMode, kModeToggleFace) then
							targetFace := BitOr(BitXor(sourceFace, offStyles), BitAnd(targetFace, BitNot(sourceFace)))
						else

{ if kModeToggleFace is clear, turn on the styles specified in sourceStyle }
							targetFace := BitOr(targetFace, sourceFace);

{ the condense and extend attributes are mutually exclusive: if one is set }
{ in sourceFace, remove it from targetFace }
						if BTST(sourceFace, tsCondense) then
							BCLR(targetFace, tsExtend)
						else if BTST(sourceFace, tsExtend) then
							BCLR(targetFace, tsCondense)
					end;

				targetStyle.tsFace := targetFace;
			end;  { if alter face }

{ if kModeColor is set, change target color }
		if BTST(longMode, kModeColor) then
			targetStyle.tsColor := sourceStyle.tsColor;

{ always clear targetStyle.tsFlags by default }
		targetStyle.tsFlags := 0;

{ if kModeFlags is set, copy the tsFlags field }
		if BTST(longMode, kModeFlags) then
			targetStyle.tsFlags := sourceStyle.tsFlags;

	end;  { _WECopyStyle }

	function _WEOffsetInRange (offset: LongInt;
									edge: SignedByte;
									rangeStart, rangeEnd: LongInt): Boolean;

{ return TRUE if the position specified by the pair < offset, edge > }
{ is within the specified range }

	begin

{ if edge is kTrailingEdge, offset really refers to the preceding character }
		if (edge = kTrailingEdge) then
			if (offset > 0) then
				offset := offset - 1;

{ return TRUE iff offset is within the specified range }
		_WEOffsetInRange := ((offset >= rangeStart) and (offset < rangeEnd));

	end;  { _WEOffsetInRange }

end.
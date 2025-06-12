unit WASTE3;

{ WASTE PROJECT }
{ Unit Three: Selecting; Highlighting; Scrolling &c. }

{ Copyright © 1993-1994 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WASTE2;

	function WEGetOffset (thePoint: LongPoint;
									var edge: SignedByte;
									hWE: WEHandle): LongInt;
	procedure WEGetPoint (offset: LongInt;
									var thePoint: LongPoint;
									var lineHeight: Integer;
									hWE: WEHandle);
	function WEGetHiliteRgn (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): RgnHandle;
	procedure _WEDrawCaret (hWE: WEHandle);
	procedure _WEHiliteRange (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle);
	procedure WEUpdate (updateRgn: RgnHandle;
									hWE: WEHandle);
	procedure WEScroll (hOffset, vOffset: LongInt;
									hWE: WEHandle);
	function _WEScrollIntoView (offset: LongInt;
									hWE: WEHandle): Boolean;
	procedure WESelView (hWE: WEHandle);
	procedure WEActivate (hWE: WEHandle);
	procedure WEDeactivate (hWE: WEHandle);
	function WEIsActive (hWE: WEHandle): Boolean;
	procedure _WEDoArrowKey (arrow: Integer;
									modifiers: Integer;
									hWE: WEHandle);
	function WEAdjustCursor (mouseLoc: Point;
									mouseRgn: RgnHandle;
									hWE: WEHandle): Boolean;
	procedure WEIdle (var maxSleep: LongInt;
									hWE: WEHandle);
	procedure WESetSelection (selStart, selEnd: LongInt;
									hWE: WEHandle);
	procedure WEClick (mouseLoc: Point;
									modifiers: Integer;
									clickTime: LongInt;
									hWE: WEHandle);
	procedure WESetAlignment (alignment: SignedByte;
									hWE: WEHandle);
	procedure WEFindWord (offset: LongInt;
									edge: SignedByte;
									var wordStart, wordEnd: LongInt;
									hWE: WEHandle);
	procedure WEFindLine (offset: LongInt;
									edge: SignedByte;
									var lineStart, lineEnd: LongInt;
									hWE: WEHandle);

implementation
	uses
		ExternQD, QDOffscreen, TextServices;

	procedure ClearHiliteBit;
	inline
		$08B8, $0007, $0938; 		{ bclr #7, HiliteMode }

	function WEGetOffset (thePoint: LongPoint;
									var edge: SignedByte;
									hWE: WEHandle): LongInt;

{ given a long point in local coordinates, }
{ find the text offset corresponding to the nearest glyph }

		var
			pWE: WEPtr;
			lineIndex: LongInt;
			pixelWidth: Fixed;
			saveWELock: Boolean;

		function SLPixelToChar (pLine: LinePtr;
										pAttrs: WERunAttributesPtr;
										pSegment: Ptr;
										segmentStart, segmentLength: LongInt;
										styleRunPosition: JustStyleCode): Boolean;
			var
				slop: Fixed;
				width: Fixed;
		begin

{ if this is the first style run on the line, subtract pen indent from pixelWidth }
			if (styleRunPosition <= smLeftStyleRun) then
				pixelWidth := pixelWidth - BSL(_WECalcPenIndent(pLine^.lineSlop, pWE^.alignment), 16);

{ strip trailing spaces if this is the last segment on the line }
			if (not Odd(styleRunPosition)) then
				segmentLength := VisibleLength(pSegment, segmentLength);

{ calculate slop for this text segment (justified text only) }
			if (pWE^.alignment = weJustify) then
				slop := FixMul(NPortionText(pSegment, segmentLength, styleRunPosition, Point(kOneToOneScaling), Point(kOneToOneScaling)), pLine^.lineJustAmount)
			else
				slop := 0;

{ call PixelToChar for this segment }
			WEGetOffset := segmentStart + NPixel2Char(pSegment, segmentLength, slop, pixelWidth, Boolean(edge), width, styleRunPosition, Point(kOneToOneScaling), Point(kOneToOneScaling));

{ update pixelWidth for next iteration }
			pixelWidth := width;

{ if pixelWidth has gone negative, we're finished; otherwise go to next run }
			SLPixelToChar := (pixelWidth < 0);

		end;  { SLPixelToChar }

	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ offset thePoint so that it is relative to the top left corner of the destination rectangle }
		thePoint.v := thePoint.v - pWE^.destRect.top;
		thePoint.h := thePoint.h - pWE^.destRect.left;

{ if the point is above the destination rect, return zero }
		if (thePoint.v < 0) then
			begin
				WEGetOffset := 0;
				edge := kTrailingEdge;
			end
		else

{ if the point is below the last line, return last char offset }
			if (thePoint.v >= WEGetHeight(0, maxLongInt, hWE)) then
				begin
					WEGetOffset := pWE^.textLength;
					edge := kLeadingEdge;
				end
			else
				begin

{ find the line index corresponding to the vertical pixel offset }
					lineIndex := _WEPixelToLine(thePoint.v, hWE);

{ express the horizontal pixel offset as a Fixed value }
					pixelWidth := BSL(thePoint.h, 16);

{ walk through the segments on this line calling PixelToChar }
					_WESegmentLoop(lineIndex, lineIndex, SLPixelToChar, hWE);

				end;  { else }

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEGetOffset }

	procedure WEGetPoint (offset: LongInt;
									var thePoint: LongPoint;
									var lineHeight: Integer;
									hWE: WEHandle);

{ given a byte offset into the text, find the corresponding glyph position }
{ this routine is useful for highlighting the text and for positioning the caret }

		var
			pWE: WEPtr;
			pLine: LinePeek;
			lineIndex: LongInt;
			saveWELock: Boolean;

		function SLCharToPixel (pLine: LinePtr;
										pAttrs: WERunAttributesPtr;
										pSegment: Ptr;
										segmentStart, segmentLength: LongInt;
										styleRunPosition: JustStyleCode): Boolean;
			var
				slop: Fixed;
		begin

{ if this is the first style run on the line, add pen indent to thePoint.h }
			if (styleRunPosition <= smLeftStyleRun) then
				thePoint.h := thePoint.h + _WECalcPenIndent(pLine^.lineSlop, pWE^.alignment);

{ calculate slop for this text segment (justified text only) }
			if (pWE^.alignment = weJustify) then
				slop := FixMul(NPortionText(pSegment, segmentLength, styleRunPosition, Point(kOneToOneScaling), Point(kOneToOneScaling)), pLine^.lineJustAmount)
			else
				slop := 0;

{ call CharToPixel to get width of segment up to specified offset }
			thePoint.h := thePoint.h + NChar2Pixel(pSegment, segmentLength, slop, offset - segmentStart, smHilite, styleRunPosition, Point(kOneToOneScaling), Point(kOneToOneScaling));

{ drop out of loop when we reach offset }
			SLCharToPixel := (offset < segmentStart + segmentLength);

		end;  { SLCharToPixel }

	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ the base point is the top left corner of the destination rectangle }
		thePoint := pWE^.destRect.topLeft;

{ first of all find the line on which the glyph lies }
		lineIndex := _WEOffsetToLine(offset, hWE);

{ calculate the vertical coordinate and the line height }
		pLine := @pWE^.hLines^^[lineIndex];
		thePoint.v := thePoint.v + pLine^.first.lineOrigin;
		lineHeight := pLine^.second.lineOrigin - pLine^.first.lineOrigin;

		if ((offset = pWE^.textLength) & (WEGetChar(offset - 1, hWE) = CHR(kEOL))) then
			begin

{ SPECIAL CASE: if offset is past the last character and }
{ the last character is a carriage return, return a point below the last line }

				thePoint.v := thePoint.v + lineHeight;
				thePoint.h := thePoint.h + _WECalcPenIndent(pWE^.destRect.right - pWE^.destRect.left, pWE^.alignment);
			end
		else

{ to get the horizontal coordinate, walk through the style runs on this line }
			_WESegmentLoop(lineIndex, lineIndex, SLCharToPixel, hWE);

{ pin the horizontal coordinate to the destination rectangle }
		thePoint.h := _WEPinInRange(thePoint.h, pWE^.destRect.left, pWE^.destRect.right);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEGetPoint }

	procedure WEFindLine (offset: LongInt;
									edge: SignedByte;
									var lineStart, lineEnd: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			pLine: LineArrayPtr;
	begin
		pWE := hWE^;
		pLine := @pWE^.hLines^^[_WEOffsetToLine(offset, hWE)];
		lineStart := pLine^[0].lineStart;
		lineEnd := pLine^[1].lineStart;
	end;  { WEFindLine }

	function WEFindScriptRun (offset: LongInt;
									edge: SignedByte;
									var scriptRunStart, scriptRunEnd: LongInt;
									hWE: WEHandle): Integer;
		var
			index, saveIndex, saveRunEnd: LongInt;
			runInfo: WERunInfo;
			script1, script2: ScriptCode;
	begin
		if BTST(hWE^^.flags, weFNonRoman) then
			begin

{ if more than one script is installed, limit the search of script run boundaries }
{ to a single line, for speed's sake }
				WEFindLine(offset, edge, scriptRunStart, scriptRunEnd, hWE);

{ find the style run the specified offset is in }
				index := _WEOffsetToRun(offset, hWE);
				_WEGetIndStyle(index, runInfo, hWE);

{ return the style run font as function result }
				WEFindScriptRun := runInfo.runAttrs.tsFont;

{ find the script code associated with this style run }
				script1 := Font2Script(runInfo.runAttrs.tsFont);

{ save index and runInfo.runEnd for the second while loop }
				saveIndex := index;
				saveRunEnd := runInfo.runEnd;

{ walk backwards across style runs preceding offset, looking for a script run boundary }
				while (runInfo.runStart > scriptRunStart) do
					begin
						index := index - 1;
						_WEGetIndStyle(index, runInfo, hWE);
						script2 := Font2Script(runInfo.runAttrs.tsFont);
						if (script1 <> script2) then
							begin
								scriptRunStart := runInfo.runEnd;
								Leave;
							end;
					end;  { while }

{ restore index and runInfo.runEnd }
				index := saveIndex;
				runInfo.runEnd := saveRunEnd;

{ walk forward across style runs following offset, looking for a script run boundary }
				while (runInfo.runEnd < scriptRunEnd) do
					begin
						index := index + 1;
						_WEGetIndStyle(index, runInfo, hWE);
						script2 := Font2Script(runInfo.runAttrs.tsFont);
						if (script1 <> script2) then
							begin
								scriptRunEnd := runInfo.runStart;
								Leave;
							end;
					end;  { while }

			end
		else
			begin

{ only the Roman script is enabled: the whole text constitutes one script run }
				WEFindScriptRun := systemFont;
				scriptRunStart := 0;
				scriptRunEnd := hWE^^.textLength;
			end;
	end;  { WEFindScriptRun }

	procedure WEFindWord (offset: LongInt;
									edge: SignedByte;
									var wordStart, wordEnd: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			port, savePort: GrafPtr;
			hText: Handle;
			runStart, runEnd: LongInt;
			wordBreaks: OffsetTable;
			saveFont: Integer;
			saveTextLock: Boolean;
	begin
		pWE := hWE^;
		port := pWE^.port;
		hText := pWE^.hText;

{ set up the port }
		GetPort(savePort);
		SetPort(port);

{ find the script run the specified offset is in (words cannot straddle script boundaries) }
{ and set the port font to the specified script }
		saveFont := port^.txFont;
		TextFont(WEFindScriptRun(offset, edge, runStart, runEnd, hWE));

{ make sure we pass FindWord short values }
		runStart := _WEPinInRange(runStart, offset - (maxint div 2), offset);
		runEnd := _WEPinInRange(runEnd, offset, offset + (maxint div 2));

{ lock the text }
		saveTextLock := _WESetHandleLock(hText, true);

{ call FindWord using the whole script run as a context }
		FindWord(Ptr(LongInt(hText^) + runStart), runEnd - runStart, offset - runStart, Boolean(edge), nil, wordBreaks);

{ unlock the text }
		IgnoreBoolean(_WESetHandleLock(hText, saveTextLock));

{ restore font and port }
		TextFont(saveFont);
		SetPort(savePort);

{ calculate wordStart and wordEnd relative to the beginning of the text }
		wordStart := runStart + wordBreaks[0].offFirst;
		wordEnd := runStart + wordBreaks[0].offSecond;

	end;  { WEFindWord }

	procedure _WEDrawCaret (hWE: WEHandle);
		var
			pWE: WEPtr;
			thePoint: LongPoint;
			caretRect: Rect;
			caretHeight: Integer;
			savePort: GrafPtr;
			saveClip: RgnHandle;
	begin

{ the WE record must be already locked }
		pWE := hWE^;

{ do nothing if we're not active }
		if (not BTST(pWE^.flags, weFActive)) then
			Exit(_WEDrawCaret);

{ find the caret position using WEGetPoint }
		WEGetPoint(pWE^.selStart, thePoint, caretHeight, hWE);
		WELongPointToPoint(thePoint, caretRect.topLeft);
		if (caretRect.left > pWE^.destRect.left) then
			caretRect.left := caretRect.left - 1;

{ calculate caret rectangle }
		caretRect.bottom := caretRect.top + caretHeight;
		caretRect.right := caretRect.left + 1;

{ set up the port }
		GetPort(savePort);
		SetPort(pWE^.port);

{ clip to the view region }
		saveClip := NewRgn;
		GetClip(saveClip);
		SetClip(pWE^.viewRgn);

{ draw the caret }
		InvertRect(caretRect);

{ invert caretVisible }
		pWE^.flags := BitXor(pWE^.flags, BSL(1, weFCaretVisible));

{ update caretTime }
		pWE^.caretTime := TickCount;

{ restore the clip region }
		SetClip(saveClip);
		DisposeRgn(saveClip);

{ restore the port }
		SetPort(savePort);

	end;  { _WEDrawCaret }

	function WEGetHiliteRgn (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): RgnHandle;

{ returns the hilite region corresponding to the specified range }
{ the caller is responsible for disposing of the returned region }
{ when it's finished with it }

		var
			pWE: WEPtr;
			hiliteRgn: RgnHandle;
			selRect: LongRect;
			firstPoint, lastPoint: LongPoint;
			firstLineHeight, lastLineHeight: Integer;
			r: Rect;
			savePort: GrafPtr;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ set up the port }
		GetPort(savePort);
		SetPort(pWE^.port);

{ make sure rangeStart comes before rangeEnd }
		_WEReorder(rangeStart, rangeEnd);

{ calculate pixel location corresponding to rangeStart }
		WEGetPoint(rangeStart, firstPoint, firstLineHeight, hWE);

{ calculate pixel location corresponding to rangeEnd }
		WEGetPoint(rangeEnd, lastPoint, lastLineHeight, hWE);

{ open a region: rects to be hilited will be accumulated in this }
		OpenRgn;

		if (firstPoint.v = lastPoint.v) then
			begin

{ selection range encompasses only one line }
				WESetLongRect(selRect, firstPoint.h, firstPoint.v, lastPoint.h, lastPoint.v + lastLineHeight);
				WELongRectToRect(selRect, r);
				FrameRect(r);
			end
		else
			begin

{ selection range encompasses more than one line }
{ hilite the first line }
				WESetLongRect(selRect, firstPoint.h, firstPoint.v, pWE^.destRect.right, firstPoint.v + firstLineHeight);
				WELongRectToRect(selRect, r);
				FrameRect(r);

{ any lines between the first and the last one? }
				if (firstPoint.v + firstLineHeight < lastPoint.v) then
					begin

{ hilite all the lines in-between }
						WESetLongRect(selRect, pWE^.destRect.left, firstPoint.v + firstLineHeight, pWE^.destRect.right, lastPoint.v);
						WELongRectToRect(selRect, r);
						FrameRect(r);
					end;

{ hilite the last line }
				WESetLongRect(selRect, pWE^.destRect.left, lastPoint.v, lastPoint.h, lastPoint.v + lastLineHeight);
				WELongRectToRect(selRect, r);
				FrameRect(r);
			end;

{ copy the accumulated region into a new region }
		hiliteRgn := NewRgn;
		CloseRgn(hiliteRgn);

{ restrict this region to the view region }
		SectRgn(hiliteRgn, pWE^.viewRgn, hiliteRgn);

{ restore the port }
		SetPort(savePort);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

{ return the hilite region }
		WEGetHiliteRgn := hiliteRgn;

	end;  { WEGetHiliteRgn }

	procedure _WEHiliteRange (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			saveClip, auxRgn, hiliteRgn: RgnHandle;
			savePen: PenState;
			savePort: GrafPtr;
	begin

{ the WE record must be already locked }
		pWE := hWE^;

{ do nothing if the specified range is empty }
		if (rangeStart = rangeEnd) then
			Exit(_WEHiliteRange);

{ set up the port }
		GetPort(savePort);
		SetPort(pWE^.port);

{ create auxiliary regions }
		saveClip := NewRgn;
		auxRgn := NewRgn;

{ restrict the clip region to the view rectangle }
		GetClip(saveClip);
		SectRgn(saveClip, pWE^.viewRgn, auxRgn);
		SetClip(auxRgn);

{ get the hilite region corresponding to the specified range }
		hiliteRgn := WEGetHiliteRgn(rangeStart, rangeEnd, hWE);

{ hilite the region or frame it, depending on the setting of the active flag }
		if BTST(pWE^.flags, weFActive) then
			begin
				ClearHiliteBit;
				InvertRgn(hiliteRgn);
			end
		else if BTST(pWE^.flags, weFOutlineHilite) then
			begin
				GetPenState(savePen);
				PenNormal;
				PenMode(patXor);
				ClearHiliteBit;
				FrameRgn(hiliteRgn);
				SetPenState(savePen);
			end;

{ restore the clip region }
		SetClip(saveClip);

{ dispose of all regions }
		DisposeRgn(saveClip);
		DisposeRgn(auxRgn);
		DisposeRgn(hiliteRgn);

{ restore the port }
		SetPort(savePort);

	end;  { _WEHiliteRange }

	procedure WESetSelection (selStart, selEnd: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			oldSelStart, oldSelEnd: LongInt;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ invalid the null style }
		BCLR(pWE^.flags, weFUseNullStyle);

{ hide the caret if it's showing }
		if BTST(pWE^.flags, weFCaretVisible) then
			_WEDrawCaret(hWE);

{ range-check parameters }
		selStart := _WEPinInRange(selStart, 0, pWE^.textLength);
		selEnd := _WEPinInRange(selEnd, 0, pWE^.textLength);

{ set the weFAnchorIsEnd bit if selStart > selEnd, then reorder the endpoints }
		if (selStart > selEnd) then
			BSET(pWE^.flags, weFAnchorIsEnd)
		else
			BCLR(pWE^.flags, weFAnchorIsEnd);
		_WEReorder(selStart, selEnd);

{ get old selection range }
		oldSelStart := pWE^.selStart;
		oldSelEnd := pWE^.selEnd;

{ set new selection range }
		pWE^.selStart := selStart;
		pWE^.selEnd := selEnd;

{ if we're active, invert the exclusive-OR between the old range and the new range. }
{ if we're inactive, this optimization can't be used because of outline highlighting. }
		if BTST(pWE^.flags, weFActive) then
			begin
				_WEReorder(oldSelStart, selStart);
				_WEReorder(oldSelEnd, selEnd);
				_WEReorder(oldSelEnd, selStart);
			end;

		_WEHiliteRange(oldSelStart, oldSelEnd, hWE);
		_WEHiliteRange(selStart, selEnd, hWE);

		if (not BTST(pWE^.flags, weFMouseTracking)) then
			begin

{ redraw the caret immediately, if the selection range is empty }
				if (pWE^.selStart = pWE^.selEnd) then
					_WEDrawCaret(hWE);

{ clear clickCount, unless we're tracking the mouse }
				pWE^.clickCount := 0;

{ scroll the selection into view, unless we're tracking the mouse }
				WESelView(hWE);

			end;

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WESetSelection }

	function CallClickLoop (hWE: WEHandle;
									clickProc: ProcPtr): Boolean;
	inline
		$205F,					{ movea.l (sp)+, a0 }
		$4E90;					{ jsr (a0) }

	procedure WEClick (mouseLoc: Point;
									modifiers: Integer;
									clickTime: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			thePoint: LongPoint;
			offset, anchor: LongInt;
			rangeStart, rangeEnd: LongInt;
			edge: SignedByte;
			isMultipleClick: Boolean;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ hide the caret if it's showing }
		if BTST(pWE^.flags, weFCaretVisible) then
			_WEDrawCaret(hWE);

{ find click offset }
		WEPointToLongPoint(mouseLoc, thePoint);
		offset := WEGetOffset(thePoint, edge, hWE);

{ determine whether this click is part of a sequence }
		isMultipleClick := ((clickTime < pWE^.clickTime + GetDblTime) and (offset = pWE^.clickLoc));

{ remember click time, click offset and edge value }
		pWE^.clickTime := clickTime;
		pWE^.clickLoc := offset;
		pWE^.clickEdge := edge;

		if (BitAnd(modifiers, shiftKey) = 0) then
			begin

{ is this click part of a sequence or is it an isolate click? }
				if (isMultipleClick) then
					begin
						pWE^.clickCount := pWE^.clickCount + 1;

{ a double (triple) click creates an anchor-word (anchor-line) }
						if (pWE^.clickCount > 1) then
							WEFindLine(offset, edge, pWE^.anchorStart, pWE^.anchorEnd, hWE)
						else
							WEFindWord(offset, edge, pWE^.anchorStart, pWE^.anchorEnd, hWE);

						offset := pWE^.anchorStart;

					end
				else
					begin
						pWE^.clickCount := 0;
						anchor := offset;
					end
			end
		else

{ if the shift key was down, use the old anchor offset found with the previous click }
			if BTST(pWE^.flags, weFAnchorIsEnd) then
				anchor := pWE^.selEnd
			else
				anchor := pWE^.selStart;

{ set the weFMouseTracking bit while we track the mouse }
		BSET(pWE^.flags, weFMouseTracking);

{ MOUSE TRACKING LOOP }
		repeat

{ get text offset corresponding to mouse position }
			WEPointToLongPoint(mouseLoc, thePoint);
			offset := WEGetOffset(thePoint, edge, hWE);

{ if we're selecting words or lines, pin offset to a word or line boundary }
			if (pWE^.clickCount > 0) then
				begin
					if (pWE^.clickCount > 1) then
						WEFindLine(offset, edge, rangeStart, rangeEnd, hWE)
					else
						WEFindWord(offset, edge, rangeStart, rangeEnd, hWE);

{ choose the word/line boundary and the anchor that are farthest away from each other }
					if (offset > pWE^.anchorStart) then
						begin
							anchor := pWE^.anchorStart;
							offset := rangeEnd;
						end
					else
						begin
							offset := rangeStart;
							anchor := pWE^.anchorEnd;
						end;
				end;

{ set the selection range from anchor point to current offset }
			WESetSelection(anchor, offset, hWE);

{ call the click loop callback, if any }
			if (pWE^.clickLoop <> nil) then
				if (CallClickLoop(hWE, pWE^.clickLoop) = false) then
					Leave;

{ update mouse position }
			GetMouse(mouseLoc);

		until (not WaitMouseUp);

{ clear the weFMouseTracking bit }
		BCLR(pWE^.flags, weFMouseTracking);

{ redraw the caret immediately if the selection range is empty }
		if (anchor = offset) then
			_WEDrawCaret(hWE);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEClick }

	procedure WESetAlignment (alignment: SignedByte;
									hWE: WEHandle);
	begin
		if ((alignment >= weFlushLeft) and (alignment <= weJustify)) then
			if (alignment <> hWE^^.alignment) then
				begin
					hWE^^.alignment := alignment;
					WEUpdate(nil, hWE);
				end;
	end;  { WESetAlignment }

	function _WEArrowOffset (arrow: Integer;
									offset: LongInt;
									hWE: WEHandle): LongInt;

{ given a byte offset corresponding to the caret position and the ASCII code }
{ of an arrow character, find the byte offset of the new caret position }

		var
			thePoint: LongPoint;
			lineHeight: Integer;
			edge: SignedByte;
	begin
		case arrow of

{ left/right arrow: move backwards or forward one character (one or two bytes) }

			kArrowLeft: 
				if (offset > 0) then
					if (WECharByte(offset - 1, hWE) = smSingleByte) then
						offset := offset - 1
					else
						offset := offset - 2;

			kArrowRight: 
				if (offset < hWE^^.textLength) then
					if (WECharByte(offset, hWE) = smSingleByte) then
						offset := offset + 1
					else
						offset := offset + 2;

			kArrowUp, kArrowDown: 
				begin

{ get the pixel location corresponding to offset }
					WEGetPoint(offset, thePoint, lineHeight, hWE);

{ move up or down one line }
					if (arrow = kArrowUp) then
						thePoint.v := thePoint.v - 1
					else
						thePoint.v := thePoint.v + lineHeight;

{ convert the point back to an offset }
					offset := WEGetOffset(thePoint, edge, hWE);
				end;

			otherwise
				;
		end;  { case }

		_WEArrowOffset := offset;
	end;  { _WEArrowOffset }

	procedure _WEDoArrowKey (arrow: Integer;
									modifiers: Integer;
									hWE: WEHandle);

{ this routine is called by WEKey to handle arrow keys }
{ the WE record is guaranteed to be already locked }

		var
			pWE: WEPtr;
			selStart, selEnd: LongInt;
			caretLoc, anchor: LongInt;
	begin
		pWE := hWE^;

{ get selection range }
		selStart := pWE^.selStart;
		selEnd := pWE^.selEnd;

		if (BitAnd(modifiers, shiftKey) = 0) then
			begin

{ if selection range isn't empty, collapse it to one of the endpoints }
				if (selStart < selEnd) then
					if ((arrow = kArrowLeft) or (arrow = kArrowUp)) then
						caretLoc := selStart
					else
						caretLoc := selEnd
				else

{ otherwise move the insertion point }
					caretLoc := _WEArrowOffset(arrow, selStart, hWE);

{ set anchor to caretLoc, so new selection will be empty }
				anchor := caretLoc;

			end
		else
			begin

{ shift key was held down: extend the selection rather than replacing it }
{ find out which selection boundary is the anchor and which is the free endpoint }
				if BTST(pWE^.flags, weFAnchorIsEnd) then
					begin
						anchor := selEnd;
						caretLoc := selStart;
					end
				else
					begin
						anchor := selStart;
						caretLoc := selEnd;
					end;

{ move the free endpoint }
				caretLoc := _WEArrowOffset(arrow, caretLoc, hWE);

			end;

{ select the new selection }
		WESetSelection(anchor, caretLoc, hWE);

	end;  { _WEDoArrowKey }

	function WEAdjustCursor (mouseLoc: Point;
									mouseRgn: RgnHandle;
									hWE: WEHandle): Boolean;

{ Call WEAdjustCursor to set the cursor shape when the mouse is in the view rectangle. }
{ MouseRgn should be either a valid region handle or NIL. }
{ If mouseRgn is supplied (i.e., if it's not NIL), it is intersected with a region }
{ in global coordinates within which the cursor is to retain its shape. }
{ WEAdjustCursor returns TRUE if the cursor has been set. }
{ Your application should set the cursor only if WEAdjustCursor returns FALSE. }

		var
			pWE: WEPtr;
			auxRgn: RgnHandle;
			portDelta: Point;
			savePort: GrafPtr;
			saveWELock: Boolean;
	begin
		WEAdjustCursor := false;

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ set up the port }
		GetPort(savePort);
		SetPort(pWE^.port);

{ calculate delta between the local coordinate system and the global one }
		portDelta := Point(0);
		LocalToGlobal(portDelta);

{ calculate the visible portion of the view rectangle, in global coordinates }
		auxRgn := NewRgn;
		CopyRgn(pWE^.viewRgn, auxRgn);
		SectRgn(auxRgn, pWE^.port^.visRgn, auxRgn);
		OffsetRgn(auxRgn, portDelta.h, portDelta.v);

		if PtInRgn(mouseLoc, auxRgn) then
			begin

{ mouse is within view rectangle: it's up to us to set the cursor }
				WEAdjustCursor := true;

{ set the cursor to an I-beam }
				SetCursor(GetCursor(iBeamCursor)^^);

{ set mouseRgn, if provided }
				if (mouseRgn <> nil) then
					SectRgn(mouseRgn, auxRgn, mouseRgn);

			end
		else

{ mouse is outside view rectangle: don't set the cursor; subtract viewRgn from mouseRgn }
			if (mouseRgn <> nil) then
				DiffRgn(mouseRgn, auxRgn, mouseRgn);

{ dispose of the temporary region }
		DisposeRgn(auxRgn);

{ restore the port }
		SetPort(savePort);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEAdjustCursor }

	procedure WEIdle (var maxSleep: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			caretInterval, sleepTime: LongInt;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ the caret blinks only if we're active and the selection point is empty }
		if (BTST(pWE^.flags, weFActive) and (pWE^.selStart = pWE^.selEnd)) then
			begin

{ the low-memory global variable CaretTime contains the preferred interval }
{ between successive inversions of the caret }
				caretInterval := GetCaretTime;

{ calculate how many ticks we can sleep before we need to invert the caret }
{ the caretTime field of the WE record contains the time of the last inversion }
				sleepTime := caretInterval - (TickCount - pWE^.caretTime);

{ if sleepTime has gone negative, invert the caret }
				if (sleepTime <= 0) then
					begin
						_WEDrawCaret(hWE);
						sleepTime := caretInterval;
					end;

			end
		else

{ if we don't need to blink the caret, we can sleep forever }
			sleepTime := maxLongInt;

{ return sleepTime to the caller if maxSleep isn't NIL }
		if (@maxSleep <> nil) then
			maxSleep := sleepTime;

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEIdle }

	procedure WEUpdate (updateRgn: RgnHandle;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			firstLine, lastLine: LongInt;
			auxRect: Rect;
			saveClip, auxRgn: RgnHandle;
			savePort: GrafPtr;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ set up the port }
		GetPort(savePort);
		SetPort(pWE^.port);

{ save the clip region }
		saveClip := NewRgn;
		GetClip(saveClip);

{ clip to the insersection between updateRgn and the view rectangle }
{ (updateRgn may be NIL; in this case, just clip to the view rectangle) }
		auxRgn := NewRgn;
		if (updateRgn <> nil) then
			SectRgn(updateRgn, pWE^.viewRgn, auxRgn)
		else
			CopyRgn(pWE^.viewRgn, auxRgn);
		SetClip(auxRgn);

		if (EmptyRgn(auxRgn) = false) then
			begin

{ set auxRect to the bounding box of the update region (clipped to the view rectangle) }
				auxRect := auxRgn^^.rgnBBox;

{ find out which lines need to be redrawn }
				firstLine := _WEPixelToLine(auxRect.top - pWE^.destRect.top, hWE);
				lastLine := _WEPixelToLine((auxRect.bottom - 1) - pWE^.destRect.top, hWE);

{ draw them (if updateRgn is NIL, erase each line rectangle before redrawing) }
				_WEDrawLines(firstLine, lastLine, (updateRgn = nil), hWE);

{ hilite the selection range or draw the caret (only if active) }
				if (pWE^.selStart < pWE^.selEnd) then
					_WEHiliteRange(pWE^.selStart, pWE^.selEnd, hWE)
				else if BTST(pWE^.flags, weFCaretVisible) then
					begin
						_WEDrawCaret(hWE);
						BSET(pWE^.flags, weFCaretVisible);
					end;

			end;  { if not empty }

		DisposeRgn(auxRgn);

{ restore the clip region }
		SetClip(saveClip);
		DisposeRgn(saveClip);

{ restore the port }
		SetPort(savePort);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEUpdate }

	procedure WEDeactivate (hWE: WEHandle);
		var
			pWE: WEPtr;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ hide the selection range or the caret }
		_WEHiliteRange(pWE^.selStart, pWE^.selEnd, hWE);
		if BTST(pWE^.flags, weFCaretVisible) then
			_WEDrawCaret(hWE);

{ clear the active flag }
		BCLR(pWE^.flags, weFActive);

{ frame the selection }
		_WEHiliteRange(pWE^.selStart, pWE^.selEnd, hWE);

{ dispose of the offscreen graphics world, if any }
		if (pWE^.offscreenPort <> nil) then
			begin
				DisposeGWorld(GWorldPtr(pWE^.offscreenPort));
				pWE^.offscreenPort := nil;
			end;

{ notify Text Services }
		if (pWE^.tsmReference <> nil) then
			IgnoreShort(DeactivateTSMDocument(pWE^.tsmReference));

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEDeactivate }

	procedure WEActivate (hWE: WEHandle);
		var
			pWE: WEPtr;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ remove the selection frame }
		_WEHiliteRange(pWE^.selStart, pWE^.selEnd, hWE);

{ set the active flag }
		BSET(pWE^.flags, weFActive);

{ show the selection range }
		_WEHiliteRange(pWE^.selStart, pWE^.selEnd, hWE);

{ notify Text Services }
		if (pWE^.tsmReference <> nil) then
			IgnoreShort(ActivateTSMDocument(pWE^.tsmReference));

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEActivate }

	function WEIsActive (hWE: WEHandle): Boolean;
	begin

{ return TRUE iff the specified WE instance is currently active }
		WEIsActive := BTST(hWE^^.flags, weFActive);

	end;  { WEIsActive }

	procedure CallScrollProc (hWE: WEHandle;
									scrollProc: ProcPtr);
	inline
		$205F,					{ movea.l (sp)+, a0 }
		$4E90;					{ jsr (a0) }

	procedure WEScroll (hOffset, vOffset: LongInt;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			viewRect: Rect;
			updateRgn: RgnHandle;
			savePort: GrafPtr;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ hide the caret if it's showing }
		if BTST(pWE^.flags, weFCaretVisible) then
			_WEDrawCaret(hWE);

{ offset the destination rectangle by the specified amount }
		WEOffsetLongRect(pWE^.destRect, hOffset, vOffset);

{ set up the port }
		GetPort(savePort);
		SetPort(pWE^.port);

		viewRect := pWE^.viewRgn^^.rgnBBox;
		updateRgn := NewRgn;

{ scroll the view rectangle }
		ScrollRect(viewRect, hOffset, vOffset, updateRgn);

{ redraw the exposed region }
		WEUpdate(updateRgn, hWE);
		DisposeRgn(updateRgn);

{ restore the port }
		SetPort(savePort);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEScroll }

	function _WEScrollIntoView (offset: LongInt;
									hWE: WEHandle): Boolean;
		var
			pWE: WEPtr;
			thePoint: LongPoint;
			lineHeight: Integer;
			hScroll, vScroll, temp: LongInt;
	begin
		pWE := hWE^;

{ do nothing if automatic scrolling is disabled }
		if (not BTST(pWE^.flags, weFAutoScroll)) then
			Exit(_WEScrollIntoView);

{ find the selection point }
		WEGetPoint(offset, thePoint, lineHeight, hWE);

{ assume no scrolling is needed }
		_WEScrollIntoView := false;
		vScroll := 0;
		hScroll := 0;

{ determine if we need to scroll vertically }
		if (thePoint.v < pWE^.viewRect.top) or (thePoint.v + lineHeight >= pWE^.viewRect.bottom) then
			begin

{ calculate the amount of vertical scrolling needed to center the selection into view }
				vScroll := BSR(pWE^.viewRect.top + pWE^.viewRect.bottom, 1) - thePoint.v;

{ we'd like to superimpose the bottom margins of the dest/view rects, if possible }
				temp := pWE^.viewRect.bottom - pWE^.destRect.bottom;
				if (temp > vScroll) then
					vScroll := temp;

{ but we also have to make sure the dest top isn't scrolled below the view top }
				temp := pWE^.viewRect.top - pWE^.destRect.top;
				if (temp < vScroll) then
					vScroll := temp;

			end;

{ determine if we need to scroll horizontally }
		if (thePoint.h - 1 < pWE^.viewRect.left) or (thePoint.h >= pWE^.viewRect.right) then
			begin

{ calculate the amount of horizontal scrolling needed to center the selection into view }
				hScroll := BSR(pWE^.viewRect.left + pWE^.viewRect.right, 1) - thePoint.h;

{ we'd like to superimpose the right margins of the dest/view rects, if possible }
				temp := pWE^.viewRect.right - pWE^.destRect.right;
				if (temp > hScroll) then
					hScroll := temp;

{ but we also have to make sure the dest left isn't scrolled to the right of the view left }
				temp := pWE^.viewRect.left - pWE^.destRect.left;
				if (temp < hScroll) then
					hScroll := temp;

			end;

{ scroll the text if necessary }
		if ((vScroll <> 0) or (hScroll <> 0)) then
			begin
				_WEScrollIntoView := true;
				WEScroll(hScroll, vScroll, hWE);
			end;

{ call the scroll callback, if any }
		if (pWE^.scrollProc <> nil) then
			CallScrollProc(hWE, pWE^.scrollProc);

	end;  { _WEScrollIntoView }

	procedure WESelView (hWE: WEHandle);
		var
			pWE: WEPtr;
			offset: LongInt;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ scroll the free endpoint of the selection into view }
		if (BTST(pWE^.flags, weFAnchorIsEnd)) then
			offset := pWE^.selStart
		else
			offset := pWE^.selEnd;
		IgnoreBoolean(_WEScrollIntoView(offset, hWE));

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WESelView }

end.
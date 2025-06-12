unit WASTE4;

{ WASTE PROJECT: }
{ Unit Four: Editing }

{ Copyright © 1993-1994 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WASTE3;

	function WESetStyle (mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle): OSErr;
	function WEUseStyleScrap (styleScrap: WEStyleScrapHandle;
									hWE: WEHandle): OSErr;
	function WEDelete (hWE: WEHandle): OSErr;
	function WEInsert (textPtr: Ptr;
									textLength: LongInt;
									styleScrap: WEStyleScrapHandle;
									hWE: WEHandle): OSErr;
	procedure WEKey (key: Char;
									modifiers: Integer;
									hWE: WEHandle);
	function WECut (hWE: WEHandle): OSErr;
	function WEPaste (hWE: WEHandle): OSErr;
	function _WEDeleteRange (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): OSErr;
	function _WEInsertText (offset: LongInt;
									textPtr: Ptr;
									textLength: LongInt;
									hWE: WEHandle): OSErr;
	function _WEApplyStyleScrap (rangeStart, rangeEnd: LongInt;
									styleScrap: WEStyleScrapHandle;
									hWE: WEHandle): OSErr;
	function _WESetStyleRange (rangeStart, rangeEnd: LongInt;
									mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle): OSErr;
	function _WERedraw (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): OSErr;

implementation

	function _WEInsertRun (runIndex: LongInt;
									offset, styleIndex: LongInt;
									pWE: WEPtr): OSErr;

{ Insert a new element in the style run array, at the specified runIndex position. }
{ The new element consists of the pair <offset, styleIndex>. }

		var
			element: RunArrayElement;
			err: OSErr;
	begin
		_WEInsertRun := noErr;

{ prepare the element record to be inserted in the array }
		element.runStart := offset;
		element.styleIndex := styleIndex;

{ do the insertion }
		err := _WEInsertSlot(pWE^.hRuns, @element, runIndex + 1, SizeOf(element));
		if (err <> noErr) then
			begin
				_WEInsertRun := err;
				Exit(_WEInsertRun);
			end;

{ increment style run count }
		pWE^.nRuns := pWE^.nRuns + 1;

{ increment the reference count field of the style table element }
{ referenced by the newly inserted style run }
		with pWE^.hStyles^^[styleIndex] do
			refCount := refCount + 1;

	end;  { _WEInsertRun }

	function _WERemoveRun (runIndex: LongInt;
									pWE: WEPtr): OSErr;

{ remove the specified element from the style run array }

		var
			styleIndex: LongInt;
	begin

		styleIndex := pWE^.hRuns^^[runIndex].styleIndex;

{ do the removal (errors returned by _WERemoveSlot can be safely ignored) }
		_WERemoveRun := _WERemoveSlot(pWE^.hRuns, runIndex, SizeOf(RunArrayElement));

{ decrement style run count }
		pWE^.nRuns := pWE^.nRuns - 1;

{ decrement the reference count field of the style table element }
{ that was referenced by the style run we have just removed }
		with pWE^.hStyles^^[styleIndex] do
			refCount := refCount - 1;

	end;  { _WERemoveRun }

	procedure _WEChangeRun (runIndex: LongInt;
									newStyleIndex: LongInt;
									pWE: WEPtr);

{ change the styleIndex field of the specified element of the style run array }

		var
			oldStyleIndex: LongInt;
	begin

{ do the change }
		with pWE^.hRuns^^[runIndex] do
			begin
				oldStyleIndex := styleIndex;
				styleIndex := newStyleIndex;
			end;

{ decrement the reference count field of the old style table element }
		with pWE^.hStyles^^[oldStyleIndex] do
			refCount := refCount - 1;

{ increment the reference count field of the new style table element }
		with pWE^.hStyles^^[newStyleIndex] do
			refCount := refCount + 1;

	end;  { _WEChangeRun }

	function _WENewStyle (var ts: WERunAttributes;
									var styleIndex: LongInt;
									pWE: WEPtr): OSErr;

{ given the specified WERunAttributes record, find the corresponding entry }
{ in the style table (create a new entry if necessary), and return its index }

		var
			pTable: StyleTablePtr;
			element: StyleTableElement;
			index, unusedIndex: LongInt;
			err: OSErr;
	begin
		_WENewStyle := noErr;
		pTable := pWE^.hStyles^;

{ see if the given style already exists in the style table }
{ while scanning the table, also remember the position of the first unused style, if any }
		index := 0;
		unusedIndex := -1;
		while (index < pWE^.nStyles) do
			begin

{ perform a bitwise comparison between the current element and the specified style }
				if _WEBlockCmp(@pTable^[index].info, @ts, SizeOf(ts)) then
					begin
						styleIndex := index;		{ found: style already present }
						Exit(_WENewStyle);
					end;

{ check for entries which aren't referenced and can be recycled }
				if (pTable^[index].refCount = 0) then
					unusedIndex := index;

				index := index + 1;
			end;  { while }

{ the specified style doesn't exist in the style table }
{ see if we can recycle an unused entry }
		if (unusedIndex >= 0) then
			begin
				index := unusedIndex;
				pTable^[index].info := ts;
			end
		else
			begin

{ no reusable entry: we have to append a new element to the table }
				element.refCount := 0;
				element.info := ts;
				err := _WEInsertSlot(pWE^.hStyles, @element, index, SizeOf(element));
				if (err <> noErr) then
					begin
						_WENewStyle := err;
						Exit(_WENewStyle);
					end;

{ update style count in the WE record }
				pWE^.nStyles := index + 1;

			end;

{ return the index to the new element }
		styleIndex := index;

	end;  { _WENewStyle }

	function _WERedraw (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): OSErr;

{ the WE record is guaranteed to be already locked }

		label
			1;
		var
			pWE: WEPtr;
			pLines: LineArrayPtr;
			startLine, endLine: LongInt;
			oldTextHeight, newTextHeight: LongInt;
			r: LongRect;
			viewRect, updateRect: Rect;
			saveClip: RgnHandle;
			savePort: GrafPtr;
			err: OSErr;
	begin
		pWE := hWE^;

{ do nothing if recalculation has been inhibited }
		if (not BTST(pWE^.flags, weFInhibitRecal)) then
			begin

{ hide the caret }
				if BTST(pWE^.flags, weFCaretVisible) then
					_WEDrawCaret(hWE);

{ remember total text height }
				oldTextHeight := pWE^.destRect.bottom - pWE^.destRect.top;

{ find line range affected by modification }
				startLine := _WEOffsetToLine(rangeStart, hWE);
				endLine := _WEOffsetToLine(rangeEnd, hWE);

{ recalculate line breaks starting from startLine }
				err := _WERecalBreaks(startLine, endLine, hWE);
				if (err <> noErr) then
					goto 1;

{ recalculate slops }
				_WERecalSlops(startLine, endLine, hWE);

{ calculate new total text height }
				newTextHeight := pWE^.destRect.bottom - pWE^.destRect.top;

{ calculate the rectangle to redraw (in long coordinates) }
				r.left := -maxint;
				r.right := maxint;
				pLines := pWE^.hLines^;
				r.top := pLines^[startLine].lineOrigin;

{ if total text height hasn't changed, it's enough to redraw lines up to endLine }
{ otherwise we must redraw all lines from startLine on }
				if ((newTextHeight = oldTextHeight) and (endLine < pWE^.nLines - 1)) then
					r.bottom := pLines^[endLine + 1].lineOrigin
				else if (newTextHeight < oldTextHeight) then
					r.bottom := oldTextHeight
				else
					r.bottom := newTextHeight;

				WEOffsetLongRect(r, 0, pWE^.destRect.top);

{ calculate the intersection between this rectangle and the view rectangle }
				WELongRectToRect(r, updateRect);
				WELongRectToRect(pWE^.viewRect, viewRect);

				if SectRect(updateRect, viewRect, updateRect) then
					begin

{ set up the port and the clip region }
						GetPort(savePort);
						SetPort(pWE^.port);

{ set the clip region to updateRect }
						saveClip := NewRgn;
						GetClip(saveClip);
						ClipRect(updateRect);

{ we only really need to redraw the visible lines }
						startLine := _WEPixelToLine(updateRect.top - pWE^.destRect.top, hWE);
						endLine := _WEPixelToLine(updateRect.bottom - pWE^.destRect.top - 1, hWE);

{ redraw the lines (pass TRUE in the doErase parameter) }
						_WEDrawLines(startLine, endLine, true, hWE);

{ erase the portion of the update rectangle below the last line (if any) }
						pLines := pWE^.hLines^;
						updateRect.top := pWE^.destRect.top + pLines^[endLine + 1].lineOrigin;
						if (updateRect.top < updateRect.bottom) then
							EraseRect(updateRect);

{ restore the clip region }
						SetClip(saveClip);
						DisposeRgn(saveClip);

{ restore the port }
						SetPort(savePort);

{ redraw the caret or the selection range }
						if (pWE^.selStart < pWE^.selEnd) then
							_WEHiliteRange(pWE^.selStart, pWE^.selEnd, hWE)
						else
							_WEDrawCaret(hWE);

					end;  { if SectRect }

{ scroll the selection range into view }
				WESelView(hWE);

			end;  { if recal not inhibited }

{ clear result code }
		err := noErr;

1:
{ return result code }
		_WERedraw := err;

	end;  { _WERedraw }

	function _WESetStyleRange (rangeStart, rangeEnd: LongInt;
									mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle): OSErr;

{ alter the style attributes of the specified text range according to ts and mode }
{ the WE record is guaranteed to be already locked }

		label
			1;
		var
			pWE: WEPtr;
			hRuns: RunArrayHandle;
			offset: LongInt;
			runIndex: LongInt;
			oldStyleIndex, newStyleIndex: LongInt;
			runInfo: WERunInfo;
			temp: Integer;
			continuousStyles: SignedByte;
			err: OSErr;
	begin
		pWE := hWE^;
		hRuns := pWE^.hRuns;

{ if mode contains weDoToggleFace, we need to determine which QuickDraw styles }
{ are continuous over the specified text range: those styles must be turned off }
		if BTST(mode, kModeToggleFace) then
			begin
				temp := weDoFace;
				_WEContinuousStyleRange(rangeStart, rangeEnd, temp, runInfo.runAttrs.runStyle, hWE);
				continuousStyles := runInfo.runAttrs.tsFace;
			end
		else
			continuousStyles := 0;

{ find the index to the first style run in the specified range }
		offset := rangeStart;
		runIndex := _WEOffsetToRun(offset, hWE);

{ run thru all the style runs that encompass the selection range }
		repeat

{ find style index for this run and retrieve corresponding style attributes }
			oldStyleIndex := hRuns^^[runIndex].styleIndex;
			_WEGetIndStyle(runIndex, runInfo, hWE);

{ _WEGetIndStyle returns textLength + 1 in runInfo.runEnd for the last style run: }
{ correct this anomaly (which is useful for other purposes, anyway) }
			if (runInfo.runEnd > pWE^.textLength) then
				runInfo.runEnd := pWE^.textLength;

{ apply changes to existing style attributes as requested }
			_WECopyStyle(ts, runInfo.runAttrs.runStyle, continuousStyles, mode);

{ recalculate font metrics, if necessary }
			if (BitAnd(mode, weDoFont + weDoSize + weDoFace + weDoAddSize) <> 0) then
				_WEFillFontInfo(pWE^.port, runInfo.runAttrs);

{ get a style index for the new attributes }
			err := _WENewStyle(runInfo.runAttrs, newStyleIndex, pWE);
			if (err <> noErr) then
				goto 1;

{ if offset falls on a style boundary and this style run has become identical }
{ to the previous one, merge the two runs together }
			if (offset = runInfo.runStart) & (runIndex > 0) & (hRuns^^[runIndex - 1].styleIndex = newStyleIndex) then
				begin
					err := _WERemoveRun(runIndex, pWE);
					if (err <> noErr) then
						goto 1;
					runIndex := runIndex - 1;
				end;

{ style index changed? }
			if (oldStyleIndex <> newStyleIndex) then
				begin

{ if offset is in the middle of a style run, insert a new style run in the run array }
					if (offset > runInfo.runStart) then
						begin
							err := _WEInsertRun(runIndex, offset, newStyleIndex, pWE);
							if (err <> noErr) then
								goto 1;
							runIndex := runIndex + 1;
						end
					else

{ otherwise just change the styleIndex field of the current style run element }
						_WEChangeRun(runIndex, newStyleIndex, pWE);

{ if specified range ends in the middle of a style run, insert yet another element }
					if (rangeEnd < runInfo.runEnd) then
						begin
							err := _WEInsertRun(runIndex, rangeEnd, oldStyleIndex, pWE);
							if (err <> noErr) then
								goto 1;
						end;

				end;  { if oldStyle <> newStyle }

{ go to next style run }
			runIndex := runIndex + 1;
			offset := runInfo.runEnd;

		until (offset >= rangeEnd);

{ if the last style run ends exactly at the end of the specified range, }
{ see if we can merge it with the following style run }
		if ((offset = rangeEnd) & (runIndex < pWE^.nRuns) & (hRuns^^[runIndex].styleIndex = newStyleIndex)) then
			begin
				err := _WERemoveRun(runIndex, pWE);
				if (err <> noErr) then
					goto 1;
			end;

{ clear result code }
		err := noErr;

1:
{ return result code }
		_WESetStyleRange := err;

	end;  { _WESetStyleRange }

	function _WEApplyStyleScrap (rangeStart, rangeEnd: LongInt;
									styleScrap: WEStyleScrapHandle;
									hWE: WEHandle): OSErr;

{ apply the given style scrap to the specified text range }
{ the WE record is guaranteed to be already locked }

		var
			pWE: WEPtr;
			pElement: WEStyleScrapPeek;
			runStart, runEnd: LongInt;
			index, lastElement: Integer;
			ts: WETextStyle;
			err: OSErr;
	begin
		_WEApplyStyleScrap := noErr;
		pWE := hWE^;

{ loop through each element of the style scrap }
		lastElement := styleScrap^^.scrpNStyles - 1;
		for index := 0 to lastElement do
			begin

{ get a pointer to the current scrap element }
				pElement := @styleScrap^^.scrpStyleTab[index];

{ calculate text run to which this element is to be applied }
				runStart := rangeStart + pElement^.first.scrpStartChar;
				if (index < lastElement) then
					runEnd := rangeStart + pElement^.second.scrpStartChar
				else
					runEnd := rangeEnd;

{ perform some range checking }
				if (runEnd > rangeEnd) then
					runEnd := rangeEnd;
				if (runStart >= runEnd) then
					Cycle;

{ copy style to a local variable in case memory moves }
				ts := pElement^.first.scrpAttrs.runStyle;

{ apply the specified style to the range }
				err := _WESetStyleRange(runStart, runEnd, weDoAll + weDoReplaceFace, ts, hWE);
				if (err <> noErr) then
					begin
						_WEApplyStyleScrap := err;
						Exit(_WEApplyStyleScrap);
					end;

			end;  { for }
	end;  { _WEApplyStyleScrap }

	procedure _WEBumpRunStart (runIndex: LongInt;
									deltaRunStart: LongInt;
									pWE: WEPtr);

{ add deltaLineStart to the lineStart field of all line records }
{ starting from lineIndex }

		var
			pStart: LongIntPtr;
			nRuns: LongInt;
	begin
		pStart := @pWE^.hRuns^^[runIndex].runStart;
		nRuns := pWE^.nRuns;

{ loop through the style run array adjusting the runStart fields }
		while (runIndex <= nRuns) do
			begin
				pStart^ := pStart^ + deltaRunStart;
				pStart := LongIntPtr(LongInt(pStart) + SizeOf(RunArrayElement));
				runIndex := runIndex + 1;
			end;
	end;  { _WEBumpRunStart }

	function _WERemoveRunRange (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): OSErr;

{ the range of text between rangeStart and rangeEnd is being deleted }
{ update the style run array (and the style table) accordingly }
{ the WE handle must be locked on entry }

		label
			1;
		var
			pWE: WEPtr;
			pRuns: RunArrayPeek;
			startRun, endRun: LongInt;
			err: OSErr;
	begin
		pWE := hWE^;

{ find the index to the first and last style runs in the specified range }
		startRun := _WEOffsetToRun(rangeStart, hWE);
		endRun := _WEOffsetToRun(rangeEnd, hWE) - 1;

{ remove all style runs between startRun and endRun }
		while (endRun > startRun) do
			begin
				err := _WERemoveRun(endRun, pWE);
				if (err <> noErr) then
					goto 1;
				endRun := endRun - 1;
			end;

{ move back all subsequent style runs }
		_WEBumpRunStart(startRun + 1, rangeStart - rangeEnd, pWE);

		if (endRun = startRun) and (endRun < pWE^.nRuns - 1) then
			begin
				pRuns := @pWE^.hRuns^^[endRun];
				pRuns^.second.runStart := rangeStart;
			end;

{ remove the first style run if is has become zero length }
		pRuns := @pWE^.hRuns^^[startRun];
		if (pRuns^.first.runStart = pRuns^.second.runStart) then
			begin
				err := _WERemoveRun(startRun, pWE);
				if (err <> noErr) then
					goto 1;
				startRun := startRun - 1;
			end;

{ merge the first and last runs if they have the same style index }
		if (startRun >= 0) then
			begin
				pRuns := @pWE^.hRuns^^[startRun];
				if (pRuns^.first.styleIndex = pRuns^.second.styleIndex) then
					begin
						err := _WERemoveRun(startRun + 1, pWE);
						if (err <> noErr) then
							goto 1;
					end;
			end;

{ clear result code }
		err := noErr;

1:
{ return result code }
		_WERemoveRunRange := err;

	end;  { _WERemoveRunRange }

	procedure _WEBumpLineStart (lineIndex: LongInt;
									deltaLineStart: LongInt;
									pWE: WEPtr);

{ add deltaLineStart to the lineStart field of all line records }
{ starting from lineIndex }

		var
			pStart: LongIntPtr;
			nLines: LongInt;
	begin
		pStart := @pWE^.hLines^^[lineIndex].lineStart;
		nLines := pWE^.nLines;

{ loop through the line array adjusting the lineStart fields }
		while (lineIndex <= nLines) do
			begin
				pStart^ := pStart^ + deltaLineStart;
				pStart := LongIntPtr(LongInt(pStart) + SizeOf(LineRec));
				lineIndex := lineIndex + 1;
			end;
	end;  { _WEBumpLineStart }

	function _WERemoveLineRange (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): OSErr;

{ the range of text between rangeStart and rangeEnd is being deleted }
{ update the line array accordingly }
{ the WE handle must be locked on entry }

		var
			pWE: WEPtr;
			startLine, endLine: LongInt;
			err: OSErr;
	begin
		_WERemoveLineRange := noErr;
		pWE := hWE^;

{ remove all line records between rangeStart and rangeEnd }
		startLine := _WEOffsetToLine(rangeStart, hWE) + 1;
		endLine := _WEOffsetToLine(rangeEnd, hWE);
		while (endLine >= startLine) do
			begin
				err := _WERemoveLine(endLine, pWE);
				if (err <> noErr) then
					begin
						_WERemoveLineRange := err;
						Exit(_WERemoveLineRange);
					end;
				endLine := endLine - 1;
			end;  { while }

{ update the lineStart field of all the line records that follow }
		_WEBumpLineStart(startLine, rangeStart - rangeEnd, pWE);

	end;  { _WERemoveLineRange }

	function _WEDeleteRange (rangeStart, rangeEnd: LongInt;
									hWE: WEHandle): OSErr;

{ used internally to delete a text range }
{ if saveNullStyle is TRUE, the first style in the range is saved in nullStyle }
{ the WE record is guaranteed to be already locked }

		label
			0, 1;
		var
			pWE: WEPtr;
			runInfo: WERunInfo;
			oldTextLength, newTextLength: LongInt;
			pText: LongInt;
			err: OSErr;
	begin
		pWE := hWE^;

{ do nothing if the specified range is empty }
		if (rangeStart = rangeEnd) then
			goto 0;

{ save the first style in the specified range in nullStyle }
		WEGetRunInfo(rangeStart, runInfo, hWE);
		pWE^.nullStyle := runInfo.runAttrs;
		BSET(pWE^.flags, weFUseNullStyle);

{ remove all line records between rangeStart and rangeEnd }
		err := _WERemoveLineRange(rangeStart, rangeEnd, hWE);
		if (err <> noErr) then
			goto 1;

{ remove all style runs between rangeStart and rangeEnd }
		err := _WERemoveRunRange(rangeStart, rangeEnd, hWE);
		if (err <> noErr) then
			goto 1;

{ calculate old and new text length }
		oldTextLength := pWE^.textLength;
		newTextLength := oldTextLength - (rangeEnd - rangeStart);

{ move the end of the text backwards over the old selection range }
		pText := LongInt(pWE^.hText^);
		%_BlockMoveData(Ptr(pText + rangeEnd), Ptr(pText + rangeStart), oldTextLength - rangeEnd);

{ compact the text handle }
		err := %_SetHandleSize(pWE^.hText, newTextLength);
		if (err <> noErr) then
			goto 1;

{ update textLength field }
		pWE^.textLength := newTextLength;

0:
{ clear result code }
		err := noErr;

1:
{ return result code }
		_WEDeleteRange := err;

	end;  { _WEDeleteRange }

	function _WEInsertText (offset: LongInt;
									textPtr: Ptr;
									textLength: LongInt;
									hWE: WEHandle): OSErr;

{ this routine assumes that the WE record is already locked }

		label
			0, 1;
		var
			pWE: WEPtr;
			oldTextLength, newTextLength: LongInt;
			pInsPoint: LongInt;
			err: OSErr;
	begin
		pWE := hWE^;

{ do nothing if textLength is zero or negative }
		if (textLength <= 0) then
			goto 0;

{ calculate old and new length of text handle }
		oldTextLength := pWE^.textLength;
		newTextLength := oldTextLength + textLength;

{ lengthen the raw text handle }
		err := %_SetHandleSize(pWE^.hText, newTextLength);
		if (err <> noErr) then
			goto 1;

{ calculate ptr to insertion point }
		pInsPoint := LongInt(pWE^.hText^) + offset;

{ make room for the new text }
		%_BlockMoveData(Ptr(pInsPoint), Ptr(pInsPoint + textLength), oldTextLength - offset);

{ insert new text at the insertion point }
		%_BlockMoveData(textPtr, Ptr(pInsPoint), textLength);

{ update the lineStart fields of all lines following the insertion point }
		_WEBumpLineStart(_WEOffsetToLine(offset, hWE) + 1, textLength, pWE);

{ update the runStart fields of all style runs following the insertion point }
		_WEBumpRunStart(_WEOffsetToRun(offset - 1, hWE) + 1, textLength, pWE);

{ update various fields in the WE record }
		pWE^.textLength := newTextLength;

{ if there is a valid null style, apply it to the newly inserted text }
		if BTST(pWE^.flags, weFUseNullStyle) then
			begin
				err := _WESetStyleRange(offset, offset + textLength, weDoAll + weDoReplaceFace, pWE^.nullStyle.runStyle, hWE);
				if (err <> noErr) then
					goto 1;
			end;

0:
{ clear result code }
		err := noErr;

1:
{ return result code }
		_WEInsertText := err;

	end;  { _WEInsertText }

	function WEDelete (hWE: WEHandle): OSErr;
		var
			pWE: WEPtr;
			offset: LongInt;
			runInfo: WERunInfo;
			saveWELock: Boolean;
			err: OSErr;
	begin
		WEDelete := noErr;

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;
		offset := pWE^.selStart;

{ do nothing if the selection range is empty }
		if (offset < pWE^.selEnd) then
			begin

{ delete the selection range }
				err := _WEDeleteRange(offset, pWE^.selEnd, hWE);
				if (err <> noErr) then
					begin
						WEDelete := err;
						Exit(WEDelete);
					end;

{ selEnd becomes equal to selStart }
				pWE^.selEnd := offset;

{ redraw the text }
				err := _WERedraw(offset, offset, hWE);
				if (err <> noErr) then
					begin
						WEDelete := err;
						Exit(WEDelete);
					end;

			end;  { if non-empty selection }

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEDelete }

	function WEInsert (textPtr: Ptr;
									textLength: LongInt;
									styleScrap: WEStyleScrapHandle;
									hWE: WEHandle): OSErr;
		label
			1;
		var
			pWE: WEPtr;
			offset, endOffset: LongInt;
			saveWELock: Boolean;
			err: OSErr;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;
		offset := pWE^.selStart;

{ delete current selection }
		err := _WEDeleteRange(offset, pWE^.selEnd, hWE);
		if (err <> noErr) then
			goto 1;

{ insert the new text at the insertion point }
		err := _WEInsertText(offset, textPtr, textLength, hWE);
		if (err <> noErr) then
			goto 1;

{ move the insertion point at the end of the inserted text }
		endOffset := offset + textLength;
		pWE^.selStart := endOffset;
		pWE^.selEnd := endOffset;

		if (styleScrap <> nil) then
			begin

{ if a styleScrap was supplied, apply it to the newly inserted text }
				err := _WEApplyStyleScrap(offset, endOffset, styleScrap, hWE);
				if (err <> noErr) then
					goto 1;
			end;

{ invalid the null style }
		BCLR(pWE^.flags, weFUseNullStyle);

{ redraw the text }
		err := _WERedraw(offset, endOffset, hWE);
		if (err <> noErr) then
			goto 1;

{ clear result code }
		err := noErr;

1:
{ return result code }
		WEInsert := err;

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEInsert }

	procedure _WEInsertByte (theByte: SignedByte;
									hWE: WEHandle);
		type
			DoubleByte = packed record
					firstByte: SignedByte;
					secondByte: SignedByte;
				end;
		var
			pWE: WEPtr;
			db: DoubleByte;
			charLength: LongInt;
			byteType: Integer;
			saveFont: Integer;
			savePort: GrafPtr;
	begin
		pWE := hWE^;
		charLength := 1;				{ assume 1-byte character by default }
		db.firstByte := theByte;

{ delete current selection, if any }
		IgnoreShort(_WEDeleteRange(pWE^.selStart, pWE^.selEnd, hWE));
		pWE^.selEnd := pWE^.selStart;

{ make sure the font script is synchronized with the keyboard script }
		_WESynchNullStyle(hWE);

		if BTST(pWE^.flags, weFDoubleByte) then
			begin

{ special processing for double-byte characters }
				if (pWE^.firstByte <> 0) then
					begin

{ if this byte is the second half of a double-byte character, }
{ insert the two bytes at the same time (flush the double-byte cache) }
						db.firstByte := pWE^.firstByte;
						db.secondByte := theByte;
						charLength := 2;
						pWE^.firstByte := 0;
					end
				else
					begin

{ determine the byte-type of theByte; first set up the port and its font }
						GetPort(savePort);
						SetPort(pWE^.port);
						saveFont := pWE^.port^.txFont;
						TextFont(pWE^.nullStyle.tsFont);

{ call CharByte }
						byteType := CharByte(@theByte, 0);

{ put back font and port }
						TextFont(saveFont);
						SetPort(savePort);

{ if theByte is the first half of a double-byte character, just cache it and exit }
						if (byteType = smFirstByte) then
							begin
								pWE^.firstByte := theByte;
								Exit(_WEInsertByte);
							end;
					end;

			end;  { if double-byte script installed }

{ insert the new character into the text }
		IgnoreShort(WEInsert(@db, charLength, nil, hWE));

	end;  { _WEInsertByte }

	procedure _WEBackspace (hWE: WEHandle);

{ this routine is called by WEKey to handle the backspace key }
{ the WE record is guaranteed to be already locked }

		var
			pWE: WEPtr;
			offset, charLength: LongInt;
			err: OSErr;
	begin
		pWE := hWE^;
		offset := pWE^.selStart;

{ if the selection range is not empty, delete the current selection range }
		if (offset < pWE^.selEnd) then
			err := _WEDeleteRange(offset, pWE^.selEnd, hWE)
		else
			begin

{ do nothing if insertion point is at the beginning of the text }
				if (offset <= 0) then
					Exit(_WEBackspace);

{ determine the byte-type of the character preceding the insertion point }
				if (WECharByte(offset - 1, hWE) = smSingleByte) then
					charLength := 1
				else
					charLength := 2;

{ delete the character }
				offset := offset - charLength;
				err := _WEDeleteRange(offset, offset + charLength, hWE);
			end;

{ keep track of current selection range }
		pWE^.selStart := offset;
		pWE^.selEnd := offset;

{ redraw the text }
		err := _WERedraw(offset, offset, hWE);

	end;  { _WEBackspace }

	procedure WEKey (key: Char;
									modifiers: Integer;
									hWE: WEHandle);
		var
			pWE: WEPtr;
			saveWELock: Boolean;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ hide the caret if it's showing }
		if BTST(pWE^.flags, weFCaretVisible) then
			_WEDrawCaret(hWE);

{ hide the cursor (it will show again as soon as it's moved) }
		ObscureCursor;

{ dispatch on key class (arrow keys, printable characters, backspace) }
		if ((ORD(key) >= kArrowLeft) and (ORD(key) <= kArrowDown)) then
			_WEDoArrowKey(ORD(key), modifiers, hWE)
		else if (ORD(key) = kBackspace) then
			_WEBackspace(hWE)
		else
			_WEInsertByte(ORD(key), hWE);

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEKey }

	function WECut (hWE: WEHandle): OSErr;
		var
			err: OSErr;
	begin
		WECut := noErr;

{ Cut is just Copy + Delete }
		err := WECopy(hWE);
		if (err <> noErr) then
			begin
				WECut := err;
				Exit(WECut);
			end;

		err := WEDelete(hWE);
		if (err <> noErr) then
			begin
				WECut := err;
				Exit(WECut);
			end;
	end;  { WECut }

	function WESetStyle (mode: Integer;
									var ts: WETextStyle;
									hWE: WEHandle): OSErr;
		label
			1;
		var
			pWE: WEPtr;
			fontScript: ScriptCode;
			saveWELock: Boolean;
			err: OSErr;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ if the selection range is empty, set the null style }
		if (pWE^.selStart = pWE^.selEnd) then
			begin

{ first make sure the nullStyle field contains valid information }
				_WESynchNullStyle(hWE);

{ apply style changes to the nullStyle record }
				_WECopyStyle(ts, pWE^.nullStyle.runStyle, pWE^.nullStyle.tsFace, mode);

{ if the font was altered, synchronize the keyboard script }
				if BTST(pWE^.flags, weFNonRoman) then
					if BTST(mode, kModeFont) then
						begin
							fontScript := Font2Script(pWE^.nullStyle.tsFont);
							if (fontScript <> GetEnvirons(smKeyScript)) then
								KeyScript(fontScript);
						end;

			end
		else
			begin

{ otherwise set the style of the selection range }
				err := _WESetStyleRange(pWE^.selStart, pWE^.selEnd, mode, ts, hWE);
				if (err <> noErr) then
					goto 1;

{ and redraw the text }
				err := _WERedraw(pWE^.selStart, pWE^.selEnd, hWE);
				if (err <> noErr) then
					goto 1;

			end;

{ clear the result code }
		err := noErr;

1:
{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

{ return result code }
		WESetStyle := err;

	end;  { WESetStyle }

	function WEUseStyleScrap (styleScrap: WEStyleScrapHandle;
									hWE: WEHandle): OSErr;
		label
			1;
		var
			pWE: WEPtr;
			saveWELock: Boolean;
			err: OSErr;
	begin

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ apply the style scrap to the selection range }
		err := _WEApplyStyleScrap(pWE^.selStart, pWE^.selEnd, styleScrap, hWE);
		if (err <> noErr) then
			goto 1;

{ redraw the text }
		err := _WERedraw(pWE^.selStart, pWE^.selEnd, hWE);

1:
{ return result code }
		WEUseStyleScrap := err;

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

	end;  { WEUseStyleScrap }

	function WEPaste (hWE: WEHandle): OSErr;
		label
			1;
		var
			hText, hStyles: Handle;
			scrapResult, scrapOffset: LongInt;
			err: OSErr;
	begin
		hText := nil;
		hStyles := nil;

{ allocate a handle to hold the text }
		err := _WEAllocate(0, kAllocTemp, hText);
		if (err <> noErr) then
			goto 1;

{ look for a 'TEXT' item }
		scrapResult := GetScrap(hText, kTypeText, scrapOffset);
		if (scrapResult <= 0) then
			begin
				err := scrapResult;
				goto 1;
			end;

{ allocate a handle to hold the style scrap }
		err := _WEAllocate(0, kAllocTemp, hStyles);
		if (err <> noErr) then
			goto 1;

{ look for a 'styl' item accompanying the text }
		scrapResult := GetScrap(hStyles, kTypeStyles, scrapOffset);

{ forget the handle if nothing was found or an error occurred }
		if (scrapResult <= 0) then
			_WEForgetHandle(hStyles);

{ lock down the text }
		HLock(hText);

{ insert the text }
		err := WEInsert(hText^, %_GetHandleSize(hText), WEStyleScrapHandle(hStyles), hWE);

1:
{ return result code }
		WEPaste := err;

{ clean up }
		_WEForgetHandle(hText);
		_WEForgetHandle(hStyles);

	end;  { WEPaste }

end.
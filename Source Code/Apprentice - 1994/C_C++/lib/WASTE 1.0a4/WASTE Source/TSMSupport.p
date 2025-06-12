unit TSMSupport;

{ WASTE PROJECT: }
{ Text Services Manager support }

{ Copyright © 1993-1994 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WASTE4;

	function WEInstallTSMHandlers: OSErr;

implementation
	uses
		TextServices;

{$IFC WASTE_SEGMENT}
{$S WASTE_TSM_SUPPORT}
{$ENDC}

	procedure CallPreUpdate (hWE: WEHandle;
									preUpdateProcPtr: ProcPtr);
	inline
		$205F,					{ movea.l (sp)+, a0 }
		$4E90;					{ jsr (a0) }

	procedure CallPostUpdate (hWE: WEHandle;
									fixLength: LongInt;
									inputAreaStart, inputAreaEnd: LongInt;
									pinRangeStart, pinRangeEnd: LongInt;
									postUpdateProcPtr: ProcPtr);
	inline
		$205F,					{ movea.l (sp)+, a0 }
		$4E90;					{ jsr (a0) }

	function _WEHiliteRangeArray (hTray: TextRangeArrayHandle;
									hWE: WEHandle): OSErr;
		label
			1;
		var
			pWE: WEPtr;
			pRange: TextRangePtr;
			rangeStart, rangeEnd: LongInt;
			hiliteStyle: Integer;
			ts: WETextStyle;
			rangeIndex: Integer;
			saveTrayLock: Boolean;
			err: OSErr;
	begin
		pWE := hWE^;

{ lock down the range array }
		saveTrayLock := _WESetHandleLock(hTray, true);
		rangeIndex := hTray^^.fNumOfRanges - 1;
		pRange := @hTray^^.fRange;

{ walk the hilite range array }
		while (rangeIndex >= 0) do
			begin

{ the offsets in the range array are relative to the beginning }
{ of the active input area: convert them to absolute offsets }
				rangeStart := pWE^.tsmAreaStart + pRange^.fStart;
				rangeEnd := pWE^.tsmAreaStart + pRange^.fEnd;
				hiliteStyle := pRange^.fHiliteStyle;

{ take the absolute value of hiliteStyle }
				hiliteStyle := ABS(hiliteStyle);

{ if hiliteStyle is kCaretPosition, set the selection range }
				if (hiliteStyle = kCaretPosition) then
					begin
						pWE^.selStart := rangeStart;
						pWE^.selEnd := rangeEnd;
					end
				else
					begin
						hiliteStyle := hiliteStyle - kRawText;

{ otherwise set the WETextStyle flags of the specified range appropriately }
						if ((hiliteStyle >= 0) and (hiliteStyle <= 3)) then
							begin
								ts.tsFlags := $10 + BSL(hiliteStyle, tsTSMSelected);
								err := _WESetStyleRange(rangeStart, rangeEnd, weDoFlags, ts, hWE);
								if (err <> noErr) then
									goto 1;
							end;
					end;

{ go to next text range element }
				rangeIndex := rangeIndex - 1;
				pRange := TextRangePtr(LongInt(pRange) + SizeOf(TextRange));
			end;  { while }

{ clear result code }
		err := noErr;

1:
{ unlock the range array }
		IgnoreBoolean(_WESetHandleLock(hTray, saveTrayLock));

{ return result code }
		_WEHiliteRangeArray := err;

	end;  { _WEHiliteRangeArray }

	function _WEHandleUpdateActiveInputArea (var ae, reply: AppleEvent;
									handlerRefCon: LongInt): OSErr;
		label
			1;
		var
			hWE: WEHandle;
			pWE: WEPtr;
			text: AEDesc;
			hiliteTray: AEDesc;
			pinRange: TextRange;
			totalLength: LongInt;
			fixLength: LongInt;
			tsmOffset: LongInt;
			returnedType: DescType;
			actualSize: LongInt;
			savePort: GrafPtr;
			saveAutoScroll: Boolean;
			saveTextLock: Boolean;
			saveWELock: Boolean;
			err: OSErr;
	begin
		hWE := nil;

{ initialize descriptors to null values }
		text.descriptorType := typeNull;
		text.dataHandle := nil;
		hiliteTray.descriptorType := typeNull;
		hiliteTray.dataHandle := nil;

{ extract WE handle }
		err := AEGetParamPtr(ae, keyAETSMDocumentRefCon, typeLongInteger, returnedType, @hWE, SizeOf(hWE), actualSize);
		if (err <> noErr) then
			goto 1;

{ lock the WE handle }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ call the pre-update callback, if present }
		if (pWE^.tsmPreUpdate <> nil) then
			CallPreUpdate(hWE, pWE^.tsmPreUpdate);

{ hide the caret if it's showing }
		if BTST(pWE^.flags, weFCaretVisible) then
			_WEDrawCaret(hWE);

{ extract the text descriptor }
		err := AEGetParamDesc(ae, keyAETheData, typeChar, text);
		if (err <> noErr) then
			goto 1;

{ get total length of text in the active input area }
		totalLength := %_GetHandleSize(text.dataHandle);

{ extract the length of confirmed text in the active input area }
		err := AEGetParamPtr(ae, keyAEFixLength, typeLongInteger, returnedType, @fixLength, SizeOf(fixLength), actualSize);
		if (err <> noErr) then
			goto 1;

{ if fixLength = -1, all text is confirmed }
		if (fixLength = -1) then
			totalLength := fixLength;

{ if there's currently no active input area, open one }
		if (pWE^.tsmAreaStart = -1) then
			begin
				pWE^.tsmAreaStart := pWE^.selStart;
				pWE^.tsmAreaEnd := pWE^.selEnd;
			end;

		tsmOffset := pWE^.tsmAreaStart;

{ the new text replaces whatever is in the active input area }
		err := _WEDeleteRange(tsmOffset, pWE^.tsmAreaEnd, hWE);
		if (err <> noErr) then
			goto 1;

{ synchronize the null style, so font script matches the keyboard script }
		_WESynchNullStyle(hWE);

{ set the port font for good measure }
		GetPort(savePort);
		SetPort(pWE^.port);
		TextFont(pWE^.nullStyle.tsFont);
		SetPort(savePort);

{ temporarily lock the text }
		saveTextLock := _WESetHandleLock(text.dataHandle, true);

{ insert the text }
		err := _WEInsertText(tsmOffset, text.dataHandle^, totalLength, hWE);
		if (err <> noErr) then
			goto 1;

{ unlock the text }
		IgnoreBoolean(_WESetHandleLock(text.dataHandle, saveTextLock));

{ extract pin range }
		err := AEGetParamPtr(ae, keyAEPinRange, typeTextRange, returnedType, @pinRange, SizeOf(pinRange), actualSize);
		if (err = noErr) then
			begin

{ we want absolute offsets }
				pinRange.fStart := pinRange.fStart + tsmOffset;
				pinRange.fEnd := pinRange.fEnd + tsmOffset;
			end
		else
			begin

{ a missing pin range descriptor isn't an error; everything else is }
				if (err <> errAEDescNotFound) then
					goto 1;

{ default pin range is active input area }
				pinRange.fStart := tsmOffset;
				pinRange.fEnd := pWE^.tsmAreaEnd;
			end;

{ extract the highlight range array }
		err := AEGetParamDesc(ae, keyAEHiliteRange, typeTextRangeArray, hiliteTray);
		if (err <> noErr) then
			if (err <> errAEDescNotFound) then
				goto 1;

		if (hiliteTray.dataHandle <> nil) then
			begin
				err := _WEHiliteRangeArray(TextRangeArrayHandle(hiliteTray.dataHandle), hWE);
				if (err <> noErr) then
					goto 1;
			end
		else
			begin
				pWE^.selStart := tsmOffset + fixLength;
				pWE^.selEnd := pWE^.selStart;
			end;

{ temporarily disable auto-scroll, as we need to scroll manually according to pinRange }
		saveAutoScroll := BTST(pWE^.flags, weFAutoScroll);
		BCLR(pWE^.flags, weFAutoScroll);

{ redraw the active input area }
		err := _WERedraw(tsmOffset, tsmOffset + totalLength, hWE);
		if (err <> noErr) then
			goto 1;

		if (saveAutoScroll) then
			begin

{ re-enable auto-scroll }
				BSET(pWE^.flags, weFAutoScroll);

{ scroll the pin range into view }
				if (_WEScrollIntoView(pinRange.fStart, hWE) = false) then
					if (pinRange.fStart <> pinRange.fEnd) then
						IgnoreBoolean(_WEScrollIntoView(pinRange.fEnd, hWE));
			end;

{ update the boundaries of the active input area }
{ if fixLength = totalLength, the inline input session is over: close the active input area }
		if (fixLength = totalLength) then
			begin
				pWE^.tsmAreaStart := -1;
				pWE^.tsmAreaEnd := -1;
			end
		else
			begin

{ otherwise, fixLength defines the boundaries of the active input area }
				pWE^.tsmAreaStart := tsmOffset + fixLength;
				pWE^.tsmAreaEnd := tsmOffset + totalLength;
			end;

{ call the post-update callback, if present }
		if (pWE^.tsmPostUpdate <> nil) then
			CallPostUpdate(hWE, fixLength, pWE^.tsmAreaStart, pWE^.tsmAreaEnd, pinRange.fStart, pinRange.fEnd, pWE^.tsmPostUpdate);

{ clear result code }
		err := noErr;

1:
{ clean up }
		IgnoreShort(AEDisposeDesc(text));
		IgnoreShort(AEDisposeDesc(hiliteTray));

{ unlock the WE record }
		IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

{ return result code }
		_WEHandleUpdateActiveInputArea := err;

	end;  { _WEHandleUpdateActiveInputArea }

	function _WEHandlePositionToOffset (var ae, reply: AppleEvent;
									handlerRefCon: LongInt): OSErr;
		label
			1;
		var
			hWE: WEHandle;
			pWE: WEPtr;
			position: Point;
			thePoint: LongPoint;
			regionClass: Integer;
			offset: LongInt;
			returnedType: DescType;
			actualSize: LongInt;
			savePort: GrafPtr;
			edge: SignedByte;
			saveWELock: Boolean;
			err: OSErr;
	begin
		hWE := nil;

{ extract WE handle }
		err := AEGetParamPtr(ae, keyAETSMDocumentRefCon, typeLongInteger, returnedType, @hWE, SizeOf(hWE), actualSize);
		if (err <> noErr) then
			goto 1;

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ extract position parameter }
		err := AEGetParamPtr(ae, keyAECurrentPoint, typeQDPoint, returnedType, @position, SizeOf(position), actualSize);
		if (err <> noErr) then
			goto 1;

{ convert position to local... }
		GetPort(savePort);
		SetPort(pWE^.port);
		GlobalToLocal(position);
		SetPort(savePort);

{ ...and long coordinates }
		WEPointToLongPoint(position, thePoint);

{ find the byte offset and the edge value corresponding to the given position }
		offset := WEGetOffset(thePoint, edge, hWE);

{ add offset parameter to reply }
		err := AEPutParamPtr(reply, keyAEOffset, typeLongInteger, @offset, SizeOf(offset));
		if (err <> noErr) then
			goto 1;

{ add edge parameter to reply }
		err := AEPutParamPtr(reply, keyAELeftSide, typeBoolean, @edge, SizeOf(edge));
		if (err <> noErr) then
			goto 1;

{ determine the region class }
		if (WELongPointInLongRect(thePoint, pWE^.viewRect)) then
			if (_WEOffsetInRange(offset, edge, pWE^.tsmAreaStart, pWE^.tsmAreaEnd)) then
				regionClass := kTSMInsideOfActiveInputArea
			else
				regionClass := kTSMInsideOfBody
		else
			regionClass := kTSMOutsideOfBody;

{ add region class parameter to reply }
		err := AEPutParamPtr(reply, keyAERegionClass, typeShortInteger, @regionClass, SizeOf(regionClass));
		if (err <> noErr) then
			goto 1;

{ clear result code }
		err := noErr;

1:
{ unlock the WE record }
		if (hWE <> nil) then
			IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

{ return result code }
		_WEHandlePositionToOffset := err;

	end;  { _WEHandlePositionToOffset }

	function _WEHandleOffsetToPosition (var ae, reply: AppleEvent;
									handlerRefCon: LongInt): OSErr;
		label
			1;
		var
			hWE: WEHandle;
			pWE: WEPtr;
			offset: LongInt;
			thePoint: LongPoint;
			position: Point;
			lineHeight: Integer;
			returnedType: DescType;
			actualSize: LongInt;
			savePort: GrafPtr;
			saveWELock: Boolean;
			err: OSErr;
	begin
		hWE := nil;

{ extract WE handle }
		err := AEGetParamPtr(ae, keyAETSMDocumentRefCon, typeLongInteger, returnedType, @hWE, SizeOf(hWE), actualSize);
		if (err <> noErr) then
			goto 1;

{ lock the WE record }
		saveWELock := _WESetHandleLock(hWE, true);
		pWE := hWE^;

{ if there's no active input area, return errOffsetInvalid }
		if (pWE^.tsmAreaStart < 0) then
			begin
				err := errOffsetInvalid;
				goto 1;
			end;

{ extract the offset parameter }
		err := AEGetParamPtr(ae, keyAEOffset, typeLongInteger, returnedType, @offset, SizeOf(offset), actualSize);
		if (err <> noErr) then
			goto 1;

{ offset is relative to the beginning of the active input area; we want an absolute offset }
		offset := offset + pWE^.tsmAreaStart;

{ make sure the offset is within the input area }
		if (offset < pWE^.tsmAreaStart) or (offset >= pWE^.tsmAreaEnd) then
			begin
				err := errOffsetInvalid;
				goto 1;
			end;

{ find the position corresponding to the given offset (in long coordinates) }
		WEGetPoint(offset, thePoint, lineHeight, hWE);
		thePoint.v := thePoint.v + lineHeight;

{ make sure offset is within view rectangle }
		if (not WELongPointInLongRect(thePoint, pWE^.viewRect)) then
			begin
				err := errOffsetIsOutsideOfView;
				goto 1;
			end;

{ convert the point to short... }
		WELongPointToPoint(thePoint, position);

{ ...and global coordinates }
		GetPort(savePort);
		SetPort(pWE^.port);
		LocalToGlobal(position);
		SetPort(savePort);

{ add keyAEPoint parameter to the reply Apple event }
		err := AEPutParamPtr(reply, keyAEPoint, typeQDPoint, @position, SizeOf(position));
		if (err <> noErr) then
			goto 1;

{ add keyAETextFont parameter to the reply Apple event }
		err := AEPutParamPtr(reply, keyAETextFont, typeShortInteger, @pWE^.nullStyle.tsFont, SizeOf(pWE^.nullStyle.tsFont));
		if (err <> noErr) then
			goto 1;

{ add keyAETextPointSize parameter to the reply Apple event }
		err := AEPutParamPtr(reply, keyAETextPointSize, typeShortInteger, @pWE^.nullStyle.tsSize, SizeOf(pWE^.nullStyle.tsSize));
		if (err <> noErr) then
			goto 1;

{ add keyAETextLineAscent parameter to the reply Apple event }
		err := AEPutParamPtr(reply, keyAETextLineAscent, typeShortInteger, @pWE^.nullStyle.runAscent, SizeOf(pWE^.nullStyle.runAscent));
		if (err <> noErr) then
			goto 1;

{ add keyAETextLineHeight parameter to the reply Apple event }
		err := AEPutParamPtr(reply, keyAETextLineHeight, typeShortInteger, @pWE^.nullStyle.runHeight, SizeOf(pWE^.nullStyle.runHeight));
		if (err <> noErr) then
			goto 1;

{ clear result code }
		err := noErr;

1:
{ unlock the WE record }
		if (hWE <> nil) then
			IgnoreBoolean(_WESetHandleLock(hWE, saveWELock));

{ return result code }
		_WEHandleOffsetToPosition := err;

	end;  { _WEHandleOffsetToPosition }

{$IFC WASTE_SEGMENT}
{$S WASTE_INIT}
{$ENDC}

	function WEInstallTSMHandlers: OSErr;
		label
			1;
		var
			err: OSErr;
	begin

{ install Apple Event handlers to be used by Text Service components }
		err := AEInstallEventHandler(kTextServiceClass, kUpdateActiveInputArea, @_WEHandleUpdateActiveInputArea, 0, false);
		if (err <> noErr) then
			goto 1;

		err := AEInstallEventHandler(kTextServiceClass, kPos2Offset, @_WEHandlePositionToOffset, 0, false);
		if (err <> noErr) then
			goto 1;

		err := AEInstallEventHandler(kTextServiceClass, kOffset2Pos, @_WEHandleOffsetToPosition, 0, false);
		if (err <> noErr) then
			goto 1;

{ clear result code }
		err := noErr;

1:
{ return result code }
		WEInstallTSMHandlers := err;

	end;  { WEInstallTSMHandlers }

end.
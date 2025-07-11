// { WASTE PROJECT: }
// { Inline Input Support }

// { Copyright � 1993-1995 Marco Piovanelli }
// { All Rights Reserved }
// C conversion by Dan Crevier

#include "WASTEIntf.h"
#include "LongCoords.h"

#define myTypeQDPoint 'QDpt'

// { static variables }

static AEEventHandlerUPP _weUpdateActiveInputAreaHandler = NULL;
static AEEventHandlerUPP _wePositionToOffsetHandler = NULL;
static AEEventHandlerUPP _weOffsetToPositionHandler = NULL;

pascal OSErr _WEHiliteRangeArray(TextRangeArrayHandle hTray, WEHandle hWE)
{
	WEPtr pWE;
	TextRangePtr pRange;
	long rangeStart, rangeEnd;
	short hiliteStyle;
	WETextStyle ts;
	short rangeIndex;
	Boolean saveTrayLock;
	OSErr err;

	pWE = *hWE;

	//{ lock down the range array }
	saveTrayLock = _WESetHandleLock((Handle)hTray, true);
	rangeIndex = (**hTray).fNumOfRanges - 1;
	pRange = (*hTray)->fRange;

	//{ walk the hilite range array }
	while (rangeIndex >= 0)
	{

		//{ the offsets in the range array are relative to the beginning }
		//{ of the active input area: convert them to absolute offsets }
		rangeStart = pWE->tsmAreaStart + pRange->fStart;
		rangeEnd = pWE->tsmAreaStart + pRange->fEnd;
		hiliteStyle = pRange->fHiliteStyle;

		//{ take the absolute value of hiliteStyle }
		hiliteStyle = ABS(hiliteStyle);

		//{ if hiliteStyle is kCaretPosition, set the selection range }
		if (hiliteStyle == kCaretPosition)
		{
			pWE->selStart = rangeStart;
			pWE->selEnd = rangeEnd;
		}
		else
		{
			hiliteStyle = hiliteStyle - kRawText;
			//{ otherwise set the WETextStyle flags of the specified range appropriately }
			if ((hiliteStyle >= 0) && (hiliteStyle <= 3))
			{
				ts.tsFlags = 0x10 + BSL(hiliteStyle, tsTSMSelected);
				err = _WESetStyleRange(rangeStart, rangeEnd, weDoFlags, &ts, hWE);
				if (err != noErr)
				{
					goto cleanup;
				}
			}
		}		
		//{ go to next text range element }
		rangeIndex = rangeIndex - 1;
		pRange++;
	}

	// { clear result code }
	err = noErr;

cleanup:
	//{ unlock the range array }
	_WESetHandleLock((Handle)hTray, saveTrayLock);

	return err;
}

pascal OSErr _WEHandleUpdateActiveInputArea(AppleEvent *ae, AppleEvent *reply,
		long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)
	WEHandle hWE;
	WEPtr pWE;
	AEDesc text;
	AEDesc hiliteTray;
	TextRange pinRange;
	long totalLength;
	long fixLength;
	long tsmOffset;
	DescType returnedType;
	long actualSize;
	GrafPtr savePort;
	WEActionHandle hAction;
	Boolean saveAutoScroll;
	Boolean saveTextLock;
	Boolean saveWELock;
	OSErr err;

	hWE = nil;

	//{ initialize descriptors to null values }
	text.descriptorType = typeNull;
	text.dataHandle = nil;
	hiliteTray.descriptorType = typeNull;
	hiliteTray.dataHandle = nil;

	//{ extract WE handle }
	err = AEGetParamPtr(ae, keyAETSMDocumentRefcon, typeLongInteger, &returnedType,
		(Ptr)&hWE, sizeof(hWE), &actualSize);
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ lock the WE handle }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	//{ call the pre-update callback, if present }
	if (pWE->tsmPreUpdate != nil)
	{
		CallWETSMPreUpdateProc(hWE, (WETSMPreUpdateUPP)pWE->tsmPreUpdate);
	}

	//{ hide the caret if it's showing }
	if (BTST(pWE->flags, weFCaretVisible))
	{
		_WEBlinkCaret(hWE);
	}
	
	//{ extract the text descriptor }
	err = AEGetParamDesc(ae, keyAETheData, typeChar, &text);
	if (err != noErr)
	{
		goto cleanup;
	}
	
	//{ get total length of text in the active input area }
	totalLength = GetHandleSize(text.dataHandle);

	//{ extract the length of confirmed text in the active input area }
	err = AEGetParamPtr(ae, keyAEFixLength, typeLongInteger, &returnedType,
		&fixLength, sizeof(fixLength), &actualSize);
	if (err != noErr)
	{
		goto cleanup;
	}
	//{ if fixLength = -1, all text is confirmed }
	if (fixLength == -1)
	{
		fixLength = totalLength;
	}
	//{ if there's currently no active input area, open one }
	if (pWE->tsmAreaStart == kInvalidOffset)
	{
		pWE->tsmAreaStart = pWE->selStart;
		pWE->tsmAreaEnd = pWE->selEnd;
		
		// { are we tracking a typing sequence? }
		if (WEIsTyping(hWE) == false)
		{

			// { nope; so start a new one }
			// { increment modification count }
			pWE->modCount = pWE->modCount + 1;

			//{ if undo support is enabled, the inline session just started may initiate }
			if (BTST(pWE->flags, weFUndoSupport))
			{
				WEClearUndo(hWE);
				if (WENewAction(pWE->selStart, pWE->selEnd, 0, weAKTyping, 0, hWE, &hAction) == noErr)
				{
					if (WEPushAction(hAction) != noErr)
					{
						;
					}
				}
			}
		}
	}

	tsmOffset = pWE->tsmAreaStart;

	//{ the new text replaces whatever is in the active input area }
	err = _WEDeleteRange(tsmOffset, pWE->tsmAreaEnd, hWE);
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ synchronize the null style, so font script matches the keyboard script }
	_WESynchNullStyle(hWE);

	//{ set the port font for good measure }
	GetPort(&savePort);
	SetPort(pWE->port);
	TextFont(pWE->nullStyle.runStyle.tsFont);
	SetPort(savePort);

	//{ temporarily lock the text }
	saveTextLock = _WESetHandleLock(text.dataHandle, true);

	// { insert the text }
	err = _WEInsertText(tsmOffset, *(text.dataHandle), totalLength, hWE);
	if (err != noErr)
	{
		goto cleanup;
	}
	//{ unlock the text }
	_WESetHandleLock(text.dataHandle, saveTextLock);

	//{ extract pin range }
	err = AEGetParamPtr(ae, keyAEPinRange, typeTextRange, &returnedType, &pinRange,
		sizeof(pinRange), &actualSize);
	if (err == noErr)
	{
		//{ we want absolute offsets }
		pinRange.fStart = pinRange.fStart + tsmOffset;
		pinRange.fEnd = pinRange.fEnd + tsmOffset;
	}
	else
	{
		//{ a missing pin range descriptor isn't an error; everything else is }
		if (err != errAEDescNotFound)
		{
			goto cleanup;
		}
		//{ default pin range is active input area }
		pinRange.fStart = tsmOffset;
		pinRange.fEnd = pWE->tsmAreaEnd;
	}

	// { NOTE: if fixLength = totalLength, the inline input session is over, so, in theory, }
	// { hiliteTray should either be missing or not specify any range to be underlined. }
	// { Unfortunately, some input methods (like Apple's input method for Simplified Chinese) }
	// { do specify kConvertedText (= thin black underline) for the whole text when }
	// { the text is confirmed (is this a bug?).  To work around this, we deliberately ignore }
	// { the hiliteTray parameter when fixLength = totalLength. }

	if (fixLength != totalLength)
	{
		//{ extract the highlight range array }
		err = AEGetParamDesc(ae, keyAEHiliteRange, typeTextRangeArray, &hiliteTray);
		if (err != noErr)
		{
			if (err != errAEDescNotFound)
			{
				goto cleanup;
			}
		}
	}

	if (hiliteTray.dataHandle != nil)
	{
		err = _WEHiliteRangeArray((TextRangeArrayHandle)hiliteTray.dataHandle, hWE);
		if (err != noErr)
		{
			goto cleanup;
		}
	}
	else
	{
		pWE->selStart = tsmOffset + fixLength;
		pWE->selEnd = pWE->selStart;
	}

	//{ temporarily disable auto-scroll, as we need to scroll manually according to pinRange }
	saveAutoScroll = BTST(pWE->flags, weFAutoScroll);
	BCLR(pWE->flags, weFAutoScroll);
	
	//{ redraw the active input area }
	err = _WERedraw(tsmOffset, tsmOffset + totalLength, hWE);
	if (err != noErr)
	{
		goto cleanup;
	}

	if (saveAutoScroll)
	{
	
		//{ re-enable auto-scroll }
		BSET(pWE->flags, weFAutoScroll);

		//{ scroll the pin range into view }
		if (_WEScrollIntoView(pinRange.fStart, hWE) == false)
		{
			if (pinRange.fStart != pinRange.fEnd)
			{
				_WEScrollIntoView(pinRange.fEnd, hWE);
			}
		}
	}

	//{ update the boundaries of the active input area }
	//{ if fixLength = totalLength, the inline input session is over: close the active input area }
	if (fixLength == totalLength)
	{
		pWE->tsmAreaStart = kInvalidOffset;
		pWE->tsmAreaEnd = kInvalidOffset;

		//{ adjust undo buffer (if any) for the confirmed text }
		_WEAdjustUndoRange(fixLength, hWE);
	}
	else
	{
		//{ otherwise, fixLength defines the boundaries of the active input area }
		pWE->tsmAreaStart = tsmOffset + fixLength;
		pWE->tsmAreaEnd = tsmOffset + totalLength;
	}

	//{ call the post-update callback, if present }
	if (pWE->tsmPostUpdate != nil)
	{
		CallWETSMPostUpdateProc(hWE, fixLength, pWE->tsmAreaStart, pWE->tsmAreaEnd,
			pinRange.fStart, pinRange.fEnd, (WETSMPostUpdateUPP)pWE->tsmPostUpdate);
	}
	//{ clear result code }
	err = noErr;

cleanup:
	//{ clean up }
	AEDisposeDesc(&text);
	AEDisposeDesc(&hiliteTray);

	//{ unlock the WE record }
	if (hWE !=NULL)
	{
		_WESetHandleLock((Handle)hWE, saveWELock);
	}

	//{ return result code }
	return err;
}


pascal OSErr _WEHandlePositionToOffset (AppleEvent *ae, AppleEvent *reply,
		long handlerRefCon)
{
#pragma unused(handlerRefCon)
	WEHandle hWE;
	WEPtr pWE;
	Point position;
	LongPt thePoint;
	short regionClass;
	long offset;
	DescType returnedType;
	long actualSize;
	GrafPtr savePort;
	char edge;
	Boolean saveWELock;
	OSErr err;

	hWE = nil;

	//{ extract WE handle }
	err = AEGetParamPtr(ae, keyAETSMDocumentRefcon, typeLongInteger, &returnedType,
		&hWE, sizeof(hWE), &actualSize);
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { extract position parameter }
	err = AEGetParamPtr(ae, keyAECurrentPoint, myTypeQDPoint, &returnedType, &position,
		sizeof(position), &actualSize);
	if (err != noErr)
	{
			goto cleanup;
	}
	//{ convert position to local... }
	GetPort(&savePort);
	SetPort(pWE->port);
	GlobalToLocal(&position);
	SetPort(savePort);

	//{ ...and long coordinates }
	WEPointToLongPoint(position, &thePoint);

	//{ find the byte offset and the edge value corresponding to the given position }
	offset = WEGetOffset(&thePoint, &edge, hWE);

	//{ determine the region class }
	if (WELongPointInLongRect(&thePoint, &pWE->viewRect))
	{
		if (_WEOffsetInRange(offset, edge, pWE->tsmAreaStart, pWE->tsmAreaEnd))
		{
  			regionClass = kTSMInsideOfActiveInputArea;
			// { if the given position is within the active input area, we're supposed to return }
			// { an offset relative to the beginning of this area (thanks, Martin!) }
        	offset -= pWE->tsmAreaStart;
		}
		else
		{
			// { otherwise the offset is relative to the beginning of the body }
			regionClass = kTSMInsideOfBody;
		}
	}
	else
	{
		regionClass = kTSMOutsideOfBody;
	}
	
	//{ add region class parameter to reply }
	err = AEPutParamPtr(reply, keyAERegionClass, typeShortInteger, &regionClass,
		sizeof(regionClass));
	if (err != noErr)
	{
		goto cleanup;
	}
	
	// { add offset parameter to reply }
	err = AEPutParamPtr(reply, keyAEOffset, typeLongInteger, &offset, sizeof(offset));
	if (err != noErr) 
	{
		goto cleanup;
	}
	
	//{ add edge parameter to reply }
	err = AEPutParamPtr(reply, keyAELeftSide, typeBoolean, &edge, sizeof(edge));
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ clear result code }
	err = noErr;

cleanup:
	//{ unlock the WE record }
	if (hWE != nil)
	{
		_WESetHandleLock((Handle)hWE, saveWELock);
	}
	
	//{ return result code }
	return err;
}

pascal OSErr _WEHandleOffsetToPosition(AppleEvent *ae, AppleEvent *reply, long handlerRefCon)
{
#pragma unused(handlerRefCon)
	WEHandle hWE;
	WEPtr pWE;
	long offset;
	LongPt thePoint;
	Point position;
	short lineHeight;
	DescType returnedType;
	long actualSize;
	GrafPtr savePort;
	Boolean saveWELock;
	OSErr err;

	hWE = nil;

	//{ extract WE handle }
	err = AEGetParamPtr(ae, keyAETSMDocumentRefcon, typeLongInteger, &returnedType, &hWE,
		sizeof(hWE), &actualSize);
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	//{ if there's no active input area, return errOffsetInvalid }
	if (pWE->tsmAreaStart < 0)
	{
		err = errOffsetInvalid;
		goto cleanup;
	}
	
	//{ extract the offset parameter }
	err = AEGetParamPtr(ae, keyAEOffset, typeLongInteger, &returnedType, &offset, 
		sizeof(offset), &actualSize);
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ offset is relative to the beginning of the active input area; we want an absolute offset }
	offset = offset + pWE->tsmAreaStart;

	// { make sure the offset is within the input area }
	if ((offset < pWE->tsmAreaStart) || (offset >= pWE->tsmAreaEnd))
	{
		err = errOffsetInvalid;
		goto cleanup;
	}

	//{ find the position corresponding to the given offset (in long coordinates) }
	WEGetPoint(offset, &thePoint, &lineHeight, hWE);
	thePoint.v = thePoint.v + lineHeight;

	//{ make sure offset is within view rectangle }
	if (!WELongPointInLongRect(&thePoint, &pWE->viewRect))
	{
		err = errOffsetIsOutsideOfView;
		goto cleanup;
	}

	//{ convert the point to short... }
	WELongPointToPoint(&thePoint, &position);

	//{ ...and global coordinates }
	GetPort(&savePort);
	SetPort(pWE->port);
	LocalToGlobal(&position);
	SetPort(savePort);

	//{ add keyAEPoint parameter to the reply Apple event }
	err = AEPutParamPtr(reply, keyAEPoint, myTypeQDPoint, &position, sizeof(position));
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ add keyAETSMTextFont parameter to the reply Apple event }
	err = AEPutParamPtr(reply, keyAETSMTextFont, typeShortInteger, &pWE->nullStyle.runStyle.tsFont,
		sizeof(pWE->nullStyle.runStyle.tsFont));
	if (err != noErr)
	{
		goto cleanup;
	}
	
	//{ add keyAETSMTextPointSize parameter to the reply Apple event }
	err = AEPutParamPtr(reply, keyAETSMTextPointSize, typeShortInteger, 
		&pWE->nullStyle.runStyle.tsSize, sizeof(pWE->nullStyle.runStyle.tsSize));
	if (err != noErr)
	{
		goto cleanup;
	}

	//{ add keyAETextLineAscent parameter to the reply Apple event }
	err = AEPutParamPtr(reply, keyAETextLineAscent, typeShortInteger, 
		&pWE->nullStyle.runAscent, sizeof(pWE->nullStyle.runAscent));
	if (err != noErr)
	{
		goto cleanup;
	}
	
	//{ add keyAETextLineHeight parameter to the reply Apple event }
	err = AEPutParamPtr(reply, keyAETextLineHeight, typeShortInteger, 
		&pWE->nullStyle.runHeight, sizeof(pWE->nullStyle.runHeight));
	if (err != noErr)
	{
		goto cleanup;
	}
	
	//{ clear result code }
	err = noErr;

cleanup:
	//{ unlock the WE record }
	if (hWE != nil)
	{
		_WESetHandleLock((Handle) hWE, saveWELock);
	}
	
	//{ return result code }
	return err;
}

pascal OSErr WEInstallTSMHandlers(void)
{
	OSErr err;
	
	// { the first time we're called, create routine descriptors for our Apple event handlers }
	if (_weUpdateActiveInputAreaHandler == nil)
	{
		_weUpdateActiveInputAreaHandler = NewAEEventHandlerProc(_WEHandleUpdateActiveInputArea);
		_wePositionToOffsetHandler = NewAEEventHandlerProc(_WEHandlePositionToOffset);
		_weOffsetToPositionHandler = NewAEEventHandlerProc(_WEHandleOffsetToPosition);
	}

	//{ install Apple Event handlers to be used by Text Service components }
	err = AEInstallEventHandler(kTextServiceClass, kUpdateActiveInputArea,
		_weUpdateActiveInputAreaHandler, 0, false);
	if (err != noErr)
	{
		return err;
	}
		
	err = AEInstallEventHandler(kTextServiceClass, kPos2Offset, 
		_wePositionToOffsetHandler, 0, false);
	if (err != noErr)
	{
		return err;
	}
	
	err = AEInstallEventHandler(kTextServiceClass, kOffset2Pos, 
		_weOffsetToPositionHandler, 0, false);
	if (err != noErr)
	{
		return err;
	}
	
	// { return result code }
	return noErr;
}

pascal OSErr WERemoveTSMHandlers(void)
{
	OSErr err;
	
	// { return an error code if WEInstallTSMHandlers has never been called }
	if (_weUpdateActiveInputAreaHandler == nil)
		return errAEHandlerNotFound;

	// { remove the handlers }
	err = AERemoveEventHandler(kTextServiceClass, kUpdateActiveInputArea,
		_weUpdateActiveInputAreaHandler, false);
	if (err != noErr)
	{
		return err;
	}
		
	err = AERemoveEventHandler(kTextServiceClass, kPos2Offset, 
		_wePositionToOffsetHandler, false);
	if (err != noErr)
	{
		return err;
	}
	
	err = AERemoveEventHandler(kTextServiceClass, kOffset2Pos, 
		_weOffsetToPositionHandler, false);
	if (err != noErr)
	{
		return err;
	}
	
	// { clear result code }
	return noErr;
} // { WERemoveTSMHandlers }

// { WASTE PROJECT: }
// { High-Level Editing Routines }

// { Copyright � 1993-1994 Marco Piovanelli }
// { All Rights Reserved }

#include "WASTEIntf.h"

typedef struct DoubleByte {
	char firstByte;
	char secondByte;
} DoubleByte;

pascal WEActionHandle WEGetActionStack(WEHandle hWE)
{
	return (WEActionHandle)(*hWE)->hActionStack;
} // { WEGetActionStack }

pascal OSErr WEPushAction(WEActionHandle hAction)
{
	WEPtr pWE;
	WEActionHandle hLast;

	// { find the last action in the given stack }
	hLast = hAction;
	while ((*hLast)->hNext != nil)
		hLast = (*hLast)->hNext;

	// { prepend hAction in front of the action stack }
	pWE = *((*hAction)->hOwner);
	(*hLast)->hNext = (WEActionHandle)pWE->hActionStack;
	pWE->hActionStack = (Handle)hAction;

	return noErr;
} // { WEPushAction }

pascal OSErr WENewAction(long rangeStart, long rangeEnd, long newTextLength,
							WEActionKind actionKind, WEActionFlags actionFlags,
							WEHandle hWE, WEActionHandle *hAction)
{
	WEActionPtr pAction;
	OSErr err;

	// { allocate a new action record }
	err = _WEAllocate(sizeof(WEAction), kAllocClear, (Handle *)hAction);
	if (err != noErr) 
		goto cleanup1;

	// { lock it down }
	HLock((Handle)*hAction);
	pAction = **hAction;

	// { fill in the fields }
	pAction->hOwner = hWE;
	pAction->delRangeStart = rangeStart;
	pAction->delRangeLength = newTextLength;
	pAction->insRangeLength = rangeEnd - rangeStart;
	pAction->actionKind = actionKind;
	pAction->actionFlags = actionFlags;

	// { remember selection range }
	WEGetSelection(&pAction->hiliteStart, &pAction->hiliteEnd, hWE);

	// { allocate a handle to hold the text to be saved, unless otherwise specified }
	if ((actionFlags & weAFDontSaveText) == 0) 
	{
		err = _WEAllocate(0, kAllocTemp, &pAction->hText);
		if (err != noErr) 
			goto cleanup1;
	}

	// { allocate a handle to hold the styles to be saved, unless otherwise specified }
	if ((actionFlags & weAFDontSaveStyles) == 0) 
	{
		err = _WEAllocate(0, kAllocTemp, &pAction->hStyles);
		if (err != noErr) 
			goto cleanup1;
	}

	// { allocate a handle to hold the "soup" to be saved, unless otherwise specified }
	if ((actionFlags & weAFDontSaveSoup) == 0) 
	{
		err = _WEAllocate(0, kAllocTemp, &pAction->hSoup);
		if (err != noErr) 
			goto cleanup1;
	}
	
	// { make a copy of text range }
	err = WECopyRange(rangeStart, rangeEnd, pAction->hText, (Handle)pAction->hStyles,
					pAction->hSoup, hWE);
	if (err != noErr)
		goto cleanup1;

	// { unlock action record }
	HUnlock((Handle)*hAction);

	// { skip clean-up section }
	goto cleanup0;

cleanup1:
	// { clean up }
	_WEForgetHandle(&pAction->hText);
	_WEForgetHandle(&pAction->hStyles);
	_WEForgetHandle(&pAction->hSoup);
	_WEForgetHandle((Handle *)hAction);

cleanup0:
	// { return result code }
	return err;

} // { WENewAction }

pascal void WEDisposeAction(WEActionHandle hAction)
{
	WEActionPtr pAction;
	WEActionHandle hNext;

	while (hAction != nil)
	{
		// { lock the action record }
		HLock((Handle)hAction);
		pAction = *hAction;
		hNext = pAction->hNext;

		// { throw away text, styles and soup }
		_WEForgetHandle(&pAction->hText);
		_WEForgetHandle(&pAction->hStyles);
		_WEForgetHandle(&pAction->hSoup);

		// { throw away the action record itself }
		DisposeHandle((Handle)hAction);

		// { repeat the same sequence with all linked actions }
		hAction = hNext;

	} // { while }
} // { WEDisposeAction }

pascal void WEForgetAction(WEActionHandle *hAction)
{
	WEActionHandle theAction;

	theAction = *hAction;
	if (theAction != nil) 
	{
		*hAction = nil;
		WEDisposeAction(theAction);
	}
} // { WEForgetAction }

pascal OSErr WEDoAction(WEActionHandle hAction)
{
	WEActionHandle hRedoAction;
	WEActionPtr pAction;
	WEHandle hWE;
	WEPtr pWE;
	long offset, delOffset, insOffset;
	long redrawStart, redrawEnd;
	Boolean saveActionLock, saveWELock, saveTextLock;
	OSErr err;

	// { sanity check: make sure hAction isn't NIL }
	if (hAction == nil) 
	{
		return nilHandleErr;
	}
	
	// { get handle to associated WE instance }
	hWE = (*hAction)->hOwner;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	// { stop any ongoing inline input session }
	WEStopInlineSession(hWE);

	// { hide selection highlighting and the caret }
	_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
	if (BTST(pWE->flags, weFCaretVisible))
		_WEBlinkCaret(hWE);

	redrawStart = LONG_MAX;
	redrawEnd = 0;

	do
	{

		// { lock the action record }
		saveActionLock = _WESetHandleLock((Handle)hAction, true);
		pAction = *hAction;
		offset = pAction->delRangeStart;
		delOffset = offset + pAction->delRangeLength;
		insOffset = offset + pAction->insRangeLength;

		// { if undo support is enabled, save the range to be affected by this action }
		if (BTST(pWE->flags, weFUndoSupport)) 
		{
			if (WENewAction(offset, delOffset, pAction->insRangeLength, pAction->actionKind,
				(pAction->actionFlags ^ weAFIsRedo), hWE, &hRedoAction) == noErr) 
			{
				if (WEPushAction(hRedoAction) != noErr)
				{
					;
				}
			}
		}
		if (pAction->hText != nil) 
		{

			// { delete the range to replace }
			err = _WEDeleteRange(offset, delOffset, hWE);
			if (err != noErr) 
				goto cleanup;

			// { insert the saved text }
			saveTextLock = _WESetHandleLock(pAction->hText, true);
			err = _WEInsertText(offset, *pAction->hText, pAction->insRangeLength, hWE);
			_WESetHandleLock(pAction->hText, saveTextLock);
			if (err != noErr) 
				goto cleanup;
		}

		// { apply the saved styles, if any }
		if (pAction->hStyles != nil) 
		{
			err = _WEApplyStyleScrap(offset, insOffset, (StScrpHandle)pAction->hStyles, hWE);
			if (err != noErr) 
				goto cleanup;
		}

		// { the same goes for the soup }
		if (pAction->hSoup != nil) 
		{
			err = _WEApplySoup(offset, pAction->hSoup, hWE);
			if (err != noErr) 
				goto cleanup;
		}

		// { adjust redraw range }
		if (offset < redrawStart) 
			redrawStart = offset;
		if (insOffset > redrawEnd) 
			redrawEnd = insOffset;

		// { unlock action record }
		_WESetHandleLock((Handle)hAction, saveActionLock);

		// { go to next action }
		hAction = (*hAction)->hNext;
	} while (hAction != nil);

	// { restore the original selection range }
	pWE->selStart = pAction->hiliteStart;
	pWE->selEnd = pAction->hiliteEnd;

	// { redraw the text }
	err = _WERedraw(redrawStart, redrawEnd, hWE);
	if (err != noErr) 
		goto cleanup;

	// { clear result code }
	err = noErr;

cleanup:
	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;
} // { WEDoAction }

pascal OSErr WEUndo(WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	Boolean saveWELock;
	OSErr retval;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;
	
	// { "detach" the action stack from the WE instance }
	hAction = (WEActionHandle)(pWE->hActionStack);
	pWE->hActionStack = nil;

	if (hAction != nil)
	{

		// { undoing a change _decrements_ the modification count; }
		// { redoing the change increments it again }
		if (((*hAction)->actionFlags & weAFIsRedo) != 0)
			pWE->modCount = pWE->modCount + 1;
		else
			pWE->modCount = pWE->modCount - 1;

		// { perform the action... }
		retval = WEDoAction(hAction);

		// { ...and throw it away }
		WEDisposeAction(hAction);
	}
	else
	{
		// { return an error code if the undo buffer is empty }
		retval = weCantUndoErr;
	}

	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	return retval;
}

pascal void WEClearUndo(WEHandle hWE)
{

	// { dispose of the action chain associated with the given WE instance }
	WEForgetAction((WEActionHandle *)&(*hWE)->hActionStack);
} // { WEClearUndo }

pascal WEActionKind WEGetUndoInfo(Boolean *redoFlag, WEHandle hWE)
{
	WEActionKind retval;
	
	retval = weAKNone;		// { assume no actions have been saved }
	*redoFlag = false;

	if ((*hWE)->hActionStack != nil) 
	{
		retval = (*((WEActionHandle)(*hWE)->hActionStack))->actionKind;
		*redoFlag = (((*((WEActionHandle)(*hWE)->hActionStack))->actionFlags & weAFIsRedo) != 0);
	}
	return retval;
} // { WEGetUndoInfo }

pascal long WEGetModCount(WEHandle hWE)
{
	return (*hWE)->modCount;
} // { WEGetModCount }

pascal void WEResetModCount(WEHandle hWE)
{
	(*hWE)->modCount = 0;
	WEClearUndo(hWE);
} // { WEResetModCount }

pascal void _WEAdjustUndoRange(long moreBytes, WEHandle hWE)
{
	WEActionHandle hAction;

	hAction = (WEActionHandle)(*hWE)->hActionStack;
	if (hAction != nil) 
		(*hAction)->delRangeLength = (*hAction)->delRangeLength + moreBytes;

} // { _WEAdjustUndoRange }

pascal OSErr _WETypeChar(char theByte, WEHandle hWE)
{
	WEPtr pWE;
	DoubleByte db;
	long offset, endOffset, charLength;
	OSErr err;

	pWE = *hWE;					// { the WE record must be already locked }
	charLength = 1;				// { assume 1-byte character by default }
	db.firstByte = theByte;
	offset = pWE->selStart;

	// { delete current selection, if any }
	err = _WEDeleteRange(offset, pWE->selEnd, hWE);
	if (err != noErr) 
		goto cleanup2;
	
	pWE->selEnd = offset; //  { needed in case we take a premature exit }

	// { make sure the font script is synchronized with the keyboard script }
	_WESynchNullStyle(hWE);

	if (BTST(pWE->flags, weFDoubleByte))
	{

		// { special processing for double-byte characters }
		if (pWE->firstByte != 0) 
		{

			// { if this byte is the second half of a double-byte character, }
			// { insert the two bytes at the same time (flush the double-byte cache) }
			db.firstByte = pWE->firstByte;
			db.secondByte = theByte;
			charLength = 2;
			pWE->firstByte = 0;
		}
		else
		{

			// { if theByte is the first half of a double-byte character, just cache it and exit }
			if (CallWECharByteProc(&theByte, 0, FontToScript(pWE->nullStyle.runStyle.tsFont),
				hWE, (WECharByteUPP)pWE->charByteHook) == smFirstByte)
			{
				pWE->firstByte = theByte;
				return noErr;
			}
		}

	} // { if double-byte script installed }

	// { insert the new character into the text }
	err = _WEInsertText(offset, (Ptr)&db, charLength, hWE);
	if (err != noErr) 
		goto cleanup2;

	// { adjust undo buffer for the new character }
	_WEAdjustUndoRange(charLength, hWE);

	// { invalid the null style }
	BCLR(pWE->flags, weFUseNullStyle);

	// { move the insertion point after the new character }
	endOffset = offset + charLength;
	pWE->selStart = endOffset;
	pWE->selEnd = endOffset;

	// { redraw the text }
	err = _WERedraw(offset, endOffset, hWE);
	if (err != noErr) 
		goto cleanup2;

cleanup1:
	// { clear result code }
	err = noErr;

cleanup2:
	// { return result code }
	return err;

} // { _WETypeChar }

pascal OSErr _WEBackspace(WEHandle hWE)
{
	// { this routine is called by WEKey to handle the backspace key }
	// { the WE record is guaranteed to be already locked }

	WEPtr pWE;
	WEActionPtr pAction;
	long rangeStart, rangeEnd, charLength;
	char *pChars;
	WERunInfo runInfo;
	Boolean saveActionLock;
	OSErr err;

	pWE = *hWE;

	// { calculate the text range to delete }
	// { if the selection is non-empty, delete that }
	rangeStart = pWE->selStart;
	rangeEnd = pWE->selEnd;
	if (rangeStart == rangeEnd) 
	{

		// { otherwise the selection is an insertion point }
		// { do nothing if insertion point is at the beginning of the text }
		if (rangeStart == 0) return noErr; 

		// { determine the byte-type of the character preceding the insertion point }
		if (WECharByte(rangeStart - 1, hWE) == smSingleByte) 
			charLength = 1;
		else
			charLength = 2;
		rangeStart = rangeStart - charLength;

		if (pWE->hActionStack != nil) 
		{
			// { UNDO SUPPORT FOR BACKSPACES }

			// { lock the action record }
			saveActionLock = _WESetHandleLock(pWE->hActionStack, true);
			pAction = *(WEActionHandle)pWE->hActionStack;

			// { backspaces over the newly entered text aren't a problem }
			if (pAction->delRangeLength > 0) 
				pAction->delRangeLength = pAction->delRangeLength - charLength;
			else
			{

				// { the hard part comes when backspacing past the new text because }
				// { the user is about to delete a character not included in the block we saved }

				// { leng our saved text handle }
				SetHandleSize(pAction->hText, pAction->insRangeLength + charLength);
				err = MemError();
				if (err != noErr) return err;

				// { move old contents forward }
				pChars = *(char **)pAction->hText;
				BlockMoveData(pChars, &pChars[charLength], pAction->insRangeLength);

				// { prepend the character to be deleted to the beginning of our saved text handle }
				pChars[0] = WEGetChar(rangeStart, hWE);
				if (charLength == 2) 
					pChars[1] = WEGetChar(rangeStart + 1, hWE);

				// { adjust internal counters }
				pAction->insRangeLength = pAction->insRangeLength + charLength;
				pAction->delRangeStart = pAction->delRangeStart - charLength;

				// { get style run info associated with the about-to-be-deleted character }
				WEGetRunInfo(rangeStart, &runInfo, hWE);

				// { prepend a new style element to our style scrap, if necessary }
				err = _WEPrependStyle(pAction->hStyles, &runInfo, charLength);
				if (err != noErr) return err; 

				// { do the same with our object "soup" }
				err = _WEPrependObject(pAction->hSoup, &runInfo, charLength);
				if (err != noErr) return err;
			
			} // { if deleting old text }

			// { unlock the action record }
			_WESetHandleLock(pWE->hActionStack, saveActionLock);

		} // { if undo support is enabled }
	} // { if selection is empty }

	err = _WEDeleteRange(rangeStart, rangeEnd, hWE);
	if (err != noErr) return err;

	// { keep track of current selection range }
	pWE->selStart = rangeStart;
	pWE->selEnd = rangeStart;

	// { redraw the text }
	err = _WERedraw(rangeStart, rangeStart, hWE);
	
	return err;
} // { _WEBackspace }

pascal OSErr _WEForwardDelete(WEHandle hWE)
{
	
	// { this routine is called by WEKey to handle the forward delete key }
	// { the WE record is guaranteed to be already locked }

	WEPtr pWE;
	WEActionPtr pAction;
	long rangeStart, rangeEnd, charLength;
	WERunInfo runInfo;
	DoubleByte db;
	Boolean saveActionLock;
	OSErr err;

	pWE = *hWE;

	// { calculate the text range to delete }
	// { if the selection is non-empty, delete that }
	rangeStart = pWE->selStart;
	rangeEnd = pWE->selEnd;
	if (rangeStart == rangeEnd)
	{
		
		// { otherwise the selection is an insertion point }
		// { do nothing if insertion point is at the end of the text }
		if (rangeStart == pWE->textLength)
			return noErr;

		// { determine the byte-type of the character following the insertion point }
		if (WECharByte(rangeStart, hWE) == smSingleByte)
			charLength = 1;
		else
			charLength = 2;
		rangeEnd = rangeStart + charLength;

		if (pWE->hActionStack != nil)
		{

			// { UNDO SUPPORT FOR FORWARD DELETE }

			// { lock the action record }
			saveActionLock = _WESetHandleLock(pWE->hActionStack, true);
			pAction = *(WEActionHandle)(pWE->hActionStack);

			// { make a copy of the character about to be deleted }
			db.firstByte = WEGetChar(rangeStart, hWE);
			if (charLength == 2)
				db.secondByte = WEGetChar(rangeStart + 1, hWE);

			// { append it to the end of our saved text handle }
			PtrAndHand(&db, pAction->hText, charLength);
			err = MemError();
			if (err != noErr)
				return err;

			// { get style run info associated with the about-to-be-deleted character }
			WEGetRunInfo(rangeStart, &runInfo, hWE);

			// { append a new style element to our style scrap, if necessary }
			err = _WEAppendStyle(pAction->hStyles, &runInfo, pAction->insRangeLength);
			if (err != noErr)
				return err;

			// { do the same with our object soup }
			err = _WEAppendObject(pAction->hSoup, &runInfo, pAction->insRangeLength);
			if (err != noErr)
				return err;

			// { adjust internal counters }
			pAction->insRangeLength = pAction->insRangeLength + charLength;

			// { unlock the action record }
			_WESetHandleLock(pWE->hActionStack, saveActionLock);

		} // { if undo support is enabled }
	} // { if selection is empty }

	err = _WEDeleteRange(rangeStart, rangeEnd, hWE);
	if (err != noErr)
		return err;

	// { keep track of current selection range }
	pWE->selStart = rangeStart;
	pWE->selEnd = rangeStart;

	// { redraw the text }
	err = _WERedraw(rangeStart, rangeStart, hWE);

	return err;
} // { _WEForwardDelete }

pascal Boolean WEIsTyping(WEHandle hWE)
{
	WEPtr pWE;

	// { return TRUE if we're tracking a typing sequence in the specified WE instance }

	pWE = *hWE;					// { the WE record must already be locked }

	// { there must be an undo buffer }
	if (pWE->hActionStack == nil) return false;

	// { the action kind must be "typing" and the redo flag must be clear }
	if ((*(WEActionHandle)pWE->hActionStack)->actionKind != weAKTyping)
		return false;
	if (((*(WEActionHandle)pWE->hActionStack)->actionFlags & weAFIsRedo) != 0) 
		return false;

	// { finally, the selection range mustn't have moved since the last WEKey }
	if ((pWE->selStart == pWE->selEnd) && (pWE->selStart == 
		(*(WEActionHandle)pWE->hActionStack)->delRangeStart + 
		(*(WEActionHandle)pWE->hActionStack)->delRangeLength)) 
		return true;
	
	return false;
} // { _WEIsTyping }

pascal void WEKey(short key, short modifiers, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	Boolean saveWELock;
	OSErr err;
	
	err = noErr;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { hide the caret if it's showing }
	if (BTST(pWE->flags, weFCaretVisible))
		_WEBlinkCaret(hWE);

	// { hide the cursor (it will show again as soon as it's moved) }
	ObscureCursor();

	// { dispatch on key class (arrow keys, printable characters, backspace) }
	if ((key >= kArrowLeft) && (key <= kArrowDown)) 
			_WEDoArrowKey(key, modifiers, hWE);
	else
	{

		// { non-arrow keys modify the text, so make sure editing is allowed }
		if (!BTST(pWE->flags, weFReadOnly))
		{
			// { are we tracking a typing sequence? }
			if (WEIsTyping(hWE) == false)
			{
				// { nope;  start a new one }
				// { increment modification count }
				pWE->modCount++;

				// if undo support is enabled, create a new action to keep track of typing
				if (BTST(pWE->flags, weFUndoSupport))
				{
					WEClearUndo(hWE);
					if (WENewAction(pWE->selStart, pWE->selEnd, 0, weAKTyping, 0, hWE, &hAction) == noErr) 
						if (WEPushAction(hAction) != noErr)
						{
							;
						}
				}
			} // { if WEIsTyping }

			if (key == kBackspace) 
				err = _WEBackspace(hWE);
			else if (key == kForwardDelete)
				err = _WEForwardDelete(hWE);
			else
				err = _WETypeChar(key, hWE);
		} //  { if not read-only }
		
	}

	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

} // { WEKey }

pascal OSErr WEInsert(Ptr textPtr, long textLength, StScrpHandle hStyles, Handle hSoup, WEHandle hWE)
{
	WEPtr pWE;
	long offset, endOffset;
	WEActionHandle hAction;
	short intPasteAction;
	Boolean saveWELock;
	char space;
	OSErr err;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;
	offset = pWE->selStart;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	// { stop any ongoing inline input session }
	WEStopInlineSession(hWE);

	// { increment modification count }
	pWE->modCount = pWE->modCount + 1;

	// { if undo support is enabled, save current selection range }
	if (BTST(pWE->flags, weFUndoSupport)) 
	{
		WEClearUndo(hWE);
		if (WENewAction(offset, pWE->selEnd, textLength, weAKUnspecified, 0, hWE, &hAction) == noErr) 
			if (WEPushAction(hAction) != noErr) 
				{ ;	}
	}

	// { delete current selection }
	err = _WEDeleteRange(offset, pWE->selEnd, hWE);
	if (err != noErr) 
		goto cleanup;

	// { insert the new text at the insertion point }
	err = _WEInsertText(offset, textPtr, textLength, hWE);
	if (err != noErr) 
		goto cleanup;
	endOffset = offset + textLength;

	if (hStyles != nil) 
	{

		// { if a style scrap was supplied, apply it to the newly inserted text }
		err = _WEApplyStyleScrap(offset, endOffset, hStyles, hWE);
		if (err != noErr) 
			goto cleanup;
	}

	if (hSoup != nil)
	{

		// { if an object soup was supplied, apply it to the newly inserted text }
		err = _WEApplySoup(offset, hSoup, hWE);
		if (err != noErr) 
			goto cleanup;
	}

	// { determine whether an extra space should be added before or after the inserted text }
	intPasteAction = _WEIntelligentPaste(offset, endOffset, hWE);

	// { add the extra space, if necessary }
	if (intPasteAction != weDontAddSpaces) 
	{

		space = kSpace;
		if (intPasteAction == weAddSpaceOnLeftSide) 
			err = _WEInsertText(offset, &space, 1, hWE);
		else
			err = _WEInsertText(endOffset, &space, 1, hWE);
		if (err != noErr) 
			goto cleanup;
		endOffset = endOffset + 1;

		// { adjust undo buffer (if any) for the extra space }
		_WEAdjustUndoRange(1, hWE);

	}

	// { invalid the null style }
	BCLR(pWE->flags, weFUseNullStyle);

	// { move the insertion point at the end of the inserted text }
	pWE->selStart = endOffset;
	pWE->selEnd = endOffset;

	// { redraw the text }
	err = _WERedraw(offset, endOffset, hWE);
	if (err != noErr) 
		goto cleanup;

	// { clear result code }
	err = noErr;

cleanup:
	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;
} // { WEInsert }

pascal OSErr WEInsertObject(OSType objectType, Handle objectDataHandle, Point objectSize, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	long offset, endOffset;
	WETextStyle ts;
	char marker;
	Boolean saveWELock;
	OSErr err;

	_WEBlockClr((Ptr)&ts, sizeof(ts));

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;
	offset = pWE->selStart;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	// { stop any ongoing inline input session }
	WEStopInlineSession(hWE);

	// { call the 'new' handler to initialize private object storage (if any) }
	// { and to calculate the default size for this object }

	err = _WENewObject(objectType, objectDataHandle, hWE, (WEObjectDescHandle *)&ts.tsObject);
	if (err != noErr) 
		goto cleanup;

	// { use the specified object size, unless it is (0, 0), in which case keep the default size }
	if (*((long *)&objectSize) != 0) 
		(*(WEObjectDescHandle)ts.tsObject)->objectSize = objectSize;

	// { increment modification count }
	pWE->modCount = pWE->modCount + 1;

	// { if undo support is enabled, save current selection range }
	if (BTST(pWE->flags, weFUndoSupport)) 
	{
		WEClearUndo(hWE);
		if (WENewAction(offset, pWE->selEnd, 1, weAKUnspecified, 0, hWE, &hAction) == noErr) 
			if (WEPushAction(hAction) != noErr) 
				{ ; }
	}

	// { delete current selection }
	err = _WEDeleteRange(offset, pWE->selEnd, hWE);
	if (err != noErr) 
		goto cleanup;

	// { insert a kObjectMarker character at the insertion point }
	marker = kObjectMarker;
	err = _WEInsertText(offset, &marker, 1, hWE);
	if (err != noErr) 
		goto cleanup;

	// { move the insertion point after the inserted text }
	endOffset = offset + 1;
	pWE->selStart = endOffset;
	pWE->selEnd = endOffset;

	// { record a reference to the object descriptor in the style table }
	err = _WESetStyleRange(offset, endOffset, weDoObject, &ts, hWE);
	ts.tsObject = kNullObject;
	if (err != noErr) 
		goto cleanup;

	// { invalid the null style }
	BCLR(pWE->flags, weFUseNullStyle);

	// { redraw the text }
	err = _WERedraw(offset, endOffset, hWE);
	if (err != noErr) 
		goto cleanup;

	// { clear result code }
	err = noErr;

cleanup:
	// { clean up }
	_WEForgetHandle((Handle *)&ts.tsObject);

	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;
} // { WEInsertObject }

pascal OSErr WEDelete(WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	long rangeStart, rangeEnd;
	Boolean saveWELock;
	OSErr err;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	// { stop any ongoing inline input session }
	WEStopInlineSession(hWE);

	// { get current selection range }
	rangeStart = pWE->selStart;
	rangeEnd = pWE->selEnd;

	// { do nothing if the selection range is empty }
	if (rangeStart < rangeEnd) 
	{

		// { increment modification count }
		pWE->modCount = pWE->modCount + 1;

		// { range extension for intelligent cut-and-paste }
		_WEIntelligentCut(&rangeStart, &rangeEnd, hWE);

		// { if undo support is enabled, save the range to be deleted }
		if (BTST(pWE->flags, weFUndoSupport)) 
		{
			WEClearUndo(hWE);
			if (WENewAction(rangeStart, rangeEnd, 0, weAKClear, 0, hWE, &hAction) == noErr) 
				if (WEPushAction(hAction) != noErr) 
					{ ; }
		}

		// { delete the selection range }
		err = _WEDeleteRange(rangeStart, rangeEnd, hWE);
		if (err != noErr) 
			goto cleanup;

		// { reset the selection range }
		pWE->selStart = rangeStart;
		pWE->selEnd = rangeStart;

		// { redraw the text }
		err = _WERedraw(rangeStart, rangeStart, hWE);
		if (err != noErr) 
			goto cleanup;

	} // { if non-empty selection }

	// { clear result code }
	err = noErr;

cleanup:
	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;
} // { WEDelete }

pascal OSErr WECut(WEHandle hWE)
{
	OSErr err;

	// { first copy... }
	err = WECopy(hWE);
	if (err != noErr) return err;

	// { ... then delete }
	err = WEDelete(hWE);
	if (err != noErr) return err;

	// { change the action kind of the most recent action, if any }
	if ((*hWE)->hActionStack != nil) 
		(*(WEActionHandle)(*hWE)->hActionStack)->actionKind = weAKCut;

	return noErr;
}  // { WECut }

pascal Boolean WECanPaste(WEHandle hWE)
{
	long scrapOffset;
	OSType objectType;
	short index;

	if (!BTST((*hWE)->flags, weFReadOnly))
	{
		// { return TRUE if the desk scrap contains a text flavor }
		if (GetScrap(nil, kTypeText, &scrapOffset) > 0) return true;

		// { see if the desk scrap contains a flavor matching one of the registered object types }
		index = 0;
		while (_WEGetIndObjectType(index, &objectType, hWE) == noErr)
		{
			if (GetScrap(nil, objectType, &scrapOffset) > 0) return true;
			index = index + 1;
		} // { while }
	}
	return false;
} // { WECanPaste }

pascal OSErr WEPaste(WEHandle hWE)
{
	Handle hItem;
	Handle hStyles;
	Handle hSoup;
	long selStart;
	long scrapOffset;
	OSType objectType;
	short index;
	OSErr err;
	Point zeroPoint = {0, 0};

	hItem = nil;
	hStyles = nil;
	hSoup = nil;
	selStart = (*hWE)->selStart;

	// { allocate a handle to hold a scrap item }
	err = _WEAllocate(0, kAllocTemp, &hItem);
	if (err != noErr) 
		goto cleanup;

	// { look for a text flavor }
	if (GetScrap(hItem, kTypeText, &scrapOffset) <= 0) 
	{

		// { no text: look for a flavor matching one of the registered object types }
		index = 0;
		while (_WEGetIndObjectType(index, &objectType, hWE) == noErr)
		{
			if (GetScrap(hItem, objectType, &scrapOffset) > 0) 
			{
			
				// { found a registered type: create a new object out of the tagged data }
				err = WEInsertObject(objectType, hItem, zeroPoint, hWE);

				// { if successful, set hItem to NIL so clean-up section won't kill the object data }
				if (err == noErr) 
					hItem = nil;
				goto cleanup;
			}

			// { try with next flavor }
			index = index + 1;
		} // { while }

		// { nothing pasteable: return an error code }
		err = noTypeErr;
		goto cleanup;
	}

	// { allocate a handle to hold the style scrap, if any }
	err = _WEAllocate(0, kAllocTemp, &hStyles);
	if (err != noErr) 
		goto cleanup;

	// { look for a 'styl' item accompanying the text }
	if (GetScrap(hStyles, kTypeStyles, &scrapOffset) <= 0) 
		// { forget the handle if nothing was found or an error occurred }
		_WEForgetHandle(&hStyles);

	// { allocate a handle to hold the soup, if any }
	err = _WEAllocate(0, kAllocTemp, &hSoup);
	if (err != noErr) 
		goto cleanup;

	// { look for a 'SOUP' item accompanying the text }
	if (GetScrap(hSoup, kTypeSoup, &scrapOffset) <= 0) 
		// { forget the handle if nothing was found or an error occurred }
		_WEForgetHandle(&hSoup);

	// { lock down the text }
	HLock(hItem);

	// { insert the text }
	err = WEInsert(*hItem, GetHandleSize(hItem), (StScrpHandle)hStyles, hSoup, hWE);

cleanup:
	// { if successful, change the action kind of the most recent action, if any }
	if (err == noErr) 
		if ((*hWE)->hActionStack != nil) 
			(*(WEActionHandle)(*hWE)->hActionStack)->actionKind = weAKPaste;

	// { clean up }
	_WEForgetHandle(&hItem);
	_WEForgetHandle(&hStyles);
	_WEForgetHandle(&hSoup);

	// { return result code }
	return err;
} // { WEPaste }

pascal OSErr WESetStyle(short mode, TextStyle *ts, WEHandle hWE)
{
	WEPtr pWE;
	WEActionHandle hAction;
	ScriptCode fontScript;
	Boolean saveWELock;
	OSErr err;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	// { stop any ongoing inline input session }
	WEStopInlineSession(hWE);

	if (pWE->selStart == pWE->selEnd) 
	{

		// { NULL SELECTION }
		// { first make sure the nullStyle field contains valid information }
		_WESynchNullStyle(hWE);

		// { apply style changes to the nullStyle record }
		_WECopyStyle((WETextStyle *)ts, &pWE->nullStyle.runStyle, pWE->nullStyle.runStyle.tsFace, mode);

		// { if the font was altered, synchronize the keyboard script }
		if (BTST(pWE->flags, weFNonRoman)) 
			if (BTST(mode, kModeFont)) 
			{
				fontScript = FontToScript(pWE->nullStyle.runStyle.tsFont);
				if (fontScript != GetScriptManagerVariable(smKeyScript)) 
					KeyScript(fontScript);
			}
	}
	else
	{
		// { NON-EMPTY SELECTION }

		// { increment modification count }
		pWE->modCount = pWE->modCount + 1;

		// { if undo support is enabled, save the styles of the text range to be affected }
		if (BTST(pWE->flags, weFUndoSupport)) 
		{
			WEClearUndo(hWE);
			if (WENewAction(pWE->selStart, pWE->selEnd, pWE->selEnd - pWE->selStart, weAKSetStyle,
				weAFDontSaveText + weAFDontSaveSoup, hWE, &hAction) == noErr) 
				if (WEPushAction(hAction) != noErr) 
					{ ; }
		}

		// { set the style of the selection range }
		err = _WESetStyleRange(pWE->selStart, pWE->selEnd, mode, (WETextStyle *)ts, hWE);
		if (err != noErr) 
			goto cleanup;

		// { and redraw the text }
		err = _WERedraw(pWE->selStart, pWE->selEnd, hWE);
		if (err != noErr) 
			goto cleanup;
	}

	// { clear the result code }
	err = noErr;

cleanup:
	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;
} // { WESetStyle }

pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEHandle hWE)
{
	WEPtr pWE;
	Boolean saveWELock;
	OSErr err;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);
	pWE = *hWE;

	// { return an error code if this instance is read-only }
	err = weReadOnlyErr;
	if (BTST(pWE->flags, weFReadOnly))
		goto cleanup;

	// { apply the style scrap to the selection range }
	err = _WEApplyStyleScrap(pWE->selStart, pWE->selEnd, hStyles, hWE);
	if (err != noErr) 
		goto cleanup;

	// { redraw the text }
	err = _WERedraw(pWE->selStart, pWE->selEnd, hWE);

cleanup:
	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;
}

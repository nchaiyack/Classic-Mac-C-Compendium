// { WASTE PROJECT: }
// { Low-Level Editing Routines }

// { Copyright � 1993-1995 Marco Piovanelli }
// { All Rights Reserved }

#include "WASTEIntf.h"

pascal Boolean _WEIsWordRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// { _WEIsWordRange returns TRUE if the specified range is a word range, }
	// { i.e. if it would be possible to select it by double-clicking and (optionally) dragging. }

	long wordStart, wordEnd;

	// { determine if rangeStart is at the beginning of a word }
	WEFindWord(rangeStart, kLeadingEdge, &wordStart, &wordEnd, hWE);
	if (rangeStart == wordStart) 
	{

		// { determine if rangeEnd is at the end of a word }
		WEFindWord(rangeEnd, kTrailingEdge, &wordStart, &wordEnd, hWE);
		return (rangeEnd == wordEnd);
	}
	return false;
} // { _WEIsWordRange }

pascal Boolean _WEIsPunct(long offset, WEHandle hWE)
{
	short cType;
	
	cType = WECharType(offset, hWE);
	if ((cType & smcTypeMask) == smCharPunct) 
	{
		cType = cType & smcClassMask;
		if (cType == smPunctNormal || cType == smPunctBlank) 
			return true;
	}
	return false;
}  // { _WEIsPunct }

pascal void _WEIntelligentCut(long *rangeStart, long *rangeEnd, WEHandle hWE)
{

	// { _WEIntelligentCut is called by other WASTE routines to determine the actual }
	// { range to be deleted when weFIntCutAndPaste is enabled. }
	// { On entry, rangeStart and rangeEnd specify the selection range visible to the user. }
	// { On exit, rangeStart and rangeEnd specify the actual range to be removed. }

	// { do nothing if the intelligent cut-and-paste feature is disabled }
	if (!BTST((*hWE)->flags, weFIntCutAndPaste)) 
		return;

	// { intelling cut-&-paste rules should be applied only to word ranges... }
	if (_WEIsWordRange(*rangeStart, *rangeEnd, hWE) == false) 
		return;

	// { ...without punctuation characters at the beginning or end }
	if (_WEIsPunct(*rangeStart, hWE)) 
		return;
	if (_WEIsPunct(*rangeEnd - 1, hWE)) 
		return;

	// { if the character preceding the selection range is a space, discard it }
	if (WEGetChar(*rangeStart - 1, hWE) == kSpace) 
		*rangeStart = *rangeStart - 1;
	// { else, if the character following the selection range is a space, discard it }
	else if (WEGetChar(*rangeEnd, hWE) == kSpace)
		*rangeEnd = *rangeEnd + 1;

} // { _WEIntelligentCut }

pascal short _WEIntelligentPaste(long rangeStart, long rangeEnd, WEHandle hWE)
{
	short retval;
	
	// { _WEIntelligentPaste is called by other WASTE routines to determine whether }
	// { an additional space character should be added (before or after) after inserting }
	// { new text (usually from the Clipboard or from a drag). }

	retval = weDontAddSpaces;

	// { do nothing unless the intelligent cut-and-paste feature is enabled }
	if (!BTST((*hWE)->flags, weFIntCutAndPaste)) 
		return retval;

	// { extra spaces will be added only if the pasted text looks like a word range, }
	// { without punctuation characters at the beginning or at the end }
	if (_WEIsPunct(rangeStart, hWE))
		return retval;
	if (_WEIsPunct(rangeEnd - 1, hWE))
		return retval;

	// { if the character on the left of the pasted text is a punctuation character }
	// { and the character on the right isn't,  add a space on the right, and vice versa }
	if (_WEIsPunct(rangeStart - 1, hWE))
	{
		if (_WEIsPunct(rangeEnd, hWE) == false) 
			retval = weAddSpaceOnRightSide;
	}
	else if (_WEIsPunct(rangeEnd, hWE))
		retval = weAddSpaceOnLeftSide;

	return retval;
} // { _WEIntelligentPaste }

pascal OSErr _WEInsertRun(long runIndex, long offset, long styleIndex, WEPtr pWE)
{

	// { Insert a new element in the style run array, at the specified runIndex position. }
	// { The new element consists of the pair <offset, styleIndex>. }

	RunArrayElement element;
	OSErr err;

	// { prepare the element record to be inserted in the array }
	element.runStart = offset;
	element.styleIndex = styleIndex;

	// { do the insertion }
	err = _WEInsertSlot((Handle)pWE->hRuns, (Ptr)&element, runIndex + 1, sizeof(element));
	if (err != noErr) 
		return err;

	// { increment style run count }
	pWE->nRuns = pWE->nRuns + 1;

	// { increment the reference count field of the style table element }
	// { referenced by the newly inserted style run }
	(*pWE->hStyles)[styleIndex].refCount = (*pWE->hStyles)[styleIndex].refCount + 1;

	return noErr;
} // { _WEInsertRun }

pascal OSErr _WERemoveRun(long runIndex, WEPtr pWE)
{
	// { remove the specified element from the style run array }

	long styleIndex;
	OSErr retval;
	
	styleIndex = (*pWE->hRuns)[runIndex].styleIndex;

	// { do the removal (errors returned by _WERemoveSlot can be safely ignored) }
	retval = _WERemoveSlot((Handle)pWE->hRuns, runIndex, sizeof(RunArrayElement));

	// { decrement style run count }
	pWE->nRuns = pWE->nRuns - 1;

	// { decrement the reference count field of the style table element }
	// { that was referenced by the style run we have just removed }
	(*pWE->hStyles)[styleIndex].refCount =(*pWE->hStyles)[styleIndex]. refCount - 1;

	// { dispose of embedded object, if any }
	if ((*pWE->hStyles)[styleIndex].refCount == 0) 
		if (_WEFreeObject((WEObjectDescHandle)((*pWE->hStyles)[styleIndex].info.runStyle.tsObject))
			!= noErr) 
			{ ; }
	
	return retval;
} // { _WERemoveRun }

pascal void _WEChangeRun(long runIndex, long newStyleIndex, Boolean keepOld, WEPtr pWE)
{
	// { change the styleIndex field of the specified element of the style run array }

	long oldStyleIndex;
	WEObjectDescHandle hObjectDesc;

	// { do the change }
	oldStyleIndex = (*pWE->hRuns)[runIndex].styleIndex;
	(*pWE->hRuns)[runIndex].styleIndex = newStyleIndex;
	
	// { increment the reference count field of the new style table element }
	(*pWE->hStyles)[newStyleIndex].refCount = (*pWE->hStyles)[newStyleIndex].refCount + 1;
	hObjectDesc = (WEObjectDescHandle)((*pWE->hStyles)[newStyleIndex].info.runStyle.tsObject);
	
	// { decrement the reference count field of the old style table element }
	(*pWE->hStyles)[oldStyleIndex].refCount = (*pWE->hStyles)[oldStyleIndex].refCount - 1;

	// { dispose of embedded object, if any, unless it is again referenced in the new style }
	if (((*pWE->hStyles)[oldStyleIndex].refCount == 0) && (keepOld == false))
		if ((WEObjectDescHandle)((*pWE->hStyles)[oldStyleIndex].info.runStyle.tsObject) != hObjectDesc)
			if (_WEFreeObject((WEObjectDescHandle)((*pWE->hStyles)[oldStyleIndex].info.runStyle.tsObject)) != noErr) 
				{ ; }

} // { _WEChangeRun }

pascal OSErr _WENewStyle(WERunAttributes *ts, long *styleIndex, WEPtr pWE)
{

	// { given the specified WERunAttributes record, find the corresponding entry }
	// { in the style table (create a new entry if necessary), and return its index }


	StyleTablePtr pTable;
	StyleTableElement element;
	long index, unusedIndex;
	OSErr err;

	// _WENewStyle = noErr;
	pTable = *pWE->hStyles;

	// { see if the given style already exists in the style table }
	// { while scanning the table, also remember the position of the first unused style, if any }
	index = 0;
	unusedIndex = -1;
	while (index < pWE->nStyles)
	{
		// { check for entries which aren't referenced and can be recycled }
		if (pTable[index].refCount == 0) 
		{
			unusedIndex = index;
		}

		// { perform a bitwise comparison between the current element and the specified style }
		if (_WEBlockCmp((Ptr)&pTable[index].info, (Ptr)ts, sizeof(WERunAttributes)))
		{ 
			*styleIndex = index;		// { found: style already present }
			return noErr;
		}

		index = index + 1;
	} // { while }

	// { the specified style doesn't exist in the style table }
	// { see if we can recycle an unused entry }
	if (unusedIndex >= 0) 
	{
		index = unusedIndex;
		pTable[index].info = *ts;
	}
	else
	{
		// { no reusable entry: we have to append a new element to the table }
		element.refCount = 0;
		element.info = *ts;
		err = _WEInsertSlot((Handle)pWE->hStyles, (Ptr)&element, index, sizeof(element));
		if (err != noErr) 
		{
			return err;
		}

		// { update style count in the WE record }
		pWE->nStyles = index + 1;
	}

	// { return the index to the new element }
	*styleIndex = index;

	return noErr;
} // { _WENewStyle }

pascal OSErr _WERedraw(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// { the WE record is guaranteed to be already locked }
	WEPtr pWE;
	LineArrayPtr pLines;
	long startLine, endLine;
	long oldTextHeight, newTextHeight;
	LongRect r;
	Rect viewRect, updateRect;
	RgnHandle saveClip;
	GrafPtr savePort;
	OSErr err;
#ifdef WEREDRAW_SPEED
	LongRect scrollRect;
	RgnHandle updateRgn,
              utilRgn;
#endif
	
	pWE = *hWE;

	// { do nothing if recalculation has been inhibited }
	if (!BTST(pWE->flags, weFInhibitRecal)) 
	{
		// { hide the caret }
#ifdef WEREDRAW_SPEED
		BCLR(pWE->flags, weFCaretVisible);
#else
		if (BTST(pWE->flags, weFCaretVisible))
		{ 
			_WEBlinkCaret(hWE);
		}
#endif

		// { remember total text height }
		oldTextHeight = pWE->destRect.bottom - pWE->destRect.top;

		// { find line range affected by modification }
		startLine = WEOffsetToLine(rangeStart, hWE);
		endLine = WEOffsetToLine(rangeEnd, hWE);

		// { recalculate line breaks starting from startLine }
		err = _WERecalBreaks(&startLine, &endLine, hWE);
		if (err != noErr) 
		{
			goto cleanup;
		}

		// { recalculate slops }
		_WERecalSlops(startLine, endLine, hWE);

		// { calculate new total text height }
		newTextHeight = pWE->destRect.bottom - pWE->destRect.top;

		// { calculate the rectangle to redraw (in long coordinates) }
		r.left = -SHRT_MAX;
		r.right = SHRT_MAX;
		pLines = *pWE->hLines;
		r.top = pLines[startLine].lineOrigin;

#ifdef WEREDRAW_SPEED
		// { if total text height hasn't changed, it's enough to redraw lines up to endLine }
		// { otherwise we must redraw all lines from startLine on }

		if (endLine < pWE->nLines - 1)
			 r.bottom = pLines[endLine + 1].lineOrigin;
		else
			r.bottom = newTextHeight;
		WEOffsetLongRect(&r, 0, pWE->destRect.top);

		if (newTextHeight == oldTextHeight)
			WELongRectToRect(&r, &updateRect);
		else
		{
			/*      Instead of scrolling the lines below the deleted text up by redrawing them,
			 *      use scroll bits to move the displayed text up.
			 */

			scrollRect = pWE->viewRect;
			if (newTextHeight > oldTextHeight)
				scrollRect.top = pLines[startLine + 1].lineOrigin + pWE->destRect.top;
			else
				scrollRect.top = pLines[startLine].lineOrigin + pWE->destRect.top;
			WELongRectToRect(&scrollRect, &updateRect);
			updateRgn = NewRgn();
			ScrollRect(&updateRect, 0, newTextHeight - oldTextHeight, updateRgn);

			/*      Redraw the exposed region (caused by a scroll up)       */

			WELongRectToRect(&r, &updateRect);
			utilRgn = NewRgn();
			RectRgn(utilRgn, &updateRect);
			DiffRgn(updateRgn, utilRgn, updateRgn);
			DisposeRgn(utilRgn);
			WEUpdate(updateRgn, hWE);
			DisposeRgn(updateRgn);
		}
#else
		// { if total text height hasn't changed, it's enough to redraw lines up to endLine }
		// { otherwise we must redraw all lines from startLine on }
		if ((newTextHeight == oldTextHeight) && (endLine < pWE->nLines - 1)) 
		{
			r.bottom = pLines[endLine + 1].lineOrigin;
		}
		else if (newTextHeight < oldTextHeight) 
		{
			r.bottom = oldTextHeight;
		}
		else
		{
			r.bottom = newTextHeight;
		}
		
		WEOffsetLongRect(&r, 0, pWE->destRect.top);

		// { calculate the intersection between this rectangle and the view rectangle }
		WELongRectToRect(&r, &updateRect);
#endif
		WELongRectToRect(&pWE->viewRect, &viewRect);

		if (SectRect(&updateRect, &viewRect, &updateRect)) 
		{
			// { set up the port and the clip region }
			GetPort(&savePort);
			SetPort(pWE->port);

			// { set the clip region to updateRect }
			saveClip = NewRgn();
			GetClip(saveClip);
			ClipRect(&updateRect);

			// { we only really need to redraw the visible lines }
			startLine = _WEPixelToLine(updateRect.top - pWE->destRect.top, hWE);
			endLine = _WEPixelToLine(updateRect.bottom - pWE->destRect.top - 1, hWE);

			// { redraw the lines (pass TRUE in the doErase parameter) }
			_WEDrawLines(startLine, endLine, true, hWE);

			// { erase the portion of the update rectangle below the last line (if any) }
			pLines = *pWE->hLines;
			updateRect.top = pWE->destRect.top + pLines[endLine + 1].lineOrigin;
			if (updateRect.top < updateRect.bottom) 
			{
				EraseRect(&updateRect);
			}

			// { restore the clip region }
			SetClip(saveClip);
			DisposeRgn(saveClip);

			// { restore the port }
			SetPort(savePort);

			// { redraw the caret or the selection range }
			if (pWE->selStart < pWE->selEnd) 
			{
				_WEHiliteRange(pWE->selStart, pWE->selEnd, hWE);
			}
			else
			{
				_WEBlinkCaret(hWE);
			}
		} // { if SectRect }

		// { scroll the selection range into view }
		WESelView(hWE);
	} // { if recal not inhibited }

	// { clear result code }
	err = noErr;

cleanup:
	// { return result code }
	return err;
} // { _WERedraw }

pascal OSErr WECalText(WEHandle hWE)
{
	Boolean saveWELock;
	OSErr err;

	// { lock WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);

	err = _WERedraw(0, 0x7fffffff, hWE);

	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	return err;
}

pascal OSErr _WESetStyleRange(long rangeStart, long rangeEnd, short mode, WETextStyle *ts, WEHandle hWE)
{
	// { alter the style attributes of the specified text range according to ts and mode }
	// { the WE record is guaranteed to be already locked }

	WEPtr pWE;
	RunArrayHandle hRuns;
	long offset;
	long runIndex;
	long oldStyleIndex, newStyleIndex;
	WERunInfo runInfo;
	short temp;
	char continuousStyles;
	OSErr err;

#ifdef WASTE_DEBUG
        _WEAssert (rangeStart < rangeEnd, "\pbad style range");
#endif

	pWE = *hWE;
	hRuns = pWE->hRuns;

	// { if mode contains weDoToggleFace, we need to determine which QuickDraw styles }
	// { are continuous over the specified text range: those styles must be turned off }
	if (BTST(mode, kModeToggleFace)) 
	{
		temp = weDoFace;
		_WEContinuousStyleRange(rangeStart, rangeEnd, &temp, &runInfo.runAttrs.runStyle, hWE);
		continuousStyles = runInfo.runAttrs.runStyle.tsFace;
	}
	else
	{
		continuousStyles = 0;
	}

	// { find the index to the first style run in the specified range }
	offset = rangeStart;
	runIndex = _WEOffsetToRun(offset, hWE);

	// { run thru all the style runs that encompass the selection range }
	do
	{
		// { find style index for this run and retrieve corresponding style attributes }
		oldStyleIndex = (*hRuns)[runIndex].styleIndex;
		_WEGetIndStyle(runIndex, &runInfo, hWE);

		// { _WEGetIndStyle returns textLength + 1 in runInfo.runEnd for the last style run: }
		// { correct this anomaly (which is useful for other purposes, anyway) }
		if (runInfo.runEnd > pWE->textLength) 
		{
			runInfo.runEnd = pWE->textLength;
		}
		
		// { apply changes to existing style attributes as requested }
		_WECopyStyle(ts, &runInfo.runAttrs.runStyle, continuousStyles, mode);

		// { recalculate font metrics, if necessary }
		if ((mode & (weDoFont + weDoSize + weDoFace + weDoAddSize)) != 0) 
		{
			_WEFillFontInfo(pWE->port, &runInfo.runAttrs);
		}
		
		// { get a style index for the new attributes }
		err = _WENewStyle(&runInfo.runAttrs, &newStyleIndex, pWE);
		if (err != noErr) 
		{
			goto cleanup;
		}
		
		// { if offset falls on a style boundary and this style run has become identical }
		// { to the previous one, merge the two runs together }
		if ((offset == runInfo.runStart) && (runIndex > 0) &&
			((*hRuns)[runIndex - 1].styleIndex == newStyleIndex))
		{
			err = _WERemoveRun(runIndex, pWE);
			if (err != noErr) 
			{
				goto cleanup;
			}
			runIndex = runIndex - 1;
		}

		// { style index changed? }
		if (oldStyleIndex != newStyleIndex) 
		{
			// { if offset is in the middle of a style run, insert a new style run in the run array }
			if (offset > runInfo.runStart) 
			{
				err = _WEInsertRun(runIndex, offset, newStyleIndex, pWE);
				if (err != noErr) 
				{
					goto cleanup;
				}
				runIndex = runIndex + 1;
			}
			else
			{
				// { otherwise just change the styleIndex field of the current style run element }
				_WEChangeRun(runIndex, newStyleIndex, (rangeEnd < runInfo.runEnd), pWE);
			}

			// { if specified range ends in the middle of a style run, insert yet another element }
			if (rangeEnd < runInfo.runEnd) 
			{
				err = _WEInsertRun(runIndex, rangeEnd, oldStyleIndex, pWE);
				if (err != noErr) 
				{
					goto cleanup;
				}
			}
		} // { if oldStyle != newStyle }

		// { go to next style run }
		runIndex = runIndex + 1;
		offset = runInfo.runEnd;

	} while (offset < rangeEnd);

	// { if the last style run ends exactly at the end of the specified range, }
	// { see if we can merge it with the following style run }
	if ((offset == rangeEnd) && (runIndex < pWE->nRuns) && 
		((*hRuns)[runIndex].styleIndex == newStyleIndex)) 
	{
		err = _WERemoveRun(runIndex, pWE);
		if (err != noErr) 
		{
			goto cleanup;
		}
	}

	// { clear result code }
	err = noErr;

cleanup:
	// { return result code }
	return err;
} // { _WESetStyleRange }

pascal OSErr _WEApplyStyleScrap(long rangeStart, long rangeEnd, StScrpHandle styleScrap, WEHandle hWE)
{
	// { apply the given style scrap to the specified text range }
	// { the WE record is guaranteed to be already locked }

	WEPtr pWE;
	TEStyleScrapPeek pElement;
	long runStart, runEnd;
	short index, lastElement;
	WETextStyle ts;
	OSErr err;

	// _WEApplyStyleScrap = noErr;
	pWE = *hWE;

	// { loop through each element of the style scrap }
	lastElement = (*styleScrap)->scrpNStyles - 1;
	for(index = 0; index<=lastElement; index++)
	{
		// { get a pointer to the current scrap element }
		pElement = (TEStyleScrapPeek)&(*styleScrap)->scrpStyleTab[index];

		// { calculate text run to which this element is to be applied }
		runStart = rangeStart + pElement->first.scrpStartChar;
		if (index < lastElement) 
		{
			runEnd = rangeStart + pElement->second.scrpStartChar;
		}
		else
		{
			runEnd = rangeEnd;
		}

		// { perform some range checking }
		if (runEnd > rangeEnd) 
		{
			runEnd = rangeEnd;
		}
		if (runStart >= runEnd) 
		{
			continue;
		}

		// { copy style to a local variable in case memory moves }
		*(TextStyle *)&ts = pElement->first.scrpTEAttrs.runTEStyle;
		// { apply the specified style to the range }
		err = _WESetStyleRange(runStart, runEnd, weDoAll + weDoReplaceFace, &ts, hWE);
		if (err != noErr) 
		{
			return err;
		}
	}
	return noErr;
} // { _WEApplyStyleScrap }

pascal OSErr _WEApplySoup(long offset, Handle hSoup, WEHandle hWE)
{
	WESoupPtr pSoup;
	long pSoupEnd;
	WETextStyle ts;
	Handle hObjectData;
	long objectOffset;
	Boolean saveWELock;
	OSErr err;

	_WEBlockClr((Ptr)&ts, sizeof(ts));
	hObjectData = nil;

	// { lock the WE record }
	saveWELock = _WESetHandleLock((Handle)hWE, true);

	// { lock the soup in high heap }
	HLockHi(hSoup);
	pSoup = (WESoupPtr)*hSoup;
	pSoupEnd = (long)pSoup + GetHandleSize(hSoup);

	// { loop through each object descriptor in the soup }
	while((long)pSoup < pSoupEnd)
	{
		// { if soupDataSize is negative, this soup is a special type that we won't handle here }
		if (pSoup->soupDataSize < 0)
			continue;

		// { create a new relocatable block the hold the object data }
		err = _WEAllocate(pSoup->soupDataSize, kAllocTemp, &hObjectData);
		if (err != noErr) 
			goto cleanup;

		// { copy the object data to this block }
		BlockMoveData((Ptr)(pSoup + 1), *hObjectData, pSoup->soupDataSize);

		// { create a new object out of the tagged data }
		err = _WENewObject(pSoup->soupType, hObjectData, hWE, (WEObjectDescHandle *)&ts.tsObject);
		if (err != noErr) 
			goto cleanup;

		// { if there was no new handler for this object, use the object size stored in the soup }
		if ((*(WEObjectDescHandle)(ts.tsObject))->objectTable == nil)
			(*(WEObjectDescHandle)(ts.tsObject))->objectSize = pSoup->soupSize;

		// { record a reference to the object descriptor in the style table }
		objectOffset = pSoup->soupOffset + offset;
		err = _WESetStyleRange(objectOffset, objectOffset + 1, weDoObject, &ts, hWE);
		hObjectData = nil;
		ts.tsObject = kNullObject;
		if (err != noErr) 
			goto cleanup;

		// { advance soup pointer }
		pSoup = (WESoupPtr)((long)pSoup + sizeof(WESoup) + pSoup->soupDataSize);

	} // { while }

	// { clear result code }
	err = noErr;

cleanup:
	// { clean up }
	HUnlock(hSoup);
	_WEForgetHandle((Handle *)&ts.tsObject);
	_WEForgetHandle(&hObjectData);

	// { unlock the WE record }
	_WESetHandleLock((Handle)hWE, saveWELock);

	// { return result code }
	return err;

} // { _WEApplySoup }

pascal void _WEBumpRunStart(long runIndex, long deltaRunStart, WEPtr pWE)
{
	// { add deltaLineStart to the lineStart field of all line records }
	// { starting from lineIndex }

	long *pStart;
	long nRuns;

	pStart = &(*pWE->hRuns)[runIndex].runStart;
	nRuns = pWE->nRuns;

	// { loop through the style run array adjusting the runStart fields }
	while (runIndex <= nRuns)
	{
		*pStart = *pStart + deltaRunStart;
		pStart = (long *)((long)(pStart) + sizeof(RunArrayElement));
		runIndex = runIndex + 1;
	}
} // { _WEBumpRunStart }

pascal OSErr _WERemoveRunRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// { the range of text between rangeStart and rangeEnd is being deleted }
	// { update the style run array (and the style table) accordingly }
	// { the WE handle must be locked on entry }

	WEPtr pWE;
	RunArrayPeek pRuns;
	long startRun, endRun;
	OSErr err;

	pWE = *hWE;

	// { find the index to the first and last style runs in the specified range }
	startRun = _WEOffsetToRun(rangeStart, hWE);
	endRun = _WEOffsetToRun(rangeEnd, hWE) - 1;

	// { remove all style runs between startRun and endRun }
	while (endRun > startRun)
	{
		err = _WERemoveRun(endRun, pWE);
		if (err != noErr) 
		{
			goto cleanup;
		}
		endRun = endRun - 1;
	}
	
	// { move back all subsequent style runs }
	_WEBumpRunStart(startRun + 1, rangeStart - rangeEnd, pWE);

	if ((endRun == startRun) && (endRun < pWE->nRuns - 1))
	{ 
		pRuns = (RunArrayPeek)&(*pWE->hRuns)[endRun];
		pRuns->second.runStart = rangeStart;
	}

	// { remove the first style run if is has become zero length }
	pRuns = (RunArrayPeek)&(*pWE->hRuns)[startRun];
	if (pRuns->first.runStart == pRuns->second.runStart) 
	{
		err = _WERemoveRun(startRun, pWE);
		if (err != noErr) 
		{
			goto cleanup;
		}
		startRun = startRun - 1;
	}

	// { merge the first and last runs if they have the same style index }
	if (startRun >= 0) 
	{
		pRuns = (RunArrayPeek)&(*pWE->hRuns)[startRun];
		if (pRuns->first.styleIndex == pRuns->second.styleIndex) 
		{
			err = _WERemoveRun(startRun + 1, pWE);
			if (err != noErr) 
			{
				goto cleanup;
			}
		}
	}
	// { clear result code }
	err = noErr;

cleanup:
	// { return result code }
	return err;
} // { _WERemoveRunRange }

pascal void _WEBumpLineStart(long lineIndex, long deltaLineStart, WEPtr pWE)
{
	// { add deltaLineStart to the lineStart field of all line records }
	// { starting from lineIndex }

	long *pStart;
	long nLines;

	pStart = &(*pWE->hLines)[lineIndex].lineStart;
	nLines = pWE->nLines;

	// { loop through the line array adjusting the lineStart fields }
	while (lineIndex <= nLines)
	{
		*pStart = *pStart + deltaLineStart;
		pStart = (long *)((long)pStart + sizeof(LineRec));
		lineIndex = lineIndex + 1;
	}
} // { _WEBumpLineStart }

pascal OSErr _WERemoveLineRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// { the range of text between rangeStart and rangeEnd is being deleted }
	// { update the line array accordingly }
	// { the WE handle must be locked on entry }

	WEPtr pWE;
	long startLine, endLine;
	OSErr err;
	
	// _WERemoveLineRange = noErr;
	pWE = *hWE;

	// { remove all line records between rangeStart and rangeEnd }
	startLine = WEOffsetToLine(rangeStart, hWE) + 1;
	endLine = WEOffsetToLine(rangeEnd, hWE);
	while (endLine >= startLine)
	{
		err = _WERemoveLine(endLine, pWE);
		if (err != noErr) 
		{
			return err;
		}
		endLine = endLine - 1;
	} // { while }

	// { update the lineStart field of all the line records that follow }
	_WEBumpLineStart(startLine, rangeStart - rangeEnd, pWE);

	return noErr;
}

pascal OSErr _WEDeleteRange(long rangeStart, long rangeEnd, WEHandle hWE)
{
	// { used internally to delete a text range }
	// { if saveNullStyle is TRUE, the first style in the range is saved in nullStyle }
	// { the WE record is guaranteed to be already locked }

	WEPtr pWE;
	WERunInfo runInfo;
	long oldTextLength, newTextLength;
	long pText;
	OSErr err;

	pWE = *hWE;

	if (rangeEnd>pWE->textLength) rangeEnd = pWE->textLength;
	
	// { do nothing if the specified range is empty }
	if (rangeStart == rangeEnd) 
	{
		goto cleanup1;
	}
	
	// { save the first style in the specified range in nullStyle }
	WEGetRunInfo(rangeStart, &runInfo, hWE);
	pWE->nullStyle = runInfo.runAttrs;
	BSET(pWE->flags, weFUseNullStyle);

	// { special case: if we're deleting up to the end of the text, see whether }
	// { there's an embedded object at the very end and remove it }
	if (rangeEnd == pWE->textLength) 
	{
		WEGetRunInfo(rangeEnd - 1, &runInfo, hWE);
		if (runInfo.runAttrs.runStyle.tsObject != kNullObject) 
		{
			runInfo.runAttrs.runStyle.tsObject = kNullObject;
			err = _WESetStyleRange(rangeEnd - 1, rangeEnd, weDoObject, &runInfo.runAttrs.runStyle, hWE);
			if (err != noErr) 
				goto cleanup2;
		}
	}

	// { remove all line records between rangeStart and rangeEnd }
	err = _WERemoveLineRange(rangeStart, rangeEnd, hWE);
	if (err != noErr) 
	{
		goto cleanup2;
	}
	
	// { remove all style runs between rangeStart and rangeEnd }
	err = _WERemoveRunRange(rangeStart, rangeEnd, hWE);
	if (err != noErr) 
	{
		goto cleanup2;
	}
	
	// { calculate old and new text length }
	oldTextLength = pWE->textLength;
	newTextLength = oldTextLength - (rangeEnd - rangeStart);

	// { move the end of the text backwards over the old selection range }
	pText = (long)(*pWE->hText);
	BlockMoveData((Ptr)pText + rangeEnd, (Ptr)pText + rangeStart, oldTextLength - rangeEnd);

	// { compact the text handle }
	SetHandleSize((Handle)pWE->hText, newTextLength);
	err = MemError();
	if (err != noErr) 
	{
		goto cleanup2;
	}

	// { update textLength field }
	pWE->textLength = newTextLength;

cleanup1:
	// { clear result code }
	err = noErr;

cleanup2:
	// { return result code }
	return err;
} // { _WEDeleteRange }

pascal OSErr _WEInsertText(long offset, Ptr textPtr, long textLength, WEHandle hWE)
{
	// { this routine assumes that the WE record is already locked }

	WEPtr pWE;
	long oldTextLength, newTextLength;
	long pInsPoint;
	OSErr err;
	short mode = 0;

	pWE = *hWE;

	// { do nothing if textLength is zero or negative }
	if (textLength <= 0) 
	{
		goto cleanup1;
	}
	
	// { calculate old and new length of text handle }
	oldTextLength = pWE->textLength;
	newTextLength = oldTextLength + textLength;

	// { lengthen the raw text handle }
	SetHandleSize(pWE->hText, newTextLength);
	err = MemError();
	if (err != noErr) 
		goto cleanup2;
	
	// { calculate ptr to insertion point }
	pInsPoint = (long)(*pWE->hText) + offset;

	// { make room for the new text }
	BlockMoveData((Ptr)pInsPoint, (Ptr)(pInsPoint + textLength), oldTextLength - offset);

	// { insert new text at the insertion point }
	BlockMoveData(textPtr, (Ptr)pInsPoint, textLength);

	// { update the lineStart fields of all lines following the insertion point }
	_WEBumpLineStart(WEOffsetToLine(offset, hWE) + 1, textLength, pWE);

	// { update the runStart fields of all style runs following the insertion point }
	_WEBumpRunStart(_WEOffsetToRun(offset - 1, hWE) + 1, textLength, pWE);

	// { update various fields in the WE record }
	pWE->textLength = newTextLength;

	// { make sure the newly inserted text doesn't reference any embedded object }
	pWE->nullStyle.runStyle.tsObject = kNullObject;
	mode = weDoObject;

	// { if there is a valid null style, apply it to the newly inserted text }
	if (BTST(pWE->flags, weFUseNullStyle))
		mode = mode + (weDoAll + weDoReplaceFace);

	err = _WESetStyleRange(offset, offset + textLength, mode, &pWE->nullStyle.runStyle, hWE);
	if (err != noErr)
	{
		goto cleanup2;
	}

cleanup1:
	// { clear result code }
	err = noErr;

cleanup2:
	// { return result code }
	return err;
} // { _WEInsertText }

/*
{ WASTE PROJECT }
{ Interface to miscellaneous utility routines }
{ Most routines are defined in WASTEUtils.Lib }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }
 */

#ifndef __TYPES__
#include <Types.h>
#endif __TYPES__

#ifndef __MEMORY__
#include <Memory.h>
#endif __MEMORY__

#include "WASTEIntf.h"
pascal Boolean _WEBlockCmp(register Ptr block1, register Ptr block2, register long blockSize)
{
	for ( ; blockSize > 0 ; blockSize-- )
		if ( *block1++ != *block2++ )
			return false;
	
	return true;
}

pascal void _WEBlockClr(register Ptr block, register long blockSize)
{
	for ( ; blockSize > 0; blockSize-- )
		*block++ = 0;
}

pascal void _WEForgetHandle(Handle *h)
{
	Handle theHandle = *h;
	
	if (theHandle != NULL)
	{
		*h = NULL;
		DisposeHandle(theHandle);
	}
}

pascal Boolean _WESetHandleLock(Handle h, Boolean lock)
{
	Boolean oldLock = (HGetState(h) & (1 << 7)) != 0;
	
	if (lock != oldLock)
		if (lock)
			HLock(h);
		else
			HUnlock(h);
			
	return oldLock;
}

pascal void _WEReorder(long *a, long *b)
{
	if (*a > *b)
	{
		register long temp = *a;
		*a = *b;
		*b = temp;
	}
}

pascal OSErr _WEAllocate(Size blockSize, short allocFlags, Handle *h)
{
	//{ Allocate a new relocatable block. }
	//{ AllocFlags may specify whether the block should be cleared and whether }
	//{ temporary memory should be used. }

	Handle theHandle;
	OSErr retval;
	
	theHandle = nil;

	//{ if kAllocTemp is specified, try tapping temporary memory }
	if ((allocFlags & kAllocTemp) != 0) 
	{
		theHandle = TempNewHandle(blockSize, &retval);
	}
	//{ if kAllocTemp isn't specified, or TempNewHandle failed, try with current heap }
	if (theHandle == nil) 
	{
		theHandle = NewHandle(blockSize);
		retval = MemError();
	}
	
	//{ if kAllocClear is specified, zero the block }
	if ((allocFlags & kAllocClear) != 0) 
	{
		if (theHandle != nil) 
		{
			_WEBlockClr(*theHandle, blockSize);
		}
	}

	//{ return handle through VAR parameter }
	*h = theHandle;
	
	return retval;
}
 be added (before or after) after inserting }
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
} // { _
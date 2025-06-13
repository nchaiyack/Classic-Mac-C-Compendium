// { WASTE PROJECT: }
// { Embedded Objects }

// { Copyright © 1993-1995 Marco Piovanelli }
// { All Rights Reserved }

#include "WASTEIntf.h"

const short kUnknownObjectType = -1;		// { specifies an object type for which no handlers are installed }
const Point kDefaultObjectSize = {32, 32};	// { default object size (32x32 pixels) }

// MPW needs topLeft and botRight defined
#ifdef applec
#define topLeft(r)              (((Point *) &(r))[0])
#define botRight(r)             (((Point *) &(r))[1])
#endif

typedef struct WEOHTableElement {
	OSType objectType;			// { 4-letter tag identifying object type }
	WENewObjectUPP newHandler;
	WEDisposeObjectUPP freeHandler;
	WEDrawObjectUPP drawHandler;
	WEClickObjectUPP clickHandler;
	UniversalProcPtr cursorHandler;
} WEOHTableElement, *WEOHTablePtr, **WEOHTableHandle;

// { static variables }

static Handle _weGlobalObjectHandlerTable;

pascal OSType WEGetObjectType(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectType;
} // { WEGetObjectType }

pascal Handle WEGetObjectDataHandle(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectDataHandle;
} // { WEGetObjectDataHandle }

pascal Point WEGetObjectSize(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectSize;
} // { WEGetObjectSize }

pascal WEHandle WEGetObjectOwner(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectOwner;
} // { WEGetObjectOwner }

pascal long WEGetObjectRefCon(WEObjectDescHandle hObjectDesc)
{
	return (*hObjectDesc)->objectRefCon;
} // { WEGetObjectRefCon }

pascal void WESetObjectRefCon(WEObjectDescHandle hObjectDesc, long refCon)
{
	(*hObjectDesc)->objectRefCon = refCon;
} // { WESetObjectRefCon }

pascal short _WELookupObjectType(OSType objectType, Handle hTable)
{

	// { look for a WEOHTableElement record for the specified object kind }
	// { in the given object handler table }

	short nEntries, index;

	// { do nothing if the Object Handler Table has not been inited yet }
	if (hTable == nil) return kUnknownObjectType;

	// { calculate entry count }
	nEntries = GetHandleSize(hTable) / sizeof(WEOHTableElement);

	// { scan the Object Handler Table looking for a type match }
	for (index = nEntries - 1; index>=0; index--)
	{
		if ((*(WEOHTableHandle)hTable)[index].objectType == objectType) 
			return index;
	}
	
	return kUnknownObjectType;
} // { _WELookupObjectType }

pascal OSErr _WEGetIndObjectType(short index, OSType *objectType, WEHandle hWE)
{
	Handle hTable;
	short nEntries;
	OSErr err;
	
	err = weUnknownObjectTypeErr; // { assume failure }
	*objectType = 0L;

	// { index must be non-negative }
	if (index < 0) return err;
	
	// { calculate number of entries in the instance-specific handler table }
	nEntries = 0;
	hTable = (*hWE)->hObjectHandlerTable;
	if (hTable != nil)
		nEntries = GetHandleSize(hTable) / sizeof(WEOHTableElement);

	// { low indices refer to the instance-specific handler table }
	if (index < nEntries)
	{
		*objectType = (*(WEOHTableHandle)hTable)[index].objectType;
		return noErr;
	}
	// { indices above that refer to the global handler table }
	index = index - nEntries;

	// { calculate number of entries in the global handler table }
	nEntries = 0;
	hTable = _weGlobalObjectHandlerTable;
	if (hTable != nil)
		nEntries = GetHandleSize(hTable) / sizeof(WEOHTableElement);

	// { return an error code if index is too large }
	if (index >= nEntries)
		return err;

	*objectType = (*(WEOHTableHandle)hTable)[index].objectType;
	
	return noErr;
} // { _WEGetIndObjectType }

pascal OSErr _WENewObject(OSType objectType, Handle objectDataHandle, WEHandle hWE,
					WEObjectDescHandle *hObjectDesc)
{
	WEObjectDescPtr pDesc;
	short index;
	OSErr err;
	Handle hTable;
	
	*hObjectDesc = nil;

	// { first look up the specified object type in the instance-specific handler table }
	hTable = (*hWE)->hObjectHandlerTable;
	index = _WELookupObjectType(objectType, hTable);
	if (index == kUnknownObjectType)
	{
		// { no match: try with the global handler table }
		hTable = _weGlobalObjectHandlerTable;
		index = _WELookupObjectType(objectType, hTable);
		if (index == kUnknownObjectType)
			hTable = nil;
	}
	
	// { look up the specified object type in the handler table }
	index = _WELookupObjectType(objectType, hTable);

	// { create a new relocatable block to hold the object descriptor }
	err = _WEAllocate(sizeof(WEObjectDesc), kAllocClear, (Handle *)hObjectDesc);
	if (err != noErr) return err;

	// { lock it down }
	HLock((Handle)*hObjectDesc);
	pDesc = (WEObjectDescPtr)**hObjectDesc;

	// { fill in the object descriptor }
	pDesc->objectType = objectType;
	pDesc->objectDataHandle = objectDataHandle;
	pDesc->objectSize = kDefaultObjectSize;
	pDesc->objectTable = hTable;
	pDesc->objectIndex = index;
	pDesc->objectOwner = hWE;

	if (hTable != nil) 
	{
		// { call the new handler, if any }
		if ((*(WEOHTableHandle)hTable)[index].newHandler != nil) 
		{
			err = CallWENewObjectProc(&pDesc->objectSize, *hObjectDesc,
				(*(WEOHTableHandle)hTable)[index].newHandler);
			if (err != noErr) 
			{
				_WEForgetHandle((Handle *)hObjectDesc);
				return err;
			}
		}
	}
	
	// { unlock the object descriptor }
	HUnlock((Handle)*hObjectDesc);

	// { clear result code }
	return noErr;
} // { _WENewObject }

pascal OSErr _WEFreeObject(WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	OSErr retval;
	
	retval = noErr;

	// { sanity check: do nothing if we have a null descriptor handle }
	if (hObjectDesc == nil) 
		return nilHandleErr;

	// { lock the descriptor record }
	HLock((Handle)hObjectDesc);
	pDesc = *hObjectDesc;

	if (pDesc->objectTable != nil) 
	{
#ifdef WASTE_DEBUG
		// { sanity check: make sure object kind matches handler kind }
		_WEAssert(pDesc->objectType == (*(WEOHTableHandle)pDesc->objectTable)
			[pDesc->objectIndex].objectType, "\pObject Type Mismatch");
#endif

		// { call the dispose handler, if any }
		if ((*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].freeHandler != nil) 
		{
			retval = CallWEDisposeObjectProc(hObjectDesc,
				(*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].freeHandler);
			pDesc->objectDataHandle = nil;
		}
	}
	
	// { if object kind is unknown or there's no custom dispose handler, use DisposeHandle }
	_WEForgetHandle(&pDesc->objectDataHandle);

	// { finally, dispose of the object descriptor itself }
	DisposeHandle((Handle)hObjectDesc);

	return retval;
} // { _WEFreeObject }

pascal OSErr _WEDrawObject(WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	Rect destRect;
	OSErr retval = noErr;
	PenState state;
	Boolean saveDescLock;
	
	// { lock the object descriptor }
	saveDescLock = _WESetHandleLock((Handle)hObjectDesc, true);
	pDesc = *hObjectDesc;

	// { get current pen state }
	// { state.pnLoc has already been set to the bottom left of the rectangle to draw }
	GetPenState(&state);

	// { calculate the new pen position }
	state.pnLoc.h = state.pnLoc.h + pDesc->objectSize.h;

	// { calculate the destination rectangle }
	*(long *)(&topLeft(destRect)) = DeltaPoint(state.pnLoc, pDesc->objectSize);
	botRight(destRect) = state.pnLoc;

	if (pDesc->objectTable != 0) 
	{
#ifdef WASTE_DEBUG
		// { sanity check: make sure object kind matches handler kind }
		_WEAssert(pDesc->objectType == (*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].objectType,
			"\pObject Type Mismatch");
#endif

		// { call the drawing handler, if any }
		if ((*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].drawHandler != nil) 
			retval = CallWEDrawObjectProc(&destRect, hObjectDesc, 
						(*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].drawHandler);
	}
	else
	{
		// { if this object kind was not registered, draw an empty frame }
		PenNormal();
		FrameRect(&destRect);
	}
	
	// { restore original pen state, advancing the pen position by the object width }
	SetPenState(&state);

	// { unlock the object descriptor }
	_WESetHandleLock((Handle)hObjectDesc, saveDescLock);

	return retval;
} // { _WEDrawObject }

pascal Boolean _WEClickObject(Point hitPt, short modifiers, long clickTime,
								WEObjectDescHandle hObjectDesc)
{
	WEObjectDescPtr pDesc;
	Boolean saveDescLock;
	Boolean clickHandled = false;

	// { lock the object descriptor }
	saveDescLock = _WESetHandleLock((Handle)hObjectDesc, true);
	pDesc = *hObjectDesc;

	if (pDesc->objectTable != 0)
	{
#ifdef WASTE_DEBUG
		// { sanity check: make sure object kind matches handler kind }
		_WEAssert(pDesc->objectType = (*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].objectType,
			"\pObject Type Mismatch");
#endif

		// { call the click handler, if any }
		if ((*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].clickHandler != nil)
			clickHandled = CallWEClickObjectProc(hitPt, modifiers, clickTime, hObjectDesc,
				(*(WEOHTableHandle)pDesc->objectTable)[pDesc->objectIndex].clickHandler);
	}

	// { unlock the object descriptor }
	_WESetHandleLock((Handle)hObjectDesc, saveDescLock);
	
	return clickHandled;
} // { _WEClickObject }

pascal OSErr WEInstallObjectHandler(OSType objectType, OSType handlerSelector,
				UniversalProcPtr handler, WEHandle hWE)
{
	Handle hTable;
	short index;
	WEOHTableElement element;
	OSErr err;

	// { if hWE is NIL, install the handler in the global handler table, }
	// { otherwise install the handler in the instance-specific handler table }
	if (hWE == nil)
		hTable = _weGlobalObjectHandlerTable;
	else
		hTable = (*hWE)->hObjectHandlerTable;

	// { create the handler table, if it doesn't exist }
	if (hTable == nil) 
	{
		hTable = NewHandle(0);
		err = MemError();
		if (err != noErr) 
			return err;
		if (hWE == nil)
			_weGlobalObjectHandlerTable = hTable;
		else
			(*hWE)->hObjectHandlerTable = hTable;
	}

	// { look for the entry corresponding to the specified object type }
	index = _WELookupObjectType(objectType, hTable);

	if (index == kUnknownObjectType) 
	{

		_WEBlockClr((Ptr)&element, sizeof(element));
		element.objectType = objectType;
		// { previously unknown object type: append a new entry at the end of the handler table }
		index = GetHandleSize(hTable) / sizeof(WEOHTableElement);
		err = _WEInsertSlot(hTable, (Ptr)&element, index, sizeof(element));
		if (err != noErr) 
			return err;
	}
	
	// { install the handler }
	err = _WESetHandler(handlerSelector, (long *)&handler, 
		(void *)&(*(WEOHTableHandle)hTable)[index].objectType);

	// { return result code }
	return err;

} // { WEInstallObjectHandler }
*pChars;
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

	// { finally, the selection range mustn't have m
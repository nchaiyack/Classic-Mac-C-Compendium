// { WASTE PROJECT: }
// { Creation and Destruction, Standard Procs, etc. }

// { Copyright � 1993-1995 Marco Piovanelli }
// { All Rights Reserved }

#include "WASTEIntf.h"
#include <Palettes.h>
#include <QDOffscreen.h>
//#include <GestaltEqu.h>
#include <Drag.h>

//  { static variables }

static WEDrawTextUPP		_weStdDrawTextProc = NULL;
static WEPixelToCharUPP		_weStdPixelToCharProc = NULL;
static WECharToPixelUPP		_weStdCharToPixelProc = NULL;
static WELineBreakUPP		_weStdLineBreakProc = NULL;
static WEWordBreakUPP		_weStdWordBreakProc = NULL;
static WECharByteUPP		_weStdCharByteProc = NULL;
static WECharTypeUPP		_weStdCharTypeProc = NULL;

pascal void _WEStdDrawText(Ptr pText, long textLength, Fixed slop, 
				JustStyleCode styleRunPosition, WEHandle hWE)
{
#pragma unused(hWE)
	DrawJustified(pText, textLength, slop, styleRunPosition,
		  kOneToOneScaling, kOneToOneScaling);
} // { _WEStdDrawText }

pascal long _WEStdPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE)
{
#pragma unused(hPos, hWE)
	Fixed lastWidth;
	long retVal;
	Boolean theEdge = *edge;

	lastWidth = *width;
	retVal = PixelToChar(pText, textLength, slop, lastWidth, &theEdge,
		width, styleRunPosition, kOneToOneScaling, kOneToOneScaling);
	*edge = theEdge;

	// { round width to nearest integer value }
	// { (this is supposed to fix an incompatibility with the WorldScript Power Adapter) }
	*width = BSL(FixRound(*width), 16);

	return retVal;
} // { _WEStdPixelToChar }

pascal short _WEStdCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE)
{
#pragma unused(hPos, hWE)
	return CharToPixel(pText, textLength, slop, offset, direction,
			styleRunPosition, kOneToOneScaling, kOneToOneScaling);
} // { _WEStdCharToPixel }

pascal StyledLineBreakCode _WEStdLineBreak(Ptr pText, long textLength,
				long textStart, long textEnd, Fixed *textWidth,
				long *textOffset, WEHandle hWE)
{
#pragma unused(hWE)
	return StyledLineBreak(pText, textLength, textStart, textEnd, 0, textWidth,
				textOffset);
} // { _WEStdLineBreak }

pascal void _WEStdWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE)
{
#pragma unused(hWE)
	FindWordBreaks(pText, textLength, offset, (Boolean)edge, nil, breakOffsets,
		script);
} // { _WEStdWordBreak }

pascal short _WEStdCharByte(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
#pragma unused(hWE)
	return CharacterByteType(pText, textOffset, script);
} // { _WEStdCharByte }

pascal short _WEStdCharType(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
#pragma unused(hWE)
	return CharacterType(pText, textOffset, script);
} // { _WEStdCharType }

pascal short _WEScriptToFont(ScriptCode script)
{
	// { given an explicit script code, return the first font ID in the corresponding range }
	// { for an explanation of the formula given below, see IM: Text, page B-8 }

	if (script == smRoman)
		return 2;
	else if ((script > smRoman) && (script <= smUninterp))
		return (0x3E00 + 0x200 * script);
	else
		return systemFont;	//	{ unknown script code (?) }
} // { _WEScriptToFont }

#if !SystemSevenFiveOrLater

pascal void _WEOldWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE)
{
	GrafPtr savePort, tempPort;
	short saveFont;

	// { the old (now obsolete) FindWord routine gets an implicit script parameter through }
	// { the current graphics port txFont field, so first of all we must have a valid port }
	GetPort(&savePort);
	tempPort = (*hWE)->port;
	SetPort(tempPort);

	// { then set the txFont field to a font number in the specified script range }
	saveFont = tempPort->txFont;
	TextFont(_WEScriptToFont(script));

	// { call _FindWord }
	FindWord(pText, textLength, offset, (Boolean)edge, nil, breakOffsets);

	// { restore font and port }
	TextFont(saveFont);
	SetPort(savePort);

} // { _WEOldWordBreak }

pascal short _WEOldCharByte(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
	GrafPtr savePort, tempPort;
	short saveFont;
	short retVal;

	// { the old (now obsolete) CharByte routine gets an implicit script parameter through }
	// { the current graphics port txFont field, so first of all we must have a valid port }
	GetPort(&savePort);
	tempPort = (*hWE)->port;
	SetPort(tempPort);

	// { then set the txFont field to a font number in the specified script range }
	saveFont = tempPort->txFont;
	TextFont(_WEScriptToFont(script));

	// { call _CharByte }
	retVal = CharByte(pText, textOffset);

	// { restore font and port }
	TextFont(saveFont);
	SetPort(savePort);

	return retVal;
} // { _WEOldCharByte }

pascal short _WEOldCharType(Ptr pText, short textOffset, ScriptCode script,
				WEHandle hWE)
{
	GrafPtr savePort, tempPort;
	short saveFont;
	short retVal;

	// { the old (now obsolete) CharType routine gets an implicit script parameter through }
	// { the current graphics port txFont field, so first of all we must have a valid port }
	GetPort(&savePort);
	tempPort = (*hWE)->port;
	SetPort(tempPort);

	// { then set the txFont field to a font number in the specified script range }
	saveFont = tempPort->txFont;
	TextFont(_WEScriptToFont(script));

	// { call _CharType }
	retVal = CharType(pText, textOffset);

	// { restore font and port }
	TextFont(saveFont);
	SetPort(savePort);

	return retVal;

} // { _WEOldCharType }

#endif

pascal OSErr _WERegisterWithTSM(WEHandle hWE)
{
	// { the WE record must be already locked }
	WEPtr pWE;
	OSType typeList[1];
	OSErr err;

	pWE = *hWE;

	// { do nothing if the Text Services Manager isn't available }
	if (BTST(pWE->flags, weFHasTextServices))
	{
		typeList[0] = kTextService;
		err = NewTSMDocument(1, typeList, &pWE->tsmReference, (long)hWE);
		if (err != noErr) 
		{
			// { we don't consider it an error if our client application isn't TSM-aware }
			if (err != tsmNeverRegisteredErr) 
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
}

pascal void _WESetStandardHooks(WEHandle hWE)
{
	WEPtr pWE;

	// { the first time we're called, create routine descriptors }
	if (_weStdDrawTextProc == nil)
	{
		_weStdDrawTextProc = NewWEDrawTextProc(_WEStdDrawText);
		_weStdPixelToCharProc = NewWEPixelToCharProc(_WEStdPixelToChar);
		_weStdCharToPixelProc = NewWECharToPixelProc(_WEStdCharToPixel);
		_weStdLineBreakProc = NewWELineBreakProc(_WEStdLineBreak);

#if !SystemSevenFiveOrLater

		if (GetScriptManagerVariable(smVersion) < 0x0710)
		{
			// { pre-7.1 version of the Script Manager: must use old hooks }
			_weStdWordBreakProc = NewWEWordBreakProc(_WEOldWordBreak);
			_weStdCharByteProc = NewWECharByteProc(_WEOldCharByte);
			_weStdCharTypeProc = NewWECharTypeProc(_WEOldCharType);
		}
		else
#endif
		{
			// { Script Manager version 7.1 or newer }
			_weStdWordBreakProc = NewWEWordBreakProc(_WEStdWordBreak);
			_weStdCharByteProc = NewWECharByteProc(_WEStdCharByte);
			_weStdCharTypeProc = NewWECharTypeProc(_WEStdCharType);
		}
	} // { if called for the first time }

	// { replace null hook fields with the addresses of the standard hooks }

	pWE = *hWE;

	// { replace null hook fields with the addresses of the standard hooks }

	if (pWE->drawTextHook == nil)
		pWE->drawTextHook = (UniversalProcPtr)_weStdDrawTextProc;

	if (pWE->pixelToCharHook== nil)
		pWE->pixelToCharHook = (UniversalProcPtr)_weStdPixelToCharProc;

	if (pWE->charToPixelHook== nil)
		pWE->charToPixelHook = (UniversalProcPtr)_weStdCharToPixelProc;

	if (pWE->lineBreakHook== nil)
		pWE->lineBreakHook = (UniversalProcPtr)_weStdLineBreakProc;

	if (pWE->wordBreakHook== nil)
		pWE->wordBreakHook = (UniversalProcPtr)_weStdWordBreakProc;
	
	if (pWE->charByteHook== nil)
		pWE->charByteHook = (UniversalProcPtr)_weStdCharByteProc;

	if (pWE->charTypeHook== nil)
		pWE->charTypeHook = (UniversalProcPtr)_weStdCharTypeProc;

} // { _WESetStandardHooks }

pascal OSErr WENew(LongRect *destRect, LongRect *viewRect, short flags, WEHandle *hWE)
{
	WEPtr pWE;
	short allocFlags;
	long weFlags;
	long response;
	Rect r;
	OSErr err;
	
	pWE = nil;
	weFlags = flags;
	allocFlags = kAllocClear;

	// { allocate the WE record }
	err = _WEAllocate(sizeof(WERec), allocFlags, (Handle *)hWE);
	if (err != noErr) 
	{
		goto cleanup;
	}
	
	// { lock it down }
	HLock((Handle)*hWE);
	pWE = **hWE;

	// { get active port }
	GetPort(&pWE->port);

	// { determine whether temporary memory should be used for data structures }
	if (BTST(weFlags, weFUseTempMem))
	{ 
		allocFlags = allocFlags + kAllocTemp;
	}
	
	// { allocate the text handle (initially empty) }
	err = _WEAllocate(0, allocFlags, (Handle *)&pWE->hText);
	if (err != noErr) 
	{
		goto cleanup;
	}

	// { allocate the line array }
	err = _WEAllocate(2 * sizeof(LineRec), allocFlags, (Handle *)&pWE->hLines);
	if (err != noErr) 
	{
		goto cleanup;
	}

	// { allocate the style table }
	err = _WEAllocate(sizeof(StyleTableElement), allocFlags, (Handle *)&pWE->hStyles);
	if (err != noErr) 
	{
		goto cleanup;
	}
	
	// { allocate the run array }
	err = _WEAllocate(2 * sizeof(RunArrayElement), allocFlags, (Handle *)&pWE->hRuns);
	if (err != noErr) 
	{
		goto cleanup;
	}
	
	// { check for the presence of various system software features }
	// { determine whether Color QuickDraw is available }
	if (Gestalt(gestaltQuickdrawVersion, &response) == noErr) 
	{
		if (response >= gestalt8BitQD)
		{ 
			BSET(weFlags, weFHasColorQD);
		}
	}
	
	// { determine whether the Text Services manager is available }
	if (Gestalt(gestaltTSMgrVersion, &response) == noErr)
	{ 
		BSET(weFlags, weFHasTextServices);
	}
	
	// { determine if there are any non-Roman scripts enabled }
	if (GetScriptManagerVariable(smEnabled) > 1) 
	{
		BSET(weFlags, weFNonRoman);
	}
	
	// { determine whether a double-byte script is installed }
#ifndef WEFORCE_2BYTE
	if (GetScriptManagerVariable(smDoubleByte) != 0) 
#endif
	{
		BSET(weFlags, weFDoubleByte);
	}
	
	// { determine whether the Drag Manager is available }
	if (Gestalt(gestaltDragMgrAttr, &response) == noErr)
	{
		if (BTST(response, gestaltDragMgrPresent))
				BSET(weFlags, weFHasDragManager);
	}

	// { initialize miscellaneous fields of the WE record }
	pWE->nLines = 1;
	pWE->nStyles = 1;
	pWE->nRuns = 1;
	pWE->viewRect = *viewRect;
	pWE->destRect = *destRect;
	pWE->flags = weFlags;
	pWE->tsmAreaStart = kInvalidOffset;
	pWE->tsmAreaEnd = kInvalidOffset;
	pWE->dragCaretOffset = kInvalidOffset;

	// { initialize hook fields with the addresses of the standard hooks }
	_WESetStandardHooks(*hWE);

	// { create a region to hold the view rectangle }
	pWE->viewRgn = NewRgn();
	WELongRectToRect(viewRect, &r);
	RectRgn(pWE->viewRgn, &r);

	// { initialize the style run array }
	(*pWE->hRuns)[1].runStart = 1;
	(*pWE->hRuns)[1].styleIndex = -1;

	// { initialize the style table }
	(*pWE->hStyles)[0].refCount = 1;

	// { copy text attributes from the active graphics port }
	(*pWE->hStyles)[0].info.runStyle.tsFont = pWE->port->txFont;
	(*pWE->hStyles)[0].info.runStyle.tsSize = pWE->port->txSize;
	(*pWE->hStyles)[0].info.runStyle.tsFace = ((GrafPtr1)pWE->port)->txFace;
	if (BTST(weFlags, weFHasColorQD))
	{ 
		GetForeColor(&(*pWE->hStyles)[0].info.runStyle.tsColor);
	}
	_WEFillFontInfo(pWE->port, &(*pWE->hStyles)[0].info);

	// { initialize the line array }
	err = WECalText(*hWE);
	if (err != noErr) 
	{
		goto cleanup;
	}
	
	// { register with the Text Services Manager }
	err = _WERegisterWithTSM(*hWE);
	if (err != noErr) 
	{
		goto cleanup;
	}

	// { unlock the WE record }
	HUnlock((Handle)*hWE);

	// { skip clean-up section }
	return noErr;

cleanup:
	// { clean up }
	if (pWE != nil) 
	{
		_WEForgetHandle((Handle *)&pWE->hText);
		_WEForgetHandle((Handle *)&pWE->hLines);
		_WEForgetHandle((Handle *)&pWE->hStyles);
		_WEForgetHandle((Handle *)&pWE->hRuns);
		if (pWE->viewRgn != nil) 
		{
			DisposeRgn(pWE->viewRgn);
		}
	}
	_WEForgetHandle((Handle *)hWE);

	return err;
}

pascal void WEDispose(WEHandle hWE)
{
	WEPtr pWE;
	StyleTablePtr pTable;
	long index;

	// { sanity check: make sure WE isn't NIL }
	if (hWE == nil)
	{
		return;
	} 

	// { lock the WE record }
	HLock((Handle)hWE);
	pWE = *hWE;

	// { clear the Undo buffer }
	WEClearUndo(hWE);

	// { unregister with the Text Services Manager }
	if (pWE->tsmReference != nil)
	{
		DeleteTSMDocument(pWE->tsmReference);			
		pWE->tsmReference = nil;
	}

	// { dispose of the offscreen graphics world }
	if (pWE->offscreenPort != nil) 
	{
		DisposeGWorld((GWorldPtr)pWE->offscreenPort);
		pWE->offscreenPort = nil;
	}
	
	if (pWE->hStyles != nil)
	{
		// { lock the style table }
		HLock((Handle)pWE->hStyles);
		pTable = *pWE->hStyles;

		// { walk the style table, disposing of all embedded objects referenced there }
		index = 0;
		while (index < pWE->nStyles)
		{
			if (pTable[index].refCount > 0)
			{
				if (_WEFreeObject((WEObjectDescHandle)(pTable[index].info.runStyle.tsObject))
					 != noErr)
				{
									;		//{ don't known what to do with errors }
				}
			}
			index = index + 1;
		}
	}
	
	// { dispose of auxiliary data structures }
	_WEForgetHandle((Handle *)&pWE->hText);
	_WEForgetHandle((Handle *)&pWE->hLines);
	_WEForgetHandle((Handle *)&pWE->hStyles);
	_WEForgetHandle((Handle *)&pWE->hRuns);
	_WEForgetHandle((Handle *)&pWE->hObjectHandlerTable);
	DisposeRgn(pWE->viewRgn);

	// { dispose of the WE record }
	DisposeHandle((Handle)hWE);

}

pascal short WEFeatureFlag(short feature, short action, WEHandle hWE)
{
	WEPtr pWE;
	short flag;
	
	pWE = *hWE;

	// { get current status of the specified flag }
	flag = BTST(pWE->flags, feature) ? weBitSet : weBitClear;

	// { if action is weBitToggle, invert flag }
	if (action == weBitToggle)
		action = 1 - flag;

	// { reset flag according to action }
	if (action == weBitClear)
	{ 
		BCLR(pWE->flags, feature);
	}
	else if (action == weBitSet)
	{ 
		BSET(pWE->flags, feature);
	}
	
	// { return old status }
	return flag;
}

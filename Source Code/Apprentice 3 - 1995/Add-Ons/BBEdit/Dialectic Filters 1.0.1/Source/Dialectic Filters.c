/*  
 * Dialectic Filters - A freeware BBEdit extension.
 *
 * Copyright (C) 1994 by Leo Breebaart, Kronto Software.
 * Based on original code copyright by Mark Pilgrim, Merrimac Software Group. 
 * See the enclosed README File for more information.
 *
 * This extension is free software. You may redistribute it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. See the enclosed GNU Licence file for more information.
 *
 *
 * CODE NOTE:
 *
 * This code makes use of gotos to jump to the 'clean up' section at the end
 * of a routine, and thus avoid deeply-nested if-statements. This use of the goto
 * statement is a bit of an experiment. I wanted to see if the code would become
 * more legible this way. I think it does. Let me know what you think.
 *
 */

	// THINK C
#include <SetupA4.h>
#include <Balloons.h>
#include <GestaltEqu.h>

	// BBEdit
#include "ExternalInterface.h"
#include "DialogUtilities.h"

	// Dialectic Filters
#include "Dialectic Filters.h"

	// Global Variables
ExternalCallbackBlock 	*BBEdit;
Boolean 				gAbort;
TCharacterStream 		gInput, gOutput;
TPreferences 			gPrefs;

	// Local Functions
pascal void main (ExternalCallbackBlock *callbacks, WindowPeek w);
static short MyDialog (void);
static short ApplyTheFilter (WindowPeek w, long selStart, long selEnd, long firstChar);
static void CopyPascalString (StringPtr to, StringPtr from);
static void GetVersionNumber (short resourceID, StringPtr versionNumber);
static void AboutBox (void);
static void SetDialogFontAndSize (DialogPtr d, short fontNum, short fontSize);
static void MyStopAlert (short msgID);


pascal void 
main (ExternalCallbackBlock *callbacks, WindowPeek w)
{
	short prefsSize, result;
	long selStart, selEnd, firstChar;

	RememberA0();
	SetUpA4();

	BBEdit = callbacks;
			
	if (BBEdit->version < 2)
	{
		MyStopAlert(kErrBBEditVersionNotHighEnough);
		goto finish;
	}
		
	if (!w && w->windowKind != userKind)
	{
		MyStopAlert(kErrWrongWindowKind);
		goto finish;
	}
		
	BBEdit->GetSelection(&selStart, &selEnd, &firstChar);
	if (selStart == selEnd)
	{
		MyStopAlert(kErrNeedSelection);
		goto finish;
	}
	
	BBEdit->GetPreference(kPrefsResourceType, sizeof(gPrefs), &gPrefs, &prefsSize);
	if (prefsSize <= 0)
		gPrefs.dialect = kFirstDialect;
		
	if (MyDialog() == kApply)
	{
		BBEdit->SetPreference(kPrefsResourceType, sizeof(gPrefs), &gPrefs, &prefsSize);

		if ((result = ApplyTheFilter(w, selStart, selEnd, firstChar)) != kSuccess)
		{
			MyStopAlert(result);
			goto finish;
		}
	}

finish:
	RestoreA4();
}


static short
ApplyTheFilter (WindowPeek w, long selStart, long selEnd, long firstChar)
{
	short result = kSuccess;
	void (*processOneCharacter)();

		// Initialize input buffer.
	if ((gInput.theHandle = BBEdit->GetWindowContents((WindowPtr) w)) == nil)
	{
		return kErrWindowContentsNil;
	}
	HLockHi(gInput.theHandle);		
	gInput.base   = ((char *) *gInput.theHandle) + selStart;
	gInput.length = selEnd - selStart;
	gInput.offset = 0;
			
		// Initialize output buffer.
	if ((gOutput.theHandle = BBEdit->Allocate(2*gInput.length, FALSE)) == nil)
	{
		result = kErrNotEnoughMemory;
		goto cleanup;
	}
	HLockHi(gOutput.theHandle);		
	gOutput.base   = (char *) *gOutput.theHandle;
	gOutput.length = 2*gInput.length;
	gOutput.offset = 0;
 
 		// Determine conversion function.
	switch (gPrefs.dialect)
	{
		case kChef:  processOneCharacter = ConvertChef;  break;
		case kFudd:  processOneCharacter = ConvertFudd;  break;
		case kOlde:  processOneCharacter = ConvertOlde;  break;
		case kWAREZ: processOneCharacter = ConvertWAREZ; break;
		case kPig:   processOneCharacter = ConvertPig;   break;
		case kRot13: processOneCharacter = ConvertRot13; break;
		case kUbby:  processOneCharacter = ConvertUbby;  break;
		case kMorse: processOneCharacter = ConvertMorse; break;
		case kOp:    processOneCharacter = ConvertOp;    break;
		default:     result = kErrInsaneDialectValue; 
				     goto cleanup;
				     break;
	}

		// Process the input stream.
	gAbort = FALSE;
	while (gInput.offset < gInput.length && !gAbort)
	{
		processOneCharacter();
	}
	
		// Did we bail out during the processing?
	if (gAbort)
	{
		result = kErrNotEnoughMemory;
		goto cleanup;
	}
	else
	{
		SetHandleSize(gOutput.theHandle, gOutput.offset);
		if (MemError() != 0)
		{	
			result = kErrNotEnoughMemory;
			goto cleanup;
		}
			// Undo only available in BBEdit 3.0 (== callbacks version 4) or later
		if (BBEdit->version >= 4)
		{
			if (BBEdit->PresetUndo())
			{
				BBEdit->Paste(gOutput.theHandle);
				BBEdit->SetSelection(selStart, selStart+gOutput.offset, firstChar);
				BBEdit->SetUndo();
			}
			else
			{	
				result = kErrNotEnoughMemory;
				goto cleanup;
			}
		}	
		else
		{
			BBEdit->Paste(gOutput.theHandle);
			BBEdit->SetSelection(selStart, selStart+gOutput.offset, firstChar);
		}
	}
		
cleanup:	
		HUnlock(gInput.theHandle);
		if (gOutput.theHandle != nil)
			DisposeHandle(gOutput.theHandle);
		return result;
}


static void 
CopyPascalString (StringPtr to, StringPtr from)
{
	BlockMove(from, to, *from+1);
}


static void
GetVersionNumber (short resourceID, StringPtr versionNumber)
{
	VersRecHndl versionInfo;

	if ((versionInfo = (VersRecHndl) GetResource('vers', resourceID)) == nil)
		CopyPascalString(versionNumber, "\p???");
	else
	{	
		CopyPascalString(versionNumber, (**versionInfo).shortVersion);
		ReleaseResource((Handle)versionInfo);
	}
}


static short
MyDialog (void)
{
	DialogPtr d;
	GrafPtr savePort;
	long answer;
	short item;
	Boolean saveBalloonStatus;
	Boolean hasHelpMgr = false;
	TDialogItemKind i;	
	
	
	GetPort(&savePort);
		
	if (Gestalt(gestaltHelpMgrAttr, &answer) == noErr) 
	{
		hasHelpMgr = true;
		saveBalloonStatus = HMGetBalloons();
	}

	d = BBEdit->CenterDialog(kOptionsDlg);
	SetPort(d);
		
	SetupUserItem (d, kLine, BBEdit->FrameDialogItem);

	do 
	{		
		for (i = kFirstDialect; i <= kLastDialect; i++)
			SetDlgCtl(d, i, i == gPrefs.dialect);
		
		ModalDialog(BBEdit->StandardFilter, &item);
		
		if (item == kAbout)
			AboutBox();
			
		if (item == kBalloonHelpToggle)
		{
			if (hasHelpMgr)
				HMSetBalloons(!HMGetBalloons());
			else
				MyStopAlert(kErrBalloonHelpNotAvailable);
		}

		if ((item >= kFirstDialect) && (item <= kLastDialect))
			gPrefs.dialect = item;
	} 
	while ((item != kApply) && (item != kCancel));
	
	if (hasHelpMgr)
		HMSetBalloons(saveBalloonStatus);
		
	SetPort(savePort);			
	DisposDialog(d);
	
	return item;
}


static void 
AboutBox (void)
{
	DialogPtr d;
	GrafPtr savePort;
	short item;
	Str255 str1, str2, str3;
	Str255 programVersion;

	GetPort(&savePort);
	
	GetVersionNumber(2, programVersion);
	GetIndString(str1, kAboutStringsResID, 1);
	GetIndString(str2, kAboutStringsResID, 2);
	GetIndString(str3, kAboutStringsResID, 3);
	if ((d = BBEdit->CenterDialog(kAboutDlg)) == nil)
		return;
	SetPort(d);
	SetDialogFontAndSize(d, geneva, 9);
	SetupUserItem (d, kAboutLine, BBEdit->FrameDialogItem);
	ParamText(programVersion, str1, str2, str3);
	ModalDialog(BBEdit->StandardFilter, &item);
	DisposDialog(d);

	SetPort(savePort);
}


	// SetDialogFontAndSize is a slightly modified version of an
	// original alt.sources.mac code snippet by Leonard Rosenthol.
	
static void
SetDialogFontAndSize (DialogPtr d, short fontNum, short fontSize)
{
	FontInfo f;
	GrafPtr savePort;
	
	GetPort(&savePort);
	SetPort(d);
	
		// set up the port info
	TextFont(fontNum);
	TextSize(fontSize);

		// now deal with the static & edit text issues
	GetFontInfo(&f);
	(*((DialogPeek)d)->textH)->txFont = fontNum;
	(*((DialogPeek)d)->textH)->txSize = fontSize;
	(*((DialogPeek)d)->textH)->lineHeight = f.ascent + f.descent + f.leading;
	(*((DialogPeek)d)->textH)->fontAscent = f.ascent;
	
	SetPort(savePort);
}


static void 
MyStopAlert (short msgID)
{
	DialogPtr d;
	GrafPtr savePort;
	short item;
	Str255 msg;
			
	GetIndString(msg, kErrorStringsResID, msgID);
	GetPort(&savePort);
	
	SysBeep(2);	
	if ((d = BBEdit->CenterDialog(kStopAlertDlg)) == nil)
		return;
		
	SetPort(d);
	ParamText(msg, nil, nil, nil);
	ModalDialog(BBEdit->StandardFilter, &item);
	DisposeDialog(d);
	SetPort(savePort);
}

// This is THE END.
// If you've learned anything from this code, or found errors in it, or
// have questions about it, or whatever: feel free to drop me a note.
// My e-mail address is: leo@cp.tn.tudelft.nl


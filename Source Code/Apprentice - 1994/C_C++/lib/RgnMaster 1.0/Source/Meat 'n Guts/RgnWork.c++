/*

		RgnWork.c++
		by Hiep Dam, 3G Software.
		March 1994.
		Last Update April 1994
		Contact: America Online: Starlabs
		         Delphi        : StarLabs
		         Internet      : starlabs@aol.com
		                      or starlabs@delphi.com

		Handles the interface between the application and the rgn creation/upkeep
		routines (i.e. bridges the application-specific stuff with the
		more generic stuff).
*/

#include <QDOffscreen.h>
#include <Aliases.h>
#include "QDUtils.h"
#include "Regions.h"
#include "RgnMaster.h"
#include "KeyUtils.h"
#include "DialogUtils.h"
#include "RgnWork.h"

static RgnHandle gCurrentRgn = nil;

RgnHandle GetCurrentRegion() {
	return(gCurrentRgn);
} // END GetCurrentRegion

void ConvertToRegion(Boolean offsetTheRegion) {
	PicHandle thePic;
	RgnHandle theRgn, saveRgn;

	// First, load in picture, if available...
	thePic = PictureFromScrap();
	if (thePic == nil) {
		DoError("\pNo picture found in clipboard to convert.");
		return;
	}

	saveRgn = gCurrentRgn;

	theRgn = ConvertPictureToRegion(thePic, offsetTheRegion);
	if (theRgn == nil) {
		DoError("\pUnable to create region!!");
		gCurrentRgn = saveRgn;	// Restore saved region
		return;
	}

	DisposeRgn(saveRgn);		// Get rid of old region
	gCurrentRgn = theRgn;		// Set the new region
} // END ConvertToRegion

void ConvertOnly() {
	if (OptionKeyDown())
		ConvertToRegion(false);
	else
		ConvertToRegion(true);
	if (gPreviewAlways)
		PreviewRgn(GetCurrentRegion());
} // END ConvertOnly

void ConvertAndReplace() {
	if (OptionKeyDown())
		ConvertToRegion(false);
	else
		ConvertToRegion(true);
	if (RegionToScrap(gCurrentRgn) != noErr) {
		DoError("\pUnable to place region onto clipboard.");
		return;
	}
	if (gPreviewAlways)
		PreviewRgn(GetCurrentRegion());
} // END ConvertAndReplace

void ViewRegion() {
	if (GetCurrentRegion() == nil)
		DoError("\pThere is no region to preview/view. Copy a picture to clipboard first, convert it, and then view.");
	else
		PreviewRgn(GetCurrentRegion());
} // END ViewRegion

void CopyRegionToClipboard() {
	if (GetCurrentRegion() == nil)
		DoError("\pThere is no region to copy. Copy a picture to clipboard first, convert it, and then copy the region to the clipboard.");
	else {
		short myErr = RegionToScrap(GetCurrentRegion());
		if (myErr != noErr)
			DoError("\pUnable to put region onto the clipboard!");
	}
} // END CopyRegionToClipboard

enum {
	okItem = 1,
	cancelItem,
	rgnTypeItem,
	rgnIDItem,
	rgnNameItem
};

void SaveRegion() {
	GrafPtr savePort;
	DialogPtr rgnDialog;
	RgnHandle theRgn;
	short rgnID;
	Str255 rgnName;
	ResType rgnType;
	short itemHit, i;
	Boolean done = false;
	short saveFileRef = CurResFile();

	UseResFile(gAppFileRef);

	theRgn = GetCurrentRegion();
	if (theRgn == nil) {
		DoError("\pThere is no region to save. Copy a picture to clipboard first and then convert it.");
		return;
	}

	GetPort(&savePort);

	rgnDialog = GetNewDialog(kSaveDialogID, nil, (WindowPtr)-1);
	SetPort(rgnDialog);

	HLockHi((Handle)theRgn);
	rgnType = GetRegionType();
	do {
		rgnID = Unique1ID(rgnType);
	} while (rgnID < 128);

	NumToString(rgnID, rgnName);
	SetDItemText(rgnDialog, rgnIDItem, rgnName);
	rgnName[0] = 4;
	for (i = 1; i <= 4; i++)
		rgnName[i] = ((unsigned char*)&rgnType)[i-1];
	SetDItemText(rgnDialog, rgnTypeItem, rgnName);

	ShowWindow(rgnDialog);
	OutlineDefaultButton(rgnDialog, ok);
	SelIText(rgnDialog, rgnNameItem, 0, 32767);
	
	do {
		ModalDialog(nil, &itemHit);
		if (itemHit == cancelItem)
			done = true;
		else if (itemHit == okItem) {
			GetIText(GetDItemHdl(rgnDialog, rgnTypeItem), rgnName);
			if (rgnName[0] != 4) {
				SysBeep(0);
				SelIText(rgnDialog, rgnTypeItem, 0, 32767);
			}
			else {
				for (i = 1; i <= 4; i++)
					((unsigned char*)&rgnType)[i-1] = rgnName[i];
				GetIText(GetDItemHdl(rgnDialog, rgnNameItem), rgnName);
				UseResFile(saveFileRef);
				AddResource((Handle)theRgn, rgnType, rgnID, rgnName);
				WriteResource((Handle)theRgn);
				UseResFile(gAppFileRef);
				done = true;
			}
		}
	} while (!done);

	HUnlock((Handle)theRgn);
	DisposeDialog(rgnDialog);

	SetPort(savePort);
	UseResFile(saveFileRef);
} // END SaveRegion

void SaveRegionToApplication() {
	if (GetCurrentRegion() == nil)
		DoError("\pThere is no region to save. Copy a picture to clipboard first and convert it.");
	else {
		SaveRegion();
	}
} // END SaveRegionToApplication

void SaveRegionToFile() {
	if (GetCurrentRegion() == nil)
		DoError("\pThere is no region to save. Copy a picture to clipboard first and convert it.");
	else {
		SFTypeList typeList;
		StandardFileReply reply;
		OSErr myErr;
		Boolean targetIsFolder, wasAliased;
		short fRefNum;

		StandardGetFile(nil, -1, typeList, &reply);

		if (reply.sfGood) {
			myErr = ResolveAliasFile(&reply.sfFile, true, &targetIsFolder, &wasAliased);
			if (targetIsFolder || myErr != noErr) {
				SysBeep(0);
				DoError("\pUnable to find specified file. Sorry!");
				return;
			}

			fRefNum = FSpOpenResFile(&reply.sfFile, fsCurPerm);
			if (fRefNum == -1) {
				SysBeep(0);
				DoError("\pUnable to open specified file. Sorry!");
				return;
			}

			UseResFile(fRefNum);

			SaveRegion();

			UpdateResFile(fRefNum);
			CloseResFile(fRefNum);
			UseResFile(gAppFileRef);
		}
	}
} // END SaveRegionToFile
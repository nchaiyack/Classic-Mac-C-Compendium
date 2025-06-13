#include "Global.h"
#include "OtherMenu.h"
#include "CEToolbox.h"			// CETurnOnOff
#include "List.h"				// DynamicArrayHdl
#include "MenubarCDEF.h"
#include "MenuSetup.h"			// CreateMenuText
#include "MiniFinder.h"			// gHotRecord
#include "Munge Dialog.h"		// gMyItemBase
#include "Pack3.h"				// gSelector
#include "Remember.h"			// SetSortProcedure, gDirectoryList, gFileList
#include "SpinningCursor.h"		// For spinning cursor stuff
#include "Utilities.h"			// CreateNewDialog, GetItemHandle

#include <Script.h>				// smSystemScript


// Stuff for FindÉ and Find Again

Str255				gStringToFind = "\p";		// ¥¥¥ Allocate dynamically

#define kFindBufferSize	(16*1024)
#define kMaxMatches		1

HParamBlockRec*		pbFind;
Handle				pFindBuffer;
FSSpec*				pMatchBuffer;
CInfoPBRec*			pSpec1;
CInfoPBRec*			pSpec2;
Boolean				pFoundOne;
short				pStartingVolume;

ListHandle			pMyList;

short				pMaxFileObjectsToRemember;
long				pNewHotKey;

short				DoNewFolder(void);
short				DoFind(void);
short				DoFindAgain(void);
void				DoSetPermanent(ControlHandle menuControl, DynamicArrayHdl list);
pascal Boolean		SetPermFilter(DialogPtr dlg, EventRecord *event, short *item);
void				FillList(ListHandle list, DynamicArrayHdl array);
void				DoConfig(ControlHandle menuControl);
pascal Boolean		ConfigFilter(DialogPtr dlg, EventRecord *event, short *item);
pascal void			ConfigUserProc(DialogPtr dlg, short item);
void				HandleSpinControl(DialogPtr dlg);
void				DrawArrow(DialogPtr dlg, short arrowID);
void				DoAbout(void);
void				DoTrash(ControlHandle menuControl);

void				FlushMenuText(void);


//--------------------------------------------------------------------------------

short	HandleOtherMenu(DialogPtr dlg, short menuItem)
{
	ControlHandle	menuControl;
	short	result = sfHookNullEvent;

	menuControl = (ControlHandle) GetItemHandle(dlg, gMyItemBase + kMenubar);

	switch (menuItem) {
		case iNewFolder:
			result = DoNewFolder();
			break;
		case iFind:
			result = DoFind();
			break;
		case iFindAgain:
			result = DoFindAgain();
			break;
		case iPermDirectories:
			DoSetPermanent(menuControl, gDirectoryList);
			break;
		case iPermFiles:
			DoSetPermanent(menuControl, gFileList);
			break;
		case iConfigure:
			DoConfig(menuControl);
			break;
		case iAbout:
			DoAbout();
			break;
		case iTrash:
			DoTrash(menuControl);
			break;
	}
	
	return result;
}

//--------------------------------------------------------------------------------

short	DoNewFolder()
{
	GrafPtr		oldPort;
	short		oldFile;
	DialogPtr	dlg;
	short		item;
	Handle		textH;
	long		newDirID;
	Str255		folderName;
	short		result = sfHookNullEvent;

	OSErr		err;
	FSSpec		spec;

	if ((gSelector == standardPutFile) || (gSelector == customPutFile)) {
		result = sfItemNewFolderUser;
	} else {
		if (gRefNum != -1) {
			oldFile = UseMyResFile();
			GetPort(&oldPort);

			dlg = CreateNewDialog(kNewFolderDialog, kNewFolderOK, kNewFolderCancel);
			ShowWindow(dlg);
			ModalDialog(NIL, &item);

			textH = GetItemHandle(dlg, kNewFolderEditText);
			GetIText(textH, folderName);

			DisposeDialog(dlg);
			SetPort(oldPort);
			UseResFile(oldFile);

			if (item == kNewFolderOK) {
				(void) FSMakeFSSpec(-LMGetSFSaveDisk(), LMGetCurDirStore(), folderName, &spec);
				err = FSpDirCreate(&spec, smSystemScript, &newDirID);
				if (err == noErr) {
					spec.parID = newDirID;
					result = GoToDirectory(&spec);
				} else {
					ReportError(kCreatingDirectory, err);
				}
			}
		}
	}
	return result;
}


//--------------------------------------------------------------------------------

short	DoFind()
{
	GrafPtr		oldPort;
	short		oldFile;
	DialogPtr	dlg;
	short		item;
	Handle		textH;
	short		result = sfHookNullEvent;
	THz			currentZone;

	if (gRefNum != -1) {
		oldFile = UseMyResFile();
		GetPort(&oldPort);

		dlg = CreateNewDialog(kFindDialog, kFindOK, kFindCancel);

		textH = GetItemHandle(dlg, kFindEditText);
		SetIText(textH, gStringToFind);
		SelIText(dlg, kFindEditText, 0, 32767);

		ShowWindow(dlg);
		ModalDialog(NIL, &item);

		if (item == kFindOK) {
			GetIText(textH, gStringToFind);
		}

		DisposeDialog(dlg);
		SetPort(oldPort);
		UseResFile(oldFile);

		if ((item == kFindOK) && (StrLength(gStringToFind) > 0)) {
			
			currentZone = GetZone();
			SetZone(SystemZone());

			if (pSpec1 == NIL)
				pSpec1 = (CInfoPBRec*) NewPtr(sizeof(CInfoPBRec));

			if (pSpec2 == NIL)
				pSpec2 = (CInfoPBRec*) NewPtr(sizeof(CInfoPBRec));
			
			if (pMatchBuffer == NIL)
				pMatchBuffer = (FSSpec*) NewPtr(sizeof(FSSpec) * kMaxMatches);
			
			if (pbFind == NIL)
				pbFind = (HParamBlockRec*) NewPtr(sizeof(HParamBlockRec));

			SetZone(currentZone);

			pbFind->csParam.ioNamePtr = NIL;
			pbFind->csParam.ioVRefNum = -LMGetSFSaveDisk();
			pbFind->csParam.ioMatchPtr = pMatchBuffer;
			pbFind->csParam.ioReqMatchCount = kMaxMatches;
			pbFind->csParam.ioSearchBits = fsSBPartialName + fsSBFlAttrib;
			pbFind->csParam.ioSearchInfo1 = pSpec1;
			pbFind->csParam.ioSearchInfo2 = pSpec2;
			pbFind->csParam.ioSearchTime = 0;
			pbFind->csParam.ioCatPosition.initialize = 0;
			pbFind->csParam.ioOptBuffer = NIL;
			pbFind->csParam.ioOptBufSize = 0;

			pSpec1->hFileInfo.ioNamePtr = gStringToFind;
			pSpec1->hFileInfo.ioFlAttrib = 0;			// clear directory mask

			pSpec2->hFileInfo.ioNamePtr = NIL;
			pSpec2->hFileInfo.ioFlAttrib = ioDirMask;

			pFoundOne = FALSE;
			pStartingVolume = pbFind->csParam.ioVRefNum;

			InitCursor();
			result = DoFindAgain();
		}
	}
	
	return result;
}


//--------------------------------------------------------------------------------

short	DoFindAgain()
{
	Boolean			keepGoing;
	OSErr			err;
	OSType			itsTypeDummy;
	OSErr			itsErrDummy;
	GrafPtr			oldPort;
	short			oldFile;
	Str255			moreString;
	short			result = sfHookNullEvent;
	
	if (pFindBuffer == NIL) {
		pFindBuffer = NewHandleSys(kFindBufferSize);
	} else if (*pFindBuffer == NIL) {
		ReallocHandle(pFindBuffer, kFindBufferSize);
	}
	if ((pFindBuffer != NIL) && (*pFindBuffer != NIL)) {
		HLock(pFindBuffer);
		HPurge(pFindBuffer);
		pbFind->csParam.ioOptBuffer = *pFindBuffer;
		pbFind->csParam.ioOptBufSize = kFindBufferSize;
	} else {
		pbFind->csParam.ioOptBuffer = NIL;
		pbFind->csParam.ioOptBufSize = 0;
	}

	StartAsyncSpinning(10);
	do {
		keepGoing = TRUE;
		err = PBCatSearchSync(&pbFind->csParam);
		if (pbFind->csParam.ioActMatchCount > 0) {
			if (CanOpenFile(&pMatchBuffer[0], &itsTypeDummy, &itsErrDummy)) {
				result = GoToFile(&pMatchBuffer[0]);
				pFoundOne = TRUE;
				StopAsyncSpinning();
				keepGoing = FALSE;
			}
		} else {
			if (err == eofErr) {
					// ¥¥¥ We could get into an infinite loop here if NONE
					// of the mounted volumes has PBCatSearch.
				do {
					pbFind->csParam.ioVRefNum = GetNextVolume(pbFind->csParam.ioVRefNum);
				} while (!VolHasCatSearch(pbFind->csParam.ioVRefNum));
				if (pbFind->csParam.ioVRefNum == pStartingVolume) {
					StopAsyncSpinning();
					oldFile = UseMyResFile();
					GetPort(&oldPort);
					if (pFoundOne)
						GetIndString(moreString, kStrings, kWordMore);
					else
						moreString[0] = 0;
					ParamText(moreString, gStringToFind, NIL, NIL);
					(void) NoteAlert(kNoFilesFoundAlert, NIL);
					SetPort(oldPort);
					UseResFile(oldFile);
					pbFind->csParam.ioCatPosition.initialize = 0;
					keepGoing = FALSE;
				} else {
					pbFind->csParam.ioCatPosition.initialize = 0;
				}
			} else if (err == catChangedErr) {
				pbFind->csParam.ioCatPosition.initialize = 0;
			} else if (err == noErr) {
			} else {
				StopAsyncSpinning();
				ReportError(kSearching, err);
				keepGoing = FALSE;
			}
		}
	} while (keepGoing);
	HUnlock(pFindBuffer);
	return result;
}


//--------------------------------------------------------------------------------

void	DoSetPermanent(ControlHandle menuControl, DynamicArrayHdl list)
{
	GrafPtr				oldPort;
	short				oldFile;
	DialogPtr			dlg;
	FontInfo			info;
	short				item;
	Point				cSize = {0, 0};
	Rect				dataBounds = {0, 0, 0, 1};
	Rect				bounds;
	Cell				theCell;
	short				oldSortType;
	FileObjectHdl		obj;

	if (gRefNum != -1) {
		oldFile = UseMyResFile();
		GetPort(&oldPort);
		dlg = CreateNewDialog(kPermanentDialog, kPermOK, kPermCancel);

		bounds = GetItemRect(dlg, kPermListUser);
	
		GetFontInfo(&info);
		cSize.h = bounds.right - bounds.left - 15;
		cSize.v = info.ascent + info.descent + info.leading;
		if (cSize.v < 16)
			cSize.v = 16;
	
		bounds.bottom = bounds.top + ((bounds.bottom - bounds.top) / cSize.v) * cSize.v;
		SetItemRect(dlg, kPermListUser, &bounds);

		bounds.right -= 15;						// To allow for the listÕs scroll bar.
		pMyList = LNew(	&bounds,				// View bounds
						&dataBounds,			// Cell range
						cSize,					// Cell size
						kCheckLDEF,				// Use my check LDEF
						dlg,					// Owning port
						FALSE,					// delay drawing list
						FALSE,					// no grow box
						FALSE,					// no horizontal scroll bar
						TRUE);					// has vertical scroll bar

		(**pMyList).selFlags = lNoExtend | lUseSense;
		(**pMyList).refCon = kCheckBoxOnIcon * 0x10000 + kCheckBoxOffIcon;

		oldSortType = gSortType;
		SetSortProcedure(kSortByName);
		FillList(pMyList, list);

		LDoDraw(TRUE, pMyList);

		ShowWindow(dlg);
		ModalDialog(SetPermFilter, &item);

		if (item == kPermOK) {
			*(long *) &theCell = 0;
			do {
				obj = (FileObjectHdl) (**list).records[theCell.v];
				if (obj != NIL)
					(**obj).permanent = LGetSelect(FALSE, &theCell, pMyList);
			} while (LNextCell(TRUE, TRUE, &theCell, pMyList));
			SetSortProcedure(oldSortType);
			RebuildMenus(menuControl);
		} else {
			SetSortProcedure(oldSortType);
		}

		LDispose(pMyList);
		DisposeDialog(dlg);
		SetPort(oldPort);
		UseResFile(oldFile);
	}
}


pascal Boolean SetPermFilter(DialogPtr dlg, EventRecord* event, short* item)
{
	GrafPtr				oldPort;
	Boolean				result;
	OSErr				err;
	ModalFilterProcPtr	standardProc;
	Point				localMouse;
	Rect				bounds;
	char				key;

	SetUpA4();
	GetPort(&oldPort);
	SetPort(dlg);

	result = FALSE;

	err = GetStdFilterProc(&standardProc);
	if (err == noErr)
		result = standardProc(dlg, event, item);

	if (!result) {
		switch (event->what) {
			case mouseDown:
				localMouse = event->where;
				GlobalToLocal(&localMouse);
				bounds = GetItemRect(dlg, kPermListUser);
				if (PtInRect(localMouse, &bounds)) {
					LClick(localMouse, shiftKey, pMyList);
				}
				break;
	
			case keyDown:
				key = event->message & charCodeMask;
				break;
	
			case updateEvt:
				if (event->message == (long) dlg) {
					bounds = GetItemRect(dlg, kPermListUser);
					InsetRect(&bounds, -1, -1);
					PenNormal();
					FrameRect(&bounds);
					LUpdate(dlg->visRgn, pMyList);
				}
				break;
		}
	}
	
	SetPort(oldPort);
	RestoreA4();
	return result;
}


void	FillList(ListHandle list, DynamicArrayHdl array)
{
	ArrayIndex		index;
	FileObjectHdl	entry;
	short			newRow;
	Cell			theCell;
	Str255			text;
	
	if (array != NIL) {
		for (index = 0; index < (**array).numRecords; index++) {
			entry = (FileObjectHdl) (**array).records[index];
			CreateMenuText(entry, text);
			newRow = LAddRow(1, 32767, list);
			SetPt(&theCell, 0, newRow);
			LSetCell(&text[1], StrLength(text), theCell, list);
			LSetSelect((**entry).permanent, theCell, list);
		}
	}
}


//--------------------------------------------------------------------------------

void	DoConfig(ControlHandle menuControl)
{
	GrafPtr				oldPort;
	short				oldFile;
	short				oldValue;
	DialogPtr			dlg;
	short				item;
	short				setRadio;
	short				temp = 0;
	short				oldAppendVolume;
	short				oldAppendParent;
	Rect				bounds;
	Boolean				dismissed;

	if (gRefNum != -1) {
		oldFile = UseMyResFile();
		GetPort(&oldPort);

		CETurnOnOff(-1, &oldValue);
		
		dlg = CreateNewDialog(kConfigureDialog, kConfigOK, kConfigCancel);
		HideDItem(dlg, kConfigTakenText);
		SetItemHandle(dlg, kConfigHotKeyUser, (Handle) ConfigUserProc);
		SetItemHandle(dlg, kConfigHowManyUser, (Handle) ConfigUserProc);

		setRadio = ((gSortType & kSortByMask) == kSortByName)
				? kConfigNameRadio
				: kConfigAgeRadio;
		SetRadioButton(dlg, setRadio, &temp);
		if ((gSortType & kSeparatePermObjects) != 0)
			SetDialogItemState(dlg, kConfigPermItemsAtTop, 1);
		
		pMaxFileObjectsToRemember = gMaxFileObjectsToRemember;

		if (gCERefNum == 0) {
			HideDItem(dlg, kConfigHotKeyText);
			HideDItem(dlg, kConfigHotKeyCheck);
			HideDItem(dlg, kConfigHotKeyUser);
		} else {
			HideDItem(dlg, kConfigNoCEText);
			if (gHotRecord.HBFlags & HBMHasHotKey) {
				SetDialogItemState(dlg, kConfigHotKeyCheck, 1);
			} else {
				SetDialogItemState(dlg, kConfigHotKeyCheck, 0);
			}
			pNewHotKey = gHotRecord.HBKeyStroke;
		}

		if (gAppendVolumeName)
			SetDialogItemState(dlg, kConfigIncludeVolCheck, 1);
		if (gAppendParentDirectoryName)
			SetDialogItemState(dlg, kConfigIncludeParCheck, 1);

		dismissed = FALSE;
		ShowWindow(dlg);
		do {
			ModalDialog(ConfigFilter, &item);

			switch (item) {
				case kConfigOK:
					gSortType = (setRadio == kConfigNameRadio) ? kSortByName : kSortByAge;
					if (GetDialogItemState(dlg, kConfigPermItemsAtTop) != 0)
						gSortType |= kSeparatePermObjects;
					gMaxFileObjectsToRemember = pMaxFileObjectsToRemember;
					CullOldest(gDirectoryList);
					CullOldest(gFileList);
					
					if (GetDialogItemState(dlg, kConfigHotKeyCheck) != 0) {
						gHotRecord.HBFlags |= HBMHasHotKey;
					} else {
						gHotRecord.HBFlags &= ~HBMHasHotKey;
					}
					gHotRecord.HBKeyStroke = pNewHotKey;
					
					oldAppendVolume = gAppendVolumeName;
					oldAppendParent = gAppendParentDirectoryName;

					gAppendVolumeName = GetDialogItemState(dlg, kConfigIncludeVolCheck) != 0;
					gAppendParentDirectoryName = GetDialogItemState(dlg, kConfigIncludeParCheck) != 0;

					if (oldAppendVolume != gAppendVolumeName ||
					  oldAppendParent != gAppendParentDirectoryName)
						FlushMenuText();

					SetSortProcedure(gSortType);
					RebuildMenus(menuControl);

				case kConfigCancel:
					dismissed = TRUE;
					break;

				case kConfigNameRadio:
				case kConfigAgeRadio:
					SetRadioButton(dlg, item, &setRadio);
					break;
				
				case kConfigArrowPICT:
					HandleSpinControl(dlg);
					break;

				case kConfigHotKeyCheck:
					ToggleCheckBox(dlg, kConfigHotKeyCheck);
					bounds = GetItemRect(dlg, kConfigHotKeyUser);
					InvalRect(&bounds);
					break;

				case kConfigPermItemsAtTop:
				case kConfigIncludeVolCheck:
				case kConfigIncludeParCheck:
					ToggleCheckBox(dlg, item);
					break;
			}
		} while (!dismissed);

		CETurnOnOff(oldValue, &oldValue);

		DisposeDialog(dlg);
		SetPort(oldPort);
		UseResFile(oldFile);
	}
}


pascal Boolean	ConfigFilter(DialogPtr dlg, EventRecord* event, short* item)
{
	GrafPtr				oldPort;
	OSErr				err;
	ModalFilterProcPtr	standardProc;
	Boolean				result;
	long				newHotKey;
	HBQPtr				otherGuy;
	Rect				bounds;

	SetUpA4();
	GetPort(&oldPort);
	SetPort(dlg);

	result = FALSE;

	err = GetStdFilterProc(&standardProc);
	if (err == noErr)
		result = standardProc(dlg, event, item);

	if (!result	&& (event->what == keyDown)
				&& (gCERefNum != 0)
				&& (GetDialogItemState(dlg, kConfigHotKeyCheck) != 0)) {
//		newHotKey = (event->message << 16) | (unsigned short) event->modifiers;
		newHotKey = MergeShorts(event->message, event->modifiers);
		err = CECheckKey(newHotKey, &otherGuy);
		if (err == noErr) {
			if (otherGuy != &gHotRecord && otherGuy != NIL) {
				ParamText(*(Str255*) otherGuy->HBName, NIL, NIL, NIL);
				ShowDItem(dlg, kConfigTakenText);
			}
		} else {
			pNewHotKey = newHotKey;
			bounds = GetItemRect(dlg, kConfigHotKeyUser);
			InsetRect(&bounds, 2, 2);
			InvalRect(&bounds);
			HideDItem(dlg, kConfigTakenText);
		}
	}

	SetPort(oldPort);
	RestoreA4();
	
	return result;
}

pascal void ConfigUserProc(DialogPtr dlg, short item)
{
	PicHandle			pic;
	Rect				bounds;
	Str255				aString;
	short				width;
	short				height;
	FontInfo			info;
	Pattern				grayPattern;
	
	SetUpA4();
	PenNormal();

	switch (item) {
		case kConfigHowManyUser:
			pic = GetPicture(kGrayBox);
			bounds = GetItemRect(dlg, kConfigHowManyUser);
			DrawPicture(pic, &bounds);
			NumToString(pMaxFileObjectsToRemember, aString);
			width = StringWidth(aString);
			GetFontInfo(&info);
			height = info.ascent + info.descent;
			MoveTo( (bounds.right + bounds.left - width) / 2,
					(bounds.bottom + bounds.top - height) / 2 + info.ascent);
			DrawString(aString);
			break;
		case kConfigHotKeyUser:
			GetIndPattern(&grayPattern, sysPatListID, 4);	// a nice gray
			bounds = GetItemRect(dlg, item);
			FrameRect(&bounds);

			InsetRect(&bounds, 2, 2);
			EraseRect(&bounds);
			CEDrawKey(pNewHotKey, &bounds);
			InvertRect(&bounds);

			if (GetDialogItemState(dlg, kConfigHotKeyCheck) == 0)
			{
				InsetRect(&bounds, -2, -2);
				PenMode(patBic);
				PenPat(&grayPattern);
				PaintRect(&bounds);
			}

			break;
	}

	RestoreA4();
}


void	HandleSpinControl(DialogPtr dlg)
{
	Rect		bounds;
	short		arrowPICTID;
	short		choke;
	Point		mouseLocation;
	long		tempLong;
	short		prevArrow;
	short		prevMax;

	bounds = GetItemRect(dlg, kConfigArrowPICT);
	choke = 0;
	prevArrow = kArrowPICT;
	prevMax = pMaxFileObjectsToRemember;
	while (StillDown()) {
		GetMouse(&mouseLocation);
		arrowPICTID = kArrowPICT;
		if (PtInRect(mouseLocation, &bounds)) {
	
			if (mouseLocation.v < (bounds.bottom + bounds.top) / 2) {
				arrowPICTID = kUpArrowPICT;
				if (pMaxFileObjectsToRemember < 99)
					pMaxFileObjectsToRemember++;
			} else {
				arrowPICTID = kDownArrowPICT;
				if (pMaxFileObjectsToRemember > 1)
					pMaxFileObjectsToRemember--;
			}
	
			if (pMaxFileObjectsToRemember != prevMax) {
				ConfigUserProc(dlg, kConfigHowManyUser);
				prevMax = pMaxFileObjectsToRemember;
			}
		}
	
		if (arrowPICTID != prevArrow) {
			DrawArrow(dlg, arrowPICTID);
			choke = 30;
			prevArrow = arrowPICTID;
		}
	
		if (choke > 3)
			--choke;
		Delay(choke/3, &tempLong);
	};
	DrawArrow(dlg, kArrowPICT);
}

void	DrawArrow(DialogPtr dlg, short arrowID)
{
	PicHandle	pic;
	Rect		bounds;

	pic = GetPicture(arrowID);
	bounds = GetItemRect(dlg, kConfigArrowPICT);
	DrawPicture(pic, &bounds);
}


//--------------------------------------------------------------------------------

void	DoAbout()
{
	GrafPtr		oldPort;
	short		oldFile;
	Str255		versionString;
	DialogPtr	dlg;
	short		fNum;
	short		item;

	if (gRefNum != -1) {
		oldFile = UseMyResFile();
		GetVersionString(1, TRUE, (StringPtr) &versionString);
		ParamText("\pKeith Rollin", versionString, NIL, NIL);
		GetPort(&oldPort);
		dlg = CreateNewDialog(kAboutDialog, kAboutOK, 0);
		GetFNum("\pNew York", &fNum);
		TextFont(fNum);
		TextSize(12);
		ShowWindow(dlg);
		ModalDialog(NIL, &item);
		DisposeDialog(dlg);
		SetPort(oldPort);
		UseResFile(oldFile);
	}
}


//--------------------------------------------------------------------------------

void	DoTrash(ControlHandle menuControl)
{
	GrafPtr			oldPort;
	short			oldFile;
	DialogPtr		dlg;
	short			item;
	DynamicArrayHdl	list;

	if (gRefNum != -1) {
		oldFile = UseMyResFile();
		GetPort(&oldPort);
		item = NoteAlert(kTrashAlert, NIL);
		SetPort(oldPort);
		UseResFile(oldFile);

		if (item == ok) {
			//
			// Another way to do this: set gMaxFilesToRemember to
			// zero and call CullOldest.
			//

			list = gFileList;
			do {
				while ((**list).numRecords > 0) {
					DeleteFileObject(list, (FileObjectHdl) (**list).records[0]);
				}
				list = (list == gFileList) ? gDirectoryList : NIL;
			} while (list != NIL);

			RebuildMenus(menuControl);
		}
	}
}


//--------------------------------------------------------------------------------

void	RebuildMenus(ControlHandle menuControl)
{
	AddDirectories(ZGetMHandle(menuControl, kDirectoryMenuResID));
	AddFiles(ZGetMHandle(menuControl, kFileMenuResID));
}


//--------------------------------------------------------------------------------

void	FlushMenuText()
{
	DynamicArrayHdl	list;
	short			index;
	FileObjectHdl	obj;
	
	list = gFileList;
	do {
		for (index = 0; index < (**list).numRecords; index++) {
			obj = (FileObjectHdl) (**list).records[index];
			DisposeHandle((Handle) (**obj).menuText);
			(**obj).menuText = NIL;
		}
		list = (list == gFileList) ? gDirectoryList : NIL;
	} while (list != NIL);
}


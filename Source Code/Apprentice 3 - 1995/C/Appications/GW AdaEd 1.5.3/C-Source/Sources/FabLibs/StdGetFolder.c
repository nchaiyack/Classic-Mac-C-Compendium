/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

/* I did not write the code in this file; I took it from some sample code
from Apple, I think.
I modified the original code a little bit, though. */

//#pragma load "MacDump"
#ifndef __MOREFILESEXTRAS__
#include "MoreFilesExtras.h"
#endif

#include	"UtilsSys7.h"
#include	"StdGetFolder.h"

/* typedefs */

typedef struct {
	StandardFileReply *replyPtr;
	void (*updateProc)(EventRecord *);
	void (*activateProc)(EventRecord *);
	FSSpec oldSelection;
	short	SelectStrRsrc;
//	short	DeskStrRsrc;
} SFData, *SFDataPtr;

/* prototypes */

static pascal short MyDlgHook(short item, DialogPtr theDlg, SFDataPtr sfp);
static pascal Boolean MyModalFilter(DialogPtr theDlg, EventRecord *ev, short *itemHit, SFDataPtr myData);
//static pascal Boolean FilterAllFiles(CInfoPBPtr pb, Ptr myDataPtr);
static void SetSelectButtonName(ConstStr255Param selName, const DialogPtr theDlg, short, Boolean);
static Boolean SameFile(const FSSpecPtr file1, const FSSpecPtr file2);
static OSErr GetDeskFolderSpec(FSSpecPtr, short vRefNum);
static OSErr MakeCanonFSSpec(FSSpecPtr);
static Boolean ShouldHiliteSelect(FSSpecPtr);

/* constants */

#define	kSelectItem			10
#define	kCanSelectDesktop	true
#define	kSelectKey			's'

/* globals */

Boolean	gHasFindFolder;
FSSpec	gDeskFolderSpec;

/*
	strHndl = GetString(kSelectStrRsrc);
	strHndl = GetString(kDeskStrRsrc);
*/

/* do getfile */

Boolean CustomGet(FSSpec *fSpec,
				void (*DoUpdate)(EventRecord *),
				void (*DoActivate)(EventRecord *),
				short SFDlgID, short SelectStrRsrc/*, short DeskStrRsrc*/)
{
//FileFilterYDUPP	FilterAllFilesUPP = NewFileFilterYDProc(FilterAllFiles);
DlgHookYDUPP	MyDlgHookUPP = NewDlgHookYDProc(MyDlgHook);
ModalFilterYDUPP	MyModalFilterUPP = NewModalFilterYDProc(MyModalFilter);
Point	where = {-1,-1};
OSType	myType = '�ldr';
StandardFileReply	sfReply;
SFData	sfUserData;
Boolean	targetIsFolder, wasAliased;

/* initialize user data area */

sfUserData.replyPtr = &sfReply;
sfUserData.updateProc = DoUpdate;
sfUserData.activateProc = DoActivate;
sfUserData.oldSelection.vRefNum = -9999;	/* init to ridiculous value */
sfUserData.SelectStrRsrc = SelectStrRsrc;
//sfUserData.DeskStrRsrc = DeskStrRsrc;

CustomGetFile(nil, 1, &myType, &sfReply, SFDlgID, where, MyDlgHookUPP,
				MyModalFilterUPP, nil, nil, &sfUserData);

if (sfReply.sfGood) {
	if (ResolveAliasFile(&sfReply.sfFile, true, &targetIsFolder, &wasAliased)) {
		sfReply.sfGood = false;
		}
	else if (StrLength(sfReply.sfFile.name) == 0) {
		(void) FSMakeFSSpecCompat(sfReply.sfFile.vRefNum, sfReply.sfFile.parID, nil, fSpec);
//		sfReply.sfGood = false;
		}
	else
		*fSpec = sfReply.sfFile;
	}

if (MyModalFilterUPP)
	DisposeRoutineDescriptor(MyModalFilterUPP);
if (MyDlgHookUPP)
	DisposeRoutineDescriptor(MyDlgHookUPP);
//if (FilterAllFilesUPP)
//	DisposeRoutineDescriptor(FilterAllFilesUPP);

return sfReply.sfGood;
}


/*	this dialog hook checks the contents of the additional edit fields
	when the user selects a file.  The focus of the dialog is changed if one
	of the fields is out of range.Ptr userData
*/

pascal short MyDlgHook(short item, DialogPtr theDlg, SFDataPtr sfUserData)
{
FSSpec curSpec;

if (GetWRefCon(GetDialogWindow(theDlg)) == sfMainDialogRefCon) {	
//	sfUserData = (SFDataPtr) userData;
	
	if (item != sfHookFirstCall && item != sfHookLastCall) {
/*		if (item == sfItemVolumeUser) {
			sfUserData->replyPtr->sfFile.name[0] = '\0';
			sfUserData->replyPtr->sfFile.parID = fsRtDirID;
			sfUserData->replyPtr->sfIsFolder = false;
			sfUserData->replyPtr->sfIsVolume = false;
			sfUserData->replyPtr->sfFlags = 0;
			item = sfHookChangeSelection;
			}
*/
		if (!SameFile(&sfUserData->replyPtr->sfFile, &sfUserData->oldSelection)) {
			curSpec = sfUserData->replyPtr->sfFile;
			(void)MakeCanonFSSpec(&curSpec);
			
			if (curSpec.vRefNum != sfUserData->oldSelection.vRefNum)
				GetDeskFolderSpec(&gDeskFolderSpec, curSpec.vRefNum);	
			SetSelectButtonName(curSpec.name, theDlg, sfUserData->SelectStrRsrc, ShouldHiliteSelect(&curSpec));
		
			sfUserData->oldSelection = sfUserData->replyPtr->sfFile;
			}

		if (item == kSelectItem)
			item = sfItemOpenButton;
		}
	}
return item;
}


pascal Boolean MyModalFilter(DialogPtr theDlg, EventRecord *ev, short *itemHit, SFDataPtr myData)
{
Boolean evHandled = false;
char keyPressed;

if (GetWRefCon(GetDialogWindow(theDlg)) == sfMainDialogRefCon)
	switch (ev->what) {
		case keyDown:
		case autoKey:
			keyPressed = CHARFROMMESSAGE(ev->message);
			if (ev->modifiers & cmdKey) {
				switch (keyPressed) {
					case kSelectKey:
						FlashButton(theDlg, kSelectItem);
						*itemHit = kSelectItem;
						evHandled = true;
						break;
				}
			}
			break;
		case updateEvt:
			if (theDlg != (DialogPtr)ev->message && myData->updateProc) {
				myData->updateProc(ev);
				evHandled = true;
				}
			break;
		case activateEvt:
			if (theDlg != (DialogPtr)ev->message && myData->activateProc) {
				myData->activateProc(ev);
				evHandled = true;
				}
			break;
		}

return evHandled;
}

/*
pascal Boolean FilterAllFiles(CInfoPBPtr pb, Ptr myDataPtr)
{
return ((pb->hFileInfo.ioFlAttrib & (1 << 4)) == 0);
}
*/

void SetSelectButtonName(ConstStr255Param selName, const DialogPtr theDlg, short SelectStrRsrc, Boolean hilited)
{
Str63	storeName, tempLenStr, tempSelName;
Rect	iRect;
StringHandle	theString;
Handle	iHndl;
short	iType;

(void) PLstrcpy(tempSelName, selName);
GetDialogItem(theDlg, kSelectItem, &iType, &iHndl, &iRect);

/* truncate select name to fit in button */

theString = GetString(SelectStrRsrc);

(void) PLstrcpy(tempLenStr, *theString);

PLstrcat(tempLenStr, "\p ��  ");
TruncString(iRect.right - iRect.left - StringWidth(tempLenStr), tempSelName, smTruncMiddle);

(void) PLstrcpy(storeName, *theString);
PLstrcat(storeName, "\p �");
PLstrcat(storeName, tempSelName);
PLstrcat(storeName, "\p�");

SetControlTitle((ControlHandle)iHndl, storeName);

//SetDItem(theDlg,kSelectItem,iType,iHndl,&iRect);

HiliteControl((ControlHandle)iHndl, hilited ? 0 : 255);
ValidRect(&iRect);
}


Boolean SameFile(const FSSpecPtr file1, const FSSpecPtr file2)
{
return ((file1->vRefNum == file2->vRefNum) &&
		(file1->parID == file2->parID) &&
		(EqualString(file1->name,file2->name,false,true)));
}


OSErr GetDeskFolderSpec(FSSpecPtr fSpec, short vRefNum)
{
register OSErr err = -1;

if (gHasFindFolder) {
	StrLength(fSpec->name) = 0;
	err = FindFolder(vRefNum, kDesktopFolderType, kDontCreateFolder,
						&fSpec->vRefNum,&fSpec->parID);
	if (err == noErr)
		err = MakeCanonFSSpec(fSpec);
	}
else
	fSpec->vRefNum = -9999;

return err;
}


Boolean ShouldHiliteSelect(FSSpecPtr fSpec)
{
//StringHandle	strHndl;

if (CheckVolLock(nil, fSpec->vRefNum))
	return false;
else if (FSpCheckObjectLock(fSpec))
	return false;
else if (SameFile(fSpec, &gDeskFolderSpec)) {
//	strHndl = GetString(DeskStrRsrc);
//	(void) PLstrcpy(fSpec->name, *strHndl);
	return kCanSelectDesktop;
	}
else
	return true;
}

OSErr MakeCanonFSSpec(FSSpecPtr fSpec)
{
DirInfo infoPB;
OSErr err = noErr;

if (StrLength(fSpec->name) == 0) {
	infoPB.ioNamePtr = fSpec->name;
	infoPB.ioVRefNum = fSpec->vRefNum;
	infoPB.ioDrDirID = fSpec->parID;
	infoPB.ioFDirIndex = -1;
	err = PBGetCatInfoSync((CInfoPBPtr)&infoPB);
	fSpec->parID = infoPB.ioDrParID;
	}
return err;
}


#include "Global.h"
#include "MiniFinder.h"
#include "Pack3.h"				// gRefNum
#include "PLStringFuncs.h"		// PStringCopy
#include "Utilities.h"			// MyOpenResFile

#include <Script.h>				// smTruncMiddle


HBQRec		gHotRecord = {	NIL,				// HBQueue
							'PStk',				// HBSig
							1,					// HBSelect
							HBMAddToAppleMenu
							+ HBMHasHotKey,		// HBFlags
							0x1F0F1800,			// HBKeyStroke (ctrl-option-o)
							NIL,				// HBProcPtr
							"\pMiniFinder",		// HBName[25]
							NIL,				// HBMenu
							0};					// HBItem		// ¥¥¥ Allocate dynamically?

#ifdef __NEVER__
FSSpec				pPreviouslySelectedFile;
#endif

void				MiniFinder(void);
void				CopyReplyToLaunchRecord(StandardFileReply *reply, 
						LaunchHdl launchHandle, short index);
#ifdef __NEVER__
pascal short		GetFolderDlgHook(short item, DialogPtr theDlg, StandardFileReply* reply);
void				SetSelectButtonText(StringPtr selName, DialogPtr theDlg);
Boolean				SameFile(FSSpec *file1, FSSpec *file2);
OSErr				GetDeskFolderSpec(short vRefNum, FSSpec *fSpec);
OSErr				MakeCanonFSSpec(FSSpec *fSpec);
OSErr				GoToFolder(FSSpec *fSpec);
#endif


//--------------------------------------------------------------------------------

void	InstallMiniFinder()
{
	OSErr				err;

	err = CEOpenToolbox();
	if (err == noErr) {
		gHotRecord.HBProcPtr = (Ptr) MiniFinder;
		err = CERegister(&gHotRecord);
		if (err != noErr) {
			CEPostError(err);
		}
	} 
}


//--------------------------------------------------------------------------------

void	MiniFinder(void)
{
	Boolean				closeIt;
	Point				where;
	StandardFileReply	reply;
	SFTypeList			types;
	LaunchHdl			launchHandle;
	OSErr				err;
	FindAppRec			findAppRec;
	FInfo				fndrInfo;
	LaunchOnePtr		tempPtr;

	SetUpA4();

#ifdef __NEVER__
	gRefNum = MyOpenResFile(gMe, fsRdPerm, &closeIt);
	if (gRefNum != -1) {

		pPreviouslySelectedFile.vRefNum = 0;
		where.h = -1; where.v = -1;
		CustomGetFile(NIL, -1, nil, &reply, kGetFolderDialog, where,
						(DlgHookYDProcPtr) GetFolderDlgHook, nil, nil, nil, &reply);
#endif

		StandardGetFile(NIL, -1, nil, &reply);
	
		if (reply.sfGood) {
	
			if (reply.sfType == kApplicationSig) {
				launchHandle = (LaunchHdl) NewHandleSys(sizeof(LaunchRec));
				(**launchHandle).numDocs = 0;
				CopyReplyToLaunchRecord(&reply, launchHandle, 0);
				err = noErr;
			} else {
				launchHandle = (LaunchHdl) NewHandleSys(sizeof(LaunchRec) + sizeof(LaunchOne));
				(**launchHandle).numDocs = 1;
				CopyReplyToLaunchRecord(&reply, launchHandle, 1);
				
				err = FSpGetFInfo(&reply.sfFile, &fndrInfo);
				if (err == noErr) {
					HLock((Handle) launchHandle);
					tempPtr = &(**launchHandle).f[0];
					tempPtr->fVRef = 0;		// search all volumes
					tempPtr->fType = kApplicationSig;
					err = CEFindApp(fndrInfo.fdCreator, tempPtr->fName,
										 &tempPtr->fVRef, &tempPtr->fDir);
					HUnlock((Handle) launchHandle);
				}
			}
	
			if (err == noErr) {
				err = CETheLaunch(launchHandle);
			}
	
			if (err != noErr) {
				CEPostError(err);
			}
	
			DisposeHandle((Handle) launchHandle);
		}

#ifdef __NEVER__
		if (closeIt) {
			CloseResFile(gRefNum);
			gRefNum = -1;
		}
	}
#endif

	RestoreA4();
}


//--------------------------------------------------------------------------------

void	CopyReplyToLaunchRecord(StandardFileReply* reply, LaunchHdl launchHandle,
									short index)
{
	LaunchOnePtr	tempPtr = &(**launchHandle).f[index];
	
	PStringCopy(tempPtr->fName, reply->sfFile.name);
	tempPtr->fDir = reply->sfFile.parID;
	tempPtr->fVRef = reply->sfFile.vRefNum;
	tempPtr->fType = reply->sfType;
}


//--------------------------------------------------------------------------------

#ifdef __NEVER__
pascal short GetFolderDlgHook(short item, DialogPtr theDlg, StandardFileReply* reply)
{
	Boolean		hiliteButton;
	FSSpec		curSpec;
	FSSpec		deskFolderSpec;
	
	SetUpA4();
	
	if (item != sfHookFirstCall && item != sfHookLastCall) {
		
		if (!SameFile(&reply->sfFile, &pPreviouslySelectedFile)) {

			//
			// Copy the FSSpec in the reply record, since weÕll
			// be munging it.
			//
			curSpec = reply->sfFile;
			
			//
			// If the user has selected a folder, spec.name will
			// hold the "\0" and spec.parID will hold the directoryÕs
			// ID. MakeCanonFSSpec turns that into an FSSpec for
			// that directory.
			//
			MakeCanonFSSpec(&curSpec);
			
			//
			// If the user has selected the desktop folder, donÕt
			// use the name ÒDesktop FolderÓ in the button. Instead,
			// use the friendlier ÒDesktopÓ.
			//
			GetDeskFolderSpec(curSpec.vRefNum, &deskFolderSpec);	
			if (SameFile(&curSpec, &deskFolderSpec)) {
				GetIndString(curSpec.name, kStrings, kDesktopName);
			}

			SetSelectButtonText(curSpec.name, theDlg);
			
			pPreviouslySelectedFile = reply->sfFile;
		}

		if (item == kSelectItem)
			item = sfItemOpenButton;
	}
	
	RestoreA4();
		
	return item;
}


//--------------------------------------------------------------------------------

void SetSelectButtonText(StringPtr folderName, DialogPtr theDlg)
{
	Str255			selectString;
	Str255			tempString;
	Str255			fullString;
	Rect			iRect;
	short			buttonWidth;
	ControlHandle	selectButton;
	
	GetIndString(selectString, kStrings, kSelectString);

	PStringCopy(tempString, folderName);
	
	/* Truncate select name to fit in button. */
	/* First, find size of "Select ÒÓ" boilerplate */

	iRect = GetItemRect(theDlg, kSelectItem);
	buttonWidth = iRect.right - iRect.left;
	PStringCopy(tempString, selectString);
	PLstrcat(tempString, "\p ÒÓ");
	buttonWidth -= StringWidth(tempString);

	PStringCopy(tempString, folderName);
	TruncString(buttonWidth, tempString, smTruncMiddle);
	
	PStringCopy(fullString, selectString);
	PLstrcat(fullString, "\p Ò");
	PLstrcat(fullString, tempString);
	PLstrcat(fullString, "\pÓ");

	selectButton = (ControlHandle) GetItemHandle(theDlg, kSelectItem);
	SetCTitle(selectButton, fullString);
}


//--------------------------------------------------------------------------------

Boolean SameFile(FSSpec *file1,FSSpec *file2)
{
	if (file1->vRefNum != file2->vRefNum)
		return false;
	if (file1->parID != file2->parID)
		return false;
	if (!PLstrcmp(file1->name, file2->name))
		return false;
	
	return true;
}


//--------------------------------------------------------------------------------

OSErr GetDeskFolderSpec(short vRefNum, FSSpec *fSpec)
{
	OSErr	err;
	
	fSpec->name[0] = 0;
	err = FindFolder(vRefNum, kDesktopFolderType, kDontCreateFolder,
						&fSpec->vRefNum, &fSpec->parID);
	if (err != noErr)
		return err;
	
	return MakeCanonFSSpec(fSpec);
}


//--------------------------------------------------------------------------------

OSErr MakeCanonFSSpec(FSSpec *fSpec)
{
	DirInfo infoPB;
	OSErr	err;

	if (fSpec->name[0] != '\0')
		return;
		
	infoPB.ioNamePtr = fSpec->name;
	infoPB.ioVRefNum = fSpec->vRefNum;
	infoPB.ioDrDirID = fSpec->parID;
	infoPB.ioFDirIndex = -1;
	err = PBGetCatInfo(&infoPB,false);
	fSpec->parID = infoPB.ioDrParID;
	
	return err;
}


//--------------------------------------------------------------------------------

OSErr GoToFolder(FSSpec *fSpec)
{
	AppleEvent aeEvent,  aeReply;
	AEDesc myAddressDesc,  aeDirDesc, listElem;
	FSSpec dirSpec,  procSpec;
	AEDesc fileList;
	StandardFileReply myReply;
	OSErr myErr;
	ProcessSerialNumber process;
	AliasHandle DirAlias,  FileAlias;
	FSSpecPtr theFileToOpen = nil;
	ProcessInfoRec infoRec;
	Str31 processName;

	infoRec.processInfoLength = sizeof(ProcessInfoRec);
	infoRec.processName = &processName;
	infoRec.processAppSpec = &procSpec;

	/* go find the Finder's process information, please */
	if (FindAProcess('FNDR', 'MACS', &process, &infoRec))
		return (procNotFound);

	if (myErr = AECreateDesc(typeProcessSerialNumber, (Ptr) & process, sizeof(process), &myAddressDesc))
		return (myErr);

	/* Create the FinderEvent */
	if (myErr = AECreateAppleEvent('FNDR', 'sope', &myAddressDesc, kAutoGenerateReturnID, kAnyTransactionID, &aeEvent))
		return (myErr);

	/* make a spec for the parent folder */
	FSMakeFSSpec(theFileToOpen->vRefNum, theFileToOpen->parID, nil, &dirSpec);
	NewAlias(nil, &dirSpec, &DirAlias);

	/* Create alias for file */
	/* if you are opening a window, then you make the file alias the same as the dir alias */
	switch (which)
	{
		case 1:
			NewAlias(nil, theFileToOpen, &FileAlias);
			break;
		case 2:
			NewAlias(nil, &dirSpec, &FileAlias);
			SetFrontProcess(&process);
			break;
	}
	/* Create the file  list */
	if (myErr = AECreateList(nil, 0, false, &fileList))
		return (myErr);

	/*  create the folder  descriptor */
	HLock((Handle)DirAlias);
	AECreateDesc(typeAlias, (Ptr) * DirAlias, GetHandleSize((Handle)DirAlias), &aeDirDesc);
	HUnlock((Handle)DirAlias);
	if ((myErr = AEPutParamDesc(&aeEvent, keyDirectObject, &aeDirDesc)) == noErr)
	{
		/* done with the desc, kill it */
		AEDisposeDesc(&aeDirDesc);
		/*  create the file descriptor and add to aliasList */
		HLock((Handle)FileAlias);
		AECreateDesc(typeAlias, (Ptr) * FileAlias, GetHandleSize((Handle)FileAlias), &listElem);
		HLock((Handle)FileAlias);
		myErr = AEPutDesc(&fileList, 0, &listElem);
	}

	if (myErr)
		return (myErr);
	AEDisposeDesc(&listElem);
	/* Add the file alias list to the event */
	if (myErr = AEPutParamDesc(&aeEvent, aeSelectionKeyword, &fileList))
		return (myErr);
	myErr = AEDisposeDesc(&fileList);

	myErr = AESend(&aeEvent, &aeReply, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer, kAENormalPriority, kAEDefaultTimeout, nil, nil);

	AEDisposeDesc(&aeEvent);
}
#endif

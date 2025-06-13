#include "kant build files.h"
#include "kant build window.h"
#include "kant build lists.h"
#include "resource layer.h"
#include "file interface.h"
#include "file utilities.h"
#include "menus.h"
#include "window layer.h"
#include "program globals.h"
#include "memory layer.h"

#define MODULE_FOLDER_NAME	"\pModules Ä"

static	OSErr CreateNewModuleFile(FSSpec *theFS, Boolean deleteTheThing);

void UseTheModule(FSSpec *theFS, Boolean useDefault)
{
	gModuleFS=*theFS;
	gUseDefault=useDefault;
	RebuildInsertMenu();
}

OSErr GetNamedModuleFS(FSSpec *theFS, Str255 theName)
{
	FSSpec			appFS;
	OSErr			oe;
	CInfoPBRec		pb_dir;
	
	if ((oe=GetApplicationFSSpec(&appFS))!=noErr)
		return oe;
	
	pb_dir.dirInfo.ioCompletion=0L;
	pb_dir.dirInfo.ioNamePtr=MODULE_FOLDER_NAME;
	pb_dir.dirInfo.ioVRefNum=appFS.vRefNum;
	pb_dir.dirInfo.ioFDirIndex=0; /* very important */
	pb_dir.dirInfo.ioDrDirID=appFS.parID;
	if ((oe=PBGetCatInfo(&pb_dir, FALSE))!=noErr)
		return oe;
	
	Mymemcpy((Ptr)theFS->name, (Ptr)theName, theName[0]+1);
	theFS->vRefNum=appFS.vRefNum;
	theFS->parID=pb_dir.dirInfo.ioDrDirID;
	
	return FileExists(theFS);
}

OSErr BuildReferencesList(MenuHandle theMenu, FSSpec theFS)
{
	OSErr			oe;
	short			oldRefNum, refNum, numStringResources, resID, i;
	Boolean			alreadyOpen;
	Str255			resName;
	ResType			resType;
	Handle			resHandle;
	short			resAttrs;
	Boolean			gotone;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, TRUE))!=noErr)
		return oe;
	
	numStringResources=Count1Resources('STR#');
	gotone=FALSE;
	for (i=1; i<=numStringResources; i++)
	{
		resHandle=Get1IndResource('STR#', i);
		if (resHandle==0L)
			return ResError();
		
		GetResInfo(resHandle, &resID, &resType, resName);
		oe=ResError();
		if (oe!=noErr)
		{
			ReleaseResource(resHandle);
			CloseTheResFile(oldRefNum, refNum, alreadyOpen);
			return oe;
		}
		resAttrs=GetResAttrs(resHandle);
		oe=ResError();
		ReleaseResource(resHandle);
		if (oe!=noErr)
		{
			CloseTheResFile(oldRefNum, refNum, alreadyOpen);
			return oe;
		}
		
		if ((gShowAllRefs) || ((resAttrs&0x00ff) & resLocked))
		{
			AppendMenu(theMenu, "\p ");
			SetItem(theMenu, CountMItems(theMenu), resName);
			gotone=TRUE;
		}
	}
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	if (!gotone)
	{
		AppendMenu(theMenu, gShowAllRefs ? "\p(No references" : "\p(No interesting references");
	}
	
	return noErr;
}

OSErr BuildModulesList(MenuHandle theMenu)
{
	FSSpec			appFS;
	OSErr			oe;
	CInfoPBRec		pb_dir;
	HParamBlockRec	pb_file;
	Str255			theName;
	short			i;
	
	if ((oe=GetApplicationFSSpec(&appFS))!=noErr)
		return oe;
	
	pb_dir.dirInfo.ioCompletion=0L;
	pb_dir.dirInfo.ioNamePtr=MODULE_FOLDER_NAME;
	pb_dir.dirInfo.ioVRefNum=appFS.vRefNum;
	pb_dir.dirInfo.ioFDirIndex=0; /* very important */
	pb_dir.dirInfo.ioDrDirID=appFS.parID;
	if ((oe=PBGetCatInfo(&pb_dir, FALSE))!=noErr)
		return oe;
	
	if (pb_dir.dirInfo.ioDrNmFls==0)
		return noErr;
	
	for (i=1; (i<=pb_dir.dirInfo.ioDrNmFls); i++)
	{
		pb_file.fileParam.ioCompletion=0L;
		pb_file.fileParam.ioNamePtr=theName;
		pb_file.fileParam.ioVRefNum=appFS.vRefNum;
		pb_file.fileParam.ioFDirIndex=i;
		pb_file.fileParam.ioDirID=pb_dir.dirInfo.ioDrDirID;
		if ((oe=PBGetCatInfo((CInfoPBPtr)&pb_file, FALSE))==noErr)
		{
			if ((pb_file.fileParam.ioFlFndrInfo.fdType==BUILD_TYPE) &&
				(pb_file.fileParam.ioFlFndrInfo.fdCreator==CREATOR))
			{
				/* if theName contains a / character, it will be interpreted as a */
				/* meta-character by AppendMenu but not by SetItem */
				AppendMenu(theMenu, "\p ");
				SetItem(theMenu, CountMItems(theMenu), theName);
			}
		}
	}
	
	return noErr;
}

OSErr DoNewModule(void)
{
	FSSpec			theFS;
	Boolean			deleteTheThing;
	OSErr			oe;
	
	theFS.name[0]=0x00;
	if (!GetDestFile(&theFS, &deleteTheThing, "\pSave new module as..."))
		return -1;
	
	if ((oe=CreateNewModuleFile(&theFS, deleteTheThing))!=noErr)
		return oe;
	
	RebuildModulesMenu();
	
	return OpenTheModule(&theFS, TRUE, FALSE);
}

OSErr OpenTheModule(FSSpec *theFS, Boolean newModule, Boolean useOldFile)
{
	WindowRef		buildWindow;
	
	if ((!newModule) && (!useOldFile))
	{
		if (!GetSourceFile(theFS, BUILD_TYPE))
			return -1;
	}
	
	SetBuildWindowTitle(theFS->name);
	OpenTheIndWindow(kBuildWindow, kAlwaysOpenNew);
	buildWindow=GetIndWindowRef(kBuildWindow);
	SetWindowFS(buildWindow, *theFS);
	UseTheModule(theFS, FALSE);
	BuildReferenceListHandle(GetWindowFS(buildWindow), FALSE);
	BuildInstantListHandle(GetWindowFS(buildWindow), FALSE);
	
	return noErr;
}

Boolean ReferenceNameExistsQQ(FSSpec theFS, Str255 refName)
{
	OSErr			oe;
	short			oldRefNum, refNum;
	Boolean			alreadyOpen;
	Handle			resHandle;
	Boolean			exists;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, TRUE))!=noErr)
		return FALSE;
	
	resHandle=Get1NamedResource('STR#', refName);
	exists=(resHandle!=0L);
	if (exists)
		ReleaseResource(resHandle);
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	return exists;
}

OSErr DeleteOneReference(FSSpec theFS, Str255 refName)
{
	OSErr			oe;
	short			oldRefNum, refNum;
	Boolean			alreadyOpen;
	Handle			resHandle;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', refName);
	if (resHandle==0L)
		return ResError();
	
	RmveResource(resHandle);
	oe=ResError();
	if (oe==noErr)
	{
		UpdateResFile(refNum);
		DisposeHandle(resHandle);	/* correct, this handle is no longer a resource */
	}
	else ReleaseResource(resHandle);
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	return oe;
}

OSErr DeleteOneInstantiation(FSSpec theFS, Str255 refName, short stringIndex)
{
	OSErr			oe;
	short			oldRefNum, refNum, resID;
	Boolean			alreadyOpen;
	ResType			resType;
	Handle			resHandle;
	Str255			dummy;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', refName);
	if (resHandle!=0L)
	{
		GetResInfo(resHandle, &resID, &resType, dummy);
		ReleaseResource(resHandle);		/* just needed resID */
		
		oe=DeleteIndString(resID, stringIndex);
	}
	else oe=ResError();
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	return oe;
}

OSErr AddOneReference(FSSpec theFS, Str255 refName)
{
	OSErr			oe;
	short			oldRefNum, refNum;
	Boolean			alreadyOpen;
	Handle			resHandle;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=NewHandle(2L);
	**((short**)resHandle)=0;
	AddResource(resHandle, 'STR#', Unique1ID('STR#'), refName);
	oe=ResError();
	
	if (oe==noErr)
	{
		WriteResource(resHandle);
		oe=ResError();
	}
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	ReleaseResource(resHandle);
	
	return oe;
}

OSErr AddOneInstantiation(FSSpec theFS, Str255 refName, Str255 instantName)
{
	OSErr			oe;
	short			oldRefNum, refNum, resID;
	Boolean			alreadyOpen;
	Str255			dummy;
	ResType			resType;
	Handle			resHandle;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', refName);
	if (resHandle!=0L)
	{
		GetResInfo(resHandle, &resID, &resType, dummy);
		ReleaseResource(resHandle);		/* just needed resID */
		
		oe=AddIndString(instantName, resID);
	}
	else oe=ResError();
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	return oe;
}

OSErr ReplaceOneReference(FSSpec theFS, Str255 oldRef, Str255 newRef)
{
	OSErr			oe;
	short			oldRefNum, refNum, resID;
	Boolean			alreadyOpen;
	Str255			resName;
	ResType			resType;
	Handle			resHandle;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', oldRef);
	if (resHandle==0L)
		return ResError();
	
	GetResInfo(resHandle, &resID, &resType, resName);
	SetResInfo(resHandle, resID, newRef);
	oe=ResError();
	if (oe==noErr)
	{
		ChangedResource(resHandle);
		WriteResource(resHandle);
		UpdateResFile(refNum);
		oe=ResError();
	}
	
	ReleaseResource(resHandle);
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	return oe;
}

OSErr ReplaceOneInstantiation(FSSpec theFS, Str255 oldRef, short stringIndex, Str255 newStr)
{
	OSErr			oe;
	short			oldRefNum, refNum, resID;
	Boolean			alreadyOpen;
	Str255			resName;
	ResType			resType;
	Handle			resHandle;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', oldRef);
	if (resHandle!=0L)
	{
		GetResInfo(resHandle, &resID, &resType, resName);
		ReleaseResource(resHandle);		/* just needed resID */
		oe=SetIndString(newStr, resID, stringIndex);
	}
	else oe=ResError();
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	return oe;
}

OSErr SetInterestingBit(FSSpec theFS, Str255 referenceName, Boolean isInteresting)
{
	OSErr			oe;
	short			oldRefNum, refNum;
	Boolean			alreadyOpen;
	Handle			resHandle;
	short			theAttrs;
	
	if ((oe=OpenTheResFile(&theFS, &oldRefNum, &refNum, &alreadyOpen, FALSE))!=noErr)
		return oe;
	
	resHandle=Get1NamedResource('STR#', referenceName);
	if (resHandle!=0L)
	{
		theAttrs=GetResAttrs(resHandle);
		if (isInteresting)
			theAttrs|=resLocked;
		else
			theAttrs&=~resLocked;
		SetResAttrs(resHandle, theAttrs);
		if ((oe=ResError())==noErr)
		{
			ChangedResource(resHandle);
			oe=ResError();
		}
	}
	else oe=ResError();
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	if (resHandle!=0L)
		ReleaseResource(resHandle);
	
	return oe;
}

/* the rest of these are internal to kant build files.c */

static	OSErr CreateNewModuleFile(FSSpec *theFS, Boolean deleteTheThing)
{
	OSErr			oe;
	
	if (deleteTheThing)
		if ((oe=FSpDelete(theFS))!=noErr)
			return oe;
	
	FSpCreateResFile(theFS, CREATOR, BUILD_TYPE, smSystemScript);
	
	return ResError();
}

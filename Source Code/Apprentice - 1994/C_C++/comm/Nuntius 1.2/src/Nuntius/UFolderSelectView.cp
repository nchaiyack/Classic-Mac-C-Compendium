// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UFolderSelectView.cp

#include "UFolderSelectView.h"
#include "Tools.h"
#include "FontTools.h"
#include "FileTools.h"
#include "UPrefsDatabase.h"

#include <RsrcGlobals.h>

#include <Packages.h>
#include <ToolUtils.h>
#include <UDialog.h>
#include <Resources.h>
#include <Folders.h>
#include <Script.h>

#pragma segment MyViewTools

StandardFileReply gReply; // used from call-backs
CStr255 gPrevSelectedName;
Boolean gWasCancel, gIsDesktopFolder;
CStr255 gPromptString = "";
ControlHandle gGetDirButtonControlH;
CRect gGetDirButtonControlRect;

void GetPathNameFromDirID(short vRefNum, long dirID, CStr255 &pathName);

// some of this code is from SCN.18.StdFilev2a3


long GetSFCurDir() 
{
	return *(long*)CurDirStore;
}

short GetSFVRefNum()
{
	return -*(short*)SFSaveDisk; // obs: contains negative of vRefNum
}

void OStypetoChars(const OSType id, char *store)
{
	store[0] = char(id >> 24 & 255);
	store[1] = char(id >> 16 & 255); 
	store[2] = char(id >>  8 & 255);
	store[3] = char(id       & 255);
	store[4] = 0;
}

void DumpReply()
{
#if qDebug
	char store[100];
	OStypetoChars(gReply.sfType, store);
	CStr255 path, s;
	GetPathNameFromDirID(gReply.sfFile.vRefNum, gReply.sfFile.parID, path);
	fprintf(stderr, "\n");
	fprintf(stderr, "Dump of gReply:\n");
	fprintf(stderr, "good = %ld, replacing = %ld\n", long(gReply.sfGood), long(gReply.sfReplacing));
	fprintf(stderr, "sfType = $%lx = %s\n", gReply.sfType, store);
	fprintf(stderr, "SFSaveDisk = %ld, CurDirStore = %ld\n", GetSFVRefNum(), long(GetSFCurDir()));
	fprintf(stderr, "sfFile/path = %ld, %ld'\n", long(gReply.sfFile.vRefNum), long(gReply.sfFile.parID));
	fprintf(stderr, "sfFile/path = '%s'\n", (char*)path);
	s = gReply.sfFile.name;
	fprintf(stderr, "sfFile/file = '%s'\n", (char*)s);
	fprintf(stderr, "flags = %ld, isFolder = %ld, isVolume = %ld\n", long(gReply.sfFlags), long(gReply.sfIsFolder), long(gReply.sfIsVolume));
#endif
}

void CalculateDialogPosition(short id, CPoint &where, TWindow *window)
{
	CPoint c = (VPoint(window->fSize.h / 2, window->fSize.v / 2) + window->fLocation).ToPoint();
	Handle h = GetResource('DLOG', id);
	FailNILResource(h);
	CRect r = DialogTPtr(*h)->boundsRect;
	CPoint halfSize ((r.right - r.left) / 2, (r.bottom - r.top) / 2);
	where = c - halfSize;
/*
#if qDebug
	fprintf(stderr, "Location of window (%ld, %ld)\n", view->fLocation[hSel], view->fLocation[vSel]);
	fprintf(stderr, "Center of window: (%hd, %hd)\n", c[hSel], c[vSel]);
	fprintf(stderr, "halfSize of DLOG: (%hd, %hd)\n", halfSize.h, halfSize.v);
	fprintf(stderr, "where: (%hd, %hd)\n", where.h, where.v);
#endif
*/
}


//**** FILE SELECT ************************************************************
#pragma push
#pragma trace off
#pragma segment A5Ref
pascal Boolean FileFFilter(ParmBlkPtr pb, void *yourDataPtr) // call-back!
{
	TFileSelectView *fsv = (TFileSelectView*) yourDataPtr;
	return fsv->DontShowFileFilter(pb);
}
#pragma pop

#pragma segment MyViewTools

Boolean TFileSelectView::DontShowFileFilter(ParmBlkPtr pb)
{
	const short kFolderBit = 4; // bit set in ioFlAttrib for a folder
	unsigned char attrib = pb->fileParam.ioFlAttrib;
	if ( (attrib >> kFolderBit) & 1)
		return false; // always show folders

	// return true means don't show file
	OSType creator = pb->fileParam.ioFlFndrInfo.fdCreator;
	OSType type = pb->fileParam.ioFlFndrInfo.fdType;
	if (fNeededFileType == 'TEXT' && fNeededCreator == '????')
	{
		if (type == 'PREF' && creator == 'CSOm')
			return false; // special case for Eudora Settings file (contains signature)
	}
	if (fNeededCreator != '????' && creator != fNeededCreator)
		return true; // creator don't match: don't show it
	if (fNeededFileType != '????' && type != fNeededFileType)
		return true; // type don't match: don't show it
	return false;
}

TFileSelectView::TFileSelectView()
{
}


pascal void TFileSelectView::Initialize()
{
	inherited::Initialize();
	fButton = nil;
	fPath = nil;
	fGotFile = false;
	fNeededFileType = 'zxcv';
	fNeededCreator = '????';
}

pascal void TFileSelectView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TFileSelectView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fButton = (TButton*)FindSubView(kFolderSelectButtonID);
	if (!fButton)
	{
#if qDebug
		ProgramBreak("Sub-view button not found!");
#endif
		Failure(0, 0);
	}
	fPath = (TStaticText*)FindSubView(kFolderSelectPathID);
	if (!fPath)
	{
#if qDebug
		ProgramBreak("Sub-view statictext to path not found!");
#endif
		Failure(0, 0);
	}
}

pascal void TFileSelectView::Free()
{
	fButton = nil; // view
	fPath = nil; // view
	inherited::Free();
}

void TFileSelectView::SpecifyFileTypes(OSType fileType, OSType creator)
{
	fNeededFileType = fileType;
	fNeededCreator = creator;
}

Boolean TFileSelectView::DoGetFile()
{
	CPoint where;
	SFTypeList myTypeList;
		
	CalculateDialogPosition(sfGetDialogID, where, GetWindow());

	CustomGetFile(FileFFilter, -1, myTypeList, gReply, sfGetDialogID, where, nil, nil, nil, nil, this);
	if (!gReply.sfGood)
		return false;
	FInfo info;
	FailOSErr(FSpGetFInfo(gReply.sfFile, info));
	fSignature = info.fdCreator;
	fSpec = gReply.sfFile;
	fGotFile = true;
	UpdatePathName();
	return true;
}

void TFileSelectView::UpdatePathName()
{
	CStr255 path;
	GetPathNameFromDirID(fSpec.vRefNum, fSpec.parID, path);
	CStr255 name(fSpec.name);
	path += name;
	fPath->Focus();
	TextStyle ts = fPath->fTextStyle;
	SetPortTextStyle(ts);
	TruncString(short(fPath->fSize.h), path, smTruncMiddle);
	fPath->SetText(path, kRedraw);
}

pascal void TFileSelectView::DoEvent(EventNumber eventNumber, TEventHandler *source, TEvent *event)
{
	if (eventNumber == mButtonHit)
		DoGetFile();
	else
		inherited::DoEvent(eventNumber, source, event);
}

void TFileSelectView::Specify(const FSSpec &spec)
{
	fSpec = spec;
	fGotFile = true;
	UpdatePathName();
}

void TFileSelectView::GetFile(FSSpec &spec)
{
	spec = fSpec;
}

void TFileSelectView::DimState(Boolean state, Boolean redraw)
{
	fButton->DimState(state, redraw);
	fPath->DimState(state, redraw);
}

Boolean TFileSelectView::GotFile()
{
	return fGotFile;
}

OSType TFileSelectView::GetFileSignature()
{
	return fSignature;
}

void TFileSelectView::InitializeFromPreferences(OSType pref)
{
	if (!gPrefs->PrefExists(pref))
		return;
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		gPrefs->GetAliasPrefs(pref, spec);
		Specify(spec);
		fi.Success();
	}
	else // fail
	{
		// ignore currently
	}	
}

void TFileSelectView::StoreInPreferences(OSType pref)
{
	if (!fGotFile)
		return;
	FSSpec spec = fSpec;
	gPrefs->SetAliasPrefs(pref, spec);
}
//**** FOLDER SELECT ************************************************************
TFolderSelectView::TFolderSelectView()
{
}


pascal void TFolderSelectView::Initialize()
{
	inherited::Initialize();
	fButton = nil;
	fPath = nil;
	fGotFolder = false;
	fPromptString = "";
}

pascal void TFolderSelectView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
	CStr255 s;
	MyGetIndString(s, kAskFolderPrompt);
	fPromptString = s;
}

pascal void TFolderSelectView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fButton = (TButton*)FindSubView(kFolderSelectButtonID);
	if (!fButton)
	{
#if qDebug
		ProgramBreak("Sub-view button not found!");
#endif
		Failure(0, 0);
	}
	fPath = (TStaticText*)FindSubView(kFolderSelectPathID);
	if (!fPath)
	{
#if qDebug
		ProgramBreak("Sub-view statictext to path not found!");
#endif
		Failure(0, 0);
	}
}

pascal void TFolderSelectView::Free()
{
	fButton = nil; // view
	fPath = nil; // view
	inherited::Free();
}

#pragma segment A5Ref
#pragma push
#pragma trace off
void SetButtonTitle(ControlHandle ButtonHdl, CStr255 &name, const CRect &buttonRect)
{
	if (!ButtonHdl)
		return;
	gPrevSelectedName = name;
	CStr255 preName, postName;
	MyGetIndString(preName, kPreSelectFolderText);
	MyGetIndString(postName, kPostSelectFolderText);
	short width = buttonRect.GetLength(hSel) - StringWidth(preName + postName);
	TruncSystemFontString(width, name, smTruncMiddle);
	CStr255 s(preName);
	s += name;
	s += postName;
	name = s;
	SetCTitle(ButtonHdl, name);
	ValidRect(buttonRect);
}
#pragma pop

#pragma segment A5Ref
#pragma push
#pragma trace off
pascal Boolean FolderFFilter(ParmBlkPtr PB, void* /* yourDataPtr */) // call-back!
{ // return true means don't show file
	const short kFolderBit = 4; // bit set in ioFlAttrib for a folder
	unsigned char attrib = PB->fileParam.ioFlAttrib;
	if ( ((attrib >> kFolderBit) & 1) == 0)
		return true;
	return false;
}
#pragma pop

#pragma segment A5Ref
#pragma push
#pragma trace off
pascal short GetDirDlgHook(short item, DialogPtr theDialog, void* /* yourDataPtr */) // call-back!
{
	if (OSType(WindowPeek(theDialog)->refCon) != sfMainDialogRefCon)
		return item;
	CStr255 messageTitle;
	CRect iRect;
	Handle iHandle;
	short iKind;
	CStr255 selectedName;

	switch (item)
	{
		case sfHookFirstCall:
			GetDItem(theDialog, kGetDirMessage, iKind, iHandle, iRect);
			SetIText(iHandle, gPromptString);
#if 1
			CInfoPBRec info;
			info.dirInfo.ioCompletion = nil;
			info.dirInfo.ioNamePtr = StringPtr(&selectedName);
			info.dirInfo.ioVRefNum = GetSFVRefNum();
			info.dirInfo.ioFDirIndex = -1;
			info.dirInfo.ioDrDirID = GetSFCurDir();
			PBGetCatInfoSync(&info);
			GetDItem(theDialog, kGetDirButton, iKind, iHandle, gGetDirButtonControlRect);
			gGetDirButtonControlH = ControlHandle(iHandle);
			SetButtonTitle(gGetDirButtonControlH, selectedName, gGetDirButtonControlRect);
			break;
#endif
		case kGetDirButton:
			gWasCancel = false;
			return sfItemCancelButton;

		case kGetDesktopButton:
			gWasCancel = false;
			gIsDesktopFolder = true;
			return sfItemCancelButton;
		
		case sfItemCancelButton:
			gWasCancel = true;
			return sfItemCancelButton;

		case sfHookNullEvent:
			if (IsControlKeyDown())
				DumpReply();
#if 1
			if (gReply.sfIsFolder || gReply.sfIsVolume)
				selectedName = gReply.sfFile.name;
			else
			{
				CInfoPBRec info;
				info.dirInfo.ioCompletion = nil;
				info.dirInfo.ioNamePtr = StringPtr(&selectedName);
				info.dirInfo.ioVRefNum = gReply.sfFile.vRefNum;
				info.dirInfo.ioFDirIndex = -1;
				info.dirInfo.ioDrDirID = gReply.sfFile.parID;
				PBGetCatInfoSync(&info);
#if qDebug & 0
				fprintf(stderr, "Non-folder/volume name: '%s'\n", (char*)selectedName);
#endif
			}
			if (selectedName != gPrevSelectedName)
				SetButtonTitle(gGetDirButtonControlH, selectedName, gGetDirButtonControlRect);

#endif
			break;

		default:
			break;
	}
	return item;
}
#pragma pop

#pragma segment MyViewTools
Boolean TFolderSelectView::DoGetDirectory()
{
	CPoint where;

	SFTypeList myTypeList;
	const short kShowAllFiles = - 1;
		
	CalculateDialogPosition(kGetDirectoryDLOG, where, GetWindow());
	gPromptString = fPromptString;
	gPrevSelectedName = "";
	gWasCancel = true;
	gIsDesktopFolder = false;
	gGetDirButtonControlH = nil;
	
	CustomGetFile(FolderFFilter, kShowAllFiles, myTypeList,
						  gReply, kGetDirectoryDLOG, where,
						  &GetDirDlgHook, nil, nil,
						  nil, nil);
	if (gWasCancel)
		return false;
#if 1
	if (gIsDesktopFolder)
	{
		short vRefNum; long dirID;
		FailOSErr(FindFolder(gReply.sfFile.vRefNum, kDesktopFolderType, kCreateFolder, vRefNum, dirID));
		fVRefNum = vRefNum;
		fDirID = dirID;
	}
	else
	{
		CInfoPBRec info;
		info.dirInfo.ioCompletion = nil;
		info.dirInfo.ioNamePtr = gReply.sfFile.name;
		info.dirInfo.ioVRefNum = gReply.sfFile.vRefNum;
		info.dirInfo.ioFDirIndex = 0;
		info.dirInfo.ioDrDirID = gReply.sfFile.parID;
		PBGetCatInfoSync(&info);
		fVRefNum = gReply.sfFile.vRefNum;
		fDirID = info.dirInfo.ioDrDirID;
	}
#else
	fVRefNum = gReply.sfFile.vRefNum;
	fDirID = gReply.sfFile.parID;
#endif
	UpdatePathName();
	fGotFolder = true;
	return true;
}

void TFolderSelectView::UpdatePathName()
{
	CStr255 path;
	GetPathNameFromDirID(fVRefNum, fDirID, path);
	fPath->Focus();
	TextStyle ts = fPath->fTextStyle;
	SetPortTextStyle(ts);
	TruncString(short(fPath->fSize.h), path, smTruncMiddle);
	fPath->SetText(path, kRedraw);
}

pascal void TFolderSelectView::DoEvent(EventNumber eventNumber, TEventHandler *source, TEvent *event)
{
	if (eventNumber == mButtonHit)
		DoGetDirectory();
	else
		inherited::DoEvent(eventNumber, source, event);
}

void TFolderSelectView::Specify(short vRefNum, long dirID)
{
	fVRefNum = vRefNum;
	fDirID = dirID;
	UpdatePathName();
	fGotFolder = true;
}

void TFolderSelectView::GetDir(short &vRefNum, long &dirID)
{
	vRefNum = fVRefNum;
	dirID = fDirID;
}

void TFolderSelectView::DimState(Boolean state, Boolean redraw)
{
	fButton->DimState(state, redraw);
	fPath->DimState(state, redraw);
}

void TFolderSelectView::InitializeFromPreferences(OSType pref)
{
	if (!gPrefs->PrefExists(pref))
		return;
	FailInfo fi;
	if (fi.Try())
	{
		FSSpec spec;
		gPrefs->GetSilentDirAliasPrefs(pref, spec);
		Specify(spec.vRefNum, spec.parID);
		fi.Success();
	}
	else // fail
	{
		if (fi.error)
			fi.ReSignal(); // ignore cancel, open dialog anyway
	}
}

void TFolderSelectView::StoreInPreferences(OSType pref)
{
	if (!fGotFolder)
		return;
	FSSpec spec;
	spec.vRefNum = fVRefNum;
	spec.parID = fDirID;
	gPrefs->SetDirAliasPrefs(pref, spec);
}

// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNewsApplCmds.cp

#include "UNewsApplCmds.h"
#include "UNewsAppl.h"
#include "UGroupTreeDoc.h"
#include "UGroupTree.h"
#include "UGroupTreeCmds.h"
#include "UPrefsFileMgr.h"
#include "UProgress.h"
#include "UProgressCache.h"
#include "UArticleCache.h"
#include "UArticleTextCache.h"
#include "UNntp.h"
#include "UMacTCP.h"
#include "Tools.h"
#include "FileTools.h"
#include "NetAsciiTools.h"
#include "UPassword.h"
#include "UThread.h"
#include "UPreferencesView.h"
#include "ViewTools.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>

#include <UDialog.h>

#include <Resources.h>
#include <ToolUtils.h>
#include <Folders.h>
#include <Errors.h>
#include <Script.h>
#include <Packages.h>

#pragma segment MyMisc

#define qDebugPrefsBrief qDebug
#define qDebugPrefs qDebugPrefsBrief & 1

#if qDebug & 1
#define macroLog(x) fprintf(stderr, x)
#else
#define macroLog(x)
#endif

const kCurrentPrefsVersion = 27;
const kMinimumPrefsVersion = 17;

void CloseDownTheManagers()
{
	CloseDownUArticleCache();
	CloseDownUArticleTextCache();
	CloseDownUNntp();
	CloseDownUMacTCP();
	CloseDownUThread();
//	CloseDownUProgressCache(); Main thread as a progress window left in it
}

TOpenPrefsCommand::TOpenPrefsCommand()
{
}

pascal void TOpenPrefsCommand::Initialize()
{
	inherited::Initialize();
	fGroupTreeFile = nil;
	fDocFileList = nil;
	fOldPrefsVersion = 0;
	fMinNeededVersion = kCurrentPrefsVersion;
#if qDebugPrefs
	fprintf(stderr, "Creates TOpenPrefsCommand at $%lx\n", this);
#endif
}

void TOpenPrefsCommand::IOpenPrefsCommand(TList *docList)
{
	inherited::ICommand(cUpdatePrefs, nil, false, false, nil);
	fDocFileList = docList;
}

pascal void TOpenPrefsCommand::Free()
{
#if qDebugPrefs
	fprintf(stderr, "Frees TOpenPrefsCommand at $%lx\n", this);
#endif
	if (fDocFileList)
	{
		fDocFileList->FreeAll();
		fDocFileList->Free(); fDocFileList = nil;
	}
	FreeIfObject(fGroupTreeFile); fGroupTreeFile = nil;
	inherited::Free();
}

pascal void TOpenPrefsCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		macroLog("Updating prefs file location\n");
		FailInfo fi2;
		if (fi2.Try())
		{
			CStr255 name;
			MyGetIndString(name, kPrefsFileName);
			MoveFileToPrivate(name);
			fi2.Success();
		}
		else // fail
		{
			// silent, don't care
		}

		macroLog("Opens prefs file\n");
		OpenPrefsFile();
		macroLog("Upgrades prefs\n");
		UpgradePrefs();

		macroLog("Updating folder locations\n");
		UpdateFolders();

		macroLog("Creates progress indicator\n");
		gCurProgress->SetProgressType(true, false);
		gCurProgress->SetWorkToDo(kCandyStribes);
		gCurProgress->SetTitle(kStartupTitle);
		gCurProgress->SetUpperText(kStartupUpperText);
		TWindow *window = gCurProgress->GetWindow();
		window->Center(true, true, window->IsModal() );

		macroLog("Init UNntp, UArticleTextCache, UArticleCache etc\n");
		InitUNetAsciiTools();
		InitUPassword();
		InitUNntp();
		InitUArticleTextCache();
		InitUArticleCache();

		macroLog("Opens group tree document\n");
		OpenGroupTreeFile();
		
		if (fUpdateServerInfo)
		{
			macroLog("Asks for server info\n");
			AskNewsServer();
		}
		else
		{
			macroLog("Checks server IP\n");
			CheckNewsServerIP();
		}

		macroLog("Changes prefs valid state to OK\n");
		gPrefsFileMgr->SetPrefsValidationState(true);
		
		if (gNewsAppl->GetGroupTreeDoc()->GetGroupTree()->TreeIsEmpty())
		{
			macroLog("Rebuilds list of all groups\n");
			BuildListOfAllGroups();
		}

		macroLog("Changes state to real-running\n");
		gNewsAppl->fApplIsRunning = true;
		InvalidateMenus(); // menus change as we're real running now
		
		macroLog("Removes progress bar\n");
		gCurProgress->WorkDone();
		
		macroLog("Opens other Finder� documents\n");
		gApplication->OpenOld(cOpen, fDocFileList);
		
		if (fUpdateServerInfo)
		{
			// actually: is prefs file new, a sort of cPrefsFileFinderNew
			macroLog("Creates new empty documents as the prefs is new\n");
			gNewsAppl->GetGroupTreeDoc()->CreateGroupTreeWindow();
			gNewsAppl->OpenNew(cNewGroupListDoc);
		}
		
		if (gPrefs->GetBooleanPrefs('ChNw'))
		{
			FailInfo checkFI;
			if (checkFI.Try())
			{
				macroLog("Checks for new groups\n");
				TUpdateListOfAllGroupsCommand *cmd = new TUpdateListOfAllGroupsCommand();
				cmd->IUpdateListOfAllGroupsCommand(gNewsAppl->fGroupTreeDoc, false);
				cmd->DoIt();
				cmd->Free(); cmd = nil;
				checkFI.Success();
			}
			else
				if (checkFI.error)
					gApplication->ShowError(checkFI.error, checkFI.message);
		}

		macroLog("Restores old window state\n");
		gNewsAppl->RestoreWindowState();

		macroLog("Checks for old pre-final version\n");
		CheckPreFinalVersion();
		
		macroLog("Done. We're running.\n");
		fi.Success();
	}
	else // fail
	{
		gCurProgress->WorkDone();
		gApplication->DoMenuCommand(cQuit);
		FailNewMessage(fi.error, fi.message, messageOpenPrefsFailed);
	}
}

TFile *TOpenPrefsCommand::GetSpecialFile(OSType fileType)
{
	CObjectIterator iter(fDocFileList);
	for (TFile *aFile = (TFile *)iter.FirstObject(); iter.More(); aFile = (TFile *)iter.NextObject())
	{
		if (aFile->fFileType == fileType && aFile->fCreator == kSignature)
		{
			fDocFileList->Delete(aFile);
			return aFile;
		}
	}
	return nil;
}

void TOpenPrefsCommand::OpenPrefsFile()
{
	FSSpec spec;
	TFile *file = GetSpecialFile(kPrefsFileType);
	if (file)
	{
		file->GetFileSpec(spec);
		FreeIfObject(file); file = nil;
	}
	else
	{
		GetPrefsDocLocation(spec);
		CStr255 name;
		MyGetIndString(name, kPrefsFileName);
		CopyCString2String(name, spec.name);
	}
	Boolean targetIsFolder, wasAliased;
	OSErr err = ResolveAliasFile(spec, true, targetIsFolder, wasAliased);
	if (err != fnfErr)
		FailOSErr(err);
	*( (short*) SFSaveDisk) = -spec.vRefNum;
	*( (long*) CurDirStore) = spec.parID;
	PPrefsFileMgr *pfm = new PPrefsFileMgr();
	pfm->IPrefsFileMgr(spec);
	gPrefsFileMgr->Load();
}

void TOpenPrefsCommand::OpenGroupTreeFile()
{
	if (!fGroupTreeFile)
	{
#if qDebugPrefs
		fprintf(stderr, "Had no GroupTree file _object_, creates one\n");
#endif
		fGroupTreeFile = GetSpecialFile(kGroupTreeDocFileType);
		if (!fGroupTreeFile)
			fGroupTreeFile = gApplication->DoMakeFile(cNewGroupTreeDoc);
	}
#if qDebug
	if (!IsObject(fGroupTreeFile))
		ProgramBreak("fGroupTreeFile is not object");
#endif
	if (!FileExist(fGroupTreeFile))
	{
#if qDebugPrefs
		fprintf(stderr, "GroupTree _file_ did not exist, does not read it...\n");
#endif
		fGroupTreeFile->Free(); fGroupTreeFile = nil;
		gApplication->OpenNew(cNewGroupTreeDoc);
		return;
	}
	ResolveAliasFile(fGroupTreeFile);
	TFile *file = fGroupTreeFile; fGroupTreeFile = nil;
	gNewsAppl->OpenOneOld(file);
}

void TOpenPrefsCommand::UpgradePrefs()
{
	gPrefsFileMgr->SetPrefsValidationState(false);
	const long kNewPrefs = 0;
	fOldPrefsVersion = kNewPrefs;
	if (gPrefs->PrefExists('Vers'))
		fOldPrefsVersion = gPrefs->GetLongPrefs('Vers');
	long fMinNeededVersion = kCurrentPrefsVersion;
	if (gPrefs->PrefExists('Ver-'))
		fMinNeededVersion = gPrefs->GetLongPrefs('Ver-');
	if (kCurrentPrefsVersion < fMinNeededVersion)
	{
		gApplication->ShowError(errTooNewFileFormat, messageUsePrefs);
		fOldPrefsVersion = kNewPrefs;
		gPrefs->DeleteAll();
	}
#if qDebugPrefsBrief
	if (qDebugPrefs || fOldPrefsVersion != kCurrentPrefsVersion)
		fprintf(stderr, "Version of prefs data: %ld -> %ld\n", fOldPrefsVersion, kCurrentPrefsVersion);
#endif
	TextStyle ts;
	fUpdateServerInfo = false;

	if (fOldPrefsVersion == kNewPrefs)
	{
		fUpdateServerInfo = true;
		gPrefs->SetStringPrefs('SvNa', "");
		gPrefs->SetLongPrefs('SvIP', 0);
		gPrefs->SetLongPrefs('STio', 45);
		gPrefs->SetShortPrefs('artS', 0);
		gPrefs->SetShortPrefs('disS', 0);
		gPrefs->SetBooleanPrefs('BiAs', true);
		gPrefs->SetBooleanPrefs('UUna', false);
		gPrefs->SetBooleanPrefs('NaOK', false);
		gPrefs->SetStringPrefs('@adr', "");
		gPrefs->SetStringPrefs('Name', "");
		gPrefs->SetStringPrefs('Orga', "");
		gPrefs->SetBooleanPrefs('DUSi', false);
		gPrefs->SetBooleanPrefs('EdHe', false);
		gPrefs->SetBooleanPrefs('EdSi', false);
		gPrefs->SetBooleanPrefs('Head', false);
		gPrefs->SetLongPrefs('STio', 45);
		gPrefs->SetLongPrefs('Mntp', 5);
		gPrefs->SetLongPrefs('Tntp', 2 * 60 * 60);
		MAGetTextStyle(kArticleViewTextStyle, ts);
		gPrefs->SetTextStylePrefs('TSar', ts);
		MAGetTextStyle(kDiscListViewTextStyle, ts);
		gPrefs->SetTextStylePrefs('TSdi', ts);
		MAGetTextStyle(kGroupListTextStyle, ts);
		gPrefs->SetTextStylePrefs('TSgl', ts);
		MAGetTextStyle(kGroupTreeTextStyle, ts);
		gPrefs->SetTextStylePrefs('TSgt', ts);
		gPrefs->SetBooleanPrefs('BLau', false);
		gPrefs->SetBooleanPrefs('BLOP', true);
		gPrefs->DeletePrefs('DLgo');
		gPrefs->DeletePrefs('EDNa');
		gPrefs->DeletePrefs('BLNa');
		gPrefs->DeletePrefs('WGup');
		gPrefs->DeletePrefs('WGTp');
		gPrefs->SetStringPrefs('UNam', "");
		gPrefs->SetBooleanPrefs('EuOp', false);
		;	// 12 is only folder change
		gPrefs->SetLongPrefs('WrLn', 75);
		gPrefs->DeletePrefs('BLid');
		gPrefs->DeletePrefs('EDid');
		gPrefs->SetBooleanPrefs('AuUp', false);
	}

	if (fOldPrefsVersion <= 18)
		gPrefs->SetShortPrefs('SvPo', 119);

	if (fOldPrefsVersion <= 19)
	{
		gPrefs->SetBooleanPrefs('ChNw', false);
		gPrefs->SetBooleanPrefs('WSav', true);
	}

	if (fOldPrefsVersion <= 20)
		; // bogous AMem set (as short)

	if (fOldPrefsVersion <= 21)
		gPrefs->SetLongPrefs('AMem', 100 * 1024);

	if (fOldPrefsVersion <= 22)
		gPrefs->SetLongPrefs('XHSz', 25);

	if (fOldPrefsVersion <= 23)
		gPrefs->SetStringPrefs('Tran', "Latin1/US-ASCII (In/Out)");

	if (fOldPrefsVersion <= 24)
		gPrefs->SetLongPrefs('Mart', 500);

	if (fOldPrefsVersion <= 25)
		gPrefs->SetBooleanPrefs('2022', false);

	if (fOldPrefsVersion <= 26)
		gPrefs->SetBooleanPrefs('AlAu', false);

	if (fOldPrefsVersion <= 27)
		gPrefs->SetLongPrefs('CNAT', 10);

// --insert updates above this line ----------------------------------------

// At last!
	if (kMinimumPrefsVersion > fMinNeededVersion)
		gPrefs->SetLongPrefs('Ver-', kMinimumPrefsVersion);
	gPrefs->SetLongPrefs('Vers', kCurrentPrefsVersion);
}

void TOpenPrefsCommand::AskNewsServer()
{
	TWindow *window = gViewServer->NewTemplateWindow(kAskNewsServerView, nil);
#if qDebug
	if (!window)
		ProgramBreak("Could not create kAskNewsServerView window");
#endif
	window->Open();
	Boolean foundServer = false;
	while (!foundServer)
	{
		window->Show(true, kRedraw);
		CStr255 name("");
		while (name.Length() < 1) // needs at least one char
		{
			while (true)
			{
				IDType dismisser = MyPoseModally(window);
				if (dismisser == 'SOpt')
				{
					DoNewsServerPreferencesDialog();
					continue;
				}
				if (dismisser == 'ok  ')
					break;
				window->CloseByUser();
				Failure(0, 0);
			}
			TEditText *et = (TEditText*)window->FindSubView(kNewsServerNameViewID);
			if (et)
				et->GetText(name);
		}
		window->Show(false, !kRedraw);
		FailInfo fi;
		if (fi.Try())
		{
			UseNewServer(name);
			foundServer = true;
			fi.Success();
		}
		else
			if (fi.error)
				gApplication->ShowError(fi.error, fi.message);
	}
	window->CloseByUser();
}

void TOpenPrefsCommand::UseNewServer(CStr255 &name)
{
	PNntp *nntp = nil;
	VOLATILE(nntp);
	FailInfo fi;
	if (fi.Try())
	{
		gCurProgress->SetText(kNewServerObtainingAddress);
		gCurProgress->StartProgress(true);

		short pos = name.Pos(" ");
		if (pos)
		{
			CStr255 num = name;
			num.Delete(1, pos);
			name.Delete(pos, name.Length() - pos + 1);
			long port;
			StringToNum(num, port);
			gPrefs->SetShortPrefs('SvPo', short(port));
		}
		long ip;
		if (UsesFakeNntp())
			ip = 0x01020304;
		else
			ip = DotName2IP(name);
		gPrefs->SetStringPrefs('SvNa', name);
		gPrefs->SetLongPrefs('SvIP', ip);
		gCurProgress->Worked(0);

		gNntpCache->FlushCache();
		
		if (!IsOptionKeyDown())
		{
			nntp = gNntpCache->GetNntp(); // checks that we can connect
			gCurProgress->SetText(kNewServerBasicTest);
			nntp->ExamineNewsServer();  // and that it can be used
			gNntpCache->ReturnNntp(nntp); nntp = nil;
		}

		fi.Success();
	}
	else // fail
	{
		gNntpCache->DiscardNntp(nntp); nntp = nil;
		gCurProgress->WorkDone();
		if (fi.error == authNameErr)
			FailNewMessage(errCouldNotFindServer, fi.message, messageUseNewNewsServer);
		else
			FailNewMessage(fi.error, fi.message, messageUseNewNewsServer);
	}
}

void TOpenPrefsCommand::CheckNewsServerIP()
{
	PNntp *nntp = nil;
	VOLATILE(nntp);
	FailInfo fi;
	if (fi.Try())
	{
		gCurProgress->SetText(kNewServerObtainingAddress);
		gCurProgress->StartProgress(true);
		long old_ip = gPrefs->GetLongPrefs('SvIP');
		CStr255 name;
		gPrefs->GetStringPrefs('SvNa', name);
		long new_ip;
		if (UsesFakeNntp())
			new_ip = 0x01020304;
		else
			new_ip = DotName2IP(name);
		if (new_ip != old_ip)
		{
#if qDebug
			fprintf(stderr, "The newsserver IP has changed $%ld -> $%ld\n", old_ip, new_ip);
#endif
			gPrefs->SetLongPrefs('SvIP', new_ip);
			gNntpCache->FlushCache();
			if (!IsOptionKeyDown())
			{
				nntp = gNntpCache->GetNntp(); // checks that we can connect
				gCurProgress->SetText(kNewServerBasicTest);
				nntp->ExamineNewsServer();  // and that it can be used
				gNntpCache->ReturnNntp(nntp); nntp = nil;
			}	
		}
		gCurProgress->Worked(0);
		fi.Success();
	}
	else // fail
	{
		gNntpCache->DiscardNntp(nntp); nntp = nil;
		gCurProgress->WorkDone();
		if (fi.error == authNameErr)
			FailNewMessage(errCouldNotFindServer, fi.message, messageUpdateServerIP);
		else
			FailNewMessage(fi.error, fi.message, messageUpdateServerIP);
	}
}

void TOpenPrefsCommand::BuildListOfAllGroups()
{
	FailInfo fi;
	if (fi.Try())
	{
		gNewsAppl->GetGroupTreeDoc()->RebuildGroupTree();
		fi.Success();
	}
	else // fail
	{
		gNewsAppl->GetGroupTreeDoc()->DoInitialState(); // discard changes
		fi.ReSignal();
	}
}

void TOpenPrefsCommand::UpdateFolders()
{
	FailInfo fi;
	if (fi.Try())
	{
		if (fOldPrefsVersion <= 12)
		{
			MovePrefFileToTrash("Nuntius data files");
			CStr255 name;
			MyGetIndString(name, kGroupTreeFileName);
			MoveFileToPrivate(name);
			MoveGroupLists();
		}
		fi.Success();
	}
	else // fail, but silent
	{
#if qDebug
		fprintf(stderr, "Got exeption in UpdateSystemFolder, ignores it\n");
		fprintf(stderr, "- err = %ld\n", long(fi.error));
#endif
	}
}

void TOpenPrefsCommand::GetNuntiusTrashFolder(FSSpec &spec)
{
	OSErr err = FindFolder(spec.vRefNum, kTrashFolderType, kCreateFolder, 
		spec.vRefNum, spec.parID);
	if (err != noErr)
	{
#if qDebug
		fprintf(stderr, "Could not create trash folder for obsolote files, err = %ld\n", long(err));
#endif
		FailOSErr(err);
	}

	CStr255 s;
	MyGetIndString(s, kObsoloteFolderName);
	CopyCString2String(s, spec.name);
	long createdDirID;
	err = FSpDirCreate(spec, smRoman, createdDirID);
	if (err != noErr && err != dupFNErr)
	{
#if qDebug
		fprintf(stderr, "Could not create 'Obsolote Nuntius files' trash folder, err = %ld\n", long(err));
#endif
		FailOSErr(err);
	}
#if qDebugPrefs
	fprintf(stderr, "Nuntius trash folder created\n");
#endif
}

void TOpenPrefsCommand::MoveToTrash(const FSSpec &spec)
{
	CInfoPBRec pb;
	pb.dirInfo.ioFDirIndex = 0;
	pb.dirInfo.ioDrDirID = spec.parID;
	pb.dirInfo.ioVRefNum = spec.vRefNum;
	pb.dirInfo.ioNamePtr = spec.name;
	PBGetCatInfoSync(&pb);
	if (pb.dirInfo.ioResult != noErr) // did not exist
		return;
	FSSpec trashSpec;
	GetNuntiusTrashFolder(trashSpec);
	FailOSErr(FSpCatMove(spec, trashSpec));
}

void TOpenPrefsCommand::MovePrefFileToTrash(const CStr255 &name)
{
#if qDebugPrefs
	fprintf(stderr, "Moves file '%s' to trash\n", (char*)name);
#endif
	FSSpec spec;
	FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, spec.vRefNum, spec.parID));
	CopyCString2String(name, spec.name);
	MoveToTrash(spec);
}

void TOpenPrefsCommand::MoveFileToPrivate(const CStr255 &name)
{
	FSSpec fromSpec;
	FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, fromSpec.vRefNum, fromSpec.parID));
	CopyCString2String(name, fromSpec.name);
	if (!FileExist(fromSpec))
		return;
#if qDebugPrefs
	fprintf(stderr, "Moves file '%s' to private files\n", (char*)name);
#endif
	FSSpec destSpec;
	FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, destSpec.vRefNum, destSpec.parID));
	CStr255 folderName;
	MyGetIndString(folderName, kPrivateFilesFolderName);
	if (UsesFakeNntp())
		folderName += " (DB)";
	CopyCString2String(folderName, destSpec.name);
	FailOSErr(FSpCatMove(fromSpec, destSpec));
}

void TOpenPrefsCommand::MoveGroupLists()
{
	short vRefNum; long dirID;
	FailOSErr(FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, vRefNum, dirID));
	CStr255 name("");
	HParamBlockRec pbf;
	pbf.fileParam.ioFDirIndex = 1;
	while (true)
	{
		pbf.fileParam.ioVRefNum = vRefNum;
		pbf.fileParam.ioDirID = dirID;
		pbf.fileParam.ioNamePtr = (StringPtr)name;
		pbf.fileParam.ioFVersNum = 0;
		if (PBHGetFInfoSync(&pbf) != noErr)
			break;
		pbf.fileParam.ioFDirIndex++;
		if (pbf.fileParam.ioFlFndrInfo.fdType != kGroupListDocFileType)
			continue;
#if qDebugPrefs
		fprintf(stderr, "Moves the grouplist '%s' to private files\n", (char*)name);
#endif
		pbf.fileParam.ioFDirIndex = 1;
		MoveFileToPrivate(name);
	}
}

void TOpenPrefsCommand::CheckPreFinalVersion()
{
	VersRecHndl vH = nil;
	vH = VersRecHndl(GetResource('vers', 1));
	FailNILResource(Handle(vH));
	FailNIL(vH);
	if ((*vH)->numericVersion.stage == 0x80)
		return;
	unsigned long nowDT;
	long compileDT;
	CStr255 s;
	MyGetIndString(s, kFECompileDateTimeNumber);
	StringToNum(s, compileDT);
	GetDateTime(nowDT);
	if (nowDT - compileDT < 30 * 24 * 60 * 60)
		return;
	StdAlert(phOldPreFinalVersionWarning);
}

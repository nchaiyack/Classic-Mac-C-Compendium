// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupDoc.cp

#include "UGroupDoc.h"
#include "UPrefsDatabase.h"
#include "UDiscListView.h"
#include "UHeaderList.h"
#include "UDiscList.h"
#include "UGroupDocCmds.h"
#include "UNewsAppl.h"
#include "StreamTools.h"
#include "UThread.h"
#include "UBufferedFileStream.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>

#include <UPrinting.h>

#include <ToolUtils.h>

#pragma segment MyGroup

#define qDebugLock qDebug
#define qDebugSave qDebug & 0


const long kCurrentDatabaseVersion = 19;
const long kMinDatabaseVersion = 19;

const long kCurrentArticleStatusVersion = 2;
const long kMinArticleStatusVersion = 1;


TGroupDoc::TGroupDoc()
{
}

pascal void TGroupDoc::Initialize()
{
	inherited::Initialize();
	fSavePrintInfo = false; // couldn't get it to create a print record...
	fDiscList = nil;
	fDiscListWindow = nil;
	fDiscListView = nil;
	fGroupDotName = "(program error)";
	fOldArticleStatus = nil;
	fArticleStatus = nil;
	fSubjectHdr = nil;
	fFromHdr = nil;
	fMsgIDHdr = nil;
	fRefsHdr = nil;
	fSaveArticleStatus = true;
	fSaveDatabase = true;
	fLockLevelAgainstClose = 0;
	fFirstArticleID = 0;
	fLastArticleID = 0;
	fReopenAlert = false;
	fDBFileHandler = nil;
	fArticleStatusIsDirty = false;
	fDatabaseIsDirty = false;
	fIdentifier = kArticleStatusFileType;
}

void TGroupDoc::IGroupDoc(TFile *statFile, TFile *dbFile, const CStr255 &dotName)
{
	inherited::IFileBasedDocument(statFile, kScrapType);
	FailInfo fi;
	if (fi.Try())
	{
		fGroupDotName = dotName;
		fTitle = dotName;

		fDBFileHandler = this->DoMakeFileHandler(dbFile);
		
		TArticleStatus *as;
		as = new TArticleStatus();
		as->IArticleStatus(this);
		fOldArticleStatus = as;
		as = new TArticleStatus();
		as->IArticleStatus(this);
		fArticleStatus = as;
	
		PDiscList *dl = new PDiscList();
		dl->IDiscList();
		fDiscList = dl;

		fSubjectHdr = NewHeaderList();
		fFromHdr = NewHeaderList();
		fMsgIDHdr = NewHeaderList();
		fRefsHdr = NewHeaderList();

		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TGroupDoc::Free()
{
	FreeIfObject(fOldArticleStatus);	fOldArticleStatus = nil;
	FreeIfObject(fArticleStatus);			fArticleStatus = nil;

	delete fDiscList;									fDiscList = nil;
	
	FreeIfObject(fSubjectHdr);				fSubjectHdr = nil;
	FreeIfObject(fFromHdr);						fFromHdr = nil;
	FreeIfObject(fMsgIDHdr);					fMsgIDHdr = nil;
	FreeIfObject(fRefsHdr);						fRefsHdr = nil;

	FreeIfObject(fDBFileHandler);			fDBFileHandler = nil;
	fDiscListWindow = nil; // view
	fDiscListView = nil; // view
	inherited::Free();
}

pascal void TGroupDoc::Close()
{
	if (fLockLevelAgainstClose > 0)
	{
		gApplication->Beep(0);
#if qDebug
		fprintf(stderr, "TGroupDoc::Close called when fLockLevelAgainstClose is %ld\n", fLockLevelAgainstClose);
#endif
		Failure(0, 0); // cannot close now, should probably not be silent
	}
	if (fDiscListView)
		fDiscListView->CloseDown();
	{
		CWindowIterator iter(this, 2, kMaxLong, false);
		for (TWindow *aWindow = iter.FirstWindow(); iter.More(); aWindow = iter.NextWindow())
		{	
			if (!aWindow->fFloats && aWindow != fDiscListWindow)
				aWindow->CloseAndFree();
		}
	}
	SetChangeCount(0); // so we can close if save fails
	SaveDocument(cClose);
	inherited::Close();
}

pascal void TGroupDoc::DoRead(TFile *aFile, Boolean /* forPrinting */)
{
	TStream *aStream = NewBufferedFileStream(aFile, 16 * 1024, 0);
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		if (aStream->GetSize() >= 4)
		{
			OSType fileType;
			FailOSErr(aFile->GetFileType(fileType));
			switch (fileType)
			{
				case kArticleStatusFileType:
					DoReadArticleStatus(aStream);
					break;
					
				case kGroupDBFileType:
					DoReadDatabase(aStream);
					break;
				
				default:
					Failure(minErr, 0);
			}
		}
		aStream->Free(); aStream = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fi.ReSignal();
	}
}

pascal void TGroupDoc::DoWrite(TFile *aFile, Boolean /* forPrinting */)
{
	FailOSErr(aFile->SetDataMark(0, fsFromStart)); 
	TStream *aStream = NewBufferedFileStream(aFile, 0, 16 * 1024);
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		OSType fileType;
		FailOSErr(aFile->GetFileType(fileType));
		switch (fileType)
		{
			case kArticleStatusFileType:
				DoWriteArticleStatus(aStream);
				break;
				
			case kGroupDBFileType:
				DoWriteDatabase(aStream);
				break;
			
			default:
				Failure(minErr, 0);
		}
		aStream->Free(); aStream = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fi.ReSignal();
	}
}

pascal void TGroupDoc::DoNeedDiskSpace(TFile *itsFile, long &dataForkBytes,  long &rsrcForkBytes)
{
	if (itsFile == fDBFileHandler->GetFile())
		DatabaseDoNeedDiskSpace(itsFile, dataForkBytes, rsrcForkBytes);
	else
		ArticleStatusDoNeedDiskSpace(itsFile, dataForkBytes, rsrcForkBytes);
}

//........................................................................
void TGroupDoc::DoReadDatabase(TStream *aStream)
{
	long version = aStream->ReadLong();
	if (!MyCheckVersion(version, kMinDatabaseVersion, kCurrentDatabaseVersion, "TGroupDoc"))
		return;
	gCurThread->CheckYield();
	fDiscList->DoRead(aStream);
	gCurThread->CheckYield();
	fSubjectHdr->DoRead(aStream);
	gCurThread->CheckYield();
	fFromHdr->DoRead(aStream);	
	gCurThread->CheckYield();
	fFirstArticleID = aStream->ReadLong();
	fLastArticleID = aStream->ReadLong();
}

void TGroupDoc::DoWriteDatabase(TStream *aStream)
{
	gCurThread->CheckYield();
	aStream->WriteLong(kCurrentDatabaseVersion);
	fDiscList->DoWrite(aStream);
	gCurThread->CheckYield();
	fSubjectHdr->DoWrite(aStream);
	gCurThread->CheckYield();
	fFromHdr->DoWrite(aStream);
	gCurThread->CheckYield();
	aStream->WriteLong(fFirstArticleID);
	aStream->WriteLong(fLastArticleID);
}

void TGroupDoc::DatabaseDoNeedDiskSpace(TFile *itsFile, long &dataForkBytes,  long &rsrcForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += fDiscList->NeededDiskSpace();
	fSubjectHdr->DoNeedDiskSpace(dataForkBytes);
	fFromHdr->DoNeedDiskSpace(dataForkBytes);
	dataForkBytes += 2 * sizeof(long); // fFirstArticleID, fLastArticleID
	inherited::DoNeedDiskSpace(itsFile, dataForkBytes, rsrcForkBytes);
}

//........................................................................

void TGroupDoc::DoReadArticleStatus(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinArticleStatusVersion, kCurrentArticleStatusVersion, "TGroupDoc/ArticleStatus");
	if (version == 1)
	{
		aStream->ReadLong(); // skip old IP# check
		fOldArticleStatus->DoIronAgeFormatRead(aStream);
	}
	else
	{
		fOldArticleStatus->DoRead(aStream);
	}
	fArticleStatus->SpecifyWithArticleStatus(fOldArticleStatus);
}

void TGroupDoc::DoWriteArticleStatus(TStream *aStream)
{
	aStream->WriteLong(kCurrentArticleStatusVersion);
	fArticleStatus->DoWrite(aStream);
}

void TGroupDoc::ArticleStatusDoNeedDiskSpace(TFile * /* itsFile */, long &dataForkBytes,  long & /* rsrcForkBytes */)
{
	dataForkBytes += sizeof(long); // version
	fArticleStatus->DoNeedDiskSpace(dataForkBytes);
}

//........................................................................

void TGroupDoc::DoPostUpdate(Boolean updateOnly)
{
	if (!updateOnly)
	{
	}
}

pascal TFile *TGroupDoc::DoMakeFile(CommandNumber itsCommandNumber)
{
	switch (itsCommandNumber)
	{
		case cSaveArticleStatus:
			return NewFile(kArticleStatusFileType, kSignature, kUsesDataFork, noResourceFork, kDataOpen, !kRsrcOpen);

		case cSaveGroupDB:
			return NewFile(kGroupDBFileType, kSignature, kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		
		default:
			FailOSErr(errNotImplemented);
	}
}

pascal void TGroupDoc::ReadDocument(Boolean forPrinting)
{
	fFileHandler->ReadFile(forPrinting);
	fDBFileHandler->ReadFile(forPrinting);
}

pascal void TGroupDoc::SaveDocument(CommandNumber /* itsCommandNumber */)
{
	CommitLastCommand();
	// if it has not been saved, SaveDocument asks for a filename
	// so just save it without checks first time!
#if qDebugSave
	fprintf(stderr, "TGroupDoc::SaveDoc, fileExists: AS: %hd, DB: %hd\n", fFileHandler->FileExists(), fDBFileHandler->FileExists());
#endif

	if (fFirstArticleID > fLastArticleID)
	{
		fArticleStatusIsDirty = false; // don't save them...
		fDatabaseIsDirty = false;
		if (fFileHandler->FileExists())
		{
#if qDebug
			fprintf(stderr, "TGroupDoc::SaveDocument, no articles, deletes AS\n");
#endif
			fFileHandler->GetFile()->CloseFile();
			fFileHandler->GetFile()->DeleteFile();
			fFileHandler->fFileExists = false;
		}
		if (fDBFileHandler->FileExists())
		{
#if qDebug
			fprintf(stderr, "TGroupDoc::SaveDocument, no articles, deletes DB\n");
#endif
			fDBFileHandler->GetFile()->CloseFile();
			fDBFileHandler->GetFile()->DeleteFile();
			fDBFileHandler->fFileExists = false;
		}
	}

	if (fSaveArticleStatus && fArticleStatusIsDirty)
	{
#if qDebugSave
		fprintf(stderr, "Saves article status as it was dirty\n");
#endif
		fArticleStatusIsDirty = false;
		if (fFileHandler->FileExists())
		{
			CheckFile(kIDBuzzString, bzSaveAnyways, FALSE);
			fFileHandler->SaveFile(cSaveArticleStatus, false, true, false);
		}
		else
			fFileHandler->SaveFile(cSaveArticleStatus, false, false, false);
	}
	
	if (fSaveDatabase && fDatabaseIsDirty)
	{
#if qDebugSave
		fprintf(stderr, "Saves group database as it was dirty\n");
#endif
		fDatabaseIsDirty = false;
		if (fDBFileHandler->FileExists())
		{
			CheckDBFile(kIDBuzzString, bzSaveAnyways, FALSE);
			fDBFileHandler->SaveFile(cSaveGroupDB, false, true, false);
		}
		else
			fDBFileHandler->SaveFile(cSaveGroupDB, false, false, false);
	}
	SetChangeCount(0);
}	

pascal void TGroupDoc::Changed(ChangeID theChange, TObject* changedBy)
{
	switch (theChange)
	{
		case cArticleStatusChange:
			fArticleStatusIsDirty = true;
			break;
		
		case cDatabaseUpdated:
			fDatabaseIsDirty = true;
			break;
		
		default:
			break;
	}
	inherited::Changed(theChange, changedBy);
}

void TGroupDoc::CheckDBFile(ResNumber rsrcId, short rsrcIndex, Boolean reverting)
{
	OSErr err = fDBFileHandler->FileChanged(reverting);	// don't care about the file type if saving 
	if (err == errFileChanged)
	{
		CStr255 name = fTitle;
		CStr255 s;
		GetIndString(s, rsrcId, rsrcIndex);
		ParamText(name, s, gEmptyString, gEmptyString);
		if (MacAppAlert(phFileChanged, NULL) == cancel)// !!! This should be programatically defeatable 
			Failure(noErr, messageCancelled);
	}
	else if ((err != noErr) && reverting)
		Failure(err, 0);
}

pascal void TGroupDoc::DoInitialState()
{
	Failure(errNotImplemented, messageNewFailed);
}

pascal void TGroupDoc::DoMakeViews(Boolean forPrinting)
{
	if (forPrinting)
		return;
	fDiscListWindow = gViewServer->NewTemplateWindow(kDiscListView, this);
	if (!fDiscListWindow)
	{
#if qDebug
		ProgramBreak("Could not create window");
#endif
		FailNIL(fDiscListWindow);
	}
	fDiscListView = (TDiscListView*) fDiscListWindow->FindSubView(kDiscListViewID);
#if qDebug
	if (!IsObject(fDiscListView))
		DebugStr(CStr255("UPS: fDiscListView IS NOT OBJECT"));
#endif
	FailNIL(fDiscListView);

	TStdPrintHandler *aHandler = new TStdPrintHandler;
	aHandler->IStdPrintHandler(this, fDiscListView, !kSquareDots, kFixedSize, !kFixedSize);

	CRect maxResize(fDiscListWindow->fResizeLimits);
	maxResize.right = 650; //@@ hack
	fDiscListWindow->SetResizeLimits(maxResize[topLeft], maxResize[botRight]);
	Lock(true);
	fDiscListWindow->SetTitle(fGroupDotName);
	Lock(false);
	gPrefs->GetSilentWindowPosPrefs('WDis', fDiscListWindow);
	fDiscListView->Startup(fGroupDotName);
}

pascal void TGroupDoc::FileHasBeenSaved(const CStr255& /* newName */)
{
	// No way. May not change the title when saving
}

TArticleStatus *TGroupDoc::GetArticleStatus()
{
	return fArticleStatus;
}

TArticleStatus *TGroupDoc::GetOldArticleStatus()
{
	return fOldArticleStatus;
}

PDiscList *TGroupDoc::GetDiscList()
{
	return fDiscList;
}

TDiscListView *TGroupDoc::GetDiscListView()
{
	return fDiscListView;
}

void TGroupDoc::GetGroupDotName(CStr255 &name)
{
	name = fGroupDotName;
}

Boolean TGroupDoc::GetFrom(long id, HandleOffsetLength &hol)
{
	if (id < fFirstArticleID || id > fLastArticleID)
		return false;
	fFromHdr->AtGet(id, hol);
	return true;
}

Boolean TGroupDoc::GetSubject(long id, HandleOffsetLength &hol)
{
	if (id < fFirstArticleID || id > fLastArticleID)
		return false;
	fSubjectHdr->AtGet(id, hol);
	return true;
}

long TGroupDoc::GetFirstArticleID()
{
	return fFirstArticleID;
}

long TGroupDoc::GetLastArticleID()
{
	return fLastArticleID;
}

void TGroupDoc::IncLockAgainstClose()
{
#if qDebugLock
	fprintf(stderr, "TGroupDoc::IncLockAgainstClose, level %ld -> %ld\n", fLockLevelAgainstClose, fLockLevelAgainstClose + 1);
#endif
	fLockLevelAgainstClose++;
}

void TGroupDoc::DecLockAgainstClose()
{
#if qDebugLock
	fprintf(stderr, "TGroupDoc::DecLockAgainstClose, level %ld -> %ld\n", fLockLevelAgainstClose, fLockLevelAgainstClose - 1);
#endif
#if qDebug
	if (fLockLevelAgainstClose == 0)
	{
		ProgramBreak("wanted to unlock and fLockLevelAgainstClose is zero");
		return;
	}
#endif
	fLockLevelAgainstClose--;
}

pascal void TGroupDoc::DoSetupMenus()
{
	inherited::DoSetupMenus();
}

pascal void TGroupDoc::DoMenuCommand(CommandNumber aCommandNumber)
{
	switch (aCommandNumber)
	{
		default: 
			inherited::DoMenuCommand(aCommandNumber);
	}
}

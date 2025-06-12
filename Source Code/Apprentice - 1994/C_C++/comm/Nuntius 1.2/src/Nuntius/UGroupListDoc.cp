// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupListDoc.cp

#include "UGroupListDoc.h"
#include "UGroupListView.h"
#include "UGroupList.h"
#include "UGroupListCmds.h"
#include "UPrefsDatabase.h"
#include "Tools.h"
#include "UFatalError.h"
#include "StreamTools.h"
#include "UBufferedFileStream.h"
#include "UNewsAppl.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <UPrinting.h>

#include <ToolUtils.h>
#include <Packages.h>

#pragma segment MyGroupList

const long kCurrentGroupListFileVersion = 3;
const long kMinGroupListFileVersion = 1;

TGroupListDoc::TGroupListDoc()
{
}

pascal void TGroupListDoc::Initialize()
{
	inherited::Initialize();
//	fSavePrintInfo = false; // couldn't get it to create a print record...
	fGroupList = nil;
	fGroupListView = nil;
	fGroupListWindow = nil;
	fIdentifier = kGroupListDocFileType;
	fWindowFrame = VRect(0, 0, 0, 0);
	fWasOldFileVersion = false;
	fPeriodicCheck = nil;
}

void TGroupListDoc::IGroupListDoc(TFile *itsFile)
{
	inherited::IFileBasedDocument(itsFile, kGroupListDocFileType);
	FailInfo fi;
	if (fi.Try())
	{
		TGroupList *gl = new TGroupList();
		gl->IGroupList(this);
		fGroupList = gl;
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TGroupListDoc::DoPostMakeViews(Boolean forPrinting)
{
	inherited::DoPostMakeViews(forPrinting);
	TPeriodicCheckNewArticles *pc = new TPeriodicCheckNewArticles();
	pc->IPeriodicCheckNewArticles(this);
	fPeriodicCheck = pc;
	fPeriodicCheck->SetSleep(gPrefs->GetLongPrefs('CNAT') * 60 * 60);
}

pascal void TGroupListDoc::DoInitialState()
{
	inherited::DoInitialState();
	fGroupList->DeleteAll();
	SetChangeCount(0);
}

pascal void TGroupListDoc::Free()
{
	if (fPeriodicCheck)
	{
#if qDebug
		fprintf(stderr, "Closing fPeriodicCheck\n");
		VerboseIsObject(fPeriodicCheck);
#endif
		fPeriodicCheck->Close();
		fPeriodicCheck->Free(); fPeriodicCheck = nil;
	}
	if (fPeriodicCheck)
		PanicExitToShell("In TGroupListDoc::Free, had fPeriodicCheck");
	FreeIfObject(fGroupList); fGroupList = nil;
	fGroupListView = nil; // view
	inherited::Free();
}

pascal void TGroupListDoc::RevertDocument()
{
	inherited::RevertDocument();
	if (fPeriodicCheck)
		fPeriodicCheck->WakeUp();
}

pascal void TGroupListDoc::Close()
{
	long changeCount = this->GetChangeCount();
	short poseResult;
	if (changeCount)
	{
		poseResult = PoseSaveDialog();
		if (poseResult == cancel)
			Failure(noErr, messageCancelled);
	}

	this->Changed(mClosed,this);

	long oldModDate = 0;
	long oldCreateDate = 0;
	
	Boolean onlyWindowPos = false;
	if (fGroupListWindow && !changeCount && fWindowFrame != VRect(0, 0, 0, 0))
	{
		VRect fr;
		fGroupListWindow->GetFrame(fr);
		if (fr != fWindowFrame)
		{
			changeCount++;
			onlyWindowPos = true;
			poseResult = kYesButton;
			TFile *file = fFileHandler->GetFile();
			if (file)
			{
				oldModDate = file->GetModificationDate();
				oldCreateDate = file->GetCreationDate();
			}
		}
	}
	if (fPeriodicCheck)
		fPeriodicCheck->Close();
	if (changeCount)
	{
		if (poseResult == kYesButton)
			SaveDocument(cClose);				// Will fail if unable to save 
		else if (poseResult == kNoButton)
			Abandon();
	}
	if (changeCount && onlyWindowPos && oldModDate && oldCreateDate)
	{
		TFile *file = fFileHandler->GetFile();
		if (file)
		{
			file->SetModificationDate(oldModDate);
			file->SetCreationDate(oldCreateDate);
		}
	}
	// Must never be called for a document related to a view in the Clipboard.
	CWindowIterator iter(this);

	for (TWindow* aWindow = iter.FirstWindow(); iter.More(); aWindow = iter.NextWindow())
		aWindow->CloseAndFree();
}

pascal void TGroupListDoc::ReadDocument(Boolean forPrinting)
{
	inherited::ReadDocument(forPrinting);
	if (fWasOldFileVersion)
		Changed(cGenericCommand, this);
}

pascal void TGroupListDoc::DoRead(TFile *aFile, Boolean /* forPrinting */)
{
	FailOSErr(aFile->SetDataMark(0, fsFromStart)); 
	TStream *aStream = NewBufferedFileStream(aFile, 16 * 1024, 0);
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		DoReadFile(aStream);
		aStream->Free(); aStream = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fi.ReSignal();
	}
}

void TGroupListDoc::DoReadFile(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinGroupListFileVersion, kCurrentGroupListFileVersion, "TGroupListDoc");
	fWasOldFileVersion = version < kCurrentGroupListFileVersion;
	if (version == 1)
		fGroupList->DoIronAgeFormatRead(aStream);
	else
		fGroupList->DoRead(aStream);
	if (version >= 2)
	{
		VRect vr;
		aStream->ReadVRect(vr);
		fWindowFrame = vr;
		if (fGroupListWindow)
		{
			fGroupListWindow->SetFrame(vr, kRedraw);
			fGroupListWindow->ForceOnScreen();
		}
	}
	if (version >= 3)
	{
		if (aStream->ReadBoolean())
		{
			if (!fPrintInfo)
				fPrintInfo = NewPermHandle(0);
			MyStreamReadHandle(aStream, fPrintInfo);
		}
	}
}

pascal void TGroupListDoc::DoWrite(TFile *aFile, Boolean /* forPrinting */)
{
	gNewsAppl->SetNeedCheckFolderMenus();
	FailOSErr(aFile->SetDataMark(0, fsFromStart)); 
	TStream *aStream = NewBufferedFileStream(aFile, 0, 16 * 1024);
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		DoWriteFile(aStream);
		aStream->Free(); aStream = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fi.ReSignal();
	}
}

void TGroupListDoc::DoWriteFile(TStream *aStream)
{
	aStream->WriteLong(kCurrentGroupListFileVersion);
	fGroupList->DoWrite(aStream);
	VRect vr;
	fGroupListWindow->GetFrame(vr);
	aStream->WriteVRect(vr);
	if (fPrintInfo)
	{
		aStream->WriteBoolean(true);
		MyStreamWriteHandle(aStream, fPrintInfo);
	}
	else
		aStream->WriteBoolean(false);
}

pascal void TGroupListDoc::DoNeedDiskSpace(TFile *itsFile, long &dataForkBytes,  long &rsrcForkBytes)
{
	dataForkBytes += 4 * sizeof(long);
	inherited::DoNeedDiskSpace(itsFile, dataForkBytes, rsrcForkBytes);
	fGroupList->DoNeedDiskSpace(dataForkBytes);
}

pascal TFile* TGroupListDoc::DoMakeFile(CommandNumber /* itsCommandNumber */)
{
	return gApplication->DoMakeFile(cNewGroupListDoc);
}

pascal void TGroupListDoc::DoMakeViews(Boolean /* forPrinting */)
{
	fGroupListWindow = gViewServer->NewTemplateWindow(kGroupListView, this);
	FailNIL(fGroupListWindow);
	fGroupListView = (TGroupListView*) fGroupListWindow->FindSubView(kGroupListViewID);
#if qDebug
	if (!IsObject(fGroupListView))
		ProgramBreak("Missing grouplistView");
#endif
	TStdPrintHandler *aHandler = new TStdPrintHandler;
	aHandler->IStdPrintHandler(this, fGroupListView, !kSquareDots, kFixedSize, !kFixedSize);

	CRect maxResize(fGroupListWindow->fResizeLimits);
	maxResize.right = 250; //@@ hack
	fGroupListWindow->SetResizeLimits(maxResize[topLeft], maxResize[botRight]);

	if (fWindowFrame != VRect(0, 0, 0, 0))
	{
		VRect vr(fWindowFrame);
		fGroupListWindow->SetFrame(vr, kRedraw);
		fGroupListWindow->fStaggered = true;
	}
	fGroupListView->UpdateList();
	fGroupListWindow->Open();
}

pascal void TGroupListDoc::CloseWindow(TWindow* aWindow)
{
	inherited::CloseWindow(aWindow);
}

AliasHandle TGroupListDoc::GetAlias()
{
	AliasHandle ah;
	FailOSErr(fFileHandler->GetFile()->GetAlias(ah));
	return ah;
}

pascal void TGroupListDoc::Changed(ChangeID theChange, TObject* changedBy)
{
	inherited::Changed(theChange, changedBy);
	if (theChange == cGroupListChange && fPeriodicCheck)
		CheckForNewArticlesNow();
}

Boolean TGroupListDoc::IsCheckingForNewArticlesNow()
{
	return fPeriodicCheck->IsBusy();
}

void TGroupListDoc::CheckForNewArticlesNow()
{
	fPeriodicCheck->WakeUp();
}

pascal void TGroupListDoc::UntitledName(CStr255& noName)
{
	short preInsert;
	short constChars;
	CStr255 num;

	MyGetIndString(noName, kGroupListUntitledName);
	if (ParseTitleTemplate(noName, preInsert, constChars))
	{
		NumToString(gNumUntitled, num);

		if (SubstituteInTitle(noName, num, preInsert, constChars))
			++gNumUntitled;
	}
}


pascal void TGroupListDoc::DoMenuCommand(CommandNumber aCommandNumber)
{
	inherited::DoMenuCommand(aCommandNumber);
}

pascal void TGroupListDoc::DoSetupMenus()
{
	inherited::DoSetupMenus();
}

pascal void TGroupListDoc::OpenAgain(CommandNumber /* itsCommandNumber */, TDocument * /* openingDoc */)
{
	if (fWindowList)
	{
		TWindow *window = (TWindow *)fWindowList->First();
		if (window)
			window->Select();
	}
}

TGroupList *TGroupListDoc::GetGroupList()
{
	return fGroupList;
}

TGroupListView *TGroupListDoc::GetGroupListView()
{
	return fGroupListView;
}

void TGroupListDoc::SpecifyList(TGroupList *groupList)
{
	CopyDynamicArray(groupList, fGroupList);
	if (fGroupListView)
	{
		fGroupListView->Focus();
		fGroupListView->ForceRedraw();
	}
}

void TGroupListDoc::UpdateGroupStatus(const CStr255 &dotName, Boolean hasNewArticles)
{
	fGroupListView->UpdateGroupStatus(dotName, hasNewArticles);
}

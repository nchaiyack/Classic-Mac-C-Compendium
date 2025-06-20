// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTreeDoc.cp

#include "UGroupTreeDoc.h"
#include "UGroupTreeView.h"
#include "UGroupTree.h"
#include "UGroupListDoc.h"
#include "UGroupList.h"
#include "UNewsAppl.h"
#include "UNntp.h"
#include "UProgress.h"
#include "UPrefsFileMgr.h"
#include "FileTools.h"
#include "Tools.h"
#include "StreamTools.h"
#include "NetAsciiTools.h"
#include "UBufferedFileStream.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <UPrinting.h>

#pragma segment MyGroupList

const long kCurrentGroupTreeDocVersion = 6;
const long kMinGroupTreeDocVersion = 6;

TGroupTreeDoc::TGroupTreeDoc()
{
}

pascal void TGroupTreeDoc::Initialize()
{
	inherited::Initialize();
	fGroupTree = nil;
	fGroupTreeViewList = nil;
	fLastUpdateDate = 0;
	fIdentifier = kGroupTreeDocFileType;
}

void TGroupTreeDoc::IGroupTreeDoc(TFile *itsFile)
{
	inherited::IFileBasedDocument(itsFile, kGroupTreeDocFileType);
	FailInfo fi;
	if (fi.Try())
	{
		TGroupTree *gt = new TGroupTree();
		gt->IGroupTree();
		fGroupTree = gt;
		fGroupTree->DoInitialState();
		
		fGroupTreeViewList = NewList();
		
//	TStdPrintHandler *aHandler = new TStdPrintHandler;
//		aHandler->IStdPrintHandler(this, nil, !kSquareDots, kFixedSize, !kFixedSize);
		fSavePrintInfo = false;
		
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TGroupTreeDoc::Free()
{
	FreeIfObject(fGroupTree); fGroupTree = nil;
#if qDebug
	if (fGroupTreeViewList->GetSize())
		ProgramBreak("fGroupTreeViewList is not empty when free'ing TGroupTreeDoc");
#endif
	FreeIfObject(fGroupTreeViewList); fGroupTreeViewList = nil;
	inherited::Free();
}

pascal void TGroupTreeDoc::Close()
{
	SaveTheTree();
	inherited::Close();
}

void TGroupTreeDoc::SaveTheTree()
{
	if (!GetChangeCount())
		return;
	if (fFileHandler->FileExists())
		SaveDocument(cSave); // if it has not been saved, SaveDocument asks for a filename
	else
	{
		// so just save it without checks first time!
		fFileHandler->SaveFile(cSave, false, true, false);
	}
}

pascal void TGroupTreeDoc::DoPostMakeViews(Boolean forPrinting)
{
	gNewsAppl->SetGroupTreeDoc(this);
	inherited::DoPostMakeViews(forPrinting);
}

void TGroupTreeDoc::ForgetGroupTreeView(TGroupTreeView *view)
{
	fGroupTreeViewList->Delete(view);
}

pascal void TGroupTreeDoc::DoInitialState()
{
	CloseAllWindows();
	fGroupTree->DoInitialState();
	inherited::DoInitialState();
	SetChangeCount(0);
}

pascal void TGroupTreeDoc::DoRead(TFile *aFile, Boolean forPrinting)
{
	FailOSErr(aFile->SetDataMark(0, fsFromStart)); 
	TStream *aStream = NewBufferedFileStream(aFile, 150 * 1024, 0);
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		DoReadFile(aStream);
		aStream->Free(); aStream = nil;
		inherited::DoRead(aFile, forPrinting);
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		fi.ReSignal();
	}
}

void TGroupTreeDoc::DoReadFile(TStream *aStream)
{
	long version = aStream->ReadLong();
	if (!MyCheckVersion(version, kMinGroupTreeDocVersion, kCurrentGroupTreeDocVersion, "TGroupTreeDoc"))
		return;
	fLastUpdateDate = aStream->ReadLong();
	fGroupTree->DoRead(aStream);
}

pascal void TGroupTreeDoc::DoWrite(TFile *aFile, Boolean forPrinting)
{
	FailOSErr(aFile->SetDataMark(0, fsFromStart)); 
	TStream *aStream = NewBufferedFileStream(aFile, 0, 150 * 1024);
	VOLATILE(aStream);
	FailInfo fi;
	if (fi.Try())
	{
		DoWriteFile(aStream);
		aStream->Free(); aStream = nil;
		inherited::DoWrite(aFile, forPrinting);
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(aStream); aStream = nil;
		aFile->SetDataLength(0);
		FailNewMessage(fi.error, fi.message, 0); //@@
	}
}

void TGroupTreeDoc::DoWriteFile(TStream *aStream)
{
#if qDebug
	fprintf(stderr, "TGroupTreeDoc is saved with fLastUpdateDate = %ld\n", fLastUpdateDate);
#endif
	aStream->WriteLong(kCurrentGroupTreeDocVersion);
	aStream->WriteLong(fLastUpdateDate);
	fGroupTree->DoWrite(aStream);
}

pascal void TGroupTreeDoc::DoNeedDiskSpace(TFile *itsFile, long &dataForkBytes, long &rsrcForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += sizeof(long); // fLastUpdateDate
	fGroupTree->DoNeedDiskSpace(dataForkBytes);
	inherited::DoNeedDiskSpace(itsFile, dataForkBytes, rsrcForkBytes);
}

void TGroupTreeDoc::WriteTreeInfo(TStream *aStream)
{
#if qDebug
		fprintf(stderr, "Saved fLastUpdateDate == $%lx\n", fLastUpdateDate);
#endif
	aStream->WriteLong(fLastUpdateDate);
	fGroupTree->WriteTreeInfo(aStream);
}

void TGroupTreeDoc::ReadTreeInfo(TStream *aStream)
{
	unsigned long modDate = (unsigned long)aStream->ReadLong();
	if (modDate != fLastUpdateDate)
	{
#if qDebug
		fprintf(stderr, "TGroupTreeDoc::ReadTreeInfo, does not read info as mod-date is different:\n");
		fprintf(stderr, "-   DiskTree == $%lx   !=   Prefs == $%lx\n", fLastUpdateDate, modDate);
#endif
		Failure(0, 0); // tree have been modified, can't read saved info
	}
	fGroupTree->ReadTreeInfo(aStream);
}

void TGroupTreeDoc::WriteWindowInfo(TStream *aStream, TWindow *window)
{
	TGroupTreeView *gtv = (TGroupTreeView *)window->FindSubView(kGroupTreeViewID);
#if qDebug
	if (!IsObject(gtv))
		ProgramBreak("Is missing TGroupTreeView");
#endif
	gtv->WriteWindowInfo(aStream);
}

void TGroupTreeDoc::ReadWindowInfo(TStream *aStream)
{
	TWindow *window = gViewServer->NewTemplateWindow(kGroupTreeView, this);
	TGroupTreeView *gtv = (TGroupTreeView*) window->FindSubView(kGroupTreeViewID);
#if qDebug
	if (!IsObject(window) || !IsObject(gtv))
		ProgramBreak("Missing GroupTreeWindow or -view");
#endif
	gtv->ReadWindowInfo(aStream);
	fGroupTreeViewList->Insert(gtv);
	TStdPrintHandler *aHandler = new TStdPrintHandler;
	aHandler->IStdPrintHandler(this, gtv, !kSquareDots, kFixedSize, !kFixedSize);

	CRect maxResize(window->fResizeLimits);
	maxResize.right = 300; //@@ hack
	window->SetResizeLimits(maxResize[topLeft], maxResize[botRight]);

	gtv->UpdateList();
	window->Open();
}

pascal void TGroupTreeDoc::UntitledName(CStr255& noName)
{
	GetPrefsLocationFileUntitledName(noName, kGroupTreeFileName);
}

pascal void TGroupTreeDoc::DoMakeViews(Boolean /* forPrinting */)
{
}

pascal void TGroupTreeDoc::OpenAgain(CommandNumber itsCommandNumber, TDocument *openingDoc)
{
	inherited::OpenAgain(itsCommandNumber, openingDoc);
}

pascal TFile *TGroupTreeDoc::DoMakeFile(CommandNumber /* itsCommandNumber */)
{
	return gNewsAppl->DoMakeFile(cNewGroupTreeDoc);
}

void TGroupTreeDoc::CreateGroupTreeWindow(ArrayIndex windowFolderIndex, TGroupTreeView *openFromView)
{
	if (!windowFolderIndex)
		windowFolderIndex = kFirstIndex;
#if qDebug
	if (!IsObject(this))
		ProgramBreak("this is not object");
	if (!IsObject(fGroupTreeViewList))
		ProgramBreak("fGroupTreeViewList is not object");
#endif
// test if already open
	{
		CObjectIterator iter(fGroupTreeViewList);
		for (TObject *obj = iter.FirstObject(); iter.More(); obj = iter.NextObject())
		{
			TGroupTreeView *gtv = (TGroupTreeView*) obj;
			if (gtv->GetWindowFolderIndex() == windowFolderIndex)
			{
				gtv->GetWindow()->Select();
				gtv->GetWindow()->Open();
				return;
			}
		}
	}
	
// not open, create new
	TWindow *window = gViewServer->NewTemplateWindow(kGroupTreeView, this);
	TGroupTreeView *gtv = (TGroupTreeView*) window->FindSubView(kGroupTreeViewID);
#if qDebug
	if (!IsObject(window) || !IsObject(gtv))
		ProgramBreak("Missing GroupTreeWindow or -view");
#endif
	fGroupTreeViewList->Insert(gtv);
	gtv->SetWindowFolder(windowFolderIndex, openFromView);
	TStdPrintHandler *aHandler = new TStdPrintHandler;
	aHandler->IStdPrintHandler(this, gtv, !kSquareDots, kFixedSize, !kFixedSize);

	CRect maxResize(window->fResizeLimits);
	maxResize.right = 300; //@@ hack
	window->SetResizeLimits(maxResize[topLeft], maxResize[botRight]);

	gtv->UpdateList();
	window->Open();
}

pascal void TGroupTreeDoc::CloseWindow(TWindow* aWindow)
{
	if (aWindow && (aWindow->fDocument == this))
		aWindow->CloseAndFree();
		// don't close document even if no windows is left open
		// so TDocument::Close can't be used
}

void TGroupTreeDoc::CloseAllWindows()
{
	while (fGroupTreeViewList->GetSize())
	{
		TGroupTreeView *gtv = (TGroupTreeView*) fGroupTreeViewList->Last();
		gtv->GetWindow()->CloseByUser();
	}
}

TGroupTree *TGroupTreeDoc::GetGroupTree()
{
	return fGroupTree;
}

void TGroupTreeDoc::RebuildGroupTree()
{	
	PNntp *nntp = nil;
	VOLATILE(nntp);
	Handle groupData = nil;
	VOLATILE(groupData);
	TGroupTree *tree = nil;
	VOLATILE(tree);
	FailInfo fi;
	if (fi.Try())
	{
		TGroupTree *gt = new TGroupTree();
		gt->IGroupTree();
		tree = gt;
		tree->DoInitialState();

		unsigned long curDate;
		GetDateTime(curDate);
		fLastUpdateDate = curDate;
		
		gCurProgress->SetWorkToDo(kCandyStribes);
		if (!gCurProgress->IsWorking())
			gCurProgress->ClearTexts();
		gCurProgress->StartProgress(true);
		nntp = gNntpCache->GetNntp();

		gCurProgress->SetText(kUpdGTFetchesGroupList);
		groupData = nntp->GetListOfAllGroups();
		gCurProgress->SetText(kUpdGTBuildsTree);
		Changed(cUpdateGroupTree, this);
		tree->UpdateGroupTree(groupData, nil);
		groupData = DisposeIfHandle(groupData);

		FailInfo innerFI;
		if (innerFI.Try())
		{
			gCurProgress->SetText(kUpdGTFetchesGroupDescriptions);
			gCurProgress->SetWorkToDo(kCandyStribes);
			groupData = nntp->GetListOfGroupDesciptions();
			if (GetHandleSize(groupData))
			{
				gCurProgress->SetText(kUpdGTProcessGroupsDescriptions);
				TranslateViaTable(gNetAscii2Mac, *groupData, GetHandleSize(groupData));
				Changed(cUpdateGroupTree, this);
				tree->UpdateDescriptions(groupData);
			}
			innerFI.Success();
		}
		else // fail
		{
			// show show alert???
		}
		groupData = DisposeIfHandle(groupData);
		
		gNntpCache->ReturnNntp(nntp); nntp = nil;
		gCurProgress->SetText(kUpdGTSavesTree);

		CloseAllWindows();
		fGroupTree->Free(); fGroupTree = tree; tree = nil; // replace with new tree
		SaveTheTree();
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(tree); tree = nil;
		gNntpCache->DiscardNntp(nntp); nntp = nil;
		groupData = DisposeIfHandle(groupData);
		FailNewMessage(fi.error, fi.message, messageUpdateGroupListFailed);
	}	
}

void TGroupTreeDoc::UpdateGroupTree()
{	
	unsigned long curDate;
	GetDateTime(curDate);
	PNntp *nntp = nil;
	VOLATILE(nntp);
	Handle groupData = nil;
	VOLATILE(groupData);
	TGroupList *groupList = nil;
	VOLATILE(groupList);
	FailInfo fi;
	if (fi.Try())
	{
		unsigned long oldDate = fLastUpdateDate - 2 * 24 * 60 * 60;
		gCurProgress->SetWorkToDo(kCandyStribes);
		gCurProgress->StartProgress(true);
		nntp = gNntpCache->GetNntp();

		gCurProgress->SetText(kUpdGTFetchesListOfNewGroups);
		groupData = nntp->GetListOfNewGroups(oldDate);

		TGroupList *gl = new TGroupList();
		gl->IGroupList(nil);
		groupList = gl;
				
		if (GetHandleSize(groupData) > 1)
		{
			gCurProgress->SetText(kUpdGTUpdatesTree);
			CloseAllWindows();

			fGroupTree->UpdateGroupTree(groupData, groupList);
		}
		groupData = DisposeIfHandle(groupData);
		
		gNntpCache->ReturnNntp(nntp); nntp = nil;
		fi.Success();
	}
	else // fail
	{
		gNntpCache->DiscardNntp(nntp); nntp = nil;
		groupData = DisposeIfHandle(groupData);
		FreeIfObject(groupList); groupList = nil;
		FailNewMessage(fi.error, fi.message, messageUpdateGroupListFailed);
	}
#if qDebug
	fprintf(stderr, "Found %ld new groups\n", groupList->GetSize());
#endif
	if (!groupList->GetSize())
	{
		groupList->Free();
		gCurProgress->WorkDone();
		return;
	}
	fLastUpdateDate = curDate;
#if qDebug
	fprintf(stderr, "New fLastUpdateDate == $%lx\n", fLastUpdateDate);
#endif
	TGroupListDoc *doc = nil;
	VOLATILE(doc);
	if (fi.Try())
	{
		Changed(cUpdateGroupTree, this);
		gCurProgress->SetText(kUpdGTSavesTree);
		SaveTheTree();
		gCurProgress->WorkDone();

		TGroupListDoc *gld = new TGroupListDoc();
		gld->IGroupListDoc(nil);
		doc = gld;
		doc->DoInitialState();
		doc->SpecifyList(groupList);

		doc->DoMakeViews(kForDisplay);
		CStr255 title;
		MyGetIndString(title, kUpdGTNewGroupsDocName);
		doc->SetTitle(title);

		FailSpaceIsLow();						// Fail if document leaves us with no room 

		// Don't attempt to show the windows until we're sure we won't fail 
		doc->DoPostMakeViews(kForDisplay);

		groupList->Free(); groupList = nil;
		fi.Success();
	}
	else // fail
	{
		FreeIfObject(doc); doc = nil;
		FreeIfObject(groupList); groupList = nil;
		FailNewMessage(fi.error, fi.message, messageUpdateGroupListFailed);
	}
}

pascal void TGroupTreeDoc::DoMenuCommand(CommandNumber aCommandNumber)
{
	inherited::DoMenuCommand(aCommandNumber);
}

pascal void TGroupTreeDoc::DoSetupMenus()
{
	inherited::DoSetupMenus();
}

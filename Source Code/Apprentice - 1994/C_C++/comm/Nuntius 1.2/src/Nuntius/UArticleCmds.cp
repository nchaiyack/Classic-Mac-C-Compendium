// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// TArticleCmds.cp

#include "UArticleCmds.h"
#include "UArticleListView.h"
#include "FileTools.h"
#include "UGroupDoc.h"
#include "UPrefsDatabase.h"
#include <ErrorGlobals.h>

#include <RsrcGlobals.h>

#include <UGridView.h>

#include <ToolUtils.h>

#pragma segment MyArticle


///---------------------------------------------------------------------
TShowDiscJunkCommand::TShowDiscJunkCommand()
{
}

pascal void TShowDiscJunkCommand::Initialize()
{
	inherited::Initialize();
	fArticleListView = nil;
	fIndex = nil;
}

void TShowDiscJunkCommand::IShowDiscJunkCommand(TArticleListView *articleListView, long index)
{
	inherited::ICommand(cShowDiscJunkCommand, nil, false, false, nil);
	fArticleListView = articleListView;
	fIndex = index;
}

pascal void TShowDiscJunkCommand::Free()
{
	fArticleListView = nil;
	fIndex = nil;
	inherited::Free();
}

pascal void TShowDiscJunkCommand::DoIt()
{
	fArticleListView->SetDiscussionDisplayed(fIndex);
}


//---------------------------------------------------------------------
TCopyArticleTextCommand::TCopyArticleTextCommand()
{
}

pascal void TCopyArticleTextCommand::ICopyArticleTextCommand(TArticleListView *articleListView, Boolean asQuote, Boolean addHeader)
{
	inherited::ICommand(cCopy, nil, false, false, nil);
	fArticleListView = articleListView;
	fAsQuote = asQuote;
	fAddHeader = addHeader;
}

pascal void TCopyArticleTextCommand::DoIt()
{
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = fArticleListView->GetSelectionAsTextForClipboard(fAsQuote, fAddHeader);
		ZeroScrap();
		HLock(h);
		FailOSErr(gClipboardMgr->PutDeskScrapData('TEXT', h));
		h = DisposeIfHandle(h);
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}
//---------------------------------------------------------------------
TUpdateArticleViewCommand::TUpdateArticleViewCommand()
{
}

pascal void TUpdateArticleViewCommand::Initialize()
{
	inherited::Initialize();
	fArticleListView = nil;
}

void TUpdateArticleViewCommand::IUpdateArticleViewCommand(TArticleListView *alv, ArticleShowType whatToShow)
{
	inherited::ICommand(cUpdateArticleListCommand, nil, false, false, nil);
	fArticleListView = alv;
	fWhatToShow = whatToShow;
}

pascal void TUpdateArticleViewCommand::Free()
{
	inherited::Free();
}

pascal void TUpdateArticleViewCommand::DoIt()
{
	fArticleListView->DoTheUpdateList(fWhatToShow);
}
//---------------------------------------------------------------------
TStoreNotesCommand::TStoreNotesCommand()
{
}

pascal void TStoreNotesCommand::Initialize()
{
	inherited::Initialize();
	fArticleListView = nil;
	fFile = nil;
	fPBP = nil;
}

void TStoreNotesCommand::IStoreNotesCommand(TArticleListView *alv)
{
	inherited::ICommand(cStoreNotes, nil, false, false, nil);
	fArticleListView = alv;
	fPBP = (ParamBlockRec*)NewPermPtr(sizeof(ParamBlockRec));
}

pascal void TStoreNotesCommand::Free()
{
	FreeIfObject(fFile); fFile = nil; // closes file
	DisposeIfPtr(Ptr(fPBP)); fPBP = nil;
	inherited::Free();
}

pascal void TStoreNotesCommand::DoIt()
{
	FailInfo fi;
	if (fi.Try())
	{
		OpenFile();
		StoreQuote();
		fFile->CloseFile();
		fi.Success();
	}
	else // failure
	{
		FailNewMessage(fi.error, fi.message, messageSaveNotes);
	}
}

void TStoreNotesCommand::OpenFile()
{
	FailInfo fi;
	if (fi.Try())
	{
		if (!gPrefs->PrefExists('FNot'))
			FailOSErr(errNoNoteFolder);
		if (!gPrefs->PrefExists('EDsi'))
			FailOSErr(errNoEditor);
		OSType signature = gPrefs->GetSignaturePrefs('EDsi');
		fFile = NewFile('TEXT', signature,
			kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		fFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		FSSpec spec;
		gPrefs->GetDirAliasPrefs('FNot', spec);
		CStr255 dotName;
		fArticleListView->fDoc->GetGroupDotName(dotName);
		GoGroupFile(dotName, spec);
		fFile->Specify(spec);
		if (!FileExist(fFile))
			FailOSErr(fFile->CreateFile());
		FailOSErr(fFile->OpenFile());
		FailOSErr(fFile->SetDataMark(0, fsFromLEOF));
		fi.Success();
	}
	else // failure
	{
		FailNewMessage(fi.error, fi.message, messageOpenNotesFile);
	}
}

void TStoreNotesCommand::StoreQuote()
{
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = fArticleListView->GetSelectionAsQuoteText(false, true, kNoteQuoteTemplate);
		HLock(h);
		long len = GetHandleSize(h);
		WriteASyncToFile(*fPBP, fFile, *h, len);
		HUnlock(h);
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}

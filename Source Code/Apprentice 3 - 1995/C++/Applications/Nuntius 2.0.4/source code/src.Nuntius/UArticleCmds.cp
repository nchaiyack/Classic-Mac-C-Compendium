// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// TArticleCmds.cp

#include "UArticleCmds.h"
#include "UArticleListView.h"
#include "FileTools.h"
#include "UASyncFile.h"
#include "UGroupDoc.h"
#include "UPrefsDatabase.h"
#include "ErrorGlobals.h"

#include "RsrcGlobals.h"

#include <UGridView.h>

#include <ToolUtils.h>

#pragma segment MyArticle


///---------------------------------------------------------------------
#undef Inherited
#define Inherited TCommand
DefineClass(TShowDiscJunkCommand, TCommand)
TShowDiscJunkCommand::TShowDiscJunkCommand()
{
	fArticleListView = nil;
	fIndex = nil;
}

void TShowDiscJunkCommand::IShowDiscJunkCommand(TArticleListView *articleListView, long index)
{
	Inherited::ICommand(cShowDiscJunkCommand, nil, false, false, nil);
	fArticleListView = articleListView;
	fIndex = index;
}

void TShowDiscJunkCommand::Free()
{
	fArticleListView = nil;
	fIndex = nil;
	Inherited::Free();
}

void TShowDiscJunkCommand::DoIt()
{
	fArticleListView->SetDiscussionDisplayed(fIndex);
}

//---------------------------------------------------------------------
#undef Inherited
#define Inherited TCommand
DefineClass(TUpdateArticleViewCommand, TCommand)
TUpdateArticleViewCommand::TUpdateArticleViewCommand()
{
	fArticleListView = nil;
}

void TUpdateArticleViewCommand::IUpdateArticleViewCommand(TArticleListView *alv, ArticleShowType whatToShow)
{
	Inherited::ICommand(cUpdateArticleListCommand, nil, false, false, nil);
	fArticleListView = alv;
	fWhatToShow = whatToShow;
}

void TUpdateArticleViewCommand::Free()
{
	Inherited::Free();
}

void TUpdateArticleViewCommand::DoIt()
{
	fArticleListView->DoTheUpdateList(fWhatToShow);
}
//---------------------------------------------------------------------
#undef Inherited
#define Inherited TCommand
DefineClass(TStoreNotesCommand, TCommand)
TStoreNotesCommand::TStoreNotesCommand()
{
	fArticleListView = nil;
	fFile = nil;
}

void TStoreNotesCommand::IStoreNotesCommand(TArticleListView *alv)
{
	Inherited::ICommand(cStoreNotes, nil, false, false, nil);
	fArticleListView = alv;
}

void TStoreNotesCommand::Free()
{
	FreeIfObject(fFile); fFile = nil; // closes file
	Inherited::Free();
}

void TStoreNotesCommand::DoIt()
{
	FailInfo fi;
	Try(fi)
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
	Try(fi)
	{
		if (!gPrefs->PrefExists('FNot'))
			FailOSErr(errNoNoteFolder);
		if (!gPrefs->PrefExists('EDid'))
			FailOSErr(errNoEditor);
		OSType signature = gPrefs->GetSignaturePrefs('EDsi');
		fFile = NewAsyncFile('TEXT', signature,
			kUsesDataFork, noResourceFork, !kDataOpen, !kRsrcOpen);
		fFile->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		FSSpec spec;
		gPrefs->GetDirAliasPrefs('FNot', spec);
		CStr255 dotName = fArticleListView->fDoc->GetGroupDotName();
		GoGroupFile(dotName, spec);
		fFile->Specify(spec);
		if (!FileExist(fFile))
		{
			FailOSErr(fFile->CreateFile());
			ChangeFileTypeAndCreator(fFile, 'TEXT', signature);
		}
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
	Try(fi)
	{
		h = fArticleListView->GetSelectionAsQuoteText(false, true, kNoteQuoteTemplate);
		HLock(h);
		long len = GetHandleSize(h);
		FailOSErr(fFile->WriteData(*h, len));
		HUnlock(h);
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}
//---------------------------------------------------------------------
